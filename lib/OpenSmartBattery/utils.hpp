#ifndef SMART_BATTERY_FIRMWARE_UTILS_H
#define SMART_BATTERY_FIRMWARE_UTILS_H

#include "config.hpp"
#include <stdint.h>
#include <SoftwareSerial.h>

namespace OpenSmartBattery {
    namespace Utils {

        #ifdef DEBUG
            extern SoftwareSerial Serial;
            extern void logCommand(char *message, uint8_t command);
        #endif

        // Calculated values from config
        const uint16_t V_HIGH = BatteryConfig::MAX_CELL_VOLTAGE * BatteryConfig::CELLS_IN_SERIES;  // in mV
        const uint16_t V_LOW  = BatteryConfig::MIN_CELL_VOLTAGE * BatteryConfig::CELLS_IN_SERIES;  // in mV

        const uint16_t MAX_DISCHARGE_RATE      = BatteryConfig::CELL_CAPACITY;  // mAh: Maximum current one cell can provide (1C)
        const uint16_t BATTERY_CAPACITY_DESIGN = BatteryConfig::CELL_CAPACITY * BatteryConfig::CELLS_IN_PARALLEL;   // mA: Total capacity of pack
        const uint16_t BATTERY_CAPACITY        = BATTERY_CAPACITY_DESIGN * (BatteryConfig::CELL_WEAR / 100.0);      // mA: Multiplied by BATTERY_VOLTAGE when calculating Wh.

        extern void splitNum(uint16_t num, uint8_t* higher, uint8_t* lower);
        extern void splitNum(int32_t num, uint8_t* higher, uint8_t* lower);
        extern bool needsLength(uint8_t type);

        extern uint8_t calculateCRC(uint8_t* dataArray, uint8_t dataArrayLength, uint8_t command);
        static void addByteToCRC(bool CRC[8], uint8_t byteToAdd);

        // ----
        enum PowerState: uint8_t {
            charging    = 0,
            discharging = 1,
            idling      = 2
        };

        enum AlarmErrorCode: uint8_t {
            Ok                  = 0b0000,
            Busy                = 0b0001,
            ReservedCommand     = 0b0010,
            UnsupportedCommand  = 0b0011,
            AccessDenied        = 0b0100,
            OverflowUnderflow   = 0b0101,
            BadSize             = 0b0110,
            UnknownError        = 0b0111
        };

        class BatteryMode {
            public:
                bool internalChargeController;
                bool primaryBatterySupport;
                bool conditionFlag;
                bool chargeControllerEnabled;
                bool primaryBattery;
                bool alarmMode;
                bool chargerMode;
                bool capacityMode;
            
                BatteryMode();
                void asSplitBytes(uint8_t*, uint8_t*);
        };

        class BatteryStatus {
            public:
                AlarmErrorCode errorCode;
                bool fullyDischarged;
                bool fullyCharged;
                bool discharging;
                bool initialized;
                bool remainingTimeAlarm;
                bool remainingCapacityAlarm;
                bool terminateDischargeAlarm;
                bool overTempAlarm;
                bool terminateChargeAlarm;
                bool overchargedAlarm;

                BatteryStatus();
                void asSplitBytes(uint8_t*, uint8_t*);

                inline bool canCharge() {
                    return !(fullyCharged || overTempAlarm || terminateChargeAlarm || overchargedAlarm);
                }

                inline bool canDischarge() {
                    return !(fullyDischarged || overTempAlarm);  // terminateDischargeAlarm says "as soon as possible" not "immediately"
                }
        };
    }
}

#endif
