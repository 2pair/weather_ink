#include "OpenWeatherMap.h"

#include <stdint.h>

#include <ArduinoJson.h>

#include "../DailyWeather.h"


using namespace weatherprovider;

void OpenWeatherMap::getCurrentWeatherUrl(char * buffer, uint8_t bufferLen)
{
    sprintf(
        buffer,
        (char *)F("https://api.openweathermap.org/data/2.5/weather?lat=%.4f&lon=%.4f&appid=%s&units=imperial"),
        mLatitude,
        mLongitude,
        mApiKey
    );
}

void OpenWeatherMap::getForecastedWeatherUrl(char * buffer, uint8_t bufferLen)
{
    sprintf(
        buffer,
        (char *)F("https://api.openweathermap.org/data/2.5/forecast/daily?lat=%f&lon=%f&cnt=%d&appid=%s&units=imperial"),
        mLatitude,
        mLongitude,
        5,
        mApiKey
    );
}

void OpenWeatherMap::toCurrentWeather(
    weather::DailyWeather& currentWeather,
    JsonDocument& currentApiResponse)
{
    using namespace weather;

    currentWeather.tempNow = currentApiResponse["main"]["temp"];
    currentWeather.feelsLike = currentApiResponse["main"]["feels_like"];
    // These are actually current local low/high, not daily
    //currentWeather.tempLow = currentApiResponse["main"]["temp_min"].as<float>();
    //currentWeather.tempHigh = currentApiResponse["main"]["temp_max"].as<float>();

    currentWeather.humidity = currentApiResponse["main"]["humidity"];
    currentWeather.pressure = currentApiResponse["main"]["pressure"];
    currentWeather.visibility = currentApiResponse["visibility"];

    currentWeather.windSpeed = currentApiResponse["wind"]["speed"];
    currentWeather.gustSpeed = currentApiResponse["wind"]["gust"];
    currentWeather.windDirection = currentApiResponse["wind"]["deg"];

    auto code =  currentApiResponse["weather"][0]["id"].as<uint16_t>();
    codeToConditions(currentWeather, code);

    if (currentWeather.precipitationType == precipitation::Type::rain) {
        float mmRain = currentApiResponse["rain"]["3h"];
        currentWeather.precipitation = mmRain /  25.4;
    }
    else if (currentWeather.precipitationType == precipitation::Type::snow) {
        float mmSnow = currentApiResponse["snow"]["3h"];
        currentWeather.precipitation = mmSnow /  25.4;
    }
    else {
        currentWeather.precipitation = 0.0;
    }
}

uint8_t OpenWeatherMap::toForecastedWeather(
    weather::DailyWeather* forecastedWeather,
    const uint8_t maxDays,
    JsonDocument& forecastApiResponse)
{
    using namespace weather;

    auto days_returned = forecastApiResponse["cnt"].as<uint8_t>();
    uint64_t lastDt = 0;
    for (int i = 0; i < std::min(maxDays, days_returned); i ++) {
        DailyWeather& dailyWeather = forecastedWeather[i];
        auto dailyData = forecastApiResponse["list"][i];

        // delete if API data order has been validated
        uint64_t dt = dailyData["dt"];
        if (dt <= lastDt) {
            Serial.print("WARNING: Weather data was not in chronological order from API");
        }
        lastDt = dt;
        // end delete

        dailyWeather.tempLow = dailyData["temp"]["min"];
        dailyWeather.tempHigh = dailyData["temp"]["max"];

        dailyWeather.sunrise = dailyData["sunrise"];
        dailyWeather.sunset = dailyData["sunset"];


        if (i == 0) {
            // Don't need any other data from this API for today
            continue;
        }

        uint16_t code = dailyData["weather"][0]["id"];
        codeToConditions(dailyWeather, code);
        if (dailyWeather.precipitationType == precipitation::Type::rain) {
            float mmRain = dailyData["rain"];
            dailyWeather.precipitation = mmRain /  25.4;
        }
        else if (dailyWeather.precipitationType == precipitation::Type::snow) {
            float mmSnow = dailyData["snow"];
            dailyWeather.precipitation = mmSnow /  25.4;
        }
        else {
            dailyWeather.precipitation = 0.0;
        }

        dailyWeather.pressure = dailyData["pressure"];
        dailyWeather.humidity = dailyData["humidity"];

        dailyWeather.windSpeed = dailyData["speed"];
        dailyWeather.gustSpeed = dailyData["gust"];
        dailyWeather.windDirection = dailyData["deg"];
    }
}

void OpenWeatherMap::codeToConditions(
    weather::DailyWeather& dailyWeather,
    const uint16_t code)
{
    using namespace weather;

    if (code >= 200 && code < 300) {
        dailyWeather.condition = Condition::thunderstorm;
        dailyWeather.precipitationType = precipitation::Type::rain;
    }
    else if (code >= 300 && code < 400) {
        dailyWeather.condition = Condition::drizzle;
        dailyWeather.precipitationType = precipitation::Type::rain;
    }
    else if (code == 500) {
        dailyWeather.condition = Condition::lightRain;
        dailyWeather.precipitationType = precipitation::Type::rain;
    }
    else if (code == 511) {
        dailyWeather.condition = Condition::freezingRain;
        dailyWeather.precipitationType = precipitation::Type::rain;
    }
    else if (code >= 501 && code < 600) {
        dailyWeather.condition = Condition::rain;
        dailyWeather.precipitationType = precipitation::Type::rain;
    }
    else if (code >= 612 && code < 614) {
        dailyWeather.condition = Condition::sleet;
        dailyWeather.precipitationType = precipitation::Type::snow;
    }
    else if (code >= 600 && code < 700) {
        dailyWeather.condition = Condition::snow;
        dailyWeather.precipitationType = precipitation::Type::snow;
    }
    else if (code >= 700 && code < 800) {
        dailyWeather.condition = Condition::foggy;
        dailyWeather.precipitationType = precipitation::Type::none;
    }
    else if (code == 800) {
        dailyWeather.condition = Condition::clear;
        dailyWeather.precipitationType = precipitation::Type::none;
    }
    else if (code >= 800 && code < 803) {
        dailyWeather.condition = Condition::partlyCloudy;
        dailyWeather.precipitationType = precipitation::Type::none;
    }
    else if (code >= 803 && code < 805) {
        dailyWeather.condition = Condition::cloudy;
        dailyWeather.precipitationType = precipitation::Type::none;
    }
    else {
        dailyWeather.condition = Condition::unknown;
        dailyWeather.precipitationType = precipitation::Type::none;
    }
}
