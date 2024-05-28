#include "Weather.h"

#include <Inkplate.h>
#include <ArduinoJson.h>

#include "../local_env.h"
#include "Network.h"
#include "DailyWeather.h"
#include "WeatherProvider/WeatherProvider.h"
#include "SdCard.h"


using namespace weather;

Weather::Weather(Inkplate& display)
    :   mDisplay(display)
{}

bool Weather::updateCurrent(network::Network& connection, const weatherprovider::WeatherProvider& provider)
{
    JsonDocument apiResponse;
    sdcard::SdCard card(mDisplay);
    bool success = false;
    if (cFakeAPIUpdates) {
        Serial.println(F("Reading file data to simulate current conditions API response"));
        success = card.getFakeCurrentData(apiResponse);
    }
    else
    {
        auto url = provider.getForecastedWeatherUrl();
        success = connection.apiGetResponse(apiResponse, url);
    }

    if (success) {
        Serial.println(F("Converting JSON to current weather data"));
        provider.toCurrentWeather(mForecast[0], apiResponse);
    }
    apiResponse.clear();
    return success;
}

bool Weather::updateForecast(network::Network& connection, const weatherprovider::WeatherProvider& provider)
{
    JsonDocument apiResponse;
    sdcard::SdCard card(mDisplay);
    bool success = false;
    if (cFakeAPIUpdates) {
        Serial.println(F("Reading file data to simulate forecasted conditions API response"));
        success = card.getFakeForecastData(apiResponse);
    }
    else {
        auto url = provider.getForecastedWeatherUrl();
        success = connection.apiGetResponse(apiResponse, url);
    }

    if (success) {
        Serial.println(F("Converting JSON to forecast weather data"));
        provider.toForecastedWeather(mForecast, apiResponse);
        mLastForecastTime = time(nullptr);
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
    return mHourlyWeather;
}

void Weather::printDailyWeather(const DailyWeather& dailyWeather)
{
    Serial.println(F("--------- Daily Weather ---------"));
    Serial.printf((const char*)F("Data collected at: %d\n"), dailyWeather.timestamp);

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
        (const char*)F("Condition: %s,  Precipitation:  %s (%.2f in.)\n"),
        weather::conditionToString(dailyWeather.condition).c_str(),
        weather::precipitationToString(dailyWeather.precipitationType).c_str(),
        dailyWeather.precipitation
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
        (const char*)F("Sunrise: %d  Sunset:  %d\n"),
        dailyWeather.sunrise,
        dailyWeather.sunset
    );
    Serial.println(F("---------------------------------"));
}

time_t Weather::getLastForecastTime()
{
    return mLastForecastTime;
}
