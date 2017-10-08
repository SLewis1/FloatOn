#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1325.h>
#include "RTClib.h"
#include <Wire.h>
RTC_PCF8523 rtc;

#define OLED_CS 6 //was 10
#define OLED_RESET 9
#define OLED_DC 5 //was 8

Adafruit_SSD1325 display(OLED_DC, OLED_RESET, OLED_CS);

int textSize = 4;
int sec = 0;
int across = 0;

//#define seconds  //show seconds

void setup() {
//  Serial.begin(9600);
//  delay(1000);
//  Serial.println("OLED test");
  
  display.setTextSize(textSize);
  display.setTextColor(1);
  display.setTextWrap(false);
  
  display.begin(); 
  display.clearDisplay();
  display.display();
  
}

void loop() {
  
  DateTime now = rtc.now();
  Serial.print(now.hour(),DEC); Serial.print(":"); Serial.print(now.minute(),DEC); Serial.print(":"); Serial.println(now.second(),DEC);
  display.setCursor(0,0);
//hours
  if (now.hour() > 12) {
    display.print(now.hour()-12);
  }
  else {
    display.print(now.hour());
  }
  
  //display.setCursor(4*textSize,0);
  display.print(":"); 

//minutes
  //display.setCursor(8*textSize,0);
  if (now.minute() < 10) {
    display.print("0");
  }
  display.print(now.minute()); 
 
/*//ampm 
  if (now.hour() < 12) {
    display.println("am");
  }
  else {
    display.println("pm");
  }
*/
 //display.display();

//seconds
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
//Serial.print("  ");Serial.println(2*now.second());
//Serial.print("  ");Serial.println(across);

  across = floor(2.16*now.second());
  if (now.minute() % 2 == 0)  //even minutes
  {
      //growing white bar
      display.fillRect(0,36,across,2,1);
      display.display();
      delay(500);
      display.fillRect(across,36,1,2,1);
      display.display();
      delay(500);
  }
  else // odd minutes
  {
      //growing black bar (white shrinking)
      display.fillRect(0,36,128,2,1);
      display.fillRect(0,36,across,2,0);
      display.display();
      delay(500);
      display.fillRect(across,36,1,2,0);
      display.display();
      delay(500);
  }

  display.clearDisplay();

}
  
