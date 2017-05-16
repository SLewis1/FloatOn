#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1325.h>
#include <Wire.h>
#include "RTClib.h"
RTC_PCF8523 rtc;

#define OLED_CS 6 
#define OLED_RESET 9
#define OLED_DC 5

Adafruit_SSD1325 display(OLED_DC, OLED_RESET, OLED_CS);

#define DisplayLED1          A0
#define DisplayLED2          A1
#define DisplayLED3          A2
#define DisplayLED4          A3
#define DisplayLED5          A4
#define DisplayLED6          A5

#define LightDemoButton      13

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("OLED test");

  pinMode(DisplayLED1, OUTPUT);
  pinMode(DisplayLED2, OUTPUT);
  pinMode(DisplayLED3, OUTPUT);
  pinMode(DisplayLED4, OUTPUT);
  pinMode(DisplayLED5, OUTPUT);
  pinMode(DisplayLED6, OUTPUT);

  pinMode(LightDemoButton, INPUT);
  
  display.begin(); 
  display.clearDisplay();
  display.display();
 
  display.setTextColor(1);
  display.setTextSize(1);
  display.setTextWrap(false);
 
  digitalWrite(DisplayLED1, LOW); digitalWrite(DisplayLED2, LOW); digitalWrite(DisplayLED3, LOW);
  digitalWrite(DisplayLED4, LOW); digitalWrite(DisplayLED5, LOW); digitalWrite(DisplayLED6, LOW);

  StartUpLightShow(); 
  }

void loop() {
  DateTime now = rtc.now();

  display.setCursor(96,0);
  MyClock();
 
  display.setCursor(0,0);
  switch (now.hour()) {
    case 7: case 8: display.println("7AM-9:30AM"); 
      break;
    case 9:  case 10: display.println("9AM-10:30AM"); 
      break;
    case 11: case 12: display.println("11AM-12:30PM"); 
      break;
    case 13: case 14: display.println("1PM-2:30PM"); 
      break;  
    case 15: case 16: display.println("3PM-4:30PM"); 
      break;
    case 17: case 18: display.println("5PM-6:30PM"); 
      break;
    case 19: case 20: display.println("7PM-8:30PM"); 
      break;  
    case 21: case 22: display.println("9PM-10:30PM"); 
      break;
    case 23: case 0: case 1: display.println("11PM-1:30AM"); 
      break;
    case 2: case 3: case 4: display.println("2AM-4:30AM"); 
      break;
    default: display.println();
      break;      
  }

  
  display.println("");         
  display.println("1. 04:16-04:26  (10m)");
  display.println("2. 04:18-04:26  (8m)");
  display.println("3. 04:18-04:25  (7m)");
  display.println("4. 04:20            ");
  display.println("5. 04:10-04:15  (5m)");
  display.println("6. 04:27            ");
  
  display.display();
  delay(1000);

  //turn all lights off
  digitalWrite(DisplayLED1, HIGH); digitalWrite(DisplayLED2, HIGH); digitalWrite(DisplayLED3, HIGH);
  digitalWrite(DisplayLED4, HIGH); digitalWrite(DisplayLED5, HIGH); digitalWrite(DisplayLED6, HIGH);
  
  //button press triggers light show
  if (digitalRead(LightDemoButton) == HIGH) {
    Serial.println("Lights!");
    StartUpLightShow();  
  }
  
  display.clearDisplay();

}

void MyClock()
{
   DateTime now = rtc.now();
   
   //hours
    int hh = 0;
    if (now.hour() == 0) {
      hh = 12;
      //display.print("12");
      }
    else {
      if (now.hour() > 12) {
        hh = now.hour()-12;
        //display.print(now.hour()-12);
        }
      else {
        hh = now.hour();
        //display.print(now.hour());
      }
    }
    display.print(hh);
    display.print(":"); 
 //minutes
    if (now.minute() < 10) {
      display.print("0");
    }
    display.print(now.minute());
    display.print(" ");  
 //ampm   
//    if (now.hour() < 12) {
//      display.println("AM");
//    }
//    else {
//      display.println("PM");
//    }
 //border line
    display.drawLine(92,0,92,9,1);display.drawLine(92,9,127,9,1);
    //display.drawRect(92,0,36,13,1);
}

void StartUpLightShow()
{
  int delayTime = 300;
  //all off
  digitalWrite(DisplayLED1, HIGH); digitalWrite(DisplayLED2, HIGH); digitalWrite(DisplayLED3, HIGH);
  digitalWrite(DisplayLED4, HIGH); digitalWrite(DisplayLED5, HIGH); digitalWrite(DisplayLED6, HIGH);

  //blink all once
  digitalWrite(DisplayLED1, LOW); digitalWrite(DisplayLED2, LOW); digitalWrite(DisplayLED3, LOW);
  digitalWrite(DisplayLED4, LOW); digitalWrite(DisplayLED5, LOW); digitalWrite(DisplayLED6, LOW);
  
  delay(delayTime*2);
  digitalWrite(DisplayLED1, HIGH); digitalWrite(DisplayLED2, HIGH); digitalWrite(DisplayLED3, HIGH);
  digitalWrite(DisplayLED4, HIGH); digitalWrite(DisplayLED5, HIGH); digitalWrite(DisplayLED6, HIGH);
  
  //cycle each on then off
  digitalWrite(DisplayLED1, LOW); delay(delayTime); digitalWrite(DisplayLED1, HIGH); delay(delayTime/2);
  digitalWrite(DisplayLED2, LOW); delay(delayTime); digitalWrite(DisplayLED2, HIGH); delay(delayTime/2);
  digitalWrite(DisplayLED3, LOW); delay(delayTime); digitalWrite(DisplayLED3, HIGH); delay(delayTime/2);
  digitalWrite(DisplayLED4, LOW); delay(delayTime); digitalWrite(DisplayLED4, HIGH); delay(delayTime/2);
  digitalWrite(DisplayLED5, LOW); delay(delayTime); digitalWrite(DisplayLED5, HIGH); delay(delayTime/2);
  digitalWrite(DisplayLED6, LOW); delay(delayTime); digitalWrite(DisplayLED6, HIGH); delay(delayTime/2);

  //blink all once
  digitalWrite(DisplayLED1, LOW); digitalWrite(DisplayLED2, LOW); digitalWrite(DisplayLED3, LOW);
  digitalWrite(DisplayLED4, LOW); digitalWrite(DisplayLED5, LOW); digitalWrite(DisplayLED6, LOW);
  delay(delayTime*4);
  digitalWrite(DisplayLED1, HIGH); digitalWrite(DisplayLED2, HIGH); digitalWrite(DisplayLED3, HIGH);
  digitalWrite(DisplayLED4, HIGH); digitalWrite(DisplayLED5, HIGH); digitalWrite(DisplayLED6, HIGH);

  int delayMixOrder = 3000;
  digitalWrite(DisplayLED1, LOW); delay(delayMixOrder); digitalWrite(DisplayLED4, LOW); delay(delayMixOrder); digitalWrite(DisplayLED2, LOW); delay(delayMixOrder);
  digitalWrite(DisplayLED6, LOW); delay(delayMixOrder); digitalWrite(DisplayLED5, LOW); delay(delayMixOrder); digitalWrite(DisplayLED3, LOW);
  delay(delayTime*4);

  digitalWrite(DisplayLED6, HIGH); delay(delayTime*2); digitalWrite(DisplayLED5, HIGH); delay(delayTime*2); digitalWrite(DisplayLED4, HIGH); delay(delayTime*2);
  digitalWrite(DisplayLED3, HIGH); delay(delayTime*2); digitalWrite(DisplayLED2, HIGH); delay(delayTime*2); digitalWrite(DisplayLED1, HIGH);
  delay(delayTime*2);
}
