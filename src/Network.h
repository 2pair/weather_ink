#pragma once

#include <stdint.h>

#include <Inkplate.h>
#include <ArduinoJson.h>


namespace network {

class Network
{
  public:
    Network(const std::string& ssid, const std::string& pass);

    ~Network();

    void shutDown();

    bool isConnected();

    bool getApiResponse(JsonDocument& apiResponse, const std::string& url);

  private:
    void waitForConnection(const uint8_t waitTimeSec);

    void setTimeNTP();
};

}
