#pragma once

#include <Inkplate.h>
#include <ArduinoJson.h>


namespace sdcard {

class SdCard
{
    public:
        SdCard(Inkplate& display);

        ~SdCard();

        void getJsonFile(JsonDocument& jsonDocument, const char* const fileName);

        void getFakeCurrentData(JsonDocument& apiResponse);

        void getFakeForecastData(JsonDocument& apiResponse);

        static void sleep(Inkplate& display);

    private:
        Inkplate& mDisplay;
        bool mInitialized;
};
}
