/**
 * @file jard_c3_mqtt.ino
 * @brief ESP32-C3 - Gestion d'un noeud WIFI-IO de controle de cuve
 * 
 * NOLOGO - ESP C3 super mini
 */

#include <WiFi.h>
#include <EspMQTTClient.h>

#include "timer.h"
#include "analog.hpp"

#include "esp_system.h"
#include "esp_sleep.h"
// #include "esp_task_wdt.h"       // Optionnel si tu veux gérer le WDT proprement

//#define NODE_MAIN
//#define NODE_PAUL
//#define NODE_REDUIT
#define NODE_BARBEC
//#define NODE_TEST

#define VERSION "V1.0-ESP32C3"

/**
 * Reglages
 */
#define TIME_BEFORE_SLEEP_S   (60*60)
#define TIME_BEFORE_FIRST_S   (3*60)
#define TIME_REPEAT_FIRST_S   (20)
#define TIME_CYCLE_MS         (100)
#define TIMEOUT_COMM_S        (5)
#define SLEEPING_TIME_MIN     (30)

/**
 * Credentials Wifi et MQTT
 */
#define WIFI_ID       "Domotique"
#define WIFI_PWD      "94582604"
#define MQTT_IP       "192.168.3.200"
#define MQTT_LOGIN    "toto"
#define MQTT_PWD      "toto"

/**
 * Brochage ESP32-C3 (EXEMPLES) : A ADAPTER !
 * - Choisis des GPIO dispo sur ta carte C3
 * - Pour l'ADC, il faut un GPIO ADC (ADC1) supporté par ton module
 */
#define PIN_IN_N1       3
#define PIN_IN_N2       4
#define PIN_IN_N3       5
#define PIN_OUT_CMD     10
#define PIN_LED_ROUGE   LED_BUILTIN
//#define PIN_LED_VERTE   21

// ADC: sur ESP32-C3, analogRead() se fait sur un GPIO ADC (ex: GPIO0/1/2/3/4 selon carte)
// Mets ici un GPIO réellement câblé vers ta mesure tension
#define PIN_ANALOG_POW  1

/**
 * Sujets MQTT
 */
#define TOPIC_PREFIX  "/wifiio"
#define TOPIC_CMD     "cmd"
#define TOPIC_DATA    "data"
#define TOPIC_LOG     "log"

bool g_comm_ok=false;
bool g_lvl_1=false;
bool g_lvl_2=false;
bool g_lvl_3=false;
int  g_rssi=0;
bool g_cmd_pump=false;
unsigned long g_tick_s=0;

typedef enum
{
  OTHER,
  NORMAL,
  HARD_WDG,
  SOFT_WDG
} E_BOOT_REASON;

E_BOOT_REASON g_eBootReason;
bool g_boot_event_sent=false;

Analog anVoltage=Analog();

Timer tmrComm((unsigned long)TIMEOUT_COMM_S*1000UL,false);
Timer tmrCycle((unsigned long)TIME_CYCLE_MS,false);
Timer tmrBeforeSleep((unsigned long)TIME_BEFORE_SLEEP_S*1000UL);
Timer tmrGotoSleep(5000UL);
Timer tmrTick1S(1000UL,false);

Timer tmrBeforeFirst((unsigned long)TIME_BEFORE_FIRST_S*1000UL);
Timer tmrRepeatFirst((unsigned long)TIME_REPEAT_FIRST_S*1000UL,false);

#ifdef NODE_TEST
  char NAME[]="test";
#endif
#ifdef NODE_MAIN
  char NAME[]="main";
#endif
#ifdef NODE_PAUL
  char NAME[]="paul";
#endif
#ifdef NODE_REDUIT
  char NAME[]="reduit";
#endif
#ifdef NODE_BARBEC
  char NAME[]="barbec";
#endif

EspMQTTClient mqttClient(
  WIFI_ID,
  WIFI_PWD,
  MQTT_IP,
  MQTT_LOGIN,
  MQTT_PWD,
  NAME
);

void sendLog(const char *strMsg)
{
  char strTopicLog[64];
  snprintf(strTopicLog,sizeof(strTopicLog),"%s/%s/%s",TOPIC_PREFIX,TOPIC_LOG,NAME);
  Serial.print("Log into ");
  Serial.println(strTopicLog);
  mqttClient.publish(strTopicLog, strMsg);
}

void addBool(char *strJson,const char *strKey,bool val)
{
  strcat(strJson,"\"");
  strcat(strJson,strKey);
  strcat(strJson,"\":");
  strcat(strJson, val ? "true" : "false");
}

void addInt(char *strJson,const char *strKey,int val)
{
  strcat(strJson,"\"");
  strcat(strJson,strKey);
  strcat(strJson,"\":");
  char tmp[16];
  snprintf(tmp,sizeof(tmp),"%d",val);
  strcat(strJson,tmp);
}

void addUshort(char *strJson,const char *strKey,unsigned short val)
{
  strcat(strJson,"\"");
  strcat(strJson,strKey);
  strcat(strJson,"\":");
  char tmp[16];
  snprintf(tmp,sizeof(tmp),"%u",val);
  strcat(strJson,tmp);
}

void addULong(char *strJson,const char *strKey,unsigned long val)
{
  strcat(strJson,"\"");
  strcat(strJson,strKey);
  strcat(strJson,"\":");
  char tmp[24];
  snprintf(tmp,sizeof(tmp),"%lu",val);
  strcat(strJson,tmp);
}

void sendData(void)
{
  char strTopicData[64];
  snprintf(strTopicData,sizeof(strTopicData),"%s/%s/%s",TOPIC_PREFIX,TOPIC_DATA,NAME);
  Serial.print("Data to ");
  Serial.println(strTopicData);

  char strData[255]="";
  strcat(strData,"{");
  addBool(strData,"cmd",g_cmd_pump); strcat(strData,",");
  addBool(strData,"n1",g_lvl_1);     strcat(strData,",");
  addBool(strData,"n2",g_lvl_2);     strcat(strData,",");
  addBool(strData,"n3",g_lvl_3);     strcat(strData,",");
  addInt (strData,"rssi",g_rssi);    strcat(strData,",");

  // ⚠️ Calibration ADC différente sur ESP32-C3 !
  // Tu auras probablement besoin d'ajuster cette formule.
  // Ici on garde ton ancien calcul en "placeholder".
  unsigned short dxvolt = (unsigned short)(120UL * anVoltage.get() / 3286UL);
  addUshort(strData,"pwr",dxvolt);   strcat(strData,",");
  addULong(strData,"tick",g_tick_s);
  strcat(strData,"}");

  mqttClient.publish(strTopicData, strData);
}

void onReceiveCmd(const String &payload)
{
  digitalWrite(PIN_LED_ROUGE,LOW);

  Serial.print("Cmd: ");
  Serial.println(payload);

  tmrBeforeFirst.stop();
  tmrRepeatFirst.stop();

  if (tmrGotoSleep.isRunning())
  {
    Serial.println("Cmd ignoree car veille demandee.");
    digitalWrite(PIN_LED_ROUGE,HIGH);
    return;
  }

  if (payload=="on")
    g_cmd_pump=true;
  else if (payload=="off")
    g_cmd_pump=false;
  else if (payload=="sleep")
  {
    g_cmd_pump=false;
    sendLog("Demande de faire dodo!");
    tmrGotoSleep.start();
  }
  else if (payload=="ping")
    sendLog("pong");
  else if (payload=="tstwdg")
  {
    while (1) { } // laisser le watchdog faire son boulot
  }
  else if (payload=="tstwdg2")
  {
    // Sur ESP32, désactiver le WDT n'est pas "ESP.wdtDisable()".
    // Si tu en as besoin, on le fera proprement avec esp_task_wdt_*.
    sendLog("tstwdg2 non implemente (ESP32)");
    while (1) { }
  }

  sendData();

  if (g_cmd_pump)
    digitalWrite(PIN_LED_ROUGE,HIGH);

  g_comm_ok=true;
  tmrComm.start();
  tmrBeforeSleep.start();
}

void onConnectionEstablished()
{
  char strTopicCmd[64];
  snprintf(strTopicCmd,sizeof(strTopicCmd),"%s/%s/%s",TOPIC_PREFIX,TOPIC_CMD,NAME);
  mqttClient.subscribe(strTopicCmd,onReceiveCmd);

  char strConnMsg[80];
  snprintf(strConnMsg,sizeof(strConnMsg),"Connected %s (%s %s)",VERSION,__DATE__,__TIME__);

  sendLog(strConnMsg);
  sendData();

  if (!g_boot_event_sent)
  {
    switch (g_eBootReason)
    {
      case NORMAL:   break;
      case SOFT_WDG: sendLog("SOFT WDG!"); break;
      case HARD_WDG: sendLog("HARD WDG!"); break;
      case OTHER:    sendLog("OTHER BOOT!"); break;
    }
    g_boot_event_sent=true;
  }
}

void setup_mqtt(void)
{
  static char strName[32];
  snprintf(strName,sizeof(strName),"wifiio_%s",NAME);
  mqttClient.setMqttClientName(strName);

  mqttClient.enableMQTTPersistence();
  mqttClient.enableDebuggingMessages(); // si tu veux tracer
}

inline void _point(void)
{
  digitalWrite(PIN_LED_ROUGE,LOW);
  delay(100);
  digitalWrite(PIN_LED_ROUGE,HIGH);
  delay(200);
}

inline void _trait(void)
{
  digitalWrite(PIN_LED_ROUGE,LOW);
  delay(400);
  digitalWrite(PIN_LED_ROUGE,HIGH);
  delay(200);
}

inline void _sos(void)
{
  for (int i=0;i<10;i++)
  {
    digitalWrite(PIN_OUT_CMD,LOW);

    for (int j=0;j<3;j++) _point();
    delay(100);

    for (int j=0;j<3;j++) _trait();
    delay(100);

    for (int j=0;j<3;j++) _point();

    delay(1000);
  }
}

static E_BOOT_REASON mapBootReason()
{
  // ESP32: cause de reset
  esp_reset_reason_t r = esp_reset_reason();

  switch (r)
  {
    case ESP_RST_POWERON:
    case ESP_RST_SW:
    case ESP_RST_EXT:
      return NORMAL;

    case ESP_RST_TASK_WDT:
      return SOFT_WDG;

    case ESP_RST_WDT:
    case ESP_RST_PANIC:
      return HARD_WDG;

    default:
      return OTHER;
  }
}

void setup()
{
  pinMode(PIN_OUT_CMD,OUTPUT);
  digitalWrite(PIN_OUT_CMD,LOW);

  pinMode(PIN_LED_ROUGE,OUTPUT);
  digitalWrite(PIN_LED_ROUGE,LOW);

  pinMode(PIN_IN_N1,INPUT_PULLUP);
  pinMode(PIN_IN_N2,INPUT_PULLUP);
  pinMode(PIN_IN_N3,INPUT_PULLUP);

  pinMode(PIN_ANALOG_POW,INPUT);

  Serial.begin(9600);
  delay(300);
  Serial.println("Boot");

  g_eBootReason = mapBootReason();
  g_boot_event_sent = false;

  if ((g_eBootReason==SOFT_WDG) || (g_eBootReason==HARD_WDG))
    _sos();

  setup_mqtt();

  g_comm_ok=false;

  tmrComm.start();
  tmrCycle.start();
  tmrBeforeSleep.start();
  tmrBeforeFirst.start();
  tmrRepeatFirst.start();
  tmrTick1S.start();
}

void goDeepSleep()
{
  g_cmd_pump=false;
  digitalWrite(PIN_OUT_CMD,LOW);
  digitalWrite(PIN_LED_ROUGE,HIGH);

  // Réveil timer uniquement (simple et fiable)
  uint64_t us = (uint64_t)SLEEPING_TIME_MIN * 60ULL * 1000000ULL;
  esp_sleep_enable_timer_wakeup(us);
  esp_deep_sleep_start();
}

void loop()
{
  // Lecture des niveaux
  g_lvl_1 = (digitalRead(PIN_IN_N1)==LOW);
  g_lvl_2 = (digitalRead(PIN_IN_N2)==LOW);
  g_lvl_3 = (digitalRead(PIN_IN_N3)==LOW);

  // Lecture lente tension + RSSI
  if (tmrCycle.tick())
  {
    unsigned short tmp = (unsigned short)analogRead(PIN_ANALOG_POW);
    anVoltage.latch(tmp);

    g_rssi = WiFi.RSSI();
  }

  // Timeout comm
  if (tmrComm.tick())
    g_comm_ok=false;

  // Pompe OFF si plus de comm
  if (!g_comm_ok)
    g_cmd_pump=false;

  // Pompe OFF si on va dormir
  if (tmrGotoSleep.isRunning())
    g_cmd_pump=false;

  // MQTT
  mqttClient.loop();

  // Phase post-boot : on renvoie les data en boucle
  if (tmrBeforeFirst.isRunning())
  {
    if (tmrRepeatFirst.tick())
      sendData();
  }

  // Fin du temps post-boot sans commande -> sommeil
  if (tmrBeforeFirst.tick())
  {
    Serial.println("Pas de reponse...");
    sendLog("No answer means sleep...");
    tmrGotoSleep.start();
  }

  // Au bout d'1h sans commande -> demande de sommeil
  if (tmrBeforeSleep.tick())
  {
    Serial.println("Sleep...");
    sendLog("Now time to sleep...");
    tmrGotoSleep.start();
  }

  // Deadline avant sommeil -> deep sleep
  if (tmrGotoSleep.tick())
    goDeepSleep();

  // Tick secondes
  if (tmrTick1S.tick())
    g_tick_s++;

  // Application sortie pompe + LED
  if (g_cmd_pump)
  {
    digitalWrite(PIN_OUT_CMD,HIGH);
    digitalWrite(PIN_LED_ROUGE,LOW);
  }
  else
  {
    digitalWrite(PIN_OUT_CMD,LOW);
    digitalWrite(PIN_LED_ROUGE,HIGH);
  }
}
