c and c++ 17

The samples directory includes a json file that can be used to specify the environment on the SD card and API samples for Open Weather Map (owm) and Weather API (wa).

Two modifications were done to the Inkplate code, first theFunction signature of getSd() was changed to `SdFat& System::getSdFat()` so that it would return a reference instead of a copy. Second, the init function was modified due to illues with the board thinking init had already been called.

https://github.com/SolderedElectronics/Inkplate-Arduino-library?tab=readme-ov-file

https://fonts.google.com/specimen/Patrick+Hand