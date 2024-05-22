# pragma once

#include <Inkplate.h>


namespace icon {

enum Size {
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
        Icon(Inkplate& display, const char* const iconName);

        // Draw the icon centered at point (x,y) at given size
        void draw(size_t x, size_t y, Size size);

    private:
        std::string getPath(Size size) const;

        std::pair<size_t, size_t> getDimensions(Size size) const;

        std::string mIconName;
        Inkplate mDisplay;
        std::string mExtension;

        static constexpr char cIconsDir[] = "/icons";
};

}