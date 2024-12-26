#include "NoiseDetector.h"
#include "DeviceHandler/SDCardHandler.h"
#include "DeviceHandler/SpeakerHandler.h"

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
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H-%M-%S", &timeInfo);

        // Generate file name based on the current timestamp
        char fileName[128];
        snprintf(fileName, sizeof(fileName), "/Data/noise_%s.txt", timestamp);

        // Write the timestamp into the newly created file
        sdcardHandler.writeSDCard(fileName, timestamp);
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Lcd.setCursor(0,80);
        M5.Lcd.println("Success to write time on SD card");
        delay(1000);
    } else {
        // Display error message if the current time cannot be obtained
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
        M5.Lcd.setCursor(0, 80);
        M5.Lcd.println("Current time has not been obtained.");
        delay(1000);
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