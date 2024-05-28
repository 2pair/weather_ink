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
#include "DailyWeather.h"
#include "Weather.h"
#include "DayStrings.h"

using namespace renderer;

Renderer::Renderer(Inkplate& display, const char* city)
    :   mDisplay(display),
        cCity(city)
{}

void Renderer::update(const weather::Weather& weatherData)
{
    Serial.println(F("Updating screen buffer"));
    mDisplay.clearDisplay();

    static constexpr size_t currentDrawStartX = 400;
    static constexpr size_t currentDrawStartY = 0;
    drawCurrentConditions(
        weatherData.getDailyWeather(0),
        currentDrawStartX,
        currentDrawStartY
    );

    static constexpr size_t hourlyDrawStartX = 720;
    static constexpr size_t hourlyDrawStartY = 0;
    drawHourlyForecast(
        weatherData.getHourlyWeather(),
        hourlyDrawStartX,
        hourlyDrawStartY
    );

    static constexpr size_t daysToForecast = 3;
    static constexpr size_t forecastDrawStartX = 0;
    static constexpr size_t forecastDrawStartY = 0;
    for (size_t i = 1; i < 1 + daysToForecast; i++)
    {
        auto& forecast = weatherData.getDailyWeather(i);
        auto y = forecastDrawStartY + (i - 1) * 200;
        drawForecastForDay(forecast, forecastDrawStartX, y);

    }

    // TODO does this return 0 with no battery connected?
    if (mDisplay.readBattery() > 0.0)
    {
        drawBatteryGauge(0, 0);
    }
}

void Renderer::render() {
    Serial.println(F("Drawing screen buffer to display"));
    mDisplay.display();
}

void Renderer::drawCurrentConditions(
    const weather::DailyWeather& currentConditions,
    size_t x,
    size_t y
)
{
    Serial.println(F("Drawing current conditions"));
    // The current conditions will take 1/2 of the screen
    static constexpr size_t width = 400;
    static constexpr size_t height = 300;

    auto iconName = icon::Icon::getIconNameForConditions(currentConditions);
    icon::Icon weatherIcon(mDisplay, iconName);
    Serial.println(F("Drawing icon"));
    static constexpr size_t iconWidth = 300;
    static constexpr size_t iconHeight = 300;
    static constexpr size_t iconWMargin = (width - iconWidth) / 2;
    static constexpr size_t iconHMargin = 25;
    weatherIcon.draw( x + iconWMargin, y + iconHMargin, icon::Size::s_300x300);

    Serial.println("drawing text");
    mDisplay.setTextSize(1);
    mDisplay.setTextColor(BLACK, WHITE);

    // Draw weather state
    mDisplay.setFont(&PatrickHand_Regular67pt7b);
    std::string currentTemp =
        std::to_string(static_cast<uint>(std::round(currentConditions.tempNow))) + "°";
    int16_t  txtEndX, txtEndY;
    uint16_t txtW, txtH;
    mDisplay.getTextBounds(currentTemp.c_str(), 0u, 0u, &txtEndX, &txtEndY, &txtW, &txtH);
    mDisplay.setCursor(
        x + iconWMargin,
        y + iconHeight + (iconHMargin * 2)
    );

    // get the width of the high and low, determine margins, draw both
    // also get the height of the high and low, position relative to the current weather
    mDisplay.printf("%uF", static_cast<uint>(currentConditions.tempNow));

    mDisplay.setFont(&PatrickHand_Regular26pt7b);
    mDisplay.setTextSize(1);
    auto conditions = weather::conditionToString(currentConditions.condition);
    mDisplay.setCursor(775 - 45 * conditions.size(), 300);
    mDisplay.println(conditions.c_str());

    // Drawing city name
    mDisplay.setTextColor(BLACK, WHITE);
    mDisplay.setFont(&PatrickHand_Regular26pt7b);
    mDisplay.setTextSize(1);

    mDisplay.setCursor(775 - 50 * strlen(cCity), 570);
    mDisplay.println(cCity);
}

void Renderer::drawHourlyForecast(
    const weather::hourly_forecast& forecast,
    size_t x,
    size_t y
)
{
    // draw horizon line of n pixels long
    // draw hourly_forecast.size() vertical lines at even intervals
    // draw hour, temp, and percent chance of precipitation
}

// Function for drawing temperatures
void Renderer::drawForecastForDay(
    const weather::DailyWeather& forecast,
    size_t x,
    size_t y
)
{
    Serial.println(F("Drawing forecast"));
    // Drawing 3 rectangles in which temperatures will be written
    int height = 175;
    int width = 370;
    int spacing = (600 - height * 3) / 4;

    mDisplay.fillRect(20, 1 * spacing + 0 * height, 20 + width, height, 6);
    mDisplay.fillRect(20, 2 * spacing + 1 * height, 20 + width, height, 5);
    mDisplay.fillRect(20, 3 * spacing + 2 * height, 20 + width, height, 4);
/*
    int textMargin = 6;

    mDisplay.setFont(&Inter20pt7b);
    mDisplay.setTextSize(1);
    mDisplay.setTextColor(WHITE, BLACK);

    mDisplay.setCursor(1 * rectSpacing + 0 * rectWidth + textMargin, 300 + textMargin + 40);
    mDisplay.println("Today");

    mDisplay.setCursor(2 * rectSpacing + 1 * rectWidth + textMargin, 300 + textMargin + 40);
    mDisplay.println(wDays[hours + 1 > 6 ? hours + 1 - 6 : hours + 1]);

    mDisplay.setCursor(3 * rectSpacing + 2 * rectWidth + textMargin, 300 + textMargin + 40);
    mDisplay.println(wDays[hours + 2 > 6 ? hours + 2 - 6 : hours + 2]);

    mDisplay.setCursor(4 * rectSpacing + 3 * rectWidth + textMargin, 300 + textMargin + 40);
    mDisplay.println(wDays[hours + 3 > 6 ? hours + 3 - 6 : hours + 3]);

    // Drawing temperature values into black rectangles
    mDisplay.setFont(&Inter20pt7b);
    mDisplay.setTextSize(1);
    mDisplay.setTextColor(WHITE, BLACK);

    mDisplay.setCursor(1 * rectSpacing + 0 * rectWidth + textMargin, 300 + textMargin + 120);
    mDisplay.print(temps[0]);
    mDisplay.println(F("C"));

    mDisplay.setCursor(2 * rectSpacing + 1 * rectWidth + textMargin, 300 + textMargin + 120);
    mDisplay.print(temps[1]);
    mDisplay.println(F("C"));

    mDisplay.setCursor(3 * rectSpacing + 2 * rectWidth + textMargin, 300 + textMargin + 120);
    mDisplay.print(temps[2]);
    mDisplay.println(F("C"));

    mDisplay.setCursor(4 * rectSpacing + 3 * rectWidth + textMargin, 300 + textMargin + 120);
    mDisplay.print(temps[3]);
    mDisplay.println(F("C"));
*/
}

void Renderer::drawBatteryGauge(size_t x, size_t y)
{
    auto batteryVoltage = mDisplay.readBattery();
    Serial.printf((const char*)F("Battery voltage is %.3f V\n"), batteryVoltage);
    // TODO: Map characteristic curve of battery voltage to a percent remaining
    // TODO: Draw the icon
}

// Current weather drawing function
// void Renderer::drawForecast()
// {
//     // Drawing current information

//     // Temperature:
//     mDisplay.setFont(&Inter48pt7b);
//     mDisplay.setTextSize(1);
//     mDisplay.setTextColor(BLACK, WHITE);

//     mDisplay.setCursor(245, 150);
//     mDisplay.print(currentTemp);

//     int x = mDisplay.getCursorX();
//     int y = mDisplay.getCursorY();

//     mDisplay.setFont(&Inter20pt7b);
//     mDisplay.setTextSize(1);

//     mDisplay.setCursor(x, y);

//     mDisplay.println(F("C"));

//     // Wind:
//     mDisplay.setFont(&Inter48pt7b);
//     mDisplay.setTextSize(1);
//     mDisplay.setTextColor(BLACK, WHITE);

//     mDisplay.setCursor(480, 150);
//     mDisplay.print(currentWind);

//     x = mDisplay.getCursorX();
//     y = mDisplay.getCursorY();

//     mDisplay.setFont(&Inter20pt7b);
//     mDisplay.setTextSize(1);

//     mDisplay.setCursor(x, y);

//     mDisplay.println(F("m/s"));

//     // Labels underneath
//     mDisplay.setFont(&Inter16pt7b);
//     mDisplay.setTextSize(1);

//     mDisplay.setCursor(215, 210);
//     mDisplay.println(F("TEMPERATURE"));

//     mDisplay.setCursor(500, 210);
//     mDisplay.println(F("WIND SPEED"));
// }
