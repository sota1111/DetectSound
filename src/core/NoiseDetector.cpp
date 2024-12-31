#include "NoiseDetector.h"
#include "DeviceHandler/SDCardHandler.h"
#include "DeviceHandler/SpeakerHandler.h"

SDCardHandler sdcardHandler;
SpeakerHandler speakerHandler;
NoiseDetector noiseDetector;

volatile int micValue = 0;
volatile unsigned long irqTime;
hw_timer_t *timer = NULL;

// 初期化
NoiseDetector::NoiseDetector() : isRequestSpeaker(false), isDataStored(false), isTimerStopped(false), write_index(0), detect_index(0) {
    memset(val_buf, 0, sizeof(val_buf));
}

void NoiseDetector::initNoiseDetector() {
    sdcardHandler.initSDCard("/Data");
}

void NoiseDetector::updateBuffer(int micValue) {
    static unsigned int detect_count = 0;
    static bool isNoiseDetected = false;
    static unsigned int startTime = 0;

    if(!isDataStored){
        // ノイズを検出前後のデータを記録        
        write_index = (write_index + 1) % RECORD_MAX_LEN;
        val_buf[write_index] = micValue;
        // ノイズ検出
        if((micValue > NOISE_CONSTANT_VALUE) && (isNoiseDetected == false)){
            isNoiseDetected = true;
            detect_index = write_index;
            M5.Lcd.setCursor(0, 20);
            M5.Lcd.printf("NOISE DETECTED");
            startTime = millis();
        }
        if(isNoiseDetected){
            detect_count++;
        }
        // ノイズ検出後、データを貯め続ける。
        if(detect_count > RECORD_AFTER_LEN){
            timerStop(timer);
            isDataStored = true;
            isNoiseDetected = false;
            isRequestSpeaker = true;
            detect_count = 0;
            M5.Lcd.setCursor(0, 40);
            M5.Lcd.printf("BUFFER FULL");
            M5.Lcd.setCursor(0, 60);
            M5.Lcd.printf("STOP TIMER");
            int stopTime = millis() - startTime;
            M5.Lcd.setCursor(0, 80);
            M5.Lcd.printf("Time: %d", stopTime);     
        }
    }
}

void NoiseDetector::logNoiseTimestamp() {
    struct tm timeInfo;
    String csvData = "";
    // detect_index+RECORD_AFTER_LENがcsvDataの先頭になるようにする
    int startIndex = detect_index + RECORD_AFTER_LEN + 1;
    int endIndex = startIndex + RECORD_MAX_LEN;
    detect_index = 0;

    for (int i = startIndex; i < endIndex; i++) {
        csvData += String(val_buf[i % RECORD_MAX_LEN]) + ",";
    }

    if (getLocalTime(&timeInfo)) {
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H-%M-%S", &timeInfo);

        // CSVファイル名を生成
        char fileName[128];
        snprintf(fileName, sizeof(fileName), "/Data/log_%s.csv", timestamp);
        File csvFile = SD.open(fileName, FILE_APPEND);
        csvFile.print(csvData.c_str());
        csvFile.close();
        M5.Lcd.setCursor(0, 100);
        M5.Lcd.printf("STORE DATA");
    } else {
        // 現在時刻が取得できなかった場合のエラーメッセージ
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
        M5.Lcd.setCursor(0, 100);
        M5.Lcd.println("Current time has not been obtained.");
        delay(1000);
    }
}

void NoiseDetector::storeNoise() {
    if (isRequestSpeaker) {
        speakerHandler.playTone(440, 100);
        isRequestSpeaker = false;
    }

    if (isDataStored) {
        logNoiseTimestamp();
        isDataStored = false;
        M5.Lcd.setCursor(0, 120);
        M5.Lcd.println("NOISE STORED");
        restartTimer();
    }
}

void IRAM_ATTR onTimer() {
    micValue = analogRead(36);
    noiseDetector.updateBuffer(micValue);
}

void NoiseDetector::startTimer() {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, TIME_IRQ, true);
    timerAlarmEnable(timer);
}

void NoiseDetector::restartTimer() {
    M5.Lcd.setCursor(0, 140);
    M5.Lcd.println("Restart Timer");
    delay(100);
    timerRestart(timer);
    timerStart(timer);
    M5.Lcd.fillScreen(TFT_BLACK);
    delay(100);
}

