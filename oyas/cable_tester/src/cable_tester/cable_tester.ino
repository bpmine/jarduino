#include <EEPROM.h>

const byte OUT_PINS[4] = {2, 3, 4, 5};
const byte IN_PINS[4]  = {6, 7, 8, 9};
const byte LED_PINS[4] = {A0, A1, A2, A3};

const byte BUTTON_PIN = 10;   // bouton vers GND
const byte EEPROM_ADDR_MODE = 0;

byte currentMode = 1;

// ---------- OUTILS ----------

bool buttonPressed()
{
  return digitalRead(BUTTON_PIN) == LOW;
}

void allLedsOff()
{
  for (byte i = 0; i < 4; i++)
    digitalWrite(LED_PINS[i], LOW);
}

void allOutputsHiZ()
{
  for (byte i = 0; i < 4; i++)
    pinMode(OUT_PINS[i], INPUT);
}

void chenillardBoot()
{
  allLedsOff();

  for (byte i = 0; i < 4; i++)
  {
    digitalWrite(LED_PINS[i], HIGH);
    delay(500);
    digitalWrite(LED_PINS[i], LOW);
  }
}

void blinkAllLeds2s()
{
  unsigned long start = millis();
  bool state = false;

  while (millis() - start < 2000)
  {
    state = !state;

    for (byte i = 0; i < 4; i++)
      digitalWrite(LED_PINS[i], state);

    delay(250);
  }

  allLedsOff();
}

void showMode(byte mode)
{
  allLedsOff();

  if (mode >= 1 && mode <= 4)
    digitalWrite(LED_PINS[mode - 1], HIGH);
}

void blinkCurrentMode2s()
{
  unsigned long start = millis();
  bool state = false;

  while (millis() - start < 2000)
  {
    state = !state;

    allLedsOff();
    digitalWrite(LED_PINS[currentMode - 1], state);

    delay(250);
  }

  allLedsOff();
}

// ---------- MODE RÉGLAGE ----------
// Appui court : mode suivant
// 3 secondes sans appui : sauvegarde et sortie

void setupMode()
{
  blinkAllLeds2s();

  byte selectedMode = currentMode;
  showMode(selectedMode);

  unsigned long lastAction = millis();
  bool oldButton = buttonPressed();

  while (millis() - lastAction < 3000)
  {
    bool b = buttonPressed();

    if (oldButton == HIGH && b == LOW)
    {
      selectedMode++;

      if (selectedMode > 4)
        selectedMode = 1;

      showMode(selectedMode);
      lastAction = millis();

      delay(200); // anti-rebond simple
    }

    oldButton = b;
  }

  currentMode = selectedMode;
  EEPROM.update(EEPROM_ADDR_MODE, currentMode);

  allLedsOff();
}

// ---------- MODE 1 : TEST CÂBLE ----------

void driveOneLow(byte n)
{
  allOutputsHiZ();

  pinMode(OUT_PINS[n], OUTPUT);
  digitalWrite(OUT_PINS[n], LOW);
}

void loop_mode1()
{
  static unsigned long lastBlink = 0;
  static bool blinkState = false;

  bool ok[4];
  bool errorBlink[4];

  for (byte i = 0; i < 4; i++)
  {
    ok[i] = false;
    errorBlink[i] = false;
  }

  for (byte out = 0; out < 4; out++)
  {
    driveOneLow(out);
    delay(5);

    byte nbLow = 0;
    byte lowIndex[4];

    for (byte in = 0; in < 4; in++)
    {
      if (digitalRead(IN_PINS[in]) == LOW)
      {
        lowIndex[nbLow++] = in;
      }
    }

    if (nbLow == 1)
    {
      if (lowIndex[0] == out)
      {
        ok[out] = true;
      }
      else
      {
        errorBlink[out] = true;
        errorBlink[lowIndex[0]] = true;
      }
    }
    else if (nbLow >= 2)
    {
      errorBlink[out] = true;

      for (byte k = 0; k < nbLow; k++)
        errorBlink[lowIndex[k]] = true;
    }
  }

  allOutputsHiZ();

  if (millis() - lastBlink >= 500)
  {
    lastBlink = millis();
    blinkState = !blinkState;
  }

  for (byte i = 0; i < 4; i++)
  {
    if (errorBlink[i])
      digitalWrite(LED_PINS[i], blinkState);
    else if (ok[i])
      digitalWrite(LED_PINS[i], HIGH);
    else
      digitalWrite(LED_PINS[i], LOW);
  }

  delay(50);
}

// ---------- MODE 2 À REMPLIR ----------

void loop_mode2()
{
  // À compléter plus tard
  allLedsOff();
  delay(100);
}

void loop_mode3()
{
  // À compléter plus tard
  allLedsOff();
  delay(100);
}

void loop_mode4()
{
  // À compléter plus tard
  allLedsOff();
  delay(100);
}

// ---------- SETUP / LOOP ----------

void setup()
{
  Serial.begin(9600);

  for (byte i = 0; i < 4; i++)
  {
    pinMode(OUT_PINS[i], INPUT);
    pinMode(IN_PINS[i], INPUT_PULLUP);
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  currentMode = EEPROM.read(EEPROM_ADDR_MODE);

  if (currentMode < 1 || currentMode > 4)
    currentMode = 1;

  chenillardBoot();

  if (buttonPressed())
  {
    setupMode();
  }

  blinkCurrentMode2s();
}

void loop()
{
  switch (currentMode)
  {
    case 1:
      loop_mode1();
      break;

    case 2:
      loop_mode2();
      break;

    case 3:
      loop_mode3();
      break;

    case 4:
      loop_mode4();
      break;

    default:
      currentMode = 1;
      break;
  }
}
