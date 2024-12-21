// --- Main.cpp ---
#include <M5Stack.h>
#include "WiFiHandler.h"
#include "NoiseDetector.h"
#include "WaveformDrawer.h"
#include "secret.h"

WiFiHandler wifiHandler;
NoiseDetector noiseDetector;
WaveformDrawer waveformDrawer;

volatile int micValue = 0;
volatile int maxMicValue = 0;
hw_timer_t *timer = NULL;

void IRAM_ATTR onTimer() {
    micValue = analogRead(36);
    waveformDrawer.updateBuffer(micValue);
    maxMicValue = waveformDrawer.calcMaxADValue(micValue);
}

void setup() {
    M5.begin();
    wifiHandler.connectWiFi(WIFI_SSID, WIFI_PASSWORD);
    wifiHandler.synchronizeTime();
    noiseDetector.initNoiseDetector();
    timer = timerBegin(0, 80, true);         // タイマー0を設定 (80分周 -> 1µs)
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000, true);      // 1msごとに割り込み
    timerAlarmEnable(timer);
}

void loop() {
    M5.Lcd.fillScreen(TFT_BLACK);
    waveformDrawer.drawMaxADValue(micValue);
    noiseDetector.detectNoise(maxMicValue);
    waveformDrawer.drawWaveform();
    delay(20);
}


