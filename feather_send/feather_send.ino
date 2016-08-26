// Feather9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_RX

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

/* for feather32u4 */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);


// timing
int  updateInterval = 2000;      // interval between updates
unsigned long lastUpdate; // last update of position

void setup()
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  pinMode(Apin, INPUT_PULLUP);
  pinMode(Bpin, INPUT_PULLUP);
  pinMode(Cpin, INPUT_PULLUP);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.clearDisplay();

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    say("LoRa radio init failed", "", "");
    while (1);
  }

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    say("setFrequency failed", "", "");
    while (1);
  }

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);

  Serial.println("GPS echo test");
  Serial.begin(9600);
  Serial1.begin(9600);

  lastUpdate = millis();
}

int16_t packetnum = 0;  // packet counter, we increment per xmission

#define BUFSIZE 100
int buf_size = 0;
char BUF[BUFSIZE];

//$GPGGA,033254.000,(lat)4044.6560,(ns)N,(lon)07400.1044,(ew)W,2,09,(hdop)0.95,(alt)20.8,M,-34.2,M,0000,0000*6A


void processBuffer() {
  char subbuf[20];

  int field = 0;
  int j = 0;
  for (int i = 0; i < BUFSIZE; i++) {
    if (BUF[i] == ',') {
      subbuf[j] = '\0';

      switch(field) {
        case 0:
          Serial.println("hello");
          Serial.println((int)subbuf[0]);
          Serial.println((int)subbuf[1]);
          Serial.println((int)subbuf[2]);
          Serial.println((int)subbuf[3]);
          if (strcmp(subbuf, "$GPGGA") != 0) {
          Serial.println("nope");
            
            return;     
          }
                    Serial.println("match");

          break;
      }
      
      field++;
      j = 0;
    } else if (j == 20) {
      // overrun
      return;
    } else {
      subbuf[j++] = BUF[i];
    }
  }
  
  buf_size = 0;
}

void loop()
{
//  if (Serial.available()) {
//    char c = Serial.read();
//    Serial1.write(c);
//  }
  if (Serial1.available()) {
    char c = Serial1.read();
    //Serial.write(c);
    if (c == 0x0d || c == 0x0a || buf_size == BUFSIZE - 1) {
      BUF[buf_size++] = '\0';
      processBuffer();
    } else {
      BUF[buf_size++] = c;
    }
  }

  if (lastUpdate > millis()) lastUpdate = millis();

  if ((millis() - lastUpdate) > updateInterval) // time to update
  {
    lastUpdate = millis();
    char radiopacket[5] = "#    ";
    itoa(packetnum++, radiopacket + 2, 10);
    // say("Sending " + String(radiopacket), fix_time(), fix());
    say("Sending " + String(radiopacket), "", "");
    radiopacket[19] = 0;

    rf95.send((uint8_t *)radiopacket, 5);
    rf95.waitPacketSent();
  }

  playaCoords(41.63, -72.59);
}

void say(String s, String t, String u) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(s);
  display.println(t);
  display.println(u);
  display.display();
}

//String fix () {
//  String s =  "";
//  if (GPS.fix) {
//    s =  String(GPS.latitude, 4) + (GPS.lat) + " " + String(GPS.longitude, 4) + (GPS.lon);
//  }
//  return s;
//}
//
//String fix_time() {
//  String s =  String(GPS.hour) + ":" + String(GPS.minute) + ":" +  String(GPS.seconds);
//  return s;
//}

///// PLAYA COORDINATES CODE /////

#define DEG_PER_RAD (180. / 3.1415926535)
#define CLOCK_MINUTES (12 * 60)
#define MAN_LAT 40.7864
#define MAN_LON -119.2065
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
#define RADIAL_GAP 2  // hours
// How far radially from edge of city to show distance relative to city streets
#define RADIAL_BUFFER .25  // hours

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

/*
  def get_ref_disp(n):
    if n == 0:
        return ')\'('
    elif n == 1:
        return 'Espl'
    else:
        return chr(ord('A') + n - 2)
*/

void playaCoords(float lat, float lon) {
  float m_dx = (lon - MAN_LON) * cos(MAN_LAT / DEG_PER_RAD) * METERS_PER_DEGREE;
  float m_dy = (lat - MAN_LAT) * METERS_PER_DEGREE;

  float dist = sqrt(m_dx * m_dx + m_dy * m_dy);
  float bearing = DEG_PER_RAD * atan2(m_dx, m_dy);

  float clock_hours = (bearing / 360. * 12. + NORTH);
  int clock_minutes = (int)(clock_hours * 60 + .5);
  clock_minutes = ((clock_minutes % CLOCK_MINUTES) + CLOCK_MINUTES) % CLOCK_MINUTES;

  int hour = clock_minutes / 60;
  int minute = clock_minutes % 60;
  //clock_disp = '%d:%02d' % (minutes // 60, minutes % 60)

  int refRing;
  if (6 - abs(clock_hours - 6) < RADIAL_GAP - RADIAL_BUFFER) {
    refRing = 0;
  } else {
    refRing = getReferenceRing(dist);
  }
  float refDelta = dist - ringRadius(refRing);
  //ref_disp = get_ref_disp(ref_ring)

  //display.println(hour);
  //display.println(minute);
  //display.println(refRing);
  //display.println(refDelta);
  //return '%s %s%s%dm' % (clock_disp, ref_disp, '+' if ref_diff >= 0 else '-', abs(int(round(ref_diff))))
}





