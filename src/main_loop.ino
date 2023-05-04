#include <Arduino.h>
#include <ArduinoJson.h>

#include "wifi_utils.h"
#include "wificonnect.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiMulti.h>

#include "HTTPClient.h"

#define USE_SERIAL Serial

unsigned long lastTime = 0;
// Set timer 
unsigned long loop_period = 10L * 1000; /* =>  10000ms : 10 s */

/*--------------------------------------------------------------------------* 
  httpGETRequest using WiFi.h
*--------------------------------------------------------------------------*/ 

// void httpGETRequest(const char* httpServer, const int httpPort = 80) {
//   WiFiClient client;
//   USE_SERIAL.printf("=================================\n");
//   if (client.connect(httpServer, httpPort) == 0) {
//     USE_SERIAL.printf("Requesting URL : %s => failed !\n", httpServer);
//     return;
//   } else{
//     USE_SERIAL.printf("URL %s connected !\n", httpServer);
//   }
//   String url = "/ip";
//   String req = String("GET "); // Now create HTTP request header INCLUDING URL
//   req += url + " HTTP/1.1\r\n";
//   req += "host: " + String(httpServer) + "\r\n";
//   req += "User-Agent: esp-idf/1.0 esp32 \r\n";
//   req += "Accept: */* \r\n";
//   req += "Connection: close \r\n";
//   req += "\r\n"; // empty line : separator header/body

//   USE_SERIAL.printf("----------------------------------------\n");
//   USE_SERIAL.printf("Request : \n %s",req.c_str());
//   USE_SERIAL.printf("----------------------------------------\n");

//   client.print(req);

//   unsigned long timeout = millis();
//   while (client.available() == 0) { // no answer => timeout mechanism !
//     if (millis() - timeout > 10000) {
//       USE_SERIAL.println(">>> Client Timeout ! => Disconnection !");
//       client.stop(); // disconnect
//       return;
//     }
//   }

//   USE_SERIAL.println("Response : ");
// // all the lines of the reply from server and print them to Serial
//   while (client.available()) { // Returns the number of bytes available for reading
//     String line = client.readStringUntil('\r'); USE_SERIAL.print(line); // echo to console "this" line
//     // en version car by car
//     //char c = client.read();
//     //USE_SERIAL.print(c);
//   }
//   // End connection and Free resources
//   USE_SERIAL.println("Closing connection from client side");
//   client.stop(); // disconnect
// }

/*--------------------------------------------------------------------------* 
  httpGETRequest using ArduinoHtppClient.h
*--------------------------------------------------------------------------*/ 

String httpGETRequest(const char* UrlServer) {
  String payload = "{}";
  HTTPClient http;
  USE_SERIAL.printf("Requesting URL : %s\n", UrlServer);
  http.begin(UrlServer);
  int httpResponseCode = http.GET();
  if (httpResponseCode>0) {
    USE_SERIAL.printf("HTTP Response code : %d", httpResponseCode); payload = http.getString();
  }
  else {
    USE_SERIAL.printf("Error code on HTTP GET Request : %d", httpResponseCode);
  }
  http.end();
  return payload;
}

/*--------------------------------------------------------------------------* 
  Arduino IDE paradigm : setup+loop 
  *--------------------------------------------------------------------------*/ 
void setup(){
  USE_SERIAL.begin(9600);
  while(!USE_SERIAL); //wait for a serial connection  

  /* WiFi connection  -----------------------*/
  String hostname = "Mon petit objet ESP32";
  wifi_connect_multi(hostname);

  /* WiFi status    --------------------------*/
  if (WiFi.status() == WL_CONNECTED) {
    USE_SERIAL.println("WiFi connected : yes");
    wifi_status();
  }
  else {
    USE_SERIAL.println("WiFi connected : no");
  }
  // Default Credentials 
  // String ssid = String("AndroidAP6109");
  // String passwd = String("evylapluscool");

  // String ssid = String("XXXX-2a60");
  // String passwd = String("QLCCWW5TQWHQ");
}

void loop(){
  char httpServer[100] = "http://httpbin.org"; // = "http://worldtimeapi.org/";
  const int httpPort = 80;

  String path = "/get";
  String params = "?led1=""OFF""&led2=""ON""";
  String url = String(httpServer)+path+params;
  if ((millis() - lastTime) > loop_period) { //Check WiFi connection status 
    if(WiFi.status()== WL_CONNECTED){
      String ret = httpGETRequest(url.c_str());
      USE_SERIAL.println(ret);
    }
    else {
      USE_SERIAL.println("WiFi Disconnected");
    }
      lastTime = millis();
    }
  // httpGETRequest(httpServer, httpPort);
  // delay(10000); //ms
}