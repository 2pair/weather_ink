#if !defined(ARDUINO_ESP32_DEV) && !defined(ARDUINO_INKPLATE6V2)
#error                                                                                                                 \
    "Wrong board configured. Select e-radionica Inkplate6 or Soldered Inkplate6 in the boards menu."
#endif

#include <Arduino.h>
#include <Inkplate.h>
#include <ArduinoJson.h>

#include "src/Environment.h"
#include "src/Weather.h"
#include "src/WeatherProvider/OpenWeatherMap.h"
#include "src/WeatherProvider/WeatherApi.h"
#include "src/DailyWeather.h"
#include "src/TimeUtils.h"
#include "src/Network.h"
#include "src/SdCard.h"
#include "src/Renderer.h"
#include "src/WeatherTypes.h"


Inkplate gDisplay(INKPLATE_3BIT);
RTC_DATA_ATTR Environment env;
RTC_DATA_ATTR weather::Weather gWeather(gDisplay);

/* Returns how long to wait until the next update, in seconds.
   It the weather could not be updated returns 0.   
*/
bool updateWeather(weather::Weather& weatherData, const weatherprovider::WeatherProvider& provider);
void draw(const weather::Weather& weatherData);

void setup()
{
    Serial.begin(115200);
    log_d("gDisplay._beginDone is %d\n", gDisplay._beginDone);
    int result = gDisplay.begin();
    log_d("Return code was %d\n", result);
    log_d("gDisplay._beginDone is now %d\n", gDisplay._beginDone);
    if (result)
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

    env = setEnvironmentFromFile("/env.json", gDisplay);
    gWeather.fakeUpdates(env.fakeApiUpdates);

    gDisplay.setRotation(1);
    Serial.printf((const char*)F("last forecast time: %llu\n"), static_cast<uint64_t>(gWeather.getLastForecastTime()));
}

void loop()
{
    constexpr uint32_t cMinSleepTimeSecs = 15 * 1000000L;
    uint32_t sleepTimeSecs = 0;
    if (std::string(env.provider) == "WeatherApi")
    {
        weatherprovider::WeatherApi provider(env.latitude, env.longitude, env.city, env.apiKey);
        sleepTimeSecs = updateWeather(gWeather, provider);
    }
    else // Only other provider is OpenWeatherMap
    {
        weatherprovider::OpenWeatherMap provider(env.latitude, env.longitude, env.city, env.apiKey);
        sleepTimeSecs = updateWeather(gWeather, provider);
    }
    if (sleepTimeSecs)
    {
        draw(gWeather);
    }
    else
    {
        // weather was not updated. Try again after a little
        sleepTimeSecs = cMinSleepTimeSecs;
    }
    Serial.printf((const char *)F("Now sleeping for %u seconds (%u minutes)...\n"),
        sleepTimeSecs,
        (sleepTimeSecs / 60)
    );
    Serial.end();
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
    esp_sleep_enable_timer_wakeup(sleepTimeSecs * 1000000L);
    sdcard::SdCard::sleep(gDisplay);
    esp_deep_sleep_start();
}

bool updateWeather(weather::Weather& weatherData, const weatherprovider::WeatherProvider& provider)
{
    network::Network connection(env.ssid, env.pass);
    // Delay between API calls. 1 minute when reading from SD, minimum 15 minutes otherwise.
    uint32_t sleepTimeSecs =
        env.fakeApiUpdates ? 60 : std::max(provider.getWeatherUpdateIntervalSeconds(), 900U);
    Serial.printf((const char *)F("Using %s API updates, with a query interval of %u seconds\n"),
        (env.fakeApiUpdates ? (const char *)F("mock") : (const char *)F("real")),
        sleepTimeSecs
    );
    // delete
    sleepTimeSecs = 60;
    bool updated = false;
    if (!connection.isConnected())
    {
        // Gotta get online!
        return 0;
    }
    static constexpr uint8_t retries = 10;
    static constexpr uint16_t retryDelaySeconds = 6;
    for (uint8_t retry = 0; retry < retries; retry++)
    {
        if (weatherData.updateWeather(connection, provider))
        {
            Serial.println(F("Forecast updated"));
            weatherData.printDailyWeather(weatherData.getDailyWeather(0));
            updated = true;
            break;
        }
        else
        {
            Serial.println(F("Failed to fetch current weather data"));
        }
        delay(retryDelaySeconds * 1000);
    }
    return updated ? sleepTimeSecs : 0;
}

void draw(const weather::Weather& weatherData)
{
    renderer::Renderer renderer(gDisplay, env.city);
    renderer.update(weatherData);
    renderer.render();
}
