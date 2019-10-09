#include "IfraESP32SDK.h"

DynamicJsonDocument _doc(_capacity);


const  long  gmtOffset_sec = 3600;
const   int   daylightOffset_sec = 3600;

IfraESP32SDK::IfraESP32SDK(char* username, char* password)
{
    _username = username;
    _password = password;
    // _mqtt_client.setClient(_espClient);
    // _mqtt_client.setServer(IFRASERVER, MQTT_PORT);
    // _mqtt_client.setCallback(callback);
    // _pack.setBaseName("")
    _recordCount = 0;
    _base_name = "";
    _base_unit = "";
}

bool IfraESP32SDK::addAccessPoint(char* ssid, char* pass)
{
}
bool IfraESP32SDK::wifiConnection(char* ssid, char* pass)
{
    WiFi.begin(ssid, pass);
    Serial.print("Start connect wifi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    WiFi.setAutoReconnect(true);
    Serial.println(F("WiFi connected"));
    Serial.println(F("IP address: "));
    Serial.println(WiFi.localIP());

 
  
     //init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, NTP_SERVER);
    
}

bool IfraESP32SDK::mqttConnection(char* mqtt_topic)
{
}

void IfraESP32SDK::setBaseUnit(char* unit)
{
    _base_unit = unit;
}

void IfraESP32SDK::setBaseTime(unsigned long int time)
{
    _base_time = time;
}
void IfraESP32SDK::setBaseName(char* name)
{
    _base_name = name;
}

void IfraESP32SDK::addMeasurement(char* var_id, char* unit, float value)
{

    JsonObject doc = _doc.createNestedObject();
    if (_base_time != 0 && _recordCount == 0) {
        doc["bt"] = _base_time;
    }
    
    if (_base_name == "") {
        doc["n"] = var_id;
    }
    else if (_recordCount == 0) {
        doc["bn"] = var_id;
    }

    if (_base_unit == "") {
        doc["u"] = unit;
    }
    else if (_recordCount == 0) {
        doc["bu"] = unit;
    }

    doc["v"] = value;
    _recordCount++;
}

void IfraESP32SDK::addMeasurement(char* var_id, char* unit, float value, double time)
{

    JsonObject doc = _doc.createNestedObject();

    if (_base_time != 0 && _recordCount == 0) {
        doc["bt"] = _base_time;
    }else if (_recordCount != 0) {
        doc["t"] = time;
    }

    if (_base_name == "") {
        doc["n"] = var_id;
    }
    else if (_recordCount == 0) {
        doc["bn"] = var_id;
    }

    if (_base_unit == "") {
        doc["u"] = unit;
    }
    else if (_recordCount == 0) {
        doc["bu"] = unit;
    }

    doc["v"] = value;

   
    _recordCount++;
}

void IfraESP32SDK::send(char* toptic)
{

    _recordCount = 0;

    serializeJson(_doc, Serial);

    _doc.clear();
}

unsigned long int IfraESP32SDK::getTimestamp(void){
 
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        return 0;
    }
    //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    time_t epochUnix = mktime(&timeinfo);
    unsigned long int  b = static_cast<time_t>(epochUnix);
    return b;
}