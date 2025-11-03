#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>

// Таймаут портала конфигурации:
// нужно чтобы не зависала в режиме настройки из-за пропадания эл. питания.
// https://github.com/tzapu/WiFiManager?tab=readme-ov-file#configuration-portal-timeout
#define CONFIG_PORTAL_TIMEOUT_SEC 180

class WiFiMgr {
public:
  WiFiMgr() {}
  void begin(const char *apName) {
    WiFiManager wifiManager;
    wifiManager.setConfigPortalTimeout(CONFIG_PORTAL_TIMEOUT_SEC);
    // Авто-подключение с точкой доступа для настройки
    if (!wifiManager.autoConnect(apName)) {
      Serial.println("Failed to connect, restarting...");
      delay(3000);
      ESP.restart();
    }

    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin(apName)) {
      Serial.print("mDNS responder started: http://");
      Serial.print(apName);
      Serial.println(".local");
      // Advertise HTTP service
      MDNS.addService("http", "tcp", 80);
    } else {
      Serial.println("Error setting up mDNS responder");
    }
  };
  void update() { MDNS.update(); }
};

#endif