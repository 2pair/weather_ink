#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include <ArduinoJson.h>

#include "../WeatherTypes.h"
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
            const std::string& city,
            const std::string& apiKey)
            :   mLatitude(latitude),
                mLongitude(longitude),
                mCity(city),
                mApiKey(apiKey)
            {}

        virtual std::string getCurrentWeatherUrl() const = 0;

        virtual  std::string getForecastedWeatherUrl() const = 0;

        virtual void toCurrentWeather(
            weather::DailyWeather& currentWeather,
            JsonDocument& currentApiResponse) const = 0;

        virtual uint8_t toForecastedWeather(
            weather::daily_forecast& forecastedWeather,
            JsonDocument& forecastApiResponse) const = 0;

    protected:
        // Normalize condition codes to internal representation
        virtual void codeToConditions(
            weather::DailyWeather& dailyWeather,
            const uint16_t code) const = 0;

        const float mLatitude;
        const float mLongitude;
        const std::string mCity;
        const std::string mApiKey;
};

}
