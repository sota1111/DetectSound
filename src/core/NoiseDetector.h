#ifndef NOISEDETECTOR_H
#define NOISEDETECTOR_H

#include <M5Stack.h>
#include <SD.h>
#include <time.h>
#include "../config/config.h"

#define RECORD_MAX_LEN 1/TIME_IRQ*1000*1000

class NoiseDetector {
private:
    int16_t val_buf[RECORD_MAX_LEN];
    int write_index;
    int maxMicValue;
    void logNoiseTimestamp();

public:
    NoiseDetector();
    void initNoiseDetector();
    void updateBuffer(int micValue);
    void detectNoise(int micValue);
};

#endif