// FIXME: клик по энкодеру после остановки рассвета приводит к резкому
// выключению ленты, хотя ожидается плавное затухание, связанно с тем что в
// режиме рассвета яркость диодов задается через СHSV, а не через
// FastLED.setBrightness().

#ifndef LED_STRIP_H
#define LED_STRIP_H
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

class LEDStrip {
private:
  CRGB *leds = nullptr;
  uint8_t _ledCount = 0;
  uint8_t _brightness = 0;

  uint8_t enabledCount = 0;
  uint8_t currentMode = 0;

  bool isFading = false;

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
    static unsigned long lastMicros = 0;
    if (isFading && micros() - lastMicros >= 50) {
      lastMicros = micros();
      uint8_t nb = constrain(FastLED.getBrightness() - 1, 0, 255);
      if (nb == 0) {
        // завершение затухания: выключаем ленту и сбрасываем флаг
        setEnabledCount(0);
        currentMode = 0;
        isFading = false;
        FastLED.setBrightness(_brightness);
        render();
        yield();
        return;
      }
      FastLED.setBrightness(nb);
    }

    render();
  }

  void nextMode() { setMode(min(currentMode + 1, 2)); }
  void prevMode() { setMode(max(currentMode - 1, 0)); }

  // Увеличивает количество включённых светодиодов на amount
  // Если все светодиоды включены, увеличивает яркость на amount * 3
  void increaseEnableLeds(int amount = 1) {
    setEnabledCount(enabledCount + amount);
    if (enabledCount == _ledCount) {
      uint8_t nb = constrain(FastLED.getBrightness() + amount * 3, 0, 255);
      FastLED.setBrightness(nb);
    }
  }
  // Уменьшает количество включённых светодиодов на amount
  // Если все светодиоды включены, уменьшает яркость на amount * 3 (минимум до
  // _brightness)
  void decreaseEnableLeds(int amount = 1) {
    if (FastLED.getBrightness() > _brightness) {
      uint8_t nb =
          constrain(FastLED.getBrightness() - amount * 3, _brightness, 255);
      FastLED.setBrightness(nb);
    } else {
      setEnabledCount(enabledCount > amount ? enabledCount - amount : 0);
    }
  }

  void turnOn() { setEnabledCount(_ledCount); }

  // Устанавливает прогресс рассвета от 0 до 1024
  // 0 - всё выключено, 1024 - все включено на полной яркости
  // Логика рассвета: каждлый диод плавно повышает яркость, затем включается
  // следующий и т.д.
  void dawn(unsigned long progress) {
    setMode(99); // специальный режим рассвета
    progress = min(progress, 1024UL);
    // Рассчитываем виртуальное "количество включенных светодиодов"
    // с дробной частью
    float fractionalLeds = (_ledCount * progress) / 1024.0f;
    int fullLeds = (int)fractionalLeds; // Целая часть - полностью включенные
    float fractionalPart = fractionalLeds - fullLeds; // Дробная часть (0.0-1.0)

    for (int i = 0; i < _ledCount; i++) {
      if (i < fullLeds) {
        // Полностью включенные
        leds[i] = CHSV(16, 255, _brightness);
      } else if (i == fullLeds) {
        // Плавно включающийся светодиод
        uint8_t brightness = (uint8_t)(_brightness * fractionalPart);
        leds[i] = CHSV(16, 255, brightness);
      } else {
        // Выключенные
        leds[i] = CHSV(0, 0, 0);
      }
    }
    FastLED.show();
  }

  void startFadeOut() { isFading = true; }

private:
  void setMode(uint8_t newMode) { currentMode = newMode; }

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
    case 99:
      return; // в режиме рассвета не рендерим
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