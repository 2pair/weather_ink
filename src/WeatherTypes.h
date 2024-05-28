#pragma once

#include "DailyWeather.h"

namespace weather
{

// Including the current day's forecast
static constexpr uint8_t cForecastDays = 4;
static constexpr uint8_t cForecastHours = 8;

typedef std::array<DailyWeather, cForecastDays> daily_forecast;
typedef std::array<HourlyWeather, cForecastHours> hourly_forecast;

}
