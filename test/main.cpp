#include "OpenSmartBattery.hpp"
#include "config.hpp"
#include "utils.hpp"
#include <assert.h>

namespace OpenSmartBattery {
    namespace Tests {
        void testBatteryMode() {
            Utils::BatteryMode batteryMode = Utils::BatteryMode();
            uint8_t higher, lower;
            batteryMode.asSplitBytes(&higher, &lower);

            // 0000 0000 0000 0001
            assert(higher == 0);
            assert(lower == 1);

            // 1000 0000 0000 0001
            batteryMode.capacityMode = 1;
            batteryMode.asSplitBytes(&higher, &lower);
            assert(higher == 128);
            assert(lower == 1);
        }
    }
}


int main() {
    OpenSmartBattery::Tests::testBatteryMode();
}

