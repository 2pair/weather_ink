#pragma once

#include <stdint.h>

#include <ArduinoJson.h>

#include "WeatherProvider.h"
#include "../DailyWeather.h"


namespace weatherprovider
{

class OpenWeatherMap : public WeatherProvider
{
    public:
        static constexpr uint32_t monthlyApiCallLimit = 1000;

        using WeatherProvider::WeatherProvider;

        std::string getCurrentWeatherUrl() override;

        std::string getForecastedWeatherUrl() override;

        void toCurrentWeather(
            weather::DailyWeather& currentWeather,
            JsonDocument& currentApiResponse) override;

        uint8_t toForecastedWeather(
            std::vector<weather::DailyWeather>& forecastedWeather,
            JsonDocument& forecastApiResponse) override;

    protected:
        // https://openweathermap.org/weather-conditions
        void codeToConditions(
            weather::DailyWeather& dailyWeather,
            const uint16_t code) override;
};

}
