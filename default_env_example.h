#pragma once

// Rename this file to default_env.h and populate with your data

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
static constexpr char apiKeyWeatherApi[] = "";

// If true, read json files from the SD card instead of querying the API
static constexpr bool cFakeApiUpdates = true;
