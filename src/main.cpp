// --- Main.cpp ---
#include <M5Stack.h>
#include "core/NoiseDetector.h"
#include "core/WaveformDrawer.h"
#include "DeviceHandler/WiFiHandler.h"
#include "config/secret.h"

WiFiHandler wifiHandler;
NoiseDetector noiseDetector;
WaveformDrawer waveformDrawer;

volatile int micValue = 0;
volatile int maxMicValue = 0;
volatile unsigned long irqTime;
hw_timer_t *timer = NULL;

void IRAM_ATTR onTimer() {
    unsigned long startTime = micros();
    micValue = analogRead(36);
    waveformDrawer.updateBuffer(micValue);
    noiseDetector.updateBuffer(micValue);
    maxMicValue = waveformDrawer.calcMaxADValue(micValue);
    irqTime = micros() - startTime;
}

void setup() {
    M5.begin();
    wifiHandler.connectWiFi(WIFI_SSID, WIFI_PASSWORD);
    wifiHandler.synchronizeTime();
    noiseDetector.initNoiseDetector();
    timer = timerBegin(0, 80, true);         // タイマー0を設定 (80分周 -> 1µs)
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, TIME_IRQ, true);
    timerAlarmEnable(timer);
}

void loop() {
    M5.Lcd.fillScreen(TFT_BLACK);
    waveformDrawer.drawMaxADValue(maxMicValue);
    noiseDetector.detectNoise(maxMicValue);
    waveformDrawer.drawWaveform();
    // 画面左下にIRQの時間を表示
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(0, 220);
    M5.Lcd.printf("IRQ Time: %lu us", irqTime);
    delay(20);
}


