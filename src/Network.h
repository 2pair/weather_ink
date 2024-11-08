#pragma once

#include <stdint.h>

#include <WiFiGeneric.h>
#include <ArduinoJson.h>


namespace network {

class Network
{
  public:
    Network(const std::string& ssid, const std::string& pass);

    ~Network();

    void shutDown();

    static bool isConnected();

    bool getApiResponse(JsonDocument& apiResponse, const std::string& url);

  private:
    wifi_event_id_t mEventId;

    bool waitForConnection(const uint8_t waitTimeSec);

    void setTimeNTP();

    static void handleWiFiEvent(arduino_event_id_t event, arduino_event_info_t eventInfo);

};

}
