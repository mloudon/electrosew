/**
 * Stacking animation for tube harness. Doesn't use any sensor input
 * 
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

#define DELAY 20

CRGB leds[STRAND_LENGTH]; 

int stack_top = STRAND_LENGTH;
int move_direction = 1;
int pos = 0;

void setup() {
  FastLED.addLeds<WS2811, PIN_LEFT, GRB>(leds, STRAND_LENGTH).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<WS2811, PIN_RIGHT, GRB>(leds, STRAND_LENGTH).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );

  Serial.begin(9600);
}

void loop(){
  if (stack_top == 0){
    stack_top = STRAND_LENGTH;
    pos = 0;  
  } else if (pos == stack_top - 1){
    pos = 0;
    stack_top -= 1;
  } else {
    pos += 1;
  }

  for (int i = 0; i < stack_top; i++){
    leds[i] = CHSV(150, 255, 0);
  }

  for (int i = stack_top; i < STRAND_LENGTH; i++){
    leds[i] = CHSV(150, 255, 150);
  }

  leds[pos] = CHSV(100, 255, 150);

  FastLED.show(); // display this frame
  FastLED.delay(DELAY);
}
