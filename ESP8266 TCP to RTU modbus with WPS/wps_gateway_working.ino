#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#define rs485 5
#define setup 14
#define wifi  12
#define mdns  13

int i,cnt;
boolean newData;
WiFiServer server(502);
WiFiClient client ;

uint8_t sbuf[255];
uint8_t rtu_buf[255];

bool startWPSPBC() 
{
  Serial.println("WPS config start");
  bool wpsSuccess = WiFi.beginWPSConfig();
  if(wpsSuccess) 
  {
      String newSSID = WiFi.SSID();
      if(newSSID.length() > 0) 
      {
         Serial.printf("WPS finished. Connected successfull to SSID '%s'\n", newSSID.c_str());
      } else 
      {
        wpsSuccess = false;
      }
  }
  return wpsSuccess; 
}

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
  pinMode(rs485,OUTPUT); //re rs485
  pinMode(14,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);
  
  digitalWrite(rs485,LOW);
  digitalWrite(14,LOW);
  digitalWrite(12,LOW);
  digitalWrite(13,LOW);
  
  Serial.begin(19200);
  Serial.printf("\nTry connecting to WiFi with SSID '%s'\n", WiFi.SSID().c_str());
  WiFi.mode(WIFI_STA);

  WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str()); 
  while (WiFi.status() == WL_DISCONNECTED) 
  {          
    delay(500);
    Serial.print(".");
  }

  wl_status_t status = WiFi.status();
  if(status == WL_CONNECTED) 
  {
    Serial.printf("\nConnected successful to SSID '%s'\n", WiFi.SSID().c_str());
  } else 
  {
    
    Serial.printf("\nCould not connect to WiFi. state='%d'\n", status);
    Serial.println("Please press WPS button on your router, until mode is indicated.");
    Serial.println("next press the ESP module WPS button, router WPS timeout = 2 minutes");
    
    while(digitalRead(4) == HIGH)  yield(); // do nothing, allow background work (WiFi) in while loops
    Serial.println("WPS button pressed");
    
    if(!startWPSPBC()) {
       Serial.println("Failed to connect with WPS :-(");  
    } else {
      WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str()); // reading data from EPROM, 
      while (WiFi.status() == WL_DISCONNECTED) {          // last saved credentials
        delay(500);
        Serial.print("."); // show wait for connect to AP
      }
      
    }
  }
  
  server.begin();
  Serial.print("\nReady! Use 'telnet ");
  Serial.print(WiFi.localIP());
  MDNS.begin("esp8266");
  MDNS.addService("http", "tcp", 80);
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
            digitalWrite(rs485,HIGH);      
            Serial.write(rtu_buf,sbuf[5]+2);
            digitalWrite(rs485,LOW);
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
