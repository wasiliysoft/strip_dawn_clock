#include "AlarmClock.h"
#include "Config.h"
#include "EncoderHandler.h"
#include "LEDStrip.h"
#include "WebServer.h"
#include "WiFiMgr.h"

AlarmClock alarmClock;
LEDStrip ledStrip;
EncoderHandler encoder;
WebUI ui;

void setup() {
  Serial.begin(115200);
  Serial.println("\nStarting Smart Alarm Clock...");

  EEPROM.begin(512);
  loadSettings();

  // Инициализация компонентов
  setupWiFi();
  ui.begin();
  alarmClock.begin();
  ledStrip.begin();
  encoder.begin();

  Serial.println("System ready!");
}

void loop() {
  ui.update();
  alarmClock.update();
  ledStrip.update();
  encoder.update();

  // Быстрая обработка
  yield();
  delay(1);
}