#include <Adafruit_NeoPixel.h>

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
 *  
 *  
 */

#if defined (__AVR_ATtiny85__)
  #define PIN 0
#else
  #define PIN 6
#endif

#define STRAND_LENGTH 17
#define BRIGHTNESS 255

/**
 * Whether the pattern is mirrored, or reversed. This is useful for scarfs where 
 * the LEDs are all daisy chained. An alternative is to have the center pixel
 * being the first one, and split the d-out line down either sides
 */
//#define MIRRORED
//#define REVERSED

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

#define RAINBOW
//#define SEAPUNK
//#define INDIGO
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

#if defined (HElART)
  #define HUE_START .833
  #define HUE_END 1.
  #define SATURATION 1.
#endif

Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRAND_LENGTH, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  strip.setBrightness(BRIGHTNESS);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop(){
  unsigned long t = millis();
  
  //byte color = getClock(t, 2);
  
  byte pulse = 255;// * PerlinNoise2(t,.25);
  pulse = 255 * PerlinNoise_1D(t);

  //strip.setPixelColor(pulse, strip.Color(255, 0, 0));
  //strip.show();
  //return;
  for (byte pix = 0; pix < ARM_LENGTH; pix++){
    // location of the pixel on a 0-RENDER_RANGE scale.
    byte dist = pix * 255 / ARM_LENGTH;

    // messy, but some sort of least-of-3 distances, allowing wraping.
    byte delta = abs(dist - pulse);
    
    // linear ramp up of brightness, for those within 1/8th of the reference point
    
    float value = max(255 - 6 * delta, 15) / 255.;

    byte loc = pix;
    #if defined (REVERSED)
      loc = ARM_LENGTH - 1 - pix;
    #endif

    uint32_t c = strip.Color(255 * value, 255 * value , 255 * value);
    //uint32_t c = hsvToRgb(hue, SATURATION, value);
    strip.setPixelColor(loc, c);
    #if defined (MIRRORED)
      strip.setPixelColor(STRAND_LENGTH - 1 - loc, c);
    #endif
  }

  //blinkPerFrame();
  strip.show();

  // delay 20ms to give max 50fps. Could do something fancier here to try to 
  // hit exactly 60fps (or whatever) if possible, but takinng another millis()
  // reading, but not sure if there would be a point to that. 
  delay(20); 
}

//Blinks the first pixel on and off. Used to check for framerate smoothness.

bool red;
void blinkPerFrame()
{
    if (red)
      strip.setPixelColor(0, 127, 0, 0);
    else
      strip.setPixelColor(0, 0, 0, 0);
    red = !red;
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

/** 
 *  HSV/HSL to RGB code From https://github.com/ratkins/RGBConverter
 */

/**
 * Converts an HSV color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes h, s, and v are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 255].
 *
 * @param   Number  h       The hue
 * @param   Number  s       The saturation
 * @param   Number  v       The value
 */
uint32_t hsvToRgb(float h, float s, float v) {
    float r, g, b;

    byte i = byte(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch(i % 6){
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    return strip.Color(r * 255, g * 255, b * 255);
}

/**
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 255].
 *
 * @param   Number  h       The hue
 * @param   Number  s       The saturation
 * @param   Number  l       The lightness
 * @return  Array           The RGB representation
 */
uint32_t hslToRgb(float h, float s, float l) {
    float r, g, b;

    if (s == 0) {
        r = g = b = l; // achromatic
    } else {
        float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        float p = 2 * l - q;
        r = hue2rgb(p, q, h + 1/3.);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1/3.);
    }

    return strip.Color(r * 255, g * 255, b * 255);
}

float hue2rgb(float p, float q, float t) {
    if(t < 0) t += 1;
    if(t > 1) t -= 1;
    if(t < 1/6) return p + (q - p) * 6 * t;
    if(t < 1/2) return q;
    if(t < 2/3) return p + (q - p) * (2/3 - t) * 6;
    return p;
}

float Noise1(int x){
    return sin(x);
    x = pow((x<<13), x);
    return ( 1.0 - ( (x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}


float SmoothedNoise1(float x){
  return Noise1(x);
    return Noise1(x)/2  +  Noise1(x-1)/4  +  Noise1(x+1)/4;
}

float InterpolatedNoise_1(float x){

      int integer_X    = int(x);
      float fractional_X = x - integer_X;

      float v1 = SmoothedNoise1(integer_X);
      float v2 = SmoothedNoise1(integer_X + 1);

      return Interpolate(v1 , v2 , fractional_X);
}

float Interpolate(float a, float b, float frac){
  return a + frac * (b-a);
}


float PerlinNoise_1D(float x){

      float total = 0;
      float p = .25;
      int n = 3 - 1;

      for (int i = 0; i < 3; i++)
{
          float frequency = pow(2, i);
          float amplitude = pow(p, i);

          total = total + InterpolatedNoise_1(x * frequency) * amplitude;

}
      return total;

}
