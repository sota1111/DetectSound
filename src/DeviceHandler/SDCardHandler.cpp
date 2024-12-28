#include "SDCardHandler.h"

void SDCardHandler::initSDCard(const char* dirname) {
    if (!SD.begin(TFCARD_CS_PIN, SPI, 40000000)) {
        M5.Lcd.println("SD Card Mount Failed");
        delay(1000);
        return;
    }
    M5.Lcd.println("SD Card Mount Success");
    delay(1000);
    if (!SD.exists(dirname)) {
        SD.mkdir(dirname);
    }
}

// SDカード書き込み
bool SDCardHandler::writeSDCard(const char* filename, const char* data) {
    File file = SD.open(filename, FILE_APPEND);
    if (!file) {
        M5.Lcd.println("Failed to open file for writing");
        delay(1000);
        return false;  // 書き込み失敗
    }
    file.println(data);
    file.close();
    return true;  // 書き込み成功
}