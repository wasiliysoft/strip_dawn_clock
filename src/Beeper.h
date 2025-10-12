#ifndef BEEPER_H
#define BEEPER_H
#ifndef CONFIG_H
#include "Config.h"
#endif
class Beeper {
public:
  Beeper()
      : _pin(255), _maxVolume(255), _seqCount(0), _seqPlayed(0), _seqToneMs(0),
        _seqPauseMs(0), _seqVolume(0), _seqOn(false), _seqLastMs(0),
        _rampActive(false), _rampStartMs(0), _rampDurationMs(0),
        _rampMaxVolume(0) {}

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
            analogWrite(_pin, _rampActive ? currentRampVolume(now) : 0);
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

    // Обработка нарастающего сигнала (ramp)
    if (_rampActive) {
      uint16_t vol = currentRampVolume(now);
      analogWrite(_pin, vol);
      // ramp остаётся активным, даже после достижения max — тон держится на max
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

  // Одиночный гудок (неблокирующий)
  void startSingle(uint16_t toneMs = 150, uint16_t volume = BUZZER_VOLUME) {
    startPulse(1, toneMs, toneMs + 10, volume);
  }

  // Запустить нарастающий сигнал (неблокирующий).
  // durationMs - время для достижения maxVolume; после достижения signal
  // держится на maxVolume. maxVolume - целевая амплитуда.
  void startRamp(int durationMs = 20000, uint16_t maxVolume = BUZZER_VOLUME) {
    _rampActive = true;
    _rampStartMs = millis();
    _rampDurationMs = max(1, durationMs);
    _rampMaxVolume = constrain(maxVolume, 0, _maxVolume);
  }

  // Остановить все звуки
  void stop() {
    _seqCount = 0;
    _seqPlayed = 0;
    _seqOn = false;
    _rampActive = false;
    analogWrite(_pin, 0);
  }

  // Возвращает true, если в данный момент воспроизводится какой-либо сигнал
  bool isActive() const { return (_seqCount > 0) || _rampActive; }

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

  // ramp
  bool _rampActive;
  uint32_t _rampStartMs;
  uint32_t _rampDurationMs;
  uint16_t _rampMaxVolume;

  // helper: текущая вычисленная громкость для ramp по времени
  uint16_t currentRampVolume(unsigned long now) const {
    if (!_rampActive)
      return 0;
    unsigned long elapsed = now - _rampStartMs;
    if (elapsed >= _rampDurationMs)
      return _rampMaxVolume;
    // линейное нарастание
    uint32_t vol = (uint32_t)_rampMaxVolume * (uint32_t)elapsed /
                   (uint32_t)_rampDurationMs;
    return (uint16_t)vol;
  }
};

#endif // BEEPER_H