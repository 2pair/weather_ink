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
    none,
    rain,
    snow
};
}

struct DailyWeather
{
    uint64_t timestamp;

    float tempNow;
    float feelsLike;
    float tempLow;
    float tempHigh;

    Condition condition;
    precipitation::Type precipitationType;
    float precipitation;

    float humidity;
    float pressure;
    float visibility;

    float windSpeed;
    float gustSpeed;
    float windDirection;

    uint64_t sunrise;
    uint64_t sunset;
};

}

