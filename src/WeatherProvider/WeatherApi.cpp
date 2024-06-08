#include "WeatherApi.h"

#include <stdint.h>
#include <map>
#include <string>
#include <string_view>

#include <ArduinoJson.h>

#include "../DailyWeather.h"
#include "../TimeUtils.h"


using namespace weatherprovider;

const std::string WeatherApi::cBaseUrl((const char*)F("http://api.weatherapi.com/v1/"));
const std::string WeatherApi::cFsDirectory = "/wa/";

int8_t timeZoneFromApiResponse(const JsonDocument& apiResponse)
{
    time_t localTime = timeutils::timeStrToEpochTime(
        apiResponse["location"]["localtime"],
        (const char*)F("%F %H:%M")
    );
    time_t collectionTimestamp = apiResponse["location"]["localtime_epoch"];
    return static_cast<int64_t>(localTime - collectionTimestamp) / cSecondsPerHour;
}

size_t WeatherApi::getWeatherUpdateIntervalSeconds() const
{
    return cWeatherUpdateIntervalSeconds;
}

size_t WeatherApi::getForecastUpdateIntervalSeconds() const
{
    return cForecastUpdateIntervalSeconds;
}

std::string WeatherApi::getCurrentWeatherUrl() const
{
    return getForecastedWeatherUrl();
    //current weather is a subset of the forecasted weather, and the forecast also has hourly data
    // const char* format = (const char *)F("current.json?q=%.4f,%.4f&key=%s");
    // int resultSize = std::snprintf( nullptr, 0, format, mLatitude, mLongitude, mApiKey.c_str()) + 1;
    // auto size = static_cast<size_t>(resultSize);
    // std::vector<char> endpoint;
    // endpoint.reserve(size);

    // std::snprintf(endpoint.data(), size, format, mLatitude, mLongitude, mApiKey.c_str());
    // return std::string(cBaseUrl).append(std::string(endpoint.data()));
}

std::string WeatherApi::getForecastedWeatherUrl() const
{
    // Reduce response size by only getting specific data
    std::string fieldFilter = (const char*)F(
        "&current_fields=last_updated_epoch,temp_f,feelslike_f,humidity,pressure_mb,"
            "vis_miles,wind_mph,gust_mph,wind_degree,condition,precip_in"
        "&day_fields=mintemp_f,maxtemp_f,daily_chance_of_rain,daily_chance_of_snow,"
            "condition,totalprecip_in,totalsnow_cm,avghumidity,avgvis_miles,maxwind_mph"
        "&hour_fields=time,time_epoch,temp_f,feelslike_f,chance_of_rain,chance_of_snow,"
            "condition,precip_in,snow_cm,pressure_mb,humidity,wind_mph,wind_degree"
    );
    static constexpr size_t days = 4;
    const char* format = (const char *)F("forecast.json?q=%f,%f&days=%d&key=%s");
    int resultSize = std::snprintf(nullptr, 0, format, mLatitude, mLongitude, days, mApiKey.c_str()) + 1;
    auto size = static_cast<size_t>(resultSize);
    std::vector<char> endpoint;
    endpoint.reserve(size);

    std::snprintf(endpoint.data(), size, format, mLatitude, mLongitude, days, mApiKey.c_str());
    std::string endpoint_data = endpoint.data();
    return std::string(cBaseUrl).append(endpoint_data).append(fieldFilter);
}

void WeatherApi::toCurrentWeather(
    weather::DailyWeather& currentWeather,
    JsonDocument& currentApiResponse) const
{
    using namespace weather;

    currentWeather.timestamp = currentApiResponse["current"]["last_updated_epoch"];
    currentWeather.timeZone = timeZoneFromApiResponse(currentApiResponse);
    currentWeather.tempNow = currentApiResponse["current"]["temp_f"];
    currentWeather.feelsLike = currentApiResponse["current"]["feelslike_f"];

    currentWeather.humidity = currentApiResponse["current"]["humidity"];
    currentWeather.pressure = currentApiResponse["current"]["pressure_mb"];
    currentWeather.visibility = currentApiResponse["current"]["vis_miles"];

    currentWeather.windSpeed = currentApiResponse["current"]["wind_mph"];
    currentWeather.gustSpeed = currentApiResponse["current"]["gust_mph"];
    currentWeather.windDirection = currentApiResponse["current"]["wind_degree"];

    const uint16_t code =  currentApiResponse["current"]["condition"]["code"];
    currentWeather.condition = codeToConditions(code);
    if (conditionIsWindy(currentWeather.condition, currentWeather.windSpeed)) {
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
    for( auto iter = forecastedWeather.begin(); iter < forecastedWeather.begin() + minDays; iter++ )
    {
        index++;
        DailyWeather& dailyWeather = *iter;
        auto dailyData = forecastResponse[index];

        uint64_t timestamp = dailyData["date_epoch"];
        if (timestamp <= lastDt)
        {
            reorder = true;
        }
        lastDt = timestamp;
        dailyWeather.timeZone = timeZoneFromApiResponse(forecastApiResponse);
        auto dailyDayData = dailyData["day"];
        dailyWeather.tempLow = dailyDayData["mintemp_f"];
        dailyWeather.tempHigh = dailyDayData["maxtemp_f"];
        dailyWeather.chanceOfPrecipitation = std::max(
            dailyDayData["daily_chance_of_rain"].as<float>(),
            dailyDayData["daily_chance_of_snow"].as<float>()
        ) / 100.0;
        dailyWeather.moonPhase = stringToMoonPhase(dailyData["astro"]["moon_phase"].as<const char*>());
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

        dailyWeather.timestamp = timestamp;

        uint16_t code = dailyDayData["condition"]["code"];
        dailyWeather.condition = codeToConditions(code);
        dailyWeather.precipitation = dailyDayData["totalprecip_in"];
        dailyWeather.precipitation += (dailyDayData["totalsnow_cm"].as<float>() /  2.54);
        if (conditionIsWindy(dailyWeather.condition, dailyWeather.windSpeed))
        {
            dailyWeather.condition = Condition::windy;
        }
        //dailyWeather.pressure = ; // not available
        dailyWeather.humidity = dailyDayData["avghumidity"];
        dailyWeather.visibility = dailyDayData["avgvis_miles"];
        dailyWeather.windSpeed = dailyDayData["maxwind_mph"];
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

uint8_t WeatherApi::toHourlyWeather(
    weather::hourly_forecast& forecastedWeather,
    JsonDocument& forecastApiResponse) const
{
    using namespace weather;

    auto const forecastResponse = forecastApiResponse["forecast"]["forecastday"][0]["hour"];
    auto hoursToGetFirstDay = 0;
     // map of day index to a pair of the hour to start at and how many hours to retrieve
    std::map<size_t, std::pair<size_t, size_t>> dayHourMap;
    time_t timeNow = forecastApiResponse["location"]["localtime_epoch"];
    for (size_t i = 0; i < forecastResponse.size(); i++)
    {
        if (forecastResponse[i]["time_epoch"] < timeNow)
        {
            continue;
        }
        hoursToGetFirstDay = std::min(forecastResponse.size() - i, forecastedWeather.size());
        dayHourMap.emplace(0, std::make_pair(i, hoursToGetFirstDay));
        break;
    }

    if (hoursToGetFirstDay < forecastedWeather.size())
    {
        size_t hoursInSecondDay = forecastApiResponse["forecast"]["forecastday"][1]["hour"].size();
        auto hoursToGetSecondDay = std::min(
            hoursInSecondDay,
            forecastedWeather.size() - hoursToGetFirstDay
        );
        dayHourMap.emplace(1, std::make_pair(0, hoursToGetSecondDay));
    }
    uint64_t lastDt = 0;
    auto minHours = std::min(forecastedWeather.size(), forecastResponse.size());
    size_t hourIndex = -1;
    for (auto const& mapIter : dayHourMap)
    {
        auto& day = mapIter.first;
        auto& startHour = mapIter.second.first;
        auto& hoursToGet = mapIter.second.second;
        Serial.printf("getting %u hours of weather with offset of %u for day %u\n",
            hoursToGet, startHour, day
        );

        bool reorder = false;
        for(
            size_t responseIter = startHour;
            responseIter < startHour + hoursToGet;
            responseIter++
        )
        {
            hourIndex++;
            HourlyWeather& hourlyWeather = forecastedWeather[hourIndex];
            auto const hourlyData =
                forecastApiResponse["forecast"]["forecastday"][day]["hour"][responseIter];
            Serial.printf("getting hourly data for %s\n", hourlyData["time"].as<std::string>().c_str());
            hourlyWeather.timestamp = hourlyData["time_epoch"];
            if (hourlyWeather.timestamp <= lastDt)
            {
                reorder = true;
            }
            lastDt = hourlyWeather.timestamp;
            hourlyWeather.timeZone = timeZoneFromApiResponse(forecastApiResponse);

            hourlyWeather.temp = hourlyData["temp_f"];
            hourlyWeather.feelsLike = hourlyData["feelslike_f"];
            hourlyWeather.chanceOfPrecipitation = std::max(
                hourlyData["chance_of_rain"].as<float>(),
                hourlyData["chance_of_snow"].as<float>()
            ) / 100;

            uint16_t code = hourlyData["condition"]["code"];
            hourlyWeather.condition = codeToConditions(code);
            hourlyWeather.precipitation = hourlyData["precip_in"];
            hourlyWeather.precipitation += hourlyData["snow_cm"].as<float>() /  2.54;

            if (conditionIsWindy(hourlyWeather.condition, hourlyWeather.windSpeed))
            {
                hourlyWeather.condition = Condition::windy;
            }
            hourlyWeather.pressure = hourlyData["pressure_mb"];
            hourlyWeather.humidity = hourlyData["humidity"];

            hourlyWeather.windSpeed = hourlyData["wind_mph"];
            hourlyWeather.windDirection = hourlyData["wind_degree"];
        }
        Serial.println(F("JSON successfully converted to hourly forecast weather"));
        if (reorder)
        {
            Serial.print(F("WARNING: Weather data was not in chronological order from API\n"));
        }
    }
    return minHours;
}

 weather::Condition WeatherApi::codeToConditions(const uint16_t code) const
{
    using namespace weather;
    switch(code)
    {
    case 1007:
        return Condition::lightning;
    case 1273:
    case 1276:
        return Condition::thunderstorm;
    case 1030:
    case 1150:
    case 1153:
        return Condition::drizzle;
    case 1063:
    case 1183:
    case 1240:
        return Condition::lightRain;
    case 1168:
    case 1171:
    case 1198:
    case 1201:
        return Condition::freezingRain;
    case 1186:
    case 1189:
    case 1243:
        return Condition::rain;
    case 1192:
    case 1195:
    case 1246:
        return Condition::heavyRain;
    case 1069:
    case 1204:
    case 1207:
    case 1237:
    case 1249:
    case 1252:
    case 1261:
    case 1264:
        return Condition::sleet;
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
        return Condition::snow;
    case 1072:
    case 1279:
    case 1282:
        return Condition::wintryMix;
    case 1135:
    case 1147:
        return Condition::foggy;
    case 1000:
        return Condition::clear;
    case 1003:
        return Condition::partlyCloudy;
    case 1006:
    case 1009:
        return Condition::cloudy;
    default:
        return Condition::unknownCondition;
    }
}

std::string WeatherApi::getFileSystemDirectory() const
{
    return cFsDirectory;
}