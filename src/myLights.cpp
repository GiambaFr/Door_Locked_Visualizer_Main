#include <Arduino.h>
#include <time.h>
#include <sys/time.h>

#include "myLights.h"
#include "defines.h"
#include "config.h"
#include "myWifi.h"
#include "myTime.h"
#include "web.h"

#define HOUR 100
#define DAY (HOUR * 24)

bool lightsOn = false;


void checkOnOff() {
    //Check time for ON/OFF the ligth
  struct tm *curTime = getTime();
  unsigned int now = curTime->tm_hour * HOUR + curTime->tm_min;
  unsigned int start = config.ligths_on_time.tm_hour * HOUR + config.ligths_on_time.tm_min;
  unsigned int stop = config.ligths_off_time.tm_hour * HOUR + config.ligths_off_time.tm_min;
  //if (config.ligths_on_time.tm_hour > config.ligths_off_time.tm_hour)
   // Serial.printf("on=%d, cur: %d, start: %d, end: %d\r\n", lightsOn, now, start, stop);
  if( stop < start) 
  {
    stop += DAY ;
    if( lightsOn && now < start )
    {
        now += DAY ;
    }
  }

  if( !lightsOn && now >= start && now < stop )
  {
      enableLight() ;
  }
  // ... otherwise if ON, check for stop condition
  else if( lightsOn && now >= stop )
  {
      disableLight() ;
  }


}

void enableLight() {
  lightsOn = true;
  digitalWrite(EXTRA_LED_PIN, LOW);
  digitalWrite(ON_OFF_RELAY_PIN, HIGH);
  sendLightsOnOffToServer(1);
  sendLightsStateEventToBrowser(true);
  Serial.println(F("Lights enabled."));
}

void disableLight() {
  lightsOn = false;
  digitalWrite(EXTRA_LED_PIN, HIGH);
  digitalWrite(ON_OFF_RELAY_PIN, LOW);
  sendLightsOnOffToServer(0);
  sendLightsStateEventToBrowser(false);
  Serial.println(F("Lights disabled."));
}