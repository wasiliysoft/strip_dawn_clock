#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include "Config.h"

void setupWiFi() {
  WiFiManager wifiManager;
  
  // Авто-подключение с точкой доступа для настройки
  if (!wifiManager.autoConnect(AP_NAME)) {
    Serial.println("Failed to connect, restarting...");
    delay(3000);
    ESP.restart();
  }
  
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

#endif