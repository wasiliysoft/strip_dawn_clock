#ifndef ENCODER_HANDLER_H
#define ENCODER_HANDLER_H

#include "AlarmClock.h"
#include "Config.h"
#include "LEDStrip.h"
#include <EncButton.h>

extern AlarmClock alarmClock;
extern LEDStrip ledStrip;

class EncoderHandler {
private:
  EncButton enc;

public:
  EncoderHandler() : enc(ENCODER_CLK, ENCODER_DT, ENCODER_SW) {}

  void begin() {
    // Инициализация энкодера
  }

  void update() {
    enc.tick();

    if (enc.click()) {
      handleClick();
      return;
    }
    if (enc.right()) {
      ledStrip.increaseBrightness();
      return;
    }
    if (enc.left()) {
      ledStrip.decreaseBrightness();
      return;
    }
    if (enc.rightH()) {
      ledStrip.setMode(ledStrip.getMode() + 1);
      return;
    }
    if (enc.leftH()) {
      // Безопасное уменьшение (чтобы не уйти в отрицательные)
      uint8_t currentMode = ledStrip.getMode();
      ledStrip.setMode(currentMode == 0 ? 2 : currentMode - 1);
      return;
    }
    if (enc.hold()) {
      alarmClock.toggleAlarm();
      beep(2);
      return;
    }
  }

private:
  void handleClick() {
    if (alarmClock.isAlarmTriggered() || alarmClock.isDawnTiggered()) {
      alarmClock.cancelAlam();
      beep(1);
      return;
    } else {
      ledStrip.turnOff();
    }
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
};

#endif