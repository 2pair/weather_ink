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

struct TemporalWeather {
    // This should be a UTC unix timestamp
    time_t timestamp = 0;
    // offset from UTC, in hours. Needed per-day due to daylight savings time.
    // This is determined from data retrieved from the weather provider's API.
    int8_t timeZone = 0;
    // All values should either be metric or imperial
    bool metricUnits = false;

    float tempNow = 0.0;
    float feelsLike = 0.0;

    Condition condition = Condition::unknownCondition;
    float chanceOfPrecipitation = 0.0; // out of 1.0
    float precipitation = 0.0;

    uint8_t humidity = 0.0;
    float pressure = 0.0;

    float windSpeed = 0.0;
    int16_t windDirection = 0;

    MoonPhase moonPhase = MoonPhase::unknownPhase;
};

struct DailyWeather : TemporalWeather
{
    float tempLow = 0.0;
    float tempHigh = 0.0;

    float visibility = 0.0;

    float gustSpeed = 0.0;

    // These are in UTC.
    time_t sunrise = 0;
    time_t sunset = 0;
};

struct HourlyWeather : TemporalWeather
{
    // If this hour occurs while the sun is up
    bool daytime = true;
};

const std::string conditionToString(const Condition condition);

constexpr size_t cWindyThreshold = 15; // mph
bool conditionIsWindy(Condition condition, float windSpeed);
std::string windDegreeToDirection(uint16_t degrees);

const std::string moonPhaseToString(const MoonPhase moonPhase);
MoonPhase parseMoonPhase(const std::string moonPhase, const size_t illuminationPct);

// returns true is the current time is before sunrise or after sunset, else false.
// default values will be used if either sunrise or sunset equals 0.
// Note: If sunrise and sunset are not the values for the current day then this function
// will always return false.
bool isNighttime(const DailyWeather& currentWeather);
}
