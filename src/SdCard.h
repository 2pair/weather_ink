#pragma once

#include <Inkplate.h>
#include <ArduinoJson.h>


namespace sdcard {

class SdCard
{
    public:
        SdCard(Inkplate& display);

        ~SdCard();

        bool openFile(const std::string& filePath);

        // Returns the file
        std::string findFileWithPrefix(const std::string& dirPath, const std::string& prefix);

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
