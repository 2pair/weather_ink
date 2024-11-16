#include "OpenWeatherMap.h"

#include <stdint.h>

#include <ArduinoJson.h>
#include <esp32-hal-log.h>

#include "../DailyWeather.h"
#include "../TimeUtils.h"


using namespace weatherprovider;

const std::string OpenWeatherMap::cBaseUrl((const char*)F("http://api.openweathermap.org/data/2.5/"));
const std::string OpenWeatherMap::cFsDirectory = "/owm/";

size_t OpenWeatherMap::getWeatherUpdateIntervalSeconds() const
{
    return cWeatherUpdateIntervalSeconds;
}

size_t OpenWeatherMap::getForecastUpdateIntervalSeconds() const
{
    return cForecastUpdateIntervalSeconds;
}

std::string OpenWeatherMap::getCurrentWeatherUrl() const
{

    const char* format = (char *)F("weather?lat=%.4f&lon=%.4f&appid=%s&units=imperial");
    int resultSize = std::snprintf( nullptr, 0, format, mLatitude, mLongitude, mApiKey.c_str()) + 1;
    auto size = static_cast<size_t>(resultSize);
    std::vector<char> endpoint;;
    endpoint.reserve(size);

    std::snprintf(endpoint.data(), size, format, mLatitude, mLongitude, mApiKey.c_str());
    return std::string(cBaseUrl).append(std::string(endpoint.data()));
}

std::string OpenWeatherMap::getHourlyWeatherUrl() const
{
    //TODO
}

std::string OpenWeatherMap::getForecastedWeatherUrl() const
{
    static constexpr size_t days = 4;
    const char* format = (char *)F("forecast/daily?lat=%f&lon=%f&cnt=%d&appid=%s&units=imperial");
    int resultSize = std::snprintf( nullptr, 0, format, mLatitude, mLongitude, days, mApiKey.c_str()) + 1;
    auto size = static_cast<size_t>(resultSize);
    std::vector<char> endpoint;
    endpoint.reserve(size);

    std::snprintf(endpoint.data(), size, format, mLatitude, mLongitude, days, mApiKey.c_str());
    return std::string(cBaseUrl).append(std::string(endpoint.data()));
}

void OpenWeatherMap::toCurrentWeather(
    weather::DailyWeather& currentWeather,
    const JsonDocument& currentApiResponse) const
{
    using namespace weather;

    currentWeather.timestamp = currentApiResponse["dt"];
    currentWeather.timeZone = currentApiResponse["timezone"].as<int64_t>()  / cSecondsPerHour;
    currentWeather.tempNow = currentApiResponse["main"]["temp"];
    currentWeather.feelsLike = currentApiResponse["main"]["feels_like"];

    currentWeather.humidity = currentApiResponse["main"]["humidity"];
    currentWeather.pressure = currentApiResponse["main"]["pressure"];
    currentWeather.visibility = currentApiResponse["visibility"];

    currentWeather.windSpeed = currentApiResponse["wind"]["speed"];
    currentWeather.gustSpeed = currentApiResponse["wind"]["gust"];
    currentWeather.windDirection = currentApiResponse["wind"]["deg"];

    const uint16_t code =  currentApiResponse["weather"][0]["id"];
    currentWeather.condition = codeToConditions(code);
        float mmRain = currentApiResponse["rain"]["3h"];
        currentWeather.precipitation = mmRain /  25.4;
        float mmSnow = currentApiResponse["snow"]["3h"];
        currentWeather.precipitation = mmSnow /  25.4;

    if (conditionIsWindy(currentWeather.condition, currentWeather.windSpeed)) {
        currentWeather.condition = Condition::windy;
    }
    log_i("JSON successfully converted to current weather");
}

uint8_t OpenWeatherMap::toForecastedWeather(
    weather::daily_forecast& forecastedWeather,
    const JsonDocument& forecastApiResponse) const
{
    using namespace weather;

    time_t lastTime = 0;
    auto minDays = std::min(forecastedWeather.size(), forecastApiResponse["cnt"].as<size_t>());
    size_t index = -1;
    bool reorder = false;
    for( auto iter = forecastedWeather.begin(); iter < forecastedWeather.begin() + minDays; iter++ )
    {
        index++;
        DailyWeather& dailyWeather = *iter;
        auto dailyData = forecastApiResponse["list"][index];

        dailyWeather.timeZone = forecastApiResponse["city"]["timezone"].as<int64_t>() / cSecondsPerHour;
        time_t timestamp = dailyData["dt"];
        if (timestamp <= lastTime)
        {
            reorder = true;
        }
        lastTime = timestamp;

        dailyWeather.tempLow = dailyData["temp"]["min"];
        dailyWeather.tempHigh = dailyData["temp"]["max"];

        dailyWeather.sunrise = dailyData["sunrise"];
        dailyWeather.sunset = dailyData["sunset"];
        // TODO: Doesn't seem like we have moon data here...

        if (index == 0)
        {
            // Don't need any other data from this API for the current day
            continue;
        }

        dailyWeather.timestamp = timestamp;

        uint16_t code = dailyData["weather"][0]["id"];
        dailyWeather.condition = codeToConditions(code);
        dailyWeather.precipitation = dailyData["rain"].as<float>() /  25.4;
        dailyWeather.precipitation += dailyData["snow"].as<float>() /  25.4;

        if (conditionIsWindy(dailyWeather.condition, dailyWeather.windSpeed)) {
        dailyWeather.condition = Condition::windy;
    }

        dailyWeather.pressure = dailyData["pressure"];
        dailyWeather.humidity = dailyData["humidity"];

        dailyWeather.windSpeed = dailyData["speed"];
        dailyWeather.gustSpeed = dailyData["gust"];
        dailyWeather.windDirection = dailyData["deg"];
    }
    log_i("JSON successfully converted to forecasted weather");
    if (reorder)
    {
        log_w("Weather data was not in chronological order from API");
    }
    return minDays;
}

uint8_t OpenWeatherMap::toHourlyWeather(
    weather::hourly_forecast& forecastedWeather,
    const JsonDocument& forecastApiResponse) const
{
    //TODO...
    return 0;
}

 weather::Condition OpenWeatherMap::codeToConditions(const uint16_t code) const
{
    using namespace weather;

    if (code >= 200 && code < 300)
    {
        return Condition::thunderstorm;
    }
    else if (code >= 300 && code < 400)
    {
        return Condition::drizzle;
    }
    else if (code == 500)
    {
        return Condition::lightRain;
    }
    else if (code == 511)
    {
        return Condition::freezingRain;
    }
    else if (code >= 501 && code < 600)
    {
        return Condition::rain;
    }
    else if (code >= 612 && code < 614)
    {
        return Condition::sleet;
    }
    else if (code >= 600 && code < 700)
    {
        return Condition::snow;
    }
    else if (code >= 700 && code < 800)
    {
        return Condition::foggy;
    }
    else if (code == 800)
    {
        return Condition::clear;
    }
    else if (code >= 800 && code < 803)
    {
        return Condition::partlyCloudy;
    }
    else if (code >= 803 && code < 805)
    {
        return Condition::cloudy;
    }
    else
    {
        return Condition::unknownCondition;
    }
}

std::string OpenWeatherMap::getFileSystemDirectory() const
{
    return cFsDirectory;
}