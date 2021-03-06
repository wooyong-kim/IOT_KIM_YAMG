#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <ESP8266WebServer.h>
#include <AnotherIFTTTWebhook.h>

#define DELAY_MS  20000
#define MOTOR     D5

WiFiClient PJWC;
HTTPClient PJHWC;
HTTPClient PJHC;
PubSubClient PJMC;
ESP8266WebServer PJWS(80);

int MPU_Address = 0x68; //mpu6050 칩의 I2C 주소
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;
int16_t Tmp, a ;
float tmp, temp, subtmp;
float Tmpd = 5;
int i = 1;
char tmpd [200];
int motorP = 0;


void fnroot(void)
{
  char tmpb[2000];
  char IP[200];
  char Tmp1[200];
  char Tmp2[200];
  strcpy (tmpb, "<html>\r\n");
  strcpy(tmpb, "<meta charset=utf-8>");
  strcat (tmpb, "IOT Project <br>\r\n");
  snprintf(Tmp1, sizeof(Tmp1),"현재 외부 온도 : %.1f <br>\r\n",temp);
  strcat(tmpb, Tmp1);
  snprintf(Tmp2, sizeof(Tmp2),"현재 내부 온도 : %.1f <br>\r\n",tmp);
  strcat(tmpb, Tmp2);
  strcat (tmpb, "<a href=/Temperature>현재 기준값</a><br>\r\n");
  strcat (tmpb, "<form method=\"get\" action=\"input\">");
  strcat (tmpb, "기준값 설정 <input type=\"text\" name=\"tmp\">");
  strcat (tmpb, "<input type=\"submit\"></form>\r\n");
  snprintf (tmpb, sizeof(tmpb), "%s%s", tmpb, "</html>");
  PJWS.send(200,"text/html", tmpb);
}

void fnNotFound(void)
{
  PJWS.send(404, "text/html", "WRONG!!");
}

void fnOn(void)
{
    char tmpb[200];
    strcpy(tmpb, "<meta charset=utf-8>");
    snprintf(tmpb, sizeof(tmpb), "%1.f", Tmpd);
    PJWS.send(200, "text/html", tmpb);
 }

 void fnInput(void)
 {
  if(PJWS.hasArg("tmp"))
  {
    strcpy(tmpd, "<meta charset=utf-8>");
    strcat(tmpd, PJWS.arg("tmp").c_str());
    Tmpd = atoi(PJWS.arg("tmp").c_str());

    PJWS.send(200, "text/html", tmpd);
  }
  else
    PJWS.send(200, "text/html", "Something Wrong!");
}


void setup()
{ 
  Serial.begin(115200);
  delay(100);
  pinMode(MOTOR,OUTPUT);
  Wire.begin(4,5);
  Wire.beginTransmission(MPU_Address);
  Wire.write(0x6B);
  Wire.write(1);
  Wire.endTransmission(true);
  delay(1000);
  Wire.requestFrom(MPU_Address, 14, true);
  a = Wire.read();
  Serial.print("0x");
  Serial.println(a,HEX);
  
  WiFi.begin("olleh_WiFi_1192","0000008053");
  //WiFi.begin("hana202_2_4GHz","0000202ho0000");
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
  Serial.printf(" MQTT Connect: %d\r\n", PC);

  PJWS.on("/",fnroot);
  PJWS.on("/Temperature",fnOn);
  PJWS.on("/input",fnInput);
  PJWS.onNotFound(fnNotFound);
  PJWS.begin();
  char ip[20];
  snprintf(ip,sizeof(ip),"%s",WiFi.localIP().toString().c_str());
  send_webhook("IOT_KIM_YAMG","dZ-52Zgk8CnPCeC_15A2T8",ip,"IPAdress","");
}

unsigned long long lastMs = 0;
DynamicJsonDocument doc(2048);

void loop()
{
  if(millis() - lastMs >= DELAY_MS)
  {
    lastMs = millis();
    Wire.beginTransmission(MPU_Address);
    Wire.write(0x3B);
    Wire.endTransmission();
    
    Wire.requestFrom(MPU_Address, 14 ,true);
    AcX = Wire.read() << 8|Wire.read();
    AcY = Wire.read() << 8|Wire.read();
    AcZ = Wire.read() << 8|Wire.read();
    Tmp = Wire.read() << 8|Wire.read();
    GyX = Wire.read() << 8|Wire.read();
    GyY = Wire.read() << 8|Wire.read();
    GyZ = Wire.read() << 8|Wire.read();
    tmp = Tmp / 340.000 + 36.53;
    //Serial.print(" Tmp = "); Serial.println(tmp);

    PJHWC.begin("http://api.openweathermap.org/data/2.5/weather?q=yongin&appid=e749f72f517350b356969095ff56fd24");
    int getResult = PJHWC.GET();
    if(getResult == HTTP_CODE_OK)
    {
      Serial.printf("site OK\n");
      String receivedData = PJHWC.getString();
      deserializeJson(doc,receivedData);  // 해석 완료
  
      const char* city = doc["name"];
      temp = (float)doc["main"]["temp"]-273.0;
      //Serial.printf("도시 : %s\r\n",city);
      //Serial.printf("현재온도 : %1f\r\n",temp);
    }
    else
    {
      Serial.printf("site ERR, code : %d\r\n",getResult);
      return;
    }
    PJHWC.end();
    
    subtmp = tmp - temp;  //내부온도 - 현재온도
    if (abs(subtmp) >= Tmpd ) 
    {
      if(abs(abs(subtmp) - Tmpd) >= 10 && abs(abs(subtmp) - Tmpd) < 15) 
        motorP = 950;
      else if(abs(abs(subtmp) - Tmpd) >= 15)
        motorP = 1023;
      else 
        motorP = 800;
    }
    else motorP = 0;
    //Serial.printf(" 차이 %1f\r\n",subtmp);
    
    if(i == 0)
    {     
      char Tmpbuffer[200];
      snprintf(Tmpbuffer, sizeof(Tmpbuffer), "http://api.thingspeak.com/update?api_key=6S31S3WI6UO1EZE6&field1=%lf", temp);
      PJHC.begin(Tmpbuffer);
      PJHC.GET();
      PJHC.getString();
      PJHC.end();
      
      i = 1;
    }
    else
    {
      int PC = PJMC.connect("mqtt.thingspeak.com");
      char tempb[100];
      snprintf(tempb, sizeof(tempb), "%lf", tmp);
      PJMC.publish("channels/1401138/publish/fields/field2/6S31S3WI6UO1EZE6", tempb);
      i = 0;
      Serial.printf("connected : %d\r\n", PJMC.connected());
    }
    PJMC.loop();
  }
  
  analogWrite(MOTOR,motorP);
  
  PJWS.handleClient();
}
