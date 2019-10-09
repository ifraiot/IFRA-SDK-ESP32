#include <WiFiMulti.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <kpn_senml.h> 
#include <iostream>

#ifndef MAX_RECORD
#define MAX_RECORD 2048
#endif

#ifndef IFRASERVER
#define IFRASERVER "hub.ifra.io"
#endif

#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif
 

const size_t _capacity = JSON_ARRAY_SIZE(500) + JSON_OBJECT_SIZE(10);

class IfraESP32SDK
{
private:
    char* _username;
    char* _password;
    WiFiClient _espClient;
    PubSubClient _mqtt_client;
    WiFiMulti _wifiMulti;
    int _recordCount;
    char* _base_name;
    char* _base_unit;
    unsigned long _base_time;
public:
    IfraESP32SDK(char* username, char* password);
    bool addAccessPoint(char *ssid, char *pass);
    bool wifiConnection(char *ssid, char *pass);
    bool mqttConnection(char *mqtt_topic);
    void addMeasurement(char *var_id, char *unit,float value);
    void addMeasurement(char *var_id, char *unit,float value, unsigned long time);
    void setBaseUnit(char *unit);
    void setBaseTime(unsigned long time);
    void setBaseName(char *name);
    void send(char *topic);
};
