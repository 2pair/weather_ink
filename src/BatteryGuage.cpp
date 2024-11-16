#include "BatteryGuage.h"

#include <Inkplate.h>
#include <esp32-hal-log.h>

#include "Icon.h"

using namespace icon;

BatteryGauge::BatteryGauge(Inkplate& display)
    :   mDisplay(display)
{}

void BatteryGauge::draw(size_t x, size_t y, size_t size)
{
    auto icon = getIcon();
    icon.draw(x, y, size);
}

void BatteryGauge::drawCentered(size_t x, size_t y, size_t size)
{
    auto icon = getIcon();
    icon.drawCentered(x, y, size);
}

BatteryGauge::Power icon::BatteryGauge::getCharge()
{
    auto batteryVoltage = mDisplay.readBattery();
    log_i("Battery voltage is %.3f V", batteryVoltage);
    if (batteryVoltage >= cVoltageBattFull)
    {
        return BatteryGauge::Power::full;
    }
    else if (batteryVoltage >= cVoltageBattTwoThirds)
    {
        return BatteryGauge::Power::twoThirds;
    }
    else if (batteryVoltage >= cVoltageBattTwoThirds)
    {
        return BatteryGauge::Power::oneThird;
    }
    else if (batteryVoltage >= cVoltageBattLow)
    {
        return BatteryGauge::Power::low;
    }
    else
    {
        return BatteryGauge::Power::empty;
    }
}

Icon icon::BatteryGauge::getIcon()
{
    switch(getCharge())
    {
        case BatteryGauge::Power::full:
            return Icon(mDisplay, "b++++");
        case BatteryGauge::Power:: twoThirds:
            return Icon(mDisplay, "b+++-");
        case BatteryGauge::Power::oneThird:
            return Icon(mDisplay, "b++--");
        case BatteryGauge::Power::low:
            return Icon(mDisplay, "b+---");
        case BatteryGauge::Power::empty:
        default:
            return Icon(mDisplay, "b----");
    }
}
