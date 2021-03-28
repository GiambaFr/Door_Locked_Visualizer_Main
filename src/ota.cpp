#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "defines.h"
#include "ota.h"
#include "config.h"

void OTASetup() {
    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname(config.device_name);

    // No authentication by default
    ArduinoOTA.setPassword(OTA_PASSWORD);

    ArduinoOTA.onStart([]() {
        Serial.println("OTA Start");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nOTA End");
        ESP.reset();
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("OTA Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("OTA Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("OTA Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("OTA End Failed");
    });
    ArduinoOTA.begin();

}

void OTALoop() {
    ArduinoOTA.handle();
}