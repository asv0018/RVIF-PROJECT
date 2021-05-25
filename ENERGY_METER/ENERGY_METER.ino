#define SELECT_SD 10

#define GPIO_CURRENT_IN A1
#define GPIO_VOLTAGE_IN A0
#define GPIO_NETWORK_STATUS_IN_CHECK 8

#define GPIO_NETWORK_STATE 5
#define GPIO_SD_EXIST 6
#define GPIO_PROCESS_STATUS 7

#include <SoftwareSerial.h>
#include <DS3231.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

RTClib RTC;
DateTime now;
bool is_card_present= false;
bool is_connected_iot = false;
SoftwareSerial iot(2,3);
String filename = "datalog.csv";

void setup() {
  Serial.begin(9600);
  iot.begin(115200);
  pinMode(SELECT_SD, OUTPUT);
  pinMode(SELECT_SD, HIGH);
  delay(40);
  if(!SD.begin(4)){
    Serial.println(F("CARD FAILED"));
  }else{
    is_card_present = true;
    Serial.println("CARD INISIALISED");
  }
  pinMode(GPIO_NETWORK_STATE,OUTPUT);
  pinMode(GPIO_SD_EXIST,OUTPUT);
  pinMode(GPIO_PROCESS_STATUS,OUTPUT);
  pinMode(GPIO_NETWORK_STATUS_IN_CHECK,INPUT);
}

void loop(){
  if(digitalRead(GPIO_NETWORK_STATUS_IN_CHECK)){
    digitalWrite(GPIO_NETWORK_STATE,HIGH);
    is_connected_iot = true;
  }else{
    digitalWrite(GPIO_NETWORK_STATE,LOW);
    is_connected_iot = false;
  }
  if(is_card_present){
    digitalWrite(GPIO_SD_EXIST,HIGH);
  }else{
    digitalWrite(GPIO_SD_EXIST,LOW);
  }
    
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
  if(!SD.exists(filename)){
    Serial.println("FIle doesnt exist, creating new");
    // IF THE FILENAME DOESN'T EXIST, THEN CREATE THE FILE AND PROVIDE THE HEADING
    file = SD.open(filename, FILE_WRITE);
    if(!file){
      Serial.print("Error opening");
      Serial.println(filename);
    }else{
    String temp = "DATE TIME STAMP, VOLTAGE (V), CURRENT (A), POWER (W)";
    file.println(temp);
    temp = datetime+", "+voltage+", "+current+", "+power;
    file.println(temp);
    file.close();
  }
  }else{
    file = SD.open(filename, FILE_WRITE);
    if(!file){
      Serial.print("Error opening");
      Serial.println(filename);
    }else{
    Serial.println("File exist");
    String temp = datetime+", "+voltage+", "+current+", "+power;
    file.println(temp);
    file.close();
  }
  }
  digitalWrite(GPIO_PROCESS_STATUS,LOW);
}
if(is_connected_iot){
  digitalWrite(GPIO_PROCESS_STATUS,HIGH);
  String temp = "{\"v\":\"" + voltage + "\",\"a\":\"" + current + "\",\"w\":\""+power+"\"}\n";
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
