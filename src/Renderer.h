#pragma once

#include <string>
#include <vector>

// this should work but my include paths are not working
//#include <gfxfont.h>
#include <Inkplate.h>
#include "WeatherTypes.h"


namespace weather {
    class DailyWeather;
    class Weather;
}
class Inkplate;

/*  Screen layout:
    +---------------+---------------+
    |               |  forecast 1   |
    |               |               |
    |               +---------------+
    |    current    |  forecast 2   |
    |               |               |
    |               +---------------+
    |               |  forecast 3   |
    |               |               |
    +---------------+---------------+
    |             hourly            |
    +--------+---------------+------+
    | updated|     city      | batt |
    +--------+---------------+------+
*/

namespace renderer {

class Renderer {
    private:
        // local definition to allow forward declaration
        //struct GFXfont;

    public:
        Renderer(Inkplate& display);

        // Updates the screen buffer
        void update(const weather::Weather& weatherData, const char* city);

        // Draw the screen buffer's contents to the screen
        void render();

        // Update the screen buffer with the given text lines centered on the screen.
        // This will first clear the screen buffer.
        // lineSpacing: Number of pixels between each line
        void drawLinesCentered(
            const std::vector<std::string>& lines,
            const GFXfont& font,
            const size_t lineSpacing
        );

    private:
        // Draws the current weather conditions in  300 wide by 600 high area
        // (x, y) is the coordinate of the top left corner of the box
        void drawCurrentConditions(
            const weather::DailyWeather& currentConditions,
            const size_t x,
            const size_t y
        );

        // Draws the hourly forecasted weather conditions in a 600 wide by 120 high area.
        // (x,y) is the coordinate of the top left corner of the box
        void drawHourlyForecast(
            const weather::hourly_forecast& forecast,
            const size_t x,
            const size_t y
        );

        // Draws the forecasted weather conditions in a 300 wide by 200 high area. (per day)
        // (x,y) is the coordinate of the top left corner of the box
        void drawForecastForDay(
            const weather::DailyWeather& forecast,
            const size_t x,
            const size_t y
        );

        // Draws the battery gauge in a N wide by M high box.
        // (x, y) is the coordinate of the top left corner of the box
        void drawBatteryGauge(size_t x, size_t y);

        // Draws the city name _centered_ at the given point
        // Due to the length being variable, this allows consistent positioning
        void drawCityName(const char* city, size_t x, size_t y);

        // Draws the last time the display was updated, with the top left corner at
        // the given point, and with the time in the given timezone.
        void drawLastUpdated(size_t x, size_t y, int8_t timeZone);

        // Gets the width and height of the given string.
        // The dimensions will depend on the currently set font.
        std::pair<uint16_t, uint16_t> getTextDimensions(const std::string& text) const;

        Inkplate& mDisplay;
        const char* cCity;

        static constexpr size_t cCurrentWidth = 300;
        static constexpr size_t cCurrentHeight = 600;
        static constexpr size_t cForecastWidth = 300;
        static constexpr size_t cForecastHeight = 200;
        static constexpr size_t cHourlyWidth = 600;
        static constexpr size_t cHourlyHeight = 120;
};

}
