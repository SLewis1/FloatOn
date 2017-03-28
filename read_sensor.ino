int sensorPin = 12;   // push button connected to digital pin 12
int LEDsensorPin = 2; // LED connected to digital pin 2


void setup()
{
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT); // sets the digital pin 3 as output
  pinMode(sensorPin, INPUT); // sets the digital pin 12 as input
  //pinMode(sensorPin, INPUT_PULLUP); // sets the digital pin 12 as input with internal pullup resistor enabled
  // sensor reading is inverted so that the closed circuit sensor reads as low and open circuit reads high
}


void loop()
{
  
  
  int sensorValue = digitalRead(sensorPin);   // read the sensor input pin and store as sensorValue
  digitalWrite(LEDsensorPin,sensorValue);
 
  if (sensorValue == 0)
    {
      Serial.print('H'); 
      delay(50);
    }
  else
    {
      Serial.print('L');
      delay(50); 
    }
    
}
