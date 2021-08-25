#include <Arduino.h>

class clockType
{
  public:
    clockType();
    void setTimeON(int, int, int32_t);
    void setTimeOFF(int, int, int32_t);
    void printTimeON() const;
    void printTimeOFF() const;
    bool onExists() const;
    bool offExists() const;
    
    int hh_on;
    int mm_on;
    bool ampm_on;
    int hh_off;
    int mm_off;
    bool ampm_off;
    int dur;
    int32_t unixxtime_on;
    int32_t unixxtime_off;
};
