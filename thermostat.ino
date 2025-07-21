#include <LiquidCrystal.h>
#include <IRremote.h>
#define thermistorPin 0
#define buzzerPin 3
#define IR_RECEIVE_PIN 2
#define backlightPin 5

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

const int seriesResistor = 10000;
const float nominalResistance = 10000;
const float nominalTemperature = 25.0;
const float bCoefficient = 3950;
const int adcMax = 1023;

bool screenOn = false;
bool soundOn = true;
bool buzzerOn = false;
String menu[] = {
  "degree", "Temperatura", "t_sound", "Pitido",
  "c_degree", "Temp. min"
};
String menuSelected = "";
int phantomMenuSelected = 0;
int menuLength = sizeof(menu) / sizeof(menu[0]);
int minTmpSound = 28;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);

  pinMode(buzzerPin, OUTPUT);
  pinMode(backlightPin, OUTPUT);
  pinMode(13, OUTPUT);

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
}

float toFarenheit(float celsius) {
  return (celsius * 1.8) + 32.0;
}

float steinhertToCelsius() {
  int adcValue = analogRead(thermistorPin);

  float resistance = (float)seriesResistor * ((float)adcMax / adcValue - 1);
  float steinhart = resistance / nominalResistance;
  steinhart = log(steinhart);
  steinhart /= bCoefficient;
  steinhart += 1.0 / (nominalTemperature + 273.15);
  steinhart = 1.0 / steinhart;
  return steinhart - 273.15;
}

void ActivateRemoteController() {
  bool wasBuzzerOn = buzzerOn;

  if (wasBuzzerOn) {
    digitalWrite(buzzerPin, LOW);
    buzzerOn = false;
    delay(5); // corta pausa para evitar conflicto
  }

  if (IrReceiver.decode()) {
    digitalWrite(13, HIGH);
    switch (IrReceiver.decodedIRData.decodedRawData) {
      case 0xBA45FF00: // On & Off
        screenOn = !screenOn;
        if (!screenOn) {
          digitalWrite(buzzerPin, LOW);
          buzzerOn = false;
          menuSelected = "";
        }
        break;
      case 0xF807FF00: // Down
        executeDown();
        break;
      case 0xF609FF00: // Up
        executeUp();
        break;
      case 0xBF40FF00: // Enter
        lcd.clear();
        executeEnter();
        break;
      case 0xB847FF00: // Back
        lcd.clear();
        menuSelected = "";
        digitalWrite(buzzerPin, LOW);
        buzzerOn = false;
        break;
    }
    IrReceiver.resume();
  }

  if (wasBuzzerOn && soundOn && menuSelected == "degree") {
    digitalWrite(buzzerPin, HIGH);
    buzzerOn = true;
  }
}

void controllLcd() {
  digitalWrite(backlightPin, screenOn ? HIGH : LOW);
}

void DrawMenu() {
  lcd.setCursor(0, 0);
  lcd.print("* ");
  lcd.print(menu[phantomMenuSelected + 1]);
  if ((phantomMenuSelected + 3) > menuLength) return;
  lcd.setCursor(0, 1);
  lcd.print(menu[phantomMenuSelected + 3]);
}

void loop() {
  controllLcd();
  ActivateRemoteController();
  if (menuSelected == "") DrawMenu();
  else ShowSelectedScreen();
  delay(50);
}

// Events
void executeDown() {
  if (menuSelected == "") {
    lcd.clear();
    if (phantomMenuSelected > 0) phantomMenuSelected -= 2;
    else phantomMenuSelected = menuLength - 2;
  } else if (menuSelected == "c_degree") {
    minTmpSound--;
  }
}

void executeUp() {
  if (menuSelected == "") {
    lcd.clear();
    if (phantomMenuSelected >= (menuLength - 2)) phantomMenuSelected = 0;
    else phantomMenuSelected += 2;
  } else if (menuSelected == "c_degree") {
    minTmpSound++;
  }
}

void executeEnter() {
  if (menuSelected == "") menuSelected = menu[phantomMenuSelected];
  else if (menuSelected == "t_sound") soundOn = !soundOn;
}

// Screens
void ShowSelectedScreen() {
  if (menuSelected == "degree") showThermostatScreen();
  else if (menuSelected == "t_sound") showSoundScreen();
  else if (menuSelected == "c_degree") showMinTempScreen();
}

void showThermostatScreen() {
  lcd.setCursor(0, 0);
  lcd.print("Temperatura ");

  float celsius = steinhertToCelsius();
  if (celsius > minTmpSound) {
    if (soundOn && !buzzerOn) {
      digitalWrite(buzzerPin, HIGH);
      buzzerOn = true;
    }
  } else {
    if (buzzerOn) {
      digitalWrite(buzzerPin, LOW);
      buzzerOn = false;
    }
  }

  lcd.setCursor(0, 1);
  lcd.print(celsius);
  lcd.print("C ");
  lcd.print(toFarenheit(celsius));
  lcd.print("F");
}

void showSoundScreen() {
  lcd.setCursor(0, 0);
  lcd.print("Sonido");
  lcd.setCursor(0, 1);
  lcd.print(soundOn ? "Activo" : "Inactivo");
}

void showMinTempScreen() {
  lcd.setCursor(0, 0);
  lcd.print("Temp. Sonido");
  lcd.setCursor(0, 1);
  lcd.print(minTmpSound);
  lcd.print(" C");
}
