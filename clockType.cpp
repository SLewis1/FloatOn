#include <Arduino.h>
#include <clockType.h>


clockType::clockType()
{
	hh_on = 0;
	mm_on = 0;
	ampm_on = 0;
	unixxtime_on = 0;

     	hh_off = 0;
	mm_off = 0;
	ampm_off = 0;
	unixxtime_off = 0;

    	dur = 0;

	
}

void clockType::setTimeON(int hours24, int minutes, int32_t unixxtime)
{
    mm_on = minutes;

    if (hours24 == 0) {
      hh_on = 12; 
      ampm_on = 0;
    }
    else {
      if (hours24 > 12) {
        hh_on = hours24-12; 
        ampm_on = 1;}
      else {
        hh_on = hours24; 
        ampm_on = 0;} 
     }
     
     unixxtime_on = unixxtime;
     
}

void clockType::setTimeOFF(int hours24, int minutes, int32_t unixxtime)
{
    mm_off = minutes;

    if (hours24 == 0) {
      hh_off = 12; 
      ampm_off = 0;
    }
    else {
      if (hours24 > 12) {
        hh_off = hours24-12; 
        ampm_off = 1;}
      else {
        hh_off = hours24; 
        ampm_off = 0;} 
     }
 
     dur = mm_off - mm_on;
     if (dur < 0) dur+=60;

     unixxtime_off = unixxtime;
}





void clockType::printTimeON() const
{
  Serial.print(hh_on); Serial.print(":");
  
  if (mm_on < 10) Serial.print(0);
  Serial.print(mm_on);
	
  if (ampm_on == 0) {
     Serial.print("AM");
  }
  else {
     Serial.print("PM");
  }
  Serial.print(" unixtime = "); Serial.println(unixxtime_on);
}


void clockType::printTimeOFF() const
{
  Serial.print(hh_off); Serial.print(":");

  if (mm_off < 10) Serial.print(0);
  Serial.print(mm_off);
	
  if (ampm_off == 0) {
     Serial.print("AM");
  }
  else {
     Serial.print("PM");
  }
   Serial.print(" unixtime = "); Serial.print(unixxtime_off);

  Serial.print("  dur: ");
  Serial.print(dur);
  Serial.println("m");

}

bool clockType::onExists() const
{
  if (hh_on > 0) 
  {
    return true;
  }
  else
  {
    return false;
  } 
}

bool clockType::offExists() const
{ 
  if (hh_off > 0) 
  {
    return true;
  }
  else
  {
    return false;
  } 

  
}



/*
void clockType::displayTime() const
{
  if (hh < 10) display.print("0");
  display.print(hh); display.print(":");

  if (mm < 10) display.print("0");
  display.print(mm);
}


void clockType::displayTimeAMPM() const
{
  if (hh < 10) display.print("0");
  display.print(hh); display.print(":");

  if (mm < 10) display.print("0");
  display.print(mm);
	
  if (ampm == 0) {
     display.print(" AM");
  }
  else {
     display.print(" PM");
  }
}
*/
