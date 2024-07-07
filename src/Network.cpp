#include "Network.h"

#include <esp32-hal-log.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <esp_sntp.h>

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
    log_i("Waiting for WIFI connection");
    for (uint8_t seconds = 0; seconds < waitTimeSec; seconds++) {
        if (isConnected())
        {
            log_i(" success");
            return;
        }
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
            log_d(" status: %d ", WiFi.status());
        }
        delay(1000);
    }
    log_w(" failure");
}

bool Network::getApiResponse(JsonDocument& apiResponse, const std::string& url)
{
    if (!isConnected())
    {
        waitForConnection(15);
    }
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.getStream().setTimeout(10);
    http.begin(url.c_str());

    int httpCode = http.GET();
    bool validData = false;
    if (httpCode == 200)
    {
        auto response = http.getString();
        log_d("Begin deserialize API response:");
        // streaming the HTTP response into the deserializer causes a chunk out of order
        // bug for chunked responses, like WeatherApi returns.
        DeserializationError error = deserializeJson(apiResponse, response);
        if (error)
        {
            log_w(
                "deserializeJson() failed for url: %s error: %s",
                url.c_str(),
                error.c_str()
            );
            validData = false;
        }
        else {
            log_i("Data has been deserialized");
            log_d("size %d", apiResponse.size());
            validData = true;
        }
    }

    http.end();
    return validData;
}

void Network::setTimeNTP()
{
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    log_i("Waiting for NTP time sync: ");
    time_t nowSecs;
    do {
        // Wait for time to be set
        delay(1000);
        nowSecs = time(nullptr);
        log_v("%ld.. ", nowSecs);
    } while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED);

    tm timeInfo;
    nowSecs = time(nullptr);
    gmtime_r(&nowSecs, &timeInfo);
    log_i("Current time UTC: %s", asctime(&timeInfo));
    if (esp_sntp_enabled())
    {
        log_d("Disabling NTP");
        esp_sntp_stop();
    }
}