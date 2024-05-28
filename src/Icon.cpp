#include "Icon.h"

#include <WString.h>

#include "SdCard.h"
#include "DailyWeather.h"


using namespace icon;

const std::string Icon::cIconsDir = (const char*)F("/icons");

Icon::Icon(Inkplate& display, const std::string&  iconName)
    :   mIconName(iconName),
        mDisplay(display),
        mExists(false)
{
    sdcard::SdCard sdCard(mDisplay);
    auto fileName = sdCard.findFileWithPrefix(
        std::string(cIconsDir) + (const char*)F("/300"),
        mIconName
    );
    if (fileName.empty())
    {
        Serial.printf((const char*)F("Icon %s does not exist\n"), mIconName.c_str());
        return;
    }
    mExists = true;
    auto separatorPosition = fileName.find_last_of('.');
    if (separatorPosition != std::string::npos)
    {
        mExtension = fileName.substr(separatorPosition + 1);
    }
    else
    {
        Serial.println(F("Found file did not have an extension, assuming PNG"));
        mExtension = "png";
    }
}

void Icon::draw(size_t x, size_t y, Size size)
{
    size_t x_top_left, y_top_left;
    size_t width, height;
    std::tie(width, height) = getDimensions(size);
    x_top_left = x - (width / 2);
    y_top_left = y - (height / 2);

    auto iconPath = getPath(size);
    Serial.printf("Drawing icon from path %s\n", iconPath.c_str());
    mDisplay.drawImage(
        iconPath.c_str(),
        x_top_left,
        y_top_left,
        true,
        false
    );
    Serial.println("draw success");
}

const std::string Icon::getIconNameForConditions(const weather::DailyWeather& conditions)
{
    using namespace weather;
    switch (conditions.condition)
    {
        case Condition::clear:
            return (const char *)F("sunny");
        case Condition::partlyCloudy:
            return (const char *)F("party_cloudy");
        case Condition::cloudy:
            return (const char *)F("cloudy");
        case Condition::foggy:
            return (const char *)F("foggy");
        case Condition::drizzle:
            return (const char *)F("light_rain");
        case Condition::lightRain:
            return (const char *)F("light_rain");
        case Condition::rain:
            return (const char *)F("rain");
        case Condition::heavyRain:
            return (const char *)F("heavy_rain");
        case Condition::lightning:
            return (const char *)F("lightning");
        case Condition::thunderstorm:
            return (const char *)F("lightning_rain");
        case Condition::freezingRain:
            return (const char *)F("freezing_rain");
        case Condition::sleet:
            return (const char *)F("sleet");
        case Condition::snow:
            return (const char *)F("snow");
        case Condition::wintryMix:
            return (const char *)F("wintry_mix");
        case Condition::windy:
            return (const char *)F("windy");
        case Condition::unknownCondition:
        default:
            return (const char *)F("unknown");
    }
}

const std::string Icon::getPath(Size size) const
{
    // TODO
    size_t width, height;
    std::tie(width, height) = getDimensions(size);
    auto size_str = std::to_string(width);
    return
        cIconsDir + (const char*)F("/") + size_str + (const char*)F("/") +
        mIconName + (const char*)F("_") + size_str + (const char*)F(".") + mExtension;
}

std::pair<size_t, size_t> Icon::getDimensions(Size size) const
{
    switch (size)
    {
        case s_50x50:
            return std::make_pair(50, 50);
        case s_75x75:
            return std::make_pair(75, 75);
        case s_100x100:
            return std::make_pair(100, 100);
        case s_150x150:
            return std::make_pair(150, 150);
        case s_300x300:
            return std::make_pair(300, 300);
        default:
            return std::make_pair(0, 0);
    }
}
