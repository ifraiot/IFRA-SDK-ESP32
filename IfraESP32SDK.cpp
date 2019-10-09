#include "IfraESP32SDK.h"

DynamicJsonDocument _doc(_capacity);

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
}

bool IfraESP32SDK::mqttConnection(char* mqtt_topic)
{
}

void IfraESP32SDK::setBaseUnit(char* unit)
{
    _base_unit = unit;
}

void IfraESP32SDK::setBaseTime(unsigned long time)
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

void IfraESP32SDK::addMeasurement(char* var_id, char* unit, float value, unsigned long time)
{

    JsonObject doc = _doc.createNestedObject();

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

    doc["t"] = time;
    _recordCount++;
}

void IfraESP32SDK::send(char* toptic)
{

    _recordCount = 0;

    serializeJson(_doc, Serial);
}