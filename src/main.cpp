// --- Main.cpp ---
#include <M5Stack.h>
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
    noiseDetector.startTimer();
}

void loop() {
    noiseDetector.storeNoise();
}


