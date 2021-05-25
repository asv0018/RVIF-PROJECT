void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop(){
  // put your main code here, to run repeatedly:
  int adc;
  int maxvolt = 0;
  int minvolt = 1024;
  for(int i=0;i<100;i++){
    adc = analogRead(A1);
    if(maxvolt < adc){
      maxvolt = adc;
      delayMicroseconds(200);
    }
    if(minvolt > adc){
      minvolt = adc;
      delayMicroseconds(200);
    }
  }
  Serial.println(maxvolt-minvolt);
  delay(1000);
}
