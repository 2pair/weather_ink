#pragma once

#include <stdint.h>

#include <Inkplate.h>
#include <ArduinoJson.h>


namespace network {

class Network
{
  public:
    Network(const char *ssid, const char *pass, const char* apiKey, const int8_t timeZone);

    ~Network();

    bool isConnected();

    void getTime(char *timeStr);

    bool apiGetResponse(JsonDocument& apiResponse, const char * url);

  private:
    void waitForConnection(const uint8_t waitTimeSec);

    void setTimeNTP();

    const char* mApiKey;
    const int8_t mTimeZone = 0;
};

}
