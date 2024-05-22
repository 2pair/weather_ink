#if !defined(ARDUINO_ESP32_DEV) && !defined(ARDUINO_INKPLATE6V2)
#error                                                                                                                 \
    "Wrong board configured. Select e-radionica Inkplate6 or Soldered Inkplate6 in the boards menu."
#endif

#include <Inkplate.h>
#include <ArduinoJson.h>

// Defines all data on the location, network, and API key
#include "local_env.h"

#include "src/Network.h"
#include "src/SdCard.h"
#include "src/Weather.h"
#include "src/WeatherProvider/OpenWeatherMap.h"
#include "src/Renderer.h"
#include "src/DailyWeather.h"

#define MAX(a, b) a > b ? a : b

static constexpr uint32_t secondsPerDay = 86400;

RTC_DATA_ATTR char currentTime[16] = "--:--";
Inkplate display(INKPLATE_3BIT);
weatherprovider::OpenWeatherMap provider(lat, lon, city, apiKey);
weather::Weather weatherData(provider);

// Delay between API calls.
// 2700s equals about 1000 per month, which is the free tier limit
static constexpr uint32_t updateIntervalSeconds =
    cFakeAPIUpdates ? 60 : MAX(secondsPerDay * 30 / provider.monthlyApiCallLimit, 900U);
// When we last got a multi-day forecast
RTC_DATA_ATTR int lastForecastTime = 0;

void printCurrentConditions(const JsonDocument& jsonResponse);

void setup()
{
    Serial.begin(115200);
    display.begin();
    if (!display.sdCardInit())
    {
        display.println(F("SD Card init failure. Icon's will not be available."));
    }
    sdcard::SdCard::sleep(display);
    display.printf((const char *)F("Using %s API updates, with a query interval of %d"),
        (cFakeAPIUpdates ? F("fake") : F("real")),
        updateIntervalSeconds
    );
    Serial.printf("last forecast time: %d\n", lastForecastTime);
}

void loop()
{
    network::Network connection(ssid, pass, apiKey, timeZone);
    if (!connection.isConnected())
    {
        // Gotta get online!
        return;
    }
    bool updated = false;
    // Update forecast only once per day to reduce API calls
    if (time(nullptr) > lastForecastTime + secondsPerDay)
    {
        bool success = weatherData.updateForecast(connection);
        if (success)
        {
            lastForecastTime = time(nullptr);
            Serial.println("Forecast updated");
            updated = true;
        }
        else
        {
            Serial.println("Failed to fetch forecast data");
        }
    }

    bool success = weatherData.updateCurrent(connection);
    if (success)
    {
        Serial.println("Current weather updated");
        weatherData.printDailyWeather(weatherData.getDailyWeather(0));
        updated = true;
    }
    else
    {
        Serial.println("Failed to fetch current weather data");
    }

    renderer::Renderer renderer(display, city);
    renderer.update(weatherData);
    renderer.render();

    //connection.getTime(currentTime);
    if (updated)
    {
        Serial.printf((const char *)F("Now sleeping for %d minutes...\n"), (updateIntervalSeconds / 60));
        esp_sleep_enable_timer_wakeup(updateIntervalSeconds * 1000000L);
        display.sdCardSleep();
        esp_deep_sleep_start();
    }
}
