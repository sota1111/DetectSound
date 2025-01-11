#ifndef WAVEFORMDRAWER_H
#define WAVEFORMDRAWER_H

#include <M5Stack.h>
#include "../config/config.h"

#define GRAPH_MAX_LEN 256
#define FFTsamples GRAPH_MAX_LEN // 2のべき乗
#define SAMPLING_FREQUENCY 20
#define X_OFFSET 0
#define Y_OFFSET 100
#define X_SCALE 1
#define MIC_Unit 36

class WaveformDrawer {
private:
    int16_t* adc_buf=nullptr;
    int16_t* val_buf=nullptr;
    int16_t* integ_buf=nullptr;
    double* vReal=nullptr;
    double* vImag=nullptr;
    int write_index;
    int data_count;
    void drawStringWithFormat(const char* label, int value, int x, int y);
    void freeBuffer();
    void doFFT();
    void DCRemoval(double *vData, uint16_t samples);
    void drawChart(int samples);

public:
    WaveformDrawer();
    ~WaveformDrawer();
    void getADCAverage();
    void startTimer();
    void drawWaveform();
    void initWaveformDrawer();
    void startFFT();
};

extern WaveformDrawer waveformDrawer;

#endif