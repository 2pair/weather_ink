
This project uses a Soldered Inkplate 6 as an IoT local weather display. The project's goal was to build a self contained weather display that directly consumed the weather API and rendered the content, without relying on an intermediary server.

The API requires using an API key, this can either be specified in a head file or in a JSON file on the SD card. I'm using the header data as a default and the SD data to override so that the config can be modified later without reprogramming the MCU. The `samples` directory includes a JSON file that can be used to specify the environment on the SD card and API samples for Open Weather Map (owm) and Weather API (wa). All of the content of this directory should be put into the SD card root, along with the `icons` directory. The icons are using good ol DOS8.3 naming conventions because the SD card library specified it, though more verbose names may be allowed and would certainly be preferable.

You might wonder why I implemented more than one weather provider, or why there is so much complexity in that area. The simple answer is that each had limitations that I wasn't aware of when I started to use them. These weren't impacting me until I started adding additional features. For OpenWeatherMap I wasn't able to get moon phase data for free and for WeatherApi I needed four days of forecast but the free tier (after the trial expires) only allows three days of forecast. However that was solvable by just calling the API more, 5x per refresh update. The incomplete OpenWeatherMap code is left as-is, in case it is useful down the road.

I also made one modification to the Inkplate code, which probably needs to be done in the local environment in order for the code to run. Namely, the function signature of `getSdFat()` was changed to `SdFat& System::getSdFat()` so that it would return a reference instead of a copy. I say 'probably' instead of 'definitely' because there might actually be enough memory on the board that it doesn't matter.

Dependencies:
ArduinoJson  (v17.0.4)
StreamUtils (v1.8.0)
InkplateLibrary (v10.0.0)
c17 and c++17

Tested with VS Code and Arduino-CLI on Windows 11 and MacOS 14

Font attribution:
https://fonts.google.com/specimen/Patrick+Hand

Inkplate library:
https://github.com/SolderedElectronics/Inkplate-Arduino-library?tab=readme-ov-file