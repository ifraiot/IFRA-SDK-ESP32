# IFRA-SDK-ESP32

## Note
- หากต้องการส่งข้อมูลขนาดมากว่า  128 bytes. ให้แก้ไข MQTT_MAX_PACKET_SIZE ในไฟล์ Arduino/libraries/PubSubClient/src/PubSubClient.h

## ตัวอย่างโค้ด
```cpp

//นำเข้า IfraESP32SDK.h สำหรับการเรียกใช้งาน IFRA SDK
#include <IfraESP32SDK.h>

//Username Password ที่ได้จากการลงทะเบียนในแพลตฟอร์ม https://ws.ifra.io/things
#define USERNAME "ac361b57-3152-45fd-8bca-7599e2e52d30"
#define PASSWORD "154d7ede-f938-4369-a45f-c0efa1ef6a90"


//ชื่อ SSID และ PASSWORD ของ Access Point
#define WIFI_SSID "xxxxx"
#define WIFI_PASSWORD "xxxxx"



unsigned int x_offset;
unsigned int y_offset;
unsigned int z_offset;
unsigned int sampling_period_us;

const uint16_t samples = 2048;          //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 6000;  //Hz, must be less than 10000 due to ADC

unsigned long microseconds;
unsigned long t[samples];
double vReal_1[samples];
double vReal_2[samples];
double vReal_3[samples];
double vImag[samples];

//สร้าง Object "device" จาก Class IfraESP32SDK โดยส่ง ค่า USERNAME, PASSWORD เข้า constructor
IfraESP32SDK device(USERNAME, PASSWORD);

void setup() {

  Serial.begin(115200);

  sampling_period_us = round(1000000 * (1.0 / samplingFrequency));

  pinMode(39, INPUT); // ADC Axis_X
  pinMode(32, INPUT); // ADC Axis_Y
  pinMode(25, INPUT); // ADC Axis_Z
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  int i = 0;
  while (i < 10)
  {
    digitalWrite(13, LOW);
    delay(500);
    x_offset = x_offset + analogRead(39);
    y_offset = y_offset + analogRead(32);
    z_offset = z_offset + analogRead(25);
    digitalWrite(13, HIGH);
    delay(500);
    i++;
  }
  x_offset = x_offset / 10;
  y_offset = y_offset / 10;
  z_offset = z_offset / 10;


  //ทำการเชื่อมต่อ Access Point โดยการเรียก Method "wifiConnection" จาก Object "device"
  device.wifiConnection(WIFI_SSID, WIFI_PASSWORD);


}

void loop() {

  unsigned long int timestamp = device.getTimestamp(); //ดึงเวลาปัจจุปันจาก Server
  Serial.println(timestamp);
  microseconds = micros();
  unsigned int i = 0;
  unsigned int n = 0;
  while (i < samples)
  {
    digitalWrite(13, LOW);
    int x = analogRead(39) - x_offset;
    int y = analogRead(32) - y_offset;
    int z = analogRead(25) - z_offset;

    // 10g Factor 80mV/g
    /*
      vReal_1[i] = (x*(3.3/4095))/0.08;   //Axis_X Signal
      vReal_2[i] = (y*(3.3/4095))/0.08;   //Axis_Y Signal
      vReal_3[i] = (z*(3.3/4095))/0.08;   //Axis_Z Signal
    */
    // 40g Factor 20mV/g
    vReal_1[i] = (x * (3.3 / 4095)) / 0.02; //Axis_X Signal
    vReal_2[i] = (y * (3.3 / 4095)) / 0.02; //Axis_Y Signal
    vReal_3[i] = (z * (3.3 / 4095)) / 0.02; //Axis_Z Signal
    i++;
    while (micros() - microseconds < sampling_period_us)
    {
      //empty loop
    }
    microseconds += sampling_period_us;
    t[i] = microseconds;
    digitalWrite(13, HIGH);


  }


  //Set BaseName setBaseUnit setBaseTime  ให้กับ Record แรก เพื่อลดขนาด message Refer:https://tools.ietf.org/html/rfc8428
  device.setBaseName("Axis_X_Amplitude");
  device.setBaseUnit("A");
  device.setBaseTime(timestamp);

  n = 0;
  i = 0;
  double ts = 0;
  int parts = 16;
  int sub_part  = samples / parts;
  while (n < parts)
  {

    Serial.println("Part:");
    Serial.println(n);

    i = n * sub_part;

    while (i < sub_part + n * sub_part)
    {
    
      //Add ค่าการวัดประกอบด้วย Measurement Name , Unit ,Value ,Time(sec)
      device.addMeasurement("Axis_X_Amplitude", "A", vReal_1[i], ts * 167e-6);
      i++;
      ts++;
    }


    //เรียก Method send เพื่อทำการส่งข้อมูลที่ได้เพิ่มเข้าไปก่อนหน้า
    //โดยจะส่งเข้าไปใน Topic ที่แพลตฟอร์มกำหนดให้
    //หลังจากส่งสำเร็จ SDK จะล้างข้อมูล Measurementทั้งหมดและพร้อมสำหรับข้อมูลชุดใหม่
    device.send("organization/2/messages");

    n++;
  }


  delay(5000);

}
```
