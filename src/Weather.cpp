#include "Weather.h"

#include <Inkplate.h>
#include <ArduinoJson.h>
#include <esp32-hal-log.h>

#include "Network.h"
#include "DailyWeather.h"
#include "WeatherProvider/WeatherProvider.h"
#include "SdCard.h"
#include "TimeUtils.h"


using namespace weather;

Weather::Weather(Inkplate& display)
    :   mDisplay(display),
        mFakeUpdates(false)
{
    // Don't initialize the weather structs to preserve info from previous update
}

bool Weather::updateCurrent(network::Network& connection, const weatherprovider::WeatherProvider& provider)
{
    time_t updatedTime;
    if (mFakeUpdates)
    {
        sdcard::SdCard card(mDisplay);
        updatedTime = provider.getCurrentWeather(mForecast[0], card);
    }
    else
    {
        updatedTime = provider.getCurrentWeather(mForecast[0], connection);
    }
    return updatedTime != 0;
}

bool Weather::updateHourly(network::Network& connection, const weatherprovider::WeatherProvider& provider)
{
    time_t updatedTime;
    if (mFakeUpdates)
    {
        sdcard::SdCard card(mDisplay);
        updatedTime = provider.getHourlyWeather(mHourlyForecast, card);
    }
    else
    {
        updatedTime = provider.getHourlyWeather(mHourlyForecast, connection);
    }
    return updatedTime != 0;
}

bool Weather::updateForecast(network::Network& connection, const weatherprovider::WeatherProvider& provider)
{
    time_t updatedTime;
    if (mFakeUpdates)
    {
        sdcard::SdCard card(mDisplay);
        updatedTime = provider.getForecastedWeather(mForecast, card);
    }
    else
    {
        updatedTime = provider.getForecastedWeather(mForecast, connection);
    }
    if (updatedTime)
    {
        mLastForecastTime = updatedTime;
    }
    return updatedTime != 0;
}

bool Weather::updateWeather(network::Network& connection, const weatherprovider::WeatherProvider& provider)
{
    auto currentSuccess = updateCurrent(connection, provider);
    auto hourlySuccess = updateHourly(connection, provider);
    auto forecastSuccess = updateForecast(connection, provider);

    return (currentSuccess && hourlySuccess && forecastSuccess);
}

const DailyWeather& Weather::getDailyWeather(const uint8_t offset) const
{
    return mForecast[offset];
}

const hourly_forecast& Weather::getHourlyWeather() const
{
    return mHourlyForecast;
}

void Weather::printDailyWeather(const DailyWeather& dailyWeather)
{
    log_i("--------- Daily Weather ---------");
    log_i("Data for day at timestamp: %d (tz %d)", dailyWeather.timestamp, dailyWeather.timeZone);

    log_i(
        "Current Temp:  %.2f (feels like %.2f)",
        dailyWeather.tempNow,
        dailyWeather.feelsLike
    );
    log_i(
        "Daily Low:  %.2f  Daily High:  %.2f",
        dailyWeather.tempLow,
        dailyWeather.tempHigh
    );

    log_i(
        "Condition: %s, (%.2f in.) (%u %%)",
        weather::conditionToString(dailyWeather.condition).c_str(),
        dailyWeather.precipitation,
        static_cast<uint>(dailyWeather.chanceOfPrecipitation * 100)
    );

    log_i(
        "H: %.2f  P:  %.2f  V:  %.2f",
        dailyWeather.humidity,
        dailyWeather.pressure,
        dailyWeather.visibility
    );

    log_i(
        "Wind: %.2f  Gust:  %.2f  Deg:  %.2f",
        dailyWeather.windSpeed,
        dailyWeather.gustSpeed,
        dailyWeather.windDirection
    );

    log_i(
        "Sunrise: %d  Sunset:  %d",
        dailyWeather.sunrise,
        dailyWeather.sunset
    );
    log_i("---------------------------------");
}

void Weather::printHourlyWeather(const HourlyWeather& hourlyWeather)
{
    log_i("--------- Hourly Weather ---------");
    log_i("Data for hour at: %d (tz %d)", hourlyWeather.timestamp, hourlyWeather.timeZone);

    log_i(
        "Current Temp:  %.2f (feels like %.2f)",
        hourlyWeather.tempNow,
        hourlyWeather.feelsLike
    );

    log_i(
        "Condition: %s, (%.2f in.) (%u %%)",
        weather::conditionToString(hourlyWeather.condition).c_str(),
        hourlyWeather.precipitation,
        static_cast<uint>(hourlyWeather.chanceOfPrecipitation * 100)
    );

    log_i(
        "H: %.2f  P:  %.2f",
        hourlyWeather.humidity,
        hourlyWeather.pressure
    );

    log_i(
        "Wind: %.2f  Deg:  %.2f",
        hourlyWeather.windSpeed,
        hourlyWeather.windDirection
    );
    log_i("---------------------------------");
}

time_t Weather::getLastForecastTime() const
{
    return mLastForecastTime;
}

size_t Weather::getDailyForecastLength() const
{
    return mForecast.size();
}

size_t Weather::getHourlyForecastLength() const
{
    return mHourlyForecast.size();
}
