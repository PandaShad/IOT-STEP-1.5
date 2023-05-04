/*** Basic/Static Wifi connection

     Fichier wificonnect/wifi_utils.ino ***/

#include <WiFi.h> // https://www.arduino.cc/en/Reference/WiFi
#include "wifi_utils.h"
/*--------------------------------------------------------------------------*/

String translateEncryptionType(wifi_auth_mode_t encryptionType) {
  // cf https://www.arduino.cc/en/Reference/WiFiEncryptionType 
  switch (encryptionType) {
  case (WIFI_AUTH_OPEN):
    return "Open";
  case (WIFI_AUTH_WEP):
    return "WEP";
  case (WIFI_AUTH_WPA_PSK):
    return "WPA_PSK";
  case (WIFI_AUTH_WPA2_PSK):
    return "WPA2_PSK";
  case (WIFI_AUTH_WPA_WPA2_PSK):
    return "WPA_WPA2_PSK";
  case (WIFI_AUTH_WPA2_ENTERPRISE):
    return "WPA2_ENTERPRISE";
  }
}

/*--------------------------------------------------------------------------*/
void wifi_status(){
  /* print the status of the connected wifi  : two ways ! */
  // Use Pure C =>  array of chars
  //Serial.printf("WiFi Status : \n");
  //Serial.printf("\t#%d\n", i);
  //Serial.printf("\tIP address : %s\n", WiFi.localIP().toString().c_str());
  //Serial.printf("\tMAC address : %s\n", WiFi.macAddress().c_str());
  //Serial.printf("\tSSID : %s\n", WiFi.SSID());
  //Serial.printf("\tReceived Signal Strength Indication : %ld dBm\n",WiFi.RSSI());
  //Serial.printf("\tReceived Signal Strength Indication : %ld %\n",constrain(2 * (WiFi.RSSI() + 100), 0, 100));
  //Serial.printf("\tBSSID : %s\n", WiFi.BSSIDstr().c_str());
  //Serial.printf("\tEncryption type : %s\n", translateEncryptionType(WiFi.encryptionType(0)));
  
  // Use of C++ =>  String !
  String s = "WiFi Status : \n";
  //s += "\t#" + String() + "\n";
  s += "\tIP address : " + WiFi.localIP().toString() + "\n"; 
  s += "\tMAC address : " + String(WiFi.macAddress()) + "\n";
  s += "\tSSID : " + String(WiFi.SSID()) + "\n";
  s += "\tReceived Signal Strength Indication : " + String(WiFi.RSSI()) + " dBm\n";
  s += "\tReceived Signal Strength Indication : " + String(constrain(2 * (WiFi.RSSI() + 100), 0, 100)) + " %\n";
  s += "\tBSSID : " + String(WiFi.BSSIDstr()) + "\n";
  s += "\tEncryption type : " + translateEncryptionType(WiFi.encryptionType(0))+ "\n";
  Serial.print(s);
}
