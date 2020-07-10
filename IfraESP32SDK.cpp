#include "IfraESP32SDK.h"

DynamicJsonDocument _doc(_capacity);
DynamicJsonDocument _docMQTT(1024);
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
IfraESP32SDK::IfraESP32SDK(){
        
}
IfraESP32SDK::IfraESP32SDK(char * username, char * password, char * server) {
        _state = Runnning_e;
        _username = username;
        _password = password;
        _mqtt_client.setClient(_espClient);
        _mqtt_client.setServer(server, MQTT_PORT);
        _recordCount = 0;
        _base_name = "";
        _base_unit = "";
}

IfraESP32SDK::IfraESP32SDK(char * username, char * password) {
        _state = Runnning_e;
        _username = username;
        _password = password;
        _mqtt_client.setClient(_espClient);
        _mqtt_client.setServer(IFRA_SERVER, MQTT_PORT);
        _recordCount = 0;
        _base_name = "";
        _base_unit = "";
}

bool IfraESP32SDK::addAccessPoint(char * ssid, char * pass) {}
void otaErrorCallback(char * message) {
        Serial.println(message);
}

void IfraESP32SDK::otaProgressCallback(DlState state, int percent) {

        String topic = "PROGRESS/OTA/";
        String client_id(_username);
        String full_topic = topic + client_id;

        char buffer_percent[50];
        sprintf(buffer_percent, "%d", percent);
        if (_mqtt_client.connected()) {
                _mqtt_client.publish(full_topic.c_str(), buffer_percent);
                _mqtt_client.loop();
        }
        Serial.printf("state = %d - percent = %d\n", state, percent);
}
void IfraESP32SDK::otaErrorCallback(char * message) {
        Serial.println(message);
}
void IfraESP32SDK::startDownloadCallback(void) {
        Serial.println("startDownloadCallback");
}

void IfraESP32SDK::endDownloadCallback(void) {
        Serial.println("Update Device Status");
        if ((WiFi.status() == WL_CONNECTED)) {
                HTTPClient http;
                http.begin(_callback_success);
                int httpCode = http.GET();
                if (httpCode > 0) { //Check for the returning code
                        String payload = http.getString();
                        Serial.println(httpCode);
                        Serial.println(payload);
                } else {
                        Serial.println("Error on HTTP request");
                }

                http.end(); //Free the resources
        }

        delay(1000);
}
void IfraESP32SDK::callback(char * topic, byte * payload, unsigned int length) {
        
        if (strncmp("OTA", topic, 3) == 0) {
                deserializeJson(_docMQTT, (char * ) payload);
                _ota_device_id = _docMQTT["device_id"];
                _ota_url = _docMQTT["url"];
                _ota_token = _docMQTT["token"];
                _callback_success = _docMQTT["callback_success"];
                _is_wifi_reset = _docMQTT["is_wifi_reset"];

                if((String) _is_wifi_reset!= ""){
                           if ((String) _ota_device_id == (String) _username) {
                                Serial.println("Reset WiFi");
                                WiFi.disconnect(false,true);
                                delay(2000);
                                ESP.restart();
                           }
                }else{
                        _state = Fota_e;
                        Serial.println("Fota_e");

                if ((String) _ota_device_id == (String) _username) {
                        Serial.println("Start Download Firmware!!");
                        DlInfo info;
                        info.url = (char * ) _ota_url;
                        info.caCert = NULL;
                        info.token = (char * ) _ota_token;
                        info.startDownloadCallback = [this](void) {
                                startDownloadCallback();
                        };
                        info.endDownloadCallback = [this](void) {
                                endDownloadCallback();
                        };
                        info.progressCallback = [this](DlState state, int percent) {
                                otaProgressCallback(state, percent);
                        };
                        info.errorCallback = [this](char * message) {
                                otaErrorCallback(message);
                        };
                        Serial.print("Download from:");
                        Serial.println(_ota_url);
                        esp32OTA.start(info);

                }

                }




        }

}
bool IfraESP32SDK::wifiConnection() {
        String ssid = "VIBRO-" + String(_username);
        if(!wifiManager.autoConnect(ssid.c_str())){
          Serial.println("failed to connect, we should reset as see if it connects");
        delay(3000);
        ESP.restart();
        delay(5000);
        }
          //init and get the time
        configTime(gmtOffset_sec, daylightOffset_sec, NTP_SERVER);

        //MQTT callback
        _mqtt_client.setCallback([this](char * topic, byte * payload, unsigned int length) {
                callback(topic, payload, length);
        });
}
bool IfraESP32SDK::wifiConnection(char * ssid, char * pass) {
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
        _mqtt_client.setCallback([this](char * topic, byte * payload, unsigned int length) {
                callback(topic, payload, length);
        });

}

bool IfraESP32SDK::mqttConnection(char * mqtt_topic) {}

void IfraESP32SDK::setBaseUnit(char * unit) {
        _base_unit = unit;
}

void IfraESP32SDK::setBaseTime(unsigned long int time) {
        _base_time = time;
}
void IfraESP32SDK::setBaseName(char * name) {
        _base_name = name;
}

void IfraESP32SDK::addMeasurement(char * var_id, char * unit, float value) {

        JsonObject doc = _doc.createNestedObject();
        if (_base_time != 0 && _recordCount == 0) {
                doc["bt"] = _base_time;
        }

        if (_base_name == "") {
                doc["n"] = var_id;
        } else if (_recordCount == 0) {
                doc["bn"] = var_id;
        }

        if (_base_unit == "") {
                doc["u"] = unit;
        } else if (_recordCount == 0) {
                doc["bu"] = unit;
        }

        doc["v"] = value;
        _recordCount++;
}

void IfraESP32SDK::addMeasurement(char * var_id, char * unit, float value, double time) {

        JsonObject doc = _doc.createNestedObject();

        if (_base_time != 0 && _recordCount == 0) {
                doc["bt"] = _base_time;
        } else if (_recordCount != 0) {
                doc["t"] = time;
        }

        if (_base_name == "") {
                doc["n"] = var_id;
        } else if (_recordCount == 0) {
                doc["bn"] = var_id;
        }

        if (_base_unit == "") {
                doc["u"] = unit;
        } else if (_recordCount == 0) {
                doc["bu"] = unit;
        }

        doc["v"] = value;

        _recordCount++;
}

void IfraESP32SDK::send(char * toptic) {
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

unsigned long int IfraESP32SDK::getTimestamp(void) {

        struct tm timeinfo;
        if (!getLocalTime( & timeinfo)) {
                Serial.println("Failed to obtain time");
                return 0;
        }
        //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
        time_t epochUnix = mktime( & timeinfo);
        unsigned long int b = static_cast < time_t > (epochUnix);
        return b;
}

bool IfraESP32SDK::connected(void) {
        return _mqtt_client.connected();
}
void IfraESP32SDK::loop(void) {
        _mqtt_client.loop();
}

void IfraESP32SDK::reconnect(void) {

        while (!_mqtt_client.connected()) {
                Serial.print("Connecting IFRA MQTT...");

                if (_mqtt_client.connect(_username, _username, _password)) {
                        Serial.println("connected ^_^");
                        String topic = "OTA/";
                        String client_id(_username);
                        String full_topic = topic + client_id;
                        _mqtt_client.subscribe(full_topic.c_str());

                } else {
                        Serial.print("failed, rc=");
                        Serial.print(_mqtt_client.state());
                        Serial.println(" try again in 5 seconds");
                        // Wait 5 seconds before retrying
                        delay(5000);

                }
        }

}
