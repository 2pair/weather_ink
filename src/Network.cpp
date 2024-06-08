// Disabling HTTP 1.1 to avoid getting chunked data
#undef HTTPCLIENT_1_1_COMPATIBLE
#define ARDUINOJSON_ENABLE_ARDUINO_STREAM 1

#include "Network.h"

#include <Inkplate.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "TimeUtils.h"


using namespace network;

Network::Network(const std::string& ssid, const std::string& pass)
{
    WiFi.mode(WIFI_MODE_NULL);
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
    WiFi.mode(WIFI_MODE_NULL);
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

bool Network::getApiResponse(JsonDocument& apiResponse, const std::string& url)
{
    if (!isConnected())
    {
        waitForConnection(15);
    }
    HTTPClient http;
    //http.getStream().setNoDelay(true);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.getStream().setTimeout(10);
    //http.addHeader("Accept", "application/json");
    //http.addHeader("Accept-Encoding", "Identity");
    http.begin(url.c_str());

    int httpCode = http.GET();
    bool validData = false;
    if (httpCode == 200)
    {
        auto response = http.getString();
        Serial.println("Begin deserialize API response:\n");
        //ReadLoggingStream loggingStream(http.getStream(), Serial);
        //DeserializationError error = deserializeJson(apiResponse, loggingStream);

        DeserializationError error = deserializeJson(apiResponse, response);//http.getStream());
        if (error)
        {
            Serial.printf(
                (const char *)F("deserializeJson() failed for url: %s error: %s\n"),
                url.c_str(),
                error.c_str()
            );
            validData = false;
        }
        else {
            Serial.println("Data has been deserialized");
            Serial.printf("size %d\n", apiResponse.size());
            validData = true;
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
        Serial.printf("%ld.. ", nowSecs);
    } while (nowSecs < 8 * 3600 * 2);
    Serial.println();

    tm timeInfo;
    nowSecs = time(nullptr);
    gmtime_r(&nowSecs, &timeInfo);
    Serial.printf((const char *)F("Current time UTC: %s\n"), asctime(&timeInfo));
}