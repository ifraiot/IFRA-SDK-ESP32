#include "IfraESP32SDK.h"

DynamicJsonDocument _doc(_capacity);
DynamicJsonDocument _docMQTT(1024);
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
 

IfraESP32SDK::IfraESP32SDK(char* username, char* password, char* server)
{
    _username = username;
    _password = password;
    _mqtt_client.setClient(_espClient);
    _mqtt_client.setServer(server, MQTT_PORT);
    _recordCount = 0;
    _base_name = "";
    _base_unit = "";
}

IfraESP32SDK::IfraESP32SDK(char* username, char* password)
{
    _username = username;
    _password = password;
    _mqtt_client.setClient(_espClient);
    _mqtt_client.setServer(IFRA_SERVER, MQTT_PORT);
    _recordCount = 0;
    _base_name = "";
    _base_unit = "";
}

bool IfraESP32SDK::addAccessPoint(char* ssid, char* pass)
{
}

void IfraESP32SDK::callback(char *topic, byte *payload, unsigned int length) {
  payload[length] = '\0';
  String topic_str = topic, payload_str = (char*)payload;
  Serial.println("[" + topic_str + "]: " + payload_str);

  deserializeJson(_docMQTT,  (char*)payload);
  const  char* device_id =  _docMQTT["device_id"];
 Serial.println(device_id);
 Serial.println(_username);
 Serial.println(device_id ==_username);
  if(device_id ==_username){
     Serial.println("Start Download Firmware!!");
     digitalWrite(13, LOW);
     delay(10000);
     digitalWrite(13, HIGH);
  }
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

    //MQTT callback
    _mqtt_client.setCallback([this] (char* topic, byte* payload, unsigned int length) { 
          callback(topic, payload, length); 
     });
   
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
    }
    else if (_recordCount != 0) {
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
    if (_mqtt_client.connected()) {
           char message[4096];
        serializeJson(_doc, message);
        _mqtt_client.publish(toptic, message);
        _mqtt_client.loop();
        //Serial.println(message);
    }


    _doc.clear();
    _recordCount = 0;
}

unsigned long int IfraESP32SDK::getTimestamp(void)
{

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return 0;
    }
    //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    time_t epochUnix = mktime(&timeinfo);
    unsigned long int b = static_cast<time_t>(epochUnix);
    return b;
}

bool IfraESP32SDK::connected(void) {
   return  _mqtt_client.connected();
}

void IfraESP32SDK::reconnect(void) {

 // Loop until we're reconnected
  while (!_mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (_mqtt_client.connect(_username, _username, _password)) {
      Serial.println("connected");
      _mqtt_client.subscribe("OTA/3");
   
    } else {
      Serial.print("failed, rc=");
      Serial.print(_mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    
    }
  }
  _mqtt_client.loop();

}
