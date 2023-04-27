#include <Arduino.h>
#include <ArduinoJson.h>

// ---------------LED BLINK-----------------------

// #define LED_BUILTIN 2

// const int ledPin = 19;

// void setup() {
  // Serial.begin(9600);
  // Serial.println("*** This message will only be displayed on start or reset. ***");
  // delay(2000);
  // pinMode(ledPin, OUTPUT);
// }

// void loop() {
  // digitalWrite(ledPin, HIGH);
  // delay(1000);
  // digitalWrite(ledPin, LOW);
  // delay(1000);
// }

// ---------------photoresistor-----------------------

// void setup(){
//   Serial.begin(9600);
// }

// void loop(){
//   int sensorValue;
//   sensorValue=analogRead(A5); // Read analog input on ADC1_CHANNEL_5 (GPIO 33) Pin "D33"
//   Serial.println(sensorValue, DEC); // Prints the value to the serial port as human-readable ASCII text
//   delay(1000); // wait as for next reading
// }


// ---------------TODO YOUR WORK-----------------------

#include "OneWire.h"
#include "DallasTemperature.h"

const int ONBOARD_LED=2;
const int RED_LED_PIN=21;
const int GREEN_LED_PIN=19;
const int FAN_PIN=27;

int numberKeyPresses = 0;

OneWire oneWire(23);
DallasTemperature tempSensor(&oneWire);

const float SH = 25.0;
const float STH = 30.0;
const float SB = 24.0;

const int fireBound = 2000;


void setup(void){
  Serial.begin(9600);
  tempSensor.begin();

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(ONBOARD_LED, OUTPUT);

  ledcAttachPin(FAN_PIN, 0);
  ledcSetup(0, 25000, 8);
  ledcWrite(0,0);
}

void loop(void){
  tempSensor.requestTemperaturesByIndex(0);
  float t = tempSensor.getTempCByIndex(0);

  setFanSpeed(SH, STH, t);

  int sensorValue;
  sensorValue = analogRead(A5);

  if(t<SB){
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
  }
  else if (t>SH)
  {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
  }
  else {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
  }

  if(sensorValue>fireBound){
    digitalWrite(ONBOARD_LED, HIGH);
  } else {
    digitalWrite(ONBOARD_LED, LOW);
  }
  // getRgbValues(SB, SH, t);
  String data = Serialize_ESPstatus(t, sensorValue);
  Serial.println(data);
  delay(1000);
}
String Serialize_ESPstatus(float temp, int light){
  /*
   * put all relevant data from esp in a "json formatted" String
   */
  StaticJsonDocument<1000> jsondoc;
  jsondoc["status"]["temperature"] = temp; // Temp value
  jsondoc["status"]["light"] = light; // Light value
  // jsondoc["status"]["heat"] = ON; // Heater
  // jsondoc["status"]["cold"] = OFF; // Cooler
  // jsondoc["status"]["running"] = RUNNING; // Regulation status : RUNNING or HALT
  // jsondoc["status"]["fire"] = NO; // NO or YES   

  // jsondoc["regul"]["sh"] = HIGH; // Seuil Haut
  // jsondoc["regul"]["sb"] = LOW;  // Seuil bas

  // jsondoc["info"]["loc"] = LOCATION;
  // jsondoc["info"]["user"] = IDENTIFIER;
  // jsondoc["info"]["uptime"] = getUptime();
  // jsondoc["info"]["ssid"] = getSSID(); // utile quand on se sera connecté .. pour l'instant inutile
  // jsondoc["info"]["ident"] = getMAC(); // utile quand on se sera connecté .. pour l'instant inutile
  // jsondoc["info"]["ip"] = getIP(); // utile quand on se sera connecté .. pour l'instant inutile
 

  // jsondoc["reporthost"]["target_ip"] = target_ip; // utile quand on se sera connecté .. pour l'instant inutile
  // jsondoc["reporthost"]["target_port"] = target_port; // utile quand on se sera connecté .. pour l'instant inutile
  // jsondoc["reporthost"]["sp"] = target_sp; // utile quand on se sera connecté .. pour l'instant inutile
 
  String data = "";
  serializeJson(jsondoc, data);
  return data;
}

void getRgbValues(float minimum, float maximum, float value){
  float normalizeValue = (value - minimum) / (maximum - minimum) * 2;
  int blue = getDistance(normalizeValue, 0);
  int green = getDistance(normalizeValue, 1);
  int red = getDistance(normalizeValue, 2);
  char strBuff[50];
  sprintf(strBuff, "Blue: %d, Green: %d, Red: %d", blue, green, red);
  // Serial.println(strBuff);
}

int getDistance(float value, float color){
  // Serial.println("VALUE");
  // Serial.println(value);
  // Serial.println("COLOR");
  // Serial.println(color);
  float distance = abs(value-color);
  // Serial.println("DISTANCE");
  // Serial.println(distance);
  float colorStrength = 1 - distance;
  if(colorStrength < 0){
    colorStrength = 0;
  }
  // Serial.println("COLORSTRENGTH");
  // Serial.println(colorStrength);
  return (int)round(colorStrength * 255);
}

void setFanSpeed(float bottom, float upper, float value) {

  // solution maison
  float res=0.0;
  float diff=upper-bottom;
  float distance=value-bottom;
  if(distance>=0){
    res=(distance/diff)*100;
    // Vitesse minimale du Fan (sinon il ne tourne pas)
    if(res<=70.0){
      res=70.0;
    }
    else if (res>=255)
    {
      res=255.0;
    }
  }

  // Api Arduino
  // res = map(value, bottom, upper, 0, 255);

  ledcWrite(0, res);
  // Serial.printf("Fan speed: %f RPM \n", res);
}