/*
* The code is developed by shreevallabha. To get more inner details of the code
* Written contact at shreevallabhas@gmail.com
* The code serves the purpose of logging of the adc-data obtained from the sensors
*/

// The following macros defined here are the the macros with corresponding pinouts
#define CHIP_SELECT 53
#define CURRENT_SENSOR_PIN A1
#define VOLTAGE_SENSOR_PIN A0

// Include the required libraries
#include <RTClib.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "arduinoFFT.h"

//The below code belongs to ArduinoFFT
arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These values can be changed in order to evaluate the functions
*/
#define CHANNEL A0
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
}state;



void setup() {
  sampling_period_us = round(1000000*(1.0/samplingFrequency));

  Serial.begin(9600);
  pinMode(CHIP_SELECT, OUTPUT);
  pinMode(CHIP_SELECT, HIGH);
  buffer.reserve(1024);
  Wire.begin();
  RTC.begin();
  RTC.adjust(DateTime(F(__DATE__),F(__TIME__)));
  if(!SD.begin(CHIP_SELECT)){
    Serial.println(F("INITIALISATION OF CARD FAILED"));
  }else{
    state.sd = true;
    Serial.println("CARD IS PRESENT");
  }
  checkandcreatedir();
}

void loop() {
  Serial.println(getFrequency());
  delay(2000);
}


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
  Serial.print("Finally created the file ");
  Serial.println(foldername+"/"+filename);
  if(state.buffer_clear){
    buffer.remove(0,chunksize);
  }
  logdatafromadc(VOLTAGE_SENSOR_PIN,CURRENT_SENSOR_PIN);
}

void logdatafromadc(int voltagepin, int currentpin){
  state.buffer_clear = true;
  unsigned long init_time = millis();
  Serial.println("Started recording....");
  file.println("time in millis, voltage, current");
  while(millis()-init_time<RECORDING_TIME){
    Serial.println(".");
    delay(samplingtime);
    int voltage_data = analogRead(voltagepin);
    int current_data = analogRead(currentpin);
    unsigned long time = millis()-init_time;
    String temp_data = String(time)+","+String(voltage_data)+","+String(current_data);
    buffer += temp_data+"\r\n";
    chunksize = file.availableForWrite();
    if(chunksize && buffer.length() >= chunksize){
      file.write(buffer.c_str(),chunksize);
      buffer.remove(0,chunksize);
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
