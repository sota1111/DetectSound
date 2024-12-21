#ifndef NOISEDETECTOR_H
#define NOISEDETECTOR_H

#define FONT_SIZE 2

#include <M5Stack.h>
#include <SD.h>
#include <time.h>
#include "common.h"

class NoiseDetector {
private:
    int maxMicValue;
    void logNoiseTimestamp();

public:
    void initNoiseDetector();
    void detectNoise(int micValue);
};

#endif