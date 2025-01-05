#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include <ArduinoJson.h>

#include "../WeatherTypes.h"
#include "../DailyWeather.h"


namespace network {
    class Network;
}
namespace sdcard {
    class SdCard;
}
namespace environment {
    struct Location;
}

namespace weatherprovider {

class WeatherProvider
{
    public:
        WeatherProvider() = delete;
        WeatherProvider(
            const environment::Location& location,
            const std::string& apiKey,
            const bool metricUnits);

        /*  overloads of the to*Weather functions are provided to facilitate loading from
            an API or a file.
        */
        virtual time_t getCurrentWeather(
            weather::DailyWeather& currentWeather,
            network::Network& connection) const;

        virtual time_t getCurrentWeather(
            weather::DailyWeather& currentWeather,
            sdcard::SdCard& sdCard) const;

        virtual time_t getForecastedWeather(
            weather::daily_forecast& forecastedWeather,
            network::Network& connection) const;

        virtual time_t getForecastedWeather(
            weather::daily_forecast& forecastedWeather,
            sdcard::SdCard& sdCard) const;

        virtual time_t getHourlyWeather(
            weather::hourly_forecast& forecastedWeather,
            network::Network& connection) const;

        virtual time_t getHourlyWeather(
            weather::hourly_forecast& forecastedWeather,
            sdcard::SdCard& sdCard) const;

        virtual size_t getWeatherUpdateIntervalSeconds() const = 0;

        virtual size_t getForecastUpdateIntervalSeconds() const = 0;

        virtual std::string getCurrentWeatherUrl() const = 0;

        virtual std::string getHourlyWeatherUrl() const = 0;

        virtual  std::string getForecastedWeatherUrl() const = 0;

        virtual std::string getFileSystemDirectory() const = 0;

    protected:
        // Normalize condition codes to internal representation
        virtual weather::Condition codeToConditions(const uint16_t code) const = 0;

        virtual void toCurrentWeather(
            weather::DailyWeather& currentWeather,
            const JsonDocument& forecastApiResponse) const = 0;

        virtual uint8_t toForecastedWeather(
            weather::daily_forecast& forecastedWeather,
            const JsonDocument& forecastApiResponse) const = 0;

        virtual uint8_t toHourlyWeather(
            weather::hourly_forecast& forecastedWeather,
            const JsonDocument& forecastApiResponse) const = 0;


        const float mLatitude;
        const float mLongitude;
        const std::string mCity;
        const std::string mApiKey;
        const bool mMetricUnits;

        // Directory on filesystem where this providers API samples are stored
        static const std::string cFsDirectory;

};

}
