#ifndef CONFIG_H
#define CONFIG_H

//#include <sys/time.h>
#include <time.h>


extern const char *configFilename; 

struct Config {
  char project_name[64];
  char device_name[64];
  char ap_ssid[64];
  char ap_pwd[64];
  char sta_ssid[64];
  char sta_pwd[64];
  char data_srv_host[64];
  char data_srv_post_path[64];
  int data_srv_port;
  char data_srv_apikey[64];
  struct tm ligths_on_time;
  struct tm ligths_off_time;
};


extern Config config;  

bool loadConfiguration(const char *filename, Config &config);
bool saveConfiguration(const char *filename, const Config &config);



#endif