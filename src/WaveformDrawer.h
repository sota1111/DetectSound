#ifndef WAVEFORMDRAWER_H
#define WAVEFORMDRAWER_H

#include <M5Stack.h>
#include "common.h"

#define MAX_LEN 320
#define X_OFFSET 0
#define Y_OFFSET 100
#define X_SCALE 1

class WaveformDrawer {
private:
    int16_t val_buf[MAX_LEN];
    int write_index;
    int data_count;

public:
    WaveformDrawer();
    void updateBuffer(int micValue);
    void drawWaveform();
};

#endif