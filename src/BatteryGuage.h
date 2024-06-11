#pragma once

#include "Icon.h"

class Inkplate;

namespace icon {

class BatteryGauge
{
    public:
        BatteryGauge(Inkplate& display);

        // Draw the icon with its top left corner at point (x,y) with given size
        void draw(size_t x, size_t y, Size size);

        // Draw the icon centered at point (x,y) with given size
        void drawCentered(size_t x, size_t y, Size size);

    private:
        enum class Power
        {
            full,
            twoThirds,
            oneThird,
            low,
            empty
        };

        Power getCharge();

        Icon getIcon();

        Inkplate& mDisplay;

        static constexpr float cVoltageBattFull = 4.11;
        static constexpr float cVoltageBattTwoThirds = 3.91;
        static constexpr float cVoltageBattOneThird = 3.79;
        static constexpr float cVoltageBattLow = 3.71;
};

}