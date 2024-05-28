#pragma once

// Time zone offset from UTC
static constexpr int8_t timeZone = 0;

// City name to display
static constexpr char city[] = "";

// Coordinates sent to the api
static constexpr float lat = -12.3456;
static constexpr float lon = 78.910;

// Your wifi ssid and password
static constexpr char ssid[] = "";
static constexpr char pass[] = "";

// Your api key:
// https://openweathermap.org/guide , register and copy the key provided
static constexpr char apiKeyOpenWeatherMap[] = "";

// https://www.weatherapi.com , register and copy the key provided
static constexpr char apiKeyWeatherApip[] = "";

// If true, read json files from the SD card instead of querying the API
static constexpr bool cFakeAPIUpdates = true;
