#include <M5Stack.h>
#include "core/NoiseDetector.h"
#include "core/WaveformDrawer.h"
#include "config/secret.h"

// 現在のモードを記憶する変数
enum Mode { NONE, NOISE_DETECTOR, WAVEFORM_DRAWER };
Mode currentMode = NONE;

void showMenu() {
    M5.Lcd.clear();
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("Press Button");
    M5.Lcd.drawString("Noise", 35, 200);
    M5.Lcd.drawString("Detector", 35, 220);
    M5.Lcd.drawString("Wave", 230, 200);
    M5.Lcd.drawString("Drawer", 230, 220);
    M5.Lcd.setTextSize(1);
}

void setup() {
    M5.begin();
    showMenu();
}

void loop() {

    if (currentMode == NONE) {
        M5.update();
        if (M5.BtnA.wasPressed()) {  // 左ボタン
            M5.Lcd.clear();
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.printf("Starting Noise Detector...\n");
            noiseDetector.initNoiseDetector();
            noiseDetector.getADCAverage();
            noiseDetector.startTimer();
            currentMode = NOISE_DETECTOR;
        } else if (M5.BtnC.wasPressed()) {  // 右ボタン
            M5.Lcd.clear();
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.printf("Starting Waveform Drawer...\n");
            waveformDrawer.initWaveformDrawer();
            waveformDrawer.getADCAverage();
            waveformDrawer.startTimer();
            M5.Lcd.clear();
            currentMode = WAVEFORM_DRAWER;
        }
    } else {
        if (currentMode == NOISE_DETECTOR) {
            noiseDetector.storeNoise();
        } else if (currentMode == WAVEFORM_DRAWER) {
        }
    }
}
