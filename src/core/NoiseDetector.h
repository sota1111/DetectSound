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
    unsigned long noiseEventTimes[MAX_NOISE_EVENTS];
    long integralValue;
    int  sampleIntegralCount;
    void initBuf();
    bool detectNoise(int avgIntegral);
    int calculateDbValue(int avgIntegral);
    int calculateMovingIntegral(int currentMicValue, int writeIndex);
    void logNoiseTimestamp();
    void notificationAWS();
    void postCSVtoServer(const char* fileName);
    void freeBuffer();

public:
    NoiseDetector();
    ~NoiseDetector();
    void initNoiseDetector();
    void getADCAverage();
    void updateBuffer(int micValue);
    void startTimer();
    void restartTimer();
    bool judgeRestartTimer();
    void storeNoise();
};

extern NoiseDetector noiseDetector;

#endif