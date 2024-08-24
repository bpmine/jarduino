/**
 * @file jard_wemos_mqtt.ino
 * @brief PROGRAMME PRINCIPAL - Gestion d'un noeud WIFI-IO de controle de cuve
 * 
 * @warning Connecter D0 (GPIO16) a RST et mettre D0 en entree sans pullup
*/
#include <ESP8266WiFi.h>
#include <EspMQTTClient.h>

#include "timer.h"
#include "analog.hpp"

#define NODE_MAIN
//#define NODE_PAUL
//#define NODE_REDUIT
//#define NODE_BARBEC
//#define NODE_TEST

#define VERSION "V1.0"

/**
 * @brief Reglages
 * @{
*/
#define TIME_BEFORE_SLEEP_S   (60*60)   ///< Temps au bout duquel on va en veille quand on ne recoit plus de commandes
#define TIME_BEFORE_FIRST_S   (3*60)    ///< Temps laisse au serveur pour repondre avant de partir en veille (apres un boot)
#define TIME_REPEAT_FIRST_S   (20)      ///< Frequence de repetition des envois de data (apres un boot)
#define TIME_CYCLE_MS         (100)     ///< Temps de cycle
#define TIMEOUT_COMM_S        (5)       ///< Temps au bout duquel on considere la comm KO (provoque la coupure de la pompe)
#define SLEEPING_TIME_MIN     (30)      ///< Temps de veille du micro avant de re-donner des nouvelles au reseau 
/**
 * @}
*/

/**
 * @brief Credentials Wifi et MQTT
 * @{
*/
#define WIFI_ID       "Domotique"
#define WIFI_PWD      "94582604"
#define MQTT_IP       "192.168.3.200"
#define MQTT_LOGIN    "toto"
#define MQTT_PWD      "toto"
/**
 * @}
*/

/**
 * @brief Brochage du micro
 * @{
*/
#define PIN_IN_N1       14
#define PIN_IN_N2       12
#define PIN_IN_N3       13
#define PIN_OUT_CMD     15
#define PIN_LED         2
#define PIN_ANALOG_POW  A0
#define PIN_WAKEUP      16    ///< (D0 du weemos)
/**
 * @}
*/

//#define MQTT_TRACE_ON

/**
 * @brief Sujets MQTT
 * @{
*/
#define TOPIC_PREFIX  "/wifiio"
#define TOPIC_CMD     "cmd"
#define TOPIC_DATA    "data"
#define TOPIC_LOG     "log"
/**
 * @}
*/

bool g_comm_ok=false;
bool g_lvl_1=false;
bool g_lvl_2=false;
bool g_lvl_3=false;
int g_rssi=0;
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

/**
 * @brief Instance du client MQTT qui va gerer le bordel
*/
EspMQTTClient mqttClient(
  WIFI_ID,    ///< ID Wifi
  WIFI_PWD,   ///< Mot de passe Wifi
  MQTT_IP,    ///< Adresse IP du broker de message
  MQTT_LOGIN, ///< Login MQTT
  MQTT_PWD,   ///< Mot de passe MQTT
  NAME        ///< Nom du noeud (ID MQTT - @warning Doit etre unique et associe au noeud)
);

/**
 * @brief Envoi d'un log MQTT
 * @param[in] Message a logguer
*/
void sendLog(char *strMsg)
{
  char strTopicLog[50];
  sprintf(strTopicLog,"%s/%s/%s",TOPIC_PREFIX,TOPIC_LOG,NAME);
  Serial.print("Log into ");
  Serial.println(strTopicLog);
  mqttClient.publish(strTopicLog, strMsg);
}

/**
 * @brief Ajout d'un Bool au texte JSON donne
 * @param[in,out] strJson Texte JSON en cours (on y ajoute le champ)
 * @param[in] strKey Nom du champ
 * @param[in] val Valeur du champ
*/
void addBool(char *strJson,const char *strKey,bool val)
{
  strcat(strJson,"\"");
  strcat(strJson,strKey);
  strcat(strJson,"\":");
  if (val==true)
    strcat(strJson,"true");
  else
    strcat(strJson,"false");  
}

/**
 * @brief Ajout d'un Int au texte JSON donne
 * @param[in,out] strJson Texte JSON en cours (on y ajoute le champ)
 * @param[in] strKey Nom du champ
 * @param[in] val Valeur du champ
*/
void addInt(char *strJson,const char *strKey,int val)
{
  strcat(strJson,"\"");
  strcat(strJson,strKey);
  strcat(strJson,"\":");
  char tmp[15];
  sprintf(tmp,"%d",val);
  strcat(strJson,tmp);
}

/**
 * @brief Ajout d'un UShort au texte JSON donne
 * @param[in,out] strJson Texte JSON en cours (on y ajoute le champ)
 * @param[in] strKey Nom du champ
 * @param[in] val Valeur du champ
*/
void addUshort(char *strJson,const char *strKey,unsigned short val)
{
  strcat(strJson,"\"");
  strcat(strJson,strKey);
  strcat(strJson,"\":");
  char tmp[15];
  sprintf(tmp,"%d",val);
  strcat(strJson,tmp);
}

/**
 * @brief Ajout d'un ULong au texte JSON donne
 * @param[in,out] strJson Texte JSON en cours (on y ajoute le champ)
 * @param[in] strKey Nom du champ
 * @param[in] val Valeur du champ
*/
void addULong(char *strJson,const char *strKey,unsigned long val)
{
  strcat(strJson,"\"");
  strcat(strJson,strKey);
  strcat(strJson,"\":");
  char tmp[20];
  sprintf(tmp,"%ld",val);
  strcat(strJson,tmp);
}

/**
 * @brief Envoi de la donnee relative au noeud dans le topic associe
*/
void sendData(void)
{
  char strTopicLog[50];
  sprintf(strTopicLog,"%s/%s/%s",TOPIC_PREFIX,TOPIC_DATA,NAME);
  Serial.print("Data to ");
  Serial.println(strTopicLog);

  char strData[255]="";
  strcat(strData,"{");
  addBool(strData,"cmd",g_cmd_pump);
  strcat(strData,",");
  addBool(strData,"n1",g_lvl_1);
  strcat(strData,",");
  addBool(strData,"n2",g_lvl_2);
  strcat(strData,",");
  addBool(strData,"n3",g_lvl_3);
  strcat(strData,",");
  addInt(strData,"rssi",g_rssi);
  strcat(strData,",");
  unsigned short dxvolt=120*anVoltage.get()/939;
  addUshort(strData,"pwr",dxvolt);
  strcat(strData,",");
  addULong(strData,"tick",g_tick_s);
  strcat(strData,"}");
  
  mqttClient.publish(strTopicLog, strData);
}

/**
 * @brief Appelee lorsque une commande a ete recue sur le topic du noeud
 * @param[in] payload Message recu
*/
void onReceiveCmd(const String &payload)
{
  bool dont_send=false;

  digitalWrite(PIN_LED,LOW);
  
  Serial.print("Cmd: ");
  Serial.println(payload);

  tmrBeforeFirst.stop();    ///< Plus dans le cas d'attente de la premiere reponse (premier)
  tmrRepeatFirst.stop();    ///< Idem (pour les re-emissions)

  if (tmrGotoSleep.isRunning()==true)
  {
    Serial.println("Cmd ignoree car veille demandee.");
    digitalWrite(PIN_LED,HIGH);    
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
    dont_send=true;
  }
  else if (payload=="ping")
    sendLog("pong");
  else if (payload=="tstwdg")
  {
    while (1);
  }
  else if (payload=="tstwdg2")
  {
    ESP.wdtDisable();
    while (1);
  }

  sendData();

  if ( g_cmd_pump==true )
    digitalWrite(PIN_LED,HIGH);
  
  g_comm_ok=true;
  tmrComm.start();
  tmrBeforeSleep.start();   ///< On va en veille naturellement 1h apres la derniere commande    
}

/**
 * @brief Appelee lorsque le client MQTT vient de se connecter
 * 
 * - On envoi un log sur MQTT pour le dire
 * - on envoi un log en cas de boot anormal
*/
void onConnectionEstablished()
{
  char strTopicCmd[50];
  sprintf(strTopicCmd,"%s/%s/%s",TOPIC_PREFIX,TOPIC_CMD,NAME);
  mqttClient.subscribe(strTopicCmd,onReceiveCmd);  

  char strConnMsg[50];
  sprintf(strConnMsg,"Connected %s (%s %s)",VERSION,__DATE__,__TIME__);
  
  sendLog(strConnMsg);
  sendData();
  
  if (g_boot_event_sent==false)
  {
    switch (g_eBootReason)
    {
      case NORMAL:break;
      case SOFT_WDG:sendLog("SOFT WDG!");break;
      case HARD_WDG:sendLog("HARD WDG!");break;
      case OTHER:sendLog("OTHER BOOT!");break;
    }
    
    g_boot_event_sent=true;
  }
}

/**
 * @brief Initialise la communication MQTT (Et Wifi)
*/
void setup_mqtt(void)
{
  static char strName[30];
  sprintf(strName,"wifiio_%s",NAME);
  mqttClient.setMqttClientName(strName);
  
  mqttClient.enableMQTTPersistence();

  #ifdef MQTT_TRACE_ON
    mqttClient.enableDebuggingMessages();
  #endif
}

/**
 * @brief Genere un point (code Morse)
*/
inline void _point(void)
{
  digitalWrite(PIN_LED,LOW);
  delay(100);
  digitalWrite(PIN_LED,HIGH);
  delay(200);  
}

/**
 * @brief Genere un trait (code Morse)
*/
inline void _trait(void)
{
  digitalWrite(PIN_LED,LOW);
  delay(400);
  digitalWrite(PIN_LED,HIGH);
  delay(200);    
}

/**
 * @brief Envoi une dizaine de S.O.S. avec la LED
*/
inline void _sos(void)
{
  for (int i=0;i<10;i++)
  {
    digitalWrite(PIN_OUT_CMD,LOW);
    yield();
      
    for (int j=0;j<3;j++)
      _point();
    delay(100);
      
    for (int j=0;j<3;j++)
      _trait();
    delay(100);
      
    for (int j=0;j<3;j++)
      _point();
        
    delay(1000);
  }
}

/**
 * @brief SETUP ARDUINO
*/
void setup()
{
  pinMode(PIN_OUT_CMD,OUTPUT);
  digitalWrite(PIN_OUT_CMD,LOW);
  //pinMode(PIN_WAKEUP,WAKEUP_PULLUP);
  pinMode(PIN_WAKEUP,INPUT); ///< Pour ne pas perturber le processus de reveil
  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED,LOW);

  pinMode(PIN_IN_N1,INPUT_PULLUP);
  pinMode(PIN_IN_N2,INPUT_PULLUP);
  pinMode(PIN_IN_N3,INPUT_PULLUP);

  pinMode(PIN_ANALOG_POW,INPUT);
  
  Serial.begin(115200);
  delay(500);
  Serial.println('Boot');
  Serial.setDebugOutput(true);  

  g_eBootReason=OTHER;
  g_boot_event_sent=false;  ///< Preparer l'envoi MQTT d'un log associe a la cause de boot
  String resetReason = ESP.getResetReason();
  Serial.print("Reset reason: ");
  Serial.println(resetReason);
  
  if (resetReason == "External System")
    g_eBootReason=NORMAL;
  else if (resetReason == "Software Watchdog")
    g_eBootReason=SOFT_WDG;
  else if (resetReason == "Hardware Watchdog")
    g_eBootReason=HARD_WDG;

  /// @remark En cas de boot provoque par un WDG, On envoi une dizaine de S.O.S. avec la LED avant de poursuivre le boot
  if ( (g_eBootReason==SOFT_WDG) || (g_eBootReason==HARD_WDG) )
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

/**
 * @brief LOOP ARDUINO
 * 
 * - Lecture des entrees
 * - Lecture de la tension et du RSSI sur cycle plus long
 * - Surveillance et process de la comm MQTT
 * - Gestion de la pompe
 * - Gestion du premier envoi de data apres boot
 * - Gestion du passage en veille
 * - Ecriture des sorties
*/
void loop() 
{  
  /// @remark Lecture cyclique (rapide) des entrees
  if (digitalRead(PIN_IN_N1)==LOW)
     g_lvl_1=true;
  else
     g_lvl_1=false;
  
  if (digitalRead(PIN_IN_N2)==LOW)
     g_lvl_2=true;
  else
     g_lvl_2=false;
  
  if (digitalRead(PIN_IN_N3)==LOW)
     g_lvl_3=true;
  else
     g_lvl_3=false;

  /// @remark Lecture cyclique (lente) de la tension et du signal RSSI
  if (tmrCycle.tick())
  {
    unsigned short tmp=analogRead(PIN_ANALOG_POW);     
    anVoltage.latch(tmp);
  
    g_rssi=WiFi.RSSI();   
  }

  /// @remark Detection plus de reception MQTT
  if (tmrComm.tick()==true)
     g_comm_ok=false;

  /// @remark Pompe OFF si plus de comm
  if (g_comm_ok==false)
    g_cmd_pump=false;

  /// @remark Pompe OFF si en train d'aller en sommeil
  if (tmrGotoSleep.isRunning())
    g_cmd_pump=false;

  /// @remark Boucle MQTT
  mqttClient.loop();

  /// @remark Tant que l'on est dans la phase apres boot, on repete l'envoi des datas (en esperant la reception d'une commande)
  if (tmrBeforeFirst.isRunning())
  {    
    if (tmrRepeatFirst.tick()==true)
      sendData();
  }

  /// @remark A la fin du temps alloue apres le boot, on passe en sommeil si aucune commande recue
  if ( tmrBeforeFirst.tick()==true )
  {
    Serial.println("Pas de reponse...");
    sendLog("No answer means sleep...");
    tmrGotoSleep.start();
  }

  /// @remark Au bout du temps sans commande on demande la mise en veille (au bout de tmrGotoSleep)
  if ( tmrBeforeSleep.tick()==true )
  {
    Serial.println("Sleep...");
    sendLog("Now time to sleep...");
    tmrGotoSleep.start();
  }

  /// @remark Au bout du timer de mise en veille, on y va....
  if (tmrGotoSleep.tick()==true)
  {
    g_cmd_pump=false;
    digitalWrite(PIN_OUT_CMD,LOW);
    digitalWrite(PIN_LED,HIGH);
    ESP.deepSleep( (unsigned long)SLEEPING_TIME_MIN * 60UL * 1000000UL );
  }

  /// @remark Comptage des secondes
  if (tmrTick1S.tick()==true)
    g_tick_s++;

  /// @remark Application de la consigne de pompe (CMD + LED)
  pinMode(PIN_OUT_CMD,OUTPUT);
  if ( g_cmd_pump==true )
  {
    digitalWrite(PIN_OUT_CMD,HIGH);
    digitalWrite(PIN_LED,LOW);
  }
  else
  {
    digitalWrite(PIN_OUT_CMD,LOW);
    digitalWrite(PIN_LED,HIGH);
  }    
}
