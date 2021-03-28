#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>

#include <string.h>
#include <stdlib.h>
#include "config.h"

const char *configFilename = "/config.json"; 
Config config;


bool loadConfiguration(const char *filename, Config &config) {

  File configFile = LittleFS.open(filename, "r");
  if (!configFile) {
    Serial.println(F("Failed to open config file"));
    return false;
  }

  size_t size = configFile.size();
  //Serial.println(size);
  if (size > 1024) {
    Serial.println(F("Config file size is too large"));
    return false;
  }

  // Allocate a buffer to store contents of the file.
 // std::unique_ptr<char[]> buf(new char[size]);
  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  //configFile.readBytes(buf.get(), size);

  StaticJsonDocument<1024> doc;
  // DynamicJsonDocument doc(2048);
    // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));



  // Copy values from the JsonDocument to the Config
  strlcpy(config.project_name,         doc["project_name"] | "project_name",            sizeof(config.project_name)); 
  strlcpy(config.device_name,         doc["device_name"] | "device_name",               sizeof(config.device_name));  
  strlcpy(config.ap_ssid,             doc["ap_ssid"] | "JB_ESP",                        sizeof(config.ap_ssid));  
  strlcpy(config.ap_pwd,              doc["ap_pwd"] | "jb10061981",                     sizeof(config.ap_pwd));  
  strlcpy(config.sta_ssid,            doc["sta_ssid"] | "SFR_6A10",                     sizeof(config.sta_ssid));  
  strlcpy(config.sta_pwd,             doc["sta_pwd"] | "bonniejb2018",                  sizeof(config.sta_pwd));  
  strlcpy(config.data_srv_host,       doc["data_srv_host"] | "merluberlu.jbelec.pro",   sizeof(config.data_srv_host));  
  config.data_srv_port =        (int) doc["data_srv_port"] | 443;
  strlcpy(config.data_srv_post_path,  doc["data_srv_post_path"] | "/datalogging/post",  sizeof(config.data_srv_post_path));  
  strlcpy(config.data_srv_apikey,     doc["data_srv_apikey"] | "2bd86541-8464-4648-bc83-4c4c430a6768",       sizeof(config.data_srv_apikey));  
  config.ligths_on_time.tm_hour = (int) doc["ligths_on_time_h"] | 0;
  config.ligths_on_time.tm_min = (int) doc["ligths_on_time_mn"] | 0;
  config.ligths_off_time.tm_hour = (int) doc["ligths_off_time_h"] | 0;
  config.ligths_off_time.tm_min = (int) doc["ligths_off_time_mn"] | 0;

  Serial.print(F("Device name: "));
  Serial.println(config.device_name);

  configFile.close();
  return true;
}

bool saveConfiguration(const char *filename, const Config &config) {
// Delete existing file, otherwise the configuration is appended to the file
  LittleFS.remove(filename);

  // Open file for writing
  File configFile = LittleFS.open(filename, "w");
  if (!configFile) {
    Serial.println(F("Failed to create file"));
    return false;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<1024> doc;

  // Set the values in the document
  doc["project_name"]         = config.project_name;
  doc["device_name"]          = config.device_name;
  doc["ap_ssid"]              = config.ap_ssid;
  doc["ap_pwd"]               = config.ap_pwd;
  doc["sta_ssid"]             = config.sta_ssid;
  doc["sta_pwd"]              = config.sta_pwd;
  doc["data_srv_host"]        = config.data_srv_host;
  doc["data_srv_port"]        = config.data_srv_port;
  doc["data_srv_post_path"]   = config.data_srv_post_path;
  doc["data_srv_apikey"]      = config.data_srv_apikey;
  doc["ligths_on_time_h"]     = config.ligths_on_time.tm_hour;
  doc["ligths_on_time_mn"]     = config.ligths_on_time.tm_min;
  doc["ligths_off_time_h"]     = config.ligths_off_time.tm_hour;
  doc["ligths_off_time_mn"]     = config.ligths_off_time.tm_min;

  Serial.printf("%d:%d %d:%d\r\n", (config.ligths_on_time.tm_hour), (config.ligths_on_time.tm_min), (config.ligths_off_time.tm_hour), (config.ligths_off_time.tm_min));

  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file"));
    return false;
  }
  configFile.close();
  // Close the file
  return true;
}

