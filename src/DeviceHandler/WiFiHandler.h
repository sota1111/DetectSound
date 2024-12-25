#ifndef WIFIHANDLER_H
#define WIFIHANDLER_H

#include <WiFi.h>
#include <M5Stack.h>
#include "../config/config.h"

class WiFiHandler {
public:
    void connectWiFi(const char* ssid, const char* password);
    void synchronizeTime();
};

#endif