#include <Arduino.h>
#include <map>

#include "DallasTemperature.h"

String getLED_fromTempDelta() {

}

float get_temperature(DallasTemperature tempSensor) {
  tempSensor.requestTemperaturesByIndex(0);
  float t = tempSensor.getTempCByIndex(0);
  return t;
}

float get_light(int lightPin) {
    return analogRead(lightPin);
}

boolean get_heater_status(float temperature, float SB) {
    return temperature < SB ? true : false;
}

boolean get_cooler_status(float temperature, float SH) {
    return temperature > SH ? true : false;
}

boolean get_fire_status(float light) {
    return light > 5000.0 ? true : false;
}