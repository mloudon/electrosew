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
bool red;

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  strip.setBrightness(BRIGHTNESS);

  red = true;
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
  byte c = getClock(t, 6);
  byte x = getClock(t, 6);
  
  for (int i = 0; i < strip.numPixels(); i++){
    // location of the pixel on a 0-255 scale
    float dist = i * 256. / strip.numPixels();
    byte delta = abs(dist - x);
    // linear ramp up of brightness, for those within 1/8th of the reference point
    float bright = max(255 - 4 * delta,0) / 255.;
    strip.setPixelColor(i, Wheel(c,bright));
  }

  blinkPerFrame();
  
  strip.show();
  delay(20); // to give max 50fps
}

//Blinks the first pixel on and off. Used to check for framerate smoothness.
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

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

uint32_t Grey(byte lum)
{
  return strip.Color(lum, lum, lum);
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
// Max saturation (?)
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
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
