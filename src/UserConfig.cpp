#include "UserConfig.h"

#include <driver/gpio.h>
#include <rom/rtc.h>
#include <Inkplate.h>

#include "../fonts/PatrickHand_Regular26pt7b.h"

#include "Environment.h"
#include "Renderer.h"


extern uint32_t gInterruptReset;

using namespace userconfig;

void IRAM_ATTR userconfig::buttonAction(void* configClass)
{
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_EXT0)
    {
        // woken from timer, not button.
        return;
    }
    auto userConfig = reinterpret_cast<UserConfig*>(configClass);
    userConfig->mButtonPressed = true;
    log_d("user pressed the button");
}

UserConfig::UserConfig(
    Inkplate& display,
    const gpio_num_t buttonPin,
    const Environment& env
)
    :   mDisplay(display),
        cEnv(env),
        mButtonPin(buttonPin),
        mState(State::None),
        mNextState(State::Initialize),
        mButtonPressed(false),
        mUpdated(false),
        mLocationIndexUpdated(false),
        mLocationIndex(0),
        mUseMetricUpdated(false),
        mUseMetric(false)
{
    enableButtonInterrupt();
}

UserConfig::~UserConfig()
{
    disableButtonInterrupt();
}

void UserConfig::enableButtonInterrupt()
{
    attachInterruptArg(mButtonPin, buttonAction, this, FALLING);
}

void UserConfig::disableButtonInterrupt()
{
    detachInterrupt(mButtonPin);
}

bool UserConfig::configUpdated() const
{
    return mUpdated;
}

 bool UserConfig::locationIndexUpdated() const
 {
    return mLocationIndexUpdated;
 }

size_t UserConfig::getLocationIndex() const
{
    return mLocationIndex;
}

bool UserConfig::useMetricUpdated() const
{
    return mUseMetricUpdated;
}

bool UserConfig::getUseMetric() const
{
    return mUseMetric;
}

void UserConfig::populateLocations()
{
    mLocations = GetLocationsFromFile("/env.json", mDisplay);
    auto itr = std::find(mLocations.cbegin(), mLocations.cend(), std::string(cEnv.city));
    if (itr == mLocations.cend())
    {
        log_w("Current city %s not in json list, defaulting index to 0", cEnv.city);
        std::string locs;
        for (auto& loc : mLocations)
        {
            locs += (loc + ", ");
        }
        log_d("available locations are: %s", locs.c_str());
        mUpdated = true;
        mLocationIndexUpdated = true;
        mLocationIndex = 0;
    }
    else
    {
        mLocationIndex = std::distance(mLocations.cbegin(), itr);
        log_d("Found default city at index %d", mLocationIndex);
    }
    log_d("%d locations available", mLocations.size());
}

void UserConfig::getConfigFromUser()
{
    log_d("Userconfig state machine start");
    while(mState != State::Terminate)
    {
        mState = mNextState;
        switch (mState)
        {
            case State::Initialize:
                stateInitialize();
                break;
            case State::DisplayLocationInstructions:
                stateDisplayLocationInstructions();
                break;
            case State::WaitForLocation:
                stateWaitForLocation();
                break;
            case State::SetLocation:
                stateSetLocation();
                break;
            case State::DisplayUnitInstructions:
                stateDisplayUnitInstructions();
                break;
            case State::WaitForUnit:
                stateWaitForUnit();
                break;
            case State::SetUnit:
                stateSetUnit();
                break;
            case State::DisplayUpdating:
                stateDisplayUpdating();
                break;
            case State::Terminate:
            default:
                stateTerminate();
                break;
        }

        mButtonPressed = false;
    }
}

void UserConfig::stateInitialize()
{
    log_d("User Config state machine in state Initialize");
    auto resetReason = rtc_get_reset_reason(PRO_CPU_NUM);
    auto wakeReason = esp_sleep_get_wakeup_cause();
    log_d("Reset reason is %d, wake reason is %d", resetReason, wakeReason);
    // device is waking due to button press
    if (wakeReason == ESP_SLEEP_WAKEUP_EXT0)
    {
        log_d("Woken from external interrupt");
        mUseMetric = cEnv.metricUnits;
        mNextState = State::DisplayUnitInstructions;
    }
    // Reset via timer from sleep (typical path)
    else if (resetReason == DEEPSLEEP_RESET)
    {
        // Technically this is also the reset reason when we are woken due to the button, but
        // we are already checking for that, so the only reason left is timed wake from sleep
        log_d("Device reset from sleep");
        mNextState = State::Terminate;
    }
    else if (resetReason == TG1WDT_SYS_RESET)
    {
        log_d("Woken by watchdog timer");
        // This seems redundant with the watchdog timer reset reason...
        if (wakeReason == ESP_SLEEP_WAKEUP_TIMER)
        {
            log_w("Woken from timer");
            mNextState = State::Terminate;
        }
        else
        {
            log_w("Unexpected wake reason %d", wakeReason);
            mNextState = State::Terminate;
        }
    }
    // Booting from power off
    else if (resetReason == POWERON_RESET)
    {
        log_d("Booting from power off state");
        populateLocations();
        mNextState = State::DisplayLocationInstructions;
    }
    // booting from button press or watchdog timer expiration
    else if (resetReason == SW_RESET)
    {
        // User pushed button while device was awake
        if (gInterruptReset == cInterruptResetCode)
        {
            log_d("Restarted from external interrupt");
            mUseMetric = cEnv.metricUnits;
            mNextState = State::DisplayUnitInstructions;
        }
        // Watchdog timer expired
        else
        {
            log_w("Restarted by watchdog reset again?!?");
            mNextState = State::Terminate;
        }
    }
    else if (resetReason == SW_CPU_RESET)
    {
        log_w("Device was reset from within software");
        mNextState = State::Terminate;
    }
    else
    {
        log_w("Unexpected restart reason %d", resetReason);
        mNextState = State::Terminate;
    }
}

void UserConfig::stateDisplayLocationInstructions()
{
    log_d("User Config state machine in state Display Location Instructions");
    std::vector<std::string> lines = {
        "Use the button to configure",
        "the current location.",
        "Current location is",
        mLocations.at(mLocationIndex) + "  (" + std::to_string(mLocationIndex + 1) + "/" + std::to_string(mLocations.size()) + ")"
    };
    renderer::Renderer renderer(mDisplay);
    renderer.drawLinesCentered(lines, PatrickHand_Regular26pt7b, 12);
    disableButtonInterrupt();
    renderer.render();
    enableButtonInterrupt();
    mNextState = State::WaitForLocation;
}

void UserConfig::stateWaitForLocation()
{
    log_d("User Config state machine in state Wait For Location");
    esp_sleep_enable_timer_wakeup(cWaitTime);
    esp_sleep_enable_ext0_wakeup(mButtonPin, LOW);
    esp_light_sleep_start();
    if (mButtonPressed)
    {
        mNextState = State::SetLocation;
    }
    else
    {
        mNextState = State::DisplayUpdating;
    }
}

void UserConfig::stateSetLocation()
{
    log_d("User Config state machine in state Set Location");
    mLocationIndex++;
    if (mLocationIndex >= mLocations.size())
    {
        mLocationIndex = 0;
    }
    log_i("Location set to %s", mLocations.at(mLocationIndex).c_str());
    mUpdated = true;
    mLocationIndexUpdated = true;
    mNextState = State::DisplayLocationInstructions;
}

void UserConfig::stateDisplayUnitInstructions()
{
    log_d("User Config state machine in state Display Unit Instructions");
    std::vector<std::string> lines = {
        "Use the button to configure",
        "the units. Current units are",
        (mUseMetric ? "metric." : "imperial."),
        "To select a new location turn",
        "the device off",
        "and then on again."
    };
    renderer::Renderer renderer(mDisplay);
    renderer.drawLinesCentered(lines, PatrickHand_Regular26pt7b, 12);
    disableButtonInterrupt();
    renderer.render();
    enableButtonInterrupt();
    mNextState = State::WaitForUnit;
}

void UserConfig::stateWaitForUnit()
{
    log_d("User Config state machine in state Wait For Unit");
    // Sleep 0.500 seconds
    esp_sleep_enable_timer_wakeup(cWaitTime);
    esp_sleep_enable_ext0_wakeup(mButtonPin, LOW);
    esp_light_sleep_start();
    if (mButtonPressed)
    {
        mNextState = State::SetUnit;
    }
    else
    {
        mNextState = State::DisplayUpdating;
    }
}

void UserConfig::stateSetUnit()
{
    log_d("User Config state machine in state Set Unit");
    mUseMetric = !mUseMetric;
    log_i("Units set to %s", mUseMetric ? "metric" : "imperial");
    mUpdated = true;
    mUseMetricUpdated = true;
    mNextState = State::DisplayUnitInstructions;
}

void UserConfig::stateDisplayUpdating()
{
     log_d("User Config state machine in state Display Updating");
    std::vector<std::string> lines = {
        "Now updating the forecast,",
        "please wait."
    };
    renderer::Renderer renderer(mDisplay);
    renderer.drawLinesCentered(lines, PatrickHand_Regular26pt7b, 12);
    disableButtonInterrupt();
    renderer.render();
    enableButtonInterrupt();
    mNextState = State::Terminate;
}

void UserConfig::stateTerminate()
{
    log_d("User Config state machine in state Terminate");
    mNextState = State::Terminate;
}
