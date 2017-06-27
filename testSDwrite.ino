#include <Fat16.h>
#include <Fat16util.h>
#include <RTClib.h>
RTC_PCF8523 rtc;
 
//SD
  const uint8_t CHIP_SELECT = 10;  // SD chip select pin.
  SdCard card;
  Fat16 file;
  #define error(s) error_P(PSTR(s)) // store error strings in flash to save RAM
  void error_P(const char* str) {
    PgmPrint("error: ");
    SerialPrintln_P(str);
    if (card.errorCode) {
      PgmPrint("SD error: ");
      Serial.println(card.errorCode, HEX);
    }
    while(1);
    } /*
 * Write an unsigned number to file.
 * Normally you would use print to format numbers.
 */
  void writeNumber(uint32_t n) {
    uint8_t buf[10];
    uint8_t i = 0;
    do {
      i++;
      buf[sizeof(buf) - i] = n%10 + '0';
      n /= 10;
    } while (n);
    file.write(&buf[sizeof(buf) - i], i); // write the part of buf with the number
  }

int hh1_ON=6;  int hh1_OFF=6; int mm1_ON=1;  int mm1_OFF=11; int dur1=5;
int hh2_ON=6;  int hh2_OFF=6; int mm2_ON=2;  int mm2_OFF=12; int dur2=6;
int hh3_ON=0;  int hh3_OFF=6; int mm3_ON=3;  int mm3_OFF=13; int dur3=5;
int hh4_ON=6;  int hh4_OFF=6; int mm4_ON=4;  int mm4_OFF=14; int dur4=6;
int hh5_ON=0;  int hh5_OFF=6; int mm5_ON=5;  int mm5_OFF=15; int dur5=5;
int hh6_ON=6;  int hh6_OFF=6; int mm6_ON=6;  int mm6_OFF=16; int dur6=6;


void comma(); void comma2(); void comma4(); void timeFormat(); void sdLZ();
void SDlogBeforeFloat();
void setup() {
Serial.begin(9600);
delay(1500);
Serial.println("SD log test");

  // initialize the SD card
  if (!card.begin(CHIP_SELECT)) error("card.begin");
  
  // initialize a FAT16 volume
  if (!Fat16::init(&card)) error("Fat16::init");

  SDlogBeforeFloat();

  SDlogAfterFloat();

}

void loop() {


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

void timeFormat(int tempHH, int tempMM)
{
  DateTime now = rtc.now();
  file.print(tempHH);
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
  char name[] = "Shower00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    name[6] = i/10 + '0';
    name[7] = i%10 + '0';
    // O_CREAT - create the file if it does not exist
    // O_EXCL - fail if the file exists
    // O_WRITE - open for write
    if (file.open(name, O_CREAT | O_APPEND | O_WRITE)) break;  // O_EXCL | 
  }
  if (!file.isOpen()) error ("file.open");
  PgmPrint("Writing to: ");
  Serial.println(name);

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
  file.print("7AM - 8:30PM");comma();
  for (int i=0; i<6; i++){
    file.print("Time On");comma();
    file.print("Time Off");comma();
    file.print("Duration");comma2();
  }
  file.println();

  //Before Float Shower,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02
  file.print(F("Before-Float Shower"));
  comma();
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

  file.close();
  PgmPrintln("Done with Before times");
}






//************************** AFTER *******

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
    if (file.open(name, O_CREAT | O_APPEND | O_WRITE)) break;  // O_EXCL | 
  }
  if (!file.isOpen()) error ("file.open");
  PgmPrint("Writing to: ");
  Serial.println(name);

  DateTime now = rtc.now();
  //Before Float Shower,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02,,03:01 PM,03:03 PM,00:02
  file.print(F("After-Float Shower"));
  comma();
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
  file.println();



  file.close();
  PgmPrintln("Done with After times");
}

