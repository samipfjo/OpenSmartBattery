#include "sha1.h"
#include <stdint.h>
#include <string.h>

namespace OpenSmartBattery {
    namespace Authentication {

        /*
        const uint8_t AUTH_KEY[] PROGMEM = {
            0x33, 0x32, 0x31, 0x30,
            0x2f, 0x2e, 0x2d, 0x2c,
            0x2b, 0x2a, 0x29, 0x28,
            0x27, 0x26, 0x25, 0x24 //,
            // 0x23, 0x22, 0x21, 0x20
        };
        */

        const uint8_t AUTH_KEY[] PROGMEM = {
            0x10, 0x32, 0x54, 0x76,
            0x98, 0xba, 0xdc, 0xfe,
            0xef, 0xcd, 0xab, 0x89,
            0x67, 0x45, 0x23, 0x01
        };

        // Define your battery's authentication method here
        // Remember that the byte order is big endian (LSB->MSB), but bit order is little endian (MSB->LSB)!
        inline uint8_t authenticate(uint8_t *output_buffer, uint8_t *challenge_buffer) {
            Sha1.initHmac(AUTH_KEY, 16);       // Key length is 16 bytes
            Sha1.write(challenge_buffer, 20);  // Challenge is 20 bytes

            // Copy 20-byte SHA hash to the output buffer
            memcpy(output_buffer, Sha1.resultHmac(), sizeof(uint8_t) * 20);

            return 20;
        }
    }
}
