#ifndef Voltage_h
#define Voltage_h
#include "Arduino.h"

class Voltage{
public:
  Voltage(byte pin);
  int getvoltage();
  int getoffset();
private:
  byte _pin;
  int _offset;
};
#endif
