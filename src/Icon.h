# pragma once

#include <Inkplate.h>

#include "DailyWeather.h"


namespace icon {

enum class Size {
    s_50x50,
    s_75x75,
    s_100x100,
    s_150x150,
    s_300x300
};

/*
Icons are saved to the SD card in a variety of sizes. This class allows
accessing an icon without having to worry about how the various sizes are
stored in the filesystem.

The files should be stored based on size in /icons/<size_pixels>/<icon_name>_<size_pixels>
*/
class Icon
{
    public:
        Icon(Inkplate& display, const std::string& iconName);

        // Draw the icon with its top left corner at point (x,y) with given size
        virtual void draw(size_t x, size_t y, Size size);

        // Draw the icon centered at point (x,y) with given size
        virtual void drawCentered(size_t x, size_t y, Size size);

        // Get the icon name for the conditions, this must be <= 5 chars due to
        // limitations in the library code
        static const std::string getIconNameForConditions(const weather::DailyWeather& conditions);

        // Gets the abbreviation for the moon phase. Due to the above constraint this
        // will be 2 characters.
        static const std::string getMoonPhaseAbbreviation(const weather::MoonPhase& moonPhase);

        bool exists() const;

    private:
        const std::string getPath(Size size) const;

        std::pair<size_t, size_t> getDimensions(Size size) const;

        // TODO: This could instead be a list of valid sizes.
        bool mExists;
        std::string mIconName;
        Inkplate& mDisplay;
        std::string mExtension;

        static const std::string cIconsDir;
};

Icon iconFactory(Inkplate& display, const weather::DailyWeather& conditions);

}