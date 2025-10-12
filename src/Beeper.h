#ifndef BEEPER_H
#define BEEPER_H
#ifndef CONFIG_H
#include "Config.h"
#endif
class Beeper {
public:
  Beeper()
      : _pin(255), _maxVolume(255), _seqCount(0), _seqPlayed(0), _seqToneMs(0),
        _seqPauseMs(0), _seqVolume(0), _seqOn(false), _seqLastMs(0) {}

  // Инициализация пина
  void begin(uint8_t pin = BUZZER_PIN, uint16_t maxVolume = 255) {
    _pin = pin;
    _maxVolume = constrain(maxVolume, 0, 1023);
    pinMode(_pin, OUTPUT);
    analogWrite(_pin, 0);
  }

  // Вызывать в loop() — обрабатывает текущие звуковые события
  void handle() {
    unsigned long now = millis();

    // Обработка последовательности гудков (приоритет выше, чем ramp)
    if (_seqCount > 0) {
      if (_seqOn) {
        if (now - _seqLastMs >= _seqToneMs) {
          // выключаем тон
          analogWrite(_pin, 0);
          _seqOn = false;
          _seqLastMs = now;
          _seqPlayed++;
        }
      } else {
        if (now - _seqLastMs >= _seqPauseMs) {
          if (_seqPlayed >= _seqCount) {
            // закончились гудки
            _seqCount = 0;
            _seqPlayed = 0;
            _seqOn = false;
            _seqLastMs = 0;
            // оставляем pin в выключенном состоянии; if ramp active, it will
            // set level below
            analogWrite(_pin, 0);
          } else {
            // включаем следующий тон
            analogWrite(_pin, constrain(_seqVolume, 0, _maxVolume));
            _seqOn = true;
            _seqLastMs = now;
          }
        }
      }
      // если последовательность активна — не трогаем ramp дальше
      // (последовательность управляет pin)
      return;
    }
  }

  // Запустить серию коротких гудков (неблокирующе)
  // count - количество гудков, toneMs - длительность одного гудка, pauseMs -
  // пауза между ними, volume - амплитуда
  void startPulse(int count = 3, uint16_t toneMs = 50, uint16_t pauseMs = 150,
                  uint16_t volume = BUZZER_VOLUME) {
    _seqCount = max(1, count);
    _seqPlayed = 0;
    _seqToneMs = toneMs;
    _seqPauseMs = pauseMs;
    _seqVolume = constrain(volume, 0, _maxVolume);
    _seqOn = true;
    _seqLastMs = millis();
    analogWrite(_pin, _seqVolume);
  }

  void startPattern(uint16_t *pattern, int count = 3,int repeats = 1,
                    uint16_t volume = BUZZER_VOLUME) {}
  // Остановить все звуки
  void stop() {
    _seqCount = 0;
    _seqPlayed = 0;
    _seqOn = false;
    analogWrite(_pin, 0);
  }

private:
  uint8_t _pin;
  uint16_t _maxVolume;

  // sequence (pulses)
  uint8_t _seqCount;
  uint8_t _seqPlayed;
  uint32_t _seqToneMs;
  uint32_t _seqPauseMs;
  uint16_t _seqVolume;
  bool _seqOn;
  uint32_t _seqLastMs;
};

#endif // BEEPER_H