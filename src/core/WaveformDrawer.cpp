#include "WaveformDrawer.h"
#include <M5Stack.h>
#include "Free_Fonts.h" 
#include "arduinoFFT.h"

WaveformDrawer waveformDrawer;

volatile int16_t micWave = 0;
hw_timer_t *timer_wave = NULL;
volatile int16_t adcAverage = 0;

WaveformDrawer::WaveformDrawer() : write_index(0), data_count(0) {
}

WaveformDrawer::~WaveformDrawer() {
    freeBuffer();
}

void WaveformDrawer::freeBuffer() {
    if (adc_buf) {
        delete[] adc_buf;
        adc_buf = nullptr;

    }
    if (val_buf) {
        delete[] val_buf;
        val_buf = nullptr;
    }
    if (integ_buf) {
        delete[] integ_buf;
        integ_buf = nullptr;
    }
    if (vReal) {
        delete[] vReal;
        vReal = nullptr;
    }
    if (vImag) {
        delete[] vImag;
        vImag = nullptr;
    }
    M5.Lcd.println("Buffer freed");
}

void WaveformDrawer::initWaveformDrawer() {
    freeBuffer();
    adc_buf = new int16_t[GRAPH_MAX_LEN];
    if (adc_buf == nullptr) {
        M5.Lcd.println("Failed to allocate buffer");
        return;
    }
    val_buf = new int16_t[GRAPH_MAX_LEN];
    if (val_buf == nullptr) {
        M5.Lcd.println("Failed to allocate buffer");
        return;
    }
    integ_buf = new int16_t[GRAPH_MAX_LEN];
    if (integ_buf == nullptr) {
        M5.Lcd.println("Failed to allocate buffer");
        return;
    }
    vReal = new double[FFTsamples];
    if (vReal == nullptr) {
        M5.Lcd.println("Failed to allocate buffer");
        return;
    }
    vImag = new double[FFTsamples];
    if (vImag == nullptr) {
        M5.Lcd.println("Failed to allocate buffer");
        return;
    }
    for (int i = 0; i < GRAPH_MAX_LEN; i++) {
        adc_buf[i] = 0;
        val_buf[i] = 0;
        integ_buf[i] = 0;
    }
    for (int i = 0; i < FFTsamples; i++) {
        vReal[i] = 0;
        vImag[i] = 0;
    }
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

void WaveformDrawer::DCRemoval(double *vData, uint16_t samples) {
    double mean = 0;
    for (uint16_t i = 1; i < samples; i++) {
        mean += vData[i];
    }
    mean /= samples;
    for (uint16_t i = 1; i < samples; i++) {
        vData[i] -= mean;
    }
}

void WaveformDrawer::drawChart(int nsamples) {
    int X0 = 30;
    int Y0 = 20;
    int _height = 240 - Y0;
    int _width = 320;
    float dmax = 5.0;
    int band_width = floor(_width / nsamples);
    int band_pad = band_width - 1;

    for (int band = 0; band < nsamples; band++) {
        int hpos = band * band_width + X0;
        float d = vReal[band];
        if (d > dmax) d = dmax;
        int h = (int)((d / dmax) * (_height));
        M5.Lcd.fillRect(hpos, _height - h, band_pad, h, WHITE);
        if ((band % (nsamples / 4)) == 0) {
            M5.Lcd.setCursor(hpos, _height + Y0 - 10);
            M5.Lcd.printf("%.1fHz", ((band * 1.0 * SAMPLING_FREQUENCY) / FFTsamples));
        }
    }
}

void WaveformDrawer::doFFT() {
    // adc_bufをvRealに代入（ADCデータのコピー）
    for (int i = 0; i < FFTsamples; i++) {
        vReal[i] = static_cast<double>(adc_buf[i]);
    }
    DCRemoval(vReal, FFTsamples);
    // FFT処理
    ArduinoFFT<double> FFT = ArduinoFFT<double>();
    FFT.windowing(vReal, FFTsamples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  // ハミング窓
    FFT.compute(vReal, vImag, FFTsamples, FFT_FORWARD);  // FFT計算
    FFT.complexToMagnitude(vReal, vImag, FFTsamples);  // 実数信号の絶対値計算
    for (int i = 0; i < FFTsamples; i++) {
        vReal[i] = vReal[i] / (FFTsamples / 2);  // 最大値をサンプル数で割る
    }
    drawChart(FFTsamples / 2);
}

void WaveformDrawer::startFFT() {
    timerStop(timer_wave);
    M5.Lcd.fillScreen(BLACK);
    doFFT();
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
    drawStringWithFormat("micWave Value", (int)micWave, 0, 10);
    drawStringWithFormat("Graph Value", (int)abs(adcVal), 0, 20);

    static int16_t pt = GRAPH_MAX_LEN - 1;
    adc_buf[pt] = adcVal;
    val_buf[pt] = map((int16_t)(adcVal * X_SCALE), -2048, 2048,  0, 100);

    // === 移動積分の計算ロジック ===
    static long integralValue = 0;       // 移動積分値(直近 N サンプルの合計)
    static int  sampleCount   = 0;       // 何サンプル蓄積したか(立ち上がり時用)

    integralValue += abs(adcVal);

    if (sampleCount < INTEGRAL_SAMPLES) {
      // まだ N サンプルに達していない場合 (立ち上がり時)
      sampleCount++;
    } else {
      uint16_t oldPos = (pt + INTEGRAL_SAMPLES) % GRAPH_MAX_LEN;
      integralValue -= abs(adc_buf[oldPos]);
    }
    integ_buf[pt] = map((int16_t)(-integralValue / INTEGRAL_SAMPLES), -2048, 2048, 0, 100);
    int16_t avgIntegral = integralValue / sampleCount;
    drawStringWithFormat("IntegralValue", (int)avgIntegral, 0, 30);

    // 0 → 40dB, 100 → 60dB, 500 → 80dB になるように補完
    int dBValue;
    if (avgIntegral < 100) {
        // f(x) = 40 + ((80 - 40) / 200) * x = 40 + 0.2 * x
        dBValue = 40 + (2 * avgIntegral)/10;
    } else {
        // 60.0f + (80.0f - 60.0f) * ((float)x - 100.0f) / 400.0f;
        // y = 60 + 0.05 * (x - 100)
        dBValue = 60 + (avgIntegral - 100)/20;
    }
    M5.Lcd.setTextSize(3);
    drawStringWithFormat("dBValue", (int)dBValue, 0, 40);
    M5.Lcd.setTextSize(1);

    if (--pt < 0) {
        pt = GRAPH_MAX_LEN - 1;
    }

    static int countGraphX;
    for (int i = 1; i < (GRAPH_MAX_LEN); i++) {
        uint16_t now_pt = (pt + i) % (GRAPH_MAX_LEN);
        M5.Lcd.drawLine(i + X_OFFSET, val_buf[(now_pt + 1) % GRAPH_MAX_LEN] + Y_OFFSET, i + 1 + X_OFFSET, val_buf[(now_pt + 2) % GRAPH_MAX_LEN] + Y_OFFSET, TFT_BLACK);
        M5.Lcd.drawLine(i + X_OFFSET, integ_buf[(now_pt + 1) % GRAPH_MAX_LEN] + Y_OFFSET, i + 1 + X_OFFSET, integ_buf[(now_pt + 2) % GRAPH_MAX_LEN] + Y_OFFSET, TFT_BLACK);
        if (i < GRAPH_MAX_LEN - 1) {
            M5.Lcd.drawLine(i + X_OFFSET, val_buf[now_pt] + Y_OFFSET, i + 1 + X_OFFSET, val_buf[(now_pt + 1) % GRAPH_MAX_LEN] + Y_OFFSET, TFT_GREEN);
            M5.Lcd.drawLine(i + X_OFFSET, integ_buf[now_pt] + Y_OFFSET, i + 1 + X_OFFSET, integ_buf[(now_pt + 1) % GRAPH_MAX_LEN] + Y_OFFSET, TFT_BLUE);
        }
    }
}
