#ifndef NOISEDETECTOR_H
#define NOISEDETECTOR_H

#include <M5Stack.h>
#include <SD.h>
#include <time.h>
#include "../config/config.h"

class NoiseDetector {
private:
    int maxMicValue;
    void logNoiseTimestamp();

public:
    void initNoiseDetector();
    void detectNoise(int micValue);
};

#endif