#include <Arduino.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <ESP8266WiFi.h>

#include "myTime.h"

bool NTP_TIME_SETTED = false;

unsigned long updateTime = 0;
unsigned long updateTimeInterval = 15 * 60 * 1000;

// Set time via NTP, as required for x.509 validation

void setTime() {
  if (WiFi.status() == WL_CONNECTED) {
    configTime(1 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    setenv("TZ","CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00",0);
    tzset();
    Serial.print("Waiting for NTP time sync: ");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
      delay(500);
      Serial.print(".");
      now = time(nullptr);
    }
    NTP_TIME_SETTED = true;
    Serial.println("");
    struct tm *timeinfo;
    timeinfo = localtime(&now);
    char stringTime[256] = {0};
    strftime(stringTime, 256, "%d/%m/%Y %H:%M:%S", timeinfo);
    Serial.print("Current time: ");
    Serial.println(stringTime);
  } else {
    Serial.println(F("Unable to set time, WiFi is NOT connected!"));
  }
}

struct tm *getTime() {
  //if (NTP_TIME_SETTED) {
    time_t now = time(nullptr);
    //gmtime_r(&now, &timeinfo); //utc
    return localtime(&now);
  //}
}

void timeLoop() {
  if (millis() > updateTime + updateTimeInterval) {
    setTime();
    updateTime = millis();
  }
}

