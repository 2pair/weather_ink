#include "SdCard.h"

#include <string>
#include <vector>

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
    bool success;
    std::string foundName;
    std::string path;
    if (*dirPath.begin() != '/') {
        path += '/';
    }
    path += dirPath;

    if (!mFileSystem.exists(path.c_str()))
    {
        Serial.printf((const char*)F("\nCould not find file. File's path (%s) does not exist\n"), path.c_str());
        return foundName;
    }
    if (!mFileSystem.chdir(path.c_str()))
    {
        Serial.printf((const char*)F("Failed to open directory %s\n"), path.c_str());
        return foundName;
    }

    File dir, file;
    success = dir.open(path.c_str(), O_RDONLY);
    if (!success)
    {
        Serial.printf((const char*)F("Failed to attach fh directory %s\n"), path.c_str());
        return foundName;
    }
    auto pixelSize = getPathComponents(dirPath).back();
    // prefix length + _ + pixels + . + extension + \0 (assumes ext max of 4 char)
    // getName requires at least 13 bytes.
    size_t iconNameLength = prefix.size() + 1 + pixelSize.size() + 6;
    size_t bufLen = std::max(iconNameLength, 13U);
    std::string baseName = prefix + "_" + pixelSize;
    while(file.openNext(&dir, O_RDONLY))
    {
        std::string candidateFilename(bufLen, '\0');
        if (
            file.getName(&candidateFilename[0], candidateFilename.size()) &&
            candidateFilename.rfind(prefix, 0) != std::string::npos
        )
        {
            foundName = candidateFilename;
        }

        file.close();
        if (!foundName.empty())
        {
            break;
        }
    }
    dir.close();
    return foundName;
}

bool SdCard::openFile(const std::string& filePath)
{
    if (!mInitialized)
    {
        Serial.println(F("Cannot read file because the SD Card could not be initialized"));
        return false;
    }
    if (mFileOpen)
    {
        Serial.printf((const char *)F("Closing open file before opening %s\n"), filePath.c_str());
        if (!mFile.close())
        {
            Serial.println(F("Failed to close file!"));
        }
    }

    Serial.printf((const char *)F("Opening file %s\n"), filePath.c_str());
    if (!mFileSystem.exists(filePath.c_str()))
    {
        Serial.println(F("File does not exist"));
        return false;
    }

    if (!mFile.open(filePath.c_str(), O_RDONLY))
    {
        Serial.println(F("File open error"));
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
        Serial.printf((const char *)F("Failed to load %s from SD card with error %d\n"), filePath, jError);
        return false;
    }
    Serial.println(F("content deserialized to json object"));
    return true;
}


bool SdCard::getFakeCurrentData(JsonDocument& apiResponse)
{
    return readJsonFile(apiResponse, (const char *)F("/current.json"));
}

bool SdCard::getFakeForecastData(JsonDocument& apiResponse)
{
    return readJsonFile(apiResponse, (const char *)F("/daily.json"));
}

std::vector<std::string> sdcard::getPathComponents(const std::string& path)
{
    std::vector<std::string> components;
    components.emplace_back();
    for (auto &character : path) {
        if (character == '/' || character == '\\') {
            components.emplace_back();
            continue;
        }
        components.back() += character;
    }

    // remove empty components (ie: if the path had // in it)
    auto iter = components.begin();
    while (iter != components.end())
    {
        if (iter->empty())
        {
            components.erase(iter);
        }
        ++iter;
    }
    return components;
}
