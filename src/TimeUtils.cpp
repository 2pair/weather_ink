#include "TimeUtils.h"

#include <Arduino.h>
#include <string>

using namespace timeutils;


static auto sunday =  F("Sunday");
static auto monday = F("Monday");
static auto tuesday = F("Tuesday");
static auto wednesday = F("Wednesday");
static auto thursday = F("Thursday");
static auto friday = F("Friday");
static auto saturday = F("Saturday");

time_t timeutils::localTime(const time_t epochTime, const int8_t tzOffset)
{
    return epochTime + (tzOffset * cSecondsPerHour);
}

time_t timeutils::localTime(const int8_t tzOffset)
{
    auto timeNow = localTime(time(nullptr), tzOffset);
    Serial.printf("Current epoch time in timezone is %llu\n", timeNow);
    return timeNow;
}

std::string timeutils::dayNameFromEpochTimestamp(const time_t timestamp)
{
    static const char* const dayArr[7] = {
        (const char*)sunday,
        (const char*)monday,
        (const char*)tuesday,
        (const char*)wednesday,
        (const char*)thursday,
        (const char*)friday,
        (const char*)saturday
    };
    static constexpr size_t cStartingOffset = 4; // Unix time starts on a Thursday
    static constexpr size_t cDaysPerWeek = 7;
    // +1 so that midnight is counted as the following day instead of the preceding day
    auto day = (((timestamp + 1) / cSecondsPerDay) + cStartingOffset) % cDaysPerWeek;
    return std::string(dayArr[day]);
}

size_t timeutils::hour24FromEpochTimestamp(const time_t timestamp)
{
    return (timestamp % cSecondsPerDay) / cSecondsPerHour;
}

std::string timeutils::hour12FromEpochTimestamp(const time_t timestamp)
{
    auto hourOfDay = hour24FromEpochTimestamp(timestamp);
    auto hourAmPm = hourOfDay % 12;
    if (hourAmPm == 0)
    {
        hourAmPm = 12;
    }
    return std::to_string(hourAmPm) + ((hourOfDay >= 12) ? "PM" : "AM");
}

time_t timeutils::timeStrToEpochTime(const std::string &timeString, const std::string &format)
{
    tm timeInfo;
    strptime(timeString.c_str(), format.c_str(), &timeInfo);
    return mktime(&timeInfo);
}
