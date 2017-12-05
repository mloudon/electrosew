#include <FastLED.h>
#include <Bounce2.h>

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
 */
 
#define DATA_PIN 5
#define CLOCK_PIN 4
#define BUTTON_PIN 9

#define STRAND_LENGTH 36

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

CRGB leds[STRAND_LENGTH];

Bounce debouncer = Bounce();

// return a cyclical (sine wave) value between min and max
float cycle(float t, float period, float min, float max) {
  return .5*(min+max) - .5*(max-min) * cos(t / period * (2*PI));
}

// blend linearly between a and b, fraction=0 returns a, fraction=1 returns b
float blend(float a, float b, float fraction) {
  return (1 - fraction) * a + fraction * b;
}

int MAX_BRIGHTNESS = 255;

// convert an abstract brightness value to appropriate HSV brightness value (0-255)
// brightness is a conceptual value between 0 (min) and 1 (max)
// min_brightness, also valued [0, 1], is the actual luminance value that brightness 0 is mapped to (i.e., "black level")
//   this is to always give some baseline glow, and to avoid unpleasant HSV dithering at very dark levels
// the result value is scaled to MAX_BRIGHTNESS (note that turning down MAX_BRIGHTNESS also scales down the black level-- may want to revisit this)
int brightness_to_value(float brightness, float min_brightness) {
  return blend(min_brightness, 1., brightness) * MAX_BRIGHTNESS;
}

void setup() {
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, STRAND_LENGTH).setCorrection( TypicalLEDStrip );
  // don't set global brightness here to anything other than max -- do brightness scaling in software; gives a better appearance with less flicker
  FastLED.setBrightness( MAX_BRIGHTNESS );
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  debouncer.attach(BUTTON_PIN);
  debouncer.interval(100);
}

int mode = 0;
int num_modes = 4;

int BASE_HUE = 175;

void pattern_rainbow_blast(float clock) {
  float per_pixel_hue_jump = 10;
  float crawl_speed_factor = 1000;
  
  for (int i = 0; i < STRAND_LENGTH; i++) {
    leds[i] = CHSV(per_pixel_hue_jump*i + crawl_speed_factor*clock, 255, MAX_BRIGHTNESS);
  }
}

void pattern_breathe(float clock) {
  float period = 20.; // s
  int sub_breaths = 3;
  float peakedness = 25;
  float min_brightness = .15; //.07;
  float sub_breath_intensity = .25; // even if sub_breaths == 1, this also helps slope the come-on of the 'main' breath, which due to the peaking factor spends most of the time at 0 intensity
  
  float main_breath = cycle(clock, period, 0, 1);
  main_breath = pow(main_breath, peakedness);
  float sub_breath = cycle(clock, period/sub_breaths, 0, 1);
  float brightness = blend(main_breath, sub_breath, sub_breath_intensity);
  int value = brightness_to_value(brightness, min_brightness);
  for (int i = 0; i < STRAND_LENGTH; i++) {
    leds[i] = CHSV(BASE_HUE, 255, value);
  }  
}

void pattern_variable_pulses(float clock) {
  float period = 30; // s
  float peakedness = 3;
  float min_pulse_width = 5.;
  float max_pulse_width = STRAND_LENGTH * 2.5;
  float crawl_speed_factor = 1;  // around 1 is the sweet spot; changing this too much seems to look much worse
  float min_brightness = .05;

  // cycle in the inverse space to balance short vs. long pulses better
  float pulse_width = 1. / cycle(clock, period, 1./min_pulse_width, 1./max_pulse_width);
  float crawl_offset = crawl_speed_factor * clock;
  for (int i = 0; i < STRAND_LENGTH; i++) {
    float brightness = cycle(i + crawl_offset*pulse_width, pulse_width, 0, 1);
    brightness = pow(brightness, peakedness);
    int value = brightness_to_value(brightness, min_brightness);
    leds[i] = CHSV(BASE_HUE, 255, value);
  }  
}

void pattern_perlin_noise(long t) {
  float HUE_SPREAD = .333;
  float SATURATION = .75;
  
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
    float left = BASE_HUE / 256.;
    float right = left + HUE_SPREAD;
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

    leds[loc] = CHSV(hue, 255 * SATURATION, brightness_to_value(value / 255., 0.));

    #if defined (MIRRORED)
      leds[STRAND_LENGTH - 1 - loc] = CHSV(hue, 255 * SATURATION, value);
    #endif
  }  
}

void loop(){

  debouncer.update();
  //int value = debouncer.read();

  //if (digitalRead(BUTTON_PIN) == LOW) {
  //  BASE_HUE = (BASE_HUE + 1) % 256;
  //}
  
  if ( debouncer.fell() ) {
    mode = (mode + 1) % num_modes;
  }
  
  unsigned long t = millis();
  float clock = t / 1000.;

  if (mode == 0) {
    pattern_rainbow_blast(clock);
  } else if (mode == 1) {
    pattern_breathe(clock);
  } else if (mode == 2) {
    pattern_variable_pulses(clock);
  } else if (mode == 3) {
    pattern_perlin_noise(t);
  }
  
  // delay 20ms to give max 50fps. Could do something fancier here to try to 
  // hit exactly 60fps (or whatever) if possible, but takinng another millis()
  // reading, but not sure if there would be a point to that.
  FastLED.show(); // display this frame
  //FastLED.delay(20);  // delay seems to cause weird flickering
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
