#include <SPI.h>
//#include <Wire.h>  //What is Wire.h used for?
#include <RH_RF69.h>
#include <RHReliableDatagram.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1325.h>
#include <RTClib.h>

//Radio
  #define RF69_FREQ     915
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
  Adafruit_SSD1325 display(OLED_DC, OLED_RESET, OLED_CS);
  
//RTC
  RTC_PCF8523 rtc;

//Front LEDs
  #define DisplayLED1          A0
  #define DisplayLED2          A1
  #define DisplayLED3          A2
  #define DisplayLED4          A3
  #define DisplayLED5          A4
  #define DisplayLED6          A5

//Temp Button for resetting the display and clearing stored time values
  #define ActiveButtonInput      12

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

//int incomingByte;      // a variable to read incoming serial data into
//int16_t packetnum = 0;  // packet counter, increment per xmission
//int counter = 0;

//*****************************************************************************

void setup() {
 
  Serial.begin(SERIAL_BAUD);
  delay(1500);
  Serial.println(F("Feather RFM69HCW Receiver"));

  pinMode(ActiveButtonInput, INPUT);
  pinMode(DisplayLED1, OUTPUT);
  pinMode(DisplayLED2, OUTPUT);
  pinMode(DisplayLED3, OUTPUT);
  pinMode(DisplayLED4, OUTPUT);
  pinMode(DisplayLED5, OUTPUT);
  pinMode(DisplayLED6, OUTPUT);


  pinMode(LED, OUTPUT);
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);
  
  // Reset (and initialize) the RFM module
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

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);

  


  
  display.setTextColor(1);
  display.setTextSize(1);
  display.setTextWrap(false);
  display.begin(); 
  display.clearDisplay();
  display.display();

  Serial.print(F("RFM69 radio @"));  Serial.print((int)RF69_FREQ);  Serial.println(F(" MHz"));
  
  StartUpLightShow();
  
}

// Dont put this on the stack:
uint8_t data[] = "And hello back to you";
// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
uint8_t from;
               
//*****************************************************************************

void loop() {
    //Serial.print(F("Iteration #")); Serial.println(counter);      
      
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

    display.clearDisplay();
    //Serial.print(F("Display: Clear..."));
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
      
      
      //display.print(F(":"));
      //displayLZ(ss);
      //border line
        //display.drawLine(92,0,92,9,1);display.drawLine(92,9,127,9,1);
        //display.drawRect(92,0,36,13,1); 
 
     //Serial.print(F("Clock..."));
   //DISPLAY SCHEDULED FLOAT TIME IN UPPER LEFT CORNER
  display.setCursor(0,0);
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
  
  //Serial.print(F("Schedule..."));
   
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
  //*********************************************

//Times that the display clears and resets for next floats                               // 
   
   if ((now.hour()==7  && now.minute()==0 && now.second()>=0 && now.second()<5) ||       //8 am   to  9     (7-8:30)    
       (now.hour()==9  && now.minute()==0 && now.second()>=0 && now.second()<5) ||       //10 am  to 11     (9-10:30)
       (now.hour()==11 && now.minute()==0 && now.second()>=0 && now.second()<5) ||       //12 pm  to 1      (11-12:30) 
       (now.hour()==13 && now.minute()==0 && now.second()>=0 && now.second()<5) ||       //2 pm   to  3     (1-2:30)
       (now.hour()==15 && now.minute()==0 && now.second()>=0 && now.second()<5) ||       //4 pm   to  5     (3-4:30)
       (now.hour()==17 && now.minute()==0 && now.second()>=0 && now.second()<5) ||       //6 pm   to  7     (5-6:30)
       (now.hour()==19 && now.minute()==0 && now.second()>=0 && now.second()<5) ||       //8 pm   to  9     (7-8:30)
       (now.hour()==21 && now.minute()==0 && now.second()>=0 && now.second()<5) ||       //10 pm  to  11    (9-10:30)
       (now.hour()==23 && now.minute()==0 && now.second()>=0 && now.second()<5) ||       //1 am   to  2     (11-1:30)
       (now.hour()==2  && now.minute()==0 && now.second()>=0 && now.second()<5) )        //4 am   to  5     (2-4:30)
   {
    ResetForNextFloat();    
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
  if (tempMinute != now.minute())
  {
    ActiveMinuteCounter = ActiveMinuteCounter + 1;
    tempMinute = now.minute();
  }

Serial.println(ActiveMinuteCounter);

    
    if ((now.hour() >= 7) && (now.hour() <= 24))
    {
      ActiveStartTime = 60;
    }
    else
    {
      ActiveStartTime = 120;
    }
*/
  if (digitalRead(ActiveButtonInput) == HIGH)
  {
    delay(1500);
    ResetForNextFloat();
    ActiveMinuteCounter = ActiveMinuteCounter + 60;
    Serial.println("Wipe out");
    
  }




  //if ((ActiveMinuteCounter >= ActiveStartTime) && (ActiveMinuteCounter <= (ActiveStartTime + TimeLimit)))  // if 60 <= counter <= 120
  if (ActiveDisplayWindow)
  {
     display.setCursor(90,54);
     display.print("Active");
     //Serial.println(F("Listening"));
  }
  else 
  {
     display.setCursor(85,54);
     display.print("Waiting");
     //Serial.println(F("Not Listening"));
  }

  display.display();
  delay(1000);


         

  
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
  }
      //******End Radio code***********************     

}



//**************** SUBFUNCTIONS *************************************************************

void ResetForNextFloat()
{
    hh1_ON=0; hh1_OFF=0;
    hh2_ON=0; hh2_OFF=0;
    hh3_ON=0; hh3_OFF=0;
    hh4_ON=0; hh4_OFF=0;
    hh5_ON=0; hh5_OFF=0;
    hh6_ON=0; hh6_OFF=0; 
    ActiveMinuteCounter = 0;
    digitalWrite(DisplayLED1, HIGH); digitalWrite(DisplayLED2, HIGH); digitalWrite(DisplayLED3, HIGH);
    digitalWrite(DisplayLED4, HIGH); digitalWrite(DisplayLED5, HIGH); digitalWrite(DisplayLED6, HIGH);
}


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
  int delayTime = 200;
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
