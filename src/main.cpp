#include "EEPROM.h"
#include "FastLED.h"
#include "GyverEncoder.h"
#include "GyverTimer.h"
#include "RTClib.h"
#include "Wire.h"
#include <CyberLib.h>
#include <SerialCommand.h>
// *************************** НАСТРОЙКИ ***************************//
#define DAWN_TIME 10 // длительность рассвета в мин. до наступления будильника

#define BUZZ 0        // пищать пищалкой (1 вкл, 0 выкл)
#define BUZZ_BRIGHT 5 // громкость звонка
#define LED_BRIGHT 100 // яркость светодиода индикатора (0 - 255)

#define ENCODER_TYPE 1 // тип энкодера (0 или 1).
// *********************** ПАРАМЕТРЫ ЛЕНТЫ ***********************

#define STRIP_BRIGHTNESS 150        // яркость ленты
#define STRIP_COLOR CRGB::LightCyan // Цвет
#define STRIP_LEDS 67      // количество светодиодов
#define STRIP_TYPE WS2812B // тип ленты
#define COLOR_ORDER GRB    // последовательность цветов

// ************ ПИНЫ ************
#define CLKe 8 // энкодер
#define DTe 9  // энкодер
#define SWe 10 // энкодер

#define STRIP_PIN 4 // Пин ленты
#define BUZZ_PIN 5  // Пин пищалки
#define LED_PIN 6   // Пин светодиода индикатора

// ***************** ОБЪЕКТЫ И ПЕРЕМЕННЫЕ *****************

CRGB leds[STRIP_LEDS];
Encoder enc(CLKe, DTe, SWe, ENCODER_TYPE);
RTC_DS3231 rtc;
GTimer_ms timerLED(30);
GTimer_ms timerStrip(33);
#if BUZZ == 1
GTimer_ms timerBUZZ(300);
#endif

GTimer_ms timerMinute(60000);
GTimer_ms timerDAWN(DAWN_TIME * 60000 / STRIP_LEDS);

SerialCommand SCmd;

struct {
  int8_t h, m;
} clock;

struct {
  int8_t h, m;
  boolean enabled = true;
  // boolean buzzer = false;
} alarm;

struct {
  int8_t h, m;
} dawn;

boolean isLostPower = false; // была потеря питания на часах

int endabled_led_count = 0;
enum Modes { STANDBY, DAWN, ALARM };
Modes mode = STANDBY;

int stripMode = 0; // 0-белый 1-радуга 2-оранжевый
void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZ_PIN, OUTPUT);

  Serial.begin(9600);
  delay(1000);
  FastLED.addLeds<STRIP_TYPE, STRIP_PIN, COLOR_ORDER>(leds, STRIP_LEDS)
      .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(STRIP_BRIGHTNESS);
  SCmd.addCommand("a", settingAlarm);
  SCmd.addCommand("t", settingTime);

  alarm.h = EEPROM.read(0);
  alarm.m = EEPROM.read(1);
  alarm.enabled = EEPROM.read(2);
  alarm.h = constrain(alarm.h, 0, 23);
  alarm.m = constrain(alarm.m, 0, 59);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1) {
    };
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    isLostPower = true;
    alarm.enabled = false;
    analogWrite(BUZZ_PIN, BUZZ_BRIGHT);
    delay(500);
    digitalWrite(BUZZ_PIN, LOW);
  }
  syncRTCTime();

  calculateDawn();
  printStatus();
  twoBeep();
}
void loop() {
  timeTick();
  dawnTick();
  indicatorTick();
  encoderTick();
  ledStripTick();
  SCmd.readSerial();

#if BUZZ == 1
  buzzTick();
#endif
}
void ledStripTick() {
  if (timerStrip.isReady()) {
    endabled_led_count = constrain(endabled_led_count, 0, STRIP_LEDS);
    if (stripMode == 0) { // Default color
      for (int dot = 0; dot < STRIP_LEDS; dot++) {
        if (dot < endabled_led_count) {
          leds[STRIP_LEDS - 1 - dot] = STRIP_COLOR;
        } else {
          leds[STRIP_LEDS - 1 - dot] = CRGB::Black;
        }
      }
    } else if (stripMode == 1) { // RainbowColors
      static uint8_t startIndex = 0;
      startIndex = startIndex + 1; /* motion speed */
      FillLEDsFromPaletteColors(startIndex);
    } else if (stripMode == 2) { // DarkOrange
      for (int dot = 0; dot < STRIP_LEDS; dot++) {
        if (dot < endabled_led_count) {
          leds[STRIP_LEDS - 1 - dot] = CRGB::DarkOrange;
        } else {
          leds[STRIP_LEDS - 1 - dot] = CRGB::Black;
        }
      }
    }
    FastLED.show();
  }
}
void syncRTCTime() {
  clock.h = rtc.now().hour();
  clock.m = rtc.now().minute();
}

void settingAlarm() {
  char *arg1 = SCmd.next();
  char *arg2 = SCmd.next();
  if (arg1 != NULL && arg2 != NULL) {
    alarm.h = atoi(arg1);
    alarm.m = atoi(arg2);
    EEPROM.update(0, alarm.h); // сохраняем будильник
    EEPROM.update(1, alarm.m); // сохраняем будильник
    calculateDawn(); // пересчет времени рассвета
    Serial.println("Saved settings");
    printStatus();
    twoBeep();
  } else {
    Serial.println("print format 'a hrs mins'");
  }
}
void settingTime() {
  char *arg1 = SCmd.next();
  char *arg2 = SCmd.next();
  if (arg1 != NULL && arg2 != NULL) {
    clock.h = atoi(arg1);
    clock.m = atoi(arg2);
    rtc.adjust(DateTime(2014, 1, 21, clock.h, clock.m,
                        0)); // установка нового времени в RTC модуль
    isLostPower = false;
    Serial.println("Saved settings");
    printStatus();
    twoBeep();
  } else {
    Serial.println("print format 't hrs mins'");
  }
}

#if BUZZ == 1
void buzzTick() {
  if (timerBUZZ.isReady()) {
    buzzFlag = !buzzFlag;
    if (alarm_enabled && mode == 2) {
      if (buzzFlag) {
        analogWrite(BUZZ_PIN, BUZZ_BRIGHT);
      } else {
        digitalWrite(BUZZ_PIN, LOW);
      }
    } else {
      digitalWrite(BUZZ_PIN, LOW);
    }
  }
}
#endif
void timeTick() {
  if (timerMinute.isReady()) {
    syncRTCTime();
    if (alarm.enabled) {
      if (clock.h == dawn.h && clock.m == dawn.m) {
        mode = DAWN;
      }
      if (clock.h == alarm.h && clock.m == alarm.m && mode == DAWN) {
        mode = ALARM;
      }
    } else {
      mode = STANDBY;
    }
    // Автоматическое включение радуги, новый год же!
    /*
    if (hrs == 17 && mins == 0) {
      endabled_led_count = STRIP_LEDS;
      stripMode = 1;
    }
    */
  }
}
void dawnTick() {
  if (alarm.enabled && mode != STANDBY) {
    if (timerDAWN.isReady()) {
      endabled_led_count++;
    }
  }
}

void indicatorTick() {

  if (timerLED.isReady()) {
    if (isLostPower) {
      static int8_t s_counterToSwichIndicatorState = 0;
      static bool s_blinkFlag = false;
      s_counterToSwichIndicatorState++;

      if (s_counterToSwichIndicatorState > 5) {
        s_counterToSwichIndicatorState = 0;
        s_blinkFlag = !s_blinkFlag;
      }
      if (s_blinkFlag) {
        digitalWrite(LED_PIN, HIGH);
      } else {
        digitalWrite(LED_PIN, LOW);
      }
      return;
    }

    if (alarm.enabled) {
      static int8_t indicatorBright = 0;
      static int8_t step = 1;

      indicatorBright = indicatorBright + step;
      if (indicatorBright > LED_BRIGHT && step > 0) {
        indicatorBright = LED_BRIGHT;
        step = -3;
      }
      if (indicatorBright < 0 && step < 0) {
        indicatorBright = 0;
        step = 1;
      }
      analogWrite(LED_PIN, indicatorBright);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  }
}

void printStatus() {
  Serial.println("");
  Serial.print("time ");
  Serial.print(clock.h);
  Serial.print(":");
  Serial.print(clock.m);
  Serial.print(" alarm ");
  Serial.print(alarm.h);
  Serial.print(":");
  Serial.print(alarm.m);
  Serial.print(" dawn start ");
  Serial.print(dawn.h);
  Serial.print(":");
  Serial.print(dawn.m);
  Serial.print(" alarm mode ");
  if (alarm.enabled) {
    Serial.print("on");
  } else {
    Serial.print("off");
  }
  Serial.println("");
}

void calculateDawn() {
  // расчёт времени рассвета
  if (alarm.m >= DAWN_TIME) {
    // если минут во времени будильника больше или равно продолжительности
    // рассвета
    dawn.h = alarm.h; // час рассвета равен часу будильника
    dawn.m = alarm.m - DAWN_TIME;
  } else {
    // если минут во времени будильника меньше продолжительности рассвета
    dawn.h = alarm.h - 1; // значит рассвет будет часом раньше
    if (dawn.h < 0)
      dawn.h = 23; // защита от совсем поехавших
    dawn.m = 60 - (DAWN_TIME - alarm.m); // находим минуту рассвета в новом часе
  }
}

void encoderTick() {
  enc.tick(); // работаем с энкодером
  if (enc.isClick()) {
    if (mode != STANDBY) {
      mode = STANDBY;
      oneBeep();
      return;
    }
    if (endabled_led_count == 0) {
      endabled_led_count = 1;
    } else {
      downStrip();
      stripMode = 0;
    }
  }
  if (enc.isRight()) {
    endabled_led_count = endabled_led_count + 4;
    endabled_led_count = constrain(endabled_led_count, 0, STRIP_LEDS);
    Serial.println(endabled_led_count);
  }
  if (enc.isLeft()) {
    endabled_led_count = endabled_led_count - 2;
    endabled_led_count = constrain(endabled_led_count, 0, STRIP_LEDS);
  }
  if (enc.isLeftH()) {
    stripMode--;
    stripMode = constrain(stripMode, 0, 2);
  }
  if (enc.isRightH()) {
    stripMode++;
    stripMode = constrain(stripMode, 0, 2);
  }

  // *********** УДЕРЖАНИЕ ЭНКОДЕРА **********
  if (enc.isHolded()) {
    alarm.enabled = !alarm.enabled;
    EEPROM.update(2, alarm.enabled); // сохраняем состояние будильника
    twoBeep();
  }
}
void FillLEDsFromPaletteColors(uint8_t colorIndex) {
  for (int dot = 0; dot < STRIP_LEDS; dot++) {
    if (dot < endabled_led_count) {
      leds[STRIP_LEDS - 1 - dot] =
          ColorFromPalette(RainbowColors_p, colorIndex, 255, LINEARBLEND);
      colorIndex -= 2;
    } else {
      leds[STRIP_LEDS - 1 - dot] = CRGB::Black;
    }
  }
}

void downStrip() {
  endabled_led_count = constrain(endabled_led_count, 0, STRIP_LEDS);
  for (int dot = endabled_led_count; dot >= 0; dot--) {
    leds[STRIP_LEDS - 1 - dot] = CRGB::Black;
    FastLED.show();
    delay(30);
  }
  endabled_led_count = 0;
}

void oneBeep() {
  analogWrite(BUZZ_PIN, BUZZ_BRIGHT);
  delay(50);
  digitalWrite(BUZZ_PIN, 0);
}

void twoBeep() {
  analogWrite(BUZZ_PIN, BUZZ_BRIGHT);
  delay(50);
  digitalWrite(BUZZ_PIN, 0);
  delay(100);
  analogWrite(BUZZ_PIN, BUZZ_BRIGHT);
  delay(50);
  digitalWrite(BUZZ_PIN, 0);
}