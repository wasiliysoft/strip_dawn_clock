#ifndef CONFIG_H
#define CONFIG_H

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

#endif