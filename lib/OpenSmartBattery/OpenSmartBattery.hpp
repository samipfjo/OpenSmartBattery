#ifndef SMART_BATTERY_FIRMWARE_H
#define SMART_BATTERY_FIRMWARE_H

#include "utils.hpp"
#include <stdint.h>

namespace OpenSmartBattery {
    extern Utils::PowerState POWER_STATE;
    extern Utils::BatteryMode BATTERY_MODE;
    extern Utils::BatteryStatus BATTERY_STATUS;

    // ====

    namespace RequestHandlers {
        inline uint8_t handleCommand(uint8_t*);
    }

    void checkValuesAndSetStates();
    void calculateChargeParameters();
    void receiveEvent(int);
    void requestEvent();
}

#endif
