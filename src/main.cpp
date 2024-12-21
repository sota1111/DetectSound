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
hw_timer_t *timer = NULL;

void IRAM_ATTR onTimer() {
    micValue = analogRead(36);
    waveformDrawer.updateBuffer(micValue);
    noiseDetector.detectNoise(micValue);
}

void setup() {
    M5.begin();
    //wifiHandler.connectWiFi(WIFI_SSID, WIFI_PASSWORD);
    //wifiHandler.synchronizeTime();
    noiseDetector.initNoiseDetector();
    timer = timerBegin(0, 80, true);         // タイマー0を設定 (80分周 -> 1µs)
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000, true);      // 1msごとに割り込み
    timerAlarmEnable(timer);
}

void loop() {
    M5.Lcd.fillScreen(TFT_BLACK); // 描画領域をクリア
  
    // 最大AD値を描画
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextSize(1); // AD値の文字サイズを初期化
    M5.Lcd.drawString("Max AD Value: " + String(micValue), 160, 10);
    int max_dB = (micValue-1920)*104/(4095-1920)+30;
    M5.Lcd.drawString("dB: " + String(max_dB), 160, 30);
    waveformDrawer.drawWaveform();
    delay(20);
}


