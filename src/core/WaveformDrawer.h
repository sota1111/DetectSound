#ifndef WAVEFORMDRAWER_H
#define WAVEFORMDRAWER_H

#include <M5Stack.h>
#include "../config/config.h"

#define MAX_LEN 320
#define X_OFFSET 0
#define Y_OFFSET 100
#define X_SCALE 1
#define MIC_Unit 36

class WaveformDrawer {
private:
    int16_t val_buf[MAX_LEN];
    int write_index;
    int data_count;
    void drawStringWithFormat(const char* label, int value, int x, int y);

public:
    WaveformDrawer();
    void getADCAverage();
    void startTimer();
    void drawWaveform();
};

extern WaveformDrawer waveformDrawer;

#endif