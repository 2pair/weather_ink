#include "SdCard.h"

#include <Inkplate.h>
#include <ArduinoJson.h>


using namespace sdcard;


SdCard::SdCard(Inkplate& display)
    :   mDisplay(display),
        mInitialized(false),
        mFileOpen(false),
        mFileSystem(display.getSdFat())
{
    mInitialized = static_cast<bool>(display.sdCardInit());
    if (!mInitialized)
    {
        Serial.println(F("WARNING: SD Card Initialization Failure"));
    }
}

SdCard::~SdCard()
{
    if (mFileOpen)
    {
        mFile.close();
    }
    sleep(mDisplay);
}

void SdCard::sleep(Inkplate& display)
{
    display.getSPIptr()->end();
    display.sdCardSleep();
}

std::string SdCard::findFileWithPrefix(const std::string& dirPath, const std::string& prefix)
{
    std::string foundName;
    auto pathComponents = getPathComponents(dirPath);
    std::string path = "/";
    for (auto& component : pathComponents)
    {
        path += component;
        auto exists = mFileSystem.exists(path.c_str());
        if (!exists)
        {
            Serial.println("File's path does not exist");
            return foundName;
        }
    }
    // Directory exists
    mFileSystem.chdir(path.c_str());
    File dir, file;
    dir.open(path.c_str(), O_RDONLY);
    while(file.openNext(&dir, O_RDONLY))
    {
        std::string candidateFilename(file.name());
        if (candidateFilename.rfind(prefix) != std::string::npos)
        {
            foundName = candidateFilename;
            break;
        }
    }
    file.close();
    dir.close();
    return foundName;
}

bool SdCard::openFile(const std::string& filePath)
{
    if (!mInitialized)
    {
        Serial.println("Cannot read file because the SD Card could not be initialized");
        return false;
    }
    if (mFileOpen)
    {
        Serial.printf("Closing open file before opening %s\n", filePath.c_str());
        mFile.close();
    }

    Serial.printf("Opening file %s\n", filePath.c_str());
    auto sd = mDisplay.getSdFat();
    if (!sd.exists(filePath.c_str()))
    {
        Serial.println("File does not exist");
        return false;
    }

    if (!mFile.open(filePath.c_str(), O_RDONLY))
    {
        Serial.println("File open error");
        return false;
    }
    mFileOpen = true;
    return true;
}

bool SdCard::readJsonFile(JsonDocument& jsonDocument, const std::string& filePath)
{
    bool success = openFile(filePath);
    if (!success) {
        return false;
    }
    auto jError = deserializeJson(jsonDocument, mFile);
    if (jError != DeserializationError::Ok)
    {
        Serial.printf("Failed to load %s from SD card with error %d\n", filePath, jError);
        return false;
    }
    Serial.println("content deserialized to json object");
    return true;
}


bool SdCard::getFakeCurrentData(JsonDocument& apiResponse)
{
    return readJsonFile(apiResponse, "/current.json");
}

bool SdCard::getFakeForecastData(JsonDocument& apiResponse)
{
    return readJsonFile(apiResponse, "/daily.json");
}

std::vector<std::string> getPathComponents(const std::string& path)
{
    std::string component;
    std::vector<std::string> components;
    for (auto &character : path) {
        if (character == '/' || character == '\\') {
            if (component.size() == 0)
            {
                continue;
            }
            components.emplace_back(component);
            component.clear();
        }
    }
    if (component.size() != 0)
    {
        components.emplace_back(component);
    }
    return components;
}
