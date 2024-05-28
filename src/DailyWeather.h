#pragma once

#include <stdint.h>
#include <string>

namespace weather
{

enum Condition
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

enum MoonPhase
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

// Add a namespace to reuse names from other namespaces
namespace precipitation
{
enum Type
{
    unknown,
    none,
    rain,
    snow
};
}

struct DailyWeather
{
    // This should be a unix timestamp in the local timezone.
    // Since the APIs offer the time in local time, or at least UTC and an offset,
    // it is easier to just work with local time.
    uint64_t timestamp = 0;

    float tempNow = 0.0;
    float feelsLike = 0.0;
    float tempLow = 0.0;
    float tempHigh = 0.0;

    Condition condition = Condition::unknownCondition;
    precipitation::Type precipitationType = precipitation::Type::unknown;
    float precipitation = 0.0;

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
    // This should be a unix timestamp in the local timezone.
    uint64_t timestamp = 0;

    float tempNow = 0.0;
    float feelsLike = 0.0;

    Condition condition = Condition::unknownCondition;
    float changeOfPrecipitation = 0.0; // out of 1.0
    float precipitation = 0.0;

    float humidity = 0.0;
    float pressure = 0.0;

    float windSpeed = 0.0;
    float windDirection = 0.0;
};

const std::string conditionToString(const Condition condition);

const std::string precipitationToString(const precipitation::Type precipitation);

const bool conditionIsWindy(const DailyWeather& dailyWeather);
}
