#pragma once

#include <stdint.h>

#include <ArduinoJson.h>

#include "WeatherProvider.h"
#include "../DailyWeather.h"


namespace weatherprovider
{

// Gets weather information from openweathermap.org
class OpenWeatherMap : public WeatherProvider
{
    public:
        using WeatherProvider::WeatherProvider;

        size_t getWeatherUpdateIntervalSeconds() const override;

        size_t getForecastUpdateIntervalSeconds() const override;

        std::string getCurrentWeatherUrl() const override;

        std::string getHourlyWeatherUrl() const override;

        std::string getForecastedWeatherUrl() const override;

        std::string getFileSystemDirectory() const override;

    protected:
        // https://openweathermap.org/weather-conditions
         weather::Condition codeToConditions(const uint16_t code) const override;

        void toCurrentWeather(
            weather::DailyWeather& currentWeather,
            const JsonDocument& currentApiResponse) const override;

        uint8_t toForecastedWeather(
            weather::daily_forecast& forecastedWeather,
            const JsonDocument& forecastApiResponse) const override;

        uint8_t toHourlyWeather(
            weather::hourly_forecast& forecastedWeather,
            const JsonDocument& forecastApiResponse) const override;

        static const std::string cBaseUrl;

        static constexpr size_t cWeatherUpdateIntervalSeconds = 7200;
        static constexpr size_t cForecastUpdateIntervalSeconds = 86400;
        static const std::string cFsDirectory;
};

}
