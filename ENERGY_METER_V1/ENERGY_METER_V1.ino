#define CHIP_SELECT 10
#define SWITCH_IN A3
#define CURRENT_SENSOR_PIN A1
#define VOLTAGE_SENSOR_PIN A0
#define GPIO_NETWORK_STATUS_IN_CHECK 8
#define GPIO_RX 5
#define GPIO_TX 3
#define GPIO_RPM_SENSOR 2
#define GPIO_SD_EXIST 6
#define GPIO_PROCESS_STATUS 7

#define VOLTAGE_OFFSET 35
#define CURRENT_OFFSET 10

#include <SoftwareSerial.h>
#include <DS3231.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>


RTClib RTC;
DateTime now;
File file;

bool is_card_present= false;
bool is_connected_iot = true;

SoftwareSerial iot(GPIO_RX,GPIO_TX);
String filename = "datalog.csv";

float revolutions = 0;
int rpm;
int rpm_previous_time = 0;
int rpm_time;


void setup(){
  pinMode(GPIO_RPM_SENSOR,OUTPUT);
  pinMode(GPIO_SD_EXIST,OUTPUT);
  pinMode(GPIO_PROCESS_STATUS,OUTPUT);
  pinMode(GPIO_NETWORK_STATUS_IN_CHECK,INPUT);
  pinMode(SWITCH_IN,INPUT);

  Serial.begin(9600);
  iot.begin(115200);

  pinMode(CHIP_SELECT, OUTPUT);
  pinMode(CHIP_SELECT, HIGH);

  delay(40);

  if(!SD.begin(4)){
    Serial.println(F("CARD FAILED"));
  }else{
    is_card_present = true;
    Serial.println("CARD INISIALISED");
    if(!SD.exists(filename)){
    Serial.println("File doesnt exist, creating new");
    // IF THE FILENAME DOESN'T EXIST, THEN CREATE THE FILE AND PROVIDE THE HEADING
    file = SD.open(filename, FILE_WRITE);
    delay(25);
    if(!file){
      Serial.print("Error opening");
      Serial.println(filename);
    }else{
    String temp = "DATE TIME STAMP, VOLTAGE ADC, CURRENT ADC";
    file.println(temp);
    delay(10);
    file.close();
    delay(10);
  }
  }
  }

  }

unsigned long previousMillis = 0;
bool is_delay_execution_successful = true;

void loop(){
  bool temp_switch_state = digitalRead(SWITCH_IN);
  Serial.println("The state was "+String(temp_switch_state));
  while(temp_switch_state){
    temp_switch_state = digitalRead(SWITCH_IN);
    unsigned long temp_timer_delay = 6000;
    subfunctions();
    float volt_temp = measureAdc(VOLTAGE_SENSOR_PIN, VOLTAGE_OFFSET);
    float current_temp = measureAdc(CURRENT_SENSOR_PIN, CURRENT_OFFSET);
    float power_temp = 0;
    Serial.println("Voltage: "+String(volt_temp)+" Current:"+String(current_temp)+" power:"+String(power_temp));

    if(is_delay_execution_successful){
      previousMillis = millis();
      is_delay_execution_successful = false;
      timer_delay(temp_timer_delay);
      save_data(volt_temp, current_temp, power_temp);
      }

    if(timer_delay(temp_timer_delay)){
      is_delay_execution_successful = true;
    }else{
      is_delay_execution_successful = false;
    }

  }
  subfunctions();
}
void testing_loop(){
  String dateime = "";
  float voltage = measureAdc(VOLTAGE_SENSOR_PIN, VOLTAGE_OFFSET);
  float current = measureAdc(CURRENT_SENSOR_PIN, CURRENT_OFFSET);
  float power = 0;
  dateime = getprintdatetime();
  Serial.println(dateime);

  Serial.println("*********************************");
  delay(1000);
}
void subfunctions(){
  if(digitalRead(GPIO_NETWORK_STATUS_IN_CHECK)){
    is_connected_iot = true;
  }else{
    is_connected_iot = false;
  }
  if(is_card_present){
    digitalWrite(GPIO_SD_EXIST,HIGH);
  }else{
    digitalWrite(GPIO_SD_EXIST,LOW);
  }
}

bool timer_delay(unsigned long delay_time){
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= delay_time){
    previousMillis = currentMillis;
    return true;
  }
  return false;
}

int measureAdc(byte adc_channel, byte offset_number){
  int adc;
  int maxvolt = 0;
  int minvolt = 1024;
  for(int i=0;i<100;i++){
    adc = analogRead(adc_channel);
    if(maxvolt < adc){
      maxvolt = adc;
      delayMicroseconds(200);
    }
    if(minvolt > adc){
      minvolt = adc;
      delayMicroseconds(200);
    }
  }
  adc = (maxvolt-minvolt);
  return (adc > offset_number ? adc : 0);
}

void getdatetime(){
  now  = RTC.now();
}

int getyear(){
  return int(now.year());
}

int getmonth(){
  return int(now.month());
}

int getdate(){
  return int(now.day());
}
int gethour(){
  return int(now.hour());
}
int getminute(){
  return int(now.minute());
}
int getsecond(){
  return int(now.second());
}

void save_data(float voltage, float current, float power){
  String datetime_str = getprintdatetime();
  updatecsv(datetime_str, String(voltage), String(current), String(power));
}

void updatecsv(String datetime, String voltage, String current, String power){
  File file;
  if(is_card_present){
    digitalWrite(GPIO_PROCESS_STATUS,HIGH);
    file = SD.open(filename, FILE_WRITE);
    delay(25);
    if(!file){
      Serial.print("Error opening ");
      Serial.println(filename);
    }else{
    String temp = datetime+", "+voltage+", "+current;
    file.println(temp);
    delay(15);
    file.close();
    delay(10);
  }
  digitalWrite(GPIO_PROCESS_STATUS,LOW);
}
is_connected_iot = true;
if(is_connected_iot){
  digitalWrite(GPIO_PROCESS_STATUS,HIGH);
  String temp = "{\"v\":\"" + voltage + "\",\"a\":\"" + current + "\",\"r\":\"" + String(44) + "\",\"f\":\"" + String(55) + "\",\"w\":\""+String(33)+"\"}\n";
  Serial.println("UPDATE TO IOT");
  iot.println(temp);
  digitalWrite(GPIO_PROCESS_STATUS,LOW);
}
}

String getprintdatetime(){
  getdatetime();
  String hour = String(gethour());
  String minute = String(getminute());
  String seconds = String(getsecond());
  String date = String(getdate());
  String month = String(getmonth());
  String year = String(getyear());
  String datetime = date+"/"+month+"/"+year+" "+hour+":"+minute+":"+seconds;
  return datetime;
}

void old_loop(){
  bool temp_switch_state = digitalRead(SWITCH_IN);
  Serial.println("The state was "+String(temp_switch_state));
  while(temp_switch_state){
    temp_switch_state = digitalRead(SWITCH_IN);
    unsigned long temp_timer_delay = 6000;
    subfunctions();
    float volt_temp = measureAdc(VOLTAGE_SENSOR_PIN, VOLTAGE_OFFSET);
    float current_temp = measureAdc(CURRENT_SENSOR_PIN, CURRENT_OFFSET);
    float power_temp = 0;
    Serial.println("Voltage: "+String(volt_temp)+" Current:"+String(current_temp)+" power:"+String(power_temp));

    if(is_delay_execution_successful){
      previousMillis = millis();
      is_delay_execution_successful = false;
      timer_delay(temp_timer_delay);
      save_data(volt_temp, current_temp, power_temp);
      }

    if(timer_delay(temp_timer_delay)){
      is_delay_execution_successful = true;
    }else{
      is_delay_execution_successful = false;
    }

  }
  subfunctions();
}

void isr(){
  revolutions++;
}
