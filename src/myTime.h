#ifndef TIME_H
#define TIME_H

#include <sys/time.h>

extern bool NTP_TIME_SETTED; 

extern void setTime();

extern void timeLoop();

extern struct tm *getTime();

#endif