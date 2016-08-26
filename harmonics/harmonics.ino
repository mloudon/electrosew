#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN 6

#define STRAND_LENGTH 9

Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRAND_LENGTH, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  //strip.setBrightness(32);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

float cycle(float t, float period, float min, float max) {
  return .5*(min+max) - .5*(max-min) * cos(t / period * (2*3.14159));
}

float GAMMA = 1.8;
int lum(float t, float period) {
  int MAXBRIGHT = 48;
  // Gamma correction is expensive. Disable it for longer strips or weaker microcontrollers.
  return MAXBRIGHT*pow(cycle(t, period, 0, 1), GAMMA);
}

// One complete cycle every this many seconds.
float CYCLE = 30;

// Go through each possible permutation of R/G/B
int channel_map[6][3] = {
  {0, 1, 2},
  {0, 2, 1},
  {1, 0, 2},
  {1, 2, 0},
  {2, 0, 1},
  {2, 1, 0}  
};

// Assign each color channel of each pixel a different harmonic frequency. The
// first cycles once per 'CYCLE' time. The 2nd twice per cycle. Then 3x, 4x, 5x,
// etc. The pixle channels move in and out of phase with different harmonics
// visible as the cycle progresses. Everything syncs up at the end of a complete
// cycle.

// After each cycle the assignment of frequencies to pixels/channels changes.
// There are 12 total possibilities for assignment.

void loop() {
  float t = millis() / 1000.;

  int round = (int)(t / CYCLE) % (2*6);

  // style is the order in which frequencies are assigned:
  // 0: iterate over pixels first, then iterate over the channel (i.e., assign
  //    px0 R, px1 R, px2 R... , px0 G, px1 G, ...
  // 1: iterate over the channels, then the pixel, i.e., px0 R, px0 G, px0 B,
  //    px1 R, px1 G, px1 B...
  // The actual order of R->G->B will change each cycle.
  int style = round % 2;

  // phase is one of the 6 different ways to permute RGB
  int phase = (round / 2);

  for (int px = 0; px < STRAND_LENGTH; px++) {
    int vals[3];
    for (int ch = 0; ch < 3; ch++) {
      if (style == 0) {
        vals[ch] = lum(t, CYCLE/(ch*STRAND_LENGTH+px+1));
      } else {
        vals[ch] = lum(t, CYCLE/(3*px+ch+1));
      }
    }

    strip.setPixelColor(px, 
       vals[channel_map[phase][0]],
       vals[channel_map[phase][1]],
       vals[channel_map[phase][2]]
    );
  }
  strip.show();
  delay(10);
}



