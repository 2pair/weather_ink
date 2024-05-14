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

        void getCurrentWeatherUrl(char * buffer, uint8_t bufferLen) override;

        void getForecastedWeatherUrl(char * buffer, uint8_t bufferLen) override;

        void toCurrentWeather(
            weather::DailyWeather& currentWeather,
            JsonDocument& currentApiResponse) override;

        uint8_t toForecastedWeather(
            weather::DailyWeather* forecastedWeather,
            const uint8_t maxDays,
            JsonDocument& forecastApiResponse) override;

    protected:
        // https://openweathermap.org/weather-conditions
        void codeToConditions(
            weather::DailyWeather& dailyWeather,
            const uint16_t code) override;
};

}
