/**
 * Coloured pulse for tube harness. Adjusted by accelerometer sensor
 * 
 * Pulse velocity gets adjusted by the magnitude of the accel sample
 * Velocity also adjusts pulse hue
 * Current set up is for two 30 LED strands driven by Arduino Nano
 */

#include <FastLED.h>

#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN_RIGHT 6
#define PIN_LEFT 7

#define STRAND_LENGTH 30
#define BRIGHTNESS 127

#define PULSE_LENGTH 0.2
#define SPECTRUM_FRAC 0.1 // fraction of the spectrum used at one time

#define MAX_VEL 4
#define MIN_VEL 0.07
#define ACCEL_DAMP 1 //factor applied to the acceleration
#define REVERSE_ACCEL 2 //slows the pulse down

CRGB leds[STRAND_LENGTH];
float values[STRAND_LENGTH];

float pix_size = 1.0/(float)STRAND_LENGTH;

float pulse_start;
float pulse_end;
float vel;

Adafruit_MMA8451 mma = Adafruit_MMA8451();

int old_t;

void reset_pulse(){
  pulse_start = -PULSE_LENGTH;
  pulse_end = 0;
}

void setup() {
  FastLED.addLeds<WS2811, PIN_LEFT, GRB>(leds, STRAND_LENGTH).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<WS2811, PIN_RIGHT, GRB>(leds, STRAND_LENGTH).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );

  Serial.begin(9600);

  if (! mma.begin()) {
    Serial.println("Couldnt start");
    while (1);
  }
  
  mma.setRange(MMA8451_RANGE_2_G);

  reset_pulse();
  
  old_t = millis();
  vel = 0.1;
}

void loop(){

//  Serial.print("start: "); Serial.print(pulse_start);
//  Serial.print(" end: "); Serial.println(pulse_end);
//  mma.read();
//  Serial.print("X:\t"); Serial.print(mma.x); 
//  Serial.print("\tY:\t"); Serial.print(mma.y); 
//  Serial.print("\tZ:\t"); Serial.print(mma.z); 
//  Serial.println();

  /* Get a new sensor event */ 
  sensors_event_t event; 
  mma.getEvent(&event);

  float accel = sqrt(pow(event.acceleration.x, 2.0) + pow(event.acceleration.y, 2.0) + pow(event.acceleration.z, 2.0));
  //Serial.println(accel);

  int new_t = millis();
  float delta_t = (new_t - old_t)/1000.0;
  old_t = new_t;
  vel = min(MAX_VEL, max(MIN_VEL, vel + (abs(accel - 9.93) * ACCEL_DAMP - REVERSE_ACCEL) * delta_t));

  for (int i=0; i<STRAND_LENGTH; i++){
    float pix_start = max(i*pix_size, pulse_start);
    float pix_end = min((i+1)*pix_size, pulse_end);
    values[i] = max(0, (pix_end - pix_start)/pix_size);
  }

  pulse_start += vel * delta_t;
  pulse_end += vel * delta_t;

  if (pulse_start > 1) reset_pulse();

  float spec_offset = (1.0 - (vel - MIN_VEL) / (MAX_VEL - MIN_VEL)) * (1.0 - SPECTRUM_FRAC);
  
  for (int i=0; i< STRAND_LENGTH; i++){
    leds[i] = CHSV(255 * (spec_offset + values[i] * SPECTRUM_FRAC), 255, values[i] * 255);
  }

  FastLED.show(); // display this frame
  FastLED.delay(20);
}
