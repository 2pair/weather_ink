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
        log_d("environment was invalid. Actual CRC was %x", env.crc);
    }
    else if (!userConfig.configUpdated())
    {
        log_d("environment was valid and config was not updated.");
        return;
    }

    JsonDocument envFile;
    sdcard::SdCard sdCard(display);
    if (sdCard.openFile(filename))
    {
        sdCard.readJsonFile(envFile, filename);

        std::string city;
        const JsonArray& locations = envFile["locations"];
        size_t locationIndex = 0;
        if (userConfig.configUpdated() && userConfig.locationIndexUpdated())
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
        else if (envIsInvalid)
        {
            log_i("Environment is invalid, resetting location.");
            city = locations[locationIndex]["city"] | cCity;
            env.latitude = locations[locationIndex]["latitude"] | cLatitude;
            env.longitude = locations[locationIndex]["longitude"] | cLongitude;
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

        if (userConfig.configUpdated() && userConfig.useMetricUpdated())
        {
            env.metricUnits = userConfig.getUseMetric();
        }
        else if (envIsInvalid)
        {
            env.metricUnits =  envFile["metric_units"] | cMetricUnits;
        }
    }
    else
    {
        // Set all to default values
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
    if (sdCard.openFile(filename))
    {
        JsonDocument envFile;
        sdCard.readJsonFile(envFile, filename);

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
