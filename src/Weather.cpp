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
{}

bool Weather::updateCurrent(network::Network& connection, const weatherprovider::WeatherProvider& provider)
{
    JsonDocument apiResponse;
    sdcard::SdCard card(mDisplay);
    bool success = false;
    if (mFakeUpdates)
    {
        log_i("Reading file data to simulate current conditions API response");
        success = card.getFakeWeatherData(
            apiResponse,
            provider.getFileSystemDirectory() + (const char *)"forecast.json"
        );
    }
    else
    {
        auto url = provider.getCurrentWeatherUrl();
        log_d("URL returned: %s", url.c_str());
        success = connection.getApiResponse(apiResponse, url);
    }

    if (success) {
        log_d("Converting JSON to current weather data");
        provider.toCurrentWeather(mForecast[0], apiResponse);
        provider.toHourlyWeather(mHourlyForecast, apiResponse); //TODO is this portable?
    }
    else
    {
        log_w("Failed to get API response");
    }
    apiResponse.clear();
    return success;
}

bool Weather::updateForecast(network::Network& connection, const weatherprovider::WeatherProvider& provider)
{
    JsonDocument apiResponse;
    sdcard::SdCard card(mDisplay);
    bool success = false;
    if (mFakeUpdates)
    {
        log_i("Reading file data to simulate forecasted conditions API response");
        success = card.getFakeWeatherData(
            apiResponse,
            provider.getFileSystemDirectory() + (const char *)"forecast.json"
        );

    }
    else
    {
        auto url = provider.getForecastedWeatherUrl();
        log_d("URL returned: %s", url.c_str());
        success = connection.getApiResponse(apiResponse, url);
    }

    if (success)
    {
        log_d("Converting JSON to forecast weather data");
        auto daysForecast = provider.toForecastedWeather(mForecast, apiResponse);
        mLastForecastTime = time(nullptr);
    }
    else
    {
        log_w("Failed to get API response");
    }
    apiResponse.clear();
    return success;
}

bool Weather::updateWeather(network::Network& connection, const weatherprovider::WeatherProvider& provider)
{
    JsonDocument apiResponse;
    sdcard::SdCard card(mDisplay);
    bool success = false;
    if (mFakeUpdates)
    {
        log_i("Reading file data to simulate forecasted conditions API response");
        success = card.getFakeWeatherData(
            apiResponse,
            provider.getFileSystemDirectory() + (const char *)"forecast.json"
        );
    }
    else
    {
        auto url = provider.getForecastedWeatherUrl();
        log_d("URL returned: %s", url.c_str());
        success = connection.getApiResponse(apiResponse, url);
    }

    if (success)
    {
        log_d("Converting JSON to current weather data");
        provider.toCurrentWeather(mForecast[0], apiResponse);
        provider.toHourlyWeather(mHourlyForecast, apiResponse);

        log_d("Converting JSON to forecast weather data");
        provider.toForecastedWeather(mForecast, apiResponse);
        mLastForecastTime = time(nullptr);
    }
    else
    {
        log_w("Failed to get API response");
    }
    apiResponse.clear();
    return success;
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
        "Daily High:  %.2f  Daily Low:  %.2f",
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
        hourlyWeather.temp,
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
