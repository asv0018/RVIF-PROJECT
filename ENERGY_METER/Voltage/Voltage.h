#ifndef Voltage_h
#define Voltage_h
#include "Arduino.h"

class Voltage{
public:
  Voltage(byte pin);
  int getvoltage();
private:
  byte _pin;
};
#endif
