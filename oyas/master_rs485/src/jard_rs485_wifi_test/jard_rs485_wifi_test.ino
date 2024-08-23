#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <string.h>

#include <uri/UriBraces.h>

#include "wifi_code.h"

#define VERSION "V0.3"
//#define MODE_SIMU
#define MODE_AP
//#define USE_OTA

#ifdef USE_OTA
  #include <ArduinoOTA.h>
#endif

#ifdef MODE_AP
  const char ssid[] = "Master";
  const char password[] = "Master38";  ///< Minimum 8 chars
  
  IPAddress local_IP(192,168,0,1);
  IPAddress gateway(192,168,0,1);
  IPAddress subnet(255,255,255,0);  
#else
  const char* ssid = STASSID;      ///< Voir le fichier wifi_code.h
  const char* password = STAPSK;    ///< Voir le fichier wifi_code.h
#endif


ESP8266WebServer server(80);    ///< Instance du serveur HTTP

void handleInfo()
{
  char strList[30];
    
  String s="{\n";
  s+="  \"name\":\"master\",\n";  
  s+="}\n";
  
  server.send(200, "text/plain", s);
}

/**
 * @brief Controleur WS en cas d'erreur
*/
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

/**
 * @brief Setup d'initialisation de l'arduino
*/
/*void setup(void)
{ 
  Serial.begin(115200);
  char tmp[20];
  sprintf(tmp,"BOOT %s",VERSION);
  Serial.println(tmp);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

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

  if (MDNS.begin("esp8266")) 
  {
    Serial.println("MDNS responder started");
  }

  server.on(UriBraces("/info"), handleInfo);
  server.onNotFound(handleNotFound);

  server.begin();
  server.enableCORS(true);
  Serial.println("HTTP server started");

  #ifdef USE_OTA
    ArduinoOTA.setHostname(OTA_NAME);

    // No authentication by default
    ArduinoOTA.setPassword((const char *)"123");
  
    ArduinoOTA.onStart([]() {
      Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
  #endif  
}*/

void setup(void)
{
  Serial.begin(9600);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  
}

void serialEvent(void)
{
  String msg=Serial.readStringUntil('\n');
  if (msg=="ping")
    Serial.println("pong");
  else
    Serial.println(msg);
}

int c=80;
void loop(void)
{
  c--;
  if (c<=0)
  {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    c=80;
  }
  delay(10);
}

/**
 * @brief Boucle cyclique de l'arduino
 * */
/*void loop()
{
  server.handleClient();
  MDNS.update();

  #ifdef USE_OTA
    ArduinoOTA.handle();
  #endif
 
  delay(10);
}*/
