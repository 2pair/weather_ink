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
        log_e("SD Card Initialization Failure");
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
    std::string path;
    if (*dirPath.begin() != '/') {
        path += '/';
    }
    path += dirPath;

    if (!mFileSystem.exists(path.c_str()))
    {
        log_w("Could not find file. File's path (%s) does not exist", path.c_str());
        return std::string();
    }


    File dir;
    success = dir.open(path.c_str(), O_RDONLY);
    if (!success)
    {
        log_w("Failed to attach fh to directory %s", path.c_str());
        return std::string();
    }
    return findFileWithPrefix(dir, prefix);
}

std::string SdCard::findFileWithPrefix(File dir, const std::string& prefix)
{
    File file;
    std::array<char, 16> currentDirName;
    dir.getName(currentDirName.data(), currentDirName.size());
    auto pixelSize = currentDirName.data();
    // prefix length + _ + pixels + . + extension + \0 (assumes ext max of 4 char)
    size_t iconNameLength = prefix.size() + 1 + currentDirName.size() + 6;
    std::string baseName = prefix + "_" + pixelSize;
    std::string foundName;
    while(file.openNext(&dir, O_RDONLY))
    {
        std::string candidateFilename(iconNameLength, '\0');
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
    if (foundName.empty())
    {
        log_d("No file exists with prefix %s in dir %s", prefix.c_str(), currentDirName.data());
    }
    return foundName;
}

bool SdCard::fileExists(const std::string& filePath)
{
    if (!mInitialized)
    {
        log_w("Cannot read file because the SD Card could not be initialized");
        return false;
    }
    return mFileSystem.exists(filePath.c_str());
}


bool SdCard::openFile(const std::string& filePath)
{
    if (mFileOpen)
    {
        log_i("Closing open file before opening %s", filePath.c_str());
        if (!mFile.close())
        {
            log_w("Failed to close file!");
        }
    }

    log_i("Opening file %s", filePath.c_str());
    if (!fileExists(filePath))
    {
        log_w("File does not exist");
        return false;
    }

    if (!mFile.open(filePath.c_str(), O_RDONLY))
    {
        log_w("File open error");
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
        log_w(
            "Failed to load %s from SD card with error: %s",
            filePath.c_str(),
            jError.c_str()
        );
        return false;
    }
    log_i("content deserialized to json object");
    return true;
}


bool SdCard::getFakeWeatherData(JsonDocument& apiResponse, const std::string& path)
{
    return readJsonFile(apiResponse, path);
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
