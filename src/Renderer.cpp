#include "Renderer.h"

#include <cmath>
#include <string>
#include <WString.h>
#include <numeric>

#include <esp32-hal-log.h>
#include <Inkplate.h>
#include <ArduinoJson.h>

#include "../fonts/PatrickHand_Regular12pt7b.h"
#include "../fonts/PatrickHand_Regular16pt7b.h"
#include "../fonts/PatrickHand_Regular21pt7b.h"
#include "../fonts/PatrickHand_Regular26pt7b.h"
#include "../fonts/PatrickHand_Regular31pt7b.h"
#include "../fonts/PatrickHand_Regular41pt7b.h"
#include "../fonts/PatrickHand_Regular48pt7b.h"
#include "../fonts/PatrickHand_Regular56pt7b.h"
#include "../fonts/PatrickHand_Regular67pt7b.h"

#include "Icon.h"
#include "BatteryGuage.h"
#include "DailyWeather.h"
#include "Weather.h"
#include "TimeUtils.h"

using namespace renderer;

Renderer::Renderer(Inkplate& display)
    :   mDisplay(display)
{
    mDisplay.clearDisplay();
}

void Renderer::update(const weather::Weather& weatherData, const char* city)
{
    log_i("Updating screen buffer");
    mDisplay.setTextWrap(false);

    static constexpr size_t maxDaysToForecast = 3;
    auto daysForecasted = weatherData.getDailyForecastLength() - 1;
    auto daysToForecast = std::min(maxDaysToForecast, daysForecasted);
    log_i("drawing %u forecasted days", daysToForecast);
    for (size_t i = 1; i < 1 + daysToForecast; i++)
    {
        auto& forecast = weatherData.getDailyWeather(i);
        if (forecast.condition == weather::Condition::unknown)
        {
            // This indicates we don't have forecast for this day, or likely any beyond it.
            log_w("Forecast for day at index %d had unknown conditions", i);
            weather::Weather::printDailyWeather(weatherData.getDailyWeather(i));
            break;
        }
        auto y = 0 + (i - 1) * 200;
        drawForecastForDay(forecast, 300, y);
    }

    drawCurrentConditions(weatherData.getDailyWeather(0), 0, 0);

    drawHourlyForecast(weatherData.getHourlyWeather(), 0, 600);

    drawCityName(city, mDisplay.width() / 2, mDisplay.height() - 40);

    drawBatteryGauge(520, 735);

    drawLastUpdated(5, 750, weatherData.getDailyWeather(0).timeZone);
}

void Renderer::render() {
    log_i("Drawing screen buffer to display");
    mDisplay.preloadScreen();
    mDisplay.display();
}

void Renderer::drawLinesCentered(
    const std::vector<std::string>& lines,
    const GFXfont& font,
    const size_t lineSpacing
)
{
    log_d("writing text to display");
    mDisplay.setTextSize(1);
    mDisplay.setTextColor(BLACK, WHITE);
    mDisplay.setFont(&font);

    std::vector<uint16_t> widths, heights;
    widths.reserve(lines.size());
    heights.reserve(lines.size());
    for (auto& line : lines)
    {
        uint16_t lineW, lineH;
        std::tie(lineW, lineH) = getTextDimensions(line);
        widths.emplace_back(lineW);
        heights.emplace_back(lineH);
    }
    auto totalHeight = std::accumulate(heights.begin(), heights.end(), 0)
        + (lineSpacing * (heights.size() - 1));
    auto textStartVertical = (mDisplay.height() - totalHeight) / 2;
    auto displayWidth = mDisplay.width();

    auto lastLineStartVertical = textStartVertical;
    for (size_t i = 0; i < lines.size(); i++)
    {
        auto lineStartHorizontal = (displayWidth - widths.at(i)) / 2;
        auto lineStartVertical = lastLineStartVertical + heights.at(i) + (lineSpacing);
        lastLineStartVertical = lineStartVertical;
        mDisplay.setCursor(lineStartHorizontal, lineStartVertical);
        mDisplay.print(lines.at(i).c_str());
    }
}

void Renderer::drawCurrentConditions(
    const weather::DailyWeather& currentConditions,
    const size_t x,
    const size_t y
)
{
    weather::Weather::printDailyWeather(currentConditions);
    auto weatherIcon = icon::iconFactory(mDisplay, currentConditions);
    static constexpr size_t iconTopMargin = 95, iconBottomMargin = 15, iconHeight = 300;
    static constexpr size_t currentRightMarginX = 15, tempYMargin = 10, HighLowMarginY = 20;
    weatherIcon.draw(x, y + iconTopMargin, iconHeight);

    mDisplay.setTextSize(1);
    mDisplay.setTextColor(BLACK, WHITE);

    // Day name
    mDisplay.setFont(&PatrickHand_Regular41pt7b);
    auto day = timeutils::dayNameFromEpochTimestamp(
        timeutils::localTime(currentConditions.timestamp, currentConditions.timeZone)
    );
    static constexpr size_t dayCenterY = 30;
    uint16_t txtW, txtH;
    std::tie(txtW, txtH) = getTextDimensions(day);
    uint16_t txtCenterX = x + (cCurrentWidth / 2);
    uint16_t txtCenterY = y + dayCenterY;
    mDisplay.setCursor(txtCenterX - (txtW / 2), txtCenterY + (txtH / 2));
    mDisplay.println(day.c_str());

    // Current temp
    mDisplay.setFont(&PatrickHand_Regular48pt7b);
    std::string currentTemp =
        std::to_string(static_cast<int>(std::round(currentConditions.tempNow))) + "°";
    uint16_t tempW, tempH;
    std::tie(tempW, tempH) = getTextDimensions(currentTemp);
    const size_t currentTempX =  x + tempYMargin;
    const size_t currentTempY = y + iconTopMargin + iconHeight + iconBottomMargin + tempH;
    mDisplay.setCursor(currentTempX, currentTempY);
    mDisplay.println(currentTemp.c_str());

    // Today's high temp
    mDisplay.setFont(&PatrickHand_Regular16pt7b);
    std::string highTempId = "H: ";
    uint16_t highTempIdW, highTempIdH;
    std::tie(highTempIdW, highTempIdH) = getTextDimensions(highTempId);
    // Compute and place the temp position so we can use its dimensions in the ID position

    mDisplay.setFont(&PatrickHand_Regular31pt7b);
    std::string tempHigh =
        std::to_string(static_cast<int>(std::round(currentConditions.tempHigh))) + "°";
    uint16_t tempHighW, tempHighH;
    std::tie(tempHighW, tempHighH) = getTextDimensions(tempHigh);
    //auto tempHighX =  x + cCurrentWidth - (tempHighW + currentRightMarginX);
    const size_t tempHighX =  x + (cCurrentWidth * 0.59) + highTempIdW;
    const size_t tempHighY = currentTempY - tempH + tempHighH;
    mDisplay.setCursor(tempHighX, tempHighY);
    mDisplay.println(tempHigh.c_str());

    mDisplay.setFont(&PatrickHand_Regular16pt7b);
    const size_t highTempIdX =  tempHighX - highTempIdW;
    const size_t highTempIdY = tempHighY;
    mDisplay.setCursor(highTempIdX, highTempIdY);
    mDisplay.println(highTempId.c_str());

    // Today's low temp
    mDisplay.setFont(&PatrickHand_Regular16pt7b);
    std::string lowTempId = "L: ";
    uint16_t lowTempIdW, lowTempIdH;
    std::tie(lowTempIdW, lowTempIdH) = getTextDimensions(lowTempId);
    // Compute and place the temp position so we can use its dimensions in the ID position

    mDisplay.setFont(&PatrickHand_Regular31pt7b);
    std::string tempLow =
        std::to_string(static_cast<int>(std::round(currentConditions.tempLow))) + "°";
    uint16_t tempLowW, tempLowH;
    std::tie(tempLowW, tempLowH) = getTextDimensions(tempLow);
    const size_t tempLowX = tempHighX;
    const size_t tempLowY = tempHighY + tempLowH + HighLowMarginY;
    mDisplay.setCursor(tempLowX, tempLowY);
    mDisplay.println(tempLow.c_str());

    mDisplay.setFont(&PatrickHand_Regular16pt7b);
    const size_t lowTempIdX =  tempLowX - lowTempIdW;
    const size_t lowTempIdY = tempLowY;
    mDisplay.setCursor(lowTempIdX, lowTempIdY);
    mDisplay.println(lowTempId.c_str());

    // Today's sunrise
    static constexpr size_t sunTextXMargin = 10, sunTextMarginY = 10;
    static constexpr int dataIconMarginX = -4, dataIconMarginY = -3;
    mDisplay.setFont(&PatrickHand_Regular16pt7b);

    tm timeData;
    auto sunriseTimestamp = timeutils::localTime(
        currentConditions.sunrise,
        currentConditions.timeZone
    );
    gmtime_r(&sunriseTimestamp, &timeData);
    std::array<char, 9> sunrise;
    strftime(sunrise.data(), sunrise.size(), "%I:%M %p", &timeData);
    uint16_t sunriseW, sunriseH;
    std::tie(sunriseW, sunriseH) = getTextDimensions(sunrise.data());
    auto sunriseIcon = icon::Icon(mDisplay, icon::cSunriseIconName);
    const size_t sunriseStartY = currentTempY + sunriseH + sunTextMarginY;
    static constexpr size_t sunriseIconSize = 50;
    const size_t sunriseIconX = x + dataIconMarginX;
    const size_t sunriseIconY = y + cCurrentHeight - sunriseIconSize - dataIconMarginY;
    sunriseIcon.draw(sunriseIconX, sunriseIconY, sunriseIconSize);
    const size_t sunriseTextX = sunriseIconX + sunriseIconSize + sunTextXMargin;
    const size_t sunriseTextY = sunriseIconY + ((sunriseIconSize - sunriseH) / 2) + sunriseH - dataIconMarginY;
    mDisplay.setCursor(sunriseTextX, sunriseTextY);
    mDisplay.println(sunrise.data());

    // Today's sunset
    auto sunsetTimestamp = timeutils::localTime(
        currentConditions.sunset,
        currentConditions.timeZone
    );
    gmtime_r(&sunsetTimestamp, &timeData);
    std::array<char, 9> sunset;
    strftime(sunset.data(), sunset.size(), "%I:%M %p", &timeData);
    uint16_t sunsetW, sunsetH;
    std::tie(sunsetW, sunsetH) = getTextDimensions(sunset.data());
    auto sunsetIcon = icon::Icon(mDisplay, icon::cSunsetIconName);
    const size_t sunsetIconX = currentTempX + ((cCurrentWidth + sunTextXMargin) / 2);
    const size_t sunsetIconY = sunriseIconY;
    static constexpr size_t sunsetIconSize = sunriseIconSize;
    sunsetIcon.draw(sunsetIconX, sunsetIconY, sunsetIconSize);
    const size_t sunsetTextX = sunsetIconX + sunsetIconSize + sunTextXMargin;
    const size_t sunsetTextY = sunsetIconY + ((sunsetIconSize - sunsetH) / 2) + sunsetH - dataIconMarginY;
    mDisplay.setCursor(sunsetTextX, sunsetTextY);
    mDisplay.println(sunset.data());

    // Current humidity
    static constexpr size_t humidityIconMarginY = 2;
    static constexpr size_t humidityTextXMargin = sunTextXMargin, humidityTextYMargin = sunTextMarginY;
    std::string humidity =
        std::to_string(static_cast<int>(currentConditions.humidity)) + "%";
    uint16_t humidityW, humidityH;
    std::tie(humidityW, humidityH) = getTextDimensions(humidity);
    static constexpr size_t humidityIconSize = 35;
    auto humidityIcon = icon::Icon(mDisplay, icon::cHumidityIconName);
    const size_t humidityIconX = x - dataIconMarginX;
    const size_t humidityIconY = sunriseIconY - humidityIconSize - humidityIconMarginY;
    humidityIcon.draw(humidityIconX, humidityIconY, humidityIconSize);
    const size_t humidityTextX = humidityIconX + humidityIconSize + humidityTextXMargin;
    const size_t humidityTextY = humidityIconY + ((humidityIconSize - humidityH) / 2) + humidityH - dataIconMarginY;
    mDisplay.setCursor(humidityTextX, humidityTextY);
    mDisplay.println(humidity.data());

    // Current wind
    static constexpr size_t windTextXMargin = humidityTextXMargin, windTextYMargin = humidityTextYMargin;
    std::string wind = std::to_string(currentConditions.windSpeed);
    auto position = wind.find(".");
    if (position != std::string::npos)
    {
        // truncate to one decimals
        wind.resize((position + 1) + 1);
    }
    wind += (currentConditions.metricUnits ? " kph " : " mph ");
    wind += weather::windDegreeToDirection(currentConditions.windDirection);
    uint16_t windW, windH;
    std::tie(windW, windH) = getTextDimensions(wind);
    const size_t windTextX = x + (cCurrentWidth * 0.45);
    const size_t windyTextY = humidityTextY;
    mDisplay.setCursor(windTextX, windyTextY);
    mDisplay.println(wind.data());

}

void Renderer::drawHourlyForecast(
    const weather::hourly_forecast& forecast,
    const size_t x,
    const size_t y
)
{
    log_i("drawing hourly");
    // draw horizontal line n pixels in length
    static constexpr size_t lineWeight = 3;
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
        weather::Weather::printHourlyWeather(forecast[i]);
        if (forecast[i].timestamp > time(nullptr))
        {
            offset = i;
            log_d("Hourly data has an offset of %u", i);
            break;
        }
    }
    auto hoursToForecast = std::min(forecast.size() - offset, maxHoursToForecast);
    if (hoursToForecast < maxHoursToForecast)
    {
        log_w("There is not enough data to fully populate the hourly forecast.");
    }
    static constexpr size_t textMarginY = 15;
    static constexpr size_t textMarginX = 4;

    mDisplay.setTextColor(BLACK, WHITE);
    mDisplay.setTextSize(1);
    size_t spacing = hLineW / (hoursToForecast - 1);
    for (size_t i = 0; i < hoursToForecast; i++)
    {
        // Draw a tick mark for this hour's data on the hourly line
        static constexpr size_t tickHeight = 5;
        static constexpr size_t tickWeight = 6;
        auto tickTopLeftX =  (spacing * i) + hLineTopLeftX;
        auto tickTopLeftY = hLineTopLeftY - tickHeight;
        auto tickBottomRightX = tickTopLeftX + tickWeight;
        auto tickBottomRightY = hLineTopLeftY;
        mDisplay.fillRect(
            tickTopLeftX, tickTopLeftY,
            tickWeight, tickHeight,
            BLACK
        );

        auto& hourlyForecast = forecast[i + offset];
        auto tickTopCenterX = tickTopLeftX + (tickWeight / 2);
        auto tickTopCenterY = tickTopLeftY;
        auto tickBottomCenterX = tickBottomRightX - (tickWeight / 2);
        auto tickBottomCenterY = tickBottomRightY;

        // Chance of precipitation and icon share a line
        static constexpr size_t iconWidth = 35, iconHeight = iconWidth;
        static constexpr size_t iconMarginH = 5;
        mDisplay.setFont(&PatrickHand_Regular12pt7b);
        std::string chanceRain =
            std::to_string(static_cast<uint>(std::round(hourlyForecast.chanceOfPrecipitation * 100)))  + "%";
        uint16_t chanceRainW, chanceRainH;
        std::tie(chanceRainW, chanceRainH) = getTextDimensions(chanceRain);
        uint16_t combinedWidth = chanceRainW + textMarginX + iconWidth;
        uint16_t chanceRainX = tickTopCenterX - (combinedWidth / 2) +  textMarginX + iconWidth;
        uint16_t chanceRainY = tickTopCenterY - textMarginY;
        mDisplay.setCursor(chanceRainX, chanceRainY);
        mDisplay.println(chanceRain.c_str());

        auto weatherIcon = icon::iconFactory(mDisplay, hourlyForecast);
        uint16_t weatherIconX = tickTopCenterX - (combinedWidth / 2);
        uint16_t weatherIconY = tickTopCenterY - iconMarginH - iconHeight;
        weatherIcon.draw(weatherIconX, weatherIconY, iconWidth);

        mDisplay.setFont(&PatrickHand_Regular21pt7b);
        std::string temp =
            std::to_string(static_cast<int>(std::round(hourlyForecast.tempNow))) + "°";
        uint16_t tempW, tempH;
        std::tie(tempW, tempH) = getTextDimensions(temp);
        uint16_t tempX = tickTopCenterX - (tempW / 2);
        uint16_t tempY = tickTopCenterY - (textMarginY * 2) - chanceRainH;
        mDisplay.setCursor(tempX, tempY);
        mDisplay.println(temp.c_str());

        mDisplay.setFont(&PatrickHand_Regular16pt7b);
        std::string time = timeutils::hour12FromEpochTimestamp(
            timeutils::localTime(hourlyForecast.timestamp, hourlyForecast.timeZone)
        );
        uint16_t timeW, timeH;
        std::tie(timeW, timeH) = getTextDimensions(time);
        uint16_t timeX = tickBottomCenterX - (timeW / 2);
        uint16_t timeY = tickBottomCenterY + timeH + textMarginY;
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
    log_i("Drawing forecast");
    // Totally clear area to create visual separation
    static constexpr size_t iconTopMarginY = 5;
    static constexpr size_t textTopMarginY = 15;
    static constexpr size_t textRightMarginX = 5;

    auto weatherIcon = icon::iconFactory(mDisplay, forecast);
    static constexpr size_t iconWidth = 150;
    weatherIcon.draw( x, y + iconTopMarginY, iconWidth);

    mDisplay.setTextSize(1);
    mDisplay.setTextColor(BLACK, WHITE);
    static constexpr size_t dataMarginY = 13;

    mDisplay.setFont(&PatrickHand_Regular16pt7b);
    std::string highTempId = "H: ";
    uint16_t highTempIdW, highTempIdH;
    std::tie(highTempIdW, highTempIdH) = getTextDimensions(highTempId);
    // Compute and place the temp position so we can use its dimensions in the ID position

    mDisplay.setFont(&PatrickHand_Regular26pt7b);
    std::string tempHigh =
        std::to_string(static_cast<int>(std::round(forecast.tempHigh))) + "°";
    uint16_t tempHighW, tempHighH;
    std::tie(tempHighW, tempHighH) = getTextDimensions(tempHigh);
    const size_t tempHighX = x + (cForecastWidth * 0.62) + highTempIdW;
    const size_t tempHighY = y + textTopMarginY + tempHighH;
    mDisplay.setCursor(tempHighX, tempHighY);
    mDisplay.println(tempHigh.c_str());

    mDisplay.setFont(&PatrickHand_Regular16pt7b);
    const size_t highTempIdX =  tempHighX - highTempIdW;
    const size_t highTempIdY = tempHighY;
    mDisplay.setCursor(highTempIdX, highTempIdY);
    mDisplay.println(highTempId.c_str());

    mDisplay.setFont(&PatrickHand_Regular16pt7b);
    std::string lowTempId = "L: ";
    uint16_t lowTempIdW, lowTempIdH;
    std::tie(lowTempIdW, lowTempIdH) = getTextDimensions(lowTempId);

    mDisplay.setFont(&PatrickHand_Regular26pt7b);
    std::string tempLow =
        std::to_string(static_cast<int>(std::round(forecast.tempLow))) + "°";
    uint16_t tempLowW, tempLowH;
    std::tie(tempLowW, tempLowH) = getTextDimensions(tempLow);
    const size_t tempLowX = tempHighX;
    const size_t tempLowY = tempHighY + tempLowH + dataMarginY;
    mDisplay.setCursor(tempLowX, tempLowY);
    mDisplay.println(tempLow.c_str());

    mDisplay.setFont(&PatrickHand_Regular16pt7b);
    const size_t lowTempIdX =  tempLowX - lowTempIdW;
    const size_t lowTempIdY = tempLowY;
    mDisplay.setCursor(lowTempIdX, lowTempIdY);
    mDisplay.println(lowTempId.c_str());

    mDisplay.setFont(&PatrickHand_Regular16pt7b);
    std::string precipId =
        (forecast.condition == weather::Condition::snow || forecast.condition == weather::Condition::wintryMix)
        ? "snow: " : "rain: ";
    uint16_t precipIdW, precipIdH;
    std::tie(precipIdW, precipIdH) = getTextDimensions(precipId);

    mDisplay.setFont(&PatrickHand_Regular26pt7b);
    std::string chanceRain =
        std::to_string(static_cast<uint>(std::round(forecast.chanceOfPrecipitation * 100))) + "%";
    uint16_t chanceRainW, chanceRainH;
    std::tie(chanceRainW, chanceRainH) = getTextDimensions(chanceRain);
    const size_t chanceRainX = tempLowX;
    const size_t chanceRainY = tempLowY + chanceRainH + dataMarginY;
    mDisplay.setCursor(chanceRainX, chanceRainY);
    mDisplay.println(chanceRain.c_str());

    mDisplay.setFont(&PatrickHand_Regular16pt7b);
    const size_t precipIdX =  chanceRainX - precipIdW;
    const size_t precipIdY = chanceRainY;
    mDisplay.setCursor(precipIdX, precipIdY);
    mDisplay.println(precipId.c_str());

    mDisplay.setFont(&PatrickHand_Regular26pt7b);
    static constexpr size_t dayBottomMarginY = 10;
    // This time will be midnight GMT, no need to convert to local timezone
    auto day = timeutils::dayNameFromEpochTimestamp(forecast.timestamp);
    uint16_t txtW, txtH;
    std::tie(txtW, txtH) = getTextDimensions(day);
    uint16_t txtCenterX = x + (cForecastWidth / 2);
    uint16_t txtCenterY = y + cForecastHeight - (txtH / 2) - dayBottomMarginY;
    mDisplay.setCursor(txtCenterX - (txtW / 2), txtCenterY + (txtH / 2));
    mDisplay.println(day.c_str());
}

void Renderer::drawBatteryGauge(size_t x, size_t y)
{
    auto battery = icon::BatteryGauge(mDisplay);
    battery.draw(x, y, 75);
}

void Renderer::drawCityName(const char* city, size_t x, size_t y)
{
    mDisplay.setTextColor(BLACK, WHITE);
    mDisplay.setFont(&PatrickHand_Regular41pt7b);
    mDisplay.setTextSize(1);

    uint16_t txtW, txtH;
    std::tie(txtW, txtH) = getTextDimensions(city);
    mDisplay.setCursor(x - (txtW / 2), y + (txtH / 2));
    mDisplay.println(city);
}

void Renderer::drawLastUpdated(size_t x, size_t y, int8_t timeZone)
{
    mDisplay.setTextColor(BLACK, WHITE);
    mDisplay.setFont(&PatrickHand_Regular12pt7b);
    mDisplay.setTextSize(1);

    auto nowSecs = timeutils::localTime(timeZone);
    tm timeData;
    gmtime_r(&nowSecs, &timeData);
    std::array<char, 16> LastUpdated;
    strftime(LastUpdated.data(), LastUpdated.size(), "%d %b %I:%M %p", &timeData);

    uint16_t txtW, txtH;
    std::tie(txtW, txtH) = getTextDimensions(LastUpdated.data());
    mDisplay.setCursor(x, y + txtH);
    mDisplay.println("last updated:");
    static constexpr size_t textMarginY = 10;
    mDisplay.setCursor(x, y + (txtH * 2) + textMarginY);
    log_d("mLastUpdated: %s", LastUpdated.data());
    mDisplay.println(LastUpdated.data());
}


std::pair<uint16_t, uint16_t> Renderer::getTextDimensions(const std::string& text) const
{
    int16_t  txtEndX, txtEndY;
    uint16_t txtW, txtH;
    mDisplay.getTextBounds(text.c_str(), 0u, 0u, &txtEndX, &txtEndY, &txtW, &txtH);
    return std::make_pair(txtW, txtH);
}
