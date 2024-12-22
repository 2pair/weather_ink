#include "Environment.h"

#include <stdint.h>
#include <vector>
#include <string>

#include <Inkplate.h>
#include <esp32-hal-log.h>
#include <rom/crc.h>
#include <ArduinoJson.h>

#include "../default_env.h"
#include "SdCard.h"
#include "UserConfig.h"


void setDefaultEnvironment(Environment& env)
{
    log_d("Setting default environment.");
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
    env.metricUnits = cMetricUnits;
}

void setEnvironmentFromFile(
    Environment& env,
    const std::string& filename,
    Inkplate& display,
    const userconfig::UserConfig& userConfig)
{
    // Exclude the CRC when calculating the CRC
    auto envAddr = reinterpret_cast<uint8_t*>(&env);
    uint32_t calculatedCrc = crc32_le(0, envAddr, sizeof(env) - sizeof(env.crc));
    log_d("Calculated environment CRC as %x", calculatedCrc);

    bool envIsInvalid = (env.crc == 0 || calculatedCrc != env.crc);
    if (envIsInvalid)
    {
        // Generally this will happen on first boot
        log_d("environment was invalid. Actual CRC was %x", env.crc);
        setDefaultEnvironment(env);
        env.crc = crc32_le(0, envAddr, sizeof(env) - sizeof(env.crc));
    }
    else{
        log_d("CRCs matched, environment is valid");
    }

    JsonDocument envFile;
    sdcard::SdCard sdCard(display);
    if (!sdCard.readJsonFile(envFile, filename))
    {
        log_d("Could not read from SD card, cannot update environment");
        return;
    }

    log_d("Network and API configuration will be re-read from SD");
    // Always re-read these critical elements
    std::string ssid = envFile["ssid"] | cSsid;
    strlcpy(env.ssid, ssid.c_str(), cSsidLength);
    std::string pass = envFile["pass"] | cPass;
    strlcpy(env.pass, pass.c_str(), cPassLength);

    env.fakeApiUpdates = envFile["fakeApiUpdates"] | cFakeApiUpdates;
    std::string provider = envFile["primaryProvider"] | cProvider;
    if (provider == std::string((const char*)F("OpenWeatherMap")))
    {
        std::string apiKey = envFile["apiKeyOpenWeatherMap"] | cApiKeyOpenWeatherMap;
        strlcpy(env.apiKey, apiKey.c_str(), cApiKeyLength);
        strlcpy(env.provider, provider.c_str(), cProviderLength);
    }
    else if (provider == std::string((const char*)F("WeatherApi")))
    {
        std::string apiKey = envFile["apiKeyWeatherApi"] | cApiKeyWeatherApi;
        strlcpy(env.apiKey, apiKey.c_str(), cApiKeyLength);
        strlcpy(env.provider, provider.c_str(), cProviderLength);
    }
    else {
        log_w("Unknown provider: %s", env.provider);
    }

    if (!userConfig.configUpdated())
    {
        log_d("Config was not updated.");
    }
    else
    {
        log_d("Config was updated.");
        std::string city;
        const JsonArray& locations = envFile["locations"];
        size_t locationIndex = 0;
        if (userConfig.locationIndexUpdated())
        {
            locationIndex = userConfig.getLocationIndex();
            if (locationIndex >= locations.size())
            {
                log_w("given location index is out of bounds, using default city.");
                city = cCity;
                env.latitude = cLatitude;
                env.longitude = cLongitude;
            }
            else
            {
                city = locations[locationIndex]["city"] | cCity;
                env.latitude = locations[locationIndex]["latitude"] | cLatitude;
                env.longitude = locations[locationIndex]["longitude"] | cLongitude;
                log_i("city changed to %s from index %d", city, locationIndex);
            }
        }
        strlcpy(env.city, city.c_str(), cCityLength);

        if (userConfig.useMetricUpdated())
        {
            env.metricUnits = userConfig.getUseMetric();
        }
    }

    // Set CRC for updated environment
    env.crc = crc32_le(0, envAddr, sizeof(env) - sizeof(env.crc));
    log_d("Calculated new CRC for environment data: %x", env.crc);
}

std::vector<std::string> GetLocationsFromFile(
    const std::string& filename,
    Inkplate& display
)
{
    std::vector<std::string> locations;
    sdcard::SdCard sdCard(display);
    JsonDocument envFile;
    if (sdCard.readJsonFile(envFile, filename))
    {
        const JsonArray& jsonLocations = envFile["locations"];
        log_d("locations list has %d items", locations.size());
        for (size_t i=0; i<jsonLocations.size(); i++)
        {
            const std::string city = jsonLocations[i]["city"];
            locations.emplace_back(city);
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
