#pragma once

#include <stdint.h>

#include <ArduinoJson.h>

#include "DailyWeather.h"
#include "Network.h"


namespace weatherprovider {
    class OpenWeatherMap;
}

namespace weather
{

class Weather
{
    public:
        Weather(weatherprovider::OpenWeatherMap& provider);

        bool updateCurrent(JsonDocument& apiResponse, network::Network& connection);

        bool updateForecast(JsonDocument& apiResponse, network::Network& connection);

        const DailyWeather& getDailyWeather(const uint8_t index) const;

        static constexpr uint8_t cForecastDays = 5;

    private:
        weatherprovider::OpenWeatherMap& mProvider;
        DailyWeather mForecast[cForecastDays];
};

const char* conditionToString(const Condition condition);
}
