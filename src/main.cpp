#include "OpenSmartBattery.hpp"
#include "utils.hpp"
#include "config.hpp"

#include <util/atomic.h>
#include <util/delay.h>
#include <Print.h>
#include <Wire.h>
#include <Arduino.h>

using namespace OpenSmartBattery;

void setup() {
    // Initialize all the pins; customize these in lib/OpenSmartBattery/config.hpp
    pinMode(HardwareConfig::Pins::SERIAL_IN,  INPUT);
    pinMode(HardwareConfig::Pins::SERIAL_OUT, OUTPUT);

    pinMode(HardwareConfig::Pins::CHARGE_TRANSISTOR, OUTPUT);
    pinMode(HardwareConfig::Pins::OUTPUT_TRANSISTOR, OUTPUT);

    pinMode(HardwareConfig::Pins::CURRENT_SENSE, INPUT);
    pinMode(HardwareConfig::Pins::CELL_0_VOLTAGE, INPUT);
    pinMode(HardwareConfig::Pins::CELL_1_VOLTAGE, INPUT);
    pinMode(HardwareConfig::Pins::CELL_2_VOLTAGE, INPUT);
    pinMode(HardwareConfig::Pins::PACK_VOLTAGE, INPUT);
    pinMode(HardwareConfig::Pins::PACK_TEMP_SENSE, INPUT);

    // Initialize the output pins to their default values
    digitalWrite(HardwareConfig::Pins::SERIAL_OUT, LOW);
    digitalWrite(HardwareConfig::Pins::CHARGE_TRANSISTOR, LOW);
    digitalWrite(HardwareConfig::Pins::OUTPUT_TRANSISTOR, LOW);

    Wire.begin(0x0B);
    Wire.onReceive(OpenSmartBattery::receiveEvent);
    Wire.onRequest(OpenSmartBattery::requestEvent);

    #ifdef DEBUG
        Utils::Serial.begin(115200);
        Utils::Serial.println(F("Awake..."));
    #endif
}

void loop() {
    _delay_us(1);

    // These following two blocks MUST BE DONE ATOMICALLY to prevent an interrupt from ruining our day

    // Every 5ms, run internal calculations to determine the current conditions of the battery
    ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
        OpenSmartBattery::checkValuesAndSetStates();
        OpenSmartBattery::calculateChargeParameters();
    }

    // Every 5-60s, report ChargingCurrent and ChargingVoltage
    ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
        if (OpenSmartBattery::BATTERY_MODE.chargerMode && false) { }
    }
}
