#include <ESP8266WiFi.h>
#include <EspMQTTClient.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

#define WIFI_ID       "Domotique"
#define WIFI_PWD      "94582604"

#define MQTT_IP       "192.168.3.200"
#define MQTT_LOGIN    "toto"
#define MQTT_PWD      "toto"
#define MQTT_NAME     "screen1"

#define MQTT_TRACE_ON

int g_lvlMain=-1;
int g_lvlReduit=-1;
int g_lvlPaul=-1;
int g_lvlBarbec=-1;

bool g_pmpMain=false;
bool g_pmpPaul=false;
bool g_pmpReduit=false;

bool g_refresh=false;

Adafruit_SSD1306 scr(128,64,&Wire,-1);

StaticJsonDocument<1024> doc;

EspMQTTClient mqttClient(
  WIFI_ID,
  WIFI_PWD,
  MQTT_IP,
  MQTT_LOGIN,
  MQTT_PWD,
  MQTT_NAME
);

void sendLog(const char *strMsg)
{
  mqttClient.publish("/wifiio/screen/log", String(strMsg));
}

void onReceiveEvent(const String &payload)
{
  Serial.println("Recu event");
  
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  
  DeserializationError error = deserializeJson(doc, payload.c_str());

  if (error) 
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  
  //sendLog(payload);

  bool n1=doc["n1"];
  bool n2=doc["n2"];
  bool n3=doc["n3"];
  bool cmd=doc["cmd"];

  int lvl=0;
  if (n3==true)
    lvl=3;
  else if (n2==true)
    lvl=2;
  else if (n1==true)
    lvl=1;

  if (doc["name"]=="main")
  {
    g_lvlMain=lvl;
    g_pmpMain=cmd;
    Serial.println("Maj main");
  }
  else if (doc["name"]=="paul")
  {
    g_lvlPaul=lvl;
    g_pmpPaul=cmd;
    Serial.println("Maj paul");
  }
  else if (doc["name"]=="reduit")
  {
    g_lvlReduit=lvl;
    g_pmpReduit=cmd;
    Serial.println("Maj reduit");
  }
  else if (doc["name"]=="barbec")
  {
    g_lvlBarbec=lvl;
    Serial.println("Maj barbec");
  }
  else
  {
    Serial.println("Name incorrect!");
  }    

  g_refresh=true;
}

void onConnectionEstablished()
{
  Serial.println("Connection est");
  mqttClient.subscribe("/wifiio/screen/data",onReceiveEvent);   
  sendLog("MQTT Connected");
} 

void drawJauge(int x,int y,int lvl,char c)
{
  int h=46;
  int w=15;

  if (lvl!=-1)
    scr.drawRoundRect(x,y,w,h,8,WHITE);
    
  if (lvl>0)
    scr.fillRect(x+2,y+h-h/3,w-4,h/3-4,WHITE);
  if (lvl>1)
    scr.fillRect(x+2,y+h-2*h/3+2,w-4,h/3-4,WHITE);
  if (lvl>2)
    scr.fillRect(x+2,y+5,w-4,h/3-4,WHITE);

  scr.setTextSize(2);
  scr.setTextColor(WHITE);
  scr.setCursor(x+1,y+5+h);
  scr.write(c);    
}

void drawScreen(void)
{
  scr.clearDisplay();

  scr.setTextSize(1);
  scr.setCursor(4,15);
  scr.setTextColor(WHITE);
  scr.write("Wifi");
  scr.setCursor(8,15+10);
  scr.write("IO");

  scr.setCursor(0,40);
  scr.write("Cuves");

  drawJauge(40,0,g_lvlMain,'M');  
  drawJauge(60,0,g_lvlPaul,'P');
  drawJauge(80,0,g_lvlReduit,'R');
  drawJauge(100,0,g_lvlBarbec,'B');

  scr.display();
}

void dispWaiting(char *strMsg)
{
  scr.clearDisplay();
  scr.setTextSize(2);
  scr.setTextColor(WHITE);
  scr.setCursor(20,20);
  scr.write("Wifi IO");  
  scr.setTextSize(1);
  scr.setCursor(18,50);
  scr.write(strMsg);
  scr.display();  
}

void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);

  Wire.begin(2,0);
  Serial.begin(115200);
  Serial.println("Boot...");  
  //Serial.setDebugOutput(true);

  scr.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  scr.clearDisplay();
  scr.display();

  scr.clearDisplay();
  scr.setTextSize(2);
  scr.setTextColor(WHITE);
  scr.setCursor(20,20);
  scr.write("Wifi IO");  
  scr.setTextSize(1);
  scr.setCursor(40,50);
  scr.write("Booting...");
  scr.display();
  delay(1000);

  mqttClient.enableMQTTPersistence();

  #ifdef MQTT_TRACE_ON
    mqttClient.enableDebuggingMessages();
  #endif

  //Serial.println(WiFi.macAddress());
  //Serial.println(WiFi.localIP());
}

bool waitWifi=false;
bool waitMqtt=false;
bool boot=true;

void loop() 
{
  mqttClient.loop(); 

  if ( !mqttClient.isWifiConnected() )
  {
    if (waitWifi==false)
    {
      dispWaiting("Waiting Wifi...");
      waitWifi=true;
    }
  }  
  else if ( !mqttClient.isMqttConnected() )
  {
    if (waitMqtt==false)
    {
      dispWaiting("Waiting MQTT...");
      waitMqtt=true;
    }
  }
  else
  {
    if (boot==true)
    {
      mqttClient.publish("/wifiio/screen/boot", MQTT_NAME);
      boot=false;      
    }
    
    if ( (waitWifi==false) || (waitMqtt==false) || (g_refresh==true) )
      drawScreen();
    
    waitWifi=false;
    waitMqtt=false;
  }

  // Serial.println(client.connected());
}
