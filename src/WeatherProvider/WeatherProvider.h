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

        virtual size_t getWeatherUpdateIntervalSeconds() const = 0;

        virtual size_t getForecastUpdateIntervalSeconds() const = 0;

        virtual std::string getCurrentWeatherUrl() const = 0;

        virtual  std::string getForecastedWeatherUrl() const = 0;

        virtual void toCurrentWeather(
            weather::DailyWeather& currentWeather,
            JsonDocument& currentApiResponse) const = 0;

        virtual uint8_t toForecastedWeather(
            weather::daily_forecast& forecastedWeather,
            JsonDocument& forecastApiResponse) const = 0;

        virtual uint8_t toHourlyWeather(
            weather::hourly_forecast& forecastedWeather,
            JsonDocument& forecastApiResponse) const = 0;

        virtual std::string getFileSystemDirectory() const = 0;

    protected:
        // Normalize condition codes to internal representation
        virtual weather::Condition codeToConditions(const uint16_t code) const = 0;

        const float mLatitude;
        const float mLongitude;
        const std::string mCity;
        const std::string mApiKey;

        static const std::string cFsDirectory;

};

}