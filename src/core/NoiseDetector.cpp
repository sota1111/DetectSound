#include <WiFi.h>
#include <HTTPClient.h>
#include "NoiseDetector.h"
#include "DeviceHandler/WiFiHandler.h"
#include "DeviceHandler/SDCardHandler.h"
#include "DeviceHandler/SpeakerHandler.h"

SDCardHandler sdcardHandler;
SpeakerHandler speakerHandler;
NoiseDetector noiseDetector;
WiFiHandler wifiHandler;

volatile int micValue = 0;
volatile unsigned long irqTime;
hw_timer_t *timer = NULL;

// 初期化
NoiseDetector::NoiseDetector() : isRequestSpeaker(false), isDataStored(false), isTimerStopped(false), write_index(0), detect_index(0) {
    memset(val_buf, 0, sizeof(val_buf));
}

void NoiseDetector::initNoiseDetector() {
    sdcardHandler.initSDCard("/Data");
    wifiHandler.connectWiFi(WIFI_SSID, WIFI_PASSWORD);
    wifiHandler.synchronizeTime();

    // hello worldをGet
    // Lambdaからデータ取得
    HTTPClient http;
    String url = String(BASE_URL) + "hello/";
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
        String payload = http.getString();
        M5.Lcd.println("Response:");
        M5.Lcd.println(payload);
    } else {
        M5.Lcd.println("HTTP GET failed, error: " + String(httpCode));
    }
    http.end();
    delay(1000);
    M5.Lcd.fillScreen(TFT_BLACK);
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

        // POST処理を追加
        postCSVtoServer(fileName);
    } else {
        // 現在時刻が取得できなかった場合のエラーメッセージ
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
        M5.Lcd.setCursor(0, 100);
        M5.Lcd.println("Current time has not been obtained.");
        delay(1000);
    }
}

void NoiseDetector::postCSVtoServer(const char* fileName) {
    HTTPClient http;
    String url = String(BASE_URL) + "detect-sound";
    File csvFile = SD.open(fileName, FILE_READ);

    if (!csvFile) {
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
        M5.Lcd.setCursor(0, 140);
        M5.Lcd.println("Failed to open CSV file.");
        return;
    }

    size_t fileSize = csvFile.size();
    uint8_t* buffer = (uint8_t*)malloc(fileSize);
    if (buffer == NULL) {
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
        M5.Lcd.setCursor(0, 140);
        M5.Lcd.println("Failed to allocate memory.");
        csvFile.close();
        return;
    }

    csvFile.read(buffer, fileSize);
    csvFile.close();

    String sanitizedFileName = fileName;
    if (sanitizedFileName.startsWith("/")) {
        sanitizedFileName = sanitizedFileName.substring(1);
    }

    http.begin(url);
    http.addHeader("Content-Type", "text/csv");
    http.addHeader("X-File-Name", sanitizedFileName);

    int httpResponseCode = http.POST(buffer, fileSize);
    free(buffer);

    if (httpResponseCode > 0) {
        String response = http.getString();
        M5.Lcd.setCursor(0, 140);
        M5.Lcd.println("Response: " + response);
    } else {
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
        M5.Lcd.setCursor(0, 140);
        M5.Lcd.printf("POST failed, error: %d", httpResponseCode);
    }

    http.end();
}

void NoiseDetector::notificationAWS() {
    HTTPClient http;
    String url = String(BASE_URL) + "notification";
    http.begin(url);
    // POSTリクエストを送信
    int httpCode = http.POST("notification");
    if (httpCode > 0) {
        String response = http.getString();
        M5.Lcd.setCursor(0, 120);
        M5.Lcd.println("Notification:");
        M5.Lcd.println(response);
    } else {
        M5.Lcd.setCursor(0, 120);
        M5.Lcd.println("HTTP POST failed, error: " + String(httpCode));
    }
}

void NoiseDetector::storeNoise() {
    if (isRequestSpeaker) {
        speakerHandler.playTone(440, 100);
        isRequestSpeaker = false;
    }

    if (isDataStored) {
        notificationAWS();
        logNoiseTimestamp();
        isDataStored = false;
        M5.Lcd.setCursor(0, 160);
        M5.Lcd.println("NOISE STORED");
        restartTimer();
    }
}

void IRAM_ATTR onTimer() {
    micValue = analogRead(36);
    noiseDetector.updateBuffer(micValue);
}

void NoiseDetector::startTimer() {
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("NOISE DETECTING");
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, TIME_IRQ, true);
    timerAlarmEnable(timer);
}

void NoiseDetector::restartTimer() {
    M5.Lcd.setCursor(0, 180);
    M5.Lcd.println("Restart Timer");
    delay(1000);
    M5.Lcd.fillScreen(TFT_BLACK);
    delay(100);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("NOISE DETECTING");
    timerRestart(timer);
    timerStart(timer);
    delay(100);
}

