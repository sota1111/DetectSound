// --- Main.cpp ---
#include <M5Stack.h>
#include "core/NoiseDetector.h"
#include "core/WaveformDrawer.h"
#include "DeviceHandler/WiFiHandler.h"
#include "config/secret.h"

WiFiHandler wifiHandler;
WaveformDrawer waveformDrawer;

volatile int maxMicValue = 0;

void setup() {
    M5.begin();
    wifiHandler.connectWiFi(WIFI_SSID, WIFI_PASSWORD);
    wifiHandler.synchronizeTime();
    noiseDetector.initNoiseDetector();
    M5.Lcd.fillScreen(TFT_BLACK);
    noiseDetector.startTimer();
}

void loop() {
    //M5.Lcd.fillScreen(TFT_BLACK);
    //waveformDrawer.drawMaxADValue(maxMicValue);
    noiseDetector.storeNoise();
    //waveformDrawer.drawWaveform();
    // 画面左下にIRQの時間を表示
    //M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    //M5.Lcd.setTextSize(1);
    //M5.Lcd.setCursor(0, 220);
    //M5.Lcd.printf("IRQ Time: %lu us", irqTime);
    //delay(20);
}


