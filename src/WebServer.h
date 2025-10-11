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
    server.on("/lightOn", [this]() { this->handleLightOn(); });
    server.on("/lightOff", [this]() { this->handleLightOff(); });
    server.on("/lightIncreaseLeds",[this]() { this->handleLightIncreaseLeds(); });
    server.on("/lightDencreaseLeds",[this]() { this->handleLightDecreaseLeds(); });
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
            <h2>Smart Alarm Clock</h2>
            <div class="status" id="status">Loading...</div>
        </div>

        <div class="card">
            <h3>Set Alarm</h3>
            <input type="number" id="alarmHours" placeholder="HH" min="0" max="23" value="7">
            <input type="number" id="alarmMinutes" placeholder="MM" min="0" max="59" value="0">
            <button onclick="setAlarm()">Set Alarm</button>
        </div>

        <div class="card">
            <h3>Controls</h3>
            <button onclick="toggleAlarm()">Toggle Alarm</button>
            <button onclick="lightOn()">Light On</button>
            <button onclick="lightOff()">Light Off</button>
            <button onclick="lightIncreaseLeds()">Increase Leds</button>
            <button onclick="lightDencreaseLeds()">Dencrease Leds</button>
        </div>
    </div>

    <script>
        function updateStatus() {
            fetch('/status')
                .then(r => r.json())
                .then(data => {
                    const statusElement = document.getElementById('status');
                    statusElement.innerHTML = 
                        `🕒 Time: <b>${data.time}</b><br>` +
                        `⏰ Alarm: <b>${data.alarm}</b> <span class="${data.alarmEnabled ? 'on' : 'off'}">${data.alarmEnabled ? 'ON' : 'OFF'}</span><br>` +
                        `🌅 Dawn: <b>${data.dawn}</b><br>` +
                        `📶 WiFi: ${data.wifi}`;
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

        function toggleAlarm() {
            fetch('/toggleAlarm').then(updateStatus);
        }

        function lightOn() { 
            fetch('/lightOn').then(() => console.log('Light turned on')); 
        }
        
        function lightOff() { 
            fetch('/lightOff').then(() => console.log('Light turned off')); 
        }

        function lightIncreaseLeds() { 
            fetch('/lightIncreaseLeds').then(() => console.log('Light increase leds')); 
        }
        function lightDencreaseLeds() { 
            fetch('/lightDencreaseLeds').then(() => console.log('Light dencrease leds')); 
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
    doc["dawn"] = alarmClock.getDawnString();
    doc["alarmEnabled"] = alarmClock.isAlarmEnabled();
    doc["wifi"] = WiFi.SSID();

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

  void handleLightOn() {
    ledStrip.turnOn();
    server.send(200, "text/plain", "Light ON");
  }

  void handleLightOff() {
    ledStrip.turnOff();
    server.send(200, "text/plain", "Light OFF");
  }
  
  void handleLightIncreaseLeds() {
    ledStrip.increaseBrightness();
    server.send(200, "text/plain", "ok");
  }

  void handleLightDecreaseLeds() {
    ledStrip.decreaseBrightness();
    server.send(200, "text/plain", "ok");
  }
};

#endif