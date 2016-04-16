#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

// Use PIN 6 for most board. But for the trinket, we don't even have pin6, so using
// pin 0 instead.
#define PIN 6
#if defined (__AVR_ATtiny85__)
  #define PIN 0
#else
  #define PIN 6
#endif

// 
#define STRAND_LENGTH 17

#define BRIGHTNESS 63

Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRAND_LENGTH, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  strip.setBrightness(BRIGHTNESS);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  //rainbowCycle(20);
  mainCycle();
}

void mainCycle(){
  uint16_t i;
  int t = millis();
  byte c = getClock(t, 2);
  byte x = getClock(t, 4);
  
  for (int i = 0; i < strip.numPixels(); i++){
    // location of the pixel on a 0-255 scale
    float dist = i * 256. / strip.numPixels();
    byte delta = min(min(abs(dist - x), abs(dist - x + 256)), abs(dist - x - 255));
    // linear ramp up of brightness, for those within 1/8th of the reference point
    float bright = max(255 - 6 * delta, 31) / 255.;
    strip.setPixelColor(i, hsvToRgb(c/255., 1., bright));
  }

  //blinkPerFrame();
  
  strip.show();
  delay(20); // to give max 50fps
}

//Blinks the first pixel on and off. Used to check for framerate smoothness.

bool red;

void blinkPerFrame()
{
    if (red)
      strip.setPixelColor(0, 127, 0, 0);
    else
      strip.setPixelColor(0, 0, 0, 0);
    red = !red;
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
byte getClock(unsigned long mil, byte rate)
{
  return mil >> (8 - rate) % 256; 
}

/** 
 *  HSV/HSL to RGB code From https://github.com/ratkins/RGBConverter
 */

/**
 * Converts an HSV color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes h, s, and v are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 255].
 *
 * @param   Number  h       The hue
 * @param   Number  s       The saturation
 * @param   Number  v       The value
 */
uint32_t hsvToRgb(float h, float s, float v) {
    float r, g, b;

    int i = int(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch(i % 6){
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    return strip.Color(r * 255, g * 255, b * 255);
}

/**
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 255].
 *
 * @param   Number  h       The hue
 * @param   Number  s       The saturation
 * @param   Number  l       The lightness
 * @return  Array           The RGB representation
 */
uint32_t hslToRgb(float h, float s, float l) {
    float r, g, b;

    if (s == 0) {
        r = g = b = l; // achromatic
    } else {
        float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        float p = 2 * l - q;
        r = hue2rgb(p, q, h + 1/3.);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1/3.);
    }

    return strip.Color(r * 255, g * 255, b * 255);
}

float hue2rgb(float p, float q, float t) {
    if(t < 0) t += 1;
    if(t > 1) t -= 1;
    if(t < 1/6) return p + (q - p) * 6 * t;
    if(t < 1/2) return q;
    if(t < 2/3) return p + (q - p) * (2/3 - t) * 6;
    return p;
}
