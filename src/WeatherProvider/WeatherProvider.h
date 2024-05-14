#pragma once

#include <stdint.h>
#include <string>

#include <ArduinoJson.h>

#include "../DailyWeather.h"

namespace weatherprovider {

class WeatherProvider
{
    public:
        // Derived classes should redefine this.
        static constexpr uint32_t monthlyApiCallLimit = 0;

        WeatherProvider() = delete;
        WeatherProvider(
            const float latitude,
            const float longitude,
            const char* city,
            const char* apiKey)
            :   mLatitude(latitude),
                mLongitude(longitude),
                mCity(city),
                mApiKey(apiKey)
            {}

        virtual void getCurrentWeatherUrl(char * buffer, uint8_t bufferLen) = 0;

        virtual  void getForecastedWeatherUrl(char * buffer, uint8_t bufferLen) = 0;

        virtual void toCurrentWeather(
            weather::DailyWeather& currentWeather,
            JsonDocument& currentApiResponse) = 0;

        virtual uint8_t toForecastedWeather(
            weather::DailyWeather* forecastedWeather,
            const uint8_t maxDays,
            JsonDocument& forecastApiResponse) = 0;

    protected:
        // Normalize condition codes to internal representation
        virtual void codeToConditions(
            weather::DailyWeather& dailyWeather,
            const uint16_t code) = 0;

        const float mLatitude;
        const float mLongitude;
        const char* mCity;
        const char* mApiKey;
};

}
