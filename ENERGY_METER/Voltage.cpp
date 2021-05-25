#include "Arduino.h"
#include "Voltage.h"

Voltage::Voltage(byte pin){
  _pin = pin;
  _offset = 0;
}

int Voltage::getoffset(){
  byte i=0;
  int temp_sum = 0;
  for(;i<100;i++){
    temp_sum += analogRead(_pin);
    delay(9);
  }
  _offset = temp_sum/i;

}

int Voltage::getvoltage(){
  int i=0;
  int adc;
  int maxvolt = 0;
  for(;i<100;i++){
    adc = analogRead(_pin);
    if(maxvolt < adc){
      maxvolt = adc;
      delay(9);
    }
  }
  int adcvp = maxvolt - _offset;
  int adcvpp = (adcvp)*2;
  float measured_voltage = float(adcvpp)*5/1024;
  float mapped_voltage = measured_voltage*250;
  float voltage_rms = mapped_voltage/sqrt(2);
  return voltage_rms;

}
