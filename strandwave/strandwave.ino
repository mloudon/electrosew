#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN 0

#define STRAND_LENGTH 7

float max_brightness = 0.7;
float min_brightness = 0.1;
int wheel_pos = 160;
int wave_center = 0;
int wave_length = 1; // how many pixels on either side of center should be brighter
float fade_step = (max_brightness - min_brightness) / (wave_length + 1);
float wave_counter = 0;
int wave_move_each = 120; //how many milliseconds between wave steps

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRAND_LENGTH, PIN, NEO_GRB + NEO_KHZ800);

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
  
  //strip.setBrightness(32);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {

  if (wave_counter >= wave_move_each) {
    wave_counter = 0;
    wheel_pos += 1;
    if (wheel_pos > 255) {
      wheel_pos -= 255;
    }

    for (int i = 0; i < strip.numPixels(); i++) {
      float brightness = min_brightness;
      if ((i >= wave_center - wave_length) && (i <= wave_center + wave_length)) {
        if (i < wave_center) {
          brightness = max_brightness - ((wave_center - i) * fade_step);
        } else if (i == wave_center) {
          brightness = max_brightness;
        } else if (i > wave_center) {
          brightness = max_brightness - ((i - wave_center) * fade_step);
        }
      }
      strip.setPixelColor(i,  Wheel(wheel_pos, brightness));

    }
    strip.show();

    wave_center += 1;
    if (wave_center >= strip.numPixels()) {
      wave_center = 0;
    }
  }
  else {
    wave_counter += 10;
  }

  delay(10);
}


uint32_t Wheel(byte WheelPos, float brightness) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color((255 - WheelPos * 3) * brightness, 0, WheelPos * 3 * brightness);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3 * brightness, (255 - WheelPos * 3) * brightness);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3 * brightness, (255 - WheelPos * 3) * brightness, 0);
}
