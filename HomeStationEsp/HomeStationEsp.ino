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
#define TXDPIN 15
#define RXDPIN 13

SoftwareSerial bluetoothSerial(TXDPIN, RXDPIN); // RX, TX
LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

String bluetoothNextVariable = "?";
String bluetoothNextData = "*";

String dataArray[100];
int dataArrayIndex = 0;

const char *ssid = "Classified WiFi DO NOT CONNECT";
const char *password = "71Pekince71";
String httpPostUrl = "http://iot-open-server.herokuapp.com/data";
String apiToken = "2673e9f43ff7b476b2a89337";
HTTPClient http;
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
float humidity = 0;
float temperature = 0;
float lightIntensity = 0;
float windForce = 0;

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
  Serial.println(dataArrayIndex);
  Serial.println(humidity);
  Serial.println(temperature);
  Serial.println(lightIntensity);  
  readBluetoothData();
  delay(500);
  sendDataToServer();
  printToLcd();
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

  JsonObject& humidityObject = jsonBuffer.createObject();
  humidityObject["key"] = "humidity";
  humidityObject["value"] = humidity;

  JsonObject& temperatureObject = jsonBuffer.createObject();
  temperatureObject["key"] = "temperature";
  temperatureObject["value"] = temperature;
  
  JsonObject& lightIntensityObject = jsonBuffer.createObject();
  lightIntensityObject["key"] = "lightIntensity";
  lightIntensityObject["value"] = lightIntensity;
  
  JsonObject& windForceObject = jsonBuffer.createObject();
  windForceObject["key"] = "windForce";
  windForceObject["value"] = windForce;

  data.add(humidityObject);
  data.add(temperatureObject);
  data.add(lightIntensityObject);
  data.add(windForceObject);

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
  if(bluetoothSerial.available()){
    if((String)(char)bluetoothSerial.peek() != bluetoothNextVariable){
      readBluetoothDataUntilNextVariable();
    } else {
      if(dataArray[dataArrayIndex].length() > 0){
        dataArrayIndex++;
        dataArray[dataArrayIndex] = "";
        Serial.println("New Data!");
        printVariable(dataArray[dataArrayIndex - 1]);
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

