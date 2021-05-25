#define GPIO_RX 5
#define GPIO_TX 3
#include <SoftwareSerial.h>
SoftwareSerial iot(GPIO_RX,GPIO_TX);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  iot.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(7000);
  String xxx = "{\"v\":\"" + String(11) + "\",\"a\":\"" + String(22) + "\",\"r\":\"" + String(44) + "\",\"f\":\"" + String(55) + "\",\"w\":\""+String(33)+"\"}\n";
  Serial.print("I TYPED ");
  Serial.println(xxx);
  iot.println(xxx);
  if(iot.available()){
    Serial.println("What you typed was "+ xxx);
    Serial.println(iot.readString());
  }
}
