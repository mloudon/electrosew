#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN 6
#define NUM_PIXELS 9
#define SATURATION 0.9
#define BRIGHTNESS 0.6

int step;
unsigned long sorted_interval = 10000;
unsigned long step_interval = 1000;
unsigned long previous_millis = millis();
unsigned long previous_step_millis = millis();
unsigned long spin_interval = 100;
unsigned long previous_spin_millis = millis();

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  Serial.println("starting");

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  step = 100 / (NUM_PIXELS * 1);
  randomSeed(analogRead(0));
}

void loop() {
  previous_millis = millis();
  run_sort();
  if ((unsigned long)(millis() - previous_millis) >= sorted_interval) {
    run_sort();
    previous_millis = millis();
  }

}

void run_sort() {
  float start_hue = random(100);
  int pix_colors[strip.numPixels()];

  for (int i = 0; i < strip.numPixels(); i++) {
    pix_colors[i] = start_hue  + i * step;
    if (pix_colors[i] > 100) {
      pix_colors[i] -= 100;
    }
    uint32_t c = hsvToRgb(pix_colors[i] / 100., SATURATION, BRIGHTNESS);
    strip.setPixelColor(i, c);
  }

  shuffle(pix_colors, strip.numPixels());
  update_strip(pix_colors);
  Serial.println("sorting");
  int n = sizeof pix_colors / sizeof pix_colors[0];
  bubble_sort(pix_colors, n);
  Serial.println("sorted");
  for (int i = 0; i < strip.numPixels(); i++) {
    spin_strip(pix_colors);
  }
}

void spin_strip(int *array) {
  int source[strip.numPixels()];
  memcpy(source, array, sizeof(array));
  int dest[strip.numPixels()];

  for (int i = 0; i < strip.numPixels(); i++) {
    for (int j = 1; j < strip.numPixels(); j++) {
      dest[j] = source[j - 1];
    }
    dest[0] = source[strip.numPixels() - 1];
    memcpy(source, dest, sizeof(dest));
    update_strip(source);
    while (!((unsigned long)(millis() - previous_spin_millis) >= spin_interval)) {
      delay(10);
      //update_strip(source);
    }
    previous_spin_millis = millis();
  }
}

void update_strip(int *array) {
  for (int i = 0; i < strip.numPixels(); i++) {
    uint32_t c = hsvToRgb(array[i] / 100.0, SATURATION, BRIGHTNESS);
    strip.setPixelColor(i, c);
  }

  strip.show();

}


void shuffle(int *array, size_t n)
{
  if (n > 1)
  {
    size_t i;
    for (i = 0; i < n - 1; i++)
    {
      size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
      int t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
  }
}

void quick_sort (int *a, int n) {
  int i, j, p, t;
  if (n < 2)
    return;
  p = a[n / 2];
  for (i = 0, j = n - 1;; i++, j--) {
    while (a[i] < p)
      i++;
    while (p < a[j])
      j--;
    if (i >= j)
      break;
    t = a[i];
    a[i] = a[j];
    a[j] = t;
    update_strip(a);
    while (!((unsigned long)(millis() - previous_step_millis) >= step_interval)) {
      delay(10);
      //update_strip(a);
    }
    Serial.println("step");
    previous_step_millis = millis();
  }
  quick_sort(a, i);
  quick_sort(a + i, n - i);
}

void bubble_sort (int *a, int n) {
  int i, t, s = 1;
  while (s) {
    s = 0;
    for (i = 1; i < n; i++) {
      if (a[i] < a[i - 1]) {
        t = a[i];
        a[i] = a[i - 1];
        a[i - 1] = t;
        s = 1;
        update_strip(a);
        while (!((unsigned long)(millis() - previous_step_millis) >= step_interval)) {
          delay(10);
          //update_strip(a);
        }
        Serial.println("step");
        previous_step_millis = millis();
      }
    }
  }
}

/**
    HSV/HSL to RGB code From https://github.com/ratkins/RGBConverter
*/

/**
   Converts an HSV color value to RGB. Conversion formula
   adapted from http://en.wikipedia.org/wiki/HSV_color_space.
   Assumes h, s, and v are contained in the set [0, 1] and
   returns r, g, and b in the set [0, 255].

   @param   Number  h       The hue
   @param   Number  s       The saturation
   @param   Number  v       The value
*/
uint32_t hsvToRgb(float h, float s, float v) {
  float r, g, b;

  byte i = byte(h * 6);
  float f = h * 6 - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);

  switch (i % 6) {
    case 0: r = v, g = t, b = p; break;
    case 1: r = q, g = v, b = p; break;
    case 2: r = p, g = v, b = t; break;
    case 3: r = p, g = q, b = v; break;
    case 4: r = t, g = p, b = v; break;
    case 5: r = v, g = p, b = q; break;
  }

  return strip.Color(r * 255, g * 255, b * 255);
}


// Get a byte that cycles from 0-255, at a specified rate
// typically, assign mil using mills();
// rates, approximately (assuming 256ms in a second :P)
// 8: 4hz
// 7: 2hz
// 6: 1hz
// 5: 1/2hz
// 4: 1/4hz
// 3: 1/8hz
// 2: 1/16hz
// 1: 1/32hz
byte getClock(unsigned long mil, byte rate)
{
  return mil >> (8 - rate) % 256;
}

/**
   Converts an HSL color value to RGB. Conversion formula
   adapted from http://en.wikipedia.org/wiki/HSL_color_space.
   Assumes h, s, and l are contained in the set [0, 1] and
   returns r, g, and b in the set [0, 255].

   @param   Number  h       The hue
   @param   Number  s       The saturation
   @param   Number  l       The lightness
   @return  Array           The RGB representation
*/
uint32_t hslToRgb(float h, float s, float l) {
  float r, g, b;

  if (s == 0) {
    r = g = b = l; // achromatic
  } else {
    float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    float p = 2 * l - q;
    r = hue2rgb(p, q, h + 1 / 3.);
    g = hue2rgb(p, q, h);
    b = hue2rgb(p, q, h - 1 / 3.);
  }

  return strip.Color(r * 255, g * 255, b * 255);
}

float hue2rgb(float p, float q, float t) {
  if (t < 0) t += 1;
  if (t > 1) t -= 1;
  if (t < 1 / 6) return p + (q - p) * 6 * t;
  if (t < 1 / 2) return q;
  if (t < 2 / 3) return p + (q - p) * (2 / 3 - t) * 6;
  return p;
}
