#pragma once

#include <vector>

#include <Arduino.h>

#include "DailyWeather.h"


class Inkplate;

namespace icon {
/*
Icons are saved to the SD card in a variety of sizes. This class allows
accessing an icon without having to worry about how the various sizes are
stored in the filesystem.

The files should be stored based on size in /icons/<size_pixels>/<icon_name><size_pixels>.<ext>
*/

const std::string cSunsetIconName = (const char*)F("suns");
const std::string cSunriseIconName = (const char*)F("sunr");

class Icon
{
    public:
        Icon(Inkplate& display, const std::string& iconName);

        // Draw the icon with its top left corner at point (x,y) with given Size
        //void draw(size_t x, size_t y, Size size);
        // Draw the icon with its top left corner at point (x,y) with given size, in pixels
        void draw(size_t x, size_t y, size_t size);

        // Draw the icon centered at point (x,y) with given size
        void drawCentered(size_t x, size_t y, size_t size);

        // Get the icon name for the conditions, this must be <= 5 chars due to
        // limitations in the library code
        static const std::string getIconNameForConditions(const weather::DailyWeather& dayData);

        // Gets the abbreviation for the moon phase. Due to the above constraint this
        // will be 2 characters.
        static const std::string getMoonPhaseAbbreviation(const weather::MoonPhase& moonPhase);

        // Does an icon with the given name exist?
        bool exists() const;

        // Gets the closest icon size larger than the given size.
        const size_t getNearestFilePixelSize(size_t size) const;

    private:
        const std::string getPath(size_t size) const;
        static bool useNighttimeIcon(const weather::DailyWeather& dayData);

        bool mExists;
        std::string mIconName;
        Inkplate& mDisplay;
        std::string mExtension;
        std::vector<size_t> mIconSizes;

        static const std::string cIconsDir;
        //static constexpr std::array<size_t, 4> pixelSizes = {25, 50, 150, 300};
};

Icon iconFactory(Inkplate& display, const weather::DailyWeather& dayData);


}