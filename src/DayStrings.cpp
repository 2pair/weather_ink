#include "DayStrings.h"

#include <string>


using namespace daystrings;


static auto sunday =  F("Sunday");
static auto monday = F("Monday");
static auto tuesday = F("Tuesday");
static auto wednesday = F("Wednesday");
static auto thursday = F("Thursday");
static auto friday = F("Friday");
static auto saturday = F("Saturday");

// Gets the name of the day of the week for a given epoch timestamp.
std::string daystrings::dayNameFromEpochTimestamp(const time_t timestamp)
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
    static constexpr size_t startingOffset = 3; // Unix time starts on a Thursday
    auto day = ((timestamp / cSecondsPerDay) + startingOffset) % 7;
    return std::string(dayArr[day]);
}
