
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

int pulsePin = 10;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 7;                // pin to blink led at each beat
int fadePin = 9;                  // pin to do fancy classy fading blink at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin


// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded!
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat".
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

// Regards Serial OutPut  -- Set This Up to your needs
static boolean serialVisual = false;   // Set to 'false' by Default.  Re-set to 'true' to see Arduino Serial Monitor ASCII Visual Pulse

int num_pixels = 7;

float brightness_max = 0.95;
float brightness_min = 0.05;

float brightness_variance = brightness_max - brightness_min;
float brightness_gap_per_pixel = brightness_variance / num_pixels;

int wheel_pos = 150;

int refresh_rate_ms = 20;
int impulse_duration_ms = 2000;

int impulse_duration_per_pixel_ms = impulse_duration_ms / (num_pixels * 2);

int time_elapsed_ms = 0;
int time_since_heartbeat_ms = 0;

void setup() {

  Serial.begin(9600);

  strip.begin();
  strip.show();

  pinMode(blinkPin, OUTPUT);
  interruptSetup();
}

void heartbeat() {
  time_since_heartbeat_ms = 0;
}

void redraw() {
  int this_color;
  int this_pixel_offset_ms;
  
  float this_brightness;
  int pixel_phase;

  for(int i = 0; i < num_pixels; i++) {
    this_pixel_offset_ms = i * impulse_duration_per_pixel_ms;
 
    pixel_phase = ((time_since_heartbeat_ms - this_pixel_offset_ms) / impulse_duration_per_pixel_ms);

    this_brightness = brightness_max - (pixel_phase * brightness_gap_per_pixel);
    if(this_brightness < 0) {
      this_brightness = 0;
    }
    if(pixel_phase < 0) {
      this_brightness = 0;
    }

    float time_frac = (float)time_elapsed_ms / 5000;
    this_color = 255 * time_frac;

    strip.setPixelColor(i, Wheel(this_color, this_brightness));
  }

  strip.show();  
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





//  Where the Magic Happens
void loop() {

//  serialOutput();

  if (QS == true) {
    heartbeat();
    QS = false;
  }

  redraw();

  delay(refresh_rate_ms);
  time_elapsed_ms += refresh_rate_ms;
  time_since_heartbeat_ms += refresh_rate_ms;
}


