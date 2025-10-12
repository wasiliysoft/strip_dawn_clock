#include "AlarmClock.h"
#include "Beeper.h"
#include "Config.h"
#include "EncoderHandler.h"
#include "LEDStrip.h"
#include "WebServer.h"
#include "WiFiMgr.h"

Beeper beeper;
AlarmClock alarmClock;
LEDStrip ledStrip;
EncoderHandler encoder;
WebUI ui;
Config config;

void setup() {
  Serial.begin(115200);
  Serial.println("\nStarting Smart Alarm Clock...");
  pinMode(STATUS_LED_PIN, OUTPUT);

  config.begin();
  beeper.begin(BUZZER_PIN, BUZZER_VOLUME);
  // Инициализация компонентов
  setupWiFi();
  ui.begin();
  alarmClock.begin();
  ledStrip.begin(config.ledCount, config.ledBrightness);
  encoder.begin();

  digitalWrite(STATUS_LED_PIN, HIGH); // LED off initially
  Serial.println("System ready!");
}

void loop() {
  ui.update();
  alarmClock.update();
  ledStrip.update();
  encoder.update();
  beeper.update();
  digitalWrite(STATUS_LED_PIN, alarmClock.isAlarmEnabled() ? LOW : HIGH);
  // Быстрая обработка
  delay(1);
}