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
    :   mProvider(provider),
        mForecast(cForecastDays)
{}

bool Weather::updateCurrent(network::Network& connection)
{
    JsonDocument apiResponse;
    sdcard::SdCard card(display);
    bool success = false;
    if (cFakeAPIUpdates) {
        Serial.println(F("Reading file data to simulate current conditions API response"));
        success = card.getFakeCurrentData(apiResponse);
    }
    else
    {
        auto url = mProvider.getForecastedWeatherUrl();
        success = connection.apiGetResponse(apiResponse, url);
    }

    if (success) {
        Serial.println("Converting JSON to current weather data");
        mProvider.toCurrentWeather(mForecast[0], apiResponse);
    }
    apiResponse.clear();
    return success;
}

bool Weather::updateForecast(network::Network& connection)
{
    JsonDocument apiResponse;
    sdcard::SdCard card(display);
    bool success = false;
    if (cFakeAPIUpdates) {
        Serial.println(F("Reading file data to simulate forecasted conditions API response"));
        success = card.getFakeForecastData(apiResponse);
    }
    else {
        auto url = mProvider.getForecastedWeatherUrl();
        success = connection.apiGetResponse(apiResponse, url);
    }

    if (success) {
        Serial.println("Converting JSON to forecast weather data");
        mProvider.toForecastedWeather(mForecast, apiResponse);
    }
    apiResponse.clear();
    return success;
}

const DailyWeather& Weather::getDailyWeather(const uint8_t index) const
{
    return mForecast[index];
}

void Weather::printDailyWeather(const DailyWeather& dailyWeather)
{
    Serial.println("--------- Daily Weather ---------");
    Serial.printf("Data collected at: %d", dailyWeather.timestamp);

    Serial.printf(
        "Current Temp:  %.2f (feels like %.2f)\n",
        dailyWeather.tempNow,
        dailyWeather.feelsLike
    );
    Serial.printf(
        "Daily High:  %.2f  Daily Low:  %.2f\n",
        dailyWeather.tempLow,
        dailyWeather.tempHigh
    );

    Serial.printf(
        "Condition: %s,  Precipitation:  %s (%.2f in.)\n",
        weather::conditionToString(dailyWeather.condition),
        weather::precipitationToString(dailyWeather.precipitationType),
        dailyWeather.precipitation
    );

    Serial.printf(
        "H: %.2f  P:  %.2f  V:  %.2f\n",
        dailyWeather.humidity,
        dailyWeather.pressure,
        dailyWeather.visibility
    );

    Serial.printf(
        "Wind: %.2f  Gust:  %.2f  Deg:  %.2f\n",
        dailyWeather.windSpeed,
        dailyWeather.gustSpeed,
        dailyWeather.windDirection
    );

    Serial.printf(
        "Sunrise: %d  Sunset:  %d\n",
        dailyWeather.sunrise,
        dailyWeather.sunset
    );
    Serial.println("---------------------------------");
}