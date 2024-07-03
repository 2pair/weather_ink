#include "TimeUtils.h"

#include <esp32-hal-log.h>
#include <Arduino.h>
#include <string>
#include <time.h>

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
    return epochTime + (static_cast<uint32_t>(tzOffset) * cSecondsPerHour);
}

time_t timeutils::localTime(const int8_t tzOffset)
{
    auto timeUtc = time(nullptr);
    if (timeUtc == (time_t)-1)
    {
        log_w("Time function was not able to determine the calendar time");
    }
    auto timeNow = localTime(timeUtc, tzOffset);
    log_v("Current epoch time is %d, with tz_offset of %d in timezone is %d", timeUtc, tzOffset, timeNow);
    return timeNow;
}

size_t timeutils::dayIndexFromEpochTimestamp(const time_t timestamp)
{
    static constexpr size_t cStartingOffset = 4; // Unix time starts on a Thursday
    static constexpr size_t cDaysPerWeek = 7;
    // +1 so that midnight is counted as the following day instead of the preceding day
    const auto day = (((timestamp + 1) / cSecondsPerDay) + cStartingOffset) % cDaysPerWeek;
    log_d("Timestamp %d mapped to %d", timestamp, day);
    return day;
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
    const auto day = dayIndexFromEpochTimestamp(timestamp);
    log_d("Timestamp %d mapped to %s", timestamp, dayArr[day]);
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
    log_d("timestr %s format %s", timeString.c_str(), format.c_str());
    return mktime(&timeInfo);
}
