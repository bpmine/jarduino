#include "flowmeter.hpp"

#define PIN_FLOW    2
#define PIN_EV      4
#define PIN_PMP     5

#define PIN_CMD_IN  7

#define IRQ_FLOW    0

bool g_cmdON=false;
bool g_flgLastCmdON=false;

FlowMeter fm;

void fm_irq_wrapper()
{
  fm.irqCounter();
}

void setup() 
{
  Serial.begin(9600);
  
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,LOW);

  pinMode(PIN_EV,OUTPUT);
  digitalWrite(PIN_EV,LOW);
  pinMode(PIN_PMP,OUTPUT);
  digitalWrite(PIN_EV,LOW);
  pinMode(PIN_CMD_IN,INPUT);
  pinMode(PIN_FLOW,INPUT);
  
  g_cmdON=false;
  g_flgLastCmdON=false;  
  fm.begin(IRQ_FLOW,fm_irq_wrapper);
}

void loop() 
{
  g_cmdON=false;

  for (int i=0;i<10;i++)
  {
    if (digitalRead(PIN_CMD_IN)==HIGH)
    {
      g_cmdON=false;
      break;
    }
    else
    {
      g_cmdON=true;
    }

    delay(10);    
  }

  if (g_cmdON!=g_flgLastCmdON)
  {
    if (g_cmdON==true)
    {
      digitalWrite(PIN_EV,HIGH);
      delay(1000);
      digitalWrite(PIN_PMP,HIGH);      
    }
    else
    {
      digitalWrite(PIN_PMP,LOW);
      delay(1000);
      digitalWrite(PIN_EV,LOW);
      delay(1000);
    }
  }

  digitalWrite(PIN_EV,g_cmdON==true?HIGH:LOW);
  digitalWrite(PIN_PMP,g_cmdON==true?HIGH:LOW);  

  g_flgLastCmdON=g_cmdON;

  if (fm.tick()==true)
  {
    float rate=fm.getFlowRate_LpH();
    unsigned long tot_mL=fm.getTotal_mL();

    Serial.print("Flow rate: ");
    Serial.print(int(rate));
    Serial.print("L/min");
    Serial.print("    ");
        
    Serial.print("Quantity: ");        
    Serial.print(tot_mL);
    Serial.print("mL"); 
    Serial.print("   ");
    Serial.print(tot_mL/1000);
    Serial.println("L");
  }
    
  delay(10);
}
