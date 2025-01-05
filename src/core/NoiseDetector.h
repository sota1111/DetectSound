#ifndef NOISEDETECTOR_H
#define NOISEDETECTOR_H

#include <M5Stack.h>
#include <SD.h>
#include <time.h>
#include "../config/config.h"
#include "../config/secret.h"

class NoiseDetector {
private:
    bool isRequestSpeaker;
    bool isDataStored;
    bool isTimerStopped;
    unsigned int write_index;
    unsigned int detect_index;
    int16_t* val_buf=nullptr;
    void logNoiseTimestamp();
    void notificationAWS();
    void postCSVtoServer(const char* fileName);
    void freeBuffer();

public:
    NoiseDetector();
    ~NoiseDetector();
    void initNoiseDetector();
    void updateBuffer(int micValue);
    void startTimer();
    void restartTimer();
    bool judgeRestartTimer();
    void storeNoise();
};

extern NoiseDetector noiseDetector;

#endif