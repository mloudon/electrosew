#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#include <math.h>

#define PIN 1
#define PIN2 0

/**
    Pattern definition. The program cycles through a range on the wheel, and
    back again. This defines the boundaries. Note that wraparound for the full
    rainbow is not enabled. Would take special case code.
*/

#define RAINBOW
//#define SEAPUNK
//#define INDIGO
//#define BLUE_GREEN
//#define HEART

#if defined (RAINBOW)
#define HUE_START 0
#define HUE_END 1
#define SATURATION 1.
#endif

#if defined (SEAPUNK)
#define HUE_START .333
#define HUE_END .833
#define SATURATION .8
#endif

#if defined (INDIGO)
#define HUE_START .666
#define HUE_END .833
#define SATURATION 1.
#endif

#if defined (BLUE_GREEN)
#define HUE_START .333
#define HUE_END .666
#define SATURATION .9
#endif

#if defined (HElART)
#define HUE_START .833
#define HUE_END 1.
#define SATURATION 1.
#endif



int num_pixels = 7;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(num_pixels, PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(num_pixels, PIN2, NEO_GRBW + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  strip2.begin();
  strip2.show(); // Initialize all pixels to 'off'
}

void loop() {

  unsigned long t = millis()/10;
  byte color = getClock(t, 1);
  float left = HUE_START;
  float right = HUE_END;
  float x = color / 255.;
  if (x >= 1)
    x -= 1.;
  // sweeps the range. for x from 0 to 1, this function does this:
  // starts at (0, _right_), goes to (.5, _left_), then back to (1, _right)
  float hue = abs(2 * (right - left) * x  - right + left) + left;
  float val = ((exp(sin(millis() / 2000.0 * PI)) - 0.36787944) * 108.0)/255.;
  float val_scaled = (val*0.9) + 0.1;
  uint32_t c = hsvToRgb(hue, SATURATION, val_scaled);

  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();

  for (int i = 0; i < strip2.numPixels(); i++) {
    strip2.setPixelColor(i, c);
  }
  strip2.show();

  delay(10);
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
