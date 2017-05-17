/*
  Firmware for AfrikaBurn Light Stick 
  ===================================
  This code includes functionality for 4 patterns:
  comets: Fire comets of random colours and at random intervals. Increase/decrease argument to adjust frequency of commets.
  chase: Two dots chase each other around.
  blocks: Three segments (red, green, blue) move around randomly and interact with each other as they overlap.
  rainbow: Vivid rainbow pattern. 

  Three user buttons (no pullup resistors necessary) are used to adjust the speed, brightness and to advance the pattern
  Attachments:
  Pin 3: "next" button
  Pin 4: "brightness" button
  Pin 5: "speed" button
  Pin 6: Neopixel data output.
 */

#include <Adafruit_NeoPixel.h>

//for LED strip:
#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN);

//pins
int button_next = 3;
int button_bright = 4;
int button_speed = 5;

//variables:
unsigned long colour[60];
int red[60], green[60], blue[60], BRIGHT = 125, SPEED_input = 400, SPEED, i, index = 0, volts, volts_old, volts_old_a;

byte redTarget, greenTarget, blueTarget, redPosition = 0, greenPosition = 19, bluePosition = 39;
byte redTime, greenTime, blueTime, redWait = 5, greenWait = 5, blueWait = 5;
byte redPixels[60], greenPixels[60], bluePixels[60];
byte space, notmoving, moving;
byte colourr, colourg, colourb;

void setup()
{
  pinMode(button_next, INPUT_PULLUP);
  pinMode(button_bright, INPUT_PULLUP);
  pinMode(button_speed, INPUT_PULLUP);

  strip.begin();
  strip.show();
  strip.setBrightness(BRIGHT);
 
  Serial.begin(9600);
  
  resetWheel();
}

void loop()
{  
  comets(40);
  chase();   
  blocks();  
  rainbow(); 
}

void comets(int density)
{
  index = 0;
  while(1)
  {
    if (!digitalRead(button_bright))
      brightness();
    
    for (i = 59 ; i >0 ; i--)
      colour[i] = colour[i-1];
    
    //new pixel, random colour
    index++;
    if (index > space)
    {
      i = random(60);
      colour[1] = strip.Color(red[i], green[i], blue[i]);
      i = random(60);
      colour[0] = strip.Color(red[i], green[i], blue[i]);
      index = 0;
      space = random(density);
    }
    else
    {
      colourr = colour[1] >> 16;
      colourg = (colour[1] >> 8) - (colourr << 8);
      colourb = colour[1] - (colourr << 16) - (colourg << 8);
      
      colour[0] = strip.Color(colourr >> 1, colourg >> 1, colourb >> 1);
    }  
  
    //set:
    for (i = 0; i < 60 ; i++)
      strip.setPixelColor(i,colour[i]);
      
    strip.show();
    
    if (!digitalRead(button_speed))
      speedup();
    SPEED = SPEED_input/10;
    delay(SPEED);
    
    if (!digitalRead(button_next))
    {
      delay(250);
      break;
    }
  }
}

void rainbow(void)
{
  index = 0;
  while(1)
  {
    if (!digitalRead(button_bright))
      brightness();
    
    for (i = 59 ; i >0 ; i--)
      colour[i] = colour[i-1];
    
    colour[0] = strip.Color(red[index], green[index], blue[index]);
    
    index++;
    if (index >=60)
      index = 0;
      
    for (i = 0; i < 60 ; i++)
      strip.setPixelColor(i, colour[i]);
    strip.show();
    
    if (!digitalRead(button_speed))
      speedup();
    SPEED = SPEED_input/10;
    delay(SPEED);
    if (!digitalRead(button_next))
    {
      delay(250);
      break;
    }
  }
}

void chase(void)
{
  index = 0;
  while(1)
  {
    if (!digitalRead(button_bright))
      brightness();
  
    if (notmoving == moving)
    {
      notmoving = random(60);
      redPixels[notmoving] = 255;
      for (i = 0 ; i < 60 ; i++)
        strip.setPixelColor(i, redPixels[i], 0, bluePixels[i]);    
      strip.show();
      delay(random(2000));
    }
    
    if (notmoving - moving > 0)
      moving += 1;
    else
      moving -= 1; 
      
    for (i = 0 ; i < 60 ; i++)
    {
      redPixels[i] = 0;
      bluePixels[i] = 0;
    }
    
    if (moving == notmoving)
      bluePixels[notmoving] = 255;
    else
    {
      redPixels[notmoving] = 255;
      bluePixels[moving] = 255;
    }
      
    for (i = 0 ; i < 60 ; i++)
      strip.setPixelColor(i, redPixels[i], 0, bluePixels[i]);    

    strip.show();
    
    if (!digitalRead(button_speed))
      speedup();
    SPEED = SPEED_input/10;
    delay(SPEED);
 
    if (!digitalRead(button_next))
    {
      delay(250);
      break;
    }
  }
}

void blocks(void)
{
  index = 0;
  while(1)
  {
    if (!digitalRead(button_bright))
      brightness();

  
  //if at target:
  if (redTarget == redPosition)
  {
    redTime++;
    if (redTime == redWait)
    {
      redTarget = random(50); //set new target
      redTime = 0;
    }
  }
  else
  {
    redTime = 0;
    redWait = random(1,40);
  }
  
  if (greenTarget == greenPosition)
  {
    greenTime++;
    if (greenTime == greenWait)
    {
      greenTarget = random(50); //set new target
      greenTime = 0;
    }
  }
  else
  {
    greenTime = 0;
    greenWait = random(1, 40);
  }
  
  if (blueTarget == bluePosition)
  {
    blueTime++;
    if (blueTime == blueWait)
    {
      blueTarget = random(50); //set new target
      blueTime = 0;
    }
  }
  else
  {
    blueTime = 0;
    blueWait = random(1, 40);
  }
  
  //Determine direction to move, and move:
  if (redTarget - redPosition > 0 && redTime == 0)
    redPosition++;
  else if (redPosition - redTarget > 0 && redTime == 0)
    redPosition--; 

  if (greenTarget - greenPosition > 0 && greenTime == 0)
    greenPosition++;
  else if (greenPosition - greenTarget > 0 && greenTime == 0)
    greenPosition--; 
      
  if (blueTarget - bluePosition > 0 && blueTime == 0)
    bluePosition++;
  else if (bluePosition - blueTarget > 0 && blueTime == 0)
    bluePosition--; 
  
  //clear old colours:    
  for (i = 0 ; i < 60 ; i++)
  {
    redPixels[i] = 0;  greenPixels[i] = 0;  bluePixels[i] = 0;  
  }
  
  //populate with new colours
  for (i = 0; i < 10 ; i++)
  {
    redPixels[redPosition + i] = 255; greenPixels[greenPosition + i] = 255; bluePixels[bluePosition + i] = 255;
  }

  //Send to device:
  for (i = 0 ; i < 60 ; i++)
    strip.setPixelColor(i, redPixels[i], greenPixels[i], bluePixels[i]);    
  strip.show();
  
    if (!digitalRead(button_speed))
      speedup();
    SPEED = SPEED_input/10;
  delay(SPEED);
    if (!digitalRead(button_next))
    {
      delay(250);
      break;
    }
  }
}

void resetWheel(void)
{
  for (i = 0; i < 20 ; i++)
    red[i]= 255;
  for (i = 20; i < 30; i++)
    red[i]= 255 - 25.5*(i-20);
  for (i = 30; i < 50; i++)
    red[i]= 0;
  for (i = 50; i < 60; i++)
    red[i]= 25.5*(i - 50);
    
  for (i = 0; i < 60; i++)
  {
    if (i >= 20)
      green[i] = red[i-20];
    else
      green[i] = red[i+40];
  
    if (i >= 40)
      blue[i] = red[i-40];
    else
      blue[i] = red[i+20];
  }
}

void brightness(void)
{
  BRIGHT -= 115;
  if (BRIGHT <= 0)
    BRIGHT = 255;
 
  strip.setBrightness(BRIGHT);

  delay(200);
}

void speedup(void)
{
  SPEED_input -= 200;
  if (SPEED_input < 200)
    SPEED_input = 1000;
      
  delay(200);
}
