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

        bool updateCurrent(network::Network& connection);

        bool updateForecast(network::Network& connection);

        const DailyWeather& getDailyWeather(const uint8_t index) const;

        void printDailyWeather(const DailyWeather& dailyWeather);

        // Including the current day's forecast
        static constexpr uint8_t cForecastDays = 5;

    private:
        weatherprovider::OpenWeatherMap& mProvider;
        std::vector<DailyWeather> mForecast;
};

}
