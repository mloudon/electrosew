#include <SPI.h>
#include <RH_RF95.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_FeatherOLED.h>

Adafruit_FeatherOLED oled = Adafruit_FeatherOLED();

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define Apin 9
#define Bpin 6
#define Cpin 5

#define LED 13

/* for feather32u4 */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);


// timing
#define TRANSMIT_INTERVAL 2000      // interval between sending updates
#define DISPLAY_INTERVAL 300      // interval between updating display
unsigned long lastSend, lastDisplay;

// tinyGPS
#include <TinyGPS.h>

TinyGPS gps;

void setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  pinMode(LED, OUTPUT);

  pinMode(Apin, INPUT_PULLUP);
  pinMode(Bpin, INPUT_PULLUP);
  pinMode(Cpin, INPUT_PULLUP);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  say("hello", "there", "feather", "");
  delay(3000);
  display.clearDisplay();

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    say("LoRa radio init failed", "", "", "");
    while (1);
  }

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    say("setFrequency failed", "", "", "");
    while (1);
  }

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);

  Serial.begin(9600);
  Serial1.begin(9600);
}

#define MAGIC_NUMBER_LEN 3
uint8_t MAGIC_NUMBER[MAGIC_NUMBER_LEN] = {0x11, 0x29, 0x83};

//String timeStr = "";
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
int lastRSSI;
long last_recv = 0;

float myLat;
float myLon;
float theirLat;
float theirLon;

void processRecv() {
  for (int i = 0; i < MAGIC_NUMBER_LEN; i++) {
    if (MAGIC_NUMBER[i] != buf[i]) {
      return;
    }
  }
  uint8_t* data = buf + MAGIC_NUMBER_LEN;
  theirLat = *((float*)data);
  theirLon = *((float*)data + 1);
  last_recv = millis();
}

void transmitData() {
  uint8_t len = 2*sizeof(float) + MAGIC_NUMBER_LEN + 1;
  uint8_t radiopacket[len];
  for (int i = 0; i < MAGIC_NUMBER_LEN; i++) {
    radiopacket[i] = MAGIC_NUMBER[i];
  }
  *((float*)(radiopacket + MAGIC_NUMBER_LEN)) = myLat;
  *((float*)(radiopacket + MAGIC_NUMBER_LEN) + 1) = myLon;
  radiopacket[len-1] = '\0';

  rf95.send((uint8_t *)radiopacket, len);
  rf95.waitPacketSent();
}

void loop() {
  if (Serial1.available()) {
    char c = Serial1.read();
    Serial.write(c);
    if (gps.encode(c)) { // Did a new valid sentence come in?
      attemptUpdateFix();
    }
  }

  if (rf95.available()) {
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len)) {
      lastRSSI = rf95.lastRssi();
      digitalWrite(LED, HIGH);
      digitalWrite(LED, LOW);
      processRecv();
    }
  }

  long sinceLastTransmit = millis() - lastSend;
  if (sinceLastTransmit < 0 || sinceLastTransmit > TRANSMIT_INTERVAL) {
    lastSend = millis();
    transmitData();
  }

  long sinceLastDisplayUpdate = millis() - lastDisplay;
  if (sinceLastDisplayUpdate < 0 || sinceLastDisplayUpdate > DISPLAY_INTERVAL) {
    lastDisplay = millis();
    updateDisplay();
  }
  
}

void attemptUpdateFix() {
  //setFixTime();
  setFix();
}

String fixAge() {
  long elapsed = (millis() - last_recv) / 1000;
  int n;
  char unit;
  if (elapsed < 2) {
    return "now";
  } else if (elapsed < 60) {
    n = elapsed;
    unit = 's';
  } else if (elapsed < 3600) {
    n = elapsed / 60;
    unit = 'm';
  } else {
    n = elapsed / 3600;
    unit = 'h';
  }
  return String(n) + String(unit) + " ago";
}

void updateDisplay() {
  say(fmtPlayaStr(theirLat, theirLon), fixAge() + "  (" + String(lastRSSI) + "db)", "", fmtPlayaStr(myLat, myLon));
}

void say(String s, String t, String u, String v) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(s);
  display.println(t);
  display.println(u);
  display.println(v);
  display.display();
}

void setFix () {
  float flat, flon;
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);

  if (flat == TinyGPS::GPS_INVALID_F_ANGLE) {
    flat = 0.0;
  }
  if (flon == TinyGPS::GPS_INVALID_F_ANGLE) {
    flon = 0.0;
  }
  Serial.println(String(flat, 6) + " " + String(flon, 6));
  myLat = flat;
  myLon = flon;
}

String fmtPlayaStr(float lat, float lon) {
  if (lat == 0.0 && lon == 0.0) {
    return "404 cosmos not found";
  } else {
    return playaStr(lat, lon);
  }
}

//void setFixTime() {
//  int year;
//  byte month, day, hour, minute, second, hundredths;
//  unsigned long age;
//  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
//  timeStr =  String(hour) + ":" + String(minute) + ":" +  String(second) + "/" + String(age) + "ms";
//}

///// PLAYA COORDINATES CODE /////

#define DEG_PER_RAD (180. / 3.1415926535)
#define CLOCK_MINUTES (12 * 60)
#define METERS_PER_DEGREE (40030230. / 360.)
// Direction of north in clock units
#define NORTH 10.5  // hours
#define NUM_RINGS 13  // Esplanade through L
#define ESPLANADE_RADIUS (2500 * .3048)  // m
#define FIRST_BLOCK_DEPTH (440 * .3048)  // m
#define BLOCK_DEPTH (240 * .3048)  // m
// How far in from Esplanade to show distance relative to Esplanade rather than the man
#define ESPLANADE_INNER_BUFFER (250 * .3048)  // m
// Radial size on either side of 12 w/ no city streets
#define RADIAL_GAP 2.  // hours
// How far radially from edge of city to show distance relative to city streets
#define RADIAL_BUFFER .25  // hours

// production
//#define MAN_LAT 40.7864
//#define MAN_LON -119.2065
//#define SCALE 1.

// testing
#define MAN_LAT 40.779625
#define MAN_LON -73.965394
#define SCALE 6.

// 0=man, 1=espl, 2=A, 3=B, ...
float ringRadius(int n) {
  if (n == 0) {
    return 0;
  } else if (n == 1) {
    return ESPLANADE_RADIUS;
  } else if (n == 2) {
    return ESPLANADE_RADIUS + FIRST_BLOCK_DEPTH;
  } else {
    return ESPLANADE_RADIUS + FIRST_BLOCK_DEPTH + (n - 2) * BLOCK_DEPTH;
  }
}

// Distance inward from ring 'n' to show distance relative to n vs. n-1
float ringInnerBuffer(int n) {
  if (n == 0) {
    return 0;
  } else if (n == 1) {
    return ESPLANADE_INNER_BUFFER;
  } else if (n == 2) {
    return .5 * FIRST_BLOCK_DEPTH;
  } else {
    return .5 * BLOCK_DEPTH;
  }
}

int getReferenceRing(float dist) {
  for (int n = NUM_RINGS; n > 0; n--) {
    if (ringRadius(n) - ringInnerBuffer(n) <= dist) {
      return n;
    }
  }
  return 0;
}

String getRefDisp(int n) {
  if (n == 0) {
    return ")(";
  } else if (n == 1) {
    return "Espl";
  } else {
    return String(char(int('A') + n - 2));
  }
}

String playaStr(float lat, float lon) {
  // Precision issues-- float for GPS only gives about ~5m resolution. Arduino doubles are
  // fake -- same precision as floats.
  // If we could get lat/lon in fixed-point (say degrees * 1e5), we could preserve precision.
  float m_dx = (lon - MAN_LON) * cos(MAN_LAT / DEG_PER_RAD) * METERS_PER_DEGREE;
  float m_dy = (lat - MAN_LAT) * METERS_PER_DEGREE;

  float dist = SCALE * sqrt(m_dx * m_dx + m_dy * m_dy);
  float bearing = DEG_PER_RAD * atan2(m_dx, m_dy);

  float clock_hours = (bearing / 360. * 12. + NORTH);
  int clock_minutes = (int)(clock_hours * 60 + .5);
  // Force into the range [0, CLOCK_MINUTES)
  clock_minutes = ((clock_minutes % CLOCK_MINUTES) + CLOCK_MINUTES) % CLOCK_MINUTES;

  int hour = clock_minutes / 60;
  int minute = clock_minutes % 60;
  String clock_disp = String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute);

  int refRing;
  if (6 - abs(clock_hours - 6) < RADIAL_GAP - RADIAL_BUFFER) {
    refRing = 0;
  } else {
    refRing = getReferenceRing(dist);
  }

  float refDelta = dist - ringRadius(refRing);
  unsigned long refDeltaRound = long(refDelta);

  return clock_disp + " & " + getRefDisp(refRing) + (refDelta >= 0 ? "+" : "-") + String(refDeltaRound) + "m";
}





