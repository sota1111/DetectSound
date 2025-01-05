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
hw_timer_t *timer = NULL;
volatile int16_t adcAverageDetect = 0;

// 初期化
NoiseDetector::NoiseDetector() : isRequestSpeaker(false), isDataStored(false), isTimerStopped(false), write_index(0), detect_index(0) {
}

NoiseDetector::~NoiseDetector() {
    freeBuffer();
}

void NoiseDetector::freeBuffer() {
    if (val_buf) {
        delete[] val_buf;
        val_buf = nullptr;
        M5.Lcd.println("Buffer freed");
    }
}

// 1秒間のAD変換値の平均を取得する関数
void NoiseDetector::getADCAverage() {
    const unsigned long duration = 1000; // 1秒間 (ミリ秒単位)
    unsigned long startTime = millis();
    long sum = 0;
    int count = 0;

    while (millis() - startTime < duration) {
        int adcValue = analogRead(36);
        sum += adcValue;
        count++;
    }
    adcAverageDetect = (int16_t)(sum / count);
}

void NoiseDetector::initNoiseDetector() {
    freeBuffer();  // 以前のバッファがあれば解放
    val_buf = new int16_t[RECORD_MAX_LEN];  // バッファを動的に確保
    if (val_buf == nullptr) {
        M5.Lcd.println("Failed to allocate buffer");
        return;
    }
    for (int i = 0; i < RECORD_MAX_LEN; i++) {
        val_buf[i] = 0;
    }

    sdcardHandler.initSDCard(APARTMENT_NAME, ROOM_NAME);
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

bool NoiseDetector::detectNoise(int avgIntegral) {
    // 0 → 40dB, 100 → 60dB, 500 → 80dB になるように補完
    int dBValue;
    if (avgIntegral < 100) {
        // f(x) = 40 + ((80 - 40) / 200) * x = 40 + 0.2 * x
        dBValue = 40 + (2 * avgIntegral)/10;
    } else {
        // 60.0f + (80.0f - 60.0f) * ((float)x - 100.0f) / 400.0f;
        // y = 60 + 0.05 * (x - 100)
        dBValue = 60 + (avgIntegral - 100)/20;
    }

    if (dBValue > NOISE_ALERT_THRESHOLD_DB) {
        return true;
    }
    return false;
}

// ============================================================
//  移動積分を計算する関数
// ============================================================
int NoiseDetector::calculateMovingIntegral(int currentMicValue, int writeIndex)
{
    static long s_integralValue = 0;
    static int  s_sampleCount   = 0;

    int16_t adcVal = abs(currentMicValue - adcAverageDetect);
    s_integralValue += adcVal;

    if (s_sampleCount < INTEGRAL_SAMPLES_DETECT) {
        s_sampleCount++;
    } else {
        int prevIndex       = writeIndex - INTEGRAL_SAMPLES_DETECT;
        unsigned int oldPos = (prevIndex + RECORD_MAX_LEN) % RECORD_MAX_LEN;
        s_integralValue    -= abs(val_buf[oldPos] - adcAverageDetect);
    }

    int avgIntegral = s_integralValue / s_sampleCount;
    return avgIntegral;
}

// ============================================================
//  updateBuffer から移動積分の計算処理を関数呼び出しに置き換え
// ============================================================
void NoiseDetector::updateBuffer(int micValue) {
    static unsigned int detect_count = 0;
    static bool isNoiseDetected      = false;
    static unsigned int startTime    = 0;

    if (!isDataStored) {
        // ノイズ検出前後のデータを記録        
        write_index = (write_index + 1) % RECORD_MAX_LEN;
        val_buf[write_index] = micValue;

        int avgIntegral = calculateMovingIntegral(micValue, write_index);

        // ノイズ検出
        if ((detectNoise(avgIntegral)) && (isNoiseDetected == false)) {
            isNoiseDetected = true;
            detect_index = write_index;
            M5.Lcd.println("NOISE DETECTED");
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
            M5.Lcd.println("BUFFER FULL");
            M5.Lcd.println("STOP TIMER");
            int stopTime = millis() - startTime;
            M5.Lcd.printf("Time: %d\n", stopTime);     
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
        String dirName = "/" + String(APARTMENT_NAME) + "/" + String(ROOM_NAME);
        snprintf(fileName, sizeof(fileName), "%s/log_%s.csv", dirName.c_str(), timestamp);
        File csvFile = SD.open(fileName, FILE_APPEND);
        csvFile.print(csvData.c_str());
        csvFile.close();
        M5.Lcd.println("STORE DATA");

        // POST処理を追加
        postCSVtoServer(fileName);
    } else {
        // 現在時刻が取得できなかった場合のエラーメッセージ
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
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
        M5.Lcd.println("Failed to open CSV file.");
        return;
    }

    size_t fileSize = csvFile.size();
    uint8_t* buffer = (uint8_t*)malloc(fileSize);
    if (buffer == NULL) {
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
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
        M5.Lcd.println("Response: " + response);
    } else {
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
        M5.Lcd.printf("POST failed, error: %d\n", httpResponseCode);
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
        M5.Lcd.println("Notification:" + response);
    } else {
        M5.Lcd.println("HTTP POST failed, error: " + String(httpCode));
    }
}

void NoiseDetector::storeNoise() {
    if (isRequestSpeaker) {
        speakerHandler.playTone(440, 100);
        isRequestSpeaker = false;
    }

    if (isDataStored) {
        //notificationAWS();
        logNoiseTimestamp();
        isDataStored = false;
        M5.Lcd.println("NOISE STORED");
        restartTimer();
    }
}

void IRAM_ATTR onTimer() {
    micValue = analogRead(36);
    noiseDetector.updateBuffer(micValue);
}

void NoiseDetector::startTimer() {
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("NOISE DETECTING");
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, TIME_IRQ, true);
    timerAlarmEnable(timer);
}

void NoiseDetector::restartTimer() {
    M5.Lcd.println("Restart Timer");
    delay(1000);
    M5.Lcd.fillScreen(TFT_BLACK);
    delay(100);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("NOISE DETECTING");
    timerRestart(timer);
    timerStart(timer);
    delay(100);
}

