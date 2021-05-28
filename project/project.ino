#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include<ArduinoJson.h>
#include <Wire.h>

#define DELAY_MS  19000

WiFiClient PJWC;
HTTPClient PJHC;
PubSubClient PJMC;

int MPU_Address = 0x68; //mpu6050 칩의 I2C 주소
int16_t Tmp;
float tmp;
int i = 1;

void setup()
{ 
  Serial.begin(115200);
  delay(100);
  
  Wire.begin(4,5);
  Wire.beginTransmission(MPU_Address);
  Wire.write(0x6B);
  Wire.write(1);
  Wire.endTransmission(true);
  i = Wire.read();
  Serial.print("0x");
  Serial.println(i,HEX);

  WiFi.begin(" "," ");
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.print("\n");
  PJMC.setClient(PJWC);
  PJMC.setServer("mqtt.thingspeak.com",1883);
  int PC = PJMC.connect("mqtt.thingspeak.com");
  PJMC.subscribe("channels/1401138/publish/fields/field2/6S31S3WI6UO1EZE6/#");
  Serial.println(" MQTT Connect: %d", PC);
}

void loop()
{
  if(millis() - lastMs >= DELAY_MS)
  {
    lastMs = millis();
    Wire.beginTransmission(MPU_Address);
    Wire.write(0x3B);
    Wire.endTransmission();
    delay(1000);
    
    Wire.requestFrom(MPU_Address, 2 ,true);
    Tmp = Wire.read() << 8 | Wire.read();
    tmp = Tmp / 340.000 + 36.53
    Serial.print(", Tmp = "); Serial.print(tmp);

    if(i == 0)
    {
      PJHC.begin("http://api.openweathermap.org/data/2.5/weather?q=yongin&appid=e749f72f517350b356969095ff56fd24");
      int getResult = PJHC.GET();
      if(getResult == HTTP_CODE_OK)
      {
        Serial.printf("site OK\n");
        String receivedData = PJHC.getString();
        //Serial.printf("receivedData : %s\r\n",receivedData.c_str());
        deserializeJson(doc,receivedData);  // 해석 완료
    
        const char* city = doc["name"];
        float temp = (float)doc["main"]["temp"]-273.0;
        Serial.printf("도시 : %s\r\n",city);
        Serial.printf("현재온도 : %.2f\r\n",temp);
        
        char Tmpbuffer[200];
        snprintf(Tmpbuffer, sizeof(Tmpbuffer), "http://api.thingspeak.com/update?api_key=A5YIWO4Z2OZLAIGK&field2=%lf", temp);
        PJHC.begin(Tmpbuffer);
        PJHC.GET();
        PJHC.getString();
        PJHC.end();
      }
      else
      {
        Serial.printf("site ERR, code : %d\r\n",getResult);
        return;
      }
      i = 1;
    }
    else
    {
      Serial.print("AZ = "); Serial.println(z);
      char AcZbuffer[100];
      snprintf(AcZbuffer, sizeof(AcZbuffer), "%lf", z);
      myMQTTClient.publish("channels/1397244/publish/fields/field2/6S31S3WI6UO1EZE6", tmp);
      i = 0;
    }
  }
}
