#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

bool wifiLed = true;
bool configLed = true;

int i;
int retry = 0;
boolean newData;
WiFiServer server(502);
WiFiClient client ;

uint8_t sbuf[255];
uint8_t rtu_buf[255];

void calcCRC(uint8_t u8length)
{
    unsigned int temp, temp2, flag;
    temp = 0xFFFF;
    for (unsigned char i = 0; i < u8length; i++)
    {
        temp = temp ^ rtu_buf[i];
        for (unsigned char j = 1; j <= 8; j++)
        {
            flag = temp & 0x0001;
            temp >>=1;
            if (flag)
                temp ^= 0xA001;
        }
    }

    rtu_buf[sbuf[5]] = temp;  
    rtu_buf[sbuf[5]+1] = temp >> 8;  
   
}

void setup() 
{
  WiFi.mode(WIFI_STA);
  
  Serial.begin(19200);
  for (int i=12;i<15;i++) 
  {
    pinMode(i, OUTPUT);
    digitalWrite(i,LOW);
    pinMode(4,INPUT_PULLUP);
    
  }
  WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str());
    
  while(WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(12,wifiLed);
    wifiLed = !wifiLed;
    delay(500);
    if (!digitalRead(4))  // smart config initiated by switch only 
    {
      digitalWrite(12,LOW);
      WiFi.beginSmartConfig();
      while (!WiFi.smartConfigDone())
      {
        digitalWrite(14,configLed);
        configLed = !configLed;
        delay(500);
      }
      break;
    }
  }
  digitalWrite(14,LOW);
  digitalWrite(12,HIGH);
  if (MDNS.begin("esp8266")) digitalWrite(13,HIGH);
  MDNS.addService("http", "tcp", 80);
  server.begin();
}

void loop() 
{
  if (!client.connected()) client = server.available();
  while (client.connected())
    {
     if (client.available())
        {
          newData = true;
          int i = 0;
          Serial.flush();
          while ((client.available()) && (i<6))
          {
            sbuf[i] = client.read();
            i ++;
          }
          for (i=0; i < sbuf[5]; i++) 
          {
            rtu_buf[i] = client.read();
          }
              
          calcCRC(sbuf[5]);
          if (newData)
          {
            newData = false;
            Serial.write(rtu_buf,sbuf[5]+2);
            Serial.flush();
          }  
        }

        if (Serial.available())
        {
          i = 0;
          delay(50);
          while (Serial.available())
          {
            sbuf[6+i] = Serial.read();
            i++;
          }
          sbuf[5] = i;
          client.write(sbuf, i+4);
        }
      }
  MDNS.update();    
}
