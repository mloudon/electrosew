
/*  Pulse Sensor Amped 1.4    by Joel Murphy and Yury Gitman   http://www.pulsesensor.com

----------------------  Notes ----------------------  ----------------------
This code:
1) Blinks an LED to User's Live Heartbeat   PIN 13
2) Fades an LED to User's Live HeartBeat
3) Determines BPM
4) Prints All of the Above to Serial

Read Me:
https://github.com/WorldFamousElectronics/PulseSensor_Amped_Arduino/blob/master/README.md
 ----------------------       ----------------------  ----------------------
*/

#include <Adafruit_NeoPixel.h>

#define PIN 6

Adafruit_NeoPixel strip = Adafruit_NeoPixel(7, PIN, NEO_GRB + NEO_KHZ800);

//  Variables
int pulsePin = 10;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 7;                // pin to blink led at each beat
int fadePin = 9;                  // pin to do fancy classy fading blink at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin

float current_brightness = 0.7;
float min_brightness = 0.05;
float max_brightness = 0.7;
int wheel_pos = 160;
int num_pixels = 7;
int wave_pos = num_pixels;
float wave_counter = 0;
int wave_move_each = 200; //how many milliseconds between wave steps


// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded!
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat".
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

// Regards Serial OutPut  -- Set This Up to your needs
static boolean serialVisual = true;   // Set to 'false' by Default.  Re-set to 'true' to see Arduino Serial Monitor ASCII Visual Pulse


void setup() {

  strip.begin();
  strip.show();

  pinMode(blinkPin, OUTPUT);        // pin that will blink to your heartbeat!
  Serial.begin(115200);             // we agree to talk fast!
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS
  // UN-COMMENT THE NEXT LINE IF YOU ARE POWERING The Pulse Sensor AT LOW VOLTAGE,
  // AND APPLY THAT VOLTAGE TO THE A-REF PIN
  //   analogReference(EXTERNAL);
}


//  Where the Magic Happens
void loop() {

  serialOutput();


  if (QS == true) {    //  A Heartbeat Was Found
    // BPM and IBI have been Determined
    // Quantified Self "QS" true when arduino finds a heartbeat
    wheel_pos += 5;
    if (wheel_pos > 255) {
      wheel_pos -= 255;
    }

    wave_pos = 0;

    // current_brightness = 0.7;
    // strip.show();
    // digitalWrite(blinkPin, HIGH);    // Blink LED, we got a beat.

    serialOutputWhenBeatHappens();   // A Beat Happened, Output that to serial.
    QS = false;                      // reset the Quantified Self flag for next time
  }
  else {
    if(wave_pos <= num_pixels && (wave_counter >= wave_move_each)) {
      wave_counter = 0;
      
      for (int i = 0; i <= num_pixels; i++) {
        strip.setPixelColor(i,  Wheel(wheel_pos, min_brightness));
      }

      if(wave_pos < num_pixels) {
        strip.setPixelColor(wave_pos,  Wheel(wheel_pos, max_brightness));
      } else {
        strip.setPixelColor(wave_pos, Wheel(wheel_pos, min_brightness));
      }
      strip.show();

      wave_pos++;
    }
    // fadeBrightness(0.01);
    // digitalWrite(blinkPin, LOW);           // There is not beat, turn off pin 13 LED
  }
  delay(20);                             //  take a break
  wave_counter += 20;
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










