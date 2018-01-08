#include <RHReliableDatagram.h>
#include <RH_RF69.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/io.h>

#define NewSleepMode


/************ Radio Setup ***************/
#define MY_ADDRESS     5
#define DEST_ADDRESS   10

// Feather 32u4 w/Radio
#define RFM69_CS      8
#define RFM69_INT     7
#define RFM69_RST     4
#define LED_OB        13
#define RF69_FREQ     915.0

// Setup the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);



//*********************************************************************************************
#define VBATPIN       A9 //battery voltage

#define interruptPin  1
#define ledPin  12 
int LEDsensorPin = A1; // LED connected to digital pin 2, Lights up when circuit closed

//*********************************************************************************************

void setup() {
  //Serial.begin(115200);
  //delay(1500);
  //Serial.println("Feather RFM69HCW Transmitter");
  
  pinMode(interruptPin, INPUT_PULLUP); // sets the digital pin 12 as input reading for sensor switch
  pinMode(ledPin, OUTPUT);
  
  pinMode(LEDsensorPin, OUTPUT); // sets the digital pin 3 as output led for sensor reading
  pinMode(LED_OB, OUTPUT);
  digitalWrite(LED_OB,LOW); //turn off onboard LED
  
  pinMode(RFM69_RST, OUTPUT);
  
  // Hard Reset (and initialize) the RFM module
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!rf69_manager.init()) {
    //Serial.println("RFM69 radio init failed");
    while (1);
  }
  //Serial.println("RFM69 radio init OK!");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    //Serial.println("setFrequency failed");
  }

  rf69.setTxPower(14, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);

  rf69_manager.setTimeout(2000);
  rf69_manager.setRetries(4);
  //Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");  
}

// Dont put this on the stack:    
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
uint8_t data[] = "  OK";

void loop() {
  delay(1000);
  
  int sensorValue = digitalRead(interruptPin);   // read the sensor input pin and store as sensorValue
  digitalWrite(LEDsensorPin,!sensorValue);   //for actual usage, change to !sensorValue
  
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage

  char radiopacket[5];
  if (sensorValue == 1)    //open circuit
  {  
      strncpy(radiopacket, "ON  ", sizeof(radiopacket));
  }
  else 
  {
      strncpy(radiopacket, "OFF " , sizeof(radiopacket));
  }
  Serial.print("measured battery: "); Serial.println(measuredvbat);
  
  
  //Battery Voltage Transmit
  /*
  char charVbat[5];                
  //1 is mininum width, 2 is precision; float value is copied onto buff
  dtostrf(measuredvbat, 1, 2, charVbat);
  strncat(radiopacket, charVbat, 4);
  strncat(radiopacket, "  ", 2);
`*/

   if (measuredvbat < 3.5) {
      strncat(radiopacket, "LBat ", 5);
      //Serial.print("VBat: " ); Serial.println(measuredvbat);
      //Blink(LED_OB, 1000, 5); //blink LED 3 times, 40ms between blinks
   }
   
 
 //RADIO TX
    //Serial.print("----Sending "); Serial.print("[");Serial.print(MY_ADDRESS);  Serial.print("] ");Serial.println(radiopacket);
    // Send a message to the DESTINATION!
    if (rf69_manager.sendtoWait((uint8_t *)radiopacket, strlen(radiopacket), DEST_ADDRESS)) {
      // Now wait for a reply from the server
      uint8_t len = sizeof(buf);
      uint8_t from;   
      if (rf69_manager.recvfromAckTimeout(buf, &len, 2000, &from)) {
        buf[len] = 0; // zero out remaining string
        //Serial.print("Got reply from #"); Serial.print(from); Serial.print(" : ");
        //Serial.print(" [RSSI :"); Serial.print(rf69.lastRssi()); Serial.print("] : ");
        //Serial.println((char*)buf);     
        Blink(13, 80, 2); //blink LED 3 times, 40ms between blinks
      }
      //else {
        //Serial.println("No reply, is anyone listening?");
        //Blink(12, 1000, 2);
      //}
    } 
    //else {
      //Serial.println("Sending failed (no ack)");
      //Blink(LED_OB, 400, 3);
    //}

  sleepNow();
   
}


#ifdef NewSleepMode
void sleepNow(void)
{
    rf69.sleep();
    
    EIFR = (1 << INTF3);   //use before attachInterrupt(3,isr,xxxx) to clear interrupt 3 flag
    
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    noInterrupts();
    
    sleep_enable();

    attachInterrupt(digitalPinToInterrupt(interruptPin), pinInterrupt, CHANGE);
    
    interrupts();

    //digitalWrite(13,HIGH); //turn on ob_LED to indicate sleep
    sleep_mode();
    
    //digitalWrite(13,LOW); //turn off ob_LED to indicade awake
}

void pinInterrupt(void)  //ISR Interrupt Service Routine
{ 
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(interruptPin)); 
}
#endif //NewSleepMode


void Blink(byte PIN, byte DELAY_MS, byte loops) {
  for (byte i=0; i<loops; i++)  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}



#ifndef NewSleepMode
void sleepNow(void)
{
    rf69.sleep();
    
    EIFR = (1 << INTF3);   //use before attachInterrupt(3,isr,xxxx) to clear interrupt 3 flag
    
    //1 Set pin as interrupt and attach handler:
    attachInterrupt(digitalPinToInterrupt(interruptPin), pinInterrupt, CHANGE);
    delay(100);

    //2 Choose our preferred sleep mode:
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    //3 Set sleep enable (SE) bit:
    sleep_enable();

    //4 Put the device to sleep:
    digitalWrite(13,HIGH); //turn on LED to indicate sleep
    sleep_mode();
 
    //5 Upon waking up, sketch continues from this point.
    sleep_disable();
    
    digitalWrite(13,LOW); //turn on LED to indicade awake
}

void pinInterrupt(void)  //ISR Interrupt Service Routine
{ 
  detachInterrupt(digitalPinToInterrupt(interruptPin)); 
}
#endif //OldSleepMode
