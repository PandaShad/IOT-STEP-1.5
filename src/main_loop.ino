#include <Arduino.h>
#include <ArduinoJson.h>

#include "ArduinoOTA.h"

#include "wifi_utils.h"
#include "wificonnect.h"

#include "WiFi.h"
#include "HTTPClient.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "routes.h"

#include "utils.h"

#define USE_SERIAL Serial

unsigned long upTime = 0;
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

// String httpGETRequest(const char* UrlServer) {
//   String payload = "{}";
//   HTTPClient http;
//   USE_SERIAL.printf("Requesting URL : %s\n", UrlServer);
//   http.begin(UrlServer);
//   int httpResponseCode = http.GET();
//   if (httpResponseCode>0) {
//     USE_SERIAL.printf("HTTP Response code : %d", httpResponseCode); payload = http.getString();
//   }
//   else {
//     USE_SERIAL.printf("Error code on HTTP GET Request : %d", httpResponseCode);
//   }
//   http.end();
//   return payload;
// }

/*--------------------------------------------------------------------------* 
  ESP as a HTTP server 4.3.1
*--------------------------------------------------------------------------*/ 

// WiFiServer server(80);

void httpReply(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close"); // the connection will be closed after completion of the response 
  client.println("Refresh: 5"); // refresh the page automatically every 5 sec
  client.println(); // Empty line between header and body
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.print("Hello, je tourne depuis : "); // Returns the ms passed since the ESP began running the current program. 
  client.print(millis()/1000); // On pourrait sans doute donner une info
  client.println("s <br />"); // plus pertinente ? temperature ?
  client.println("</html>");
}

/*--------------------------------------------------------------------------* 
  ESP as a HTTP server with ESPAsyncWebServer (4.6)
*--------------------------------------------------------------------------*/ 

void setup_OTA();

const int GREEN_LED_PIN = 19;
const int ONBOARD_LED=2;
const int RED_LED_PIN=21;
const int FAN_PIN=27;
const int LightPin = A5;

float SHJ = 30;
float STH = 30.0;
float SBJ = 20;

float SBN = 20;
float SHN = 25;

OneWire oneWire(23);
DallasTemperature TempSensor(&oneWire);

String LEDState = "off";
float last_temp;
float last_light;
String ssid, mac, ip, location;

AsyncWebServer server(80);

short int Light_threshold = 250;

boolean heater_on = false;
boolean cooler_on = false;
boolean is_fire = false;

boolean requestOnBoardLED, requestRedLED, requestGreenLED = false;

String target_ip = "http://192.168.43.9";
int target_port = 1880;
int target_sp = 0; // remaining time before the ESP stops transmitting

void DoSmth(int d) {
  static uint32_t tick = 0;
  if (millis() - tick < d) {
    return;
  }
  else {
    USE_SERIAL.println("deja 10 secondes écoulées !");
    tick = millis();
    last_temp = get_temperature(TempSensor);
    last_light = get_light(LightPin);
    heater_on = get_heater_status(last_temp, SBJ);
    cooler_on = get_cooler_status(last_temp, SHJ);
    is_fire = get_fire_status(last_light);
    sendPostRequest(target_ip, target_port, target_sp);
  }
}

void updateLeds(int temperature) {
  digitalWrite(ONBOARD_LED, requestOnBoardLED ? HIGH : LOW);
  if(heater_on) {
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
  } else if (cooler_on) {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
  } else {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
  }
}

String getLocation() {
  HTTPClient http;
  String url = "http://ip-api.com/json/";
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) { 
    String payload = http.getString();
    StaticJsonDocument<1024> data;
    deserializeJson(data, payload);
    String city = data["city"];
    String country = data["country"];
    return city + ", " + country;
  } else {
    return "Unknown";
  }
  http.end();
}

void sendPostRequest(String ip, int port, int sp) {
  if (!target_sp == 0) {
    String url = ip + ":" + port + "/esp?mac=" + mac;
    USE_SERIAL.println("SHEESH : " + url);
    String data = Serialize_ESPstatus();
    USE_SERIAL.println("SHEESH data : " + data);
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int response = http.POST(data);
    USE_SERIAL.println("SHEESH repsonse : " + response);
    http.end();
    target_sp -= 1;
  }
  return;
}

String Serialize_ESPstatus(){
  StaticJsonDocument<1024> jsondoc;
  jsondoc["status"]["temperature"] = String(last_temp); // Temp value
  jsondoc["status"]["light"] = String(last_light); // Light value
  jsondoc["status"]["heat"] = heater_on; // Heater
  jsondoc["status"]["cold"] = cooler_on; // Cooler
  jsondoc["status"]["fire"] = is_fire; // NO or YES 
  jsondoc["regul"]["sh"] = SHJ; // Seuil Haut
  jsondoc["regul"]["sb"] = SBJ;  // Seuil bas
  jsondoc["regul"]["shn"] = SHN; // Seuil Haut
  jsondoc["regul"]["sbn"] = SBN;  // Seuil bas
  jsondoc["info"]["uptime"] = upTime;
  jsondoc["info"]["ssid"] = ssid; 
  jsondoc["info"]["ident"] = mac;
  jsondoc["info"]["ip"] = ip;
  jsondoc["reporthost"]["target_ip"] = target_ip; // utile quand on se sera connecté .. pour l'instant inutile
  jsondoc["reporthost"]["target_port"] = target_port; // utile quand on se sera connecté .. pour l'instant inutile
  jsondoc["reporthost"]["sp"] = target_sp; // utile quand on se sera connecté .. pour l'instant inutile
 
  String data = "";
  serializeJson(jsondoc, data);
  return data;
}

void setup(){
  /* Serial connection -----------------------*/ 
  USE_SERIAL.begin(9600);
  while(!USE_SERIAL); //wait for a serial connection
  /* WiFi connection -----------------------*/
  String hostname = "Mon petit objet ESP32";
  wifi_connect_multi(hostname);
/* WiFi status --------------------------*/
  if (WiFi.status() == WL_CONNECTED){ 
    USE_SERIAL.print("\nWiFi connected : yes ! \n"); wifi_status();
  }
  else {
    USE_SERIAL.print("\nWiFi connected : no ! \n");
    // ESP.restart();
  }
  // OTA //setup_OTA();
  // Initialize the LED
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  // Init temperature sensor 
  TempSensor.begin();
  // Initialize SPIFFS
  SPIFFS.begin(true);
  // Setup routes of the ESP Web server
  setup_http_routes(&server);
  server.begin();

  location = getLocation();
  ssid = String(WiFi.SSID());
  mac = String(WiFi.macAddress());
  ip = WiFi.localIP().toString();
}

void loop(){
  int delai = 10000;
  upTime = millis() / 1000; // convert millis in seconds
  /* OTA*/
  //ArduinoOTA.handle();  
  /* Update sensors */
  DoSmth(delai);
  //delay(loop_period); // ms
}

/*--------------------------------------------------------------------------* 
  Arduino IDE paradigm : setup+loop 
  *--------------------------------------------------------------------------*/ 
// void setup(){
//   USE_SERIAL.begin(9600);
//   while(!USE_SERIAL); //wait for a serial connection  
//   /* WiFi connection  -----------------------*/
//   String hostname = "Mon petit objet ESP32";
//   wifi_connect_multi(hostname);
//   /* WiFi status    --------------------------*/
//   if (WiFi.status() == WL_CONNECTED) {
//     USE_SERIAL.println("WiFi connected : yes");
//     wifi_status();
//   }
//   else {
//     USE_SERIAL.println("WiFi connected : no");
//   }
//   server.begin();
// }

// void loop() {
//   WiFiClient client = server.available();
//   if(client) {
//     Serial.println("New client is connecting !"); 
//     // an http request ends with a blank line CRLF
//     boolean currentLineIsBlank = true; 
//     while (client.connected()) {
//       if (client.available()) {
//         char c = client.read();
//         Serial.write(c); // Echo the request on the console
//         if (c == '\n' && currentLineIsBlank) { 
//           httpReply(client); // so you can send a reply break;
//         }
//         if (c == '\r') { // you’re starting a new line
//           currentLineIsBlank = true;
//         } else if (c != '\r') { // you’ve gotten a character on the current line
//           currentLineIsBlank = false; 
//         }
//       } // end if 
//     }// end while
//     delay(loop_period);
//     client.stop();
//     Serial.println("client disconnected");
//   }
// }

// void loop(){
//   char httpServer[100] = "http://httpbin.org"; // = "http://worldtimeapi.org/";
//   const int httpPort = 80;
//   String path = "/get";
//   String params = "?led1=""OFF""&led2=""ON""";
//   String url = String(httpServer)+path+params;
//   if ((millis() - upTime) > loop_period) { //Check WiFi connection status 
//     if(WiFi.status()== WL_CONNECTED){
//       String ret = httpGETRequest(url.c_str());
//       USE_SERIAL.println(ret);
//     }
//     else {
//       USE_SERIAL.println("WiFi Disconnected");
//     }
//       upTime = millis();
//     }
//   // httpGETRequest(httpServer, httpPort);
//   // delay(10000); //ms
// }