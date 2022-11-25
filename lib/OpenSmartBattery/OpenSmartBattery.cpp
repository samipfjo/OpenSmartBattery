#include "OpenSmartBattery.hpp"
#include "authentication.hpp"
#include "config.hpp"
#include "utils.hpp"

#include <string.h>
#include <stdint.h>

#include <Wire.h>

// Comments ending with * are either paraphrases or excerpts from this excellent RichTek article on Li-ion fuel gauging:
// https://www.richtek.com/Design%20Support/Technical%20Document/AN024
// (archived) https://web.archive.org/web/20221007161707/https://www.richtek.com/Design%20Support/Technical%20Document/AN024 

namespace OpenSmartBattery {
    // Note on replies in SMBus: Bit order is MSB -> LSB, but byte order is LSB -> MSB

    volatile uint8_t COMMAND = 0;    // Stores current command
    uint8_t REPLY_BUFFER[20];        // Stores reply to current command; this is really 32 bytes by spec
    uint8_t COMMAND_DATA_BUFFER[20]; // Stores the data portion of the current command; this is really 32 bytes by spec

    Utils::PowerState POWER_STATE       = Utils::PowerState::idling;
    Utils::BatteryMode BATTERY_MODE     = Utils::BatteryMode();
    Utils::BatteryStatus BATTERY_STATUS = Utils::BatteryStatus();

    unsigned long ALARM_MODE_SET_AT = millis();

    // ====

    namespace RequestHandlers {
        /**
         * Each handler name is prefixed by the byte associated with the command
         * When called, each handler writes to the REPLY_BUFFER (passed in as *buff) from LSB->MSB
         * The handler then returns the number of bytes written to the buffer
        **/

        inline uint8_t x00_ManufacturerAccess(uint8_t *buff) {
            buff[0] = 0x00;
            buff[1] = 0x00;

            return 2;
        }

        inline uint8_t x01_BatteryCapacityAlarm(uint8_t *buff) {
            // TODO
            buff[0] = 0x94;
            buff[1] = 0x02;

            return 2;
        }

        inline uint8_t x02_RemainingTimeAlarm(uint8_t *buff) {
            // TODO
            buff[0] = 0x0A;
            buff[1] = 0x00;

            return 2;
        }

        inline uint8_t x03_BatteryMode(uint8_t *buff) {
            uint8_t lower, higher;

            BATTERY_MODE.asSplitBytes(&higher, &lower);
            buff[0] = lower;
            buff[1] = higher;

            return 2;
        }

        inline uint8_t x04_AtRate(uint8_t *buff) {
            // TODO
            buff[0] = 0x00;
            buff[1] = 0x00;

            return 2;
        }

        inline uint8_t x05_AtRateTimeToFull(uint8_t *buff) {
            // TODO
            buff[0] = 0x00;
            buff[1] = 0x00;

            return 2;
        }

        inline uint8_t x06_AtRateTimeToEmpty(uint8_t *buff) {
            // TODO
            buff[0] = 0xff;
            buff[1] = 0xff;

            return 2;
        
            /*buff[0] = 0xf0;  // f0 = 240 = 4h
            buff[1] = 0x00;

            return 2;*/
        }

        inline uint8_t x07_AtRateOK(uint8_t *buff) {
            buff[0] = 0x01;
            buff[1] = 0x00;

            return 2;
        }

        inline uint8_t x08_Temperature(uint8_t *buff) {
            // TODO
            // HardwareConfig::Pins::PACK_TEMP_SENSE

            buff[0] = 137;
            buff[1] = 11;

            return 2;
        }

        inline uint8_t x09_Voltage(uint8_t *buff) {
            // TODO
            // 
            uint8_t lower, higher;

            Utils::splitNum(Utils::V_HIGH, &higher, &lower);
            buff[0] = lower;
            buff[1] = higher;

            return 2;
        }

        inline uint8_t x0a_PresentCurrentChargeOrDraw(uint8_t *buff) {
            // TODO
            // HardwareConfig::Pins::CURRENT_SENSE

            uint8_t lower, higher;

            if (POWER_STATE == Utils::PowerState::charging) {
                Utils::splitNum(BatteryConfig::CELL_CAPACITY, &higher, &lower);

            } else if (POWER_STATE == Utils::PowerState::discharging) {
                Utils::splitNum((int16_t)-BatteryConfig::CELL_CAPACITY, &higher, &lower);

            } else {
                lower = 0, higher = 0;
            }

            buff[0] = lower;
            buff[1] = higher;

            return 2;
        }

        inline uint8_t x0b_AverageCurrent(uint8_t *buff) {
            // TODO
            return x0a_PresentCurrentChargeOrDraw(buff);
        }

        inline uint8_t x0c_MaxError(uint8_t *buff) {
            // TODO
            buff[0] = 0x00;
            buff[1] = 0x00;

            return 2;
        }

        inline uint8_t x0d_RelativeStateOfCharge(uint8_t *buff) {
            // TODO

            // 0x0064 = 100%
            buff[0] = 100;
            buff[1] = 0x00;

            return 2;
        }

        inline uint8_t x0e_AbsoluteStateOfCharge(uint8_t *buff) {
            // TODO

            // 0x0064 = 100%
            buff[0] = 100;
            buff[1] = 0x00;

            return 2;
        }

        inline uint8_t x0f_RemainingCapacity(uint8_t *buff) {
            // TODO

            uint8_t lower, higher;

            Utils::splitNum(Utils::BATTERY_CAPACITY, &higher, &lower);
            buff[0] = lower;
            buff[1] = higher;

            return 2;
        }

        inline uint8_t x10_FullChargeCapacity(uint8_t *buff) {
            uint8_t lower, higher;

            Utils::splitNum(Utils::BATTERY_CAPACITY, &higher, &lower);
            buff[0] = lower;
            buff[1] = higher;

            return 2;
        }

        inline uint8_t x11_RunTimeToEmpty(uint8_t *buff) {
            // TODO

            // 0x00f0 = 240 minutes = 4 hours
            buff[0] = 0xf0;
            buff[1] = 0x00;

            return 2;
        }

        inline uint8_t x12_AverageRuneTimeToEmpty(uint8_t *buff) {
            // TODO

            // 0x00f0 = 240 minutes = 4 hours
            buff[0] = 0xf0;
            buff[1] = 0x00;

            return 2;
        }

        inline uint8_t x13_AverageTimeToFull(uint8_t *buff) {
            // TODO

            // 0x00b4 = 180 minutes = 3 hours
            buff[0] = 0xb4;
            buff[1] = 0x00;

            return 2;
        }

        inline uint8_t x14_ChargingCurrentRequested(uint8_t *buff) {
            // TODO : This should ramp off near full capacity

            if (BATTERY_STATUS.canCharge() && POWER_STATE == Utils::PowerState::charging) {
                uint8_t lower, higher;

                Utils::splitNum(BatteryConfig::CELL_CAPACITY, &higher, &lower);
                buff[0] = lower;
                buff[1] = higher;

            } else {
                buff[0] = 0;
                buff[1] = 0;
            }

            return 2;
        }

        inline uint8_t x15_ChargingVoltageRequested(uint8_t *buff) {
            // We use constant-voltage charging, so this won't change

            if (BATTERY_STATUS.canCharge() && POWER_STATE == Utils::PowerState::charging) {
                uint8_t lower, higher;

                Utils::splitNum(BatteryConfig::CHARGE_VOLTAGE, &higher, &lower);
                buff[0] = lower;
                buff[1] = higher;

            } else {
                buff[0] = 0;
                buff[1] = 0;
            }
            
            return 2;
        }

        inline uint8_t x16_BatteryStatus(uint8_t *buff) {
            uint8_t lower, higher;

            BATTERY_STATUS.asSplitBytes(&higher, &lower);
            buff[0] = lower;
            buff[1] = higher;

            return 2;
        }

        inline uint8_t x17_CycleCount(uint8_t *buff) {
            // TODO

            buff[0] = 0x05;
            buff[1] = 0x00;

            return 2;
        }

        inline uint8_t x18_DesignCapacity(uint8_t *buff) {
            uint8_t lower, higher;

            Utils::splitNum(Utils::BATTERY_CAPACITY_DESIGN, &higher, &lower);
            buff[0] = lower;
            buff[1] = higher;

            return 2;
        }

        inline uint8_t x19_DesignVoltage(uint8_t *buff) {
            uint8_t lower, higher;

            Utils::splitNum(BatteryConfig::BATTERY_VOLTAGE, &higher, &lower);
            buff[0] = lower;
            buff[1] = higher;

            return 2;
        }

        inline uint8_t x1a_SpecificationInfo(uint8_t *buff) {
            // TODO : ?

            buff[0] = 0b00110001;
            buff[1] = 0b00000000;

            return 2;
        }

        inline uint8_t x1b_ManufactureDate(uint8_t *buff) {
            // 0x4b6b = 2017.11.11  | 0x4cb2 = 2018.05.18
            buff[0] = 0x6b;
            buff[1] = 0x4b;

            return 2;
        }

        inline uint8_t x1c_SerialNumber(uint8_t *buff) {
            uint8_t lower, higher;

            Utils::splitNum((uint16_t)BatteryConfig::SERIAL_CODE, &higher, &lower);
            buff[0] = lower;
            buff[1] = higher;

            return 2;
        }

        // 0x1d-0x1f

        inline uint8_t x20_ManufacturerName(uint8_t *buff) {
            size_t vendor_string_length = strlen(BatteryConfig::BATTERY_VENDOR);
            for (unsigned int x = 0; x < vendor_string_length; ++x) {
                buff[x] = BatteryConfig::BATTERY_VENDOR[x];
            }

            /*
            // First 4 bytes may be cut off
            int end = vendor_string_length;
            buff[end + 0] = 0;
            buff[end + 1] = 49;
            buff[end + 2] = 49;
            */

            return vendor_string_length; // + 3;
        }

        inline uint8_t x21_DeviceName(uint8_t *buff) {
            for (unsigned int x = 0; x < strlen(BatteryConfig::BATTERY_MODEL); ++x) {
                buff[x] = BatteryConfig::BATTERY_MODEL[x];
            }

            return strlen(BatteryConfig::BATTERY_MODEL);
        }

        inline uint8_t x22_DeviceChemistry(uint8_t *buff) {
            buff[0] = 76;  // L
            buff[1] = 73;  // I
            buff[2] = 79;  // O
            buff[3] = 78;  // N

            return 4;
        }

        inline uint8_t x23_ManufacturerData(uint8_t *buff) {
            // TODO : Provide a way to customize this

            buff[0] = 0x00;
            buff[1] = 0x00;
            buff[2] = 0x00;
            buff[3] = 0x00;
            buff[4] = 0x00;
            buff[5] = 0x6e;
            buff[6] = 0x00;
            buff[7] = 0xaa;
            buff[8] = 0x00;
            buff[9] = 0x02;
            buff[10] = 0x10;
            buff[11] = 0x00;
            buff[12] = 0x00;
            buff[13] = 0x00;

            return 14;
        }

        // 0x24-0x2e

        inline uint8_t x2f_Authenticate(uint8_t *output_buffer, uint8_t *input_buffer) {
            return Authentication::authenticate(output_buffer, input_buffer);
        }

        // ?
        inline uint8_t x30(uint8_t *buff) {
            buff[0] = 112;
            buff[1] = 156;
            buff[2] = 191;
            buff[3] = 25;
            buff[4] = 132;
            buff[5] = 74;
            buff[6] = 151;
            buff[7] = 0;
            buff[8] = 11;
            buff[9] = 0;

            return 10;
        }

        // 0x31-0x34

        // ?
        inline uint8_t x35(uint8_t *buff) {
            buff[0] = 64;
            buff[1] = 0x00;

            return 2;
        }

        // 0x36

        // ?
        inline uint8_t x37(uint8_t *buff) {
            buff[0] = 4;
            buff[1] = 0;
            buff[2] = 61;
            buff[3] = 94;
            buff[4] = 97;
            buff[5] = 1;
            buff[6] = 64;
            buff[7] = 1;

            return 8;
        }

        // 0x38-0x3a

        // ?
        inline uint8_t x3b(uint8_t *buff) {
            buff[0] = 135;
            buff[1] = 11;

            return 2;
        }

        inline uint8_t x3c_x3f_CellVoltage(uint8_t *buff, uint8_t cell) {
            // TODO :: Do actual measurements

            switch (cell) {
                case 0:
                    // HardwareConfig::Pins::CELL_0_VOLTAGE;
                    break;

                case 1:
                    // HardwareConfig::Pins::CELL_1_VOLTAGE;
                    break;

                case 2:
                    // HardwareConfig::Pins::CELL_2_VOLTAGE;
                    break;

                default: break;
            };

            // TODO :: voltage per cell for each 0x3c-0x3f
            buff[0] = 0x68;
            buff[1] = 0x10;

            return 2;
        }

        inline uint8_t x63_x66_AuthKey(uint8_t *buff) {
            short offset = COMMAND - 0x63;
            buff[0] = Authentication::AUTH_KEY[offset * 4];
            buff[1] = Authentication::AUTH_KEY[offset * 4 + 1];
            buff[2] = Authentication::AUTH_KEY[offset * 4 + 2];
            buff[3] = Authentication::AUTH_KEY[offset * 4 + 3];

            return 4;
        }

        // Map command codes to event handlers. Event handlers write to the global buffer and return amount of bytes written.
        // Commands that are not used or have not been implemented will return 255.
        // See https://www.nxp.com/docs/en/application-note/AN4471.pdf for more information about what each command does
        inline uint8_t handleCommand(uint8_t *buffer) {
            uint8_t cell = 0;

            switch (COMMAND) {
                case 0x00: return x00_ManufacturerAccess(buffer);
                case 0x01: return x01_BatteryCapacityAlarm(buffer);
                case 0x02: return x02_RemainingTimeAlarm(buffer);
                case 0x03: return x03_BatteryMode(buffer);
                case 0x04: return x04_AtRate(buffer);
                case 0x05: return x05_AtRateTimeToFull(buffer);
                case 0x06: return x06_AtRateTimeToEmpty(buffer);
                case 0x07: return x07_AtRateOK(buffer);
                case 0x08: return x08_Temperature(buffer);
                case 0x09: return x09_Voltage(buffer);
                case 0x0a: return x0a_PresentCurrentChargeOrDraw(buffer);
                case 0x0b: return x0b_AverageCurrent(buffer);
                case 0x0c: return x0c_MaxError(buffer);
                case 0x0d: return x0d_RelativeStateOfCharge(buffer);
                case 0x0e: return x0e_AbsoluteStateOfCharge(buffer);
                case 0x0f: return x0f_RemainingCapacity(buffer);
                case 0x10: return x10_FullChargeCapacity(buffer);
                case 0x11: return x11_RunTimeToEmpty(buffer);
                case 0x12: return x12_AverageRuneTimeToEmpty(buffer);
                case 0x13: return x13_AverageTimeToFull(buffer);
                case 0x14: return x14_ChargingCurrentRequested(buffer);
                case 0x15: return x15_ChargingVoltageRequested(buffer);
                case 0x16: return x16_BatteryStatus(buffer);
                case 0x17: return x17_CycleCount(buffer);
                case 0x18: return x18_DesignCapacity(buffer);
                case 0x19: return x19_DesignVoltage(buffer);
                case 0x1a: return x1a_SpecificationInfo(buffer);
                case 0x1b: return x1b_ManufactureDate(buffer);
                case 0x1c: return x1c_SerialNumber(buffer);
                case 0x20: return x20_ManufacturerName(buffer);
                case 0x21: return x21_DeviceName(buffer);
                case 0x22: return x22_DeviceChemistry(buffer);
                case 0x23: return x23_ManufacturerData(buffer);
                case 0x2f: return x2f_Authenticate(buffer, COMMAND_DATA_BUFFER);
                case 0x30: return x30(buffer);
                case 0x35: return x35(buffer);
                case 0x37: return x37(buffer);
                case 0x3b: return x3b(buffer);

                case 0x3c: cell = 0;
                case 0x3d: cell = 1;
                case 0x3e: cell = 2;
                case 0x3f: cell = 3;
                    return x3c_x3f_CellVoltage(buffer, cell);

                case 0x63:
                case 0x64:
                case 0x65:
                case 0x66: return x63_x66_AuthKey(buffer);

                default: return 255;
            };
        }
    }

    // Run checks to make sure all values are nominal
    void checkValuesAndSetStates() {
        // TODO

        // Temperature is no longer acceptable
        if (false) {
            BATTERY_STATUS.overTempAlarm = true;
            BATTERY_STATUS.terminateChargeAlarm = true;
            BATTERY_STATUS.terminateDischargeAlarm = true;
        }

        // Battery is charged
        if (false) {
            // The battery is considered fully charged when the difference between battery voltage and
            // charging voltage is within 100mV and charging current is less than C/10 *
            BATTERY_STATUS.fullyCharged = true;
            BATTERY_STATUS.terminateChargeAlarm = true;
            BATTERY_STATUS.overchargedAlarm = true;
        }

        // Battery is discharged
        if (false) {
            // SOC estimation by static voltage measurement should be done with separated charge and discharge look-up table *
            // Usually, the full charged capacity reduce 10%~20% after 500 cycles *
            BATTERY_STATUS.fullyDischarged = true;
            BATTERY_STATUS.terminateDischargeAlarm = true;
        }

        // Battery is discharging (can be self-discharge, not always system)
        if (false && BATTERY_STATUS.canDischarge()) {
            BATTERY_STATUS.discharging = true;
        }

        // ALARM_MODE must be reset every <=45s
        if (BATTERY_MODE.alarmMode && millis() - ALARM_MODE_SET_AT > (30 * 1000)) {
            BATTERY_MODE.alarmMode = false;
        }
    }

    // Calculate the voltage and current that should be requested by comparing the battery's current capacity
    // and voltage to lookup tables.
    void calculateChargeParameters() {
        // TODO
    }

    // Read command sent from laptop
    void receiveEvent(int howMany)
    {
        // Set command
        COMMAND = (uint8_t)Wire.read();

        for (int x = 0; x < howMany-1; x++) {
            COMMAND_DATA_BUFFER[x] = (uint8_t)Wire.read();
        }

        #ifdef DEBUG
            Utils::logCommand((char* const)F("Received command: "), COMMAND);
        #endif
    }

    // Write information and send it to laptop
    void requestEvent () {
        // Call the handler responsible for the current command, which writes the relevant data
        // Then get the response length, 255 if no match is found
        uint8_t replyLength = RequestHandlers::handleCommand(REPLY_BUFFER);

        // No matching callback was found, return without further processing
        if (replyLength == 255) {
            #ifdef DEBUG
                Utils::logCommand((char* const)F("WARN: Unimplemented command: "), COMMAND);
            #endif

            return;
        }

        // Some commands require their length to be added. See needsLength function for a list of these commands
        // This length command is sent first, before all the data
        if (Utils::needsLength((uint8_t)COMMAND)) {
            Wire.write(replyLength);
        }

        // Write all bytes from the buffer
        for (uint8_t y = 0; y < replyLength; ++y) {
            Wire.write(REPLY_BUFFER[y]);
        }

        // SMBus messages end with a CRC-8 byte
        Wire.write(Utils::calculateCRC(REPLY_BUFFER, replyLength, COMMAND));

        #ifdef DEBUG
            uint8_t curByte;
            for (int y = replyLength - 1; y >= 0; --y) {
                curByte = REPLY_BUFFER[y];

                if (curByte < 16) Serial.print('0');
                Serial.print(curByte, HEX);

                Serial.print(' ');
            }
            Serial.print("\n");
        #endif
    }
}
