#ifndef LED_STRIP_H
#define LED_STRIP_H
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#ifndef CONFIG_H
#include "Config.h"
#endif
extern Config config;

class LEDStrip {
private:
  CRGB *leds = nullptr;
  uint8_t enabledCount = 0;
  uint8_t currentMode = 0;

  // Поля для неблокирующего затухания
  bool isFading = false;
  unsigned long fadeStart = 0;
  uint32_t fadeDuration = 1000; // милисекунд

  uint8_t _ledCount = 0;
  uint8_t _brightness = 0;

public:
  void begin(uint8_t ledCount, uint8_t brightness) {
    _ledCount = ledCount;
    _brightness = brightness;
    leds = new CRGB[ledCount];
    FastLED.addLeds<WS2812B, STRIP_PIN, GRB>(leds, _ledCount);
    FastLED.setBrightness(_brightness);
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
        FastLED.setBrightness(_brightness);
        render();
        yield();
        return;
      }
      // компактный расчёт текущей яркости (линеарно от LED_BRIGHTNESS до 0)
      FastLED.setBrightness((uint8_t)((uint32_t)_brightness *
                                      (fadeDuration - elapsed) / fadeDuration));
    }

    render();
  }

  void nextMode() { setMode(currentMode + 1); }
  void prevMode() { setMode(currentMode > 0 ? currentMode - 1 : 0); }

  void increaseEnableLeds(int amount = 1) {
    setEnabledCount(enabledCount + amount);
  }

  void decreaseEnableLeds(int amount = 1) {
    setEnabledCount(enabledCount > amount ? enabledCount - amount : 0);
  }

  void turnOn() { setEnabledCount(_ledCount); }

  // Неблокирующий метод для плавного затухания ленты
  void startFadeOut(uint32_t durationMs = 1000) {
    if (!isFading && enabledCount > 0) {
      isFading = true;
      fadeStart = millis();
      fadeDuration = durationMs;
    }
  }

private:
  void setMode(uint8_t newMode) { currentMode = constrain(newMode, 0, 2); }

  void setEnabledCount(uint8_t count) {
    enabledCount = constrain(count, 0, _ledCount);
  }

  void render() {
    FastLED.clear();
    switch (currentMode) {
    case 0:
      renderOrange();
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

  void renderOrange() {
    for (int i = 0; i < enabledCount && i < _ledCount; i++) {
      leds[i] = CRGB::OrangeRed;
    }
  }

  void renderRainbow(uint8_t speed = 1) {
    static unsigned long lastRainbowUpdate = millis();
    static uint8_t hueOffset = 0;
    uint8_t _speed = constrain(speed, 1, 10);

    for (int i = 0; i < enabledCount && i < _ledCount; i++) {
      leds[i] = CHSV(hueOffset + (i * 5), 255, 255);
    }

    if (millis() - lastRainbowUpdate > (100 / _speed)) {
      lastRainbowUpdate = millis();
      hueOffset += 2;
    }
  }

  void renderCoolWhite() {
    for (int i = 0; i < enabledCount && i < _ledCount; i++) {
      leds[i] = CRGB(200, 200, 255);
    }
  }
};

#endif