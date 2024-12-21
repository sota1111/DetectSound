#include "WiFiHandler.h"

void WiFiHandler::connectWiFi(const char* ssid, const char* password) {
    M5.Lcd.printf("Connecting to %s", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Lcd.print('.');
    }
    M5.Lcd.println("\nWiFi Connected!");
}

void WiFiHandler::synchronizeTime() {
    configTime(JST, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp");
}