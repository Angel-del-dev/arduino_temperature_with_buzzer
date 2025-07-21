#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

int thermistorPin = 0;
#define buzzerPin 6

const int seriesResistor = 10000; // resistencia en serie de 10kΩ
const float nominalResistance = 10000; // resistencia del termistor a 25ºC
const float nominalTemperature = 25.0; // en grados Celsius
const float bCoefficient = 3950; // coeficiente B típico del NTC Elegoo
const int adcMax = 1023;

void setup() {
  Serial.begin(9600);
  // LCD's columns and rows
  lcd.begin(16, 2);
  pinMode(buzzerPin, OUTPUT);
}

float toFarenheit(float celsius) {
 return (celsius * 1.8) + 32.0; 
}

float steinhertToCelsius() {
  int adcValue = analogRead(thermistorPin);

  float resistance = (float)seriesResistor * ((float)adcMax / adcValue - 1);
  // Fórmula de Steinhart-Hart simplificada
  float steinhart;
  steinhart = resistance / nominalResistance;     // (R/Ro)
  steinhart = log(steinhart);                     // ln(R/Ro)
  steinhart /= bCoefficient;                      // 1/B * ln(R/Ro)
  steinhart += 1.0 / (nominalTemperature + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                    // Invertir
  return steinhart - 273.15;                      // Restamos para convertir de Kelvin a C
}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("Temperatura ");

  lcd.setCursor(0, 1);

  float celsius = steinhertToCelsius();
  if(celsius > 28)
  {
    tone(buzzerPin, 494, 1000);
  }
  lcd.print(celsius);
  lcd.print("C ");
  lcd.print(toFarenheit(celsius));
  lcd.print("F");
  delay(500);
  lcd.clear();
}