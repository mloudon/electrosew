/*
  Pixie reads data in at 115.2k serial, 8N1.
  Byte order is R1, G1, B1, R2, G2, B2, ... where the first triplet is the
  color of the LED that's closest to the controller. 1ms of silence triggers
  latch. 2 seconds silence (or overheating) triggers LED off (for safety).

  Do not look into Pixie with remaining eye!
*/

#include "SoftwareSerial.h"
#include "Adafruit_Pixie.h"

#define NUMPIXELS 1 // Number of Pixies in the strip
#define PIXIEPIN  1 // Pin number for SoftwareSerial output

#define NUMPIXELS2 1 // Number of Pixies in the strip
#define PIXIEPIN2  2 // Pin number for SoftwareSerial output

SoftwareSerial pixieSerial(-1, PIXIEPIN);
SoftwareSerial pixieSerial2(-1, PIXIEPIN2);

Adafruit_Pixie strip = Adafruit_Pixie(NUMPIXELS, &pixieSerial);
Adafruit_Pixie strip2 = Adafruit_Pixie(NUMPIXELS, &pixieSerial2);

void setup() {
  pixieSerial.begin(115200); // Pixie REQUIRES this baud rate
  strip.setBrightness(200);  // Adjust as necessary to avoid blinding

  pixieSerial2.begin(115200); // Pixie REQUIRES this baud rate
  strip2.setBrightness(200);  // Adjust as necessary to avoid blinding

  delay(300);

}

void loop() {
  rainbowCycle(40);
}


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < NUMPIXELS; i++) {
      uint32_t pos = Wheel(((i * 256 / strip.numPixels()) + j) & 255);
      strip.setPixelColor(i, pos);
      uint32_t pos2 = Wheel(((i * 256 / strip.numPixels()) + j + 30) & 255);
      strip2.setPixelColor(i, pos2);
    }
    strip.show();
    strip2.show();
    delay(wait);
  }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
