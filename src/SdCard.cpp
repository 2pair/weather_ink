#include "SdCard.h"

#include <Inkplate.h>
#include <ArduinoJson.h>


using namespace sdcard;


SdCard::SdCard(Inkplate& display)
    :   mDisplay(display)
{
    mInitialized = static_cast<bool>(display.sdCardInit());
    if (!mInitialized)
    {
        Serial.println(F("WARNING: SD Card Initialization Failure"));
    }
}

SdCard::~SdCard()
{
    sleep(mDisplay);
}

void SdCard::sleep(Inkplate& display)
{
    display.getSPIptr()->end();
    display.sdCardSleep();
}

void SdCard::getJsonFile(JsonDocument& jsonDocument, const char* const fileName)
{
    if (!mInitialized)
    {
        Serial.println("Cannot read file because the SD Card could not be initialized");
        return;
    }

    SdFile file;
    Serial.printf("Opening file %s\n", fileName);
    auto sd = mDisplay.getSdFat();
    if (!sd.exists(fileName))
    {
        Serial.println("File does not exist");
    }
    else if (!file.open(fileName, O_RDONLY))
    {
        Serial.println("File open error");
    }
    else
    {
        size_t fileSizeBytes = file.fileSize();
        std::string content;
        content.reserve(fileSizeBytes);
        // null the buffer because file size might be a bit too large and I was seeing garbage at the end of the buffer.
        content.replace(0, fileSizeBytes, fileSizeBytes, 0);
        file.read(&content[0], fileSizeBytes);
        auto jError = deserializeJson(jsonDocument, content.c_str(), fileSizeBytes);
        if (jError != DeserializationError::Ok)
        {
            Serial.printf("Failed to load %s from SD card with error %d\n", fileName, jError);
        }
        else
        {
            Serial.println("content deserialized to json object");
        }
    }
    file.close();
}


void SdCard::getFakeCurrentData(JsonDocument& apiResponse)
{
    getJsonFile(apiResponse, "/current.json");
}

void SdCard::getFakeForecastData(JsonDocument& apiResponse)
{
    getJsonFile(apiResponse, "/daily.json");
}