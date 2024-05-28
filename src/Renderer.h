#pragma once

#include <Inkplate.h>

#include "WeatherTypes.h"
#include "DailyWeather.h"
#include "Weather.h"


namespace renderer {

class Renderer {
    public:
        Renderer(Inkplate& display, const char* city);

        // Updates the screen buffer
        void update(const weather::Weather& weatherData);

        // Draw the screen buffer's contents to the screen
        void render();

    private:
        // Draws the current weather conditions in  300 wide by 600 high area
        // (x, y) is the coordinate of the top left corner of the box
        void drawCurrentConditions(
            const weather::DailyWeather& currentConditions,
            size_t x,
            size_t y
        );

        // Draws the hourly forecasted weather conditions in a 600 wide by 120 high area.
        // (x,y) is the coordinate of the top left corner of the box
        void drawHourlyForecast(
            const weather::hourly_forecast& forecast,
            size_t x,
            size_t y
        );

        // Draws the forecasted weather conditions in a 300 wide by 200 high area.
        // (x,y) is the coordinate of the top left corner of the box
        void drawForecastForDay(
            const weather::DailyWeather& forecast,
            size_t x,
            size_t y
        );

        // Draws the battery gauge in a N wide by M high box.
        // (x, y) is the coordinate of the top left corner of the box
        void drawBatteryGauge(size_t x, size_t y);

        Inkplate& mDisplay;
        const char* cCity;
};

}
