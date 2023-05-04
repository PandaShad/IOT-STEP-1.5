/*** Basic/Static Wifi connection

     Fichier wificonnect/wifi_utils.h ***/

#include <WiFi.h> // https://www.arduino.cc/en/Reference/WiFi

String translateEncryptionType(wifi_auth_mode_t encryptionType);
void wifi_status();
