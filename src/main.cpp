// --- Main.cpp ---
#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "core/NoiseDetector.h"
#include "core/WaveformDrawer.h"
#include "DeviceHandler/WiFiHandler.h"
#include "config/secret.h"

WiFiHandler wifiHandler;
WaveformDrawer waveformDrawer;

void setup() {
    M5.begin();
    wifiHandler.connectWiFi(WIFI_SSID, WIFI_PASSWORD);
    wifiHandler.synchronizeTime();
    noiseDetector.initNoiseDetector();
    M5.Lcd.fillScreen(TFT_BLACK);
    // hello worldをGet
    // Lambdaからデータ取得
    HTTPClient http;
    String url = "https://k4eittjcp9.execute-api.ap-northeast-1.amazonaws.com/Prod/hello/";
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
        String payload = http.getString();
        M5.Lcd.println("Response:");
        M5.Lcd.println(payload);
    } else {
        M5.Lcd.println("HTTP GET failed, error: " + String(httpCode));
    }
    http.end();
    delay(1000);
    M5.Lcd.fillScreen(TFT_BLACK);
    noiseDetector.startTimer();
}

void loop() {
    noiseDetector.storeNoise();
}


