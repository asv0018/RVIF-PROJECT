#define GPIO_INTERNET_AVAILABLE 2

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
WiFiManager wifiManager;

bool is_internet_available = false;
const char* host = "script.google.com";
const int httpsPort = 443;

WiFiClientSecure client;

String GAS_ID = "AKfycbx6QYgiwdGXgzYgBpLIWgrC_3EFvLfNdSBNM-c8xuC1TCj2FuoFaLieRQ";

void setup() {
  Serial.begin(115200);
  
  pinMode(GPIO_INTERNET_AVAILABLE, OUTPUT);
  digitalWrite(GPIO_INTERNET_AVAILABLE, LOW);

  wifiManager.autoConnect("ENERGY METER","savithapr");
  
  is_internet_available = true;

  client.setInsecure();
  
}


void loop() {
  String temp;
  if(is_internet_available){
    digitalWrite(GPIO_INTERNET_AVAILABLE, HIGH);
  }else{
    digitalWrite(GPIO_INTERNET_AVAILABLE, LOW);
  }
  if(Serial.available()>0){
    temp = Serial.readString();
    Serial.println(temp);
    delay(30);
    sendData(temp);
  }
  
}

void sendData(String data){
  if (!client.connect(host, httpsPort)){
    Serial.println("connection failed");
    is_internet_available = false;
    digitalWrite(GPIO_INTERNET_AVAILABLE, LOW);
    return;
  }
  is_internet_available = true;
  digitalWrite(GPIO_INTERNET_AVAILABLE, HIGH);
  
  DynamicJsonDocument doc(200);
  deserializeJson(doc, data);
  
  String url = "/macros/s/" + GAS_ID + "/exec?"+"current="+String(doc["a"].as<String>())+"&voltage="+String(doc["v"].as<String>())+"&power="+String(doc["w"].as<String>())+"&rpm="+String(doc["r"].as<String>())+"&frequency="+String(doc["f"].as<String>());
  
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");
         
  while (client.connected()){
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  
  String line = client.readStringUntil('\n');
  if(line.startsWith("238")){
    digitalWrite(GPIO_INTERNET_AVAILABLE, HIGH);
    is_internet_available = true;
  }else{
    digitalWrite(GPIO_INTERNET_AVAILABLE, LOW);
    is_internet_available = false;
  }

}
