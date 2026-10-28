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
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Dawn Alarm</title>
<style>
  :root { --bg:#f7f7f8; --card:#fff; --line:#e5e7eb; --muted:#6b7280; --accent:#2563eb; --ok:#059669; --err:#dc2626; }
  * { box-sizing:border-box; }
  body { margin:0; font:14px/1.5 -apple-system,Segoe UI,Roboto,Arial,sans-serif; background:var(--bg); color:#111; padding:16px; }
  .wrap { max-width:520px; margin:0 auto; display:grid; gap:12px; }
  .card { background:var(--card); border:1px solid var(--line); border-radius:8px; padding:16px; }
  h1 { font-size:16px; font-weight:600; margin:0 0 12px; }
  h2 { font-size:13px; font-weight:600; margin:0 0 10px; color:var(--muted); text-transform:uppercase; letter-spacing:.04em; }
  label { display:block; font-size:12px; color:var(--muted); margin-bottom:4px; }
  input { width:100%; padding:8px 10px; border:1px solid var(--line); border-radius:6px; font:inherit; background:#fff; }
  input:focus { outline:none; border-color:var(--accent); }
  .row { display:flex; gap:8px; align-items:flex-end; }
  .row > * { flex:1; }
  .row .sep { flex:0 0 auto; padding-bottom:8px; color:var(--muted); }
  .grid2 { display:grid; grid-template-columns:1fr 1fr; gap:8px; }
  button { font:inherit; padding:8px 12px; border:1px solid var(--line); background:#fff; color:#111; border-radius:6px; cursor:pointer; }
  button:hover { background:#f3f4f6; }
  button.primary { background:var(--accent); border-color:var(--accent); color:#fff; }
  button.primary:hover { filter:brightness(.95); }
  .btns { display:flex; flex-wrap:wrap; gap:8px; margin-top:12px; }
  .kv { display:grid; grid-template-columns:1fr 1fr; gap:8px 16px; }
  .kv div { display:flex; justify-content:space-between; gap:8px; padding:6px 0; border-bottom:1px solid var(--line); font-size:13px; }
  .kv div span:first-child { color:var(--muted); }
  .ok { color:var(--ok); font-weight:600; }
  .err { color:var(--err); font-weight:600; }
  #status { margin-bottom:12px; }
  @media (max-width:480px){ .grid2{grid-template-columns:1fr;} .btns button{flex:1;} }
</style>
</head>
<body>
<div class="wrap">

  <div class="card">
    <h1>Будильник «Рассвет»</h1>
    <div id="status" class="kv"><div><span>Загрузка</span><span>…</span></div></div>
    <div class="btns">
      <button class="primary" onclick="toggleAlarm()">Переключить будильник</button>
      <button onclick="toggleMuteWeekend()">Пропуск выходных</button>
    </div>
  </div>

  <div class="card">
    <h2>Будильник</h2>
    <div class="grid2">
      <div>
        <label>Время</label>
        <div class="row">
          <input type="number" id="alarmHours" min="0" max="23" value="7">
          <span class="sep">:</span>
          <input type="number" id="alarmMinutes" min="0" max="59" value="0">
        </div>
      </div>
      <div>
        <label>Рассвет, мин</label>
        <input type="number" id="dawnDuration" min="1" max="60" value="10">
      </div>
    </div>
    <div class="btns"><button class="primary" onclick="setAlarmAndDawn()">Применить</button></div>
  </div>

  <div class="card">
    <h2>Лента</h2>
    <div class="grid2">
      <div><label>Диодов (1–255)</label><input type="number" id="ledCount" min="1" max="255" value="29"></div>
      <div><label>Яркость (1–255)</label><input type="number" id="brightness" min="1" max="255" value="128"></div>
    </div>
    <div class="btns">
      <button class="primary" onclick="setStrip()">Применить</button>
      <button onclick="lightOn()">Включить</button>
      <button onclick="lightOff()">Выключить</button>
    </div>
  </div>

  <div class="card">
    <h2>Звук</h2>
    <div class="btns">
      <button onclick="beeperTestAlarm()">Сигнал</button>
      <button onclick="beeperStop()">Стоп</button>
    </div>
  </div>

  <div class="card"> 
    <h2>Информация</h2>
    <div class="kv">
      <div><span>Прошивка</span><span>1.2.0</span></div>
      <div><span>WiFi RSSI</span><span id="wifi">…</span></div>
      <div><span>Соединение</span><span id="conn">…</span></div>
    </div>
    <div class="btns"><button onclick='location.href="/update"'>Обновление прошивки</button></div>
  </div>

</div>
<script>
const $ = id => document.getElementById(id);
const on = b => `<b class="${b?'ok':'err'}">${b?'ВКЛ':'ВЫКЛ'}</b>`;

function updateStatus() {
  fetch('/status')
    .then(r => r.json())
    .then(d => {
      $('status').innerHTML =
        `<div><span>Время</span><span>${d.time}</span></div>
         <div><span>Будильник</span><span>${d.alarm} ${on(d.alarmEnabled)}</span></div>
         <div><span>Рассвет</span><span>${d.dawn} (${d.dawnDuration} мин)</span></div>
         <div><span>Пропуск выходных</span><span>${on(d.isMuteWeekend)}</span></div>`;
      $('wifi').textContent = `${d.rssi}`;

      const c = $('conn'); c.textContent = 'OK'; c.className = 'ok';
    })
    .catch(() => { const c = $('conn'); c.textContent = 'Нет связи'; c.className = 'err'; });
}

function init() {
  fetch('/status')
    .then(r => r.json())
    .then(d => {
      const [h, m] = d.alarm.split(':');
      $('alarmHours').value = h;
      $('alarmMinutes').value = m;
      $('dawnDuration').value = d.dawnDuration;
      $('ledCount').value = d.ledCount;
      $('brightness').value = d.ledBrightness;
      updateStatus();
    })
    .catch(updateStatus);
}

function setAlarmAndDawn() {
  const h = String($('alarmHours').value).padStart(2,'0');
  const m = String($('alarmMinutes').value).padStart(2,'0');
  const d = $('dawnDuration').value;
  fetch(`/setAlarm?h=${h}&m=${m}`)
    .then(() => fetch(`/setDawnDuration?duration=${d}`))
    .then(updateStatus);
}

function setStrip() {
  const c = $('ledCount').value, b = $('brightness').value;
  fetch(`/setStrip?count=${c}&brightness=${b}`).then(updateStatus);
}

function toggleAlarm()       { fetch('/toggleAlarm').then(updateStatus); }
function toggleMuteWeekend() { fetch('/toggleMuteWeekend').then(updateStatus); }
function lightOn()           { fetch('/lightOn'); }
function lightOff()          { fetch('/lightOff'); }
function beeperTestAlarm()   { fetch('/beeperTestAlarm'); }
function beeperStop()        { fetch('/beeperStop'); }

document.addEventListener('DOMContentLoaded', init);
setInterval(updateStatus, 5000);
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