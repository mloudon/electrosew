#include "Adafruit_WS2801.h"
#ifdef __AVR__
#include <avr/power.h>
#endif

/*
 * Harmonic cycling color pattern.
 * 
 * Each color channel of each pixel is assigned a harmonic frequency from 1 to N. The first
 * frequency cycles once per cycle, the next 2x per cylce, and then 3x, 4x, 5x, etc. Pixel
 * channels move in and out of phase, with different sub-harmonics visible as the cycle
 * progresses. Everything syncs up at the end of a complete cycle.
 * 
 * Each cycle also varies the style/ordering in which the harmonics are assigned to individual
 * pixels/channels (12 such total variations).
 */

uint8_t dataPin  = 0;    // Yellow wire on Adafruit Pixels
uint8_t clockPin = 2;    // Green wire on Adafruit Pixels
#define FRAME_STEP 10  // ms

// Number of pixels. Total number of harmonics is 3 (R/G/B) * #pixels. A cycle completes when all harmonics go back in sync.
// Note: the integer nature of the harmonics is critical to the effect.
const int STRIP_LENGTH = 25;

// Duration of cycle for a strip of 'baseline' pixels. Actual cycle length is changed accordingly with
// actual number of pixels in strip.
const float BASELINE_CYCLE_LENGTH = 30.;  // s
const int BASELINE_STRIP_LENGTH = 9;

const int MAX_BRIGHTNESS = 48;  // max 256; don't set too high or arduino gets stressed and can't be easily reflashed
const bool USE_GAMMA_CORRECTION = false;  // more linear brightness in color ramp, but expensive. disable if framerate is too low on long strips
const float GAMMA = 1.8;

// Every channel of every pixel must be assigned a harmonic frequency of [1, 3*#pixels]
enum interleaving {
  // Assign the harmonics in order for all the pixels of a single channel before moving on to the next
  // channel. This is the conventional harmonic pattern.
  channel,
  // Assign the harmonics in order for all the channels of a pixel before moving on to the next pixel.
  // This creates an 'ants marching'-type look and is kind of an 'inside out' view of the previous mode.
  // May not look good if there are too many pixels. 
  pixel
};
const int num_enabled_interleave_modes = 2;
interleaving interleave_modes[num_enabled_interleave_modes] = {channel, pixel};

// don't modify
#define PI 3.14159
#define NUM_CHANNELS 3  // R, G, B
const float CYCLE_LENGTH = BASELINE_CYCLE_LENGTH * STRIP_LENGTH / BASELINE_STRIP_LENGTH;
/////////////

Adafruit_WS2801 strip = Adafruit_WS2801(25, dataPin, clockPin);

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

float cycle(float t, float period, float min, float max) {
  return .5*(min+max) - .5*(max-min) * cos(t / period * (2*PI));
}

int lum(float t, float period) {
  float k = cycle(t, period, 0, 1);
  return MAX_BRIGHTNESS * (USE_GAMMA_CORRECTION ? pow(k, GAMMA) : k);
}

// All the ways to permute the R/G/B color channels.
#define NUM_CHANNEL_PERMUTATIONS 6
int channel_permutations[NUM_CHANNEL_PERMUTATIONS][NUM_CHANNELS] = {
  {0, 1, 2},
  {0, 2, 1},
  {1, 0, 2},
  {1, 2, 0},
  {2, 0, 1},
  {2, 1, 0}
};

void loop() {
  float t = millis() / 1000.;

  // Number of full cycles completed.
  int cycle_count = (int)(t / CYCLE_LENGTH);
  // Convert the current cycle number into the appropriate interleaving and permutation modes.
  interleaving interleave_mode = interleave_modes[cycle_count % num_enabled_interleave_modes];
  int* channel_permutation = channel_permutations[(cycle_count / num_enabled_interleave_modes) % NUM_CHANNEL_PERMUTATIONS];

  for (int px = 0; px < STRIP_LENGTH; px++) {
    int channels[NUM_CHANNELS];
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
      // Determine the harmonic frequency for the pixel/channel.
      int harmonic;
      switch (interleave_mode) {
      case channel:
        harmonic = ch * STRIP_LENGTH + px + 1;
        break;
      case pixel:
        harmonic = px * NUM_CHANNELS + ch + 1;
        break;
      }
      // Set the current luminosity for that harmonic.
      channels[ch] = lum(t, CYCLE_LENGTH / harmonic);
    }

    // Assign the luminosities to a pixel color according to the current channel permutation.
    strip.setPixelColor(px, 
       channels[channel_permutation[0]],
       channels[channel_permutation[1]],
       channels[channel_permutation[2]]
    );
  }
  
  strip.show();
  delay(FRAME_STEP);
}



