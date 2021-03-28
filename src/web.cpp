#include <Arduino.h>
#include "LittleFS.h"
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESP8266HTTPClient.h>

#include "defines.h"
#include "web.h"
#include "config.h"
#include "myLights.h"
#include "myDoor.h"
#include "myWifi.h"

#define AUTHENTICATION_USER "user"
#define AUTHENTICATION_PASS "pass"

DNSServer dnsServer;
AsyncWebServer server(80);
AsyncEventSource events("/events");



void onNotFound(AsyncWebServerRequest *request){
  //Handle Unknown Request
  request->send(404);
}

String processor(const String& var)
{
  if(var == "HELLO_FROM_TEMPLATE")
    return F("Hello world!");
  else if(var == "PROJECT_TITLE")
    return config.project_name;
  else if(var == "WIFI_SSID")
    return config.sta_ssid;
  else if(var == "WIFI_PWD")
    return config.sta_pwd;
  else if (var == "DATA_SRV_HOST")
    return config.data_srv_host;
  else if (var == "DATA_SRV_PORT")
    return String(config.data_srv_port);
  else if (var == "DATA_SRV_PATH")
    return config.data_srv_post_path;
  else if (var == "DATA_SRV_APIKEY")
    return config.data_srv_apikey;
  else if (var == "LIGHTS_ON_TIME") {
    char buf[6];
    strftime(buf, 6, "%H:%M", &config.ligths_on_time);
    return String(buf);
  }
  else if (var == "LIGHTS_OFF_TIME") {
    char buf[6];
    strftime(buf, 6, "%H:%M", &config.ligths_off_time);
    return String(buf);
  } else if (var == "LIGHTS_ON") {
    if(lightsOn)
      return "ON";
    else
      return "OFF";
  } else if (var == "DOOR_STATE") {
    if(!doorClose)
      return "OPEN";
    else
      return "CLOSE";
  } else if (var == "START") {
    char buf[6];
    strftime(buf, 6, "%H:%M", &config.ligths_on_time);
    return String(buf);
  } else if (var == "STOP") {
    char buf[6];
    strftime(buf, 6, "%H:%M", &config.ligths_off_time);
    return String(buf);
  }

  return String();
}

void webServerSetup() {
  
  dnsServer.start(53, "*", WiFi.softAPIP());

//littleFs is started from main
  server.onNotFound(onNotFound);

  server
    .serveStatic("/wifi_settings.html", LittleFS, "/www/wifi_settings.html")
    .setTemplateProcessor(processor)
    .setAuthentication(AUTHENTICATION_USER, AUTHENTICATION_PASS)
    .setFilter(ON_STA_FILTER);
  server
    .serveStatic("/wifi_settings.html", LittleFS, "/www/wifi_settings.html")
    .setTemplateProcessor(processor)
    .setFilter(ON_AP_FILTER);
  server.on("/post_config", HTTP_POST, [](AsyncWebServerRequest *request){
    if(!request->authenticate(AUTHENTICATION_USER, AUTHENTICATION_PASS))
        return request->requestAuthentication();
    Serial.println(F("Config received from webserver POST"));
      if (request->hasParam("WIFI_SSID", true)) {
        strlcpy(config.sta_ssid, request->getParam("WIFI_SSID", true)->value().c_str(), sizeof(config.sta_ssid));
      }
      if (request->hasParam("WIFI_PWD", true)) {
        strlcpy(config.sta_pwd, request->getParam("WIFI_PWD", true)->value().c_str(), sizeof(config.sta_pwd));
      }
      if (request->hasParam("DATA_SRV_HOST", true)) {
        strlcpy(config.data_srv_host, request->getParam("DATA_SRV_HOST", true)->value().c_str(), sizeof(config.data_srv_host));
      }
      if (request->hasParam("DATA_SRV_PORT", true)) {
        config.data_srv_port = atoi(request->getParam("DATA_SRV_PORT", true)->value().c_str());
      }
      if (request->hasParam("DATA_SRV_PATH", true)) {
        strlcpy(config.data_srv_post_path, request->getParam("DATA_SRV_PATH", true)->value().c_str(), sizeof(config.data_srv_post_path));
      }
      if (request->hasParam("DATA_SRV_APIKEY", true)) {
        strlcpy(config.data_srv_apikey, request->getParam("DATA_SRV_APIKEY", true)->value().c_str(), sizeof(config.data_srv_apikey));
      }
      if (request->hasParam("LIGHTS_ON_TIME", true)) {
        char hour[3] = {0}; //2 + null char
        char min[3] = {0}; //2 + null char
        char onTimeString[6]; //5 + null char
        strncpy(onTimeString, request->getParam("LIGHTS_ON_TIME", true)->value().c_str(), sizeof(onTimeString));
        strncpy(hour, onTimeString, 2);
        strncpy(min, onTimeString+3, 2);
        Serial.printf("Lights Start Time setted at %s:%s\r\n",hour, min);
        config.ligths_on_time.tm_hour = atoi(hour);
        config.ligths_on_time.tm_min = atoi(min);
      }
      if (request->hasParam("LIGHTS_OFF_TIME", true)) {
        char hour[3] = {0}; //2 + null char
        char min[3] = {0}; //2 + null char
        char offTimeString[6]; //5 + null char
        strncpy(offTimeString, request->getParam("LIGHTS_OFF_TIME", true)->value().c_str(), sizeof(offTimeString));
        strncpy(hour, offTimeString, 2);
        strncpy(min, offTimeString+3, 2);
        Serial.printf("Lights Stop Time setted at %s:%s\r\n", hour, min);
        config.ligths_off_time.tm_hour = atoi(hour);
        config.ligths_off_time.tm_min = atoi(min);
        sendStartStopTimesEventToBrowser();
      }
      if(!saveConfiguration(configFilename, config)){
          Serial.println(F("An Error has occurred while writing config file"));
      }
      //request->send(200, "text/plain", "Hello, POST: ");
      request->redirect(F("wifi_settings.html"));

  });
  server.on("/config.json", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(AUTHENTICATION_USER, AUTHENTICATION_PASS))
        return request->requestAuthentication();
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/config.json", String(), true);
    //response->addHeader("Content-Disposition", "attachment");
    request->send(response);
  });
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(AUTHENTICATION_USER, AUTHENTICATION_PASS))
        return request->requestAuthentication();
    String json = "[";
    int n = WiFi.scanComplete();
    if(n == -2){
      WiFi.scanNetworks(true);
    } else if(n){
      for (int i = 0; i < n; ++i){
        if(i) json += ",";
        json += "{";
        json += "\"rssi\":"+String(WiFi.RSSI(i));
        json += ",\"ssid\":\""+WiFi.SSID(i)+"\"";
        json += ",\"bssid\":\""+WiFi.BSSIDstr(i)+"\"";
        json += ",\"channel\":"+String(WiFi.channel(i));
        json += ",\"secure\":"+String(WiFi.encryptionType(i));
        json += ",\"hidden\":"+String(WiFi.isHidden(i)?"true":"false");
        json += "}";
      }
      WiFi.scanDelete();
      if(WiFi.scanComplete() == -2){
        WiFi.scanNetworks(true);
      }
    }
    json += "]";
    request->send(200, "application/json", json);
    json = String();
  });
  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(AUTHENTICATION_USER, AUTHENTICATION_PASS))
        return request->requestAuthentication();
    request->redirect("wifi_settings.html");
    reboot_asked = true;
  });
  server
    .serveStatic("/", LittleFS, "/www/")
    .setDefaultFile("index.html")
    .setTemplateProcessor(processor);

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
    }
    //send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!",NULL,millis(),1000);
  });
  //HTTP Basic authentication
  //events.setAuthentication("user", "pass");
  server.addHandler(&events);

  //=========================== Démarrer le serveur ============================
  server.begin(); 
  Serial.println(F("WebServer started"));
}   

void webServerLoop() {

  //server.handleClient();  // gérer les requêtes

}

void sendDoorStateToServer(int value) {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient https;
    char message[255];
    sprintf(message, "state=%d", value);
    Serial.printf("POST %s to %s%s on port %d -> ", message, config.data_srv_host, config.data_srv_post_path, config.data_srv_port);
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    if (https.begin(*netSecure, config.data_srv_host, config.data_srv_port, config.data_srv_post_path, (config.data_srv_port == 443 ? true:false))) {
      https.addHeader("Content-Type", "application/x-www-form-urlencoded");
      https.addHeader("X-APIKEY", String(config.data_srv_apikey));
      int httpCode = https.POST(message);
      String payload = https.getString();
      Serial.printf("%d: %s\r\n", httpCode, payload.c_str());
      https.end();
    } else {
      Serial.println(F("[HTTPS] Unable to connect"));
    }
  } else {
    Serial.println(F("Can't send request to server, WiFi NOT connected!"));
  }
}

void sendLightsOnOffToServer(int value) {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient https;
    char message[255];
    sprintf(message, "lightsOnOff=%d", value);
    Serial.printf("POST %s to %s%s on port %d -> ", message, config.data_srv_host, config.data_srv_post_path, config.data_srv_port);
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    if (https.begin(*netSecure, config.data_srv_host, config.data_srv_port, config.data_srv_post_path, (config.data_srv_port == 443 ? true:false))) {
      https.addHeader("Content-Type", "application/x-www-form-urlencoded");
      https.addHeader("X-APIKEY", String(config.data_srv_apikey));
      int httpCode = https.POST(message);
      String payload = https.getString();
      Serial.printf("%d: %s\r\n", httpCode, payload.c_str());
      https.end();
    } else {
      Serial.println(F("[HTTPS] Unable to connect"));
    }
  } else {
    Serial.println(F("Can't send request to server, WiFi NOT connected!"));
  }
}

void sendDoorStateEventToBrowser(bool doorClose) {
  char state[6];
  if (doorClose)
    strcpy(state, "close");
  else 
    strcpy(state, "open");
  events.send(state,"doorClose",millis());
}

void sendLightsStateEventToBrowser(bool lightsOn) {
  char state[4];
  if (lightsOn)
    strcpy(state, "on");
  else 
    strcpy(state, "off");
  events.send(state,"lightsOn",millis());
}

void sendStartStopTimesEventToBrowser() {
  char timeStartStr[6] = {0};
  char timeStopStr[6] = {0};
  char message[46] = {0};

  strftime(timeStartStr, 6, "%H:%M", &config.ligths_on_time);
  strftime(timeStopStr, 6, "%H:%M", &config.ligths_off_time);
  sprintf(message, "{\"start\":\"%s\", \"stop\":\"%s\"}", timeStartStr, timeStopStr);

  events.send(message,"start_stop",millis());
}