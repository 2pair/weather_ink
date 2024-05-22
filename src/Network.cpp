#include "Network.h"

#include <Inkplate.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "../local_env.h"


using namespace network;

Network::Network(const std::string& ssid, const std::string& pass, const std::string& apiKey, const int8_t timeZone)
    :   mApiKey(apiKey),
        mTimeZone(timeZone)
{
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(ssid.c_str(), pass.c_str());
    WiFi.setAutoReconnect(true);
    waitForConnection(60);
    if (isConnected())
    {
        setTimeNTP();
    }
}

Network::~Network()
{
    WiFi.disconnect(false, false);
    WiFi.setSleep(true);
}

bool Network::isConnected()
{
    return WiFi.isConnected();
}

void Network::waitForConnection(const uint8_t waitTimeSec)
{
    Serial.print(F("Waiting for WIFI connection..."));
    for (uint8_t seconds = 0; seconds < waitTimeSec; seconds++) {
        if (isConnected())
        {
            Serial.println("success");
            return;
        }
        delay(1000);
    }
    Serial.println(F("failure"));
}

bool Network::apiGetResponse(JsonDocument& apiResponse, const std::string& url)
{
    if (!isConnected())
    {
        waitForConnection(15);
    }

    HTTPClient http;
    http.getStream().setNoDelay(true);
    http.getStream().setTimeout(1);
    http.begin(url.c_str());

    int httpCode = http.GET();
    bool validData = false;
    if (httpCode == 200)
    {
        int32_t len = http.getSize();
        if (len > 0)
        {
            DeserializationError error = deserializeJson(apiResponse, http.getStream());
            if (error)
            {
                Serial.print(F("deserializeJson() failed for url: "));
                Serial.print(url.c_str());
                Serial.printf((char *)F(" error: %s\n"), error.c_str());
                validData = false;
            }
            else {
                validData = true;
            }
        }
    }

    http.end();
    return validData;
}

// Gets time from ntp server
void Network::getTime(char *timeStr)
{
    // Get seconds since 1.1.1970.
    time_t nowSecs = time(nullptr);

    // Used to store time
    struct tm timeinfo;
    gmtime_r(&nowSecs, &timeinfo);

    // Copies time string into timeStr
    strncpy(timeStr, asctime(&timeinfo) + 11, 5);

    // Setting time string timezone
    int hr = 10 * timeStr[0] + timeStr[1] + mTimeZone;

    // Better defined modulo, in case timezone makes hours to go below 0
    hr = (hr % 24 + 24) % 24;

    // Adding time to '0' char makes it into whatever time char, for both digits
    timeStr[0] = hr / 10 + '0';
    timeStr[1] = hr % 10 + '0';
}

void Network::setTimeNTP()
{
    // Used for setting correct time
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    Serial.print(F("Waiting for NTP time sync: "));
    time_t nowSecs;
    do {
        // Wait for time to be set
        delay(1000);
        nowSecs = time(nullptr);
        Serial.printf("%d.. ", nowSecs);
    } while (nowSecs < 8 * 3600 * 2);

    Serial.println();

    // Used to store time info
    struct tm timeinfo;
    gmtime_r(&nowSecs, &timeinfo);

    Serial.print(F("Current time: "));
    Serial.print(asctime(&timeinfo));
}

// void Network::getDays(char *day, char *day1, char *day2, char *day3)
// {
//     // Seconds since 1.1.1970.
//     time_t nowSecs = time(nullptr);

//     // Find weekday

//     // We get seconds since 1970, add 3600 (1 hour) times the time zone and add 3 to
//     // make monday the first day of the week, as 1.1.1970. was a thursday
//     // finally do mod 7 to insure our day is within [0, 6]
//     int dayWeek = ((long)((nowSecs + 3600L * mTimeZone) / 86400L) + 3) % 7;

//     // Copy day data to globals in main file
//     strncpy(day, wDays[dayWeek], 3);
//     strncpy(day1, wDays[(dayWeek + 1) % 7], 3);
//     strncpy(day2, wDays[(dayWeek + 2) % 7], 3);
//     strncpy(day3, wDays[(dayWeek + 3) % 7], 3);
// }
