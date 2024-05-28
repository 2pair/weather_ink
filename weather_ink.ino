#if !defined(ARDUINO_ESP32_DEV) && !defined(ARDUINO_INKPLATE6V2)
#error                                                                                                                 \
    "Wrong board configured. Select e-radionica Inkplate6 or Soldered Inkplate6 in the boards menu."
#endif

#include <Inkplate.h>
#include <ArduinoJson.h>

// Defines all data on the location, network, and API key
#include "local_env.h"

#include "src/Weather.h"
#include "src/WeatherProvider/OpenWeatherMap.h"
#include "src/WeatherProvider/WeatherApi.h"
#include "src/DailyWeather.h"
#include "src/DayStrings.h"
#include "src/Network.h"
#include "src/SdCard.h"
#include "src/Renderer.h"


Inkplate gDisplay(INKPLATE_3BIT);
RTC_DATA_ATTR weather::Weather gWeather(gDisplay);
uint32_t gUpdateIntervalSeconds = 0;

bool updateWeather();
void draw();

void setup()
{
    Serial.begin(115200);
    int result = gDisplay.begin();
    Serial.printf("Return code was %d\n", result);
    Serial.printf("gDisplay._beginDone is %d\n", gDisplay._beginDone);
    if (false)
    {
        Serial.println(F("WARNING: board init failure. Cannot continue. restarting device in 10 seconds..."));
        delay(10000);
        esp_restart();
    }
    gDisplay.clearDisplay();
    if (!gDisplay.sdCardInit())
    {
        Serial.println(F("WARNING: SD Card init failure. Icon's will not be available."));
    }
    sdcard::SdCard::sleep(gDisplay);
    Serial.printf((const char*)F("last forecast time: %llu\n"), static_cast<uint64_t>(gWeather.getLastForecastTime()));
}

void loop()
{
    if (updateWeather())
    {
        draw();
        // Only sleep if we got an update and we have forecast data, otherwise keep trying
        if (gWeather.getLastForecastTime() != 0)
        {
            Serial.printf((const char *)F("Now sleeping for %u minutes...\n"), (gUpdateIntervalSeconds / 60));
            esp_sleep_enable_timer_wakeup(gUpdateIntervalSeconds * 1000000L);
            sdcard::SdCard::sleep(gDisplay);
            esp_deep_sleep_start();
        }
    }
}

bool updateWeather()
{
    network::Network connection(ssid, pass, timeZone);
    weatherprovider::OpenWeatherMap provider(lat, lon, city, apiKeyOpenWeatherMap);
    // Delay between API calls.
    // 2700s equals about 1000 per month, which is the free tier limit for Open Weather Map
    gUpdateIntervalSeconds =
        cFakeAPIUpdates ? 60 : std::max(cSecondsPerDay * 30 / provider.monthlyApiCallLimit, 900U);
    Serial.printf((const char *)F("Using %s API updates, with a query interval of %u seconds\n"),
        (cFakeAPIUpdates ? (const char *)F("mock") : (const char *)F("real")),
        gUpdateIntervalSeconds
    );
    bool updated = false;
    if (!connection.isConnected())
    {
        // Gotta get online!
        return false;
    }
    static constexpr uint8_t retries = 10;
    static constexpr uint16_t retryDelaySeconds = 6;
    for (uint8_t retry = 0; retry < retries; retry++)
    {
        // Update forecast only once per day to reduce API calls
        if (time(nullptr) > gWeather.getLastForecastTime() + cSecondsPerDay)
        {
            if (gWeather.updateForecast(connection, provider))
            {
                Serial.println(F("Forecast updated"));
                updated = true;
            }
            else
            {
                Serial.println(F("Failed to fetch forecast data"));
            }
        }

        if (gWeather.updateCurrent(connection, provider))
        {
            Serial.println(F("Current weather updated"));
            gWeather.printDailyWeather(gWeather.getDailyWeather(0));
            updated = true;
            break;
        }
        else
        {
            Serial.println(F("Failed to fetch current weather data"));
        }
        delay(retryDelaySeconds * 1000);
    }
    return updated;
}

void draw()
{
    renderer::Renderer renderer(gDisplay, city);
    renderer.update(gWeather);
    renderer.render();
}
