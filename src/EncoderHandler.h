#ifndef ENCODER_HANDLER_H
#define ENCODER_HANDLER_H
#include <EncButton.h>
#ifndef CONFIG_H
#include "Config.h"
#endif
#ifndef LED_STRIP_H
#include "LEDStrip.h"
#endif
#ifndef ALARM_CLOCK_H
#include "AlarmClock.h"
#endif

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
      if (alarmClock.isAlarmTriggered() || alarmClock.isDawnTiggered()) {
        alarmClock.cancelAlarm();
      } else {
        ledStrip.startFadeOut();
      }
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
      return;
    }
  }

private:
};

#endif