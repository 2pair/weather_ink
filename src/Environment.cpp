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


using namespace environment; 

void environment::setDefaultEnvironment(Environment& env)
{
    log_d("Setting default environment.");
    strlcpy(env.location.name, cCity, cCityLength);
    env.location.latitude = cLatitude;
    env.location.longitude = cLongitude;
    strlcpy(env.network.ssid, cSsid, cSsidLength);
    strlcpy(env.network.pass, cPass, cPassLength);
    strlcpy(env.provider.name, cProvider, cProviderLength);
    strlcpy(env.provider.apiKey, cApiKey, cApiKeyLength);
    env.fakeApiUpdates = cFakeApiUpdates;
    env.metricUnits = cMetricUnits;
}

void environment::setEnvironmentFromFile(
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
    std::string ssid = envFile["networks"][0]["ssid"] | cSsid;
    strlcpy(env.network.ssid, ssid.c_str(), cSsidLength);
    std::string pass = envFile["networks"][0]["pass"] | cPass;
    strlcpy(env.network.pass, pass.c_str(), cPassLength);

    env.fakeApiUpdates = envFile["fakeApiUpdates"] | cFakeApiUpdates;
    std::string provider = envFile["providers"][0]["name"] | cProvider;
    strlcpy(env.provider.name, provider.c_str(), cProviderLength);
    std::string apiKey = envFile["providers"][0]["apiKey"] | cApiKey;
    strlcpy(env.provider.apiKey, apiKey.c_str(), cApiKeyLength);
    log_d("Provider set to: %s", env.provider);

    if (!userConfig.configUpdated())
    {
        log_d("No new config updates.");
    }
    else
    {
        log_d("Modifying config with new user input.");
        if (userConfig.locationIndexUpdated())
        {
            std::string city;
            const JsonArray& locations = envFile["locations"];
            size_t locationIndex = userConfig.getLocationIndex();
            if (locationIndex >= locations.size())
            {
                log_w("given location index is out of bounds, using default city.");
                city = cCity;
                env.location.latitude = cLatitude;
                env.location.longitude = cLongitude;
            }
            else
            {
                auto location = locations[locationIndex];
                city = location["city"] | cCity;
                env.location.latitude = location["latitude"] | cLatitude;
                env.location.longitude = location["longitude"] | cLongitude;
                log_i("city changed to %s at index %d", city.c_str(), locationIndex);
                if (
                    !(location.containsKey("city")
                    && location.containsKey("latitude")
                    && location.containsKey("longitude"))
                )
                {
                    log_w("Location object was missing one or more required keys");
                }
            }
            strlcpy(env.location.name, city.c_str(), cCityLength);
        }

        if (userConfig.useMetricUpdated())
        {
            env.metricUnits = userConfig.getUseMetric();
        }
    }

    // Set CRC for updated environment
    env.crc = crc32_le(0, envAddr, sizeof(env) - sizeof(env.crc));
    log_d("Calculated new CRC for environment data: %x", env.crc);
}

std::vector<std::string> environment::GetListFromFile(
    const std::string& filename,
    const std::string& listName,
    const std::string& defaultItem,
    Inkplate& display
)
{
    std::vector<std::string> items;
    sdcard::SdCard sdCard(display);
    JsonDocument envFile;
    if (sdCard.readJsonFile(envFile, filename))
    {
        const JsonArray& jsonItems = envFile[listName];
        log_d("%s list has %d items", listName.c_str(), jsonItems.size());
        for (size_t i=0; i<jsonItems.size(); i++)
        {
            const std::string item = jsonItems[i]["name"];
            items.emplace_back(item);
        }
    }
    if (items.empty())
    {
        // If the json file is not setup correctly
        log_d("json list is empty, seeding with default value, %s", defaultItem);
        items.emplace_back(defaultItem);
    }
    return items;
}

std::vector<std::string> environment::GetProvidersFromFile(
    const std::string& filename,
    Inkplate& display
)
{
    return GetListFromFile(
        filename,
        "providers",
        cSsid,
        inkplate
    );
}

std::vector<std::string> environment::GetNetworksFromFile(
    const std::string& filename,
    Inkplate& display
)
{
    return GetListFromFile(
        filename,
        "networks",
        cProvider,
        inkplate
    );
}

std::vector<std::string> environment::GetLocationsFromFile(
    const std::string& filename,
    Inkplate& display
)
{
    return GetListFromFile(
        filename,
        "locations",
        cCity,
        inkplate
    );
}
