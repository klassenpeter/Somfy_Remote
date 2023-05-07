#pragma once

#include <Arduino.h>
#include "config.h"

void connectWiFi()
{
    // Connect to WiFi
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);

    WiFi.begin(wifi_ssid, wifi_password);

#ifdef ESP32
    WiFi.setHostname("ESP32-somfy");
#elif ESP8266
    WiFi.hostname("ESP8266-somfy");
#endif

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

WiFiClient wifiClient;