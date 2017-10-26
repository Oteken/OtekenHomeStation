#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <Wire.h>
#include <FastIO.h>
#include <I2CIO.h>
#include <LCD.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_SR.h>
#include <LiquidCrystal_SR2W.h>
#include <LiquidCrystal_SR3W.h>

#define I2C_ADDR    0x27 // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7
#define TXDPIN 12
#define RXDPIN 14
#define buzzer 10

#define NOTE_C4  262   //Defining note frequency
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988

int notesDay[] = {       //Note of the song, 0 is a rest/pulse
   NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, 
   NOTE_G4, NOTE_A4, NOTE_B4
};

int durationDay[] = {         //duration of each note (in ms) Quarter Note is set to 250 ms
  125, 125, 125, 125, 125, 
  125, 125
};

int notesNight[] = {       //Note of the song, 0 is a rest/pulse
   NOTE_B4, NOTE_A4, NOTE_G4, NOTE_F4, 
   NOTE_E4, NOTE_D4, NOTE_C4
};

int durationNight[] = {         //duration of each note (in ms) Quarter Note is set to 250 ms
  125, 125, 125, 125, 125, 
  125, 125
};

int songspeed = 1;

SoftwareSerial bluetoothSerial(TXDPIN, RXDPIN); // RX, TX
LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

String bluetoothNextVariable = "?";
String bluetoothNextData = "*";

String dataArray[100];
int dataArrayIndex = 0;

float windForceArray[4];
int windForceArrayIndex = 0;
int motorMaxRpm = 11500;
int motorMaxVoltage = 6;
int motorMinVoltage = 3;
// Taking a linear formula for the relation between Rpm and voltage.
// 11500/(6-3) = 3833.3 rpm per voltage.
// 5/1024 = 0.0049 volt per bit in analog input
float rpmPerBit = (5.00/1024.00) * (11500.00 / (6.00-3.00));

const char *ssid = "Wifi ssid";
const char *password = "Wifi password";
String httpPostUrl = "http://iot-open-server.herokuapp.com/data";
String apiToken = "2673e9f43ff7b476b2a89337";
HTTPClient http;
String imageUrl = "https://lh4.googleusercontent.com/28cyjr25CPIp9u4okR8HlpTsum4HXVXp0zjjiLmcxYi6ZQzLYexfs1L5D0UnrX1y0yJhudzNXwOVQJY=w1366-h662-rw";
int printSwitch = 0;

// Types that variables can be. These are needed for sending of data via bluetooth.
String stringType = "St";
String intType = "In";
String floatType = "Fl";

// Names of the variables being tracked
String humidityName = "humidity";
String temperatureName = "temperature";
String lightIntensityName = "light";
String windForceName = "wind";
// Types of the variables 
String humidityType = floatType;
String temperatureType = floatType;
String lightType = floatType;
String windType = floatType;
// Values of the variables
float defaultFloat = -10.00;
float humidity = defaultFloat;
float temperature = defaultFloat;
float lightIntensity = defaultFloat;
float windForce = defaultFloat;
boolean dark = false;

void setup() {
  Serial.begin(9600);
  dataArray[0] = "";
  connectToWifi();
  bluetoothSerial.begin(9600);
  lcd.begin(16,2);
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();
}

void loop() {
  processNextData();
  readBluetoothData();
  delay(500);
  sendDataToServer();
  printToLcd();
  actuator();
}

void actuator(){
  if(dark && lightIntensity > 50){
    playSongDay();
    dark = false;
  }
  if(!dark && lightIntensity < 50){
    playSongNight();
    dark = true;
  }
}

void playSongDay(){
  for (int i=0;i<7;i++){              //203 is the total number of music notes in the song
    int wait = durationDay[i] * songspeed;
    tone(buzzer,notesDay[i],wait);          //tone(pin,frequency,duration)
    delay(wait);
  }
}

void playSongNight(){
    for (int i=0;i<7;i++){              //203 is the total number of music notes in the song
    int wait = durationNight[i] * songspeed;
    tone(buzzer,notesNight[i],wait);          //tone(pin,frequency,duration)
    delay(wait);
  }
}

void printToLcd(){
  lcd.clear();
  switch(printSwitch) {
    case 0:
      printHumidity(humidity);
      printSwitch++;
      break;
    case 1:
      printTemperature(temperature);
      printSwitch++;
      break;
    case 2:
      printLightIntensity(lightIntensity);
      printSwitch++;
      break;
    case 3:
      printWindForce(windForce);
      printSwitch = 0;
      break;
  }
}

void sendDataToServer(){
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& jsonRoot = jsonBuffer.createObject();
  
  jsonRoot["token"] = apiToken;
  
  JsonArray& data = jsonRoot.createNestedArray("data");

  if(humidity != defaultFloat){
    JsonObject& humidityObject = jsonBuffer.createObject();
    humidityObject["key"] = "humidity";
    humidityObject["value"] = humidity;
    data.add(humidityObject);
  }
  if(temperature != defaultFloat){
    JsonObject& temperatureObject = jsonBuffer.createObject();
    temperatureObject["key"] = "temperature";
    temperatureObject["value"] = temperature;
    data.add(temperatureObject);
  }
  if(lightIntensity != defaultFloat){
    JsonObject& lightIntensityObject = jsonBuffer.createObject();
    lightIntensityObject["key"] = "lightIntensity";
    lightIntensityObject["value"] = lightIntensity;
    data.add(lightIntensityObject);
  }
  if(windForce != defaultFloat){
    JsonObject& windForceObject = jsonBuffer.createObject();
    windForceObject["key"] = "windForce";
    windForceObject["value"] = windForce;
    data.add(windForceObject);
  }

  JsonObject& imageObject = jsonBuffer.createObject();
  imageObject["key"] = "image";
  imageObject["value"] = imageUrl;
  data.add(imageObject);

  String dataToSend;
  jsonRoot.printTo(dataToSend);

  jsonRoot.prettyPrintTo(Serial);
  postData(dataToSend);
}

void printVariable(String dataArrayVariable){
  Serial.println(readVariableName(dataArrayVariable));
  Serial.println(readVariableType(dataArrayVariable));
  Serial.println(readVariableValue(dataArrayVariable));
}

void readBluetoothData(){
    Serial.println("A");
  if(bluetoothSerial.available()){
    if((String)(char)bluetoothSerial.peek() != bluetoothNextVariable){
      readBluetoothDataUntilNextVariable();
    } else {
      if(dataArray[dataArrayIndex].length() > 0){
        if(!isDataCorrupt(dataArray[dataArrayIndex])){
          dataArrayIndex++;
          dataArray[dataArrayIndex] = "";
          printVariable(dataArray[dataArrayIndex - 1]);
        } else {
          dataArray[dataArrayIndex] = "";
        }
      }
      bluetoothSerial.read();
    }
  }
}

void readBluetoothDataUntilNextVariable(){
  String currentData = dataArray[dataArrayIndex];
  while(bluetoothSerial.available()){
    if((String)(char)bluetoothSerial.peek() == bluetoothNextVariable){
      break;
    } else {
      currentData += (String)(char)bluetoothSerial.read();
    }
  }
  dataArray[dataArrayIndex] = currentData;
}

String readVariableName(String dataVariable){
  String variableName = "";
  String lastChar = "";
  int i = 6; //Names of variables start at the 6th character
  int j = 0;
  lastChar = dataVariable[i + j];
  while(lastChar != bluetoothNextData){
    variableName += lastChar;
    j++;
    lastChar = dataVariable[i + j];
  }
  return variableName;
}

String readVariableType(String dataVariable){
  String variableType = "";
  String lastChar = "";
  int i = 6; //Names of variables start at the 6th character
  int j = 0;
  lastChar = dataVariable[i + j];
  while(lastChar != bluetoothNextData){
    j++;
    lastChar = dataVariable[i + j];
  }
  j++;
  lastChar = dataVariable[i + j];
  while(lastChar != bluetoothNextData){
    variableType += lastChar;
    j++;
    lastChar = dataVariable[i + j];
  }
  return variableType;
}

String readVariableValue(String dataVariable){
  String variableValue = "";
  String lastChar = "";
  int i = 6; //Names of variables start at the 6th character
  int j = 0;
  lastChar = dataVariable[i + j];
  while(lastChar != bluetoothNextData){
    j++;
    lastChar = dataVariable[i + j];
  }
  j++;
  lastChar = dataVariable[i + j];
  while(lastChar != bluetoothNextData){
    j++;
    lastChar = dataVariable[i + j];
  }
  j++;
  lastChar = dataVariable[i + j];
  while(lastChar != bluetoothNextData && (i + j) < dataVariable.length()){
    variableValue += lastChar;
    j++;
    lastChar = dataVariable[i + j];
  }
  return variableValue;
}

void moveDataArrayToLeft(){
  for(int i = 1; i <= dataArrayIndex; i++){
    dataArray[i - 1] = dataArray[i];
  }
}

void processNextData(){
  if(dataArrayIndex > 0){
    String variableName = readVariableName(dataArray[0]);
    String variableType = readVariableType(dataArray[0]);
    String variableValue = readVariableValue(dataArray[0]);
    if(variableName == humidityName){
      humidity = variableValue.toFloat();
    }
    if(variableName == temperatureName){
      temperature = variableValue.toFloat();
    }
    if(variableName == lightIntensityName){
      lightIntensity = variableValue.toFloat();
    }
    if(variableName == windForceName){
      windForce = variableValue.toFloat();
    }
    moveDataArrayToLeft();
    dataArrayIndex--;
  }
}

boolean isDataCorrupt(String data){
  int index = data.indexOf(bluetoothNextData);
  if(index > -1){
    int indexOne = data.indexOf(bluetoothNextData, index + 1);
    if(indexOne - index < 1){
      return true;
    }
    if(indexOne > -1){
      int indexTwo = data.indexOf(bluetoothNextData, indexOne + 1);
      if(indexTwo - indexOne < 1){
        return true;
      }
      if(indexTwo > -1){
        if(!(indexTwo != (data.length()-1)))
          return true;
      } else {
        return true;
      }
    } else {
      return true;
    }
  } else {
    return true;
  }
  index = data.indexOf("Name");
  if(!index)
    return true;
  if(data.indexOf(intType) == -1 && data.indexOf(floatType) == -1 && data.indexOf(stringType) == -1)
    return true;
  return false;
}


void connectToWifi() {
   WiFi.begin ( ssid, password );
  
  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  
  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );
}

void postData(String stringToPost) {
  if(WiFi.status()== WL_CONNECTED){
    http.begin(httpPostUrl);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(stringToPost);
    String payload = http.getString();
  
    Serial.println(httpCode);
    Serial.println(payload);
    http.end();
  } else {
    Serial.println("Wifi connection failed.");
  }
}

float printHumidity(float humidity){
  lcd.print("Humidity = ");
  lcd.setCursor(0, 1);
  lcd.print(humidity);
  lcd.setCursor(1, 1);
}

float printTemperature(float temperature){
  lcd.print("Temperature = ");
  lcd.setCursor(0, 1);
  lcd.print(temperature);
  lcd.setCursor(1, 1);
}

float printLightIntensity(float lightIntensity){
  lcd.print("Light Intensity = ");
  lcd.setCursor(0, 1);
  lcd.print(lightIntensity);
  lcd.setCursor(1, 1);
}

float printWindForce(float windForce){
  lcd.print("Wind Force = ");
  lcd.setCursor(0, 1);
  lcd.print(windForce);
  lcd.setCursor(1, 1);
}
