#include <WiFiMulti.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <kpn_senml.h>
#include "time.h"
#include <stdio.h>
#include <ESP32OTA.h>
#include <string> 
#include <HTTPClient.h>
#ifndef MAX_RECORD
#define MAX_RECORD 2048
#endif

#ifndef IFRA_SERVER
#define IFRA_SERVER "hub.ifra.io"
#endif

#ifndef NTP_SERVER
#define NTP_SERVER "asia.pool.ntp.org"
#endif

#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif



//WiFi Auto Connect

// #if defined(ESP8266)
// #include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
// #else
// #include <WiFi.h>          //https://github.com/esp8266/Arduino
// #endif

//needed for library
#include <DNSServer.h>
#if defined(ESP8266)
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager


 
 
const size_t _capacity = JSON_ARRAY_SIZE(500) + JSON_OBJECT_SIZE(10);
typedef enum {
  Runnning_e = 0x01,
  Fota_e
} SysState;

class IfraESP32SDK
{
private:
    char* _username;
    char* _password;
    WiFiClient _espClient;
    PubSubClient _mqtt_client;
    WiFiMulti _wifiMulti;
    WiFiManager wifiManager;
    int _recordCount;
    char* _base_name;
    char* _base_unit;
    unsigned long int _base_time;
    void callback(char *topic, byte*payload, unsigned int length);
    void otaErrorCallback(char *message);
    void otaProgressCallback(DlState state, int percent);
    void startDownloadCallback(void);
    void endDownloadCallback(void);
    SysState _state;
    const  char* _ota_device_id ;
    const  char* _ota_url ;
    const  char* _ota_token  ;
    const  char* _callback_success;
    const  char* _is_wifi_reset;
    

public:
    IfraESP32SDK(char* username, char* password, char* server);
    IfraESP32SDK(char* username, char* password);
    IfraESP32SDK();
    bool addAccessPoint(char *ssid, char *pass);
    bool wifiConnection();
    bool wifiConnection(char *ssid, char *pass);
    bool mqttConnection(char *mqtt_topic);
    void addMeasurement(char *var_id, char *unit,float value);
    void addMeasurement(char *var_id, char *unit,float value, double time);
    void setBaseUnit(char *unit);
    void setBaseTime(unsigned long time);
    void setBaseName(char *name);
    void send(char *topic);
    unsigned long int getTimestamp(void);
    bool connected(void);
    void reconnect(void);
    void loop(void);
};


