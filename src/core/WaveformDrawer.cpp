#include "WaveformDrawer.h"
#include <M5Stack.h>
#include "Free_Fonts.h" 

WaveformDrawer waveformDrawer;

volatile int micWave = 0;
hw_timer_t *timer_wave = NULL;

WaveformDrawer::WaveformDrawer() : write_index(0), data_count(0) {
    memset(val_buf, 0, sizeof(val_buf));
}

// AD値を表示
int WaveformDrawer::calcMaxADValue(int micWave) {
    static int maxMicValue = 0;
    static unsigned long lastUpdateTime = 0;

    if (micWave > maxMicValue) {
        maxMicValue = micWave;
    }

    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= TIME_UPDATE_MAX_VALUE) { 
        lastUpdateTime = currentTime;
        maxMicValue = 0;
    }
    return maxMicValue;
}

void WaveformDrawer::drawMaxADValue(int maxMicValue) {
    M5.Lcd.drawString("Max AD Value: " + String(maxMicValue), 160, 10);
    int max_dB = (maxMicValue-1920)*104/(4095-1920)+30;
    M5.Lcd.drawString("dB: " + String(max_dB), 160, 30);
}

void WaveformDrawer::updateBuffer(int micWave) {
    val_buf[write_index] = map(micWave * X_SCALE, 1800, 4095, 0, 100);
    write_index = (write_index + 1) % MAX_LEN;
    if (data_count < MAX_LEN) {
        data_count++;
    }
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

void WaveformDrawer::drawWaveform() {
    // 実行周期を計測
    static unsigned long lastTime = 0;
    unsigned long currentTime = micros();
    
    M5.Lcd.drawString("Time: " + String(currentTime - lastTime), 160, 50);
    lastTime = currentTime;

    static int16_t val_buf[MAX_LEN] = {0};
    static int16_t pt = MAX_LEN - 1;
    val_buf[pt] = map((int16_t)(micWave * X_SCALE), 1800, 4095,  0, 100);

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
}
