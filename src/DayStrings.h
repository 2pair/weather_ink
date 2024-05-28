#pragma once

#include <string>
#include <WString.h>


constexpr uint32_t cSecondsPerDay = 86400U;

namespace daystrings {

// Gets the name of the day of the week for a given epoch timestamp.
std::string dayNameFromEpochTimestamp(const time_t timestamp);

}
