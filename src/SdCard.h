#pragma once

#include <vector>

#include <ArduinoJson.h>
#include <SdFat.h>


class Inkplate;

namespace sdcard {

class SdCard
{
    public:
        SdCard(Inkplate& display);

        ~SdCard();

        bool openFile(const std::string& filePath);
        bool fileExists(const std::string& filePath);

        // Check in the given directory for a file named with the given prefix
        // Returns the name of the file if it's found.
        std::string findFileWithPrefix(const std::string& dirPath, const std::string& prefix);
        std::string findFileWithPrefix(File dir, const std::string& prefix);

        bool readJsonFile(JsonDocument& jsonDocument, const std::string& filePath);

        bool getFakeWeatherData(JsonDocument& apiResponse, const std::string& path);

        static void sleep(Inkplate& display);

        inline SdFat& getLib() {return mFileSystem;};

    private:
        Inkplate& mDisplay;
        bool mInitialized;
        File mFile;
        bool mFileOpen;
        SdFat& mFileSystem;
};

std::vector<std::string> getPathComponents(const std::string& path);

}
