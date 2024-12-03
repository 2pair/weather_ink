#include "UserConfig.h"

#include <driver/gpio.h>
#include <Inkplate.h>

#include "../fonts/PatrickHand_Regular26pt7b.h"

#include "Environment.h"
#include "Renderer.h"

using namespace userconfig;

void userconfig::buttonAction(void* configClass)
{
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
        mUpdated(false),
        mButtonPressed(false),
        mLocationIndex(0),
        mUseMetric(false),
        mState(State::None),
        mNextState(State::Initialize)

{
    gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
    auto fptr = &buttonAction;
    gpio_isr_handler_add(buttonPin, fptr, this);
}

UserConfig::~UserConfig()
{
    log_d("destroy userconfig");
    gpio_uninstall_isr_service();
}

bool UserConfig::configUpdated() const
{
    return mUpdated;
}

size_t UserConfig::getLocationIndex() const
{
    return mLocationIndex;
}

bool UserConfig::getUseMetric() const
{
    return mUseMetric;
}

void UserConfig::getConfigFromUser()
{
    log_d("userconfig state machine start");
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
    auto wakeReason = esp_sleep_get_wakeup_cause();
    // typical wake cycle
    if (wakeReason == ESP_SLEEP_WAKEUP_TIMER)
    {
        log_d("woken from timer");
        mNextState = State::Terminate;
    }
    // undefined means the device is booting from an off state instead of a sleep state
    else if (wakeReason == ESP_SLEEP_WAKEUP_UNDEFINED)
    {
        log_d("woken by undefined, assuming first boot");
        mLocations = GetLocationsFromFile("/env.json", mDisplay);
        auto itr = std::find(mLocations.cbegin(), mLocations.cend(), cEnv.city);
        if (itr == mLocations.cend())
        {
            log_w("default city not in json list, defaulting index to 0");
            mLocationIndex = 0;
        }
        else
        {
            mLocationIndex = std::distance(mLocations.cbegin(), itr);
            log_d("found default city at index %d", mLocationIndex);
        }

        mNextState = State::DisplayLocationInstructions;
    }
    // device is booting due to button press
    else if (wakeReason == ESP_SLEEP_WAKEUP_EXT0)
    {
        log_d("woken from external interrupt");
        mUseMetric = cEnv.metricUnits;
        mNextState = State::DisplayUnitInstructions;
    }
    else
    {
        log_d("Unexpected wake reason %d", wakeReason);
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
    log_d("creating renderer");
    renderer::Renderer renderer(mDisplay);
    log_d("created renderer, writing lines to frame buffer");
    renderer.drawLinesCentered(lines, PatrickHand_Regular26pt7b, 12);
    log_d("drawing text to screen");
    renderer.render();
    log_d("getting the time");
    mReferenceTime = esp_timer_get_time();
    mNextState = State::WaitForLocation;
}

void UserConfig::stateWaitForLocation()
{
    log_d("User Config state machine in state Wait For Location");
    // Wait 0.25 seconds
    delay(250);
    auto currentTime = esp_timer_get_time();
    if (currentTime >= mReferenceTime + cWaitTime)
    {
        log_d("current time is %d, referenceTime is %d, timeoutTime is %d ", currentTime, mReferenceTime, mReferenceTime + cWaitTime);
        mNextState = State::DisplayUpdating;
    }
    else if (mButtonPressed)
    {
        mNextState = State::SetLocation;
    }
    else
    {
        mNextState = State::WaitForLocation;
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
    mNextState = State::DisplayLocationInstructions;
}

void UserConfig::stateDisplayUnitInstructions()
{
    log_d("User Config state machine in state Display Unit Instructions");
    std::vector<std::string> lines = {
        "Use the button to configure",
        "the units. Current units are",
        (mUseMetric ? "metric." : "imperial.")
    };
    renderer::Renderer renderer(mDisplay);
    renderer.drawLinesCentered(lines, PatrickHand_Regular26pt7b, 12);
    renderer.render();

    mReferenceTime = esp_timer_get_time();
    mNextState = State::WaitForUnit;
}

void UserConfig::stateWaitForUnit()
{
    log_d("User Config state machine in state Wait For Unit");
    // Wait 0.25 seconds
    delay(250);
    if (esp_timer_get_time() >= mReferenceTime + cWaitTime)
    {
        mNextState = State::DisplayUpdating;
    }
    else if (mButtonPressed)
    {
        mNextState = State::SetUnit;
    }
    else
    {
        mNextState = State::WaitForUnit;
    }
}

void UserConfig::stateSetUnit()
{
    log_d("User Config state machine in state Set Unit");
    mUseMetric = !mUseMetric;
    log_i("Units set to %s", mUseMetric ? "metric" : "imperial");
    mUpdated = true;
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
    renderer.render();
    mNextState = State::Terminate;
}

void UserConfig::stateTerminate()
{
    log_d("User Config state machine in state Terminate");
    mNextState = State::Terminate;
}
