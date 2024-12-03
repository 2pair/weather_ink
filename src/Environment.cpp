#include "Environment.h"

#include <vector>
#include <string>

#include <Inkplate.h>
#include <esp32-hal-log.h>
#include <ArduinoJson.h>

#include "../default_env.h"
#include "SdCard.h"

Environment setEnvironmentFromFile(
    const std::string& filename,
    Inkplate& display,
    size_t locationIndex)
{
    Environment env;
    JsonDocument envFile;
    sdcard::SdCard sdCard(display);
    if (sdCard.openFile(filename))
    {
        sdCard.readJsonFile(envFile, filename);

        std::string city;
        const JsonArray& locations = envFile["locations"];
        if (locationIndex >= locations.size())
        {
            log_w("Given location index is out of bounds.");
            city = cCity;
            env.latitude = cLatitude;
            env.longitude = cLongitude;
        }
        else
        {
            city = envFile["city"] | cCity;
            env.latitude = envFile["latitude"] | cLatitude;
            env.longitude = envFile["longitude"] | cLongitude;
        }
        strlcpy(env.city, city.c_str(), cCityLength);

        std::string ssid = envFile["ssid"] | cSsid;
        strlcpy(env.ssid, ssid.c_str(), cSsidLength);
        std::string pass = envFile["pass"] | cPass;
        strlcpy(env.pass, pass.c_str(), cPassLength);

        std::string provider = envFile["primaryProvider"] | cProvider;
        strlcpy(env.provider, provider.c_str(), cProviderLength);
        if (provider == std::string((const char*)F("OpenWeatherMap")))
        {
            std::string apiKey = envFile["apiKeyOpenWeatherMap"] | cApiKeyOpenWeatherMap;
            strlcpy(env.apiKey, apiKey.c_str(), cApiKeyLength);
        }
        else if (provider == std::string((const char*)F("WeatherApi")))
        {
            std::string apiKey = envFile["apiKeyWeatherApi"] | cApiKeyWeatherApi;
            strlcpy(env.apiKey, apiKey.c_str(), cApiKeyLength);
        }
        else {
            log_w("Unknown provider: %s", env.provider);
        }

        env.fakeApiUpdates = envFile["fakeApiUpdates"] | cFakeApiUpdates;
    }
    else
    {
        strlcpy(env.city, cCity, cCityLength);
        env.latitude = cLatitude;
        env.longitude = cLongitude;
        strlcpy(env.ssid, cSsid, cSsidLength);
        strlcpy(env.pass, cPass, cPassLength);
        strlcpy(env.provider, cProvider, cProviderLength);
        if (strcmp(env.provider, "OpenWeatherMap") == 0)
        {
            strlcpy(env.apiKey, cApiKeyOpenWeatherMap, cApiKeyLength);
        }
        else if (strcmp(env.provider, "WeatherApi") == 0)
        {
            strlcpy(env.apiKey, cApiKeyWeatherApi, cApiKeyLength);
        }
        else {
            log_e("No API key provided!");
        }
        env.fakeApiUpdates = cFakeApiUpdates;
    }

    return env;
}

std::vector<std::string> GetLocationsFromFile(
    const std::string& filename,
    Inkplate& display
)
{
    std::vector<std::string> locations;
    JsonDocument envFile;
    sdcard::SdCard sdCard(display);
    if (sdCard.openFile(filename))
    {
        sdCard.readJsonFile(envFile, filename);

        std::string city;
        const JsonArray& locations = envFile["locations"];
        for (auto location : locations)
        {
            locations.add(location["name"]);
        }
    }
    if (locations.empty())
    {
        // If the json file is not setup correctly
        log_d("json list is empty, seeding with default city, %s", cCity);
        locations.emplace_back(cCity);
    }
    return locations;
}
