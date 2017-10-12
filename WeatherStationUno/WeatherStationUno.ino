#include "DHT.h"
#include <LightDependentResistor.h>
#include <SoftwareSerial.h>// import the serial library

#define TXDPIN 10
#define RXDPIN 11

#define DHTPIN 2   
#define DHTTYPE DHT22 

#define RESISTOR_LDR 10000 //ohms
#define INPUT_PIN_LIGHT_INTENSITY A0
#define USED_PHOTOCELL LightDependentResistor::GL5528

// Create a GL5528 photocell instance (on A0 pin)
LightDependentResistor photocell(INPUT_PIN_LIGHT_INTENSITY, RESISTOR_LDR, USED_PHOTOCELL);

SoftwareSerial bluetoothSerial(TXDPIN, RXDPIN);

#define ANEMOMETERPIN A1

DHT dht(DHTPIN, DHTTYPE);
String bluetoothNextVariable = "?";
String bluetoothNextData = "*";

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
float humidity = 10.00;
float temperature = 12.00;
float lightIntensity = 14.00;
float windForce = 16.00;

int loopDelaySeconds = 2;
int bluetoothSendDelaySeconds = 1;

void setup() {
  Serial.begin(9600);
  
  bluetoothSerial.begin(9600);
  dht.begin();
  photocell.setPhotocellPositionOnGround(false);
}

void loop() {
  humidity = readHumidity();
  temperature = readTemp();
  lightIntensity = readLightIntensity();
  windForce = readWindForce();
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Light Intensity: ");
  Serial.println(lightIntensity);
  Serial.print("Wind Force: ");
  Serial.println(windForce);
  Serial.println("");
  
  delay(loopDelaySeconds * 1000);
  writeDataToBluetooth();
}

void writeDataToBluetooth(){
  writeVariableToBluetooth(humidityName, humidity);
  delay(bluetoothSendDelaySeconds * 1000);
  
  writeVariableToBluetooth(temperatureName, temperature);
  delay(bluetoothSendDelaySeconds * 1000);
  
  writeVariableToBluetooth(lightIntensityName, lightIntensity);
  delay(bluetoothSendDelaySeconds * 1000);
  
  writeVariableToBluetooth(windForceName, windForce);
  delay(bluetoothSendDelaySeconds * 1000);
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
  return analogRead(ANEMOMETERPIN);
}

void writeVariableToBluetooth(String variableName, String variableValue){
  writeBluetoothData(bluetoothNextVariable);
  writeBluetoothData(bluetoothNextData);  
  writeBluetoothData("Name");
  writeBluetoothData(bluetoothNextData);  
  writeBluetoothData(variableName);
  writeBluetoothData(bluetoothNextData);  
  writeBluetoothData(stringType);
  writeBluetoothData(bluetoothNextData);  
  writeBluetoothData(variableValue);    
}

void writeVariableToBluetooth(String variableName, int variableValue){
  writeBluetoothData(bluetoothNextVariable);  
  writeBluetoothData(bluetoothNextData);  
  writeBluetoothData("Name");
  writeBluetoothData(bluetoothNextData);  
  writeBluetoothData(variableName);
  writeBluetoothData(bluetoothNextData);  
  writeBluetoothData(intType);
  writeBluetoothData(bluetoothNextData);  
  writeBluetoothData(variableValue);    
}

void writeVariableToBluetooth(String variableName, float variableValue){
  writeBluetoothData(bluetoothNextVariable);  
  writeBluetoothData(bluetoothNextData);  
  writeBluetoothData("Name");
  writeBluetoothData(bluetoothNextData);  
  writeBluetoothData(variableName);
  writeBluetoothData(bluetoothNextData);  
  writeBluetoothData(floatType);
  writeBluetoothData(bluetoothNextData);  
  writeBluetoothData(variableValue);  
}

void writeBluetoothData(String data){
  bluetoothSerial.print(data);
}

void writeBluetoothData(int data){
  bluetoothSerial.print(data);
}

void writeBluetoothData(float data){
  bluetoothSerial.print(data);
}


