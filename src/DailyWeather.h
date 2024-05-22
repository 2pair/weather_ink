#pragma once

#include <stdint.h>


namespace weather
{

enum Condition
{
    unknown,
    clear,
    partlyCloudy,
    cloudy,
    foggy,
    drizzle,
    lightRain,
    rain,
    thunderstorm,
    freezingRain,
    sleet,
    snow,
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
    uint64_t timestamp = 0;

    float tempNow = 0.0;
    float feelsLike = 0.0;
    float tempLow = 0.0;
    float tempHigh = 0.0;

    Condition condition = Condition::unknown;
    precipitation::Type precipitationType = precipitation::Type::unknown;
    float precipitation = 0.0;

    float humidity = 0.0;
    float pressure = 0.0;
    float visibility = 0.0;

    float windSpeed = 0.0;
    float gustSpeed = 0.0;
    float windDirection = 0.0;

    uint64_t sunrise = 0;
    uint64_t sunset = 0;
};

const char* conditionToString(const Condition condition);

const char* precipitationToString(const precipitation::Type precipitation);

}
