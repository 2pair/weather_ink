#pragma once

#include <vector>

#include <Arduino.h>

#include "DailyWeather.h"
#include "TimeUtils.h"


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
        template <class T_weatherData>
        static const std::string getIconNameForConditions(const T_weatherData& weatherData);

        // Gets the abbreviation for the moon phase. Due to the above constraint this
        // will be 2 characters.
        static const std::string getMoonPhaseAbbreviation(const weather::MoonPhase& moonPhase);

        // Does an icon with the given name exist?
        bool exists() const;

        // Gets the closest icon size larger than the given size.
        size_t getNearestFilePixelSize(size_t size) const;

    private:
        const std::string getPath(size_t size) const;
        template <class T_weatherData>
        static bool useNighttimeIcon(const T_weatherData& weatherData);

        bool mExists;
        // These sizes must be sorted in ascending order
        std::string mIconName;
        Inkplate& mDisplay;
        std::string mExtension;
        std::vector<size_t> mIconSizes;

        static const std::string cIconsDir;
};


template <class T_weatherData>
const std::string Icon::getIconNameForConditions(const T_weatherData& weatherData)
{
    using namespace weather;

    const bool getMoonPhase = useNighttimeIcon(weatherData);
    if (getMoonPhase)
    {
        log_d("Getting nighttime icon for weather data with timestamp %d", weatherData.timestamp);
    }
    switch (weatherData.condition)
    {
        case Condition::clear:
            if (getMoonPhase) {
                return (const char *)F("clr") + getMoonPhaseAbbreviation(weatherData.moonPhase);
            }
            return (const char *)F("clr");
        case Condition::partlyCloudy:
            if (getMoonPhase) {
                return (const char *)F("pcd") + getMoonPhaseAbbreviation(weatherData.moonPhase);
            }
            return (const char *)F("pcd");
        case Condition::drizzle:
            if (getMoonPhase) {
                return (const char *)F("dzl") + getMoonPhaseAbbreviation(weatherData.moonPhase);
            }
            return (const char *)F("dzl");
        case Condition::cloudy:
            return (const char *)F("cd");
        case Condition::foggy:
            return (const char *)F("fog");
        case Condition::lightRain:
            return (const char *)F("lrn");
        case Condition::rain:
            return (const char *)F("rn");
        case Condition::heavyRain:
            return (const char *)F("hrn");
        case Condition::lightning:
            return (const char *)F("lng");
        case Condition::thunderstorm:
            return (const char *)F("tst");
        case Condition::freezingRain:
            return (const char *)F("frn");
        case Condition::sleet:
            return (const char *)F("slt");
        case Condition::snow:
            return (const char *)F("sno");
        case Condition::wintryMix:
            return (const char *)F("wmx");
        case Condition::windy:
            return (const char *)F("wnd");
        case Condition::unknownCondition:
        default:
            log_d("Icon not found");
            return (const char *)F("undef");
    }
}

template <class T_weatherData>
Icon iconFactory(Inkplate& display, const T_weatherData& weatherData)
{
    icon::Icon weatherIcon(display, icon::Icon::getIconNameForConditions(weatherData));
    if (!weatherIcon.exists())
    {
        log_w("Icon %s does not exist",conditionToString(weatherData.condition).c_str());
    }
    return weatherIcon;
}

}