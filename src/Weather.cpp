#include "Weather.h"

#include <ArduinoJson.h>

#include "../local_env.h"
#include "Network.h"
#include "DailyWeather.h"
#include "WeatherProvider/OpenWeatherMap.h"
#include "SdCard.h"


using namespace weather;

extern Inkplate display;

Weather::Weather(weatherprovider::OpenWeatherMap& provider)
    :   mProvider(provider)
{}

bool Weather::updateCurrent(JsonDocument& apiResponse, network::Network& connection)
{
    bool success = false;
    if (cFakeAPIUpdates) {
        Serial.println(F("Reading file data to simulate current conditions API response"));
        sdcard::SdCard card(display);
        card.getFakeCurrentData(apiResponse);
        success = (apiResponse.size() > 0);
    }
    else
    {
        char url[256];
        mProvider.getForecastedWeatherUrl(url, 256);
        success = connection.apiGetResponse(apiResponse, url);
    }
    if (success) {
        mProvider.toCurrentWeather(mForecast[0], apiResponse);
    }
    return success;
}

bool Weather::updateForecast(JsonDocument& apiResponse, network::Network& connection)
{
    bool success = false;
    if (cFakeAPIUpdates) {
        Serial.println(F("Reading file data to simulate forecasted conditions API response"));
        sdcard::SdCard card(display);
        card.getFakeForecastData(apiResponse);
        success = (apiResponse.size() > 0);
    }
    else {
        char url[256];
        mProvider.getForecastedWeatherUrl(url, 256);
        success = connection.apiGetResponse(apiResponse, url);
    }
    if (success) {
        mProvider.toForecastedWeather(&mForecast[1], cForecastDays, apiResponse);
    }
    return success;
}


const DailyWeather& Weather::getDailyWeather(const uint8_t index) const
{
    return mForecast[index];
}

void formatTemp(char *str, float temp)
{
    // Built in function for float to char* conversion
    dtostrf(temp, 2, 0, str);
}

void formatWind(char *str, float wind)
{
    // Built in function for float to char* conversion
    dtostrf(wind, 2, 0, str);
}

const char* weather::conditionToString(const Condition condition)
{
    switch(condition) {
        case clear:
            return (const char *)F("Clear");
        case partlyCloudy:
            return (const char *)F("Partly Cloudy");
        case cloudy:
            return (const char *)F("Cloudy");
        case foggy:
            return (const char *)F("Foggy");
        case drizzle:
            return (const char *)F("Drizzle");
        case lightRain:
            return (const char *)F("Light Rain");
        case rain:
            return (const char *)F("Rain");
        case thunderstorm:
            return (const char *)F("Thunderstorm");
        case freezingRain:
            return (const char *)F("Freezing Rain");
        case sleet:
            return (const char *)F("Sleet");
        case snow:
            return (const char *)F("Snow");
        case unknown:
        default:
            return (const char *)F("Unknown Conditions");
    }
}
