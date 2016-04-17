#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>


/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345);

#define NP_PIN 8
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, NP_PIN, NEO_GRB + NEO_KHZ800);

float max_x, min_x, max_y, min_y, max_z, min_z;
float nx, ny, nz;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting Serial stuff");
  Serial.flush();

  // Magnets and stuff
  mag.enableAutoRange(true);

  max_x = max_y = max_z = -1000;
  min_x = min_y = min_z = 1000;

  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }
  displaySensorDetails();
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  /* Get a new sensor event */ 
  updateMagnets();

  strip.setPixelColor(0, hsvToRgb(nx, 1., 1.));
  strip.show();
  
  delay(10);
  //rainbowCycle(20);
}

void updateMagnets(){
  sensors_event_t event; 
  mag.getEvent(&event);

  float x = event.magnetic.x;
  float y = event.magnetic.y;
  float z = event.magnetic.z;
 


  max_x = max(x, max_x);
  min_x = min(x, min_x);
  nx = (x - min_x)/(max_x - min_x);

  max_y = max(y, max_y);
  min_y = min(y, min_y);
  ny = (y - min_y)/(max_y - min_y);

  int time = millis();
  if (time % 1000 > 900 && time % 1000 < 920)
  {
    Serial.println(millis());
    Serial.print("X: "); Serial.print(x); Serial.print("  ");
    Serial.print("Y: "); Serial.print(y); Serial.print("  ");
    Serial.print("Z: "); Serial.print(z); Serial.print("  ");Serial.println("uT");
  
    Serial.print("X: "); Serial.print(nx); Serial.print("  ");
    Serial.print("Y: "); Serial.print(ny); Serial.print("  ");
    Serial.print("Z: "); Serial.print(nz); Serial.print("  ");Serial.println("uT");
  }

}

void displaySensorDetails(void)
{
  sensor_t sensor;
  mag.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
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

    byte i = int(h * 6);
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
