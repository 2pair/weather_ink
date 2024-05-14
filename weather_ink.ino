#if !defined(ARDUINO_ESP32_DEV) && !defined(ARDUINO_INKPLATE6V2)
#error                                                                                                                 \
    "Wrong board configured. Select e-radionica Inkplate6 or Soldered Inkplate6 in the boards menu."
#endif

// Defines all data on the location, network, and API key
#include "local_env.h"

#include "src/Network.h"
#include "src/SdCard.h"
#include "src/Weather.h"
#include "src/WeatherProvider/OpenWeatherMap.h"
#include "src/Renderer.h"

#include <Inkplate.h>
#include <ArduinoJson.h>

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
    display.println("Here");
    display.println(F("We Go"));
}

void loop()
{
    network::Network connection(ssid, pass, apiKey, timeZone);
    if (!connection.isConnected())
    {
        // Gotta get online!
        return;
    }

    JsonDocument jsonResponse;
    bool updated = false;
    // Update forecast only once per day to reduce API calls
    if (time(nullptr) > lastForecastTime + secondsPerDay)
    {
        bool success = weatherData.updateForecast(jsonResponse, connection);
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
        jsonResponse.clear();
    }

    bool success = weatherData.updateCurrent(jsonResponse, connection);
    if (success)
    {
        Serial.println("Current weather updated");
        printCurrentConditions(jsonResponse);
        updated = true;
    }
    else
    {
        Serial.println("Failed to fetch current weather data");
    }
    jsonResponse.clear();

    renderer::Renderer renderer(display, city);
    renderer.update(weatherData);
    display.display();

    //connection.getTime(currentTime);
    if (updated)
    {
        Serial.printf((const char *)F("Now sleeping for %d minutes...\n"), (updateIntervalSeconds / 60));
        esp_sleep_enable_timer_wakeup(updateIntervalSeconds * 1000000L);
        display.sdCardSleep();
        esp_deep_sleep_start();
    }
}

void printCurrentConditions(const JsonDocument& jsonResponse) {
    Serial.print("Condition code: ");
    Serial.println(jsonResponse["weather"][0]["id"].as<int>());
    Serial.print("Condition: ");
    Serial.println(jsonResponse["weather"][0]["main"].as<const char*>());
    Serial.print("Description: ");
    Serial.println(jsonResponse["weather"][0]["description"].as<const char*>());
    Serial.print("Temp: ");
    Serial.println(jsonResponse["main"]["temp"].as<float>());
    Serial.print("Wind speed: ");
    Serial.println(jsonResponse["wind"]["speed"].as<float>());
    Serial.print("unix timestamp: ");
    Serial.println(jsonResponse["dt"].as<int64_t>());
    Serial.print("Timezone: ");
    Serial.println(jsonResponse["timezone"].as<int>());
    Serial.print("City: ");
    Serial.println(jsonResponse["name"].as<const char*>());
}
