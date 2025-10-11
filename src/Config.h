#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>
#include <EEPROM.h>
// Пины
#define ENCODER_CLK 14 // D5 s1
#define ENCODER_DT 12  // D6 s2
#define ENCODER_SW 13  // D7 key
#define STRIP_PIN 2    // D4
#define BUZZER_PIN 5   // D1
// #define STATUS_LED D2

// Настройки ленты
#define LED_COUNT 29
#define LED_BRIGHTNESS 150

// Настройки будильника
#define DAWN_DURATION 20 // минут
#define BUZZER_VOLUME 5

// WiFi
#define AP_NAME "SmartAlarmClock"

struct Config {
  struct {
    uint8_t hours = 7;
    uint8_t minutes = 0;
    bool enabled = true;
  } alarm;

  struct {
    uint8_t hours = 0;
    uint8_t minutes = 0;
  } dawn;

} config;

void loadSettings() {
  config.alarm.hours = EEPROM.read(0);
  config.alarm.minutes = EEPROM.read(1);
  config.alarm.enabled = EEPROM.read(2);

  // Валидация загруженных значений
  if (config.alarm.hours > 23)
    config.alarm.hours = 7;
  if (config.alarm.minutes > 59)
    config.alarm.minutes = 0;
}

void saveSettings() {
  EEPROM.write(0, config.alarm.hours);
  EEPROM.write(1, config.alarm.minutes);
  EEPROM.write(2, config.alarm.enabled);
  EEPROM.commit();
}

void beep(uint8_t count) {
  for (int i = 0; i < count; i++) {
    analogWrite(BUZZER_PIN, BUZZER_VOLUME);
    delay(50);
    digitalWrite(BUZZER_PIN, LOW);
    if (i < count - 1)
      delay(100);
  }
}
#endif