
#include "ESPAsyncWebServer.h" 
#include "routes.h"
#include "SPIFFS.h"

#include "ArduinoJson.h"

extern String target_ip, ssid, mac, ip, location;
extern int target_port, target_sp, target_sp;
extern short int Light_threshold;
extern unsigned long upTime;
extern boolean heater_on, cooler_on, is_fire, requestOnBoardLED, requestRedLED, requestGreenLED;
extern float last_temp, last_light, SHJ, SBJ, SBN, SHN;

/*===================================================*/
String processor(const String& var){
    /* Replaces "placeholder" in html file with sensors values */ /* accessors functions get_... are in sensors.ino file */
    if(var == "TEMPERATURE"){
        return String(last_temp);
        /* On aimerait écrire : return get_temperature(TempSensor);
        * mais c’est un exemple de ce qu’il ne faut surtout pas écrire ! 
        * yield + async => core dump !
        */
    //return get_temperature(TempSensor);
    }
    else if(var == "LIGHT") {
        return String(last_light);
    }
    else if(var == "SSID") {
        return ssid;
    }
    else if(var == "MAC") {
        return mac;
    }
    else if(var == "IP") {
        return ip;
    }
    else if(var == "UPTIME"){
        return String(upTime);
    }
    else if(var == "WHERE"){
        return location;
    }
    else if(var == "COOLER"){
        return cooler_on ? "ON" : "OFF";
    }
    else if(var == "HEATER"){
        return heater_on ? "ON" : "OFF";
    }
    else if(var == "FIRE"){
        return is_fire ? "YES" : "NO";
    }
    else if(var == "LT"){
        return String(Light_threshold);
    }
    else if(var == "SBJ"){
        return String(SBJ);
    }
    else if(var == "SHJ"){
        return String(SHJ);
    }
    else if(var == "SBN"){
        return String(SBN);
    }
    else if(var == "SHN"){
        return String(SHN);
    }
    else if(var == "PRT_IP"){
        return target_ip;
    }
    else if(var == "PRT_PORT"){
        return String(target_port);
    }
    else if(var == "PRT_T"){
        return String(target_sp);
    }
    return String();
}

/*===================================================*/
void setup_http_routes(AsyncWebServer* server) { /*
    * Sets up AsyncWebServer and routes */
    // doc => Serve files in directory "/" when request url starts with "/"
    // Request to the root or none existing files will try to server the default
    // file name "index.htm" if exists
    // => premier param la route et second param le repertoire servi (dans le SPIFFS) // Sert donc les fichiers css !
    server->serveStatic("/", SPIFFS, "/").setTemplateProcessor(processor);
    // Declaring root handler, and action to be taken when root is requested
    auto root_handler = server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    /* This handler will download index.html (stored as SPIFFS file) and will send it back */ 
        request->send(SPIFFS, "/index.html", String(), false, processor);
        // cf "Respond with content coming from a File containing templates" section in manual ! // https://github.com/me-no-dev/ESPAsyncWebServer
        // request->send_P(200, "text/html", page_html, processor); // if page_html was a string .
    });
    server->on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
        /* The most simple route => hope a response with temperature value */
        /* Exemple de ce qu’il ne faut surtout pas écrire car yield + async => core dump !*/ //request->send_P(200, "text/plain", get_temperature(TempSensor).c_str()); 
        request->send_P(200, "text/plain", String(last_temp).c_str());
    });
    server->on("/value", HTTP_GET, [](AsyncWebServerRequest *request){
        StaticJsonDocument<1024> doc;
        int paramsNr = request->params();
        Serial.println(paramsNr);
        for(int i=0; i<paramsNr; i++) {
            AsyncWebParameter* p = request->getParam(i);
            const String name = p->name();

            if (name == "temperature") {
                doc["temperature"] = last_temp;
            }
            else if (name == "light") {
                doc["light"] = last_light;
            }
            else if (name == "target_ip") {
                doc["target_ip"] = target_ip;
            }
            else if (name == "target_port") {
                doc["target_port"] = target_port;
            }
            else if (name == "sp") {
                doc["sp"] = target_sp;
            }
            else if (name == "uptime") {
                doc["uptime"] = upTime;
            }
            else if (name == "ssid") {
                doc["ssid"] = ssid;
            }
            else if (name == "mac") {
                doc["mac"] = mac;
            }
            else if (name == "ip_esp") {
                doc["ip_esp"] = ip;
            }
            else if (name == "loc") {
                doc["loc"] = location;
            }
        }
        if(doc.size()) {
            String response;
            serializeJson(doc, response);
             request->send(200, "application/json", response);
        } else {
             request->send(404);
        }
    });
    server->on("/light", HTTP_GET, [](AsyncWebServerRequest *request){ 
        /* The most simple route => hope a response with light value */ 
        request->send_P(200, "text/plain", String(last_light).c_str());
    });
    // This route allows users to change thresholds values through GET params
    server->on("/set", HTTP_GET, [](AsyncWebServerRequest *request){
    /* A route with a side effect : this get request has a param and should
    * set a new light_threshold ... used for regulation !
    */
        if (request->hasArg("led1")) {
            String value = request->arg("led1").c_str();
            requestOnBoardLED = value == "on" ? true : false;
            request->send_P(200, "text/plain", "OnBoard LET Set !");
        }
        if (request->hasArg("led2")) {
            String value = request->arg("led2").c_str();
            requestRedLED = value == "on" ? true : false;
            request->send_P(200, "text/plain", "OnBoard LET Set !");
        }
        if (request->hasArg("led3")) {
            String value = request->arg("led3").c_str();
            requestGreenLED = value == "on" ? true : false;
            request->send_P(200, "text/plain", "OnBoard LET Set !");
        }
        if (request->hasArg("light_threshold")) { // request may have arguments
            Light_threshold = atoi(request->arg("light_threshold").c_str());
            request->send_P(200, "text/plain", "Threshold Set !"); 
        }
    });
    server->on("/target", HTTP_POST, [](AsyncWebServerRequest *request){ 
        /* A route receiving a POST request with Internet coordinates
        * of the reporting target host.
        */
        Serial.println("Receive Request for a periodic report !");
        if (request->hasArg("ip") && request->hasArg("port") && request->hasArg("sp")) {
            target_ip = request->arg("ip");
            target_port = atoi(request->arg("port").c_str()); 
            target_sp = atoi(request->arg("sp").c_str());
        }
        request->send(SPIFFS, "/index.html", String(), false, processor); 
    });
    // If request doesn’t match any route, returns 404.
    server->onNotFound([](AsyncWebServerRequest *request){ 
        request->send(404);
    });
}
