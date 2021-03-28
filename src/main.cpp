#include <Arduino.h>

#include "LittleFS.h" 
#include "defines.h"
#include "config.h"
#include "myWifi.h"
#include "myLights.h"
#include "myDoor.h"
#include "web.h" 
//#include "ota.h"

#include "myTime.h"

bool reboot_asked = false;

// Setup GPIO2
int pinGPIO2 = 2; //To control LED
int ledStatus = 0; //0=off,1=on,2=dimmed




bool canStart;


void setup() {
  canStart = false;
  
  Serial.begin(115200);
  delay(10);
  Serial.println(F(""));
  Serial.println(F(""));
  Serial.println(CODE_VERSION);

  if(!LittleFS.begin()){
    Serial.println(F("An Error has occurred while mounting LittleFS"));
    Serial.println(F("Can't start the loop"));
    return;
  }

  if(!loadConfiguration(configFilename, config)){
    Serial.println(F("An Error has occurred while loading config file"));
    Serial.println(F("Can't start the loop"));
    return;
  }

  // contact pin conf
  pinMode(CONTACT_PIN, INPUT);
  digitalWrite(CONTACT_PIN, LOW);


 //on_off relay pin conf
  pinMode(ON_OFF_RELAY_PIN, OUTPUT);
  digitalWrite(ON_OFF_RELAY_PIN, LOW);
 //extra led pin conf
  pinMode(EXTRA_LED_PIN, OUTPUT);
  digitalWrite(EXTRA_LED_PIN, HIGH);
  
   //relay pin conf
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
 
  myWifi_init(config.sta_ssid, config.sta_pwd); //also set time if connected

  setTime();

  webServerSetup();

  enableLight();

  canStart = true;
 // OTASetup();
}


bool oldContactState = false;
bool contactState = false;

void loop() {
  // OTALoop();
  timeLoop();
  if (canStart) {

    if (NTP_TIME_SETTED) checkOnOff();

    if(digitalRead(CONTACT_PIN) == HIGH) {
      contactState = false;
    } else {
      contactState = true;
    }

    if (oldContactState == false && contactState == true) {
      //front montant
      Serial.println(F("FRONT MANTANT"));
      oldContactState = true;
      if (lightsOn) digitalWrite(RELAY_PIN, HIGH);
      doorClose = true;
      sendDoorStateToServer(1);
      sendDoorStateEventToBrowser(true);
    } else if (oldContactState == true && contactState == false) {
      // front descendant
      Serial.println(F("FRONT DESCENDANT"));
      oldContactState = false;

      if (lightsOn) digitalWrite(RELAY_PIN, LOW);
      doorClose = false;
      sendDoorStateToServer(0);
      sendDoorStateEventToBrowser(false);
    } /*else if (oldContactState == true && contactState == true) {
      // front haut
      //Serial.println("FRONT HAUT");
    }  else if (oldContactState == false && contactState == false) {
      //Serial.println("FRONT BAS");
      //front bas
    }*/

    if (reboot_asked)
      ESP.reset();

  }
    
}


