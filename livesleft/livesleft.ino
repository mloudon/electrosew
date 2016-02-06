#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN 6

float brightness = 0.7;
int num_pixels = 7;
long mils = 0;
int remaining = num_pixels;
bool flash_on = true;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(num_pixels, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {

  if ((mils > 20000L) && (remaining > 1)) {
    remaining --;
    mils = 0;
  } else if (remaining <= 1) {
    remaining = num_pixels;
  }

  if (mils % 100 == 0) {
    flash_on = !flash_on;
  }

  for (int i = 0; i < remaining; i++) {
    if (remaining < 3) {
      if (flash_on) {
        strip.setPixelColor(i,  strip.Color(255 * brightness, 0, 0));
      }
      else {
        strip.setPixelColor(i,  strip.Color(0, 0, 0));
      }
    }
    else {
      strip.setPixelColor(i,  strip.Color(255 * brightness, 0, 0));
    }
  }
  for (int j = remaining; j < num_pixels; j++) {
    strip.setPixelColor(j,  strip.Color(0, 0, 0));
  }
  strip.show();

  delay(10);
  mils += 10L;
  Serial.println(mils);
}
