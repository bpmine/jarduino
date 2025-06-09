/**
 * @file jard_rs485_master_test_standalone.ino
 * 
 * @brief Programme de test PRODUCTION pour la carte maitre des oyas
 * 
 * - Brancher un bandeau à LEDs 5V de 16 LEDs minimum
 * - Connecter un ESP01 avec le programme de test Wifi qui répond pong à ping sur le port serie
 * - Alimenter la carte en 12V ainsi que le POWER OYAs (même alim)
 * - Connecter une LED avec 1k sur le 12V commute du bus (Entre 1 et 4)
 * - Connecter un Usb serie/TTL sur le port de LOG / Serial3
 * - Connecter un USB/485 sur A/B du bus
 * 
 * Sur le PC faire tourner le programme loop.py entre les deux ports COMs détectés
 * 
 * Voir les etapes de test dans le code et suite les instructions sur le terminal arduino (Serial 0)...
*/
#include "pins.h"

#include <FastLED.h>

//#define INIT_TIME

#define NUM_LEDS  (16)
static CRGB _leds[NUM_LEDS];  ///< Tableau des LEDs

#include <Wire.h>
#include <RTClib.h>
static DS1307 _rtc;

#define STEP_LEDS   (0)
#define STEP_RTC    (1)
#define STEP_RS485  (2)
#define STEP_BUTTON (3)
#define STEP_12V    (4)
#define STEP_33V    (5)
#define STEP_WIFI   (6)

void test_set_led(int inx,int r=0,int g=255,int b=0)
{
  if (inx<NUM_LEDS)
  {
    _leds[inx]=CRGB(r,g,b);
    FastLED.setBrightness(100);
    FastLED.show();
  }
}

void go_in_error(int stp)
{
  Serial.println(">> ERROR! <<");
  test_set_led(stp,255,0,0);
  while(1)
  {
    yield();
    delay(10);
  }
}


void test_leds_rgb(void)
{
  Serial.println("Test LEDs RGB...");
  digitalWrite(PIN_PWR_LEDS,HIGH);
  digitalWrite(PIN_PWR_LEDS2,LOW);
  delay(1000);

  digitalWrite(PIN_PWR_LEDS,LOW);
  digitalWrite(PIN_PWR_LEDS2,HIGH);
  for (int i=0;i<NUM_LEDS;i++)
  {
    for (int j=0;j<NUM_LEDS;j++)
    {
      if (i==j)
        _leds[j]=CRGB(127,0,0);
      else
        _leds[j]=CRGB(0,0,0);
    }

    FastLED.setBrightness(255);
    FastLED.show();
    delay(250);    
  }

  for (int j=0;j<NUM_LEDS;j++)
    _leds[j]=CRGB(0,0,0);

  FastLED.setBrightness(10);
  FastLED.show();

  test_set_led(STEP_LEDS,0,255,0);
}

void test_rtc(void)
{
  Serial.println("Test RTC...");
  test_set_led(STEP_RTC,0,0,255); 
  Wire.begin();
  _rtc.begin();

  #ifdef INIT_TIME
    _rtc.adjust(DateTime(__DATE__, __TIME__));
    Serial.println("INIT TIME!");
  #endif
  
   DateTime now = _rtc.now();
   char strDte[30];
   sprintf(strDte,"%02d/%02d/%04d %02d:%02d:%02d",
   now.day(),
   now.month(),
   now.year(),
   now.hour(),
   now.minute(),
   now.second()
   );
   Serial.print("Date: ");
   Serial.println(strDte);

   if (now.year()!=2025)
   {
    Serial.println("Date lue NOK!");
    go_in_error(STEP_RTC);
   }
   test_set_led(STEP_RTC,0,255,0); 
}

/**
 * @brief Test RS485
 * 
 * Rappel des ports serie:
 * Serial 0: USB
 * Serial 1: RS485
 * Serial 2: Wifi
 * Serial 3: Log
*/
void test_send_rs485(void)
{
  Serial.println("Test RS485...");
  test_set_led(STEP_RS485,0,0,255);
  Serial1.begin(9600);
  Serial3.begin(9600);
  delay(500);

  Serial.println("  Purge RX buffers...");
  digitalWrite(PIN_TX_EN,LOW);
  delay(10);
  while (Serial3.available()>0)
    Serial1.read();
  while (Serial1.available()>0)
    Serial3.read();

  Serial.println("  Send MOTIF on Log");
  Serial3.print("MOTIF");
  delay(200);

  Serial.println("  Check MOTIF received on RS485/Serial 3");
  if (Serial1.available()<5)
    go_in_error(STEP_RS485);

  String recv=Serial1.readString();  
  Serial.println(recv);  
  if (recv!="MOTIF")
    go_in_error(STEP_RS485);

  Serial.println("  RS485 RX was OK!");
  
  delay(500);
  digitalWrite(PIN_TX_EN,HIGH);
  delay(50);
  Serial.println("  Send FITOM on RS485");
  Serial1.print("FITOM");
  delay(200);
  digitalWrite(PIN_TX_EN,LOW);

  Serial.println("  Check FITOM on log");
  if (Serial3.available()<5)
    go_in_error(STEP_RS485);

  recv=Serial3.readString();  
  Serial.println(recv);  
  if (recv!="FITOM")
    go_in_error(STEP_RS485);

  Serial.println("  TX was also OK!\n");
  
  test_set_led(STEP_RS485,0,255,0);   
}

void test_button(void)
{
  Serial.println("Test Bouton...");
  test_set_led(STEP_BUTTON,0,0,255); 
  if (digitalRead(PIN_TEST_BTN)==LOW)
    go_in_error(STEP_BUTTON);
  
  while (digitalRead(PIN_TEST_BTN)==HIGH)
    delay(100);
  test_set_led(STEP_BUTTON,0,255,0);     
}

void test_pwr_12V(void)
{
  Serial.println("Test POWER 12V...");
  for (int i=0;i<3;i++)
  {
    test_set_led(STEP_12V,0,0,255);
    digitalWrite(PIN_PWR_ON,HIGH);
    delay(2000);
    test_set_led(STEP_12V,0,0,0);
    digitalWrite(PIN_PWR_ON,LOW);
    delay(2000);
  }
  test_set_led(STEP_12V,0,255,0);
}

void test_pwr_33V(void)
{
  Serial.println("Test POWER 3.3V...");
  for (int i=0;i<3;i++)
  {    
    test_set_led(STEP_33V,0,0,255);
    digitalWrite(PIN_PWR_WIFI,LOW);
    digitalWrite(PIN_PWR_WIFI2,HIGH);
    delay(2000);
    test_set_led(STEP_33V,0,0,0);
    digitalWrite(PIN_PWR_WIFI,HIGH);
    digitalWrite(PIN_PWR_WIFI2,LOW);
    delay(2000);
  }
  test_set_led(STEP_33V,0,255,0);
}

void test_wifi_comm(void)
{
  test_set_led(STEP_WIFI,0,0,255);
  Serial.println("Test comm wifi...");
  Serial2.begin(9600);
  digitalWrite(PIN_PWR_WIFI,LOW);
  digitalWrite(PIN_PWR_WIFI2,HIGH);
  delay(5000);
  while (Serial2.available()>0)
    Serial2.read();    
  Serial.println("Send ping...");
  Serial2.print("ping\n");
  delay(1000);
  if (Serial2.available()<5)
    go_in_error(STEP_WIFI);
  
  String recv=Serial2.readString();
  Serial.print("Receive: ");
  Serial.println(recv);
  if (memcmp(recv.c_str(),"pong",4)!=0)
    go_in_error(STEP_WIFI);

  digitalWrite(PIN_PWR_WIFI,HIGH);
  digitalWrite(PIN_PWR_WIFI2,LOW);
  test_set_led(STEP_WIFI,0,255,0);
}

void setup() 
{
  pinMode(PIN_TX_EN,OUTPUT);
  digitalWrite(PIN_TX_EN,LOW);
  pinMode(PIN_TEST_BTN,INPUT_PULLUP);
  
  pinMode(PIN_PWR_ON,OUTPUT);
  digitalWrite(PIN_PWR_ON,LOW);  
  
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,LOW);
  
  pinMode(PIN_DATA_LEDS,OUTPUT);
  digitalWrite(PIN_DATA_LEDS,LOW);

  pinMode(PIN_PWR_LEDS,OUTPUT);
  digitalWrite(PIN_PWR_LEDS,HIGH);
  pinMode(PIN_PWR_LEDS2,OUTPUT);
  digitalWrite(PIN_PWR_LEDS2,LOW);

  pinMode(PIN_PWR_WIFI,OUTPUT);
  digitalWrite(PIN_PWR_WIFI,HIGH);
  pinMode(PIN_PWR_WIFI2,OUTPUT);
  digitalWrite(PIN_PWR_WIFI2,LOW);
  
  /*pinMode(11,INPUT);
  pinMode(12,INPUT);
  pinMode(13,INPUT);*/

  FastLED.addLeds<NEOPIXEL, PIN_DATA_LEDS>(_leds, NUM_LEDS);
  for (int j=0;j<NUM_LEDS;j++)
    _leds[j]=CRGB(0,0,0);    

  delay(100);
  Serial.begin(9600);  
  Serial.println("Boot Tst Std");
  
  test_leds_rgb();  
  test_rtc();
  test_send_rs485();
  test_button();
  test_pwr_12V();  
  test_pwr_33V();
  test_wifi_comm();

  Serial.println("_________");
  Serial.println("TEST OK !");
  delay(2000);
  for (int j=0;j<NUM_LEDS;j++)
    _leds[j]=CRGB(0,0,0);    
  FastLED.show();
} 


void loop() 
{
}
