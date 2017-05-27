#include <SPI.h>
//#include <Wire.h>  What is Wire.h used for?
#include <RH_RF69.h>
#include <RHReliableDatagram.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1325.h>
#include <RTClib.h>

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

//Temp button for lights demo
  #define LightDemoButton      12

//Serial
  #define SERIAL_BAUD   115200


int hh; int mm; int ss;
int hh1On=0;  int hh1Off; int mm1On;  int mm1Off; int dur1;
int hh2On=0;  int hh2Off; int mm2On;  int mm2Off; int dur2;
int hh3On=0;  int hh3Off; int mm3On;  int mm3Off; int dur3;
int hh4On=0;  int hh4Off; int mm4On;  int mm4Off; int dur4;
int hh5On=0;  int hh5Off; int mm5On;  int mm5Off; int dur5;
int hh6On=0;  int hh6Off; int mm6On;  int mm6Off; int dur6;

//int incomingByte;      // a variable to read incoming serial data into
//int16_t packetnum = 0;  // packet counter, increment per xmission

//*****************************************************************************

void setup() {
 
  Serial.begin(SERIAL_BAUD);
  delay(1000);
  Serial.println("Feather RFM69HCW Receiver");

  pinMode(LED, OUTPUT);
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);
  
  // Reset (and initialize) the RFM module
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!rf69_manager.init()) {
    Serial.println("RFM69 radio init failed");
    while (1);
  }
  Serial.println("RFM69 radio init OK!");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);

  
  pinMode(LightDemoButton, INPUT);
  pinMode(DisplayLED1, OUTPUT);
  pinMode(DisplayLED2, OUTPUT);
  pinMode(DisplayLED3, OUTPUT);
  pinMode(DisplayLED4, OUTPUT);
  pinMode(DisplayLED5, OUTPUT);
  pinMode(DisplayLED6, OUTPUT);

  
  display.setTextColor(1);
  display.setTextSize(1);
  display.setTextWrap(false);
  display.begin(); 
  display.clearDisplay();
  display.display();

  Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
  
  StartUpLightShow();
  Serial.println("End of Setup()");
}

// Dont put this on the stack:
uint8_t data[] = "And hello back to you";
// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
uint8_t from;
uint8_t NodeID; //stores 'from' ID for use after the recieve response erases from
               
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
     ss = now.second();
Serial.println("adjust time variables");      


  if (rf69_manager.available()) {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from; NodeID = from;
    Serial.println("Message available");
    
    if (rf69_manager.recvfromAck(buf, &len, &from)) {
      buf[len] = 0; // zero out remaining string
      
      Serial.print("Got packet from #"); Serial.print(from);
      //Serial.print(" [RSSI :");
      //Serial.print(rf69.lastRssi());
      Serial.print(" : ");
      Serial.println((char*)buf);
      Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks

      // Send a reply back to the originator client
      if (!rf69_manager.sendtoWait(data, sizeof(data), from)){
        Serial.println("Sending failed (no ack)");}
    }
  
    //check if received message contains ON, if yes turn on Front LED # corresponding to the Sender ID
      if (strstr((char*)buf, "ON")) {
        switch (NodeID) {
          case 1: digitalWrite(DisplayLED1, LOW);
            hh1On = hh; mm1On = now.minute(); break;
          case 2: digitalWrite(DisplayLED2, LOW);
            hh2On = hh; mm2On = now.minute(); break;
          case 3: digitalWrite(DisplayLED3, LOW); 
            hh3On = hh; mm3On = now.minute(); break;
          case 4: digitalWrite(DisplayLED4, LOW);
            hh4On = hh; mm4On = now.minute(); break;
          case 5: digitalWrite(DisplayLED5, LOW); 
            hh5On = hh; mm5On = now.minute(); break;
          case 6: digitalWrite(DisplayLED6, LOW); 
            hh6On = hh; mm6On = now.minute(); break;
         }
      }
      if (strstr((char*)buf, "OFF")) {
         switch (NodeID) {
          case 1: digitalWrite(DisplayLED1, HIGH);
            hh1Off = hh; mm1Off = now.minute(); break;
          case 2: digitalWrite(DisplayLED2, HIGH);
            hh2Off = hh; mm2Off = now.minute(); break;
          case 3: digitalWrite(DisplayLED3, HIGH); 
            hh3Off = hh; mm3Off = now.minute(); break;
          case 4: digitalWrite(DisplayLED4, HIGH);
            hh4Off = hh; mm4Off = now.minute(); break;
          case 5: digitalWrite(DisplayLED5, HIGH); 
            hh5Off = hh; mm5Off = now.minute(); break;
          case 6: digitalWrite(DisplayLED6, HIGH); 
            hh6Off = hh; mm6Off = now.minute(); break;
         }  
      }
  
  }
      //******End Radio code***********************      

    display.clearDisplay();
    
   //DISPLAY CLOCK IN UPPER RIGHT CORNER  
      display.setCursor(80,0);
      display.print(hh);
      display.print(":"); 
      displayLZ(mm);
      display.print(":");
      displayLZ(ss);
      //border line
        //display.drawLine(92,0,92,9,1);display.drawLine(92,9,127,9,1);
        //display.drawRect(92,0,36,13,1); 
 

   //DISPLAY SCHEDULED FLOAT TIME IN UPPER LEFT CORNER
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
  
   
   //DISPLAY INDIVIDUAL LINES WITH TIMES AND DURATIONS
   
   dur1 = mm1Off - mm1On;   if (dur1 < 0) dur1 = dur1 + 60;
   dur2 = mm2Off - mm2On;   if (dur2 < 0) dur2 = dur2 + 60;  
   dur3 = mm3Off - mm3On;   if (dur3 < 0) dur3 = dur3 + 60;
   dur4 = mm4Off - mm4On;   if (dur4 < 0) dur4 = dur4 + 60;
   dur5 = mm5Off - mm5On;   if (dur5 < 0) dur5 = dur5 + 60;
   dur6 = mm6Off - mm6On;   if (dur6 < 0) dur6 = dur6 + 60;

    display.setCursor(0,16);
    display.print("1. "); 
      if (hh1On > 0) {
        display.print(hh1On); display.print(":"); displayLZ(mm1On);display.print("-");display.print(hh1Off);display.print(":");displayLZ(mm1Off);
        display.print(" (");display.print(dur1);display.println("m)");
      }
      else {
        display.println();
      }
      
    display.print("2. "); 
     if (hh2On > 0) {
        display.print(hh2On); display.print(":");displayLZ(mm2On);display.print("-");display.print(hh2Off);display.print(":");displayLZ(mm2Off);
        display.print(" (");display.print(dur2);display.println("m)");
     }
     else {
      display.println();
     }
      
    display.print("3. "); 
      if (hh3On > 0) {
        display.print(hh3On); display.print(":");displayLZ(mm3On);display.print("-");display.print(hh3Off);display.print(":");displayLZ(mm3Off);
        display.print(" (");display.print(dur3);display.println("m)"); 
      }
      else {
        display.println();
      }

    display.print("4. "); 
      if (hh4On > 0) {
        display.print(hh4On); display.print(":");displayLZ(mm4On);display.print("-");display.print(hh4Off);display.print(":");displayLZ(mm4Off);
        display.print(" (");display.print(dur4);display.println("m)"); 
      }
      else { 
        display.println();
      }
    
    display.print("5. "); 
      if (hh5On > 0) {
        display.print(hh5On); display.print(":");displayLZ(mm5On);display.print("-");display.print(hh5Off);display.print(":");displayLZ(mm5Off);
        display.print(" (");display.print(dur5);display.println("m)"); 
      }  
      else {
        display.println(); 
      }
    
    display.print("6. "); 
      if (hh6On > 0) {
        display.print(hh6On); display.print(":");displayLZ(mm6On);display.print("-");display.print(hh6Off);display.print(":");displayLZ(mm6Off);
        display.print(" (");display.print(dur6);display.println("m)");
      }
      else {
        display.println();
      }
   

  display.display();
  Serial.println("display printed");
  delay(100);
  
  //button press triggers light show
  if (digitalRead(LightDemoButton) == HIGH) {
    Serial.println("Wipe OUT!");
    hh1On=0;
    hh2On=0;
    hh3On=0;
    hh4On=0;
    hh5On=0;
    hh6On=0;
  }
}

//**************** SUBFUNCTIONS *************************************************************

void displayLZ(int zMin) //adds a leading zero if needed to fill up two spaces
{
    if (zMin < 10) {
      display.print("0");
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
  int delayTime = 100;
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
