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
      ledStrip.increaseEnableLeds(enc.fast() ? 4 : 1);
      return;
    }

    if (enc.left()) {
      ledStrip.decreaseEnableLeds(enc.fast() ? 4 : 1);
      return;
    }

    if (enc.rightH()) {
      ledStrip.nextMode();
      return;
    }

    if (enc.leftH()) {
      ledStrip.prevMode();
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