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

        std::string getHourlyWeatherUrl() const override;

        std::string getForecastedWeatherUrl() const override;

        /*  This needs to be overriden because the API only lets us get 3 days of forecast
            data, but also only lets us get a single day of forecast data if given a
            date offset.
        */
        time_t toForecastedWeather(
            weather::daily_forecast& forecastedWeather,
            network::Network& connection) const override;

        std::string getFileSystemDirectory() const override;

    protected:
        // https://www.weatherapi.com/docs/weather_conditions.json
        weather::Condition codeToConditions(const uint16_t code) const override;

        void toCurrentWeather(
            weather::DailyWeather& currentWeather,
            const JsonDocument& currentApiResponse) const override;

        /*  Populate data for a single day. Unless otherwise specified, it is assumed that
            the forecast data at index 0 is the coorsponding data.
        */
        void toForecastedWeather(
            weather::DailyWeather& dailyWeather,
            const JsonDocument& forecastApiResponse,
            const size_t index = 0) const;

        uint8_t toForecastedWeather(
            weather::daily_forecast& forecastedWeather,
            const JsonDocument& forecastApiResponse) const override;

        uint8_t toHourlyWeather(
            weather::hourly_forecast& forecastedWeather,
            const JsonDocument& forecastApiResponse) const override;

        // Get 'days' number of forecasted days, at 'offset' number of days in the future.
        // offset == 0 for today's forecast.
        std::string getForecastedWeatherUrl(const uint8_t days, const uint8_t offset) const;

        void setAstroData(
            weather::DailyWeather& dailyWeather,
            const JsonObjectConst& astroData,
            const std::string& dateTime) const;

        static const std::string cBaseUrl;

        static constexpr size_t cWeatherUpdateIntervalSeconds = 900;
        static constexpr size_t cForecastUpdateIntervalSeconds = 28800;
        static const std::string cFsDirectory;
};

}
