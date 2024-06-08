#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>

namespace weather
{

enum class Condition
{
    unknownCondition,
    clear,
    partlyCloudy,
    cloudy,
    foggy,
    drizzle,
    lightRain,
    rain,
    heavyRain,
    lightning,
    thunderstorm,
    freezingRain,
    wintryMix,
    sleet,
    snow,
    windy
};

enum class MoonPhase
{
    unknownPhase,
    newMoon,
    waxingCrescent,
    firstQuarter,
    waxingGibbous,
    fullMoon,
    waningGibbous,
    thirdQuarter,
    waningCrescent
};

struct DailyWeather
{
    // This should be a UTC unix timestamp
    uint64_t timestamp = 0;

    // offset from UTC, in hours. Needed per-day due to daylight savings time.
    // This is determined from data retrieved from the weather provider's API.
    int8_t timeZone = 0;

    float tempNow = 0.0;
    float feelsLike = 0.0;
    float tempLow = 0.0;
    float tempHigh = 0.0;

    Condition condition = Condition::unknownCondition;
    float precipitation = 0.0;
    float chanceOfPrecipitation = 0.0;

    float humidity = 0.0;
    float pressure = 0.0;
    float visibility = 0.0;

    float windSpeed = 0.0;
    float gustSpeed = 0.0;
    float windDirection = 0.0;

    // These are in local time.
    uint64_t sunrise = 0;
    uint64_t sunset = 0;
    MoonPhase moonPhase = MoonPhase::unknownPhase;
};

struct HourlyWeather
{
    // This should be a UTC unix timestamp
    uint64_t timestamp = 0;

    // offset from UTC, in hours.
    int8_t timeZone = 0;

    float temp = 0.0;
    float feelsLike = 0.0;

    Condition condition = Condition::unknownCondition;
    float chanceOfPrecipitation = 0.0; // out of 1.0
    float precipitation = 0.0;

    float humidity = 0.0;
    float pressure = 0.0;

    float windSpeed = 0.0;
    float windDirection = 0.0;
};

const std::string conditionToString(const Condition condition);

bool conditionIsWindy(Condition condition, float windSpeed);

const std::string moonPhaseToString(const MoonPhase moonPhase);
MoonPhase stringToMoonPhase(const std::string moonPhase);

bool isNightTime(const DailyWeather& dailyWeather);
}
