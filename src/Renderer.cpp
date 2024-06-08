#include "Renderer.h"

#include <cmath>
#include <string>
#include <WString.h>

#include <Inkplate.h>
#include <ArduinoJson.h>

#include "../Fonts/PatrickHand_Regular16pt7b.h"
#include "../Fonts/PatrickHand_Regular21pt7b.h"
#include "../Fonts/PatrickHand_Regular26pt7b.h"
#include "../Fonts/PatrickHand_Regular31pt7b.h"
#include "../Fonts/PatrickHand_Regular41pt7b.h"
#include "../Fonts/PatrickHand_Regular67pt7b.h"

#include "Icon.h"
#include "BatteryGuage.h"
#include "DailyWeather.h"
#include "Weather.h"
#include "TimeUtils.h"

using namespace renderer;

Renderer::Renderer(Inkplate& display, const char* city)
    :   mDisplay(display),
        cCity(city)
{}

void Renderer::update(const weather::Weather& weatherData)
{
    Serial.println(F("Updating screen buffer"));
    mDisplay.clearDisplay();
    mDisplay.setTextWrap(false);

    static constexpr size_t currentDrawStartX = 0;
    static constexpr size_t currentDrawStartY = 0;
    drawCurrentConditions(
        weatherData.getDailyWeather(0),
        currentDrawStartX,
        currentDrawStartY
    );

    static constexpr size_t maxDaysToForecast = 3;
    auto daysForecasted = weatherData.getDailyForecastLength() - 1;
    auto daysToForecast = std::min(maxDaysToForecast, daysForecasted);
    Serial.printf("drawing %u forecasted days\n", daysToForecast);
    for (size_t i = 1; i < 1 + daysToForecast; i++)
    {
        auto& forecast = weatherData.getDailyWeather(i);
        if (forecast.condition == weather::Condition::unknownCondition)
        {
            // This indicates we don't have forecast for this day, or likely any beyond it.
            break;
        }
        auto y = 0 + (i - 1) * 200;
        drawForecastForDay(forecast, 300, y);
    }

    drawHourlyForecast(weatherData.getHourlyWeather(), 0, 600);

    drawCityName(mDisplay.width() / 2, mDisplay.height() - 40);

    drawBatteryGauge(0, 0);
}

void Renderer::render() {
    Serial.println(F("Drawing screen buffer to display"));
    mDisplay.display();
}

void Renderer::drawCurrentConditions(
    const weather::DailyWeather& currentConditions,
    const size_t x,
    const size_t y
)
{
    Serial.println(F("Drawing current conditions"));
    auto weatherIcon = icon::iconFactory(mDisplay, currentConditions);
    static constexpr size_t iconTopMargin = 95;
    static constexpr size_t iconHeight = 300;
    weatherIcon.draw( x, y + iconTopMargin, icon::Size::s_300x300);

    mDisplay.setTextSize(1);
    mDisplay.setTextColor(BLACK, WHITE);

    mDisplay.setFont(&PatrickHand_Regular41pt7b);
    auto day = timeutils::dayNameFromEpochTimestamp(
        currentConditions.timestamp + (currentConditions.timeZone * cSecondsPerHour)
    );
    static constexpr size_t dayCenterY = 30;
    uint16_t txtW, txtH;
    std::tie(txtW, txtH) = getTextDimensions(day);
    uint16_t txtCenterX = x + (cCurrentWidth / 2);
    uint16_t txtCenterY = y + dayCenterY;
    mDisplay.setCursor(txtCenterX - (txtW / 2), txtCenterY + (txtH / 2));
    mDisplay.println(day.c_str());

    mDisplay.setFont(&PatrickHand_Regular67pt7b);
    std::string currentTemp =
        std::to_string(static_cast<uint>(std::round(currentConditions.tempNow))) + "°";
    uint16_t tempW, tempH;
    std::tie(tempW, tempH) = getTextDimensions(currentTemp);
    static constexpr size_t iconBottomMargin = 50;
    static constexpr size_t tempMargin = 25;
    auto currentTempX =  x + tempMargin;
    auto currentTempY = y + iconHeight + iconTopMargin + iconBottomMargin + tempH;
    mDisplay.setCursor(currentTempX, currentTempY);
    mDisplay.printf("%u%c", static_cast<uint>(currentConditions.tempNow), 0xB0); // °

    mDisplay.setFont(&PatrickHand_Regular31pt7b);
    static constexpr size_t paddingY = 5;
    std::string tempHigh =
        std::to_string(static_cast<uint>(std::round(currentConditions.tempHigh)));
    uint16_t tempHighW, tempHighH;
    std::tie(tempHighW, tempHighH) = getTextDimensions(tempHigh);
    auto tempHighLowMarginX = 20;
    auto tempHighX = cCurrentWidth - tempHighW - tempHighLowMarginX;
    auto tempHighY = currentTempY - tempH + tempHighH - paddingY;
    mDisplay.setCursor(tempHighX, tempHighY);
    mDisplay.printf("%u%c", static_cast<uint>(currentConditions.tempHigh), 0xB0); // °

    std::string tempLow =
        std::to_string(static_cast<uint>(std::round(currentConditions.tempLow)));
    uint16_t tempLowW, tempLowH;
    std::tie(tempLowW, tempLowH) = getTextDimensions(tempLow);
    auto tempLowX = cCurrentWidth - tempLowW - tempHighLowMarginX;
    auto tempLowY = currentTempY + paddingY;
    mDisplay.setCursor(tempLowX, tempLowY);
    mDisplay.printf("%u%c", static_cast<uint>(currentConditions.tempLow), 0xB0); // °
}

void Renderer::drawHourlyForecast(
    const weather::hourly_forecast& forecast,
    const size_t x,
    const size_t y
)
{
    Serial.println("drawing hourly");
    // draw horizon line of n pixels long
    static constexpr size_t lineWeight = 6;
    static constexpr size_t hourlyMarginX = 45;
    static constexpr size_t hourlyMarginY = 35;
    static constexpr size_t maxHoursToForecast = 8;
    auto hLineTopLeftX = x + hourlyMarginX;
    auto hLineTopLeftY = y + (cHourlyHeight / 2) + hourlyMarginY;
    auto hLineW = cHourlyWidth - hourlyMarginX - hLineTopLeftX;
    auto hLineH = lineWeight;
    mDisplay.fillRect(
        hLineTopLeftX,
        hLineTopLeftY,
        hLineW,
        hLineH,
        BLACK
    );

    // Hourly data could be stale so make sure we're only showing data for the future
    size_t offset = 0;
    for (size_t i = 0; i < forecast.size(); i++)
    {
        if (forecast[i].timestamp > timeutils::localTime(forecast[i].timeZone))
        {
            offset = i;
            Serial.printf("Hourly data has an offset of %u\n", i);
            break;
        }
    }
    auto hoursToForecast = std::min(forecast.size() - offset, maxHoursToForecast);
    if (hoursToForecast < maxHoursToForecast)
    {
        Serial.println("WARNING: There is not enough data to fully populate the hourly forecast.");
    }
    auto textMarginH = 10;

    mDisplay.setTextColor(BLACK, WHITE);
    mDisplay.setTextSize(1);
    size_t spacing = hLineW / (hoursToForecast - 1);
    for (size_t i = 0; i < hoursToForecast; i++)
    {
        static constexpr size_t tickHeight = 20;
        auto tickTopLeftX =  (spacing * i) + hLineTopLeftX;
        auto tickTopLeftY = hLineTopLeftY - tickHeight;
        auto tickBottomRightX = tickTopLeftX + lineWeight;
        auto tickBottomRightY = hLineTopLeftY;
        mDisplay.fillRect(
            tickTopLeftX, tickTopLeftY,
            lineWeight, tickHeight,
            BLACK
        );
        auto& hourlyForecast = forecast[i + offset];
        auto tickTopCenterX = tickTopLeftX + (lineWeight / 2);
        auto tickTopCenterY = tickTopLeftY;
        auto tickBottomCenterX = tickBottomRightX - (lineWeight / 2);
        auto tickBottomCenterY = tickBottomRightY;

        mDisplay.setFont(&PatrickHand_Regular16pt7b);
        std::string chanceRain =
            std::to_string(static_cast<uint>(std::round(hourlyForecast.chanceOfPrecipitation * 100)))  + "%";
        uint16_t chanceRainW, chanceRainH;
        std::tie(chanceRainW, chanceRainH) = getTextDimensions(chanceRain);
        uint16_t chanceRainX = tickTopCenterX - (chanceRainW / 2);
        uint16_t chanceRainY = tickTopCenterY - textMarginH;
        mDisplay.setCursor(chanceRainX, chanceRainY);
        mDisplay.println(chanceRain.c_str());

        mDisplay.setFont(&PatrickHand_Regular21pt7b);
        std::string temp =
            std::to_string(static_cast<uint>(std::round(hourlyForecast.temp)));
        uint16_t tempW, tempH;
        std::tie(tempW, tempH) = getTextDimensions(temp);
        uint16_t tempX = tickTopCenterX - (tempW / 2);
        uint16_t tempY = tickTopCenterY - (textMarginH * 2) - chanceRainH;
        mDisplay.setCursor(tempX, tempY);
        mDisplay.println(temp.c_str());

        mDisplay.setFont(&PatrickHand_Regular16pt7b);
        std::string time = timeutils::hour12FromEpochTimestamp(
            timeutils::localTime(hourlyForecast.timestamp, hourlyForecast.timeZone)
        );
        uint16_t timeW, timeH;
        std::tie(timeW, timeH) = getTextDimensions(time);
        uint16_t timeX = tickBottomCenterX - (timeW / 2);
        uint16_t timeY = tickBottomCenterY + timeH + textMarginH;
        mDisplay.setCursor(timeX, timeY);
        mDisplay.println(time.c_str());
    }
}

void Renderer::drawForecastForDay(
    const weather::DailyWeather& forecast,
    const size_t x,
    const size_t y
)
{
    Serial.println(F("Drawing forecast"));
    // Totally clear area to create visual separation
    static constexpr size_t iconTopMarginY = 5;
    static constexpr size_t textTopMarginY = 25;
    static constexpr size_t textRightMarginX = 5;

    auto weatherIcon = icon::iconFactory(mDisplay, forecast);
    static constexpr size_t iconWidth = 150;
    weatherIcon.draw( x, y + iconTopMarginY, icon::Size::s_150x150);

    mDisplay.setTextSize(1);
    mDisplay.setTextColor(BLACK, WHITE);
    mDisplay.setFont(&PatrickHand_Regular31pt7b);
    static constexpr size_t tempLowToDayMarginY = 15;

    std::string tempHigh =
        std::to_string(static_cast<uint>(std::round(forecast.tempHigh)));
    uint16_t tempHighW, tempHighH;
    std::tie(tempHighW, tempHighH) = getTextDimensions(tempHigh);
    int16_t tempMarginX = 5;
    auto tempHighX = x + iconWidth + tempMarginX;
    auto tempHighY = y + textTopMarginY + tempHighH;
    mDisplay.setCursor(tempHighX, tempHighY);
    mDisplay.printf("%u%c", static_cast<uint>(forecast.tempHigh), 0xB0); // °

    std::string tempLow =
        std::to_string(static_cast<uint>(std::round(forecast.tempLow)));
    uint16_t tempLowW, tempLowH;
    std::tie(tempLowW, tempLowH) = getTextDimensions(tempLow);
    auto tempLowX = tempHighX;
    auto tempLowY = y + cForecastHeight - tempLowH - tempLowToDayMarginY;
    mDisplay.setCursor(tempLowX, tempLowY);
    mDisplay.printf("%u%c", static_cast<uint>(forecast.tempLow), 0xB0); // °

    mDisplay.setFont(&PatrickHand_Regular26pt7b);
    std::string chanceRain =
        std::to_string(static_cast<uint>(std::round(forecast.chanceOfPrecipitation * 100))) + "%";
    uint16_t chanceRainW, ChanceRainH;
    std::tie(chanceRainW, ChanceRainH) = getTextDimensions(chanceRain);
    auto chanceRainX = x + cForecastWidth - chanceRainW - textRightMarginX;//iconWidth + chanceRainOffsetX + chanceRainMarginX;
    auto chanceRainY = y + (cForecastHeight / 2);
    mDisplay.setCursor(chanceRainX, chanceRainY);
    mDisplay.printf("%s%c", chanceRain.c_str(), 0xB0); // °

    static constexpr size_t dayBottomMarginY = 15;
    // This time will be midnight GMT, no need to convert to local timezone
    auto day = timeutils::dayNameFromEpochTimestamp(forecast.timestamp);
    uint16_t txtW, txtH;
    std::tie(txtW, txtH) = getTextDimensions(day);
    uint16_t txtCenterX = x + (cForecastWidth / 2);
    uint16_t txtCenterY = y + cForecastHeight - (txtH / 2) - dayBottomMarginY;//+ tempLowToDayMarginY;
    mDisplay.setCursor(txtCenterX - (txtW / 2), txtCenterY + (txtH / 2));
    mDisplay.println(day.c_str());
}

void Renderer::drawBatteryGauge(size_t x, size_t y)
{
    auto battery = icon::BatteryGauge(mDisplay);
    battery.draw(520, 735, icon::Size::s_75x75);
}

void Renderer::drawCityName(size_t x, size_t y)
{
    mDisplay.setTextColor(BLACK, WHITE);
    mDisplay.setFont(&PatrickHand_Regular41pt7b);
    mDisplay.setTextSize(1);

    uint16_t txtW, txtH;
    std::tie(txtW, txtH) = getTextDimensions(cCity);
    mDisplay.setCursor(x - (txtW / 2), y + (txtH / 2));
    mDisplay.println(cCity);
}

std::pair<uint16_t, uint16_t> Renderer::getTextDimensions(const std::string& text) const
{
    int16_t  txtEndX, txtEndY;
    uint16_t txtW, txtH;
    mDisplay.getTextBounds(text.c_str(), 0u, 0u, &txtEndX, &txtEndY, &txtW, &txtH);
    return std::make_pair(txtW, txtH);
}
