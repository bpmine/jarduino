#include <ModbusRTU.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <uri/UriBraces.h>
#include <WiFiUdp.h>

#include "jardinfo.hpp"
#include "credentials.h"

#define ENABLE_MQTT
//#define BYPASS_MODBUS
#define MODBUS_SLAVE_ADDRESS  4

ModbusRTU mb;

bool g_flgHttpMode=true;

JardInfo jarduino;
JardInfo jarduino_latched;

bool tmpBool[10];
unsigned short tmpReg[10];

ESP8266WebServer server(80);
const char* ssid = STASSID;
const char* password = STAPSK;

#define DEEPSLEEP_US  (30 * 1000000UL)
#define NBRE_MB_NOK_MAX   (4)
bool g_flgMbOK=false;
int g_nbreMbNok=0;

#define DELAY_BEFORE_SLEEP_MS (4*60*1000UL)
unsigned long g_t0Start_ms;

#ifdef ENABLE_MQTT
  #include <EspMQTTClient.h>

  //#define MQTT_TRACE_ON
  
  #define TOPIC_PREFIX  "/jarduino"
  #define TOPIC_CMD     "cmd"
  #define TOPIC_DATA    "data"
  #define TOPIC_LOG     "log"
  
  #define DELAY_TASK_REPORT_COMM  (3600*1000UL)
  #define DELAY_TASK_REPORT_DATA  (5*60*1000UL)
  #define DELAY_TASK_MQTT_OK      (800UL)
  
  EspMQTTClient mqttClient(
    STASSID,
    STAPSK,
    MQTT_IP,
    MQTT_LOGIN,
    MQTT_PASS,
    "jarduino_xxx"
  );
  
  int g_id=0;
  bool g_flgProduce=true;
  
  void sendLog(char *strMsg)
  {
    char strTopicLog[50];
    sprintf(strTopicLog,"%s/%s/%03d",TOPIC_PREFIX,TOPIC_LOG,g_id);
    mqttClient.publish(strTopicLog, strMsg);
  }
  
  void sendData(char *strMsg)
  {
    char strTopicData[50];
    sprintf(strTopicData,"%s/%s/%03d",TOPIC_PREFIX,TOPIC_DATA,g_id);
    mqttClient.publish(strTopicData, strMsg);
  }
  
  void sendMqttDataInfo()
  { 
    static char strJson[1000];
  
    jarduino_latched.makeJson(strJson);
    sendData(strJson);
  }
 
  void setup_mqtt(void)
  {
    g_id=jarduino_latched.serial;
    static char strName[15];
    sprintf(strName,"jarduino_%03d",g_id);
    mqttClient.setMqttClientName(strName);
    //Serial.print("Start Mqtt ");
    //Serial.print(strName);
    //Serial.println("...");
  
    mqttClient.enableMQTTPersistence();
  
    #ifdef MQTT_TRACE_ON
      mqttClient.enableDebuggingMessages();
    #endif
  }
  
  void onReceiveCmd(const String &payload)
  {
    //Serial.println(payload);
  }
  
  void onConnectionEstablished()
  {
    //Serial.println("Connection Mqtt.");
  
    char strTopicCmd[50];
    sprintf(strTopicCmd,"%s/%s/%03d",TOPIC_PREFIX,TOPIC_CMD,g_id);
    mqttClient.subscribe(strTopicCmd,onReceiveCmd);  
    sendLog("MQTT Connected");
  }
  
#endif


bool cbModbus_ReadCoils_pmp1(Modbus::ResultCode event, uint16_t transactionId, void* data)
{  
  if (event==Modbus::EX_SUCCESS)
  {    
    jarduino.pump1.flgForce=tmpBool[0];
    jarduino.pump1.flgEnabled=tmpBool[1];
    jarduino.pump1.flgAuto=tmpBool[2];
    jarduino.pump1.flgRemote=tmpBool[3];
    g_flgMbOK=true;
  }
  return true;
}

bool cbModbus_ReadCoils_pmp2(Modbus::ResultCode event, uint16_t transactionId, void* data)
{  
  if (event==Modbus::EX_SUCCESS)
  {    
    jarduino.pump2.flgForce=tmpBool[0];
    jarduino.pump2.flgEnabled=tmpBool[1];
    jarduino.pump2.flgAuto=tmpBool[2];
    jarduino.pump2.flgRemote=tmpBool[3];
    g_flgMbOK=true;
  }
  
  return true;
}

bool cbModbus_ReadHoldingReg_Hour(Modbus::ResultCode event, uint16_t transactionId, void* data)
{  
  if (event==Modbus::EX_SUCCESS)
  {    
    jarduino.year=tmpReg[0];
    jarduino.month=tmpReg[1];
    jarduino.day=tmpReg[2];

    jarduino.hour=tmpReg[3];
    jarduino.minute=tmpReg[4];
    jarduino.second=tmpReg[5];    
    g_flgMbOK=true;
  }
  
  return true;
}

bool cbModbus_ReadInputReg_Measures(Modbus::ResultCode event, uint16_t transactionId, void* data)
{  
  if (event==Modbus::EX_SUCCESS)
  {    
    jarduino.batt_dxV=tmpReg[0];
    jarduino.sun_dxV=tmpReg[1];
    jarduino.temp_deg=tmpReg[2];
    jarduino.hum_pc=tmpReg[3];
    g_flgMbOK=true;
  }
  
  return true;
}


void waitReadSlaveInfo(void)
{
  g_nbreMbNok=0;
  while (1)
  {
    digitalWrite(LED_BUILTIN,LOW);

    static uint16_t tmp[5];
    mb.readIreg(MODBUS_SLAVE_ADDRESS, 200, tmp, 3);
    while (mb.slave())
      mb.task();
    
    jarduino.version_jarduino=tmp[0];
    jarduino.serial=tmp[1];
    jarduino.version_soft=tmp[2];
    digitalWrite(LED_BUILTIN,HIGH);
    
    if (jarduino.version_jarduino==1)
      break;

    #ifdef BYPASS_MODBUS
      break;
    #endif

    for (int i=0;i<4;i++)
    {
      digitalWrite(LED_BUILTIN,LOW);
      delay(100);
      digitalWrite(LED_BUILTIN,HIGH);
      delay(100);      
    }

    g_nbreMbNok++;
    if (g_nbreMbNok>NBRE_MB_NOK_MAX)
    {
      g_flgMbOK=0;
      ESP.deepSleep(DEEPSLEEP_US);
    }
    
    delay(2000);
  }
  
  while (mb.slave())
    mb.task();
}


void readSlaveData(void)
{  
  mb.readHreg(MODBUS_SLAVE_ADDRESS, 0, tmpReg, 6,cbModbus_ReadHoldingReg_Hour);
  while (mb.slave())
  {
    mb.task();
    yield();
  }

  mb.readIreg(MODBUS_SLAVE_ADDRESS, 0, tmpReg, 4,cbModbus_ReadInputReg_Measures);
  while (mb.slave())
  {
    mb.task();
    yield();
  }

  mb.readCoil(MODBUS_SLAVE_ADDRESS, 10, tmpBool, 4,cbModbus_ReadCoils_pmp1);
  while (mb.slave())
  {
    mb.task();
    yield();
  }

  mb.readCoil(MODBUS_SLAVE_ADDRESS, 20, tmpBool, 4,cbModbus_ReadCoils_pmp2);
  while (mb.slave())
  {
    mb.task();
    yield();
  }
}

void writeCoil(unsigned short usAddr,bool flgValue)
{
  static bool tmp[2];
  tmp[0]=flgValue;
  mb.writeCoil(MODBUS_SLAVE_ADDRESS,usAddr, tmp, 1);
  while (mb.slave())
  {
    mb.task();
    yield();
  }  
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleTest()
{        
  static int val=0;
  char strTxt[10];
  sprintf(strTxt,"[Test OK] - %d\n",val++);  
  server.send(200, "text/plain", strTxt);
}

void handleInfo()
{        
  char strJson[1000];

  jarduino_latched.makeJson(strJson);
  
  server.send(200, "text/plain", strJson);
}


void setup() 
{
  g_flgProduce=true;
  g_nbreMbNok=0;
  g_flgMbOK=false;
  Serial.begin(115200);
  
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);  

  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,LOW);
  delay(500);
  digitalWrite(LED_BUILTIN,HIGH);
  delay(500);    

  mb.begin(&Serial);
  
  mb.master();
  waitReadSlaveInfo();

  #ifndef BYPASS_MODBUS
    g_flgMbOK=false;
    readSlaveData();  
    if (g_flgMbOK==false)
    {    
    }
  #else
    g_flgMbOK=true;
    
    jarduino.version_jarduino=1;
    jarduino.serial=0;
    jarduino.version_soft=7;

    jarduino.hour=19;
    jarduino.minute=45;
    jarduino.second=0;
  #endif

  memcpy(&jarduino_latched,&jarduino,sizeof(jarduino));
  
  // Chaque quart d'heure, on poste un message
  if ( ( (jarduino_latched.minute % 15) != 0 ) || (g_flgMbOK==false) )
  {
    ESP.deepSleep(DEEPSLEEP_US);
  }

  WiFi.mode(WIFI_STA);
  WiFi.hostname("Jarduino");
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

  if (g_flgHttpMode==true)
  {
    server.on(UriBraces("/test"), handleTest);
    server.on(UriBraces("/info"), handleInfo);
    server.onNotFound(handleNotFound);
    server.begin();
  }

  #ifdef ENABLE_MQTT
    setup_mqtt();   
  #endif

  g_t0Start_ms=millis();  
}

void loop() 
{     
  digitalWrite(LED_BUILTIN,LOW);
  g_flgMbOK=false;
  readSlaveData();
  digitalWrite(LED_BUILTIN,HIGH);

  #ifndef BYPASS_MODBUS
    if (g_flgMbOK==false)
      g_nbreMbNok++;
  #else
    g_nbreMbNok=0;
  #endif

  if (g_nbreMbNok>NBRE_MB_NOK_MAX)
  {
    g_nbreMbNok=0;
    ESP.deepSleep(DEEPSLEEP_US);
  }

  memcpy(&jarduino_latched,&jarduino,sizeof(jarduino));
  
  for (int i=0;i<100;i++)
  {
    delay(40);
    if (g_flgHttpMode==true)
      server.handleClient();
      
    #ifdef ENABLE_MQTT
      mqttClient.loop();
    #endif
      
  }

  #ifdef ENABLE_MQTT
    if ( (g_flgProduce==true) && (mqttClient.isConnected()) )
    {
      sendMqttDataInfo();
      g_flgProduce=false;
    }
  #endif

  unsigned long ul=millis();
  if (ul-g_t0Start_ms>DELAY_BEFORE_SLEEP_MS)
  {
    ESP.deepSleep(DEEPSLEEP_US);
  }
}
