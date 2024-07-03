#if !defined(ARDUINO_ESP32_DEV) && !defined(ARDUINO_INKPLATE6V2)
#error                                                                                                                 \
    "Wrong board configured. Select e-radionica Inkplate6 or Soldered Inkplate6 in the boards menu."
#endif
//921600
#include <Arduino.h>
#include <Inkplate.h>
#include <ArduinoJson.h>
#include <esp_attr.h>

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
uint32_t updateWeather(weather::Weather& weatherData, const weatherprovider::WeatherProvider& provider);
void draw(const weather::Weather& weatherData);

void setup()
{
    Serial.begin(115200);
    int result = gDisplay.begin();
    if (!result)
    {
        log_e("board init failure. Cannot continue. restarting device in 10 seconds...");
        delay(10 * 1000);
        esp_restart();
    }
    gDisplay.clearDisplay();
    if (!gDisplay.sdCardInit())
    {
        log_e("SD Card init failure. Icon's will not be available.");
    }
    sdcard::SdCard::sleep(gDisplay);

    env = setEnvironmentFromFile("/env.json", gDisplay);
    gWeather.fakeUpdates(env.fakeApiUpdates);

    gDisplay.setRotation(1);
    log_d("last forecast time: %d"), static_cast<uint64_t>(gWeather.getLastForecastTime());
}

void loop()
{
    constexpr uint32_t cMinSleepTimeSecs = 15;
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
    log_d("Now sleeping for %u seconds (%u minutes)...",
        sleepTimeSecs,
        (sleepTimeSecs / 60)
    );
    // pause to finish writing
    delay(10);
    Serial.end();
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
    esp_sleep_enable_timer_wakeup(sleepTimeSecs * 1000000L);
    sdcard::SdCard::sleep(gDisplay);
    esp_deep_sleep_start();
}

uint32_t updateWeather(weather::Weather& weatherData, const weatherprovider::WeatherProvider& provider)
{
    network::Network connection(env.ssid, env.pass);
    // Delay between API calls. 1 minute when reading from SD, minimum 15 minutes otherwise.
    uint32_t sleepTimeSecs =
        env.fakeApiUpdates ? 60 : std::max(provider.getWeatherUpdateIntervalSeconds(), 900U);
    log_d("Using %s API updates, with a query interval of %u seconds",
        (env.fakeApiUpdates ? (const char *)"mock" : "real"),
        sleepTimeSecs
    );
    bool updated = false;
    if (!connection.isConnected())
    {
        // Gotta get online!
        static constexpr uint32_t connectionRetrySeconds = 15;
        return connectionRetrySeconds;
    }
    static constexpr uint8_t retries = 10;
    static constexpr uint16_t retryDelaySeconds = 5;
    for (uint8_t retry = 0; retry < retries; retry++)
    {
        if (weatherData.updateWeather(connection, provider))
        {
            log_i("Forecast updated");
            weatherData.printDailyWeather(weatherData.getDailyWeather(0));
            updated = true;
            break;
        }
        else
        {
            log_w("Failed to fetch current weather data");
        }
        if (retry != retries - 1)
        {
            log_i("Will retry in %lu seconds", retryDelaySeconds);
            delay(retryDelaySeconds * 1000);
        }
        else
        {
            log_w("Failed to update weather after %u retries", retries);
        }
    }
    return (updated) ? sleepTimeSecs : 0;
}

void draw(const weather::Weather& weatherData)
{
    renderer::Renderer renderer(gDisplay, env.city);
    renderer.update(weatherData);
    renderer.render();
}
