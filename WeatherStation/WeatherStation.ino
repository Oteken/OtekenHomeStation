#include "DHT.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LightDependentResistor.h>

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

#define DHTPIN 2   
#define DHTTYPE DHT22 

#define RESISTOR_LDR 10000 //ohms
#define INPUT_PIN_LIGHT_INTENSITY A0
#define USED_PHOTOCELL LightDependentResistor::GL5528

// Create a GL5528 photocell instance (on A0 pin)
LightDependentResistor photocell(INPUT_PIN_LIGHT_INTENSITY, RESISTOR_LDR, USED_PHOTOCELL);

LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

DHT dht(DHTPIN, DHTTYPE);

int loopDelaySeconds = 5;
const char *ssid = "Classified WiFi DO NOT CONNECT";
const char *password = "71Pekince71";
String httpPostUrl = "http://iot-open-server.herokuapp.com/data";
String apiToken = "2673e9f43ff7b476b2a89337";

HTTPClient http;

void setup() {
  Serial.begin(9600);
  dht.begin();
  connectToWifi();
  photocell.setPhotocellPositionOnGround(false);
  lcd.begin(16,2);
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();
}

void loop() {
  float humidity = readHumidity();
  float temperature = readTemp();
  float lightIntensity = readLightIntensity();
  float windForce = readWindForce();

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
  lcd.clear();
  printHumidity(humidity);
  delay(loopDelaySeconds * 1000);
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

float readHumidity(){
  return dht.readHumidity();
}

float readTemp(){
  return dht.readTemperature();
}

float readLightIntensity(){
  return photocell.getCurrentLux();
}

float readWindForce(){
  return 0;
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

