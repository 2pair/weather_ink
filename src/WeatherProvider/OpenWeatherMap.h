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
        static constexpr uint32_t monthlyApiCallLimit = 1000;

        using WeatherProvider::WeatherProvider;

        std::string getCurrentWeatherUrl() const override;

        std::string getForecastedWeatherUrl() const override;

        void toCurrentWeather(
            weather::DailyWeather& currentWeather,
            JsonDocument& currentApiResponse) const override;

        uint8_t toForecastedWeather(
            weather::daily_forecast& forecastedWeather,
            JsonDocument& forecastApiResponse) const override;

    protected:
        // https://openweathermap.org/weather-conditions
        void codeToConditions(
            weather::DailyWeather& dailyWeather,
            const uint16_t code) const override;

        static const std::string cBaseUrl;
};

}
