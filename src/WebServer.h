#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "AlarmClock.h"
#include "LEDStrip.h"
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
extern AlarmClock alarmClock;
extern LEDStrip ledStrip;

class WebUI {
private:
  ESP8266WebServer server;

public:
  WebUI() : server(80) {}
  void begin() {
    server.on("/", [this]() { this->handleRoot(); });
    server.on("/status", [this]() { this->handleStatus(); });
    server.on("/setAlarm", [this]() { this->handleSetAlarm(); });
    server.on("/toggleAlarm", [this]() { this->handleToggleAlarm(); });
    server.on("/toggleMuteWeekend",
              [this]() { this->handleToggleMuteWeekend(); });
    server.on("/lightOn", [this]() { this->handleLightOn(); });
    server.on("/lightOff", [this]() { this->handleLightOff(); });
    server.on("/beeperTestAlarm", [this]() { this->handleBeeperTestAlarm(); });
    server.on("/beeperStop", [this]() { this->handleStopBeeper(); });
    server.on("/setStrip", [this]() { this->handleSetStrip(); });
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
    <title>Smart Alarm</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .container { max-width: 500px; margin: 0 auto; }
        .card { background: white; padding: 20px; margin: 10px 0; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
        button { background: #4CAF50; color: white; border: none; padding: 10px 15px; margin: 5px; border-radius: 5px; cursor: pointer; }
        input { padding: 8px; margin: 5px; width: 80px; }
        .status { color: #666; font-size: 16px; line-height: 1.5; }
        .on { color: green; font-weight: bold; }
        .off { color: red; font-weight: bold; }
    </style>
</head>
<body>
    <div class="container">
        <div class="card">
            <h2>Будильник "Рассвет"</h2>
            <div class="status" id="status">Loading...</div><br>
            <button onclick="toggleAlarm()">Переключить будильник</button><br>
            <button onclick="toggleMuteWeekend()">Переключить пропуск по выходным</button>
        </div>

        <div class="card">
            <h3>Установка будильника</h3>
            <input type="number" id="alarmHours" placeholder="HH" min="0" max="23" value="7">
            <input type="number" id="alarmMinutes" placeholder="MM" min="0" max="59" value="0">
            <button onclick="setAlarm()">Применить</button>
        </div>
        <div class="card">
            <h3>Настройки ленты</h3>
            <label for="ledCount">Количество диодов (1-255):</label>
            <input type="number" id="ledCount" min="1" max="255" value="29"><br>
            <label for="brighness">Яркость (1-255):</label>
            <input type="number" id="brighness"  min="1" max="255" value="128"><br>
            <button onclick="setStrip()">Применить</button>
        </div>
        <div class="card">
            <h3>Управление</h3>
            <button onclick="beeperTestAlarm()">Тест громкости будильника</button>
            <button onclick="beeperStop()">Прекратить мелодию</button><br>
            <button onclick="lightOn()">Вкл. ленту</button>
            <button onclick="lightOff()">Откл. ленту</button>
        </div>
    </div>

    <script>
        document.addEventListener('DOMContentLoaded', (event) => {
            initialize();
        });

        function initialize() {
            fetch('/status')
              .then(r => r.json())
              .then(data => {
                  document.getElementById('alarmHours').value = data.alarm.split(':')[0];
                  document.getElementById('alarmMinutes').value = data.alarm.split(':')[1];
                  document.getElementById('ledCount').value = data.ledCount;
                  document.getElementById('brighness').value = data.ledBrightness;
              })
              .catch(err => {
                  console.error('Error loading initial settings', err);
              });
        }
              
        function updateStatus() {
            fetch('/status')
                .then(r => r.json())
                .then(data => {
                    const statusElement = document.getElementById('status');
                    statusElement.innerHTML = 
                        `🕒 Время: <b>${data.time}</b><br>` +
                        `⏰ Будильник: <b>${data.alarm}</b> <span class="${data.alarmEnabled ? 'on' : 'off'}">${data.alarmEnabled ? 'ВКЛ.' : 'ОТКЛ.'}</span><br>` +
                        `⏰ Пропускать субботу и воскресенье: <span class="${data.isMuteWeekend ? 'on' : 'off'}">${data.isMuteWeekend ? 'ДА' : 'НЕТ'}</span><br>` +
                        `🌅 Рассвет: <b>${data.dawn}</b><br>` +
                        `📶 WiFi: <b>${data.wifi}</b><br>` +
                        `📶 RSSI: <b>${data.rssi} dBm</b><br>` +
                        `💡 Количество диодов: <b>${data.ledCount}</b><br>`+
                        `💡 Яркость: <b>${data.ledBrightness}</b>`;
                })
                .catch(err => {
                    document.getElementById('status').innerHTML = 'Error loading status';
                });
        }

        function setAlarm() {
            const hours = document.getElementById('alarmHours').value.padStart(2, '0');
            const minutes = document.getElementById('alarmMinutes').value.padStart(2, '0');
            fetch('/setAlarm?h=' + hours + '&m=' + minutes)
                .then(updateStatus)
                .catch(err => alert('Error setting alarm'));
        }

        function setStrip() {
            const ledCount = document.getElementById('ledCount').value; 
            const brightness = document.getElementById('brighness').value;
            fetch(`/setStrip?count=${ledCount}&brightness=${brightness}`)
                .then(() => alert('Настройки ленты сохранены успешно'))
                .catch(err => alert('Ошибка сохранения настроек ленты'));
        }

        function toggleAlarm() {
            fetch('/toggleAlarm').then(updateStatus);
        }

        function toggleMuteWeekend() {
            fetch('/toggleMuteWeekend').then(updateStatus);
        }

        function lightOn() { 
            fetch('/lightOn').then(() => console.log('Light turned on')); 
        }
        
        function lightOff() { 
            fetch('/lightOff').then(() => console.log('Light turned off')); 
        }

        function beeperTestAlarm() { 
            fetch('/beeperTestAlarm').then(() => console.log('Beeper ramp started')); 
        }
        function beeperStop() { 
            fetch('/beeperStop').then(() => console.log('Beeper stopped')); 
        }
        // Auto-update status every 5 seconds
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