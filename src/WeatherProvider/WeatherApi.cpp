#include "WeatherApi.h"

#include <stdint.h>
#include <map>
#include <string>

#include <ArduinoJson.h>
#include <esp32-hal-log.h>

#include "../DailyWeather.h"
#include "../TimeUtils.h"
#include "../Network.h"


using namespace weatherprovider;

const std::string WeatherApi::cBaseUrl((const char*)F("http://api.weatherapi.com/v1/"));
const std::string WeatherApi::cFsDirectory = "/wa/";

int8_t timeZoneFromApiResponse(const JsonDocument& apiResponse)
{
    time_t localTime = timeutils::timeStrToEpochTime(
        apiResponse["location"]["localtime"],
        (const char*)F("%F %H:%M")
    );
    time_t epochTime = apiResponse["location"]["localtime_epoch"];
    return static_cast<int8_t>(std::round(difftime(localTime, epochTime) / cSecondsPerHour));
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
    // Current weather is a subset of the forecasted weather, however the free tier
    // doesn't let us get more than 3 days at a time, so this needs to be a separate call.
    // We get 1,000,000 calls a month though, so it's fine.

    return std::string(cBaseUrl)
        .append("forecast.json?")
        .append("q=" + std::to_string(mLatitude) + "," + std::to_string(mLongitude))
        .append("&days=1&key=" + mApiKey);
}

std::string WeatherApi::getForecastedWeatherUrl(const uint8_t days, const uint8_t offset) const
{
    auto forecastUrl = std::string(cBaseUrl)
        .append("forecast.json?")
        .append("q=" + std::to_string(mLatitude) + "," + std::to_string(mLongitude))
        .append("&days=" + std::to_string(days))
        .append("&key=" + mApiKey);
    if (offset > 0)
    {
        const uint64_t atTime = time(nullptr) + (cSecondsPerDay * offset);
        log_d("Getting %d days of forecast starting at time %d", days, atTime);
        if (days > 1)
        {
            log_w("Fetching more than one day of data with a time offset is not allowed on the free tier");
        }
        forecastUrl += "&unixdt=" + std::to_string(atTime);
    }
    else
    {
        log_d("Getting %d days of forecast starting from today", days);
    }
    return forecastUrl;
}

std::string WeatherApi::getHourlyWeatherUrl() const
{
    // Get two days in case we are close to midnight
    return getForecastedWeatherUrl(2, 0);
}

std::string WeatherApi::getForecastedWeatherUrl() const
{
    return getForecastedWeatherUrl(3, 1);
}

time_t WeatherApi::toForecastedWeather(
    weather::daily_forecast& forecastedWeather,
    network::Network& connection) const
{
    JsonDocument apiResponse;
    bool totalSuccess = true;
    for (size_t i = 1; i < forecastedWeather.size(); i++) {
        if (forecastedWeather[i].timestamp >= time(nullptr))
        {
            log_i("Already have forecast for  day %d, skipping update", i);
            continue;
        }

        auto url = getForecastedWeatherUrl(1, i);
        log_d("URL returned: %s", url.c_str());
        bool success = connection.getApiResponse(apiResponse, url);
        if (success)
        {
            log_d("Converting JSON to forecast weather data for index %d", i);
            toForecastedWeather(forecastedWeather[i], apiResponse);
        }
        else
        {
            log_w("Failed to get API response for forecasted weather for index %d", i);
        }
        apiResponse.clear();
        totalSuccess = (success) ? totalSuccess : false;
    }
    return (totalSuccess) ? time(nullptr) : 0;
}

void WeatherApi::setAstroData(
    weather::DailyWeather& dailyWeather,
    const JsonObjectConst& astroData,
    const std::string& dateTime) const
{
    dailyWeather.moonPhase = weather::stringToMoonPhase(astroData["moon_phase"].as<std::string>());

    std::string sunrise = astroData["sunrise"];
    log_d("Sunrise: %s", sunrise.c_str());
    if (sunrise.front() == '0')
    {
        sunrise.erase(sunrise.begin());
        log_v("Sunrise started with 0, now equals", sunrise.c_str());
    }
    sunrise = dateTime + " " + sunrise;
    log_v("Sunrise: %s", sunrise.c_str());
    tm sunriseTime;
    strptime(sunrise.c_str(), "%F %I:%M %p", &sunriseTime);
    dailyWeather.sunrise = mktime(&sunriseTime);

    std::string sunset = astroData["sunset"];
    log_d("Sunset: %s", sunset.c_str());
    if (sunset.front() == '0')
    {
        sunset.erase(sunset.begin());
        log_v("Sunset started with 0, now equals", sunset.c_str());
    }
    sunset = dateTime + " " + sunset;
    log_v("Sunset: %s", sunset.c_str());
    tm sunsetTime;
    strptime(sunset.c_str(), "%F %I:%M %p", &sunsetTime);
    dailyWeather.sunset = mktime(&sunsetTime);
}

void WeatherApi::toCurrentWeather(
    weather::DailyWeather& currentWeather,
    const JsonDocument& currentApiResponse) const
{
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
        currentWeather.condition = weather::Condition::windy;
    }

    currentWeather.precipitation =  currentApiResponse["current"]["precip_in"];

    currentWeather.timeZone = timeZoneFromApiResponse(currentApiResponse);
    auto dailyData = currentApiResponse["forecast"]["forecastday"][0];
    auto dailyDayData = dailyData["day"];
    currentWeather.tempLow = dailyDayData["mintemp_f"];
    currentWeather.tempHigh = dailyDayData["maxtemp_f"];
    currentWeather.chanceOfPrecipitation = std::max(
        dailyDayData["daily_chance_of_rain"].as<float>(),
        dailyDayData["daily_chance_of_snow"].as<float>()
    ) / 100.0;

    setAstroData(currentWeather, dailyData["astro"], dailyData["date"]);

    log_i("JSON successfully converted to current weather");
}

void WeatherApi::toForecastedWeather(
    weather::DailyWeather& dailyWeather,
    const JsonDocument& forecastApiResponse,
    const size_t index) const
{
    auto forecastResponse = forecastApiResponse["forecast"]["forecastday"];
    auto dailyData = forecastResponse[index];
    auto timeZone = timeZoneFromApiResponse(forecastApiResponse);
    log_d("time zone set to %d", timeZone);
    dailyWeather.timestamp = dailyData["date_epoch"].as<uint64_t>();

    auto dailyDayData = dailyData["day"];
    dailyWeather.tempLow = dailyDayData["mintemp_f"];
    dailyWeather.tempHigh = dailyDayData["maxtemp_f"];
    dailyWeather.chanceOfPrecipitation = std::max(
        dailyDayData["daily_chance_of_rain"].as<float>(),
        dailyDayData["daily_chance_of_snow"].as<float>()
    ) / 100.0;
    setAstroData(dailyWeather, dailyData["astro"], dailyData["date"]);

    uint16_t code = dailyDayData["condition"]["code"];
    dailyWeather.condition = codeToConditions(code);
    dailyWeather.precipitation = dailyDayData["totalprecip_in"];
    dailyWeather.precipitation += (dailyDayData["totalsnow_cm"].as<float>() /  2.54);
    if (conditionIsWindy(dailyWeather.condition, dailyWeather.windSpeed))
    {
        dailyWeather.condition = weather::Condition::windy;
    }
    //dailyWeather.pressure = ; // not available
    dailyWeather.humidity = dailyDayData["avghumidity"];
    dailyWeather.visibility = dailyDayData["avgvis_miles"];
    dailyWeather.windSpeed = dailyDayData["maxwind_mph"];
    //dailyWeather.gustSpeed = ; // not available
    //dailyWeather.windDirection = ; // not available
    log_i(
        "JSON successfully converted to forecasted weather for day with timestamp %llu",
        dailyWeather.timestamp
    );
}

uint8_t WeatherApi::toForecastedWeather(
    weather::daily_forecast& forecastedWeather,
    const JsonDocument& forecastApiResponse) const
{
    auto forecastResponse = forecastApiResponse["forecast"]["forecastday"];
    uint64_t lastDt = 0;
    // size() - 1 to exclude current day's weather.
    auto minDays = std::min(forecastedWeather.size() - 1, forecastResponse.size());
    size_t index = 0;
    bool reorder = false;
    // Skip *begin() because it is the current day's weather.
    for( auto iter = forecastedWeather.begin() + 1; iter < forecastedWeather.begin() + minDays + 1; iter++ )
    {
        index++;
        log_d("populating forecast from index %d", index);
        weather::DailyWeather& dailyWeather = *iter;
        toForecastedWeather(dailyWeather, forecastResponse, index);
        if (dailyWeather.timestamp <= lastDt)
        {
            reorder = true;
        }
        lastDt = dailyWeather.timestamp;
    }
    log_i("JSON successfully converted to forecasted weather");
    if (reorder)
    {
        log_w("Weather data was not in chronological order from API");
    }
    return minDays;
}

uint8_t WeatherApi::toHourlyWeather(
    weather::hourly_forecast& forecastedWeather,
    const JsonDocument& forecastApiResponse) const
{
    auto const forecastResponse = forecastApiResponse["forecast"]["forecastday"][0]["hour"];
    auto hoursToGetFirstDay = 0;
     // map of day index to a pair of the hour to start at and how many hours to retrieve
     // ex: {0 : (20, 4), 1: (0, 4)} # 8 pm to midnight on day 0, midnight to 4 am on day 1
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
        if (hoursToGet > 0)
        {
            log_d("getting %u hours of weather with offset of %u for day %u",
                hoursToGet, startHour, day
            );
        }
        else{
            log_d("Not getting hourly data for day %u", day);
            continue;
        }

        bool reorder = false;
        for(
            size_t responseIter = startHour;
            responseIter < startHour + hoursToGet;
            responseIter++
        )
        {
            hourIndex++;
            weather::HourlyWeather& hourlyWeather = forecastedWeather[hourIndex];
            auto const hourlyData =
                forecastApiResponse["forecast"]["forecastday"][day]["hour"][responseIter];
            log_d("getting hourly data for %s", hourlyData["time"].as<std::string>().c_str());
            hourlyWeather.timestamp = hourlyData["time_epoch"].as<uint64_t>();
            log_v("epoch time for hour was %d", hourlyWeather.timestamp);
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
                hourlyWeather.condition = weather::Condition::windy;
            }
            hourlyWeather.pressure = hourlyData["pressure_mb"];
            hourlyWeather.humidity = hourlyData["humidity"];

            hourlyWeather.windSpeed = hourlyData["wind_mph"];
            hourlyWeather.windDirection = hourlyData["wind_degree"];
        }
        log_i("JSON successfully converted to hourly forecast weather");
        if (reorder)
        {
            log_w("Weather data was not in chronological order from API");
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