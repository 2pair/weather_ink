#pragma once

#include <ArduinoJson.h>

// foreword declaration
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

bool fakeApiUpdates;
};

Environment setEnvironmentFromFile(const std::string& filename, Inkplate& display);