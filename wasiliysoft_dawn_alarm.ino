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

#define STRIP_BRIGHTNESS 100            // яркость ленты
#define STRIP_COLOR CRGB(200, 255, 255) // Цвет
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

int8_t hrs, mins;
int8_t alm_hrs, alm_mins;
int8_t dwn_hrs, dwn_mins;
boolean alarm_enabled = true;
boolean blinkFlag = false; //техническая переменная
boolean buzzFlag = false;  //техническая переменная
int ledBright = 0;         //техническая переменная
int endabled_led_count = 0;
int mode = 0;      // 0 держурный 1 рассвет 2 будильник
int stripMode = 0; // 0 белый 1 радуга
void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZ_PIN, OUTPUT);

  Serial.begin(9600);
  delay(100);
  FastLED.addLeds<STRIP_TYPE, STRIP_PIN, COLOR_ORDER>(leds, STRIP_LEDS)
      .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(STRIP_BRIGHTNESS);
  SCmd.addCommand("a", settingAlarm);
  SCmd.addCommand("t", settingTime);

  rtc.begin();
  syncRTCTime();
  alm_hrs = EEPROM.read(0);
  alm_mins = EEPROM.read(1);
  alarm_enabled = EEPROM.read(2);
  alm_hrs = constrain(alm_hrs, 0, 23);
  alm_mins = constrain(alm_mins, 0, 59);
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
    if (stripMode == 0) {
      endabled_led_count = constrain(endabled_led_count, 0, STRIP_LEDS);
      for (int dot = 0; dot < STRIP_LEDS; dot++) {
        if (dot < endabled_led_count) {
          leds[dot] = STRIP_COLOR;
        } else {
          leds[dot] = CRGB::Black;
        }
      }
      FastLED.show();
    } else if (stripMode == 1) {
      static uint8_t startIndex = 0;
      startIndex = startIndex + 1; /* motion speed */
      FillLEDsFromPaletteColors(startIndex);
      FastLED.show();
    }
  }
}
void syncRTCTime() {
  DateTime now = rtc.now();
  hrs = now.hour();
  mins = now.minute();
}

void settingAlarm() {
  char *arg1 = SCmd.next();
  char *arg2 = SCmd.next();
  if (arg1 != NULL && arg2 != NULL) {
    alm_hrs = atoi(arg1);
    alm_mins = atoi(arg2);
    EEPROM.update(0, alm_hrs);  // сохраняем будильник
    EEPROM.update(1, alm_mins); // сохраняем будильник
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
    hrs = atoi(arg1);
    mins = atoi(arg2);
    rtc.adjust(DateTime(2014, 1, 21, hrs, mins,
                        0)); // установка нового времени в RTC модуль
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
    if (alarm_enabled) {
      if (hrs == dwn_hrs && mins == dwn_mins) {
        mode = 1; // рассвет
      }
      if (hrs == alm_hrs && mins == alm_mins && mode == 1) {
        mode = 2; // будильник
      }
    } else {
      mode = 0; // ожидание
    }
  }
}
void dawnTick() {
  if (alarm_enabled && mode > 0) {
    if (timerDAWN.isReady()) {
      endabled_led_count++;
    }
  }
}

void indicatorTick() {
  if (timerLED.isReady()) {
    if (alarm_enabled) {
      if (blinkFlag) {
        ledBright++;
      } else {
        ledBright = ledBright - 3;
      }
      if (ledBright >= LED_BRIGHT) {
        ledBright = LED_BRIGHT;
        blinkFlag = false;
      }
      if (ledBright <= 0) {
        ledBright = 0;
        blinkFlag = true;
      }
      analogWrite(LED_PIN, ledBright);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  }
}

void printStatus() {
  Serial.println("");
  Serial.print("time ");
  Serial.print(hrs);
  Serial.print(":");
  Serial.print(mins);
  Serial.print(" alarm ");
  Serial.print(alm_hrs);
  Serial.print(":");
  Serial.print(alm_mins);
  Serial.print(" alarm mode ");
  if (alarm_enabled) {
    Serial.print("on");
  } else {
    Serial.print("off");
  }
  Serial.println("");
}

void calculateDawn() {

  // расчёт времени рассвета
  if (alm_mins > DAWN_TIME) {
    // если минут во времени будильника больше продолжительности рассвета
    dwn_hrs = alm_hrs; // час рассвета равен часу будильника
    dwn_mins = alm_mins - DAWN_TIME;
  } else {
    // если минут во времени будильника меньше продолжительности рассвета
    dwn_hrs = alm_hrs - 1; // значит рассвет будет часом раньше
    if (dwn_hrs < 0)
      dwn_hrs = 23; // защита от совсем поехавших
    dwn_mins =
        60 - (DAWN_TIME - alm_mins); // находим минуту рассвета в новом часе
  }
}

void encoderTick() {
  enc.tick(); // работаем с энкодером
  if (enc.isClick()) {
    if (mode > 0) {
      mode = 0;
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
  }
  if (enc.isLeft()) {
    endabled_led_count = endabled_led_count - 2;
    endabled_led_count = constrain(endabled_led_count, 0, STRIP_LEDS);
  }
  if (enc.isLeftH()) {
    stripMode--;
    stripMode = constrain(stripMode, 0, 1);
  }
  if (enc.isRightH()) {
    stripMode++;
    stripMode = constrain(stripMode, 0, 1);
  }

  // *********** УДЕРЖАНИЕ ЭНКОДЕРА **********
  if (enc.isHolded()) {
    alarm_enabled = !alarm_enabled;
    EEPROM.update(2, alarm_enabled); // сохраняем состояние будильника
    twoBeep();
  }
}
void FillLEDsFromPaletteColors(uint8_t colorIndex) {
  for (int dot = 0; dot < STRIP_LEDS; dot++) {
    if (dot < endabled_led_count) {
      leds[dot] =
          ColorFromPalette(RainbowColors_p, colorIndex, 255, LINEARBLEND);
      colorIndex += 2;
    } else {
      leds[dot] = CRGB::Black;
    }
  }
}

void downStrip() {
  endabled_led_count = constrain(endabled_led_count, 0, STRIP_LEDS);
  for (int dot = endabled_led_count; dot >= 0; dot--) {
    leds[dot] = CRGB::Black;
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