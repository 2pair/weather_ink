#include "Network.h"

#include <esp32-hal-log.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <esp_sntp.h>

#include "TimeUtils.h"


using namespace network;

// Global variables for connection maintenance
bool connectionActive;
uint8_t connectionAttempts;
static constexpr uint8_t cMaxConnectionAttempts = 5;

Network::Network(const std::string& ssid, const std::string& pass)
{
    connectionAttempts = 0;
    WiFi.mode(WIFI_MODE_NULL);
    WiFi.useStaticBuffers(true);
    WiFi.persistent(true);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    mEventId = WiFi.onEvent(handleWiFiEvent);
    auto status = WiFi.begin(ssid.c_str(), pass.c_str());
    connectionActive = true;
    waitForConnection(60);
    if (isConnected())
    {
        setTimeNTP();
    }
}

Network::~Network()
{
    log_d("Network cleanup");
    shutDown();
    WiFi.removeEvent(mEventId);
}

void Network::shutDown() {
    log_i("Shutting down network connection");
    connectionActive = false;
    auto disconnected = WiFi.disconnect(true, true);
}

bool Network::isConnected()
{
    return WiFi.isConnected();
}

bool Network::waitForConnection(const uint8_t waitTimeSec)
{
    log_i("Waiting for WIFI connection");
    for (uint8_t seconds = 0; seconds < waitTimeSec; seconds++) {
        if (isConnected())
        {
            log_i(" success");
            return true;
        }
        if (seconds % 10 == 0)
        {
            log_d(" status: %d ", WiFi.status());
        }
        delay(1000);
    }
    log_w(" failure");
    return false;
}

bool Network::getApiResponse(JsonDocument& apiResponse, const std::string& url)
{
    if (!isConnected())
    {
        bool success = waitForConnection(15);
        if (!success)
        {
            return false;
        }
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

    tm timeData;
    nowSecs = time(nullptr);
    gmtime_r(&nowSecs, &timeData);
    log_i("Current time UTC: %s", asctime(&timeData));
    if (esp_sntp_enabled())
    {
        log_d("Disabling NTP");
        esp_sntp_stop();
    }
}

void Network::handleWiFiEvent(arduino_event_id_t event, arduino_event_info_t eventInfo)
{
    log_d("Wifi event was %d", event);
    // If the connection is going down, potentially on purpose
    bool connectionFailure = false;
    switch(event)
    {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            // Connection is fully established
            connectionAttempts = 0;
            log_v("connection fully established, clearing connection attempts counter");
        case ARDUINO_EVENT_WIFI_READY:
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
        case ARDUINO_EVENT_WIFI_STA_START:
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            connectionFailure = false;
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            connectionFailure = true;
            break;
    }
    log_d("connectionActive is %s", (connectionActive) ? "true" : "false");
    log_d("connectionFailure is %s", (connectionFailure) ? "true" : "false");
    log_d("connection attempts is %d", connectionAttempts + 1);
    if (connectionActive && connectionFailure)
    {
        connectionAttempts++;
        if (connectionAttempts >= cMaxConnectionAttempts)
        {
            log_i("exceeded max connection attempts, restarting.");
            delay(1000); // wait 1 second before restart
            esp_restart();
        }
        log_d("unexpected connection loss, changing to disconnected state");
        WiFi.disconnect(false, false);
        log_d("attempting reconnection");
        auto status = WiFi.begin();
    }
}