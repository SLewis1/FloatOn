#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/io.h>

#define interruptPin  1
//#define debug
int counter = 0;

void setup() {
  pinMode(interruptPin, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  digitalWrite(13,HIGH);   //turn onboard LED on
  Serial.begin(9600);
  delay(1500);
  Serial.print("Testing sleep mode\n");
  Serial.print("------------------\n\n");
  //Serial.print("interrupt #: "); Serial.println(digitalPinToInterrupt(interruptPin));
  
}

void loop() {
  // Stay awake for 1 second, then sleep.
  // LED turns off when sleeping, then back on upon wake.
  delay(1000);
  Serial.print("Count #"); Serial.println(counter);
  Serial.print("Int pin reading: "); Serial.println(digitalRead(interruptPin));
  
  if(digitalRead(interruptPin) == 1)
  {  
    Serial.println("Good night...");
    sleepNow();
    delay(2000);
    Serial.println("Wake up!\n");
  } 
  ++counter;
}

void sleepNow(void)
{
        #ifdef debug
        Serial.println("enter sleepNow()");
        #endif
    
    //1 Set pin as interrupt and attach handler:
    attachInterrupt(digitalPinToInterrupt(interruptPin), pinInterrupt, LOW);
    delay(100);
    
        #ifdef debug
        Serial.println("  attachInterrupt");
        #endif
    
    //2 Choose our preferred sleep mode:
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

        #ifdef debug
        Serial.println("  set sleep mode");
        #endif
    
    //3 Set sleep enable (SE) bit:
    sleep_enable();

        #ifdef debug
        Serial.println("  sleep enable bit");
        #endif

    //4 Put the device to sleep:
    digitalWrite(13,LOW); //turn off LED to indicate sleep
    sleep_mode();

        #ifdef debug
        Serial.println("  sleep_mode()");
        #endif
        
    delay(1000);
 
    //5 Upon waking up, sketch continues from this point.
    sleep_disable();

        #ifdef debug
        Serial.println("  sleep disable bit");
        #endif
    
    digitalWrite(13,HIGH); //turn on LED to indicade awake
    delay(2000);
    
}

void pinInterrupt(void)  //ISR Interrupt Service Routine
{ 
  detachInterrupt(digitalPinToInterrupt(interruptPin)); 
}

