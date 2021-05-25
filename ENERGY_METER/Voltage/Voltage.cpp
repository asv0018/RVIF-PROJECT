#include "Arduino.h"
#include "Voltage.h"

Voltage::Voltage(byte pin){
  _pin = pin;
}

int Voltage::getvoltage(){
  return analogRead(_pin);
}
