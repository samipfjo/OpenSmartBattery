#ifndef SMART_BATTERY_FIRMWARE_CONFIG_H
#define SMART_BATTERY_FIRMWARE_CONFIG_H

#include <stdint.h>
#include <Arduino.h>

namespace OpenSmartBattery {
    namespace BatteryConfig {

        const char* const BATTERY_MODEL  = "AS10D51";  // First 4 chars are cut off?
        const char* const BATTERY_VENDOR = "Panasonic";
        const uint8_t SERIAL_CODE = 64;

        const uint16_t CHARGE_VOLTAGE  = 12600;  // mV: Voltage at which the pack should be charged
        const uint16_t BATTERY_VOLTAGE = 10800;  // mV: Nominal voltage of battery pack

        const uint16_t MAX_CELL_VOLTAGE = 4200;  // mV: Li-ion cells have a max voltage of ~4200mV.

        // Li-ion cells have massive voltage drop below 3400-3500mV
        // Lower MIN_CELL_VOLTAGE at your own risk according to your cells' datasheet
        const uint16_t MIN_CELL_VOLTAGE = 3500;  // in mV

        // Arrangement of cells in pack
        const uint8_t CELLS_IN_SERIES   = 3;  // Number of cells in series within the pack. Defines the voltage. For example, 3 * 3700 = 11.1v
        const uint8_t CELLS_IN_PARALLEL = 3;  // Number of cells in parallel within the pack. Defines the capacity. For example, 3 * 1480mA = 4440mA

        const uint16_t CELL_CAPACITY = 3200 - 50;  // mAh: Cell capacity - tolerance
        const uint8_t  CELL_WEAR     = 98;         // Percentage of cell life left (whole number)

        const bool HAS_INTERNAL_CHARGE_CONTROLLER = true;   // Pack has a charger that controls voltage and current (normally true)
        const bool HAS_MULTI_BATTERY_SUPPORT      = false;  // Pack has internal switch for multiple batteries (normally false)
        const bool REQUEST_CONDITIONING_CYCLE     = false;  // Cells are new and need to be conditioned (normally false)
    }

    namespace HardwareConfig {
        namespace Pins {
            const uint8_t SERIAL_IN          = PB0;  // External -> device
            const uint8_t SERIAL_OUT         = PB1;  // Device   -> external

            const uint8_t OUTPUT_TRANSISTOR  = PA6;
            const uint8_t CHARGE_TRANSISTOR  = PA7;

            const uint8_t CURRENT_SENSE      = PA0;
            const uint8_t CELL_0_VOLTAGE     = PA1;
            const uint8_t CELL_1_VOLTAGE     = PA2;
            const uint8_t CELL_2_VOLTAGE     = PA3;
            const uint8_t PACK_VOLTAGE       = PA4;
            const uint8_t PACK_TEMP_SENSE    = PA5;
        }
    }
}

#endif
