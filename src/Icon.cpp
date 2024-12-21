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

    // get icon sizes and validate icon exists
    File srcDir, pixDir;
    std::string fileName;
    srcDir.open(cIconsDir.c_str());
    srcDir.rewindDirectory();
    while(true) {
        pixDir = srcDir.openNextFile(O_RDONLY);
        if (!pixDir)
        {
            // No more items in directory
            break;
        }
        if (!pixDir.isDir())
        {
            continue;
        }
        std::string newFileName = sdCard.findFileWithPrefix(pixDir, mIconName);
        if (newFileName.empty())
        {
            continue;
        }
        fileName = newFileName;
        std::array<char, 16> currentDirName;
        pixDir.getName(currentDirName.data(), currentDirName.size());
        mIconSizes.emplace_back(std::stoi(currentDirName.data()));
        mExists = true;
        pixDir.close();
    }
    srcDir.close();
    if (mIconSizes.empty())
    {
        log_d("No icon directory contained an icon with name '%s'", iconName);
    }
    sort(mIconSizes.begin(), mIconSizes.end());
    auto separatorPosition = fileName.find_last_of('.');
    if (separatorPosition != std::string::npos)
    {
        mExtension = fileName.substr(separatorPosition + 1);
    }
    else
    {
        log_w("Found file '%s' did not have an extension, assuming PNG", fileName.c_str());
        mExtension = "png";
    }
}

const size_t Icon::getNearestFilePixelSize(size_t size) const
{
    // default to largest icon
    size_t filePixelSize = mIconSizes.back();
    for (auto pixelSize : mIconSizes)
    {
        if (pixelSize >= size)
        {
            filePixelSize = pixelSize;
            break;
        }
    }
    return filePixelSize;
}

void Icon::draw(size_t x, size_t y, size_t size)
{
    auto filePixelSize = Icon::getNearestFilePixelSize(size);
    auto iconPath = getPath(filePixelSize);
    // Enables SD card device for just this scope
    sdcard::SdCard sdCard(mDisplay);
    if (filePixelSize != size)
    {
        log_w(
            "Icon %s not available with size %d, using %d and re-centering position",
            mIconName.c_str(), size, filePixelSize
        );
        const size_t iconOffset = std::max(static_cast<int>(filePixelSize - size), 0) / 2;
        x -= iconOffset;
        y -= iconOffset;

    }
    if (!sdCard.fileExists(iconPath))
    {
        log_e("Could not draw icon at path %s because it does not exist!", iconPath.c_str());
        return;
    }
    log_i("drawing icon %s", iconPath.c_str());
    mDisplay.drawImage(iconPath.c_str(), x, y, true, false);
}

void Icon::drawCentered(size_t x, size_t y, size_t size)
{
    size_t width = size, height = size;
    size_t x_top_left = x - (width / 2);
    size_t y_top_left = y - (height / 2);

    draw(x_top_left, y_top_left, size);
}

template <>
bool Icon::useNighttimeIcon<weather::DailyWeather>(const weather::DailyWeather& dayData)
{
    // Only get nighttime icons if this is todays weather and it's nighttime.
    const auto nowDayIndex = timeutils::dayIndexFromEpochTimestamp(
        timeutils::localTime(dayData.timeZone)
    );
    const auto dayDataDayIndex = timeutils::dayIndexFromEpochTimestamp(
        timeutils::localTime(dayData.timestamp, dayData.timeZone)
    );

    return (
        nowDayIndex == dayDataDayIndex &&
        weather::isNighttime(dayData)
    );
}

template <>
bool Icon::useNighttimeIcon<weather::HourlyWeather>(const weather::HourlyWeather& hourlyData)
{
    return !hourlyData.daytime;
}

const std::string Icon::getMoonPhaseAbbreviation(const weather::MoonPhase& moonPhase)
{
    using namespace weather;
    switch (moonPhase) {
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

const std::string Icon::getPath(size_t size) const
{
    auto size_str = std::to_string(size);
    return cIconsDir + "/" + size_str + "/" + mIconName  + size_str + "." + mExtension;
}

bool Icon::exists() const
{
    return mExists;
}
