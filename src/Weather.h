#pragma once

#include <stdint.h>
#include <memory>

#include <ArduinoJson.h>

#include "WeatherTypes.h"


class Inkplate;
namespace weatherprovider {
    class WeatherProvider;
}
namespace weather {
    class DailyWeather;
}
namespace network {
    class Network;
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

        bool updateWeather(
            network::Network& connection,
            const weatherprovider::WeatherProvider& provider
        );

        // returns a const reference to the weather data for the day
        // 'offset' days from todays's weather
        const DailyWeather& getDailyWeather(const uint8_t offset) const;

        const hourly_forecast& getHourlyWeather() const;

        // Print's the days weather to the serial console
        void printDailyWeather(const DailyWeather& dailyWeather) const;

        // Print's the hours weather to the serial console
        void printHourlyWeather(const HourlyWeather& hourlyWeather) const;

        time_t getLastForecastTime() const;

        // How many days of forecast we have, including the current day
        size_t getDailyForecastLength() const;

        // How many hours of forecast we have
        size_t getHourlyForecastLength() const;

        // if updates should be faked
        inline void fakeUpdates(bool fakeUpdates) { mFakeUpdates = fakeUpdates; }

    private:
        daily_forecast mForecast;
        hourly_forecast mHourlyForecast;
        Inkplate& mDisplay;
        time_t mLastForecastTime;
        bool mFakeUpdates;
};

}
