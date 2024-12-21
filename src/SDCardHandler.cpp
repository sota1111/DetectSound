#include "SDCardHandler.h"

void SDCardHandler::initSDCard(const char* dirname) {
    if (!SD.begin(TFCARD_CS_PIN, SPI, 40000000)) {
        M5.Lcd.println("SD Card Mount Failed");
        return;
    }
    M5.Lcd.println("SD Card Mount Success");
    if (!SD.exists(dirname)) {
        SD.mkdir(dirname);
    }
}

// SDカード書き込み
void SDCardHandler::writeSDCard(const char* filename, const char* data) {
    File file = SD.open(filename, FILE_APPEND);
    if (!file) {
        M5.Lcd.println("Failed to open file for writing");
        return;
    }
    file.println(data);
    file.close();
}