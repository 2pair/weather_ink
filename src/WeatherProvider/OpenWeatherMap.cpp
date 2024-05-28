#include "OpenWeatherMap.h"

#include <stdint.h>

#include <ArduinoJson.h>

#include "../DailyWeather.h"


using namespace weatherprovider;

const std::string OpenWeatherMap::cBaseUrl((const char*)F("https://api.openweathermap.org/data/2.5/"));

std::string OpenWeatherMap::getCurrentWeatherUrl() const
{
    return cBaseUrl + std::string(
        (char *)F("weather?lat=%.4f&lon=%.4f&appid=%s&units=imperial")
    ) +
    std::to_string(mLatitude) +
    std::to_string(mLongitude) +
    std::string(mApiKey);
}

std::string OpenWeatherMap::getForecastedWeatherUrl() const
{
    return cBaseUrl + std::string(
        (char *)F("forecast/daily?lat=%f&lon=%f&cnt=%d&appid=%s&units=imperial")
    ) +
    std::to_string(mLatitude) +
    std::to_string(mLongitude) +
    std::to_string(5) +
    std::string(mApiKey);
}

void OpenWeatherMap::toCurrentWeather(
    weather::DailyWeather& currentWeather,
    JsonDocument& currentApiResponse) const
{
    using namespace weather;

    currentWeather.timestamp =
        currentApiResponse["dt"].as<uint64_t>() - currentApiResponse["timezone"].as<uint64_t>();
    currentWeather.tempNow = currentApiResponse["main"]["temp"];
    currentWeather.feelsLike = currentApiResponse["main"]["feels_like"];

    currentWeather.humidity = currentApiResponse["main"]["humidity"];
    currentWeather.pressure = currentApiResponse["main"]["pressure"];
    currentWeather.visibility = currentApiResponse["visibility"];

    currentWeather.windSpeed = currentApiResponse["wind"]["speed"];
    currentWeather.gustSpeed = currentApiResponse["wind"]["gust"];
    currentWeather.windDirection = currentApiResponse["wind"]["deg"];

    const uint16_t code =  currentApiResponse["weather"][0]["id"];
    codeToConditions(currentWeather, code);

    if (currentWeather.precipitationType == precipitation::Type::rain)
    {
        float mmRain = currentApiResponse["rain"]["3h"];
        currentWeather.precipitation = mmRain /  25.4;
    }
    else if (currentWeather.precipitationType == precipitation::Type::snow)
    {
        float mmSnow = currentApiResponse["snow"]["3h"];
        currentWeather.precipitation = mmSnow /  25.4;
    }
    else
    {
        currentWeather.precipitation = 0.0;
    }

    if (conditionIsWindy(currentWeather)) {
        currentWeather.condition = Condition::windy;
    }
    Serial.println(F("JSON successfully converted to current weather"));
}

uint8_t OpenWeatherMap::toForecastedWeather(
    weather::daily_forecast& forecastedWeather,
    JsonDocument& forecastApiResponse) const
{
    using namespace weather;

    uint64_t lastTime = 0;
    auto minDays = std::min(forecastedWeather.size(), forecastApiResponse["cnt"].as<size_t>());
    size_t index = -1;
    bool reorder = false;
    for( auto iter = forecastedWeather.begin(); iter <= forecastedWeather.begin() + minDays; iter++ )
    {
        index++;
        DailyWeather& dailyWeather = *iter;
        auto dailyData = forecastApiResponse["list"][index];

        uint64_t timezoneOffsetSec = forecastApiResponse["city"]["timezone"];
        uint64_t localTimestamp = dailyData["dt"].as<uint64_t>() - timezoneOffsetSec;
        if (localTimestamp <= lastTime)
        {
            reorder = true;
        }
        lastTime = localTimestamp;

        dailyWeather.tempLow = dailyData["temp"]["min"];
        dailyWeather.tempHigh = dailyData["temp"]["max"];

        dailyWeather.sunrise = dailyData["sunrise"].as<uint64_t>() - timezoneOffsetSec;
        dailyWeather.sunset = dailyData["sunset"].as<uint64_t>() - timezoneOffsetSec;
        // TODO: Doesn't seem like we have moon data here...

        if (index == 0)
        {
            // Don't need any other data from this API for the current day
            continue;
        }

        dailyWeather.timestamp = localTimestamp;

        uint16_t code = dailyData["weather"][0]["id"];
        codeToConditions(dailyWeather, code);
        if (dailyWeather.precipitationType == precipitation::Type::rain)
        {
            float mmRain = dailyData["rain"];
            dailyWeather.precipitation = mmRain /  25.4;
        }
        else if (dailyWeather.precipitationType == precipitation::Type::snow)
        {
            float mmSnow = dailyData["snow"];
            dailyWeather.precipitation = mmSnow /  25.4;
        }
        else
        {
            dailyWeather.precipitation = 0.0;
        }

        if (conditionIsWindy(dailyWeather)) {
        dailyWeather.condition = Condition::windy;
    }

        dailyWeather.pressure = dailyData["pressure"];
        dailyWeather.humidity = dailyData["humidity"];

        dailyWeather.windSpeed = dailyData["speed"];
        dailyWeather.gustSpeed = dailyData["gust"];
        dailyWeather.windDirection = dailyData["deg"];
    }
    Serial.println(F("JSON successfully converted to forecasted weather"));
    if (reorder)
    {
        Serial.print(F("WARNING: Weather data was not in chronological order from API\n"));
    }
    return minDays;
}

void OpenWeatherMap::codeToConditions(
    weather::DailyWeather& dailyWeather,
    const uint16_t code) const
{
    using namespace weather;

    if (code >= 200 && code < 300)
    {
        dailyWeather.condition = Condition::thunderstorm;
        dailyWeather.precipitationType = precipitation::Type::rain;
    }
    else if (code >= 300 && code < 400)
    {
        dailyWeather.condition = Condition::drizzle;
        dailyWeather.precipitationType = precipitation::Type::rain;
    }
    else if (code == 500)
    {
        dailyWeather.condition = Condition::lightRain;
        dailyWeather.precipitationType = precipitation::Type::rain;
    }
    else if (code == 511)
    {
        dailyWeather.condition = Condition::freezingRain;
        dailyWeather.precipitationType = precipitation::Type::rain;
    }
    else if (code >= 501 && code < 600)
    {
        dailyWeather.condition = Condition::rain;
        dailyWeather.precipitationType = precipitation::Type::rain;
    }
    else if (code >= 612 && code < 614)
    {
        dailyWeather.condition = Condition::sleet;
        dailyWeather.precipitationType = precipitation::Type::snow;
    }
    else if (code >= 600 && code < 700)
    {
        dailyWeather.condition = Condition::snow;
        dailyWeather.precipitationType = precipitation::Type::snow;
    }
    else if (code >= 700 && code < 800)
    {
        dailyWeather.condition = Condition::foggy;
        dailyWeather.precipitationType = precipitation::Type::none;
    }
    else if (code == 800)
    {
        dailyWeather.condition = Condition::clear;
        dailyWeather.precipitationType = precipitation::Type::none;
    }
    else if (code >= 800 && code < 803)
    {
        dailyWeather.condition = Condition::partlyCloudy;
        dailyWeather.precipitationType = precipitation::Type::none;
    }
    else if (code >= 803 && code < 805)
    {
        dailyWeather.condition = Condition::cloudy;
        dailyWeather.precipitationType = precipitation::Type::none;
    }
    else
    {
        dailyWeather.condition = Condition::unknownCondition;
        dailyWeather.precipitationType = precipitation::Type::none;
    }
}
