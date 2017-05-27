#include <RHReliableDatagram.h>
#include <RH_RF69.h>
#include <SPI.h>

/************ Radio Setup ***************/
#define MY_ADDRESS     5
#define DEST_ADDRESS   10

// Feather 32u4 w/Radio
#define RFM69_CS      8
#define RFM69_INT     7
#define RFM69_RST     4
#define LED           13
#define RF69_FREQ 915.0

// Setup the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);



//*********************************************************************************************
#define VBATPIN       A9 //battery voltage

int sensorPin = 12;   // push button connected to digital pin 12
int LEDsensorPin = 2; // LED connected to digital pin 2, Lights up when circuit closed
bool currentState = 0;
bool previousState = 0;
bool stateChange = 0;

int16_t packetnum = 0;  // packet counter, we increment per xmission

//*********************************************************************************************

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("Feather RFM69HCW Transmitter");
  
  pinMode(sensorPin, INPUT); // sets the digital pin 12 as input reading for sensor switch
  pinMode(LEDsensorPin, OUTPUT); // sets the digital pin 3 as output led for sensor reading
  pinMode(LED, OUTPUT);
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);
  
  // Hard Reset (and initialize) the RFM module
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

  rf69_manager.setTimeout(2000);
  rf69_manager.setRetries(4);
  Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");  
}

// Dont put this on the stack:     [what does this mean? and why is it after setup?]
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
uint8_t data[] = "  OK";

void loop() {
  delay(500);  // Wait 1 second between transmits, could also 'sleep' here!

  int sensorValue = digitalRead(sensorPin);   // read the sensor input pin and store as sensorValue
  digitalWrite(LEDsensorPin,sensorValue);   //for actual usage, change to !sensorValue
  
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage

  char radiopacket[5];
  if (sensorValue == 1) {  //1 for testing, 0 for production
      strncpy(radiopacket, "ON  ", sizeof(radiopacket));
      currentState = 1; }
  else {
      strncpy(radiopacket, "OFF " , sizeof(radiopacket));
      currentState = 0;
  }

  if (currentState != previousState) {
    stateChange = 1; }
  else {
    stateChange = 0;
  }
  
  //Serial.print("VBat: " ); Serial.println(measuredvbat);
  //   if (measuredvbat < 4.18) {
  //      strncat(radiopacket, " LowBattery", 11);
  //   }
  
  if (stateChange){
    Serial.print("----Sending "); Serial.print("[");Serial.print(MY_ADDRESS);  Serial.print("] ");Serial.println(radiopacket);

    // Send a message to the DESTINATION!
    if (rf69_manager.sendtoWait((uint8_t *)radiopacket, strlen(radiopacket), DEST_ADDRESS)) {
      // Now wait for a reply from the server
      uint8_t len = sizeof(buf);
      uint8_t from;   
      if (rf69_manager.recvfromAckTimeout(buf, &len, 2000, &from)) {
        buf[len] = 0; // zero out remaining string
        Serial.print("Got reply from #"); Serial.print(from); Serial.print(" : ");
        //Serial.print(" [RSSI :"); Serial.print(rf69.lastRssi()); Serial.print("] : ");
        Serial.println((char*)buf);     
        Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks
      } else {
        Serial.println("No reply, is anyone listening?");
      }
    } else {
      Serial.println("Sending failed (no ack)");
    }

  }
   
    previousState = currentState;
}

void Blink(byte PIN, byte DELAY_MS, byte loops) {
  for (byte i=0; i<loops; i++)  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}
