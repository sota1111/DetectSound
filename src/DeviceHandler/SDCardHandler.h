#ifndef SDCARDHANDLER_H
#define SDCARDHANDLER_H

#include <M5Stack.h>
#include <SD.h>
#include <time.h>

class SDCardHandler {
public:
    void initSDCard(const char* apartmentName, const char* roomName);
    bool writeSDCard(const char* filename, const char* data);
};

#endif