#include "WeatherApi.h"

#include <stdint.h>

#include <ArduinoJson.h>

#include "../DailyWeather.h"


using namespace weatherprovider;

const std::string WeatherApi::cBaseUrl((const char*)F("https://api.weatherapi.com/v1/"));

std::string WeatherApi::getCurrentWeatherUrl() const
{
    return cBaseUrl + std::string(
        (char *)F("current.json?q=%.4f,%.4f&key=%s")
    ) +
    std::to_string(mLatitude) +
    std::to_string(mLongitude) +
    std::string(mApiKey);
}

std::string WeatherApi::getForecastedWeatherUrl() const
{
    return cBaseUrl + std::string(
        (char *)F("forecast.json/daily?q=%f,%f&days=%d&key=%s")
    ) +
    std::to_string(mLatitude) +
    std::to_string(mLongitude) +
    std::to_string(5) +
    std::string(mApiKey);
}

void WeatherApi::toCurrentWeather(
    weather::DailyWeather& currentWeather,
    JsonDocument& currentApiResponse) const
{
    using namespace weather;

    currentWeather.timestamp = currentApiResponse["current"]["last_updated_epoch"];
    currentWeather.tempNow = currentApiResponse["current"]["temp_f"];
    currentWeather.feelsLike = currentApiResponse["current"]["feelslike_f"];

    currentWeather.humidity = currentApiResponse["current"]["humidity"];
    currentWeather.pressure = currentApiResponse["current"]["pressure_mb"];
    currentWeather.visibility = currentApiResponse["current"]["vis_miles"];

    currentWeather.windSpeed = currentApiResponse["current"]["wind_mph"];
    currentWeather.gustSpeed = currentApiResponse["current"]["gust_mph"];
    currentWeather.windDirection = currentApiResponse["current"]["wind_degree"];

    const uint16_t code =  currentApiResponse["current"]["condition:code"];
    codeToConditions(currentWeather, code);
    if (conditionIsWindy(currentWeather)) {
        currentWeather.condition = Condition::windy;
    }

    currentWeather.precipitation =  currentApiResponse["current"]["precip_in"];

    Serial.println(F("JSON successfully converted to current weather"));
}

uint8_t WeatherApi::toForecastedWeather(
    weather::daily_forecast& forecastedWeather,
    JsonDocument& forecastApiResponse) const
{
    using namespace weather;

    auto forecastResponse = forecastApiResponse["forecast"]["forecastday"];
    uint64_t lastDt = 0;
    auto minDays = std::min(forecastedWeather.size(), forecastResponse.size());
    size_t index = -1;
    bool reorder = false;
    for( auto iter = forecastedWeather.begin(); iter <= forecastedWeather.begin() + minDays; iter++ )
    {
        index++;
        DailyWeather& dailyWeather = *iter;
        auto dailyData = forecastResponse[index];

        uint64_t localTimestamp = dailyData["date_epoch"];
        if (localTimestamp <= lastDt)
        {
            reorder = true;
        }
        lastDt = localTimestamp;

        auto dailyDayData = dailyData["day"];
        dailyWeather.tempLow = dailyDayData["mintemp_f"];
        dailyWeather.tempHigh = dailyDayData["maxtemp_f"];

        dailyWeather.moonPhase = dailyData["astro"]["moon_phase"];
        std::string dateTime = dailyData["date"];
        tm time;

        auto sunrise = dateTime + ' ' + dailyData["astro"]["sunrise"].as<const char*>();
        strptime(dateTime.c_str(), "%F %I:%M %p", &time);
        dailyWeather.sunrise = mktime(&time);

        auto sunset = dateTime + ' ' + dailyData["astro"]["sunset"].as<const char*>();
        strptime(dateTime.c_str(), "%F %I:%M %p", &time);
        dailyWeather.sunset = mktime(&time);

        if (index == 0)
        {
            // Don't need any other data from this API for the current day
            continue;
        }

        dailyWeather.timestamp = localTimestamp;

        uint16_t code = dailyDayData["condition"]["code"];
        codeToConditions(dailyWeather, code);
        if (dailyWeather.precipitationType == precipitation::Type::rain)
        {
            float mmRain = dailyDayData["totalprecip_in"];
            dailyWeather.precipitation = mmRain /  25.4;
        }
        else if (dailyWeather.precipitationType == precipitation::Type::snow)
        {
            float mmSnow = dailyDayData["totalsnow_cm"];
            dailyWeather.precipitation = mmSnow /  2.54;
        }
        else
        {
            dailyWeather.precipitation = 0.0;
        }

        if (conditionIsWindy(dailyWeather)) {
        dailyWeather.condition = Condition::windy;
    }
        //dailyWeather.pressure = ; // not available
        dailyWeather.humidity = dailyDayData["avghumidity"];
        dailyWeather.visibility = dailyDayData["avgvis_miles"];

        dailyWeather.windSpeed = dailyData["maxwind_mph"];
        //dailyWeather.gustSpeed = ; // not available
        //dailyWeather.windDirection = ; // not available
    }
    Serial.println(F("JSON successfully converted to forecasted weather"));
    if (reorder)
    {
        Serial.print(F("WARNING: Weather data was not in chronological order from API\n"));
    }
    return minDays;
}

void WeatherApi::codeToConditions(
    weather::DailyWeather& dailyWeather,
    const uint16_t code) const
{
    using namespace weather;
    switch(code)
    {
    case 1007:
        dailyWeather.condition = Condition::lightning;
        dailyWeather.precipitationType = precipitation::Type::rain;
        break;
    case 1273:
    case 1276:
        dailyWeather.condition = Condition::thunderstorm;
        dailyWeather.precipitationType = precipitation::Type::rain;
        break;
    case 1030:
    case 1150:
    case 1153:
        dailyWeather.condition = Condition::drizzle;
        dailyWeather.precipitationType = precipitation::Type::rain;
        break;
    case 1063:
    case 1183:
    case 1240:
        dailyWeather.condition = Condition::lightRain;
        dailyWeather.precipitationType = precipitation::Type::rain;
        break;
    case 1168:
    case 1171:
    case 1198:
    case 1201:
        dailyWeather.condition = Condition::freezingRain;
        dailyWeather.precipitationType = precipitation::Type::rain;
        break;
    case 1186:
    case 1189:
    case 1243:
        dailyWeather.condition = Condition::rain;
        dailyWeather.precipitationType = precipitation::Type::rain;
        break;
    case 1192:
    case 1195:
    case 1246:
        dailyWeather.condition = Condition::heavyRain;
        dailyWeather.precipitationType = precipitation::Type::rain;
        break;
    case 1069:
    case 1204:
    case 1207:
    case 1237:
    case 1249:
    case 1252:
    case 1261:
    case 1264:
        dailyWeather.condition = Condition::sleet;
        dailyWeather.precipitationType = precipitation::Type::snow;
        break;
    case 1066:
    case 1114:
    case 1117:
    case 1210:
    case 1213:
    case 1216:
    case 1219:
    case 1222:
    case 1225:
    case 1255:
    case 1258:
        dailyWeather.condition = Condition::snow;
        dailyWeather.precipitationType = precipitation::Type::snow;
        break;
    case 1072:
    case 1279:
    case 1282:
        dailyWeather.condition = Condition::wintryMix;
        dailyWeather.precipitationType = precipitation::Type::snow;
        break;
    case 1135:
    case 1147:
        dailyWeather.condition = Condition::foggy;
        dailyWeather.precipitationType = precipitation::Type::none;
        break;
    case 1000:
        dailyWeather.condition = Condition::clear;
        dailyWeather.precipitationType = precipitation::Type::none;
        break;
    case 1003:
        dailyWeather.condition = Condition::partlyCloudy;
        dailyWeather.precipitationType = precipitation::Type::none;
        break;
    case 1006:
    case 1009:
        dailyWeather.condition = Condition::cloudy;
        dailyWeather.precipitationType = precipitation::Type::none;
        break;
    default:
        dailyWeather.condition = Condition::unknownCondition;
        dailyWeather.precipitationType = precipitation::Type::none;
    }
}
