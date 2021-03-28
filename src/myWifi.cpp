#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>


#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include <FS.h>
#include "LittleFS.h" // LittleFS is declared

#include "myWifi.h"
#include "myTime.h"
#include "defines.h"
#include "config.h"

//WiFiClient net;
//WiFiClientSecure *mqttSecure = new WiFiClientSecure();

WiFiClientSecure *netSecure = new WiFiClientSecure();
BearSSL::Session session;  //for netSecure, to avoid ssl computation on each connection

BearSSL::X509List *netSecureCert;
  

//WiFiServer WebServer(80);
// Web Client
//WiFiClient client;

//for tests
/*const char *   host = "merluberlu.jbelec.pro";
const uint16_t port = 443;
const char *   path = "/datalogging/post";*/



BearSSL::X509List *getCACertFromFile(const char *filename) {
  //littleFs is started from main
  File cert = LittleFS.open(filename, "r");
  BearSSL::X509List *list = new BearSSL::X509List(cert, cert.size());
  cert.close();
  return list;
}

void myWifi_init(const char* SSID, const char* WIFIPWD) {
// Connect to WiFi network
  Serial.println("");
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  IPAddress local_IP(192,168,4,22); //AP
  IPAddress gateway(192,168,4,9); //AP
  IPAddress subnet(255,255,255,0); //AP

  WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
  WiFi.softAP(config.ap_ssid, config.ap_pwd);

  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, WIFIPWD);

  unsigned char test = 0;

  while (WiFi.status() != WL_CONNECTED && test < 20) {
    delay(500);
    Serial.print(".");
    test++;
  }

  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("Connected to WiFi");

    // Print the IP address
    Serial.printf("My IP address is: %s.\r\n", WiFi.localIP().toString().c_str());

    File cert = LittleFS.open("/cert.pem", "r");
    netSecureCert = new BearSSL::X509List(cert, cert.size());
    cert.close();
   // netSecureCert = getCACertFromFile("cert.pem");
  
    netSecure->setTrustAnchors(netSecureCert);
    netSecure->setSession(&session); 

  } else {
    Serial.println(F("Wifi NOT Connected. Please check AP to set the Wifi SSID and Password!"));
  }
}

