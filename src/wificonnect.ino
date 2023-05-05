/*** Basic/Static Wifi connection

     Fichier wificonnect/wificonnect.ino ***/

#include <WiFi.h> // https://www.arduino.cc/en/Reference/WiFi
#include <WiFiMulti.h>
#include "wifi_utils.h"
#include "wificonnect.h"

#define SaveDisconnectTime 1000 // Connection seems to need several tries (often two)
// Time in ms for save disconnection, => delay between try
// cf https://github.com/espressif/arduino-esp32/issues/2501  
#define WiFiMaxTry 10

/*--------------------------------------------------------------------------*/
void wifi_connect_basic(String hostname, String ssid, String passwd){
  int nbtry = 0; // Nb of try to connect

  WiFi.mode(WIFI_OFF);   
  WiFi.mode(WIFI_STA); // Set WiFi to station mode 
  // delete old config
  // WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.disconnect(true); // Disconnect from an AP if it was previously connected
  // WiFi.persistent(false); // Avoid to store Wifi configuration in Flash
  
  // Define hostname before begin => in C str ! not C++
  WiFi.setHostname(hostname.c_str()); 
  
  Serial.printf("\nAttempting %d to connect AP of SSID : %s", nbtry, ssid.c_str());
  WiFi.begin(ssid.c_str(), passwd.c_str());
  //WiFi.begin(ssid.c_str(), passwd.c_str(), 0, WiFi.BSSID(thegoodone));
  while(WiFi.status() != WL_CONNECTED && (nbtry < WiFiMaxTry)){
    delay(SaveDisconnectTime); // 500ms seems to work in most cases, may depend on AP
    Serial.print(".");
    nbtry++;
  }
}

/*--------------------------------------------------------------------------*/
int wifi_search_neighbor(){
  //  -90 dBm : niveau minimum permettant d'exploiter le signal
#define MinimumWiFiRSSI -90
  
  /* Scan of neighbor Networks -----*/
  int N = WiFi.scanNetworks(); 
  if (N>0){ // Print descriptions if some ?
    Serial.print("\n-------------------\n");
    Serial.printf("Networks found : # %d\n",N);
    for (int i=0 ; i<N ; i++){
      Serial.printf("#%d => Signal Strength (higher is better): %ld dBm\n", i , WiFi.RSSI(i));
      delay(1000);  // slow it for serial 
    }
  }
  /* Choose one among -----*/
  int thegoodone = -1;
  for (int i=0 ; i<N ; i++){
    if (WiFi.RSSI(i) > MinimumWiFiRSSI) {
      thegoodone = i;
      Serial.printf("The #%d  satisfies criteria !\n", thegoodone);
      break; // the first 
    }
  }
  return thegoodone;
}

/*--------------------------------------------------------------------------*/
void wifi_connect_multi(String hostname){
  
  WiFiMulti wm; // Creates an instance of the WiFiMulti class
  // Attention ! PAS arrivé à sortir l'instance de la fonction => heap error ! Why ???? 
  Serial.println("AndroidAP6109");
  wm.addAP("AndroidAP6109", "evylapluscool");
  wm.addAP("AndroidAP3269", "14062001");
  // wm.addAP("XXXX-2a60", "QLCCWW5TQWHQ");
  // wm.addAP("IOT", "iotmiage");
  
  WiFi.mode(WIFI_OFF);   
  WiFi.mode(WIFI_STA); // Set WiFi to station mode 
  // delete old config
  // WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.disconnect(true); // Disconnect from an AP if it was previously connected
  // WiFi.persistent(false); // Avoid to store Wifi configuration in Flash
  
  // Define hostname  => in C str ! not C++
  WiFi.setHostname(hostname.c_str());
  
  Serial.println(String("\nAttempting to connect to SSIDs : "));
  while(wm.run() != WL_CONNECTED) {
    delay(SaveDisconnectTime);
    Serial.print(".");
  }
  
  if(wm.run() == WL_CONNECTED) {
    Serial.print("wifiMulti connected : Yes !");
  }
}