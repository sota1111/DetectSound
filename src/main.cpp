// --- Main.cpp ---
#include <M5Stack.h>
#include "core/NoiseDetector.h"
#include "core/WaveformDrawer.h"
#include "config/secret.h"

void setup() {
    M5.begin();
    //noiseDetector.initNoiseDetector();
    //noiseDetector.startTimer();
    waveformDrawer.getADCAverage();
    waveformDrawer.startTimer();
}

void loop() {
    //noiseDetector.storeNoise();
    //waveformDrawer.drawWaveform();
}


