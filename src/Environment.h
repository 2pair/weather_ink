#pragma once

#include <vector>
#include <string>

#include <ArduinoJson.h>

class Inkplate;


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
};

Environment setEnvironmentFromFile(
    const std::string& filename,
    Inkplate& display,
    size_t locationIndex=0
);

std::vector<std::string> GetLocationsFromFile(
    const std::string& filename,
    Inkplate& display
);
