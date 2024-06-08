#include "DailyWeather.h"

#include <string>
#include <WString.h>

#include "TimeUtils.h"


using namespace weather;

const std::string  weather::conditionToString(const Condition condition)
{
    switch(condition)
    {
        case Condition::clear:
            return (const char *)F("Clear");
        case Condition::partlyCloudy:
            return (const char *)F("Partly Cloudy");
        case Condition::cloudy:
            return (const char *)F("Cloudy");
        case Condition::foggy:
            return (const char *)F("Foggy");
        case Condition::drizzle:
            return (const char *)F("Drizzle");
        case Condition::lightRain:
            return (const char *)F("Light Rain");
        case Condition::rain:
            return (const char *)F("Rain");
        case Condition::heavyRain:
            return (const char *)F("Heavy Rain");
        case Condition::lightning:
            return (const char *)F("Lightning");
        case Condition::thunderstorm:
            return (const char *)F("Thunderstorm");
        case Condition::freezingRain:
            return (const char *)F("Freezing Rain");
        case Condition::sleet:
            return (const char *)F("Sleet");
        case Condition::snow:
            return (const char *)F("Snow");
        case Condition::wintryMix:
            return (const char *)F("Wintry Mix");
        case Condition::windy:
            return (const char *)F("Windy");
        case Condition::unknownCondition:
        default:
            return (const char *)F("Unknown Conditions");
    }
}

bool weather::conditionIsWindy(Condition condition, float windSpeed)
{
    return
    condition == Condition::windy ||
    (
        (
            condition == Condition::clear ||
            condition == Condition::partlyCloudy
        ) &&
        windSpeed >= 20 // mph
    );
}

const std::string weather::moonPhaseToString(const MoonPhase moonPhase)
{
    switch(moonPhase)
    {
        case MoonPhase::newMoon:
            return (const char *)F("New Moon");
        case MoonPhase::waxingCrescent:
            return (const char *)F("Waxing Crescent");
        case MoonPhase::firstQuarter:
            return (const char *)F("First Quarter");
        case MoonPhase::waxingGibbous:
            return (const char *)F("Waxing Gibbous");
        case MoonPhase::fullMoon:
            return (const char *)F("Full Moon");
        case MoonPhase::waningGibbous:
            return (const char *)F("Waning Gibbous");
        case MoonPhase::thirdQuarter:
            return (const char *)F("Third Quarter");
        case MoonPhase::waningCrescent:
            return (const char *)F("Waning Crescent");
        case MoonPhase::unknownPhase:
        default:
            return (const char *)F("Unknown Phase");
    }
}

weather::MoonPhase weather::stringToMoonPhase(const std::string moonPhase)
{
    static const std::unordered_map<std::string, MoonPhase> moonMap = {
        {(const char *)F("New Moon"), MoonPhase::newMoon},
        {(const char *)F("Waxing Crescent"), MoonPhase::waxingCrescent},
        {(const char *)F("First Quarter"), MoonPhase::firstQuarter},
        {(const char *)F("Waxing Gibbous"), MoonPhase::waxingGibbous},
        {(const char *)F("Full Moon"), MoonPhase::fullMoon},
        {(const char *)F("Waning Gibbous"), MoonPhase::waningGibbous},
        {(const char *)F("Third Quarter"), MoonPhase::thirdQuarter},
        {(const char *)F("Waning Crescent"), MoonPhase::waningCrescent}
    };
    auto itr = moonMap.find(moonPhase);
    if (itr == moonMap.end())
    {
        return MoonPhase::unknownPhase;
    }
    return itr->second;
}


bool weather::isNightTime(const DailyWeather& dailyWeather)
{
    if (dailyWeather.sunset != 0 and dailyWeather.sunrise != 0)
    {
        return (
            dailyWeather.timestamp > dailyWeather.sunset || // before 12 AM
            dailyWeather.timestamp < dailyWeather.sunrise  // after 12 AM
        );
    }
    else // fall back to rough times
    {
        auto timeOfDay = timeutils::hour24FromEpochTimestamp(dailyWeather.timestamp);
        return timeOfDay > 20 ||  timeOfDay < 6;
    }
}

