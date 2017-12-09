#include <FastLED.h>
#include <Bounce2.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

// description of UI controls
// - in the default mode, party mode is activated whenever the button is pressed
// - pressing 3 times within 1.5 seconds will switch to the next pattern (party mode is included in the rotation)
// - pressing 4 times within 1.5 seconds will activate brightness adjust mode
// - pressing 5 times within 1.5 seconds will activate color adjust mode
// - pressing 7 times within 1.5 seconds will toggle various strobing modes to keep crappy battery packs alive
// - in brightness or color adjust mode:
//   - hold the button to cycle through the values. all other button actions are disabled
//   - after 10 seconds without pressing the button, it will return to default mode
//   - the first few LEDs will change to indicate the current mode:
//     - for brightness mode, they will set to the max possible brightness for the pattern
//     - for color mode, they will show a rainbow

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

// A configuration - trinket, apa
#define STRAND_LENGTH 36

// B configuration - nano, apa
//#define STRAND_LENGTH 30

// C configuration - nano, ws2811
//#define STRAND_LENGTH 60
//#define DATA_PIN 6
//#define LED_WS2811

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

// max allowed brightness within a pattern. adjustable
int MAX_BRIGHTNESS = 128;
// max value that max brightness can be adjusted to
int BRIGHTNESS_ADJUST_MAX = 255;
// min value that max brightness can be adjusted to
int BRIGHTNESS_ADJUST_MIN = 16;

// convert an abstract brightness value to appropriate HSV brightness value (0-255)
// brightness is a conceptual value between 0 (min) and 1 (max)
// min_brightness, also valued [0, 1], is the actual luminance value that brightness 0 is mapped to (i.e., "black level")
//   this is to always give some baseline glow, and to avoid unpleasant HSV dithering at very dark levels
// the result value is scaled to MAX_BRIGHTNESS (note that turning down MAX_BRIGHTNESS also scales down the black level-- may want to revisit this)
int brightness_to_value(float brightness, float min_brightness) {
  return blend(min_brightness, 1., brightness) * MAX_BRIGHTNESS;
}

// admin bookkeeping shit
// how much of the most recent button presses to track
const int button_press_buffer_size = 7;
// timestamps of most recent N presses, as a cyclical buffer
long button_press_buffer[button_press_buffer_size];
// index to implement above cyclical buffer
int button_press_buffer_ix = 0;
// last timestamp in which button was depressed (note: NOT a discrete unpressed->pressed event)
long last_pressed = 0;
// window in which to count button presses to do special things
const float button_multipress_timeout = 1.5; // s
// timestamp in which we started counting button presses - set to first press outside an active window
long button_multipress_window_start = 0;
// timeout before we revert out of admin mode
const float admin_mode_timeout = 10; // s
// 'tick' for updates to hue/brightness - this rate limits how quickly we cycle through
int admin_mode_tick = 15; // ms
// timestamp of last discrete update to hue/brightness
long last_admin_adjust = 0;
// whether brightness is being adjusted in the up or down direction
bool brightness_adjust_dir_up = false;

int keepalive_mode = 0;
int num_keepalive_modes = 7;
long last_strobe = 0;

void setup() {
  #if defined (LED_WS2811)
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, STRAND_LENGTH).setCorrection( TypicalLEDStrip );
  #else
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, STRAND_LENGTH).setCorrection( TypicalLEDStrip );
  #endif
 
  // don't set global brightness here to anything other than max -- do brightness scaling in software; gives a better appearance with less flicker
  FastLED.setBrightness( 255 );
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  debouncer.attach(BUTTON_PIN);
  debouncer.interval(20);

  for (int i = 0; i < button_press_buffer_size; i++) {
    button_press_buffer[i] = 0;
  }
}

// currently displayed pattern
int mode = 1;
int num_modes = 5;

// meta-mode, 0 for default, >0 for adjusting some parameter
int admin_mode = 0;

int BASE_HUE = 175;

void pattern_static() {
  float rel_brightness = .5;
  for (int i = 0; i < STRAND_LENGTH; i++) {
    leds[i] = CHSV(BASE_HUE, 255, brightness_to_value(rel_brightness, 0));
  }  
}

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
    float left = BASE_HUE / 256. - HUE_SPREAD / 2;
    float right = left + HUE_SPREAD;
    float x = color / 255. + pix * .5 / ARM_LENGTH;
    if (x >= 1)
      x -= 1.;
    // sweeps the range. for x from 0 to 1, this function does this:
    // starts at (0, _right_), goes to (.5, _left_), then back to (1, _right)
    int hue = 255 * (abs(2 * (right - left) * x  - right + left) + left);
    hue = hue % 256;

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
  unsigned long t = millis();
  float clock = t / 1000.;

  int display_mode = mode;

  debouncer.update();
  if (debouncer.fell()) {
    // capture the times of the N most recent button presses
    button_press_buffer[button_press_buffer_ix] = t;
    button_press_buffer_ix = (button_press_buffer_ix + 1) % button_press_buffer_size;
    if (button_multipress_window_start == 0) {
      button_multipress_window_start = t;    
    }
  }
  if (button_multipress_window_start > 0 && button_multipress_window_start + button_multipress_timeout * 1000 < t) {
    // count the number of presses within the recently completed window to see if we should trigger something special (only if in default mode)
    int num_presses = 0;
    for (int i = 0; i < button_press_buffer_size; i++) {
      long delta = button_press_buffer[i] - button_multipress_window_start;
      if (delta >= 0 && delta < button_multipress_timeout * 1000) {
        num_presses++;
      }
    }
    button_multipress_window_start = 0;

    if (admin_mode == 0) {
      if (num_presses == 3) {
        // change pattern, stay in default mode
        mode = (mode + 1) % num_modes;
      } else if (num_presses == 4) {
        // brightness change mode
        admin_mode = 1;
      } else if (num_presses == 5) {
        // hue change mode
        admin_mode = 2;
      } else if (num_presses == 7) {
        // change keepalive mode
        keepalive_mode = (keepalive_mode + 1) % num_keepalive_modes;
      }
    }
  }
  // if button pressed...
  if (debouncer.read() == LOW) {
    last_pressed = t;
    // prevent cycling the parameter too fast
    bool admin_lockout = t - last_admin_adjust < admin_mode_tick;

    if (admin_mode == 0) {
      // in default mode, engage party mode
      display_mode = 0;
    }
    if (!admin_lockout) {
      if (admin_mode == 1) {
        MAX_BRIGHTNESS += (brightness_adjust_dir_up ? 1 : -1);
        if (MAX_BRIGHTNESS <= BRIGHTNESS_ADJUST_MIN || MAX_BRIGHTNESS >= BRIGHTNESS_ADJUST_MAX) {
          MAX_BRIGHTNESS = max(min(MAX_BRIGHTNESS, BRIGHTNESS_ADJUST_MAX), BRIGHTNESS_ADJUST_MIN);
          brightness_adjust_dir_up = !brightness_adjust_dir_up;
        }
      } else if (admin_mode == 2) {
        BASE_HUE = (BASE_HUE + 1) % 256;
      }
      last_admin_adjust = t;
    }
  }
  // in an admin mode, button presses only change the relevant variable, so use an inactivity timeout to return to default mode
  if (admin_mode > 0 && t - last_pressed > admin_mode_timeout * 1000) {
    admin_mode = 0;
  }

  // render current pattern
  if (display_mode == 0) {
    pattern_rainbow_blast(clock);
  } else if (display_mode == 1) {
    pattern_variable_pulses(clock);
  } else if (display_mode == 2) {
    pattern_breathe(clock);
  } else if (display_mode == 3) {
    pattern_perlin_noise(t);
  } else if (display_mode == 4) {
    pattern_static();
  }

  // for special admin modes, overwrite the first few leds to indicate the mode
  int admin_indicator_size = 5;
  if (admin_mode == 1) {
    for (int i = 0; i < admin_indicator_size; i++) {
      leds[i] = CHSV(BASE_HUE, 255, MAX_BRIGHTNESS);
    }    
  } else if (admin_mode == 2) {
    for (int i = 0; i < admin_indicator_size; i++) {
      leds[i] = CHSV(256. * i / admin_indicator_size, 255, MAX_BRIGHTNESS);
    }
  }

  // thwart low current cutoff
  int keepalive_strobe_delay = 0;
  int keepalive_strobe_duration = 60;
  int num_keepalive_pixels = 2;
  if (keepalive_mode == 0) {
    // off
    num_keepalive_pixels = 0;
  } else if (keepalive_mode == 1) {
    // 2px @ 5s
    keepalive_strobe_delay = 5000;
  } else if (keepalive_mode == 2) {
    // 3px @ 5s
    keepalive_strobe_delay = 5000;
    num_keepalive_pixels = 3;
  } else if (keepalive_mode == 3) {
    // 2px @ 1s
    keepalive_strobe_delay = 1000;
  } else if (keepalive_mode == 4) {
    // 3px @ 1s
    keepalive_strobe_delay = 1000;
    num_keepalive_pixels = 3;
  } else if (keepalive_mode == 5) {
    // 2px continuous
  } else if (keepalive_mode == 6) {
    // 3px continuous
    num_keepalive_pixels = 3;
  }

  bool keepalive = false;
  if (t - last_strobe > keepalive_strobe_delay) {
    keepalive = true;
    last_strobe = t;
  } else if (t - last_strobe < keepalive_strobe_duration) {
    keepalive = true;
  }
  if (keepalive) {
    for (int i = 0; i < num_keepalive_pixels; i++) {
      leds[STRAND_LENGTH - 1 - i] = CHSV(0, 0, 255);
    }
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
