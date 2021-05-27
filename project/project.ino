#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <Wire.h>

WiFiClient PJWC;
HTTPClient PJHC;
PubSubClient PJMC;

int MPU_Address = 0x68; //mpu6050 칩의 I2C 주소
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
  // put your main code here, to run repeatedly:

}
