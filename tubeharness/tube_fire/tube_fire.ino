/**
 * Flameish looking thing for tube harness. Doesn't use any sensor input
 * 
 * Shoots particles with randomized hue down the strands
 * Current set up is for two 30 LED strands driven by Arduino Nano
 */

#include <FastLED.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN_RIGHT 6
#define PIN_LEFT 7

#define STRAND_LENGTH 30
#define BRIGHTNESS 127

#define SUPER_FAC 10
#define NUM_POS 300 // the number of particles - must be a multiple of STRAND_LENGTH

CRGB leds[STRAND_LENGTH];

int particles[NUM_POS];
int values[STRAND_LENGTH];

void setup() {
  FastLED.addLeds<WS2811, PIN_LEFT, GRB>(leds, STRAND_LENGTH).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<WS2811, PIN_RIGHT, GRB>(leds, STRAND_LENGTH).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
  
  for (int i=NUM_POS-4*SUPER_FAC; i<NUM_POS; i++){
    particles[i] = 255;
  }
}

void loop(){
  for (int i=0; i<NUM_POS - 1; i++){
    particles[i] = particles[i+1];
  }
  
  particles[NUM_POS - 1] = 255 * (rand() % 2);

  for (int i=0; i<STRAND_LENGTH; i++){
    values[i] = 0;
  }

  for (int i=0; i<NUM_POS; i++){
    //Serial.println(particles[i]);
    values[i/SUPER_FAC] += particles[i];
  }

  for (int i=0; i< STRAND_LENGTH; i++){
    int val = values[i] / SUPER_FAC;
    leds[i] = CHSV(val/10, 255, val);
  }

  FastLED.show(); // display this frame
  //FastLED.delay(100/SUPER_FAC);
}
