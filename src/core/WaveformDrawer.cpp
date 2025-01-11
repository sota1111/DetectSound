#include "WaveformDrawer.h"
#include <M5Stack.h>
#include "Free_Fonts.h" 
#include "arduinoFFT.h"

WaveformDrawer waveformDrawer;

volatile int16_t micWave = 0;
hw_timer_t *timer_wave = NULL;
volatile int16_t adcAverage = 0;

WaveformDrawer::WaveformDrawer() {
    adc_buf = nullptr;
    val_buf = nullptr;
    integ_buf = nullptr;
    vReal = nullptr;
    vImag = nullptr;
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
    //M5.Lcd.println("Buffer freed");
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
    M5.Lcd.fillScreen(BLACK);
    timer_wave = timerBegin(1, 80, true);
    timerAttachInterrupt(timer_wave, &onTimerWave, true);
    timerAlarmWrite(timer_wave, TIME_WAVE, true);
    timerAlarmEnable(timer_wave);
}

void WaveformDrawer::restartTimer() {
    memset(adc_buf, 0, sizeof(adc_buf));
    timerRestart(timer_wave);
    timerStart(timer_wave);
}

void WaveformDrawer::DCRemoval(double *vData, unsigned int samples) {
    double mean = 0;
    for (uint16_t i = 0; i < samples; i++) {
        mean += vData[i];
    }
    mean /= samples;
    for (uint16_t i = 0; i < samples; i++) {
        vData[i] -= mean;
    }
}

void WaveformDrawer::drawChart(int nsamples) {
    // グラフ原点や描画可能領域の定義
    int X0 = 30;
    int Y0 = 20;
    int _height = 240 - Y0;   // 実際に使える縦サイズ
    int _width = 320;         // 実際に使える横サイズ
    float dmax = 5.0;         // グラフ化する際の最大値（振幅）

    //==============================================
    // 1. スキップ係数を計算: サンプル数が描画幅を超える場合は間引く
    //   例: nsamples = 500, _width = 320 のとき
    //       -> skipFactor = ceil(500 / 320) = ceil(1.5625) = 2
    //       -> 2 サンプルに 1 回描画する
    //==============================================
    int skipFactor = 1;
    if (nsamples > _width) {
        skipFactor = ceil((float)nsamples / _width);
    }

    // 実際に描画を行う回数(横方向に何本描画するか)
    int drawCount = nsamples / skipFactor;
    if (drawCount < 1) {
        // 安全策として 1 本だけでも描画させる
        drawCount = 1;
    }

    //==============================================
    // 2. 横幅あたりの描画バンド幅を計算し、0 にならないようにする
    //==============================================
    int band_width = floor((float)_width / drawCount);
    if (band_width < 1) {
        band_width = 1;  // 幅が 0 にならないように最低 1 を確保
    }

    // fillRect に与えるパッド幅(余白)を計算
    int band_pad = band_width - 1;
    if (band_pad < 1) {
        band_pad = 1;  // 負数にならないように最低 1 を確保
    }

    //==============================================
    // 3. グラフを描画するループ (スキップ分を加味)
    //==============================================
    for (int i = 0; i < nsamples; i += skipFactor) {
        // band (描画バンドのインデックス) は i / skipFactor で計算
        int band = i / skipFactor;
        int hpos = band * band_width + X0;  // 横方向の描画位置

        // 実際のサンプル値
        float d = vReal[i];
        if (d > dmax) d = dmax;  // オーバー分はクリップ
        if (d < 0.0)  d = 0.0;   // 負値が来る場合はとりあえず 0 にクリップ

        // 高さを計算 (d/dmax) を描画可能領域にスケーリング
        int h = (int)((d / dmax) * _height);

        // fillRect で縦バー描画
        //  (注) y 座標は上から下へ増える前提なので、塗りつぶしは
        //       下から上へ行うように座標を調整
        M5.Lcd.fillRect(hpos, (_height - h), band_pad, h, WHITE);

        //==============================================
        // 4. ラベル(周波数表示など)を描画 (任意)
        //    こちらもサンプル数が大きいと文字が重なるため
        //    ある程度の区切りで表示するようにする
        //==============================================
        if (drawCount >= 4) {
            // drawCount(実際に描画する本数)の1/4おきにラベルを描画
            int label_interval = drawCount / 4;
            if (label_interval <= 0) {
                label_interval = 1;
            }
            if ( (band % label_interval) == 0 ) {
                M5.Lcd.setCursor(hpos, _height + Y0 - 10);
                M5.Lcd.printf("%.1fHz",
                    ((i * 1.0 * SAMPLING_FREQUENCY) / FFTsamples)
                );
            }
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
    Serial.println("vReal contents:");
    for (int i = 0; i < GRAPH_MAX_LEN; i++) {
        Serial.print(vReal[i]);
        if (i < GRAPH_MAX_LEN - 1) {
            Serial.print(", ");
        }
    }
    Serial.println();
    drawChart(FFTsamples / 2);
}

void WaveformDrawer::startFFT() {
    timerStop(timer_wave);
    M5.Lcd.fillScreen(BLACK);
    doFFT();
    freeBuffer();
    initWaveformDrawer();
    restartTimer();
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

    #if 0

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

    #endif
    static int16_t pt = GRAPH_MAX_LEN - 1;
    adc_buf[pt] = micWave;

    if (--pt < 0) {
        pt = GRAPH_MAX_LEN - 1;
    }

    #if 0
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
    #endif
}
