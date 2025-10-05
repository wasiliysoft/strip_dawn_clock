#include <Arduino.h>
#include "Config.h"
#include "WiFiMgr.h"
#include "WebServer.h"
#include "AlarmClock.h"
#include "LEDStrip.h"
#include "EncoderHandler.h"

AlarmClock alarmClock;
LEDStrip ledStrip;
EncoderHandler encoder;

void setup() {
  Serial.begin(115200);
  Serial.println("\nStarting Smart Alarm Clock...");

  // Инициализация компонентов
  setupWiFi();
  setupWebServer();
  alarmClock.begin();
  ledStrip.begin();
  encoder.begin();

  Serial.println("System ready!");
}

void loop() {
  handleWebClient();
  alarmClock.update();
  ledStrip.update();
  encoder.update();
  
  // Быстрая обработка
  yield();
  delay(1);
}