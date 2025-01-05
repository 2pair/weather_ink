#pragma once

// Rename this file to default_env.h and populate with your data

// City name to display
static constexpr char cCity[] = "";

// Coordinates sent to the api
static constexpr float cLatitude = -12.3456;
static constexpr float cLongitude = 78.910;

// Your wifi ssid and password
static constexpr char cSsid[] = "";
static constexpr char cPass[] = "";

// your chosen weather provider
static constexpr char cProvider[] = "WeatherApi";

// Your api key
// https://www.weatherapi.com , register and copy the key provided OR
// https://openweathermap.org/guide , register and copy the key provided
static constexpr char cApiKey[] = "";

// If true, read json files from the SD card instead of querying the API
static constexpr bool cFakeApiUpdates = true;

// If true use metric units. If false use Imperial units
static constexpr bool cMetricUnits = false;