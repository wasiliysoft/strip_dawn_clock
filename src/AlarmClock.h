#ifndef ALARM_CLOCK_H
#define ALARM_CLOCK_H
#ifndef CONFIG_H
#include "Config.h"
#endif
#ifndef BEEPER_H
#include "Beeper.h"
#endif
#include <NTPClient.h>
#include <WiFiUdp.h>
extern Config config;
extern Beeper beeper;

class AlarmClock {
private:
  WiFiUDP ntpUDP;
  NTPClient timeClient;

  int alarmStep = -1; // -1: ожидание, 0: рассвет, 1: будильник
  unsigned long dawnStartEpoch = 0;

public:
  AlarmClock() : timeClient(ntpUDP, "192.168.4.1", 18000, 60000) {}

  void begin() {
    timeClient.begin();
    // Пытаемся синхронизироваться, но не висим, если сети нет
    for (int i = 0; i < 10 && !timeClient.isTimeSet(); ++i) {
      timeClient.forceUpdate();
      delay(200);
    }
    calculateDawnTime();
    Serial.printf("NTP time set: %s\n",
                  timeClient.isTimeSet() ? "yes" : "no (will retry in loop)");
  }

  void update() {
    timeClient.update();
    tick();
  }

  void setAlarm(uint8_t h, uint8_t m) {
    config.alarm.hours = h;
    config.alarm.minutes = m;
    calculateDawnTime();
    config.commit();
  }

  void setDawnDuration(uint8_t minutes) {
    config.dawnDuration = minutes;
    calculateDawnTime();
    config.commit();
  }

  void setEnabled(bool enabled) {
    config.alarm.enabled = enabled;
    config.commit();
    if (config.alarm.enabled) {
      beeper.startTwoBeep();
    } else {
      beeper.startOneBeep();
    }
  }

  void toggleAlarm() {
    config.alarm.enabled = !config.alarm.enabled;
    config.commit();
    if (config.alarm.enabled) {
      beeper.startTwoBeep();
    } else {
      beeper.startOneLongBeep();
    }
  }

  // Сбросить триггеры будильник, рассвет
  void cancelAlarm() {
    alarmStep = -1;
    beeper.stop();
    beeper.startOneBeep();
  }

  String getTimeString() { return timeClient.getFormattedTime(); }

  String getAlarmString() {
    char buffer[6];                      // "HH:MM" + null terminator = 6 bytes
    uint8_t h = config.alarm.hours % 24; // Гарантируем диапазон 0-23
    uint8_t m = config.alarm.minutes % 60; // Гарантируем диапазон 0-59
    snprintf(buffer, sizeof(buffer), "%02d:%02d", h, m);
    return String(buffer);
  }

  String getDawnString() {
    char buffer[6];                       // "HH:MM" + null terminator = 6 bytes
    uint8_t h = config.dawn.hours % 24;   // Гарантируем диапазон 0-23
    uint8_t m = config.dawn.minutes % 60; // Гарантируем диапазон 0-59
    snprintf(buffer, sizeof(buffer), "%02d:%02d", h, m);
    return String(buffer);
  }

  bool isAlarmTriggered() const { return alarmStep >= 1; }
  bool isDawnTiggered() const { return alarmStep >= 0; }
  bool isAlarmEnabled() const { return config.alarm.enabled; }

  bool isWeekEnd() const {
    return timeClient.getDay() % 6 == 0; // 0 - Sunday, 6 - Saturday
  }

  // Возвращает прогресс рассвета от 0 до 1024
  unsigned long dawnProgress() {
    unsigned long now = timeClient.getEpochTime();
    unsigned long duration = config.dawnDuration * 60;
    unsigned long elapsed = now > dawnStartEpoch ? now - dawnStartEpoch : 0;
    if (elapsed >= duration)
      return 1024;
    return (elapsed * 1024 / duration);
  }

private:
  void tick() {
    static int lastMinutes = -1; // -1 = «ещё не инициализировано»

    if (!timeClient.isTimeSet())
      return;
    int currentMinute = timeClient.getMinutes();
    int currentHour = timeClient.getHours();

    if (lastMinutes == -1) {
      lastMinutes = currentMinute; // первый тик — только запоминаем
      return;
    }

    if (currentMinute != lastMinutes) {
      lastMinutes = currentMinute;
      if (config.alarm.enabled && !(config.isMuteWeekend && isWeekEnd())) {
        // Проверка начала рассвета
        if (currentHour == config.dawn.hours &&
            currentMinute == config.dawn.minutes) {
          alarmStep = 0;
          dawnStartEpoch = timeClient.getEpochTime();
          Serial.println("Dawn started");
        }

        // Сработает когда рассвет уже был запущен и не сброшен
        if (isDawnTiggered() && currentHour == config.alarm.hours &&
            currentMinute == config.alarm.minutes) {
          alarmStep = 1;
          Serial.println("Alarm triggered");
        }

        // Авто-стоп будильника через 30 минут
        if (alarmStep > 30) {
          cancelAlarm();
          Serial.println("Alarm auto-canceled");
        }

        // Запуск мелодии будильника
        if (isAlarmTriggered()) {
          alarmStep = alarmStep + 1;
          beeper.startAlarmBeep();
          Serial.println("Alarm!");
        }
      }
    }
  }

  void calculateDawnTime() {
    int total = (int)config.alarm.hours * 60 + (int)config.alarm.minutes -
                (int)config.dawnDuration;
    if (total < 0)
      total += 1440;
    config.dawn.hours = (uint8_t)(total / 60);
    config.dawn.minutes = (uint8_t)(total % 60);
  }
};

#endif