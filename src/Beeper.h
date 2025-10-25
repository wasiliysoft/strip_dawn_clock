#ifndef BEEPER_H
#define BEEPER_H

class Beeper {
public:
  Beeper() = default;

  // Инициализация пина и громкости
  // volume: 0-255
  void begin(uint8_t pin = 0, uint16_t volume = 0) {
    _pin = pin;
    _volume = volume;
    pinMode(_pin, OUTPUT);
  }

  // Вызывать в loop() — обрабатывает текущие звуковые события
  void update() {
    unsigned long now = millis();
    if (_pattern.active && _pattern.count > 0) {
      if (now - _pattern.lastMs >= _pattern.arr[_pattern.pos]) {
        // advance position
        _pattern.pos++;
        if (_pattern.pos >= _pattern.count) {
          // finished one repetition
          _pattern.repeatsLeft--;
          if (_pattern.repeatsLeft <= 0) {
            // pattern finished
            _pattern = {};
            analogWrite(_pin, 0);
            return;
          }
          _pattern.pos = 0;
        }
        // even -> tone on, odd -> pause
        if ((_pattern.pos % 2) == 0) {
          analogWrite(_pin, _volume);
        } else {
          analogWrite(_pin, 0);
        }
        _pattern.lastMs = now;
      }
      return;
    }
  }

  void startOneBeep() {
    const uint16_t pattern[] = {50, 0};
    startPattern(pattern, 2);
  }

  void startTwoBeep() {
    const uint16_t pattern[] = {50, 100, 50, 0};
    startPattern(pattern, 4);
  }

  void startOneLongBeep() {
    const uint16_t pattern[] = {200, 0};
    startPattern(pattern, 2);
  }

  void startAlarmBeep() {
    const uint16_t pattern[] = {50, 100, 50, 100, 50, 100, 50, 500};
    startPattern(pattern, 8, 10);
  }

  // Остановить все звуки
  void stop() {
    _pattern = {};
    analogWrite(_pin, 0);
  }

private:
  uint8_t _pin;
  uint8_t _volume;

  static const int BEEPER_PATTERN_MAX = 32;
  struct Pattern {
    uint16_t arr[BEEPER_PATTERN_MAX];
    int count = 0;
    int pos = 0;
    int repeatsLeft = 0;
    bool active = false;
    uint32_t lastMs = 0;
  } _pattern;

  void startPattern(const uint16_t *pattern, int count = 0, int repeats = 1) {
    stop();
    if (pattern == nullptr || count <= 0 || repeats <= 0)
      return;
    int c = min(count, (int)BEEPER_PATTERN_MAX);
    for (int i = 0; i < c; ++i)
      _pattern.arr[i] = pattern[i];
    _pattern.count = c;
    _pattern.pos = 0;
    _pattern.repeatsLeft = repeats;

    _pattern.active = true;
    _pattern.lastMs = millis();
    // start with tone or pause based on pos parity (pos==0 -> tone)
    if ((_pattern.pos % 2) == 0)
      analogWrite(_pin, _volume);
    else
      analogWrite(_pin, 0);
  }
};

#endif // BEEPER_H