#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>
#include <RTClib.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1325.h>
#include <Fat16.h>
#include <Fat16util.h>

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

  #define OLED_CS 6 
  #define OLED_RESET 9
  #define OLED_DC 5
  //Adafruit_SSD1325 display(OLED_DC, OLED_RESET, OLED_CS);
  
//RTC
  RTC_PCF8523 rtc;

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

  
//Front LEDs
  #define DisplayLED1          A0
  #define DisplayLED2          A1
  #define DisplayLED3          A2
  #define DisplayLED4          A3
  #define DisplayLED5          A4
  #define DisplayLED6          A5

//Temp Button for resetting the display and clearing stored time values
  #define ButtonInput      12

//Serial
  #define SERIAL_BAUD   115200


int hh; int mm; //int ss;
int hh1_ON=0;  int hh1_OFF; int mm1_ON;  int mm1_OFF; int dur1;
int hh2_ON=0;  int hh2_OFF; int mm2_ON;  int mm2_OFF; int dur2;
int hh3_ON=0;  int hh3_OFF; int mm3_ON;  int mm3_OFF; int dur3;
int hh4_ON=0;  int hh4_OFF; int mm4_ON;  int mm4_OFF; int dur4;
int hh5_ON=0;  int hh5_OFF; int mm5_ON;  int mm5_OFF; int dur5;
int hh6_ON=0;  int hh6_OFF; int mm6_ON;  int mm6_OFF; int dur6;

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
  pinMode(DisplayLED1, OUTPUT);
  pinMode(DisplayLED2, OUTPUT);
  pinMode(DisplayLED3, OUTPUT);
  pinMode(DisplayLED4, OUTPUT);
  pinMode(DisplayLED5, OUTPUT);
  pinMode(DisplayLED6, OUTPUT);

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
  Serial.println(F("RFM69 radio init OK!"));
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println(F("setFrequency failed"));
  }

  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
  Serial.print(F("RFM69 radio @"));  Serial.print((int)RF69_FREQ);  Serial.println(F(" MHz"));
  /*
  display.setTextColor(1);
  display.setTextSize(1);
  display.setTextWrap(false);
  display.begin(); 
  display.clearDisplay();
  display.display();
*/
  // initialize the SD card
  if (!card.begin(10)) error("card.begin");
  // initialize a FAT16 volume
  if (!Fat16::init(&card)) error("Fat16::init");

  StartUpLightShow();
  
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
   //seconds
     //ss = now.second();  

      //Serial.println(F("RTC Updated..."));
 //****************************************
/*
    display.clearDisplay();
   //DISPLAY CLOCK IN UPPER RIGHT CORNER  
      if (hh >= 10) {
        display.setCursor(84,0); //97
        //display.drawLine(82,0,82,9,1);display.drawLine(82,9,127,9,1); //94
      }
      else {
        display.setCursor(91,0); //103
        //display.drawLine(88,0,88,9,1);display.drawLine(88,9,127,9,1); //100
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

  
  switch (now.hour()) {   
    case 7: case 8:  display.println(F("7AM-9:30AM")); 
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
  
   
   //DISPLAY INDIVIDUAL LINES WITH TIMES AND DURATIONS 
    display.setCursor(0,16);
    display.print(F("1. ")); 
      if (hh1_ON > 0) {
        display.print(hh1_ON); display.print(F(":")); displayLZ(mm1_ON);
        if (hh1_OFF > 0) {
          display.print(F("-"));display.print(hh1_OFF);display.print(F(":"));displayLZ(mm1_OFF);
          display.print(F(" ("));display.print(dur1);display.print(F("m)"));
        }
      }
      display.println();  
    //Serial.print(1);
      
    display.print(F("2. ")); 
     if (hh2_ON > 0) {
        display.print(hh2_ON); display.print(F(":"));displayLZ(mm2_ON);
        if (hh2_OFF > 0) {
          display.print(F("-"));display.print(hh2_OFF);display.print(F(":"));displayLZ(mm2_OFF);
          display.print(F(" ("));display.print(dur2);display.print(F("m)"));
        }     
     }
     display.println();
     //Serial.print(2);
      
    display.print(F("3. ")); 
      if (hh3_ON > 0) {
        display.print(hh3_ON); display.print(F(":"));displayLZ(mm3_ON);
        if (hh3_OFF >0) {
          display.print(F("-"));display.print(hh3_OFF);display.print(F(":"));displayLZ(mm3_OFF);
          display.print(F(" ("));display.print(dur3);display.print(F("m)")); 
        }
      }
      display.println();
      //Serial.print(3);

    display.print(F("4. ")); 
      if (hh4_ON > 0) {
        display.print(hh4_ON); display.print(F(":"));displayLZ(mm4_ON);
        if (hh4_OFF > 0) {
          display.print(F("-"));display.print(hh4_OFF);display.print(F(":"));displayLZ(mm4_OFF);
          display.print(F(" ("));display.print(dur4);display.print(F("m)")); 
        }
      }
      display.println();
      //Serial.print(4);
      
    display.print(F("5. ")); 
      if (hh5_ON > 0) {
        display.print(hh5_ON); display.print(F(":"));displayLZ(mm5_ON);
        if (hh5_OFF > 0) {
          display.print(F("-"));display.print(hh5_OFF);display.print(F(":"));displayLZ(mm5_OFF);
          display.print(F(" ("));display.print(dur5);display.print(F("m)")); 
        }      
      }
      display.println();
      //Serial.print(5);
     
    display.print(F("6. ")); 
      if (hh6_ON > 0) {
        display.print(hh6_ON); display.print(F(":"));displayLZ(mm6_ON);
        if (hh6_OFF > 0) {
          display.print(F("-"));display.print(hh6_OFF);display.print(F(":"));displayLZ(mm6_OFF);
          display.print(F(" ("));display.print(dur6);display.print(F("m)"));
        }
      }
      display.println();
      //Serial.print(6);

      //Serial.println(F("...Display Updated..."));
*/

//Times that the display clears and resets for next floats                               
    
     if ((now.hour()==7  && now.minute()==59 && now.second()>=57 && now.second()<59) ||       //8     (7-8:30)    
         (now.hour()==9  && now.minute()==59 && now.second()>=57 && now.second()<59) ||       //10    (9-10:30)
         (now.hour()==11 && now.minute()==59 && now.second()>=57 && now.second()<59) ||       //12    (11-12:30) 
         (now.hour()==13 && now.minute()==59 && now.second()>=57 && now.second()<59) ||       //2     (1-2:30)
         (now.hour()==15 && now.minute()==59 && now.second()>=57 && now.second()<59) ||       //4     (3-4:30)
         (now.hour()==17 && now.minute()==59 && now.second()>=57 && now.second()<59) ||       //6     (5-6:30)
         (now.hour()==19 && now.minute()==59 && now.second()>=57 && now.second()<59) ||       //8     (7-8:30)
         (now.hour()==21 && now.minute()==59 && now.second()>=57 && now.second()<59) ||       //10    (9-10:30)
         (now.hour()==23 && now.minute()==59 && now.second()>=57 && now.second()<59) ||       //1     (11-1:30)
         (now.hour()==2  && now.minute()==59 && now.second()>=57 && now.second()<59) )        //3     (2-4:30)
   {
      delay(3000);
      //SDlogBeforeFloat();
      ResetTimeVariables();
   }    
   

   if ((now.hour()==9  && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //8 am   to  9     (7-8:30)    
       (now.hour()==11 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //10 am  to 11     (9-10:30)
       (now.hour()==13 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //12 pm  to 1      (11-12:30) 
       (now.hour()==15 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //2 pm   to  3     (1-2:30)
       (now.hour()==17 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //4 pm   to  5     (3-4:30)
       (now.hour()==19 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //6 pm   to  7     (5-6:30)
       (now.hour()==21 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //8 pm   to  9     (7-8:30)
       (now.hour()==23 && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //10 pm  to  11    (9-10:30)
       (now.hour()==2  && now.minute()==0 && now.second()>=0 && now.second()<2) ||       //1 am   to  2     (11-1:30)
       (now.hour()==5  && now.minute()==0 && now.second()>=0 && now.second()<2) )        //4 am   to  5     (2-4:30)
   {
      delay(3000);
      SDlogAfterFloat();
      ResetTimeVariables();    
   }
   
    
   

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
/*
  if (ActiveDisplayWindow)
  {
     display.setCursor(90,54);
     display.print("Active");
     Serial.println(F("Listening"));
  }
  else 
  {
     display.setCursor(85,54);
     display.print("Waiting");
     Serial.println(F("Not Listening"));
  }

  display.display();
  delay(1000);
*/

  if (digitalRead(ButtonInput) == HIGH)
  {
    Serial.print(F("Button pressed... ")); 
    delay(1500);
    //********************************************************************************************************
 
    //********************************************************************************
    SDlogBeforeFloat();
    SDprintTimes();file.println();file.println();file.println();
    ResetTimeVariables();
    //Serial.println(F("and written to SD"));
  }
/*
  if (ActiveDisplayWindow)  //Listen with the Radio
  {
    if (rf69_manager.available()) {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
   Serial.print(F("Message Available..."));

     
    if (rf69_manager.recvfromAck(buf, &len, &from)) {
      buf[len] = 0; // zero out remaining string
      
      Serial.print(F("Got packet from #")); Serial.print(from);
      //Serial.print(F(" [RSSI :")); Serial.print(rf69.lastRssi()); Serial.print(F("]"));
      Serial.print(F(" : "));
      Serial.println((char*)buf);
     //Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks

      //check received message sender ID, if and turn ON or OFF corresponding Front LED. Also store time info.
   
      if (from == 1) {
          if (strstr((char*)buf, "ON")) {
              digitalWrite(DisplayLED1,LOW);
              hh1_ON = hh; mm1_ON = now.minute();
          }
          if (strstr((char*)buf, "OFF")) {
              hh1_OFF = hh; mm1_OFF = now.minute();
          }
      }  
    
      if (from == 2) {
          if (strstr((char*)buf, "ON")) {
              digitalWrite(DisplayLED2,LOW);
              hh2_ON = hh; mm2_ON = now.minute();
          }
          if (strstr((char*)buf, "OFF")) {
              hh2_OFF = hh; mm2_OFF = now.minute();
          }
      }

      if (from == 3) {
          if (strstr((char*)buf, "ON")) {
              digitalWrite(DisplayLED3,LOW);
              hh3_ON = hh; mm3_ON = now.minute();
          }
          if (strstr((char*)buf, "OFF")) {
              hh3_OFF = hh; mm3_OFF = now.minute();
          }
      }
      
      if (from == 4) {
          if (strstr((char*)buf, "ON")) {
              digitalWrite(DisplayLED4,LOW);
              hh4_ON = hh; mm4_ON = now.minute();
          }
          if (strstr((char*)buf, "OFF")) {           
              hh4_OFF = hh; mm4_OFF = now.minute();
          }
      }


     if (from == 5) {
          if (strstr((char*)buf, "ON")) {
              digitalWrite(DisplayLED5,LOW);
              hh5_ON = hh; mm5_ON = now.minute();
          }
          if (strstr((char*)buf, "OFF")) {           
              hh5_OFF = hh; mm5_OFF = now.minute();
          }
      }


      if (from == 6) {
          if (strstr((char*)buf, "ON")) {
              digitalWrite(DisplayLED6,LOW);
              hh6_ON = hh; mm6_ON = now.minute();
          }
          if (strstr((char*)buf, "OFF")) {
              hh6_OFF = hh; mm6_OFF = now.minute();
          }
      }

      // Send a reply back to the originator client
      if (!rf69_manager.sendtoWait(data, sizeof(data), from)){
        Serial.println(F("Sending failed (no ack)"));
      }
    
    
    }
  
     dur1 = mm1_OFF - mm1_ON;   if (dur1 < 0) dur1 = dur1 + 60;
     dur2 = mm2_OFF - mm2_ON;   if (dur2 < 0) dur2 = dur2 + 60;  
     dur3 = mm3_OFF - mm3_ON;   if (dur3 < 0) dur3 = dur3 + 60;
     dur4 = mm4_OFF - mm4_ON;   if (dur4 < 0) dur4 = dur4 + 60;
     dur5 = mm5_OFF - mm5_ON;   if (dur5 < 0) dur5 = dur5 + 60;
     dur6 = mm6_OFF - mm6_ON;   if (dur6 < 0) dur6 = dur6 + 60;
   // Serial.print(F("Duration Calculations..."));
        
  }
    
    //Serial.println(F("Finished radio loop."));
    //Serial.println();Serial.println();
  } //******End Radio code***********************     
  */
     

  
  if (ActiveDisplayWindow)  //Listen with the Radio
  {
    if (rf69_manager.available()) {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
   //Serial.print(F("Message Available..."));

     
    if (rf69_manager.recvfromAck(buf, &len, &from)) {
      buf[len] = 0; // zero out remaining string
      
      Serial.print(F("Got packet from #")); Serial.print(from);
      Serial.print(F(" [RSSI :")); Serial.print(rf69.lastRssi()); Serial.print(F("]"));
      Serial.print(F(" : "));
      Serial.println((char*)buf);
     //Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks

      //check received message sender ID, if and turn ON or OFF corresponding Front LED. Also store time info.
   
      if (from == 1) {
          if (strstr((char*)buf, "ON")) {
              digitalWrite(DisplayLED1,LOW);
              hh1_ON = hh; mm1_ON = now.minute();
          }
          if (strstr((char*)buf, "OFF")) {
              hh1_OFF = hh; mm1_OFF = now.minute();
          }
      }  
    
      if (from == 2) {
          if (strstr((char*)buf, "ON")) {
              digitalWrite(DisplayLED2,LOW);
              hh2_ON = hh; mm2_ON = now.minute();
          }
          if (strstr((char*)buf, "OFF")) {
              hh2_OFF = hh; mm2_OFF = now.minute();
          }
      }

      if (from == 3) {
          if (strstr((char*)buf, "ON")) {
              digitalWrite(DisplayLED3,LOW);
              hh3_ON = hh; mm3_ON = now.minute();
          }
          if (strstr((char*)buf, "OFF")) {
              hh3_OFF = hh; mm3_OFF = now.minute();
          }
      }
      
      if (from == 4) {
          if (strstr((char*)buf, "ON")) {
              digitalWrite(DisplayLED4,LOW);
              hh4_ON = hh; mm4_ON = now.minute();
          }
          if (strstr((char*)buf, "OFF")) {           
              hh4_OFF = hh; mm4_OFF = now.minute();
          }
      }


     if (from == 5) {
          if (strstr((char*)buf, "ON")) {
              digitalWrite(DisplayLED5,LOW);
              hh5_ON = hh; mm5_ON = now.minute();
          }
          if (strstr((char*)buf, "OFF")) {           
              hh5_OFF = hh; mm5_OFF = now.minute();
          }
      }


      if (from == 6) {
          if (strstr((char*)buf, "ON")) {
              digitalWrite(DisplayLED6,LOW);
              hh6_ON = hh; mm6_ON = now.minute();
          }
          if (strstr((char*)buf, "OFF")) {
              hh6_OFF = hh; mm6_OFF = now.minute();
          }
      }

      // Send a reply back to the originator client
      if (!rf69_manager.sendtoWait(data, sizeof(data), from)){
        Serial.println(F("Sending failed (no ack)"));
      }
    
    
    }
  
     dur1 = mm1_OFF - mm1_ON;   if (dur1 < 0) dur1 = dur1 + 60;
     dur2 = mm2_OFF - mm2_ON;   if (dur2 < 0) dur2 = dur2 + 60;  
     dur3 = mm3_OFF - mm3_ON;   if (dur3 < 0) dur3 = dur3 + 60;
     dur4 = mm4_OFF - mm4_ON;   if (dur4 < 0) dur4 = dur4 + 60;
     dur5 = mm5_OFF - mm5_ON;   if (dur5 < 0) dur5 = dur5 + 60;
     dur6 = mm6_OFF - mm6_ON;   if (dur6 < 0) dur6 = dur6 + 60;
   // Serial.print(F("Duration Calculations..."));
        
  }
    
    //Serial.println(F("Finished radio loop."));
    //Serial.println();Serial.println();
  } //******End Radio code***********************     
  
     




}

//*******************************************************************************************
//**************** SUBFUNCTIONS *************************************************************
//*******************************************************************************************

void ResetTimeVariables()
{
    hh1_ON=0; hh1_OFF=0;
    hh2_ON=0; hh2_OFF=0;
    hh3_ON=0; hh3_OFF=0;
    hh4_ON=0; hh4_OFF=0;
    hh5_ON=0; hh5_OFF=0;
    hh6_ON=0; hh6_OFF=0; 
    //ActiveMinuteCounter = 0;
    digitalWrite(DisplayLED1, HIGH); digitalWrite(DisplayLED2, HIGH); digitalWrite(DisplayLED3, HIGH);
    digitalWrite(DisplayLED4, HIGH); digitalWrite(DisplayLED5, HIGH); digitalWrite(DisplayLED6, HIGH);
}

/*
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
*/

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

void timeFormat(int tempHH, int tempMM)
{
  
  DateTime now = rtc.now();
  sdLZ(tempHH);
  file.print(F(":"));
  sdLZ(tempMM); //file.print(tempMM);  
  if (now.hour() >= 12){
    file.print(F(" PM"));
  }
  else {
    file.print(F(" AM"));
  }
}

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
    // open ShowerData.csv and append new data
  //char name[] = "Shower00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    name[6] = i/10 + '0';
    name[7] = i%10 + '0';
    // O_CREAT - create the file if it does not exist
    // O_EXCL - fail if the file exists
    // O_WRITE - open for write
    if (file.open(name, O_CREAT | O_APPEND | O_WRITE)) break;
  }
  if (!file.isOpen()) {
    error("file.open");
    Serial.println("Error opening SD file");
    return;
    Serial.print("Returning");
  }
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
  
  for (int i=0; i<6; i++){
    file.print("Time On");comma();
    file.print("Time Off");comma();
    file.print("Duration");comma2();
  }
  file.println();

  //Before Float Shower,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02
  file.print(F("Before-Float Shower"));
  comma();

  SDprintTimes();

  file.close();
  PgmPrintln(" ...Done");
}

void SDlogAfterFloat()
{
    // open ShowerData.csv and append new data
  char name[] = "Shower00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    name[6] = i/10 + '0';
    name[7] = i%10 + '0';
    // O_CREAT - create the file if it does not exist
    // O_EXCL - fail if the file exists
    // O_WRITE - open for write
    if (file.open(name, O_CREAT | O_APPEND | O_WRITE)) break; 
  }
  if (!file.isOpen()) error ("file.open");
  PgmPrint("Writing to: ");
  Serial.println(name);

  DateTime now = rtc.now();
  //Before Float Shower,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02
  file.print(F("After-Float Shower"));
  comma();
  
  SDprintTimes();
  file.println();
  
  file.close();
  PgmPrintln("Done with After times");
}


void SDprintTimes()
{
  if (hh1_ON > 0 && hh1_OFF > 0) {
    timeFormat(hh1_ON,mm1_ON);comma();
    timeFormat(hh1_OFF,mm1_OFF);comma();
    file.print(dur1);comma2(); }
  else {
    comma4();
  }

  if (hh2_ON > 0 && hh2_OFF > 0) {
    timeFormat(hh2_ON,mm2_ON);comma();
    timeFormat(hh2_OFF,mm2_OFF);comma();
    file.print(dur2);comma2(); }
  else {
    comma4();
  }

  if (hh3_ON > 0 && hh3_OFF > 0) {
    timeFormat(hh3_ON,mm3_ON);comma();
    timeFormat(hh3_OFF,mm3_OFF);comma();
    file.print(dur3);comma2(); }
  else {
    comma4();
  }

  if (hh4_ON > 0 && hh4_OFF > 0) {
    timeFormat(hh4_ON,mm4_ON);comma();
    timeFormat(hh4_OFF,mm4_OFF);comma();
    file.print(dur4);comma2(); }
  else {
    comma4();
  }

  if (hh5_ON > 0 && hh5_OFF > 0) {
      timeFormat(hh5_ON,mm5_ON);comma();
      timeFormat(hh5_OFF,mm5_OFF);comma();
      file.print(dur5);comma2(); }
    else {
      comma4();
  }

  if (hh6_ON > 0 && hh6_OFF > 0) {
    timeFormat(hh6_ON,mm6_ON);comma();
    timeFormat(hh6_OFF,mm6_OFF);comma();
    file.print(dur6);comma2(); }
  else {
    comma4();
  }
  file.println();
}


void Blink(byte PIN, byte DELAY_MS, byte loops)
{
  for (byte i=0; i<loops; i++)
  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}

void StartUpLightShow()
{
  int delayTime = 70;
  //all off
  digitalWrite(DisplayLED1, HIGH); digitalWrite(DisplayLED2, HIGH); digitalWrite(DisplayLED3, HIGH);
  digitalWrite(DisplayLED4, HIGH); digitalWrite(DisplayLED5, HIGH); digitalWrite(DisplayLED6, HIGH);

  //blink all once
  digitalWrite(DisplayLED1, LOW); digitalWrite(DisplayLED2, LOW); digitalWrite(DisplayLED3, LOW);
  digitalWrite(DisplayLED4, LOW); digitalWrite(DisplayLED5, LOW); digitalWrite(DisplayLED6, LOW);
  delay(delayTime*2);
  digitalWrite(DisplayLED1, HIGH); digitalWrite(DisplayLED2, HIGH); digitalWrite(DisplayLED3, HIGH);
  digitalWrite(DisplayLED4, HIGH); digitalWrite(DisplayLED5, HIGH); digitalWrite(DisplayLED6, HIGH);
  
  //cycle each on then off in increasing order
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
  
}


