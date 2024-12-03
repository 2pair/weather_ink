#pragma once

#include <vector>
#include <string>

#include <driver/gpio.h>

#include "TimeUtils.h"

class Inkplate;
class Environment;

namespace userconfig {

/* Implements a state machine to get user configuration options. The machine has two
 * state loops and selects one or the other based on how the device was started. If the
 * device is powered on from a cold boot the state machine will enter the location
 * configuration loop. If the device is woken from a sleep state by a button press it
 * will enter the unit type configuration loop. If woken by the timer (typical behavior)
 * or any other reason the state machine exits immediately.
 */
class UserConfig
{
    friend void buttonAction(void*);
    public:
        // buttonPin: The ESP32 GPIO pin that the button is connected to
        // env: current environment
        UserConfig(
            Inkplate& display,
            const gpio_num_t buttonPin,
            const Environment& env
        );
        ~UserConfig();

        // Runs the user config state machine
        void getConfigFromUser();

        bool configUpdated() const;
        size_t getLocationIndex() const;
        bool getUseMetric() const;

    private:
        void stateInitialize();
        void stateDisplayLocationInstructions();
        void stateWaitForLocation();
        void stateSetLocation();
        void stateDisplayUnitInstructions();
        void stateWaitForUnit();
        void stateSetUnit();
        void stateDisplayUpdating();
        void stateTerminate();

        enum class State {
            // begin
            None,
            Initialize,
            // set location loop
            DisplayLocationInstructions,
            WaitForLocation,
            SetLocation,
            // set units loop
            DisplayUnitInstructions,
            WaitForUnit,
            SetUnit,
            // exit
            DisplayUpdating,
            Terminate
        };

        Inkplate& mDisplay;
        const Environment& cEnv;
        std::vector<std::string> mLocations;

        /* variables to control state machine */
        State mState;
        State mNextState;
        bool mButtonPressed; // If the user pressed the button
        int64_t mReferenceTime; // Used for calculating wait times

        /* variables to store results */
        bool mUpdated;
        size_t mLocationIndex;
        bool mUseMetric; // mMetric or imperial units

        static constexpr size_t cWaitTime = 10 * cMicrosecondPerSecond;
};

// callback when user presses the 'wake' button on the device.
// argument is an instance of the UserConfig class.
void buttonAction(void* configClass);

}