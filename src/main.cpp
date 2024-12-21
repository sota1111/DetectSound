// --- Main.cpp ---
#include <M5Stack.h>
#include "WiFiHandler.h"
#include "NoiseDetector.h"
#include "WaveformDrawer.h"
#include "secret.h"

WiFiHandler wifiHandler;
NoiseDetector noiseDetector;
WaveformDrawer waveformDrawer;

void setup() {
    M5.begin();
    wifiHandler.connectWiFi(WIFI_SSID, WIFI_PASSWORD);
    wifiHandler.synchronizeTime();
    noiseDetector.initNoiseDetector();
}

void loop() {
    int micValue = analogRead(36); // MIC_Unit
    waveformDrawer.updateBuffer(micValue);
    noiseDetector.detectNoise(micValue);
    waveformDrawer.drawWaveform();
    delay(20);
}