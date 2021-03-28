#ifndef WEB_H
#define WEB_H

#include <Arduino.h>

String processor(const String& var);


void webServerSetup();

void webServerLoop();

extern void sendDoorStateToServer(int value);
extern void sendLightsOnOffToServer(int value);

extern void sendDoorStateEventToBrowser(bool doorClose);
extern void sendLightsStateEventToBrowser(bool lightsOn);

extern void sendStartStopTimesEventToBrowser();

#endif