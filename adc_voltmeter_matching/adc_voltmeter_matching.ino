float sensorValue1 = 0;

void setup() {
  Serial.begin(9600);
}

void loop(){
  int adc;
  int maxvolt = 0;
  int minvolt = 1024;
  for(int i=0;i<100;i++){
    adc = analogRead(A0);
    if(maxvolt < adc){
      maxvolt = adc;
      delayMicroseconds(200);
    }
    if(minvolt > adc){
      minvolt = adc;
      delayMicroseconds(200);
    }
  }
   Serial.println (maxvolt-minvolt);
}
