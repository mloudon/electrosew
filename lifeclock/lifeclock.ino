/*
SparkFun Inventor's Kit
Example sketch 15

LIQUID CRYSTAL DISPLAY (LCD)

  A Liquid Crystal Display (LCD) is a sophisticated module 
  that can be used to display text or numeric data. The display
  included in your SIK features two lines of 16 characters, and
  a backlight so it can be used at night.

  If you've been using the Serial Monitor to output data,
  you'll find that a LCD provides many of the same benefits
  without needing to drag a large computer around.
  
  This sketch will show you how to connect an LCD to your Arduino
  and display any data you wish.

Hardware connections:

  The LCD has a 16-pin male header attached to it along the top
  edge. Pin 1 is the pin closest to the corner of the LCD.
  Pin 16 is the pin closest to the center of the LCD.
  
  Plug the LCD into your breadboard.
  
  As usual, you will want to connect the + and - power rails
  on the side of the breadboard to 5V and GND on your Arduino.
  
  Plug your 10K potentiometer into three unused rows on your
  breadboard. Connect one side of the potentiometer to 5V,
  and the other side to GND (it doesn't matter which). When you
  run this sketch, you'll use the potentiometer to adjust the
  contrast of the LCD so you can see the display.

  Now connect the LCD pins. Remember that pin 1 on the LCD
  is the one closest to the corner. Start there and work your
  way up.
  
  1 to GND
  2 to 5V
  3 to the center pin on the potentiometer
  4 to Arduino digital pin 12
  5 to GND
  6 to Arduino digital pin 11
  7 (no connection)
  8 (no connection)
  9 (no connection)
  10 (no connection)
  11 to Arduino digital pin 5
  12 to Arduino digital pin 4
  13 to Arduino digital pin 3
  14 to Arduino digital pin 2
  15 to 5V
  16 to GND

  Once everything is connected, load this sketch into the
  Arduino, and adjust the potentiometer until the display
  is clear.

Library

  The LCD has a chip built into it that controls all the
  individual dots that make up the display, and obeys commands
  sent to it by the the Arduino. The chip knows the dot patterns
  that make up all the text characters, saving you a lot of work.
  
  To communicate with this chip, we'll use the LiquidCrystal
  library, which is one of the standard libraries that comes
  with the Arduino. This library does most of the hard work
  of interfacing to the LCD; all you need to pick a location
  on the display and send your data!
  
Tips

  The LCD comes with a protective film over the display that
  you can peel off (but be careful of the display surface as
  it scratches easily).
  
  The LCD has a backlight that will light up when you turn on
  your Arduino. If the backlight doesn't turn on, check your 
  connections.

  As we said above, the potentiometer adjusts the contrast of
  the display. If you can't see anything when you run the sketch,
  turn the potentiometer's knob until the text is clear.
  
This sketch was written by SparkFun Electronics,
with lots of help from the Arduino community.
This code is completely free for any use.
Visit http://learn.sparkfun.com/products/2 for SIK information.
Visit http://www.arduino.cc to learn about the Arduino.

Version 1.0 2/2013 MDG
*/

// Load the LiquidCrystal library, which will give us
// commands to interface to the LCD:

#include <LiquidCrystal.h>

#include <EEPROM.h>

// Initialize the library with the pins we're using.
// (Note that you can use different pins if needed.)
// See http://arduino.cc/en/Reference/LiquidCrystal
// for more information:

LiquidCrystal lcd(12,11,5,4,3,2);

char x[17];
char y[17];

float LIFE_EXP_STRETCH = 1.06;
//long long int LIFE_EXP_0 = 58717743121;
long long int LIFE_EXP_0 = 143759309600;
float LIFE_EXP_11_MILLIS = 24.6;

void setup()
{
  // The LiquidCrystal library can be used with many different
  // LCD sizes. We're using one that's 2 lines of 16 characters,
  // so we'll inform the library of that:
  
  lcd.begin(16, 2);

  // Data sent to the display will stay there until it's
  // overwritten or power is removed. This can be a problem
  // when you upload a new sketch to the Arduino but old data
  // remains on the display. Let's clear the LCD using the
  // clear() command from the LiquidCrystal library:

  lcd.clear();

  // Now we'll display a message on the LCD!
  
  // Just as with the Arduino IDE, there's a cursor that
  // determines where the data you type will appear. By default,
  // this cursor is invisible, though you can make it visible
  // with other library commands if you wish.
  
  // When the display powers up, the invisible cursor starts 
  // on the top row and first column.
  
  lcd.print("The final countdown");

  // Adjusting the contrast (IMPORTANT!)
  
  // When you run the sketch for the first time, there's a
  // very good chance you won't see anything on the LCD display.
  // This is because the contrast likely won't be set correctly.
  // Don't worry, it's easy to set, and once you set it you won't
  // need to change it again.

  // Run the sketch, then turn the potentiometer until you can
  // clearly see the "hello, world!" text. If you still can't
  // see anything, check all of your connections, and ensure that
  // the sketch was successfully uploaded to the Arduino.
  
  if (EEPROM.read(0) == 1) {
    byte* p = (byte*)(void*)&LIFE_EXP_0;
    unsigned int i;
    int ee = 1;
    for (i = 0; i < sizeof(LIFE_EXP_0); i++)
          *p++ = EEPROM.read(ee++);
  }
}

void fmt(long long int t) {
  x[0] = '0' + (t / 100000000000) % 10;
  x[1] = ',';
  x[2] = '0' + (t / 10000000000) % 10;
  x[3] = '0' + (t / 1000000000) % 10;
  x[4] = '0' + (t / 100000000) % 10;
  x[5] = ',';
  x[6] = '0' + (t / 10000000) % 10;
  x[7] = '0' + (t / 1000000) % 10;
  x[8] = '0' + (t / 100000) % 10;
  x[9] = ',';
  x[10] = '0' + (t / 10000) % 10;
  x[11] = '0' + (t / 1000) % 10;
  x[12] = '0' + (t / 100) % 10;
  x[13] = '.';
  x[14] = '0' + (t / 10) % 10;
  x[15] = '0' + (t / 1) % 10;
  x[16]='\0'; 
}

void fmtYDHMS(long long int t) {
  t = t/100 - 1420070400;
  
  x[0] = '4';
  x[1] = '5';
  x[2] = 'y';
  x[3] = '0' + (t / 8640000) % 10;
  x[4] = '0' + (t / 864000) % 10;
  x[5] = '0' + (t / 86400) % 10;
  x[6] = 'd';
  x[7] = '0' + ((t / 3600) % 24) / 10;
  x[8] = '0' + ((t / 3600) % 24) % 10;
  x[9] = 'h';
  x[10] = '0' + (t / 600) % 6;
  x[11] = '0' + (t / 60) % 10;
  x[12] = 'm';
  x[13] = '0' + (t / 10) % 6;
  x[14] = '0' + t % 10;
  x[15] = 's';
  x[16]='\0';   
}

void fmtpct(long long int t) {
  int o = 4;
  for (int i = 0; i < 4; i++) {
    x[i] = ' '; 
  }
  x[0+o] = '0' + (t / 10000000000) % 10;
  x[1+o] = '0' + (t / 1000000000) % 10;
  x[2+o] = '.';
  x[3+o] = '0' + (t / 100000000) % 10;
  x[4+o] = '0' + (t / 10000000) % 10;
  x[5+o] = '0' + (t / 1000000) % 10;
  x[6+o] = '0' + (t / 100000) % 10;
  x[7+o] = '0' + (t / 10000) % 10;
  x[8+o] = '0' + (t / 1000) % 10;
  x[9+o] = '0' + (t / 100) % 10;
  x[10+o] = '0' + (t / 10) % 10;
  //x[11] = '0' + (t / 1) % 10;
  x[11+o]='%'; 
  x[12+o]='\0'; 
}

void marquee(String s) {
  int offset = millis() / 400;
  for (int i = 0; i < 16; i++) {
    y[i] = s.charAt((i + offset) % s.length());
  }
  y[16] = '\0'; 
}

long int last_cache = 0;
int cache_freq = 10000;

void loop()
{
  // You can move the invisible cursor to any location on the
  // LCD before sending data. Counting starts from 0, so the top
  // line is line 0 and the bottom line is line 1. Columns range
  // from 0 on the left side, to 15 on the right.

  // In additon to the "o, world!" printed above, let's
  // display a running count of the seconds since the Arduino
  // was last reset. Note that the data you send to the display
  // will stay there unless you erase it by overwriting it or
  // sending a lcd.clear() command.
  
  // Here we'll set the invisible cursor to the first column
  // (column 0) of the second line (line 1):

  lcd.setCursor(0,0);
  //marquee(String("For whom the bell tolls... it tolls for thee.  "));
  marquee(String("#YOLO   "));
  lcd.print(y);

  lcd.setCursor(0,1);

  // Now we'll print the number of seconds (millis() / 1000)
  // since the Arduino last reset:

  unsigned long int elapsed = millis() / (10 * LIFE_EXP_STRETCH);
  long long int remain = LIFE_EXP_0 - elapsed;
//  fmt(remain);
  fmtYDHMS(remain);
  lcd.print(x);
  
//  unsigned long int elapsed = millis() / (LIFE_EXP_11_MILLIS * LIFE_EXP_STRETCH);
//  long long int remain = LIFE_EXP_0 - elapsed;
//  fmtpct(remain);
//  lcd.print(x);
  
  if (millis() - last_cache > cache_freq) {

    EEPROM.write(0, 1);
    const byte* p = (const byte*)(const void*)&remain;
    unsigned int i;
    int ee = 1;
    for (i = 0; i < sizeof(remain); i++)
          EEPROM.write(ee++, *p++);
    
    last_cache = millis(); 
  }

}

