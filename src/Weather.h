#pragma once

#include <stdint.h>
#include <memory>

#include <Inkplate.h>
#include <ArduinoJson.h>

#include "WeatherTypes.h"
#include "WeatherProvider/WeatherProvider.h"
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
        Weather(Inkplate& display);

        bool updateCurrent(
            network::Network& connection,
            const weatherprovider::WeatherProvider& provider
        );

        bool updateForecast(
            network::Network& connection,
            const weatherprovider::WeatherProvider& provider
        );

        // returns a const reference to the weather data for the day
        // 'offset' days from todays's weather
        const DailyWeather& getDailyWeather(const uint8_t offset) const;

        const hourly_forecast& getHourlyWeather() const;

        // Print's the days weather to the serial console
        void printDailyWeather(const DailyWeather& dailyWeather);

        time_t getLastForecastTime();

    private:
        daily_forecast mForecast;
        hourly_forecast mHourlyWeather;
        Inkplate& mDisplay;
        time_t mLastForecastTime;
};

}
