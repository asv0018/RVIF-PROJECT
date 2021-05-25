#include <SoftwareSerial.h>

SoftwareSerial mainController(3,4);
float revolutions = 0;
int rpm;
int rpm_previous_time = 0;
int rpm_time;


void isr(){
  revolutions++;
}

void setup(){
  Serial.begin(9600);
  mainController.begin(9600);
  attachInterrupt(digitalPinToInterrupt(2),isr,RISING);
}

void loop(){
  detachInterrupt(digitalPinToInterrupt(2));
  rpm_time = millis()-rpm_previous_time;
  rpm=(revolutions/rpm_time)*60000;
  rpm_previous_time = millis();
  revolutions = 0;
  attachInterrupt(digitalPinToInterrupt(2),isr,RISING);
  Serial.println(rpm);
  mainController.println(rpm);
  delay(1000);
}
