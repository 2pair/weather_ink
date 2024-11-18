#include "DailyWeather.h"

#include <string>
#include <WString.h>
#include <cstdlib>
#include <esp32-hal-log.h>

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
        windSpeed >= cWindyThreshold
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

weather::MoonPhase weather::parseMoonPhase(const std::string moonPhase, const size_t illuminationPct)
{
    // WeatherApi only uses these states on the exact days of these events, be more tolerant
    if (illuminationPct >= 90) {
        log_d("illumination is %d, returning full moon", illuminationPct);
        return MoonPhase::fullMoon;
    }
    if (illuminationPct <= 10) {
        log_d("illumination is %d, returning new moon", illuminationPct);
        return MoonPhase::newMoon;
    }
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


bool weather::isNighttime(const DailyWeather& currentWeather)
{
    const auto& sunrise = timeutils::localTime(currentWeather.sunrise, currentWeather.timeZone);
    const auto& sunset = timeutils::localTime(currentWeather.sunset, currentWeather.timeZone);
    const auto nowTime = timeutils::localTime(currentWeather.timeZone);
    if (sunset != 0 && sunrise != 0)
    {
        log_d("Sunset and sunrise data available. sunrise %d sunset %d timestamp %d",
            sunrise, sunset, nowTime
        );
        if (
            std::abs(difftime(nowTime, sunset)) > cSecondsPerDay ||
            std::abs(difftime(nowTime, sunrise)) > cSecondsPerDay
        )
        {
            log_w("sunrise and sunset values are not from the current day, returning false.");
            return false;
        }
        return (
            nowTime > sunset || // nighttime before 12 AM
            nowTime < sunrise   // nighttime after 12 AM
        );
    }
    else // fall back to rough times
    {
        log_d("Sunset and sunrise data not available. using 8pm and 6am. timestamp %d", nowTime);
        auto timeOfDay = timeutils::hour24FromEpochTimestamp(nowTime);
        return timeOfDay > 20 ||  timeOfDay < 6;
    }
}

