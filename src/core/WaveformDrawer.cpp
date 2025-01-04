#include "WaveformDrawer.h"
#include <M5Stack.h>
#include "Free_Fonts.h" 

WaveformDrawer waveformDrawer;

volatile int16_t micWave = 0;
hw_timer_t *timer_wave = NULL;
volatile int16_t adcAverage = 0;

WaveformDrawer::WaveformDrawer() : write_index(0), data_count(0) {
    memset(val_buf, 0, sizeof(val_buf));
}

// 1秒間のAD変換値の平均を取得する関数
void WaveformDrawer::getADCAverage() {
    const unsigned long duration = 1000; // 1秒間 (ミリ秒単位)
    unsigned long startTime = millis();
    long sum = 0;
    int count = 0;

    while (millis() - startTime < duration) {
        int adcValue = analogRead(36);
        sum += adcValue;
        count++;
    }
    adcAverage = (float)sum / count;
}

void IRAM_ATTR onTimerWave() {
    micWave = analogRead(36);
    waveformDrawer.drawWaveform();
}

void WaveformDrawer::startTimer() {
    timer_wave = timerBegin(1, 80, true);
    timerAttachInterrupt(timer_wave, &onTimerWave, true);
    timerAlarmWrite(timer_wave, TIME_WAVE, true);
    timerAlarmEnable(timer_wave);
}

void WaveformDrawer::drawStringWithFormat(const char* label, int value, int x, int y) {
    char buffer[32];
    sprintf(buffer, "%s: %8d", label, value);
    M5.Lcd.drawString(buffer, x, y);
}

void WaveformDrawer::drawWaveform() {
    static unsigned long lastTime = 0;
    unsigned long currentTime = micros();
    unsigned long elapsedTime = currentTime - lastTime;

    drawStringWithFormat("Time", elapsedTime, 0, 0);
    lastTime = currentTime;

    int16_t adcVal = micWave - adcAverage;
    drawStringWithFormat("adcAverage Value", (int)adcAverage, 0, 10);
    drawStringWithFormat("micWave Value", (int)micWave, 0, 20);
    drawStringWithFormat("Graph Value", (int)abs(adcVal), 0, 30);

    static int16_t adc_buf[MAX_LEN] = {0};
    static int16_t val_buf[MAX_LEN] = {0};
    static int16_t integ_buf[MAX_LEN] = {0};
    static int16_t pt = MAX_LEN - 1;
    // 移動積分を管理するための変数
    static long integralValue = 0;       // 移動積分値(直近 N サンプルの合計)
    static int  sampleCount   = 0;       // 何サンプル蓄積したか(立ち上がり時用)
    adc_buf[pt] = adcVal;
    val_buf[pt] = map((int16_t)(adcVal * X_SCALE), -2048, 2048,  0, 100);

    // === 移動積分の計算ロジック ===
    integralValue += abs(adcVal);

    if (sampleCount < INTEGRAL_SAMPLES) {
      // まだ N サンプルに達していない場合 (立ち上がり時)
      sampleCount++;
    } else {
      uint16_t oldPos = (pt + INTEGRAL_SAMPLES) % MAX_LEN;
      integralValue -= abs(adc_buf[oldPos]);
    }
    integ_buf[pt] = map((int16_t)(-integralValue / INTEGRAL_SAMPLES), -2048, 2048, 0, 100);
    drawStringWithFormat("IntegralValue", (int)integralValue/INTEGRAL_SAMPLES, 0, 40);

    if (--pt < 0) {
        pt = MAX_LEN - 1;
    }

    for (int i = 1; i < (MAX_LEN); i++) {
        uint16_t now_pt = (pt + i) % (MAX_LEN);
        M5.Lcd.drawLine(i + X_OFFSET, val_buf[(now_pt + 1) % MAX_LEN] + Y_OFFSET, i + 1 + X_OFFSET, val_buf[(now_pt + 2) % MAX_LEN] + Y_OFFSET, TFT_BLACK);
        if (i < MAX_LEN - 1) {
            M5.Lcd.drawLine(i + X_OFFSET, val_buf[now_pt] + Y_OFFSET, i + 1 + X_OFFSET, val_buf[(now_pt + 1) % MAX_LEN] + Y_OFFSET, TFT_GREEN);
        }
    }

    for (int i = 1; i < (MAX_LEN); i++) {
        uint16_t now_pt = (pt + i) % (MAX_LEN);
        M5.Lcd.drawLine(i + X_OFFSET, integ_buf[(now_pt + 1) % MAX_LEN] + Y_OFFSET, i + 1 + X_OFFSET, integ_buf[(now_pt + 2) % MAX_LEN] + Y_OFFSET, TFT_BLACK);
        if (i < MAX_LEN - 1) {
            M5.Lcd.drawLine(i + X_OFFSET, integ_buf[now_pt] + Y_OFFSET, i + 1 + X_OFFSET, integ_buf[(now_pt + 1) % MAX_LEN] + Y_OFFSET, TFT_BLUE);
        }
    }
}
