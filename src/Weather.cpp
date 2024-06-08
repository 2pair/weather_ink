#include "Weather.h"

#include <Inkplate.h>
#include <ArduinoJson.h>

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
        Serial.println(F("Reading file data to simulate current conditions API response"));
        success = card.getFakeWeatherData(
            apiResponse,
            provider.getFileSystemDirectory() + (const char *)F("forecast.json")
        );
    }
    else
    {
        auto url = provider.getCurrentWeatherUrl();
        Serial.printf("URL returned: %s\n", url.c_str());
        success = connection.getApiResponse(apiResponse, url);
    }

    if (success) {
        Serial.println(F("Converting JSON to current weather data"));
        provider.toCurrentWeather(mForecast[0], apiResponse);
        provider.toHourlyWeather(mHourlyForecast, apiResponse); //TODO is this portable?
    }
    else
    {
        Serial.println("Failed to get API response");
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
        Serial.println(F("Reading file data to simulate forecasted conditions API response"));
        success = card.getFakeWeatherData(
            apiResponse,
            provider.getFileSystemDirectory() + (const char *)F("forecast.json")
        );

    }
    else
    {
        auto url = provider.getForecastedWeatherUrl();
        Serial.printf("URL returned: %s\n", url.c_str());
        success = connection.getApiResponse(apiResponse, url);
    }

    if (success)
    {
        Serial.println(F("Converting JSON to forecast weather data"));
        provider.toForecastedWeather(mForecast, apiResponse);
        mLastForecastTime = time(nullptr);
    }
    else
    {
        Serial.println("Failed to get API response");
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
        Serial.println(F("Reading file data to simulate forecasted conditions API response"));
        success = card.getFakeWeatherData(
            apiResponse,
            provider.getFileSystemDirectory() + (const char *)F("forecast.json")
        );
    }
    else
    {
        auto url = provider.getForecastedWeatherUrl();
        Serial.printf("URL returned: %s\n", url.c_str());
        success = connection.getApiResponse(apiResponse, url);
    }

    if (success)
    {
        Serial.println(F("Converting JSON to current weather data"));
        provider.toCurrentWeather(mForecast[0], apiResponse);
        provider.toHourlyWeather(mHourlyForecast, apiResponse);

        Serial.println(F("Converting JSON to forecast weather data"));
        provider.toForecastedWeather(mForecast, apiResponse);
        mLastForecastTime = time(nullptr);
    }
    else
    {
        Serial.println("Failed to get API response");
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

void Weather::printDailyWeather(const DailyWeather& dailyWeather) const
{
    Serial.println(F("--------- Daily Weather ---------"));
    Serial.printf((const char*)F("Data collected at: %llu\n"), dailyWeather.timestamp);

    Serial.printf(
        (const char*)F("Current Temp:  %.2f (feels like %.2f)\n"),
        dailyWeather.tempNow,
        dailyWeather.feelsLike
    );
    Serial.printf(
        (const char*)F("Daily High:  %.2f  Daily Low:  %.2f\n"),
        dailyWeather.tempLow,
        dailyWeather.tempHigh
    );

    Serial.printf(
        (const char*)F("Condition: %s, (%.2f in.) (%u %%)\n"),
        weather::conditionToString(dailyWeather.condition).c_str(),
        dailyWeather.precipitation,
        static_cast<uint>(dailyWeather.chanceOfPrecipitation * 100)
    );

    Serial.printf(
        (const char*)F("H: %.2f  P:  %.2f  V:  %.2f\n"),
        dailyWeather.humidity,
        dailyWeather.pressure,
        dailyWeather.visibility
    );

    Serial.printf(
        (const char*)F("Wind: %.2f  Gust:  %.2f  Deg:  %.2f\n"),
        dailyWeather.windSpeed,
        dailyWeather.gustSpeed,
        dailyWeather.windDirection
    );

    Serial.printf(
        (const char*)F("Sunrise: %llu  Sunset:  %llu\n"),
        dailyWeather.sunrise,
        dailyWeather.sunset
    );
    Serial.println(F("---------------------------------"));
}

void Weather::printHourlyWeather(const HourlyWeather& hourlyWeather) const
{
    Serial.println(F("--------- Hourly Weather ---------"));
    Serial.printf((const char*)F("Data collected at: %llu\n"), hourlyWeather.timestamp);

    Serial.printf(
        (const char*)F("Current Temp:  %.2f (feels like %.2f)\n"),
        hourlyWeather.temp,
        hourlyWeather.feelsLike
    );

    Serial.printf(
        (const char*)F("Condition: %s, (%.2f in.) (%u %%)\n"),
        weather::conditionToString(hourlyWeather.condition).c_str(),
        hourlyWeather.precipitation,
        static_cast<uint>(hourlyWeather.chanceOfPrecipitation * 100)
    );

    Serial.printf(
        (const char*)F("H: %.2f  P:  %.2f\n"),
        hourlyWeather.humidity,
        hourlyWeather.pressure
    );

    Serial.printf(
        (const char*)F("Wind: %.2f  Deg:  %.2f\n"),
        hourlyWeather.windSpeed,
        hourlyWeather.windDirection
    );
    Serial.println(F("---------------------------------"));
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
