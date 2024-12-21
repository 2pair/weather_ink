#pragma once

#include <vector>
#include <string>

#include <ArduinoJson.h>

class Inkplate;
namespace userconfig {
    class UserConfig;
}

static constexpr uint cCityLength = 17;
static constexpr uint cSsidLength = 33;
static constexpr uint cPassLength = 65;
static constexpr uint cProviderLength = 17;
static constexpr uint cApiKeyLength = 33;

struct Environment
{
char city[cCityLength];
float latitude;
float longitude;

char ssid[cSsidLength];
char pass[cPassLength];

char provider[cProviderLength];
char apiKey[cApiKeyLength];

bool fakeApiUpdates = false;

bool metricUnits = false;

// CRC of all other fields, used to validate if all of the other data is valid
// MUST BE LAST DATA IN STRUCT
uint32_t crc;
};

void setEnvironmentFromFile(
    Environment& env,
    const std::string& filename,
    Inkplate& display,
    const userconfig::UserConfig& userConfig
);

std::vector<std::string> GetLocationsFromFile(
    const std::string& filename,
    Inkplate& display
);
