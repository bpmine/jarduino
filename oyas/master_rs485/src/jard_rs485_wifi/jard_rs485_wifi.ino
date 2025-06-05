#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <string.h>
#include <EspMQTTClient.h>

#include <uri/UriBraces.h>

#include <ArduinoJson.h>

#include "wifi_code.h"
#include "wificomm.h"
#include "timer.h"

#define VERSION "V0.0"
//#define MODE_SIMU
//#define MODE_AP
//#define MQTT_TRACE_ON

//#define NODE_MASTER_BARBEC
//#define NODE_MASTER_REDUIT
#define NODE_MASTER_PAUL
//#define NODE_TEST

#if defined(NODE_TEST) && !defined(NODE_MASTER_BARBEC) && !defined(NODE_MASTER_REDUIT) && !defined(NODE_MASTER_PAUL)
  char NAME[]="test";
#elif !defined(NODE_TEST) && defined(NODE_MASTER_BARBEC) && !defined(NODE_MASTER_REDUIT) && !defined(NODE_MASTER_PAUL)
  char NAME[]="barbec";
#elif !defined(NODE_TEST) && !defined(NODE_MASTER_BARBEC) && defined(NODE_MASTER_REDUIT) && !defined(NODE_MASTER_PAUL)
  char NAME[]="reduit";  
#elif !defined(NODE_TEST) && !defined(NODE_MASTER_BARBEC) && !defined(NODE_MASTER_REDUIT) && defined(NODE_MASTER_PAUL)
  char NAME[]="paul";  
#else
  #error Noeud non defini
#endif

#define TOPIC_PREFIX  "/oyas"
#define TOPIC_CMD     "cmd"
#define TOPIC_DATA    "data"
#define TOPIC_LOG     "log"

#ifdef MODE_AP
  const char ssid[] = "Master";
  const char password[] = "Master38";  ///< Minimum 8 chars
  
  IPAddress local_IP(192,168,0,1);
  IPAddress gateway(192,168,0,1);
  IPAddress subnet(255,255,255,0);  
#else
  const char* ssid = WIFI_ID;      ///< Voir le fichier wifi_code.h
  const char* password = WIFI_PWD; ///< Voir le fichier wifi_code.h
#endif

Timer tmrBlink=Timer(1000,false);
bool flgBlink=false;

EspMQTTClient mqttClient(
  WIFI_ID,
  WIFI_PWD,
  MQTT_IP,
  MQTT_LOGIN,
  MQTT_PWD,
  NAME
);

void setup_mqtt(void)
{
  static char strName[30];
  sprintf(strName,"oyas_%s",NAME);
  mqttClient.setMqttClientName(strName);
  
  mqttClient.enableMQTTPersistence();

  #ifdef MQTT_TRACE_ON
    mqttClient.enableDebuggingMessages();
  #endif
}

void sendLog(char *strMsg)
{
  char strTopicLog[50];
  sprintf(strTopicLog,"%s/%s/%s",TOPIC_PREFIX,TOPIC_LOG,NAME);
  
  mqttClient.publish(strTopicLog, strMsg);
}

void sendData(char *strMsg)
{
  char strTopicLog[50];
  sprintf(strTopicLog,"%s/%s/%s",TOPIC_PREFIX,TOPIC_DATA,NAME);
  
  mqttClient.publish(strTopicLog, strMsg);
}

void onReceiveCmd(const String &payload)
{
  Comm.sendMsgToMaster(payload.c_str());
}

void onConnectionEstablished()
{
  char strTopicCmd[50];
  sprintf(strTopicCmd,"%s/%s/%s",TOPIC_PREFIX,TOPIC_CMD,NAME);
  mqttClient.subscribe(strTopicCmd,onReceiveCmd);  
  
  sendLog("MQTT Connected");
  //sendData();
}

void setup(void)
{
  Serial.begin(9600);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  

  #ifdef MODE_AP
    if (WiFi.softAP(ssid,password,2,false,8)==false)
      Serial.println("Impossible to config AP!");
      
    if (WiFi.softAPConfig(local_IP,gateway,subnet)==false)
      Serial.println("Impossible to config IPs!");
    

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);    
  #else
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");
  
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  #endif

  /*if (MDNS.begin("esp8266")) 
  {
    //Serial.println("MDNS responder started");
  }*/

  for (int i=0;i<10;i++)
  {
    delay(100);
    yield();
  }

  setup_mqtt();

  Comm.begin(&Serial);
  tmrBlink.start();
  flgBlink=false;
}

/**
 * @brief Boucle cyclique de l'arduino
 * */
void loop()
{
  //MDNS.update();

  Comm.loop();
  mqttClient.loop();

  if (tmrBlink.tick()==true)
    flgBlink=!flgBlink;


  if (Comm.isMqttOk()==false)
  {
    digitalWrite(BUILTIN_LED,HIGH);
  }
  else
  {
    if (Comm.isMasterOk()==false)
    {
      digitalWrite(BUILTIN_LED,LOW);
    }
    else
    {
      digitalWrite(BUILTIN_LED,flgBlink?LOW:HIGH);
    }
  }  
}
