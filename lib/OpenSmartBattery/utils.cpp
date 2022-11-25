#include "utils.hpp"
#include "config.hpp"
#include <stdint.h>
#include <string.h>

#include <SoftwareSerial.h>
#include <Print.h>

namespace OpenSmartBattery {
    namespace Utils {

        #ifdef DEBUG
        SoftwareSerial Serial = SoftwareSerial(PB0, PB1, false);

        void logCommand(char *message, uint8_t command)
        {
            Utils::Serial.print(message);
            if (command < 16) Serial.print('0');
            Utils::Serial.println(command, 16);
        }
        #endif

        // Split a 16-bit number into two byte chunks
        void splitNum(uint16_t num, uint8_t* higher, uint8_t* lower)
        {
            *higher = num >> 8;
            *lower = num & 0xff;
        }

        // Split the last 16 bits of a 32-bit signed int into two bytes. Does nothing if int is negative.
        void splitNum(int32_t num, uint8_t* higher, uint8_t* lower)
        {
            // Safely handle negative ints
            if (num < 0) {
                return;
            }

            *higher = num >> 8;
            *lower  = num & 0xff;
        }

        /**
         * This is a normal implementation of CRC-8 as a reference for the reader.
         * It is split into two functions below so that bytes can be added individually before finalizing.
        **
        uint8_t wiki_crc8(uint8_t* data, uint8_t len) {
            // AVR is little-endian by default
            const uint8_t generatorPolynomial = 0b111000001;  // x^8 + x^2 + x + 1

            uint8_t remainder = 0;

            // Byte order is big-endian
            for (uint8_t byteFromLeft = 0; byteFromLeft < len; ++byteFromLeft) {
                remainder ^= data[byteFromLeft];

                // Bit order is little-endian (AVR's default)
                for (uint8_t bit = 0; bit < 8; ++bit) {
                    if (remainder & 0x0001) {
                        remainder = (remainder >> 1) ^ generatorPolynomial;
                    } else {
                        remainder >>= 1;
                    }
                }
            }

            return remainder;
        }
        **/

        bool needsLength(uint8_t type)
        {
            return type == 0x20 ||
                   type == 0x21 ||
                   type == 0x22 ||
                   type == 0x23 ||
                   type == 0x30 ||
                   type == 0x3C ||
                   type == 0x37 ||
                   type == 0x2F;
        }

        // The CRC-8 that SMBus uses is x^8 + x^2 + x + 1,
        // which this method runs against a single byte
        static void addByteToCRC(bool CRC[8], uint8_t byteToAdd)
        {
            // CRC = [0,0, ...]; byteToAdd = 0x01
            for (uint8_t y = 1; y <= 8; ++y) {
                // Little-endian means we need to right shift to get the current bit
                // (0x01 >> 7) & 1 = 128 = 1 = 0b00000001
                bool currentBit = (byteToAdd >> (8-y)) & 0x01;

                // 128 ^ 0 = 128 = 1
                // XOR current bit with the right-most bit (MSB)
                bool invert = currentBit ^ CRC[7];

                // 1, (0^1 = 1), (0^1 = 1), 0, 0, 0, 0, 0 = 0b11100000
                CRC[7] = CRC[6];
                CRC[6] = CRC[5];
                CRC[5] = CRC[4];
                CRC[4] = CRC[3];
                CRC[3] = CRC[2];
                CRC[2] = CRC[1] ^ invert;
                CRC[1] = CRC[0] ^ invert;
                CRC[0] = invert;
            }
        }

        // CRC function, used as a checksum for the data sent to the laptop
        uint8_t calculateCRC(uint8_t* dataArray, uint8_t dataArrayLength, uint8_t command)
        {
            bool CRC[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            bool addLength = needsLength(command);

            // lukepfjo: No idea what 0x22 and 0x23 are for, but they seem important
            addByteToCRC(CRC, 22);
            addByteToCRC(CRC, command);
            addByteToCRC(CRC, 23);

            // Some commands need the length added to the output data, others don't
            if (addLength) {
                addByteToCRC(CRC, dataArrayLength);
            }

            // Byte order is big-endian per SMBus spec
            for (uint8_t x = 0; x < dataArrayLength; ++x) {
                addByteToCRC(CRC, dataArray[x]);
            }

            // What we ultimately want is the remainder, which this calculates
            uint8_t remainder = 0;
            for (uint8_t x = 0; x < 8; ++x) {
                remainder += (1 << x) * CRC[x];
            }

            return remainder;
        }

        // ----

        // Flags that comprise the 0x03 BatteryMode() output
        BatteryMode::BatteryMode()
        {
            internalChargeController = BatteryConfig::HAS_INTERNAL_CHARGE_CONTROLLER;  // Pack has a charger than controls voltage and current (not just a protection circuit!)
            primaryBatterySupport    = BatteryConfig::HAS_MULTI_BATTERY_SUPPORT;       // Pack has internal switch for multiple batteries

            // Bits 2-6 are reserved for future use by the SBS standards committee

            conditionFlag            = BatteryConfig::REQUEST_CONDITIONING_CYCLE;  // R/O | Battery requests conditioning cycle
            chargeControllerEnabled  = false;  // R/W | If INTERNAL_CHARGE_CONTROLLER is set, enable/disable internal charge controller
            primaryBattery           = false;  // R/W | If PRIMARY_BATTERY_SUPPORT is set, select this battery pack as the sole battery in use

            // Bits 10-12 are reserved for future use by the SBS standards committee

            alarmMode    = false;  // R/W | System is responsible for detecting and responding to alarm events. This is cleared automatically every <=45s by this firmware.
            chargerMode  = false;  // R/W | System is responding for polling ChargingCurrent() and ChargingVoltage() (auto broadcasting every 5-60s is disabled)
            capacityMode = false;  // R/W | Report in mW/10 instead of mA
        }

        void BatteryMode::asSplitBytes(uint8_t *higher, uint8_t *lower)
        {
            splitNum((uint16_t)(
                internalChargeController << 0 |
                primaryBatterySupport << 1 |
                conditionFlag << 7 |
                chargeControllerEnabled << 8 |
                primaryBattery << 9 |
                alarmMode << 13 |
                chargerMode << 14 |
                capacityMode << 15),
                higher, lower
            );
        }

        // ----

        // Flags that comprise 0x16 BatteryStatus()
        BatteryStatus::BatteryStatus()
        {
            errorCode = AlarmErrorCode::Ok;  // Current error code. Consolodates bits 0-3.

            // Status bits

            // Set: Battery is empty
            // Action: Stop discharging
            // Cleared: RelativeStateOfCharge() > 20%
            fullyDischarged = false;

            // Set: Battery is full
            // Action: Stop charging
            // Cleared: Battery is not longer full (this does not request charging on its own)
            fullyCharged = true;

            // Set: Battery is discharging. Note that this can be self-discharge, so it doesn't always mean
            //      the battery is delivering power to the system.
            // Action: N/A
            // Cleared: Battery is being charged
            discharging = false;

            // Set: Our capacity measurements are known to be accurate and can be trusted
            // Action: N/A
            // Cleared: Capacity measurements are seriously faulty (cannot be trusted)
            initialized = true;

            // Alarm bits

            // Set: AverageTimeToEmpty() < RemainingTimeAlarm()
            // Action: N/A
            // Cleared: AverageTimeToEmpty() > RemainingTimeAlarm() or RemainingTimeAlarm() == 0
            remainingTimeAlarm = false;

            // Set: RemainingCapacity() < RemainingCapacityAlarm()
            // Action: N/A
            // Cleared: RemainingCapacity() > RemainingCapacityAlarm() or RemainingCapacityAlarm() == 0
            remainingCapacityAlarm = false;

            // Bit 10 is reserved

            // Set: Battery is empty
            // Action: Stop discharge as soon as possible
            // Cleared: Discharge is no longer detected
            terminateDischargeAlarm = false;

            // Set: Temperature exceeded limit
            // Action: Stop charging
            // Cleared: Temperature has returned to acceptable level
            overTempAlarm = false;

            // Bit 13 is reserved

            // Set: Charging needs to be stopped temporarily
            // Action: Stop charging
            // Cleared: Charging is no longer detected and condition causing alarm has been resolved
            terminateChargeAlarm = false;

            // Set: Battery is fully charged and charging is complete.
            // Action: Stop charging
            // Cleared: Charging is no longer detected and remaining charge has dropped below full charge
            overchargedAlarm = false;
        }

        void BatteryStatus::asSplitBytes(uint8_t *higher, uint8_t *lower)
        {
            splitNum((uint16_t)(
                (((errorCode >> 0) & 1) << 0) |
                (((errorCode >> 1) & 1) << 1) |
                (((errorCode >> 2) & 1) << 2) |
                (((errorCode >> 3) & 1) << 3) |
                (fullyDischarged << 4) |
                (fullyCharged << 5) |
                (discharging << 6) |
                (initialized << 7) |
                (remainingTimeAlarm << 8) |
                (remainingCapacityAlarm << 9) |
                (terminateDischargeAlarm << 11) |
                (overTempAlarm << 12) |
                (terminateChargeAlarm << 14) |
                (overchargedAlarm << 15)),
                higher, lower
            );
        }
    }
}
