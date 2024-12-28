#include "NoiseDetector.h"
#include "DeviceHandler/SDCardHandler.h"
#include "DeviceHandler/SpeakerHandler.h"

SDCardHandler sdcardHandler;
SpeakerHandler speakerHandler;

// 初期化
NoiseDetector::NoiseDetector() : write_index(0){
    memset(val_buf, 0, sizeof(val_buf));
}

void NoiseDetector::initNoiseDetector() {
    sdcardHandler.initSDCard("/Data");
}

void NoiseDetector::updateBuffer(int micValue) {
    val_buf[write_index] = micValue;
    write_index = (write_index + 1) % RECORD_MAX_LEN;
}

void NoiseDetector::logNoiseTimestamp() {
    struct tm timeInfo;
    if (getLocalTime(&timeInfo)) {
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H-%M-%S", &timeInfo);

        // CSVファイル名を生成
        char fileName[128];
        snprintf(fileName, sizeof(fileName), "/Data/noise_%s.csv", timestamp);

        // CSV形式で val_buf のデータを文字列に変換
        String csvData = "";
        for (int i = 0; i < RECORD_MAX_LEN; i++) {
            csvData += String(val_buf[i]);
            if (i < RECORD_MAX_LEN - 1) {
                csvData += ",";
            }
        }

        // SDカードにCSV形式でデータを書き込み
        if (sdcardHandler.writeSDCard(fileName, csvData.c_str())) {
            // 書き込み成功メッセージ
            M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
            M5.Lcd.setCursor(0, 80);
            M5.Lcd.println("Success to write data on SD card");
        } else {
            // 書き込み失敗メッセージ
            M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
            M5.Lcd.setCursor(0, 80);
            M5.Lcd.println("Failed to write data to SD card");
        }

        delay(1000);
    } else {
        // 現在時刻が取得できなかった場合のエラーメッセージ
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