#include "NoiseDetector.h"
#include "SDCardHandler.h"
#include "SpeakerHandler.h"

SDCardHandler sdcardHandler;
SpeakerHandler speakerHandler;

// 初期化
void NoiseDetector::initNoiseDetector() {
    sdcardHandler.initSDCard("/Data");
}

void NoiseDetector::logNoiseTimestamp() {
    struct tm timeInfo;
    if (getLocalTime(&timeInfo)) {
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeInfo);
        sdcardHandler.writeSDCard("/Data/noise.txt", timestamp);
    } else {
        M5.Lcd.println("Failed to get time");
    }
}

void NoiseDetector::detectNoise(int micValue) {
    maxMicValue = micValue;
    if (maxMicValue > NOISE_CONSTANT_VALUE) {
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
        M5.Lcd.setTextSize(FONT_SIZE);
        M5.Lcd.drawString("NOISE", 160, 360);
        speakerHandler.playTone(440, 100);
        logNoiseTimestamp();
    }
}