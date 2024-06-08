#pragma once

#include <stdint.h>

#include <ArduinoJson.h>

#include "WeatherProvider.h"
#include "../WeatherTypes.h"
#include "../DailyWeather.h"


namespace weatherprovider
{

// Gets weather information from weatherapi.com
class WeatherApi : public WeatherProvider
{
    public:
        using WeatherProvider::WeatherProvider;

        size_t getWeatherUpdateIntervalSeconds() const override;

        size_t getForecastUpdateIntervalSeconds() const override;

        std::string getCurrentWeatherUrl() const override;

        std::string getForecastedWeatherUrl() const override;

        void toCurrentWeather(
            weather::DailyWeather& currentWeather,
            JsonDocument& currentApiResponse) const override;

        uint8_t toForecastedWeather(
            weather::daily_forecast& forecastedWeather,
            JsonDocument& forecastApiResponse) const override;

        uint8_t toHourlyWeather(
            weather::hourly_forecast& forecastedWeather,
            JsonDocument& forecastApiResponse) const override;

        std::string getFileSystemDirectory() const override;

    protected:
        // https://www.weatherapi.com/docs/weather_conditions.json
        weather::Condition codeToConditions(const uint16_t code) const override;

        static const std::string cBaseUrl;

        static constexpr size_t cWeatherUpdateIntervalSeconds = 900;
        static constexpr size_t cForecastUpdateIntervalSeconds = 28800;
        static const std::string cFsDirectory;
};

}
