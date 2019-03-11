// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

RTC_PCF8523 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup () {
  
  while (!Serial) {
    delay(1);  // for Leonardo/Micro/Zero
  }

  Serial.begin(57600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.initialized()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
   
  }
   //rtc.adjust(DateTime(2017, 10, 2, 18, 41, 20));   //rtc at float on     10/30  9:09pm +2:38
   //rtc.adjust(DateTime(2017, 10, 30, 21, 10, 10));   //rtc at float on 
   //rtc.adjust(DateTime(2017, 11, 6, 18, 0, 0));   //rtc at float on 
   //rtc.adjust(DateTime(2017, 12, 18, 22, 29, 0));   //rtc at float on 
   //rtc.adjust(DateTime(2018, 1, 8, 20, 33, 0));   //rtc at float on  was +50 sec at 8:35pm (set 2 min slow)
   //4/15/18 at 7:07:00 rtc was 6:17:03 (w/o DST) +10m 3s = +603s [remember was set 2 min slow] over 97 days (+6s/day)
   //rtc.adjust(DateTime(2018, 4, 15, 19, 12, 0));   //rtc at float on  was +10m +50 sec at 8:35pm (set 2 min slow)
    
   //12/1/18 sat 10:40:00 AM  rtc at float on
   //board time 2018/12/1 (Saturday) 0:7:34
   //rtc.adjust(DateTime(2018, 12, 1, 10, 43, 10)); 
  
  /*//rtc at home
  //rtc.adjust(DateTime(2017, 10, 6, 17, 19, 10));   //rtc at home
  //at 1:48:00pm on October 2, 2017, the rtc time is 1:49:35pm
  // +0:01:35 (95 seconds) over, time elapsed
  //rtc.adjust(DateTime(2017, 11, 8, 11, 6, 10));   //rtc at home
  //at 10:53:00am on November 8, 2017, the rtc time is 10:56:31am (dst adjusted)
  // +0:03:31 (211 seconds) over, time elapsed since last rtc setting
  // 32 days, 17 hrs, 33 min, 50 sec
*/
   
rtc.adjust(DateTime(2018, 12, 1, 10, 43, 10));   

}

void loop () {
    DateTime now = rtc.now();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    if(now.hour() < 12) Serial.print(now.hour(), DEC);
    if(now.hour() >= 12) Serial.print(now.hour()-12, DEC);
    if(now.hour() == 0) Serial.print(12, DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    
    Serial.println();
    
    Serial.println();
    delay(3000);
}
