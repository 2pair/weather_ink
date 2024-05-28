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
        // https://www.weatherapi.com/docs/weather_conditions.json
        void codeToConditions(
            weather::DailyWeather& dailyWeather,
            const uint16_t code) const override;

        static const std::string cBaseUrl;
};

}
