#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code


  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  while (true) {
    morse(millis());
  }
}

#define TWOPI 6.28

// Scroll morse code down the strip.
void morse(long t) {
  uint8_t step = 75;  // Advance every N milliseconds
  int stretch = 2; // The smallest unit is this many pixels
  // dash=3 on, dot = 1 on, new letter=3 off
  // Dee Tee Eff...
  uint8_t code[] = {1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0};
  int len = 28;
  
  uint32_t A = strip.Color(0, 128, 0);
  uint32_t B = strip.Color(0, 0, 0);

  int x = t / step;
  for (uint16_t i=0; i<strip.numPixels(); i++) {
    uint32_t color_on = Wheel(t/400+0*i, .5); // Slow color cycle
    uint32_t color_off = B;
    strip.setPixelColor(i, (code[(i + x)/stretch % len] ? color_on : color_off));
  }
  strip.show();
}

// Flash the strip in exponentially smaller alternating segments.
void doCoolShit(uint8_t wait) {
  uint32_t A = strip.Color(0, 128, 0);
  uint32_t B = strip.Color(0, 0, 0);
  uint32_t C = A; //strip.Color(0, 128, 0);
  uint32_t D = B; //strip.Color(0, 0, 0);
  
  uint8_t EXP = 6;
  uint8_t CYCLES = 4;
  
  for (int8_t e = EXP; e >= 0; e--) {
    for (uint8_t cyc = 0; cyc < CYCLES; cyc++) {
      for (uint8_t k = 0; k < 2; k++) {
        for(uint16_t i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i, (i / (1 << e) + k) % 2 == 0 ? (e % 2 == 0 ? A : C): (e % 2 == 0 ? B : D));
        }
        strip.show();
        delay(wait);
      }
    } 
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos, float brightness) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color((255 - WheelPos * 3) * brightness, 0, WheelPos * 3 * brightness);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3 * brightness, (255 - WheelPos * 3) * brightness);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3 * brightness, (255 - WheelPos * 3) * brightness, 0);
}
