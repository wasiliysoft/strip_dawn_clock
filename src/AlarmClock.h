#ifndef ALARM_CLOCK_H
#define ALARM_CLOCK_H

#include "Config.h"
#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

class AlarmClock {
private:
  WiFiUDP ntpUDP;
  NTPClient timeClient;
  
  struct {
    uint8_t hours = 7;
    uint8_t minutes = 0;
    bool enabled = true;
  } alarm;

  struct {
    uint8_t hours = 0;
    uint8_t minutes = 0;
  } dawn;

  bool dawnTriggered = false;
  bool alarmTriggered = false;

public:
  AlarmClock() : timeClient(ntpUDP, "pool.ntp.org", 18000, 60000) {}

  void begin() {
    EEPROM.begin(512);
    loadSettings();
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
    if (timeClient.update()) {
      uint8_t currentHour = timeClient.getHours();
      uint8_t currentMinute = timeClient.getMinutes();

      if (alarm.enabled) {
        // Проверка начала рассвета
        if (currentHour == dawn.hours && currentMinute == dawn.minutes) {
          dawnTriggered = true;
          Serial.println("Dawn started");
        }
        // Проверка будильника
        if (currentHour == alarm.hours && currentMinute == alarm.minutes) {
          alarmTriggered = true;
          Serial.println("Alarm!");
        }
      }
    }
  }

  void setAlarm(uint8_t h, uint8_t m) {
    alarm.hours = h;
    alarm.minutes = m;
    calculateDawnTime();
    saveSettings();
  }

  void setEnabled(bool enabled) {
    alarm.enabled = enabled;
    saveSettings();
  }

  void toggleAlarm() {
    alarm.enabled = !alarm.enabled;
    saveSettings();
  }

  // Сбросить триггеры будильник, рассвет
  void cancelAlam() {
    alarmTriggered = false;
    dawnTriggered = false;
  }

  String getTimeString() { return timeClient.getFormattedTime(); }

  String getAlarmString() {
    char buffer[6];                 // "HH:MM" + null terminator = 6 bytes
    uint8_t h = alarm.hours % 24;   // Гарантируем диапазон 0-23
    uint8_t m = alarm.minutes % 60; // Гарантируем диапазон 0-59
    snprintf(buffer, sizeof(buffer), "%02d:%02d", h, m);
    return String(buffer);
  }

  String getDawnString() {
    char buffer[6];                // "HH:MM" + null terminator = 6 bytes
    uint8_t h = dawn.hours % 24;   // Гарантируем диапазон 0-23
    uint8_t m = dawn.minutes % 60; // Гарантируем диапазон 0-59
    snprintf(buffer, sizeof(buffer), "%02d:%02d", h, m);
    return String(buffer);
  }

  bool isAlarmTriggered() const { return alarmTriggered; }
  bool isDawnTiggered() const { return dawnTriggered; }
  bool isAlarmEnabled() const { return alarm.enabled; }

private:
  void calculateDawnTime() {
    if (alarm.minutes >= DAWN_DURATION) {
      dawn.hours = alarm.hours;
      dawn.minutes = alarm.minutes - DAWN_DURATION;
    } else {
      dawn.hours = (alarm.hours == 0) ? 23 : alarm.hours - 1;
      dawn.minutes = 60 - (DAWN_DURATION - alarm.minutes);
    }
  }

  void loadSettings() {
    alarm.hours = EEPROM.read(0);
    alarm.minutes = EEPROM.read(1);
    alarm.enabled = EEPROM.read(2);

    // Валидация загруженных значений
    if (alarm.hours > 23)
      alarm.hours = 7;
    if (alarm.minutes > 59)
      alarm.minutes = 0;
  }

  void saveSettings() {
    EEPROM.write(0, alarm.hours);
    EEPROM.write(1, alarm.minutes);
    EEPROM.write(2, alarm.enabled);
    EEPROM.commit();
  }
};

#endif