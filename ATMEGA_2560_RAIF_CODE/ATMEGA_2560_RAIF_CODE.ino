/*
* The code is developed by shreevallabha. To get more inner details of the code
* Written contact at shreevallabhas@gmail.com
* The code serves the purpose of logging of the adc-data obtained from the sensors
*/

// The following macros defined here are the the macros with corresponding pinouts
#define CHIP_SELECT 53
#define CURRENT_SENSOR_PIN A1
#define VOLTAGE_SENSOR_PIN A0
#define DPDT 2
#define CHANNEL A0

// Include the required libraries
#include <SoftwareSerial.h>
#include <RTClib.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "arduinoFFT.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

//The below code belongs to ArduinoFFT
arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These values can be changed in order to evaluate the functions
*/

const uint16_t samples = 64; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 100; //Hz, must be less than 10000 due to ADC

unsigned int sampling_period_us;
unsigned long microseconds;
unsigned int chunksize;
/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03


// Initialise few instances
SoftwareSerial client(A11,A10);

SoftwareSerial rpmmeter(5,4);
RTC_DS3231 RTC;
DateTime now;
File file;
String buffer;
String foldername;
String filename;
unsigned long RECORDING_TIME = 20000;
int samplingtime = 0;

// Initialise all statuses here
struct status{
  bool sd = false;
  bool buffer_clear = false;
  bool logdataflag = false;
}state;



void setup() {
  sampling_period_us = round(1000000*(1.0/samplingFrequency));
  Serial.begin(9600);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  pinMode(CHIP_SELECT, OUTPUT);
  pinMode(CHIP_SELECT, HIGH);
  pinMode(DPDT,INPUT);
  buffer.reserve(1024);
  Wire.begin();
  rpmmeter.begin(9600);
  client.begin(9600);
  RTC.begin();
  RTC.adjust(DateTime(F(__DATE__),F(__TIME__)));
  if(!SD.begin(CHIP_SELECT)){
    Serial.println(F("INITIALISATION OF CARD FAILED"));
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(13,0);
    display.println(F("INSERT AN SD CARD"));
    display.display();
    display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
    display.display();
  }else{
    state.sd = true;
    Serial.println("CARD IS PRESENT");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20,0);
    display.println(F("SD CARD PRESENT"));
    display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
    display.display();
  }
  checkandcreatedir();
  display.clearDisplay();

}


// THIS IS VALID
void loop(){

  bool temp_switch_state = digitalRead(DPDT);
  if(!state.sd){
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(13,0);
    display.println(F("INSERT AN SD CARD"));
    display.display();
    display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
    display.display();
  }else{
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20,0);
    display.println(F("SD CARD PRESENT"));
    display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
    display.display();
  }
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(25,30);
  display.println(F("PUSH UP"));
  display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
  display.display();
  Serial.println(temp_switch_state);
  if(temp_switch_state){
    temp_switch_state = digitalRead(DPDT);
    while(temp_switch_state){
      temp_switch_state = digitalRead(DPDT);
      Serial.println("The switch is turned to start logging");
      updatedir();
      temp_switch_state = digitalRead(DPDT);
      if(temp_switch_state){
        display.clearDisplay();
        while(true){
          Serial.println("TURN OF NOW");
          if(!state.sd){
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(13,0);
            display.println(F("INSERT AN SD CARD"));
            display.display();
            display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
            display.display();
          }else{
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(20,0);
            display.println(F("SD CARD PRESENT"));
            display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
            display.display();
          }
          display.setTextSize(2);
          display.setTextColor(SSD1306_WHITE);
          display.setCursor(10,30);
          display.println(F("PUSH DOWN"));
          display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
          display.display();
          temp_switch_state = digitalRead(DPDT);
          if(!temp_switch_state){
            break;
          }
        }
      }
      temp_switch_state = digitalRead(DPDT);
      if(!temp_switch_state){
        break;
      }
    }
    display.clearDisplay();
  }

}


char data;
String mainstring = "";
char previous_rpm = '0';

void rpmreader_reference(){
  
}

/*
void loop(){

  bool temp_switch_state = digitalRead(DPDT);
  if(!state.sd){
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(13,0);
    display.println(F("INSERT AN SD CARD"));
    display.display();
    display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
    display.display();
  }else{
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20,0);
    display.println(F("SD CARD PRESENT"));
    display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
    display.display();
  }
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(25,30);
  display.println(F("PUSH UP"));
  display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
  display.display();
  Serial.println(temp_switch_state);
  if(temp_switch_state){
    temp_switch_state = digitalRead(DPDT);
    while(temp_switch_state){
      temp_switch_state = digitalRead(DPDT);
      Serial.println("The switch is turned to start logging");
      updatedir();
      temp_switch_state = digitalRead(DPDT);
      if(temp_switch_state){
        display.clearDisplay();
        while(true){
          Serial.println("TURN OF NOW");
          if(!state.sd){
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(13,0);
            display.println(F("INSERT AN SD CARD"));
            display.display();
            display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
            display.display();
          }else{
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(20,0);
            display.println(F("SD CARD PRESENT"));
            display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
            display.display();
          }
          display.setTextSize(2);
          display.setTextColor(SSD1306_WHITE);
          display.setCursor(10,30);
          display.println(F("PUSH DOWN"));
          display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
          display.display();
          temp_switch_state = digitalRead(DPDT);
          if(!temp_switch_state){
            break;
          }
        }
      }
      temp_switch_state = digitalRead(DPDT);
      if(!temp_switch_state){
        break;
      }
    }
    display.clearDisplay();
  }

}
*/

/*
void loop() {
  if(rpmmeter.available()){
    int res = rpmmeter.read();
    Serial.println(res);
  }
}
*/

void checkandcreatedir(){
  now = RTC.now();
  foldername = String(now.year())+"/"+String(now.month())+"/"+String(now.day());
  Serial.print("The folder name is ");
  Serial.println(foldername);
  if(!SD.exists(foldername)){
    SD.mkdir(foldername);
    Serial.println("Folder is created");
  }else{
    Serial.println("Folder already exist");
  }
}

void updatedir(){
  now = RTC.now();
  filename = String(now.hour())+""+String(now.minute())+""+String(now.second())+".csv";
  Serial.print("The filename is ");
  Serial.println(filename);
  file = SD.open(foldername+"/"+filename, FILE_WRITE);
  display.clearDisplay();
  if(!state.sd){
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(13,0);
    display.println(F("INSERT AN SD CARD"));
    display.display();
    display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
    display.display();
  }else{
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20,0);
    display.println(F("SD CARD PRESENT"));
    display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
    display.display();
  }
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(3,30);
  display.println(filename);
  display.drawLine(0, 10, display.width(),10, SSD1306_WHITE);
  display.display();
  Serial.print("Finally created the file ");
  Serial.println(foldername+"/"+filename);
  if(state.buffer_clear){
    buffer.remove(0,chunksize);
  }
  double temp_freq = getFrequency();
  file.println("Frequency, "+String(temp_freq));
  file.println(" , , ");
  logdatafromadc(VOLTAGE_SENSOR_PIN,CURRENT_SENSOR_PIN);
}

//char previous_rpm = '0';

void logdatafromadc(int voltagepin, int currentpin){
  state.logdataflag = true;
  state.buffer_clear = true;
  unsigned long init_time = millis();
  Serial.println("Started recording....");
  //file.println("time in millis, voltage, current");
  file.println("time in millis, voltage, current, rpm");
  while(millis()-init_time<RECORDING_TIME){
    Serial.println(".");
    delay(samplingtime);
    int voltage_data = analogRead(voltagepin);
    int current_data = analogRead(currentpin);
    unsigned long time = millis()-init_time;
    if(client.available()){
    mainstring = "";
    while(client.available()){
      data = client.read();
      if(data!='\r'){
        if(data!='\n'){
          mainstring = mainstring + String(data);
        }
      }      
    }
  }
    Serial.println("RPM IS "+String(mainstring));
    //String temp_data = String(time)+","+String(voltage_data)+","+String(current_data);
    String temp_data = String(time)+","+String(voltage_data)+","+String(current_data)+","+String(mainstring);
    buffer += temp_data+"\n";
    chunksize = file.availableForWrite();
    if(chunksize && buffer.length() >= chunksize){
      file.write(buffer.c_str(),chunksize);
      buffer.remove(0,chunksize);
    }
    if(!digitalRead(DPDT)){
      state.logdataflag = false;
      Serial.println("Closing the process forcefully");
      break;
    }
  }
  file.close();
  Serial.println("Recorded the signals successfully");
}

double getFrequency(){
  /*SAMPLING*/
  microseconds = micros();
  for(int i=0; i<samples; i++)
  {
      vReal[i] = analogRead(CHANNEL);
      vImag[i] = 0;
      while(micros() - microseconds < sampling_period_us){
        //empty loop
      }
      microseconds += sampling_period_us;
  }
  /* Print the results of the sampling according to time */
  //Serial.println("Data:");
  PrintVector(vReal, samples, SCL_TIME);
  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	/* Weigh data */
  //Serial.println("Weighed data:");
  PrintVector(vReal, samples, SCL_TIME);
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
  //Serial.println("Computed Real values:");
  PrintVector(vReal, samples, SCL_INDEX);
  //Serial.println("Computed Imaginary values:");
  PrintVector(vImag, samples, SCL_INDEX);
  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */
  //Serial.println("Computed magnitudes:");
  PrintVector(vReal, (samples >> 1), SCL_FREQUENCY);
  double x = FFT.MajorPeak(vReal, samples, samplingFrequency);
  //Serial.println(x, 6); //Print out what frequency is the most dominant.
  return x;
}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
    {
      double abscissa;
      /* Print abscissa value */
      switch (scaleType)
      {
        case SCL_INDEX:
          abscissa = (i * 1.0);
  	break;
        case SCL_TIME:
          abscissa = ((i * 1.0) / samplingFrequency);
  	break;
        case SCL_FREQUENCY:
          abscissa = ((i * 1.0 * samplingFrequency) / samples);
  	break;
      }
      //Serial.print(abscissa, 6);
      //if(scaleType==SCL_FREQUENCY)
        //Serial.print("Hz");
      //Serial.print(" ");
      //Serial.println(vData[i], 4);
    }
    //Serial.println();
}

char get_rpm(){
  char data;
    if(client.available()){
      while(client.available()){
        data = client.read();
      }
      return data;
  }
  return 'A';
}














void refer2(int voltagepin, int currentpin){
  unsigned long init_time = millis();
  Serial.println("Started recording....");
  file.println("time in millis, voltage, current");
  while(millis()-init_time<RECORDING_TIME){
    Serial.println(".");
    int voltage_data = analogRead(voltagepin);
    int current_data = analogRead(currentpin);
    unsigned long time = millis()-init_time;
    String temp_data = String(time)+","+String(voltage_data)+","+String(current_data);
    file.println(temp_data);
  }
  file.close();
  Serial.println("Recorded the signals successfully");
}

void refer(){
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
