#pragma once

#include <string>
#include <WString.h>


constexpr uint32_t cSecondsPerHour = 3600U;
constexpr uint32_t cSecondsPerDay = cSecondsPerHour * 24;


namespace timeutils {

// translate the given epoch time, to the local timezone
time_t localTime(const time_t epochTime, const int8_t tzOffset);
// get the current epoch time, with the local timezone offset
time_t localTime(const int8_t tzOffset);

// Gets the index of the day of the week for a given epoch timestamp.
// Sunday == 0, Saturday == 6
size_t dayIndexFromEpochTimestamp(const time_t timestamp);

// Gets the name of the day of the week for a given epoch timestamp.
std::string dayNameFromEpochTimestamp(const time_t timestamp);

// Gets the hour of the day for a given epoch timestamp, in 24 hour clock format.
size_t hour24FromEpochTimestamp(const time_t timestamp);

// Gets the hour of the day for a given epoch timestamp, in 12 hour AM/PM clock format.
std::string hour12FromEpochTimestamp(const time_t timestamp);

time_t timeStrToEpochTime(const std::string& timeString, const std::string& format);

}
