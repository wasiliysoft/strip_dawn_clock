// TODO: настройка длительности будильника через веб-интерфейс
// TODO: уровень сигнала будильника через веб-интерфейс (но через PWM это
// работает плохо)
// TODO: RSSI перевести в плохой, средний, хороший

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "AlarmClock.h"
#include "LEDStrip.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
extern AlarmClock alarmClock;
extern LEDStrip ledStrip;

class WebUI {
private:
  ESP8266WebServer server;
  ESP8266HTTPUpdateServer httpUpdater;

public:
  WebUI() : server(80) {}
  void begin() {
    server.on("/", [this]() { this->handleRoot(); });
    server.on("/status", [this]() { this->handleStatus(); });
    server.on("/setAlarm", [this]() { this->handleSetAlarm(); });
    server.on("/setDawnDuration", [this]() { this->handleSetDawnDuration(); });
    server.on("/toggleAlarm", [this]() { this->handleToggleAlarm(); });
    server.on("/toggleMuteWeekend",
              [this]() { this->handleToggleMuteWeekend(); });
    server.on("/lightOn", [this]() { this->handleLightOn(); });
    server.on("/lightOff", [this]() { this->handleLightOff(); });
    server.on("/beeperTestAlarm", [this]() { this->handleBeeperTestAlarm(); });
    server.on("/beeperStop", [this]() { this->handleStopBeeper(); });
    server.on("/setStrip", [this]() { this->handleSetStrip(); });
    httpUpdater.setup(&server);
    server.begin();

    Serial.println("HTTP server started");
  }

  void update() { server.handleClient(); }

private:
  void handleRoot() {
    String html = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>⏰ Dawn Alarm</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body { font-family: Arial, sans-serif; padding: 15px; background: #f5f5f5; }
        .container { max-width: 500px; margin: 0 auto; }
        .card { background: white; padding: 20px; margin-bottom: 15px; border-radius: 8px; }
        h2, h3 { margin-bottom: 15px; }
        button { 
            background: #4CAF50; color: white; border: none; padding: 10px 15px; 
            border-radius: 4px; cursor: pointer; margin: 5px; min-width: 120px;
        }
        button.secondary { background: #2196F3; }
        input { 
            padding: 10px; border: 1px solid #ddd; border-radius: 4px; 
            width: 100%; margin-bottom: 12px;
        }
        .status { background: #f9f9f9; padding: 15px; border-radius: 4px; margin-bottom: 15px; }
        .on { color: green; font-weight: bold; }
        .off { color: red; font-weight: bold; }
        .form-group { margin-bottom: 15px; }
        .input-row { display: flex; gap: 8px; align-items: center; margin-bottom: 12px; }
        .input-row input { flex: 1; margin: 0; }
        .button-row { display: flex; flex-wrap: wrap; margin-top: 10px; }
        .status-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-bottom: 15px; }
        .status-item { padding: 10px; background: #f0f0f0; border-radius: 4px; font-size: 13px; line-height: 1.6; }
        .status-item div:first-child { margin-bottom: 8px; font-weight: 500; }
        label { display: block; margin-bottom: 5px; font-weight: 500; }
        @media (max-width: 480px) {
            .status-grid { grid-template-columns: 1fr; }
            .button-row { flex-direction: column; }
            button { width: 100%; margin: 3px 0; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="card">
            <h2>⏰ Будильник "Рассвет"</h2>
            <div id="status">Загрузка...</div>
            <div class="button-row">
                <button onclick="toggleAlarm()">Переключить будильник</button>
                <button onclick="toggleMuteWeekend()" class="secondary">Пропуск выходных</button>
            </div>
        </div>

        <div class="card">
            <h3>⏰ Установка будильника</h3>
            <div class="form-group">
                <label>Время будильника</label>
                <div class="input-row">
                    <input type="number" id="alarmHours" placeholder="ЧЧ" min="0" max="23" value="7">
                    <span>:</span>
                    <input type="number" id="alarmMinutes" placeholder="ММ" min="0" max="59" value="0">
                </div>
            </div>
            <div class="form-group">
                <label>Длительность рассвета (минут)</label>
                <input type="number" id="dawnDuration" min="1" max="60" value="10">
            </div>
            <button onclick="setAlarmAndDawn()">Применить настройки</button>
        </div>
        
        <div class="card">
            <h3>💡 Настройки ленты</h3>
            <div class="form-group">
                <label>Количество диодов (1-255)</label>
                <input type="number" id="ledCount" min="1" max="255" value="29">
            </div>
            <div class="form-group">
                <label>Яркость (1-255)</label>
                <input type="number" id="brightness" min="1" max="255" value="128">
            </div>
            <div class="button-row">
                <button onclick="setStrip()">Применить</button>
                <button onclick="lightOn()" class="secondary">Включить</button>
                <button onclick="lightOff()" class="secondary">Выключить</button>
            </div>
        </div>
        
        <div class="card">
            <h3>🔊 Тестирование звука</h3>
            <div class="button-row">
                <button onclick="beeperTestAlarm()">Вкл. сигнал</button>
                <button onclick="beeperStop()">Остановить</button>
            </div>
        </div>
        
        <div class="card">
            <h3>ℹ️ Информация</h3>
            <div class="status-grid">
                <div class="status-item">
                    <div>Версия прошивки</div>
                    <div>1.0.0</div>
                </div>
                <div class="status-item">
                    <div>Состояние</div>
                    <div id="connection-status">Подключение...</div>
                </div>
            </div>
            <button onclick='location.href="/update"'>Обновление прошивки</button>
        </div>
    </div>

    <script>
        document.addEventListener('DOMContentLoaded', initialize);

        function initialize() {
            fetch('/status')
                .then(r => r.json())
                .then(data => {
                    const [h, m] = data.alarm.split(':');
                    document.getElementById('alarmHours').value = h;
                    document.getElementById('alarmMinutes').value = m;
                    document.getElementById('ledCount').value = data.ledCount;
                    document.getElementById('brightness').value = data.ledBrightness;
                    document.getElementById('dawnDuration').value = data.dawnDuration;
                    updateConnectionStatus(true);
                })
                .catch(() => updateConnectionStatus(false));
        }
              
        function updateStatus() {
            fetch('/status')
                .then(r => r.json())
                .then(data => {
                    document.getElementById('status').innerHTML = 
                        `<div class="status-grid">
                            <div class="status-item"><div>🕒 Время</div><div>${data.time}</div></div>
                            <div class="status-item"><div>📶 WiFi</div><div>${data.wifi} (${data.rssi}dBm)</div></div>
                        </div>
                        <div class="status-grid">
                            <div class="status-item"><div>⏰ Будильник</div><div>${data.alarm} <span class="${data.alarmEnabled?'on':'off'}">${data.alarmEnabled?'ВКЛ':'ВЫКЛ'}</span></div></div>
                            <div class="status-item"><div>📅 Пропускать субботу и воскресенье</div><div><span class="${data.isMuteWeekend?'on':'off'}">${data.isMuteWeekend?'ДА':'НЕТ'}</span></div></div>
                        </div>
                        <div class="status-grid">
                            <div class="status-item"><div>🌅 Рассвет</div><div>${data.dawn} (${data.dawnDuration}мин)</div></div>
                        </div>
                        <div class="status-grid">
                            <div class="status-item"><div>💡 Диоды</div><div>${data.ledCount}</div></div>
                            <div class="status-item"><div>💡 Яркость</div><div>${data.ledBrightness}</div></div>
                        </div>`;
                })
                .catch(() => document.getElementById('status').innerHTML = 'Ошибка загрузки статуса');
        }

        function setAlarmAndDawn() {
            const h = document.getElementById('alarmHours').value.padStart(2,'0');
            const m = document.getElementById('alarmMinutes').value.padStart(2,'0');
            const d = document.getElementById('dawnDuration').value;
            
            fetch('/setAlarm?h='+h+'&m='+m)
                .then(() => fetch('/setDawnDuration?duration='+d))
                .then(() => {
                    updateStatus();
                    alert('Настройки сохранены');
                })
                .catch(() => alert('Ошибка сохранения'));
        }

        function setStrip() {
            const c = document.getElementById('ledCount').value; 
            const b = document.getElementById('brightness').value;
            fetch('/setStrip?count='+c+'&brightness='+b)
                .then(() => {
                    updateStatus();
                    alert('Настройки ленты сохранены');
                })
                .catch(() => alert('Ошибка сохранения'));
        }

        function toggleAlarm() {
            fetch('/toggleAlarm').then(updateStatus).catch(() => alert('Ошибка'));
        }

        function toggleMuteWeekend() {
            fetch('/toggleMuteWeekend').then(updateStatus).catch(() => alert('Ошибка'));
        }

        function lightOn() { fetch('/lightOn').catch(() => alert('Ошибка')); }
        function lightOff() { fetch('/lightOff').catch(() => alert('Ошибка')); }
        function beeperTestAlarm() { fetch('/beeperTestAlarm').catch(() => alert('Ошибка')); }
        function beeperStop() { fetch('/beeperStop').catch(() => alert('Ошибка')); }

        function updateConnectionStatus(connected) {
            const el = document.getElementById('connection-status');
            el.textContent = connected ? 'Подключено' : 'Ошибка подключения';
            el.style.color = connected ? 'green' : 'red';
        }

        setInterval(updateStatus, 5000);
        updateStatus();
    </script>
</body>
</html>
)=====";
    server.send(200, "text/html", html);
  }

  void handleStatus() {
    JsonDocument doc;

    doc["time"] = alarmClock.getTimeString();
    doc["alarm"] = alarmClock.getAlarmString();
    doc["dawnDuration"] = config.dawnDuration;
    doc["isMuteWeekend"] = config.isMuteWeekend;
    doc["dawn"] = alarmClock.getDawnString();
    doc["alarmEnabled"] = alarmClock.isAlarmEnabled();
    doc["ledCount"] = config.ledCount;
    doc["ledBrightness"] = config.ledBrightness;
    doc["wifi"] = WiFi.SSID();
    doc["rssi"] = WiFi.RSSI();

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  }

  void handleSetAlarm() {
    if (server.hasArg("h") && server.hasArg("m")) {
      int hours = server.arg("h").toInt();
      int minutes = server.arg("m").toInt();

      // Валидация ввода
      if (hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59) {
        alarmClock.setAlarm(hours, minutes);
        server.send(200, "text/plain", "Alarm set successfully");
      } else {
        server.send(400, "text/plain", "Invalid time format");
      }
    } else {
      server.send(400, "text/plain", "Missing parameters");
    }
  }

  void handleSetDawnDuration() {
    if (server.hasArg("duration")) {
      int duration = server.arg("duration").toInt();
      // Валидация ввода
      if (duration >= 1 && duration <= 60) {
        alarmClock.setDawnDuration(duration);
        server.send(200, "text/plain", "Dawn duration set successfully");
      } else {
        server.send(400, "text/plain", "Invalid duration format");
      }
    } else {
      server.send(400, "text/plain", "Missing parameters");
    }
  }

  void handleToggleAlarm() {
    alarmClock.toggleAlarm();
    server.send(200, "text/plain", "OK");
  }

  void handleToggleMuteWeekend() {
    config.isMuteWeekend = !config.isMuteWeekend;
    config.commit();
    server.send(200, "text/plain", "OK");
  }
  void handleLightOn() {
    ledStrip.turnOn();
    server.send(200, "text/plain", "Light ON");
  }

  void handleLightOff() {
    ledStrip.startFadeOut();
    server.send(200, "text/plain", "Light OFF");
  }
  void handleSetStrip() {
    if (server.hasArg("count") && server.hasArg("brightness")) {
      int count = server.arg("count").toInt();
      int brightness = server.arg("brightness").toInt();
      config.ledCount = count;
      config.ledBrightness = brightness;
      config.commit();
      server.send(200, "text/plain", "LED strip settings updated");
      delay(1000);
      ESP.restart(); // Перезагрузка для применения новых настроек
    } else {
      server.send(400, "text/plain", "Missing parameters");
    }
  }

  void handleStopBeeper() {
    beeper.stop();
    server.send(200, "text/plain", "Beeper stopped");
  }
  void handleBeeperTestAlarm() {
    beeper.startAlarmBeep();
    server.send(200, "text/plain", "Beeper started");
  }
};

#endif