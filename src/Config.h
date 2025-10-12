#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>
#include <EEPROM.h>
// Пины
#define BUZZER_VOLUME 5  // 0..1023
#define BUZZER_PIN 5     // D1
#define ENCODER_CLK 14   // D5 s1
#define ENCODER_DT 12    // D6 s2
#define ENCODER_SW 13    // D7 key
#define STRIP_PIN 4      // D2
#define STATUS_LED_PIN 2 // D4

// Настройки ленты
#define LED_COUNT 29
#define LED_BRIGHTNESS 150

// Настройки будильника
#define DAWN_DURATION 20 // минут

// WiFi
#define AP_NAME "SmartAlarmClock"

// Класс для управления настройками
class Config {
public:
  // Инициализация EEPROM и загрузка настроек
  void begin() {
    EEPROM.begin(512);
    load();
  }

  struct {
    uint8_t hours = 7;
    uint8_t minutes = 0;
    bool enabled = true;
  } alarm;

  struct {
    uint8_t hours = 0;
    uint8_t minutes = 0;
  } dawn;

  // Сохраняет настройки в EEPROM
  void commit() {
    EEPROM.write(0, alarm.hours);
    EEPROM.write(1, alarm.minutes);
    EEPROM.write(2, alarm.enabled);
    EEPROM.commit();
  }

private:
  // Загружает настройки из EEPROM и выполняет валидацию
  void load() {
    alarm.hours = EEPROM.read(0);
    alarm.minutes = EEPROM.read(1);
    alarm.enabled = EEPROM.read(2);

    // Валидация загруженных значений
    if (alarm.hours > 23)
      alarm.hours = 7;
    if (alarm.minutes > 59)
      alarm.minutes = 0;
  }
};

#endif