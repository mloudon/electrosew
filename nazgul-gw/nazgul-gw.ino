#include <FastLED.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

/**
 * This part is for hardware related config. It allows you to spec the board
 * you are using, along with the LED strand length/brightness. The brightness is
 * typically used to limit max power draw: at 60mA per LED (full white), you can
 * power at most 8 LEDs from the 500mA 5V pin on boards like the Trinket/Arduino
 * Nano. As the strand gets longer, you should use brightness to limit max current
 * draw. However, the typical pattern won't ever reach full white on all LEDs, so
 * the actual max current varies. It's probably best established via direct 
 * measurement. An alternative reason to limit brightness is to improve battery
 * life.
 *
 * Current configs:
 * 
 *  * Arduino Nano, use pin 6
 *  * Adafruit Trinket 5V 16Mhz, use pin 0
 */

#define PIN 6


#define STRAND_LENGTH 36
//#define BRIGHTNESS 127
#define BRIGHTNESS 255

/**
 * Whether the pattern is mirrored, or reversed. This is useful for scarfs where 
 * the LEDs are all daisy chained. An alternative is to have the center pixel
 * being the first one, and split the d-out line down either sides
 */
//#define MIRRORED
#define REVERSED

#if defined (MIRRORED)
  #define ARM_LENGTH (STRAND_LENGTH /2)
#else
  #define ARM_LENGTH STRAND_LENGTH
#endif
  

/** 
 *  Pattern definition. The program cycles through a range on the wheel, and
 *  back again. This defines the boundaries. Note that wraparound for the full
 *  rainbow is not enabled. Would take special case code.
 */

//#define RAINBOW
//#define SEAPUNK
#define INDIGO
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

#if defined (HEART)
  #define HUE_START .833
  #define HUE_END 1.
  #define SATURATION 1.
#endif

CRGB leds[STRAND_LENGTH];

const int MAX_BRIGHTNESS = 255;  // max 256; don't set too high or arduino gets stressed and can't be easily reflashed
const bool USE_GAMMA_CORRECTION = true;  // more linear brightness in color ramp, but expensive. disable if framerate is too low on long strips
const float GAMMA = 1.8;

float cycle(float t, float period, float min, float max) {
  return .5*(min+max) - .5*(max-min) * cos(t / period * (2*PI));
}

int lum(float t, float period) {
  float k = cycle(t, period, 0, 1);
  return MAX_BRIGHTNESS * (USE_GAMMA_CORRECTION ? pow(k, GAMMA) : k);
}

void setup() {
  FastLED.addLeds<APA102, 5, 4, BGR>(leds, STRAND_LENGTH).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
  pinMode(9,INPUT_PULLUP);
}

int offset = 0;

int mode = 0;
int num_modes = 3;

void loop(){

    // needs debouncing
    if (digitalRead(9) == LOW)
    {
      mode = (mode + 1) % num_modes;
    }

  
  unsigned long t = millis();

  if (mode == 0) {

  // rainbow
  for (int i = 0; i < STRAND_LENGTH; i++) {
    leds[i] = CHSV(10*i + 1*millis(), 255, 255);
  }

  } else if (mode == 1) {

  // breathe
  float bright = cycle(millis(), 12000, 0, 1);
  bright = pow(bright, 25);
  int v = (.07+.93*bright)*255;
  for (int i = 0; i < STRAND_LENGTH; i++) {
    leds[i] = CHSV(175, 255, v);
  }

  } else if (mode == 2) {

  // variable pulse
  float len = 1. / cycle(millis(), 20000, 1./(STRAND_LENGTH * 2.5), 1/5.);
  float offset = millis() / 1000.;
  for (int i = 0; i < STRAND_LENGTH; i++) {
    float bright = cycle(i + offset*len, len, 0, 1);
    bright = pow(bright, 3);
    int v = (.05+.95*bright)*255;
    leds[i] = CHSV(170, 255, v);
  }

  }

  /* perlin
  byte color = getClock(t, 2);
  byte pulse = inoise8(t / 4.) * .5;
  byte drift = getClock(t, 3);
  pulse += drift;
  if (pulse > 255)
    pulse -= 255;

  for (byte pix = 0; pix < ARM_LENGTH; pix++){
    // location of the pixel on a 0-RENDER_RANGE scale.
    byte dist = pix * 255 / ARM_LENGTH;

    // messy, but some sort of least-of-3 distances, allowing wraping.
    byte delta = min(min(abs(dist - pulse), abs(dist - pulse + 256)), abs(dist - pulse - 255));  
    // linear ramp up of brightness, for those within 1/8th of the reference point   
    float value = max(255 - 6 * delta, 64);

    // hue selection. Mainly driving by c, but with some small shifting along
    // the length of the strand.

    // sweep of a subset of the spectrum. 
    float left = HUE_START;
    float right = HUE_END;
    float x = color / 255. + pix * .5 / ARM_LENGTH;
    if (x >= 1)
      x -= 1.;
    // sweeps the range. for x from 0 to 1, this function does this:
    // starts at (0, _right_), goes to (.5, _left_), then back to (1, _right)
    float hue = 255 * (abs(2 * (right - left) * x  - right + left) + left);
    
    byte loc = pix;
    #if defined (REVERSED)
      loc = ARM_LENGTH - 1 - pix;
    #endif

    if (digitalRead(9) == LOW)
    {
       offset++;
    }
    
    leds[loc] = CHSV((hue + (offset >> 4) % 255), 255 * SATURATION, value);
    
    #if defined (MIRRORED)
      leds[STRAND_LENGTH - 1 - loc] = CHSV(hue, 255 * SATURATION, value);
    #endif
  }
  */

  // delay 20ms to give max 50fps. Could do something fancier here to try to 
  // hit exactly 60fps (or whatever) if possible, but takinng another millis()
  // reading, but not sure if there would be a point to that. 
  FastLED.show(); // display this frame
  //FastLED.delay(20);
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
