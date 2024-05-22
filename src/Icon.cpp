#include "Icon.h"

#include "SdCard.h"


using namespace icon;

Icon::Icon(Inkplate &display, const char *const iconName)
    :   mIconName(iconName),
        mDisplay(display)
{
    sdcard::SdCard sdCard(mDisplay);
    auto fileName = sdCard.findFileWithPrefix(
        std::string(cIconsDir) + "/300",
        iconName
    );
    if (fileName.empty())
    {
        Serial.printf("Icon %s does not exist\n");
        return;
    }
    auto separatorPosition = fileName.find_last_of('.');
    if (separatorPosition != std::string::npos)
    {
        mExtension = fileName.substr(separatorPosition);
    }
}

void Icon::draw(size_t x, size_t y, Size size)
{
    size_t x_top_left, y_top_left;
    size_t width, height;
    std::tie(width, height) = getDimensions(size);
    x_top_left = x - (width / 2);
    y_top_left = y - (height / 2);

    mDisplay.drawImage(
        getPath(size).c_str(),
        x_top_left,
        y_top_left,
        true,
        false
    );
}

std::string Icon::getPath(Size size) const
{
    // TODO
    size_t width, height;
    std::tie(width, height) = getDimensions(size);
    return
        cIconsDir + '/' + std::to_string(width) + '/' +
        mIconName + '_' + std::to_string(width) + '_' + std::to_string(height) + '.' + mExtension;
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