#ifndef ALARM_CLOCK_H
#define ALARM_CLOCK_H
#ifndef CONFIG_H
#include "Config.h"
#endif
#include <NTPClient.h>
#include <WiFiUdp.h>
extern Config config;

class AlarmClock {
private:
  WiFiUDP ntpUDP;
  NTPClient timeClient;

  bool dawnTriggered = false;
  bool alarmTriggered = false;

public:
  AlarmClock() : timeClient(ntpUDP, "pool.ntp.org", 18000, 60000) {}

  void begin() {

    timeClient.begin();
    calculateDawnTime();
  }

  void update() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 60000) {
      lastUpdate = millis();
      tick();
    }
  }

  void tick() {
    timeClient.update();
    uint8_t currentHour = timeClient.getHours();
    uint8_t currentMinute = timeClient.getMinutes();

    if (config.alarm.enabled) {
      // Проверка начала рассвета
      if (currentHour == config.dawn.hours &&
          currentMinute == config.dawn.minutes) {
        dawnTriggered = true;
        Serial.println("Dawn started");
      }
      // Проверка будильника
      if (currentHour == config.alarm.hours &&
          currentMinute == config.alarm.minutes) {
        alarmTriggered = true;
        Serial.println("Alarm!");
      }
    }
  }

  void setAlarm(uint8_t h, uint8_t m) {
    config.alarm.hours = h;
    config.alarm.minutes = m;
    calculateDawnTime();
    config.commit();
  }

  void setEnabled(bool enabled) {
    config.alarm.enabled = enabled;
    config.commit();
  }

  void toggleAlarm() {
    config.alarm.enabled = !config.alarm.enabled;
    config.commit();
  }

  // Сбросить триггеры будильник, рассвет
  void cancelAlam() {
    alarmTriggered = false;
    dawnTriggered = false;
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

  bool isAlarmTriggered() const { return alarmTriggered; }
  bool isDawnTiggered() const { return dawnTriggered; }
  bool isAlarmEnabled() const { return config.alarm.enabled; }

private:
  void calculateDawnTime() {
    if (config.alarm.minutes >= DAWN_DURATION) {
      config.dawn.hours = config.alarm.hours;
      config.dawn.minutes = config.alarm.minutes - DAWN_DURATION;
    } else {
      config.dawn.hours =
          (config.alarm.hours == 0) ? 23 : config.alarm.hours - 1;
      config.dawn.minutes = 60 - (DAWN_DURATION - config.alarm.minutes);
    }
  }
};

#endif