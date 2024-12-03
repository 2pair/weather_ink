#if !defined(ARDUINO_ESP32_DEV) && !defined(ARDUINO_INKPLATE6V2)
#error                                                                                                                 \
    "Wrong board configured. Select e-radionica Inkplate6 or Soldered Inkplate6 in the boards menu."
#endif

#include <Arduino.h>
#include <Inkplate.h>
#include <ArduinoJson.h>
#include <esp_attr.h>
#include <esp_task_wdt.h>

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
#include "src/UserConfig.h"


constexpr uint32_t cMinSleepTimeSecs = 15;

Inkplate gDisplay(INKPLATE_3BIT);
RTC_DATA_ATTR Environment env;
RTC_DATA_ATTR weather::Weather gWeather(gDisplay);

// Returns how long to wait until the next update, in seconds. Returns 0 if the weather could not be updated.
std::pair<bool, uint32_t> updateWeather(weather::Weather& weatherData, const weatherprovider::WeatherProvider& provider);
// Draw the data to the display.
void draw(const weather::Weather& weatherData);

void setup()
{
    Serial.begin(115200);//921600
    // Very long initial timeout to safeguard against program hanging at any point
    esp_task_wdt_init(10 * 60, true);
    enableLoopWDT();

    int result = gDisplay.begin();
    if (!result)
    {
        log_e("board init failure. Cannot continue. restarting device in 10 seconds...");
        delay(10 * 1000);
        esp_restart();
    }
    gDisplay.clearDisplay();
    gDisplay.setRotation(1);
    if (!gDisplay.sdCardInit())
    {
        log_e("SD Card init failure. Icon's will not be available.");
    }
    sdcard::SdCard::sleep(gDisplay);

    userconfig::UserConfig userConfig(gDisplay, GPIO_NUM_36, env);
    userConfig.getConfigFromUser();
    size_t locationIndex = 0;
    if (userConfig.configUpdated())
    {
        locationIndex = userConfig.getLocationIndex();
        env.metricUnits = userConfig.getUseMetric();
    }

    // todo: interrupt on gpio
    env = setEnvironmentFromFile("/env.json", gDisplay, locationIndex);
    gWeather.useFakeUpdates(env.fakeApiUpdates);
    log_d("last forecast time: %d", gWeather.getLastForecastTime());

    // The weather program should always be able to finish executing in 1 minute, restart if hung
    esp_task_wdt_init(60, true);
}

void loop()
{
    uint32_t sleepTimeSecs = 0;
    bool weather_updated = false;
    if (std::string(env.provider) == "WeatherApi")
    {
        weatherprovider::WeatherApi provider(env.latitude, env.longitude, env.city, env.apiKey, env.metricUnits);
        std::tie(weather_updated, sleepTimeSecs) = updateWeather(gWeather, provider);
    }
    else // Only other provider is OpenWeatherMap
    {
        weatherprovider::OpenWeatherMap provider(env.latitude, env.longitude, env.city, env.apiKey, env.metricUnits);
        std::tie(weather_updated, sleepTimeSecs) = updateWeather(gWeather, provider);
    }

    if (weather_updated)
    {
        draw(gWeather);
    }
    log_d("Now sleeping for %u seconds (%u minutes)...", sleepTimeSecs, (sleepTimeSecs / 60));
    // pause to finish writing
    delay(10);
    Serial.end();
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
    esp_sleep_enable_timer_wakeup(sleepTimeSecs * cMicrosecondPerSecond);
    sdcard::SdCard::sleep(gDisplay);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, LOW);
    esp_deep_sleep_start();
}

std::pair<bool, uint32_t> updateWeather(weather::Weather& weatherData, const weatherprovider::WeatherProvider& provider)
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
        log_d("Connection has not been established, skipping update");
        // Gotta get online!
        return std::make_pair(updated, cMinSleepTimeSecs);
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
        if (retry < retries - 1)
        {
            log_i("Will retry in %lu seconds", retryDelaySeconds);
            delay(retryDelaySeconds * 1000);
        }
        else
        {
            log_w("Failed to update weather after %u retries", retries);
        }
    }
    return std::make_pair(updated, sleepTimeSecs);
}

void draw(const weather::Weather& weatherData)
{
    renderer::Renderer renderer(gDisplay);
    renderer.update(weatherData, env.city);
    renderer.render();
}
