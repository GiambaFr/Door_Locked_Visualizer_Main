#ifndef MYWIFI_H
#define MYWIFI_H


#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>


// WiFi Router Login - change these to your router settings
extern const char* SSID;
extern const char* WIFIPWD;

extern WiFiClient net;
extern WiFiClientSecure *netSecure;
//extern WiFiClientSecure *mqttSecure;




#define TZ              1       // (utc+) TZ in hours
#define DST_MN          0      // use 60mn for summer time in some countries
#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

extern void myWifi_init(const char* SSID, const char* WIFIPWD);

BearSSL::X509List *getCACertFromFile(const char *filename);




#endif