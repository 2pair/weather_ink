#include "Icon.h"

#include <WString.h>

#include <Inkplate.h>
#include <esp32-hal-log.h>

#include "SdCard.h"
#include "DailyWeather.h"
#include "TimeUtils.h"


using namespace icon;

const std::string Icon::cIconsDir = (const char*)F("/icons");

Icon::Icon(Inkplate& display, const std::string&  iconName)
    :   mExists(false),
        mIconName(iconName),
        mDisplay(display)
{
    sdcard::SdCard sdCard(mDisplay);
    auto fileName = sdCard.findFileWithPrefix(
        std::string(cIconsDir) + (const char*)F("/300"),
        mIconName
    );
    if (fileName.empty())
    {
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
        log_w("Found file did not have an extension, assuming PNG");
        mExtension = "png";
    }
}

void Icon::draw(size_t x, size_t y, Size size)
{
    auto iconPath = getPath(size);
    // Enables SD card device for just this scope
    sdcard::SdCard sdCard(mDisplay);
    mDisplay.drawImage(
        iconPath.c_str(),
        x,
        y,
        true,
        false
    );
}

void Icon::drawCentered(size_t x, size_t y, Size size)
{
    size_t x_top_left, y_top_left;
    size_t width, height;
    std::tie(width, height) = getDimensions(size);
    x_top_left = x - (width / 2);
    y_top_left = y - (height / 2);

    draw(x_top_left, y_top_left, size);
}

const std::string Icon::getIconNameForConditions(const weather::DailyWeather& conditions)
{
    using namespace weather;

    // Get Moon icons only if this is todays weather and its nighttime.
    auto nowTime = time(nullptr);
    log_d("now time is %llu", nowTime);
    bool getMoonPhase = (
        (
            timeutils::dayNameFromEpochTimestamp(timeutils::localTime(conditions.timestamp, conditions.timeZone)) ==
            timeutils::dayNameFromEpochTimestamp(timeutils::localTime(nowTime, conditions.timeZone))
        ) &&
        isNightTime(conditions)
    );
    log_v("Getting nighttime icon for conditions with timestamp %llu mapped to day %s, at timestamp %llu mapped to day %s",
        conditions.timestamp, 
        timeutils::dayNameFromEpochTimestamp(timeutils::localTime(conditions.timestamp, conditions.timeZone)).c_str(),
        nowTime,
        timeutils::dayNameFromEpochTimestamp(timeutils::localTime(nowTime, conditions.timeZone)).c_str()
    );
    switch (conditions.condition)
    {
        case Condition::clear:
            if (getMoonPhase) {
                return (const char *)F("clr") + getMoonPhaseAbbreviation(conditions.moonPhase);
            }
            return (const char *)F("clr");
        case Condition::partlyCloudy:
            if (getMoonPhase) {
                return (const char *)F("pcd") + getMoonPhaseAbbreviation(conditions.moonPhase);
            }
            return (const char *)F("pcd");
        case Condition::cloudy:
            return (const char *)F("cd");
        case Condition::foggy:
            return (const char *)F("fog");
        case Condition::drizzle:
            return (const char *)F("dzl");
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
            return (const char *)F("undef");
    }
}

const std::string Icon::getMoonPhaseAbbreviation(const weather::MoonPhase& moonPhase)
{
    using namespace weather;
    switch(moonPhase) {
        case MoonPhase::newMoon:
            return (const char *)F("nm");
        case MoonPhase::waxingCrescent:
            return (const char *)F("xc");
        case MoonPhase::firstQuarter:
            return (const char *)F("fq");
        case MoonPhase::waxingGibbous:
            return (const char *)F("xg");
        case MoonPhase::fullMoon:
            return (const char *)F("fm");
        case MoonPhase::waningGibbous:
            return (const char *)F("wg");
        case MoonPhase::thirdQuarter:
            return (const char *)F("tq");
        case MoonPhase::waningCrescent:
            return (const char *)F("wc");
        case MoonPhase::unknownPhase:
        default:
            return "";
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
        mIconName  + size_str + (const char*)F(".") + mExtension;
}

std::pair<size_t, size_t> Icon::getDimensions(Size size) const
{
    switch (size)
    {
        case Size::s_50x50:
            return std::make_pair(50, 50);
        case Size::s_75x75:
            return std::make_pair(75, 75);
        case Size::s_100x100:
            return std::make_pair(100, 100);
        case Size::s_150x150:
            return std::make_pair(150, 150);
        case Size::s_300x300:
            return std::make_pair(300, 300);
        default:
            return std::make_pair(0, 0);
    }
}

bool Icon::exists() const
{
    return mExists;
}

icon::Icon icon::iconFactory(Inkplate& display, const weather::DailyWeather& conditions)
{
    icon::Icon weatherIcon(display, icon::Icon::getIconNameForConditions(conditions));
    if (!weatherIcon.exists())
    {
        log_w("Icon %s does not exist",conditionToString(conditions.condition).c_str());
    }
    return weatherIcon;
}