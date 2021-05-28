#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <Wire.h>

#define DELAY_MS  19000

WiFiClient PJWC;
HTTPClient PJHC;
PubSubClient PJMC;

int MPU_Address = 0x68; //mpu6050 칩의 I2C 주소
int i = 1;
int16_t Tmp;
float tmp;

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
  
    Wire.requestFrom(MPU_Address, 14 ,true);
    Tmp = Wire.read() << 8 | Wire.read();
    tmp = Tmp / 340.000 + 36.53
    Serial.print(", Tmp = "); Serial.print(tmp);

    if(i == 0)
    {
      Serial.print("Tmp = "); Serial.println(Tmp / 340.000 + 36.53);
      char Tmpbuffer[200];
      snprintf(Tmpbuffer, sizeof(Tmpbuffer), "http://api.thingspeak.com/update?api_key=6S31S3WI6UO1EZE6&field1=%lf", Tmp / 340.000 + 36.53);
      myClient.begin(Tmpbuffer);
      myClient.GET();
      myClient.getString();
      myClient.end();
      i = 1;
    }
    else
    {
      Serial.print("AZ = "); Serial.println(z);
      char AcZbuffer[100];
      snprintf(AcZbuffer, sizeof(AcZbuffer), "%lf", z);
      myMQTTClient.publish("channels/1397244/publish/fields/field2/6S31S3WI6UO1EZE6", AcZbuffer);
      i = 0;
    }
  }
}
