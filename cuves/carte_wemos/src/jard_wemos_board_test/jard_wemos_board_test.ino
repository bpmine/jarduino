#define PIN_IN_N1       14
#define PIN_IN_N2       12
#define PIN_IN_N3       13
#define PIN_OUT_CMD     15
#define PIN_LED         2
#define PIN_ANALOG_POW  A0

void setup() 
{
  pinMode(PIN_OUT_CMD,OUTPUT);
  digitalWrite(PIN_OUT_CMD,LOW);
  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED,LOW);

  pinMode(PIN_IN_N1,INPUT_PULLUP);
  pinMode(PIN_IN_N2,INPUT_PULLUP);
  pinMode(PIN_IN_N3,INPUT_PULLUP);

  pinMode(PIN_ANALOG_POW,INPUT);
  
  Serial.begin(115200);
  delay(50);
  Serial.println('Boot');
}
 
void loop() 
{  
  int val=analogRead(PIN_ANALOG_POW);
  Serial.print("PWR: ");
  Serial.print(val);
  Serial.print(", ");

  val=digitalRead(PIN_IN_N1);
  Serial.print("N1: ");
  Serial.print(val);
  Serial.print(", ");

  if (val==0)
    digitalWrite(PIN_OUT_CMD,HIGH);
   else
    digitalWrite(PIN_OUT_CMD,LOW);

  val=digitalRead(PIN_IN_N2);
  Serial.print("N2: ");
  Serial.print(val);
  Serial.print(", ");

  val=digitalRead(PIN_IN_N3);
  Serial.print("N3: ");
  Serial.println(val);

  delay(500);
}
