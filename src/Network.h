#pragma once

#include <stdint.h>

#include <Inkplate.h>
#include <ArduinoJson.h>


namespace network {

class Network
{
  public:
    Network(const std::string& ssid, const std::string& pass, const std::string& apiKey, const int8_t timeZone);

    ~Network();

    bool isConnected();

    void getTime(char *timeStr);

    bool apiGetResponse(JsonDocument& apiResponse, const std::string& url);

  private:
    void waitForConnection(const uint8_t waitTimeSec);

    void setTimeNTP();

    const std::string mApiKey;
    const int8_t mTimeZone = 0;
};

}
