#include "Network.h"

#include <Inkplate.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "../local_env.h"


using namespace network;

Network::Network(const std::string& ssid, const std::string& pass, const int8_t timeZone)
    :   mTimeZone(timeZone)
{
    WiFi.useStaticBuffers(true);
    WiFi.persistent(true);
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
    shutDown();
}

void Network::shutDown() {
    WiFi.disconnect(false, false);
    WiFi.setSleep(true);
}

bool Network::isConnected()
{
    return WiFi.isConnected();
}

void Network::waitForConnection(const uint8_t waitTimeSec)
{
    Serial.print(F("Waiting for WIFI connection"));
    for (uint8_t seconds = 0; seconds < waitTimeSec; seconds++) {
        if (isConnected())
        {
            Serial.println(F(" success"));
            return;
        }
        Serial.print(F("."));
        if (seconds % 10 == 0)
        {
            /* Status codes copied from wl_status_t in WifiType.h
                WL_IDLE_STATUS      = 0,
                WL_NO_SSID_AVAIL    = 1,
                WL_SCAN_COMPLETED   = 2,
                WL_CONNECTED        = 3,
                WL_CONNECT_FAILED   = 4,
                WL_CONNECTION_LOST  = 5,
                WL_DISCONNECTED     = 6
            */
            Serial.printf((const char*)F(" status: %d "), WiFi.status());
        }
        delay(1000);
    }
    Serial.println(F(" failure"));
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

void Network::setTimeNTP()
{
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

    struct tm timeinfo;
    gmtime_r(&nowSecs, &timeinfo);

    Serial.print(F("Current time: "));
    Serial.print(asctime(&timeinfo));
}