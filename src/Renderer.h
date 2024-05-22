#pragma once

#include <Inkplate.h>

#include "DailyWeather.h"
#include "Weather.h"


namespace renderer {

class Renderer {
    public:
        Renderer(Inkplate& display, const char* city);
        void update(const weather::Weather& weatherData);
        void render();

    private:
        enum LevelOfDetail {
            small,
            medium,
            large
        };

        void drawConditions(const weather::DailyWeather& currentConditions);
        // // Function for drawing weather info
        // void getIcon(LevelOfDetail lod);

        // // Function for drawing current time
        // void drawTime();

        // Function for drawing city name
        void drawLocation();

        // // Function for drawing temperatures
        void drawTemps(const weather::DailyWeather& currentConditions);

        // // Current weather drawing function
        // void drawForecast();

        Inkplate& mDisplay;
        const char* cCity;
};

}
