#include "WeatherProvider.h"

#include "../DailyWeather.h"
#include "../SdCard.h"
#include "../Network.h"


using namespace weatherprovider;

time_t WeatherProvider::getCurrentWeather(
    weather::DailyWeather& currentWeather,
    network::Network& connection) const
{
    JsonDocument apiResponse;
    auto url = getCurrentWeatherUrl();
    log_d("URL returned: %s", url.c_str());
    bool success = connection.getApiResponse(apiResponse, url);

    if (success)
    {
        log_d("Converting JSON to current weather data");
        toCurrentWeather(currentWeather, apiResponse);
    }
    else
    {
        log_w("Failed to get API response for current weather");
    }
    apiResponse.clear();
    return (success) ? time(nullptr) : 0;
}

time_t WeatherProvider::getCurrentWeather(
    weather::DailyWeather& currentWeather,
    sdcard::SdCard& sdCard) const
{
    JsonDocument apiResponse;
    log_i("Reading file data to simulate current conditions API response");
    bool success = sdCard.getFakeWeatherData(
        apiResponse,
        getFileSystemDirectory() + "current.json"
    );

    if (success)
    {
        log_d("Converting JSON to current weather data");
        toCurrentWeather(currentWeather, apiResponse);
    }
    else
    {
        log_w("Failed to get file data for current weather");
    }
    apiResponse.clear();
    return (success) ? time(nullptr) : 0;
}

time_t WeatherProvider::getForecastedWeather(
    weather::daily_forecast& forecastedWeather,
    network::Network& connection) const
{
    JsonDocument apiResponse;
    auto url = getForecastedWeatherUrl();
    log_d("URL returned: %s", url.c_str());
    bool success = connection.getApiResponse(apiResponse, url);

    if (success)
    {
        log_d("Converting JSON to forecast weather data");
        toForecastedWeather(forecastedWeather, apiResponse);
    }
    else
    {
        log_w("Failed to get API response for forecasted weather");
    }
    apiResponse.clear();
    return (success) ? time(nullptr) : 0;
}

time_t WeatherProvider::getForecastedWeather(
    weather::daily_forecast& forecastedWeather,
    sdcard::SdCard& sdCard) const
{
    JsonDocument apiResponse;
    log_i("Reading file data to simulate forecasted conditions API response");
    bool success = sdCard.getFakeWeatherData(
        apiResponse,
        getFileSystemDirectory() + "forecast.json"
    );

    if (success)
    {
        log_d("Converting JSON to forecast weather data");
        toForecastedWeather(forecastedWeather, apiResponse);
    }
    else
    {
        log_w("Failed to get file data for forecasted weather");
    }
    apiResponse.clear();
    return (success) ? time(nullptr) : 0;
}

time_t WeatherProvider::getHourlyWeather(
    weather::hourly_forecast& forecastedWeather,
    network::Network& connection) const
{
    JsonDocument apiResponse;
    auto url = getHourlyWeatherUrl();
    log_d("URL returned: %s", url.c_str());
    bool success = connection.getApiResponse(apiResponse, url);

    if (success)
    {
        log_d("Converting JSON to hourly weather data");
        toHourlyWeather(forecastedWeather, apiResponse);
    }
    else
    {
        log_w("Failed to get API response for hourly weather");
    }
    apiResponse.clear();
    return (success) ? time(nullptr) : 0;
}

time_t WeatherProvider::getHourlyWeather(
    weather::hourly_forecast& forecastedWeather,
    sdcard::SdCard& sdCard) const
{
    JsonDocument apiResponse;
    log_i("Reading file data to simulate hourly conditions API response");
    bool success = sdCard.getFakeWeatherData(
        apiResponse,
        getFileSystemDirectory() + "forecast.json"
    );

    if (success)
    {
        log_d("Converting JSON to hourly weather data");
        toHourlyWeather(forecastedWeather, apiResponse);
    }
    else
    {
        log_w("Failed to get file data for hourly weather");
    }
    apiResponse.clear();
    return (success) ? time(nullptr) : 0;
}
