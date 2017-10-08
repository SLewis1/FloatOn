#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1325.h>
#include "RTClib.h"
#include <Wire.h>
RTC_PCF8523 rtc;
#include <Fonts/FreeSerif24pt7b.h> 
#define font    FreeSerif24pt7b

#define OLED_CS 6 //was 10
#define OLED_RESET 9
#define OLED_DC 5 //was 8

Adafruit_SSD1325 display(OLED_DC, OLED_RESET, OLED_CS);

int textSize = 1;
int sec = 0;
int across = 0;
int vspacing;
int myHour;
//#define seconds  //show seconds
//#define seriald

void setup() {
#ifdef seriald
  Serial.begin(9600);
  delay(1000);
  Serial.println("OLED test");
#endif
  
  display.setTextSize(textSize);
  display.setTextColor(1);
  display.setTextWrap(false);
  display.setFont(&font);
  display.begin(); 
  display.clearDisplay();
  display.display();
  
}

void loop() {
  
  DateTime now = rtc.now();
  #ifdef seriald
    Serial.print(now.hour(),DEC); Serial.print(":"); Serial.print(now.minute(),DEC); Serial.print(":"); Serial.println(now.second(),DEC);
  #endif
  
//hours
 
  //convert to 12-hour time from 24-hour
  if (now.hour() > 12)   
  {
    myHour = now.hour()-12;
  }
  else
  {
    myHour = now.hour();
  }
  
  //adjust horizontal spacing so display is centered
  if (myHour < 10)
  {
    display.setCursor(22,36);
  }
  else
  {
    display.setCursor(10,36);
  }
  
  display.print(myHour);
  display.print(":"); 

//minutes
  //add leading 0 if needed
  if (now.minute() < 10) {
    display.print("0");
  }
  
  display.print(now.minute()); 

//seconds in bottom right corner
#ifdef seconds
  display.setTextSize(1);
  display.setCursor(115,55);
  if (now.second() < 10) {
    display.print("0");
  }
  display.println(now.second());
  display.setTextSize(textSize);
#endif //seconds

//line going across
  across = floor(2.16*now.second());
  vspacing = 50;
  if (now.minute() % 2 == 0)  //even minutes
  {
      //growing white bar
      display.fillRect(0,vspacing,across,2,1);
      display.display();
      delay(500);
      display.fillRect(across,vspacing,1,2,1);
      display.display();
      delay(500);
  }
  else // odd minutes
  {
      //growing black bar (white shrinking)
      display.fillRect(0,vspacing,128,2,1);
      display.fillRect(0,vspacing,across,2,0);
      display.display();
      delay(500);
      display.fillRect(across,vspacing,1,2,0);
      display.display();
      delay(500);
  }

  display.clearDisplay();

}
  
