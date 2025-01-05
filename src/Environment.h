#pragma once

#include <vector>
#include <string>

#include <ArduinoJson.h>

class Inkplate;
namespace userconfig {
    class UserConfig;
}

namespace environment {

static constexpr size_t cCityLength = 33;
static constexpr size_t cSsidLength = 33;
static constexpr size_t cPassLength = 65;
static constexpr size_t cProviderLength = 17;
static constexpr size_t cApiKeyLength = 33;

struct Network
{
    char ssid[cSsidLength];
    char pass[cPassLength];
};

struct DataProvider
{
    char name[cProviderLength];
    char apiKey[cApiKeyLength];
};

struct Location
{
    char name[cCityLength];
    float latitude;
    float longitude;
};

struct Environment
{
    Location location;
    Network network;
    DataProvider provider;
    bool fakeApiUpdates;
    bool metricUnits;
    // CRC of all other fields, used to validate if all of the other data is valid
    // MUST BE LAST DATA IN STRUCT
    uint32_t crc;
};

void setDefaultEnvironment(Environment& env);

void setEnvironmentFromFile(
    Environment& env,
    const std::string& filename,
    Inkplate& display,
    const userconfig::UserConfig& userConfig
);

// filename: The name of the json file to load from
// listName: The key of the json list. Should be in the document root.
// defaultItem: The sole item toreturn if the list could not be found
// display: My everything
std::vector<std::string> GetListFromFile(
    const std::string& filename,
    const std::string& listName,
    const std::string& defaultItem,
    Inkplate& display
);

std::vector<std::string> GetProvidersFromFile(
    const std::string& filename,
    Inkplate& display
);

std::vector<std::string> GetNetworksFromFile(
    const std::string& filename,
    Inkplate& display
);

std::vector<std::string> GetLocationsFromFile(
    const std::string& filename,
    Inkplate& display
);

}
