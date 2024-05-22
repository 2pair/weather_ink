#include "DailyWeather.h"

#include <WString.h>


using namespace weather;

const char* weather::conditionToString(const Condition condition)
{
    switch(condition) {
        case clear:
            return (const char *)F("Clear");
        case partlyCloudy:
            return (const char *)F("Partly Cloudy");
        case cloudy:
            return (const char *)F("Cloudy");
        case foggy:
            return (const char *)F("Foggy");
        case drizzle:
            return (const char *)F("Drizzle");
        case lightRain:
            return (const char *)F("Light Rain");
        case rain:
            return (const char *)F("Rain");
        case thunderstorm:
            return (const char *)F("Thunderstorm");
        case freezingRain:
            return (const char *)F("Freezing Rain");
        case sleet:
            return (const char *)F("Sleet");
        case snow:
            return (const char *)F("Snow");
        case unknown:
        default:
            return (const char *)F("Unknown Conditions");
    }
}

const char* weather::precipitationToString(const precipitation::Type precipitation)
{
    switch(precipitation) {
        case precipitation::none:
            return (const char *)F("None");
        case precipitation::rain:
            return (const char *)F("Rain");
        case precipitation::snow:
            return (const char *)F("Snow");
        case precipitation::unknown:
        default:
            return (const char *)F("Unknown");
    }
}
