#include <SPI.h>
#include <Wire.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1325.h>
#include <RTClib.h>
#include <Fat16.h>
#include <Fat16util.h>
#include <clockType.h>
clockType room[6];


  //#define SDd

  #define OLEDdebugHeader
  #define OLEDdebugSetup
  #define OLEDdebugClockSch
  #define OLEDdebugLines
  #define OLEDdebugDisplay
  #define OLEDdebugLZ

//  #define BUTTON


//Radio
  #define RF69_FREQ     915.0
  #define MY_ADDRESS    10
  #define RFM69_CS      8
  #define RFM69_INT     7
  #define RFM69_RST     4
  #define LED           13
  RH_RF69 rf69(RFM69_CS, RFM69_INT);
  RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);

//OLED
  #ifdef OLEDdebugHeader
  #define OLED_CS 6 
  #define OLED_RESET 9
  #define OLED_DC 5

  //#define OLED_CLK 13
  //#define OLED_MOSI 11
  Adafruit_SSD1325 display(OLED_DC, OLED_RESET, OLED_CS);
  #endif //OLEDdebugHeader

//RTC
  RTC_PCF8523 rtc;

#ifdef SDd
//SD
  SdCard card;
  Fat16 file;

  char name[] = "Shower00.CSV";
  #define error(s) error_P(PSTR(s)) // store error strings in flash to save RAM
  void error_P(const char* str) {
    PgmPrint("error: ");
    SerialPrintln_P(str);
    //if (card.errorCode) {
    //  PgmPrint("SD error: ");
    //  Serial.println(card.errorCode, HEX);
    //}
    //while(1);
    } 
  void writeNumber(uint32_t n) {
    //Write an unsigned number to file.
    //Normally you would use print to format numbers.
    uint8_t buf[10];
    uint8_t i = 0;
    do {
      i++;
      buf[sizeof(buf) - i] = n%10 + '0';
      n /= 10;
    } while (n);
    file.write(&buf[sizeof(buf) - i], i); // write the part of buf with the number
  }
#endif //SDd


//Front LEDs
  //               n/a  A0, A1, A2, A3, A4, A5
int DisplayLED[ ] = {0, 18, 19, 20, 21, 22, 23};

//Temp Button for resetting the display and clearing stored time values
  #define ButtonInput      12

//Serial
  #define SERIAL_BAUD   115200


int hh; int mm; //int ss;

bool ActiveDisplayWindow = true;
int ActiveMinuteCounter = 0;
int tempMinute = 0;
int ActiveStartTime = 60;
int TimeLimit = 59;

//*****************************************************************************

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1500);
  Serial.println(F("Feather RFM69HCW Receiver"));

  pinMode(ButtonInput, INPUT);

  pinMode(LED, OUTPUT);
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);
  
  // Reset and initialize the RFM module
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  if (!rf69_manager.init()) {
    Serial.println(F("RFM69 radio init failed"));
    while (1);
  }
  //Serial.println(F("RFM69 radio init OK!"));
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println(F("setFrequency failed"));
  }

  rf69.setTxPower(17, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
  Serial.print(F("RFM69 radio @"));  Serial.print((int)RF69_FREQ);  Serial.println(F(" MHz"));

#ifdef SDd
  // initialize the SD card
  if (!card.begin(10)) 
  {
    error("card.begin");
    Serial.println("SD card not inserted");
  }
  // initialize a FAT16 volume
  if (!Fat16::init(&card)) 
  {
    error("Fat16::init");
  }
  else
  {
    Serial.println("SD card ready");
  }


  // open ShowerData.csv and append new data
  for (uint8_t i = 0; i < 100; i++) {
    name[6] = i/10 + '0';
    name[7] = i%10 + '0';
    // O_CREAT - create the file if it does not exist
    // O_EXCL - fail if the file exists
    // O_WRITE - open for write
    if (!file.open(name, O_CREAT | O_APPEND | O_WRITE)) break;
  }
  if (!file.isOpen()) {
    error("file.open");
    Serial.println("Error opening SD file");
    return;
  }
#endif //SDd

#ifdef OLEDdebugSetup
  display.setTextColor(1);
  display.setTextSize(1);
  display.setTextWrap(false);
  display.begin(); 
  display.clearDisplay();
  display.display();
  Serial.println("Display ready");
#endif //OLEDdebugSetup

  for (int k=1;k<=6;k++)
  {
    //Serial.println(DisplayLED[k]);
    pinMode(DisplayLED[k], OUTPUT);
  }

  StartUpLightShow();

  ResetAll();
}

// Dont put this on the stack:
uint8_t data[] = "And hello back to you";
// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
uint8_t from;
               
//*****************************************************************************

void loop() {    
  DateTime now = rtc.now(); 
  //ADJUST TIME VARIABLES FOR 12-HR MODE
    //hours
    if (now.hour() == 0) {
      hh = 12; }
    else {
      if (now.hour() > 12) {
        hh = now.hour()-12; }
      else {
        hh = now.hour(); } }
   //minutes
     mm = now.minute(); 

      //Serial.println(F("RTC Updated..."));
 //****************************************
#ifdef OLEDdebugClockSch
    display.clearDisplay();
   //DISPLAY CLOCK IN UPPER RIGHT CORNER  
      if (hh >= 10) {
        display.setCursor(84,0); //97
        display.drawLine(82,0,82,9,1);display.drawLine(82,9,127,9,1); //94
      }
      else {
        display.setCursor(91,0); //103
        display.drawLine(88,0,88,9,1);display.drawLine(88,9,127,9,1); //100
      }
      display.print(hh);
      display.print(F(":")); 
      displayLZ(mm);
      display.setCursor(116,0);
      if (now.hour() >=12) {
        display.print("PM");
      }
      else {
        display.print("AM");
      }
 
  //DISPLAY SCHEDULED FLOAT TIME IN UPPER LEFT CORNER
  display.setCursor(0,0);
 // char schedule[13];
  switch (now.hour()) {   
    case 7: case 8: display.println(F("7AM-9:30AM")); 
      break;
    case 9:  case 10: display.println(F("9AM-10:30AM")); 
      break;
    case 11: case 12: display.println(F("11AM-12:30PM")); 
      break;
    case 13: case 14: display.println(F("1PM-2:30PM")); 
      break;  
    case 15: case 16: display.println(F("3PM-4:30PM")); 
      break;
    case 17: case 18: display.println(F("5PM-6:30PM")); 
      break;
    case 19: case 20: display.println(F("7PM-8:30PM")); 
      break;  
    case 21: case 22: display.println(F("9PM-10:30PM")); 
      break;
    case 23: case 0: case 1: display.println(F("11PM-1:30AM")); 
      break;
    case 2: case 3: case 4: display.println(F("2AM-4:30AM")); 
      break;
    default: display.println();
      break;      
   }
  #endif //OLEDdebugClockSch
  
 #ifdef OLEDdebugLines  
   //DISPLAY INDIVIDUAL LINES WITH TIMES AND DURATIONS 
    display.setCursor(0,16);
   
    for (int i=1;i<=6;i++)
    {
      display.print(i); display.print(F(". "));
      if (room[i].onExists()) {
        display.print(room[i].hh_on); display.print(F(":")); displayLZ(room[i].mm_on);
        if (room[i].offExists()) {
          display.print(F("-"));display.print(room[i].hh_off);display.print(F(":"));displayLZ(room[i].mm_off);
          display.print(F(" ("));display.print(room[i].dur);display.print(F("m)"));
        }
      }
      display.println();   
    }
#endif  //OLEDdebugLines

      //Serial.println(F("...Display Updated..."));
  //*********************************************

//Times that the display clears and resets for next floats                               
#ifdef SDd   
   if ((now.hour()==8  && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //8 am     (7-8:30)    
       (now.hour()==10 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //10 am    (9-10:30)
       (now.hour()==12 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //12 pm    (11-12:30) 
       (now.hour()==14 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //2 pm     (1-2:30)
       (now.hour()==16 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //4 pm     (3-4:30)
       (now.hour()==18 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //6 pm     (5-6:30)
       (now.hour()==20 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //8 pm     (7-8:30)
       (now.hour()==22 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //10 pm    (9-10:30)
       (now.hour()==0  && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //12 am     (11-1:30)
       (now.hour()==3  && now.minute()==0 && now.second()>=0 && now.second()<2) )        //3 am     (2-4:30)
   {
    delay(4000);
    SDlogBeforeFloat();
    ResetAll();    
   }
   
   if ((now.hour()==9  && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //9 am     (7-8:30)
       (now.hour()==11 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //11 am    (9-10:30)
       (now.hour()==13 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //1 pm    (11-12:30) 
       (now.hour()==15 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //3 pm     (1-2:30)
       (now.hour()==17 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //5 pm     (3-4:30)
       (now.hour()==19 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //7 pm     (5-6:30)
       (now.hour()==21 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //9 pm     (7-8:30)
       (now.hour()==23 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //11 pm    (9-10:30)
       (now.hour()==2  && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //2 am    (11-1:30)
       (now.hour()==5  && now.minute()==0 && now.second()>=0 && now.second()<2) )        //5 am     (2-4:30)
       
   {
    delay(4000);
    SDlogAfterFloat();
    ResetAll();    
   }
#endif //SDd

   if ((now.hour()>=8  && now.hour()<9)  ||
       (now.hour()>=10 && now.hour()<11) ||
       (now.hour()>=12 && now.hour()<13) ||
       (now.hour()>=14 && now.hour()<15) ||
       (now.hour()>=16 && now.hour()<17) ||
       (now.hour()>=18 && now.hour()<19) ||
       (now.hour()>=20 && now.hour()<21) ||
       (now.hour()>=22 && now.hour()<23) ||
       (now.hour()>=1  && now.hour()<2)  ||
       (now.hour()>=4  && now.hour()<5)   )
    {
       ActiveDisplayWindow = true;
    }
    else 
    {
       ActiveDisplayWindow = false;
    }
    
 



   
#ifdef BUTTON
  if (digitalRead(ButtonInput) == HIGH)   //kk
  {
    Serial.print("Button Pushed...");
    delay(1500);
    SDlogBeforeFloat();
    SDlogAfterFloat();
    ResetAll();
    //ActiveMinuteCounter = ActiveMinuteCounter + 60;
  }
#endif
    
#ifdef OLEDdebugDisplay
  display.display();
  delay(1000);
#endif //OLEDdebugDisplay

    if (rf69_manager.available()) {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
   //Serial.print(F("Message Available..."));

     
    if (rf69_manager.recvfromAck(buf, &len, &from)) {
      buf[len] = 0; // zero out remaining string
      Serial.print(F("Got packet from #")); Serial.print(from);
      //Serial.print(F(" [RSSI :")); Serial.print(rf69.lastRssi()); Serial.print(F("]"));
      Serial.print(F(": "));
      Serial.print((char*)buf);
    
      //check received message sender ID, if and turn ON or OFF corresponding Front LED. Also store time info.
       if ( 1 <= from && from <=6)
       {
          //  Always log. Before float lights go on and off with shower. After float lights go on and stay on until reset later.
          if (strstr((char*)buf, "ON")) 
          {
            if (strstr((char*)buf, "LBat")) 
            {
              Blink(DisplayLED[from], 200, 8); //blink LED 8 times, 40ms between blinks
              //Serial.println("blink time!");
            }
            digitalWrite(DisplayLED[from],LOW); 
            room[from].setTimeON(now.hour(),now.minute());
            room[from].printTimeON(); 
          }
          
          if (strstr((char*)buf, "OFF")) {
            if (!ActiveDisplayWindow)
            {
             digitalWrite(DisplayLED[from],HIGH); 
            }
            room[from].setTimeOFF(now.hour(),now.minute());
            room[from].printTimeOFF();
          }
     

       // Send a reply back to the originator client
       if (!rf69_manager.sendtoWait(data, sizeof(data), from)){
         Serial.println(F("Sending failed (no ack)"));
       }
    }
        
  }  

//
////Serial CLOCK  
////      DateTime now = rtc.now();
//      Serial.print(now.hour());
//      Serial.print(F(":")); 
//      Serial.print(now.minute());
//      Serial.print(F(":")); 
//      Serial.print(now.second());
//      if (now.hour() >=12) {
//        Serial.println("PM");
//      }
//      else {
//        Serial.println("AM");
//      }
   






     
  delay(1000);  //End Loop
  }
}


//**************** SUBFUNCTIONS *************************************************************

void ResetAll()
{
    for (int i=1;i<=6;i++)
    {
      room[i].setTimeON(-1,0);
      room[i].setTimeOFF(-1,0);
    }
    //all off
    for (int l=1;l<=6;l++) { digitalWrite(DisplayLED[l],HIGH); }

}

void StartUpLightShow()
{
  int delayTime = 150;
  //all off
  for (int l=1;l<=6;l++) { digitalWrite(DisplayLED[l],HIGH); }
 
  //blink all once
  for (int l=1;l<=6;l++) { digitalWrite(DisplayLED[l],LOW); }
  delay(delayTime*2);
  for (int l=1;l<=6;l++) { digitalWrite(DisplayLED[l],HIGH); }

  //cycle each on then off in increasing order
  for (int l=1;l<=6;l++) 
  {
    digitalWrite(DisplayLED[l],LOW);
    delay(delayTime);
    digitalWrite(DisplayLED[l],HIGH);
    delay(delayTime/2);
  }

  //blink all longer
  for (int l=1;l<=6;l++) { digitalWrite(DisplayLED[l],LOW); }
  delay(delayTime*4);
  for (int l=1;l<=6;l++) { digitalWrite(DisplayLED[l],HIGH); }
}

#ifdef OLEDdebugLZ
void displayLZ(int zMin) //adds a leading zero if needed to fill up two spaces
{
    if (zMin < 10) {
      display.print(F("0"));
      display.print(zMin);
    }
    else {
      display.print(zMin);
    }
}
#endif //OLEDdebugLZ

#ifdef SDd
void sdLZ(int zMin) //adds a leading zero if needed to fill up two spaces
{
    if (zMin < 10) {
      file.print(F("0"));
      file.print(zMin);
    }
    else {
      file.print(zMin);
    }
}


void SDlogBeforeFloat()
{
  //file.open(name, O_APPEND | O_WRITE);
  Serial.print("Writing to: ");
  Serial.print(name);

  //06/26/17,Room 1,,,,Room 2,,,,Room 3,,,,Room 4,,,,Room 5,,,,Room 6
  //file.print("RTC MMDDYY");
  DateTime now = rtc.now();
  file.print(now.month(), DEC); 
  file.print('/');
  file.print(now.day(), DEC); 
  file.print('/');
  file.print(now.year(), DEC); 
  comma();
  
  file.print("Room 1");comma4();file.print("Room 2");comma4();file.print("Room 3");comma4();file.print("Room 4");comma4();file.print("Room 5");comma4();file.print("Room 6");
  file.println();
  
  //7AM – 8:30AM,Time On,Time Off,Duration,,Time On,Time Off,Duration,,Time On,Time Off,Duration,,Time On,Time Off,Duration,,Time On,Time Off,Duration,,Time On,Time Off,Duration
  //file.print("7AM-8:30AM");
    switch (now.hour()) {   
    case 7: case 8:  file.print(F("7AM-9:30AM")); 
      break;
    case 9:  case 10: file.print(F("9AM-10:30AM")); 
      break;
    case 11: case 12: file.print(F("11AM-12:30PM")); 
      break;
    case 13: case 14: file.print(F("1PM-2:30PM")); 
      break;  
    case 15: case 16: file.print(F("3PM-4:30PM")); 
      break;
    case 17: case 18: file.print(F("5PM-6:30PM")); 
      break;
    case 19: case 20: file.print(F("7PM-8:30PM")); 
      break;  
    case 21: case 22: file.print(F("9PM-10:30PM")); 
      break;
    case 23: case 0: case 1: file.print(F("11PM-1:30AM")); 
      break;
    case 2: case 3: case 4: file.print(F("2AM-4:30AM")); 
      break;
    default: comma();
      break;      
   }
  comma();
  for (int i=1; i<=6; i++){
    file.print("Time On");comma();
    file.print("Time Off");comma();
    file.print("Duration");comma2();
  }
  file.println();

  //Before Float Shower,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02
  file.print(F("Before-Float Shower"));
  comma();

  SDprintTimes();
  
  if (!file.sync()) error("sync");
  PgmPrintln(" ...Done");
}

void SDlogAfterFloat()
{
  //file.open(name, O_APPEND | O_WRITE);
  Serial.print("Writing to: ");
  Serial.print(name);

  //Before Float Shower,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02
  file.print(F("After-Float Shower"));
  comma();
  
  SDprintTimes();
  file.println();
  
  if (!file.sync()) error("sync");
  PgmPrintln("Done with After times");
}

void SDprintTimes()
{
  for (int i=1;i<=6;i++)
  {
    if (room[i].onExists() && room[i].offExists()) {
      timeFormat(room[i].hh_on, room[i].mm_on, room[i].ampm_on);comma();
      timeFormat(room[i].hh_off, room[i].mm_off, room[i].ampm_off);comma();
      file.print(room[i].dur); comma2(); }
    else {
      comma4();
    }
  }
  file.println();
}

void timeFormat(int tempHH, int tempMM, bool tempAP)
{
  
  //DateTime now = rtc.now();
  sdLZ(tempHH);
  file.print(F(":"));
  sdLZ(tempMM); 
  if (tempAP > 0){
    file.print(F(" PM"));
  }
  else {
    file.print(F(" AM"));
  }
}

void comma()
{
  file.print(",");
}

void comma2()
{
  file.print(",,");
}

void comma4()
{
  file.print(",,,,");
}
#endif //SDd

void Blink(byte PIN, byte DELAY_MS, byte loops)
{
  for (byte i=0; i<loops; i++)
  {
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
  }
}
