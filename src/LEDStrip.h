#ifndef LED_STRIP_H
#define LED_STRIP_H
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include "Config.h"
#include <FastLED.h>

class LEDStrip {
private:
  CRGB leds[LED_COUNT];
  uint8_t enabledCount = 0;
  uint8_t currentMode = 0;

  uint8_t hueOffset = 0;

  // Поля для неблокирующего затухания
  bool isFading = false;
  unsigned long fadeStart = 0;
  uint32_t fadeDuration = 1000; // милисекунд

public:
  void begin() {
    FastLED.addLeds<WS2812B, STRIP_PIN, GRB>(leds, LED_COUNT);
    FastLED.setBrightness(LED_BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
  }

  void update() {
    // Если идёт затухание — вычисляем и устанавливаем текущую яркость
    if (isFading) {
      unsigned long elapsed = millis() - fadeStart;
      if (elapsed >= fadeDuration) {
        // завершение затухания: выключаем ленту и сбрасываем флаг
        setEnabledCount(0);
        currentMode = 0;
        isFading = false;
        FastLED.setBrightness(LED_BRIGHTNESS);
        render();
        yield();
        return;
      }
      // компактный расчёт текущей яркости (линеарно от LED_BRIGHTNESS до 0)
      FastLED.setBrightness((uint8_t)((uint32_t)LED_BRIGHTNESS * (fadeDuration - elapsed) / fadeDuration));
    }

    render();

    // Авто-обновление для анимаций
    if (currentMode == 1) {
      static unsigned long lastRainbowUpdate = 0;
      if (millis() - lastRainbowUpdate > 50) {
        lastRainbowUpdate = millis();
        renderRainbow();
        FastLED.show();
      }
    }
    yield();
  }

  void setEnabledCount(uint8_t count) {
    if (count <= LED_COUNT)
      enabledCount = count;
    else
      enabledCount = LED_COUNT;
  }

  void setMode(uint8_t newMode) {
    if (newMode < 3) { // Только 0, 1, 2
      currentMode = newMode;
    }
  }

  void increaseBrightness(int amount = 4) {
    setEnabledCount(enabledCount + amount);
  }

  void decreaseBrightness(int amount = 4) {
    if (amount <= enabledCount) {
      setEnabledCount(enabledCount - amount);
    } else {
      setEnabledCount(0);
    }
  }

  void turnOff() {
    setEnabledCount(0);
    currentMode = 0;
  }

  void turnOn() { setEnabledCount(LED_COUNT); }

  // Методы доступа
  uint8_t getEnabledCount() const { return enabledCount; }
  uint8_t getMode() const { return currentMode; }

  // Неблокирующий метод для плавного затухания ленты
  void startFadeOut(uint32_t durationMs = 1000) {
    if (!isFading && enabledCount > 0) {
      isFading = true;
      fadeStart = millis();
      fadeDuration = durationMs;
    }
  }

private:
  void render() {
    FastLED.clear();
    yield();
    switch (currentMode) {
    case 0:
      renderWarmWhite();
      break;
    case 1:
      renderRainbow();
      break;
    case 2:
      renderCoolWhite();
      break;
    }

    FastLED.show();
    yield();
  }

  void renderWarmWhite() {
    for (int i = 0; i < enabledCount && i < LED_COUNT; i++) {
      leds[i] = CRGB(255, 180, 100);
    }
  }

  void renderRainbow() {
    for (int i = 0; i < enabledCount && i < LED_COUNT; i++) {
      leds[i] = CHSV(hueOffset + (i * 5), 255, 255);
    }
    hueOffset += 2;
  }

  void renderCoolWhite() {
    for (int i = 0; i < enabledCount && i < LED_COUNT; i++) {
      leds[i] = CRGB(200, 200, 255);
    }
  }
};

#endif