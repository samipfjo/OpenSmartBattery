#include <stdint.h>
#include "utils.hpp"

void main() {
    bool CRC[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    bool expectedCRC[5][8] = { // TODO
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0}
    };

    uint8_t inData[5] = { 0xB6, 0x27, 0xB7, 0x5B, 0x3C };
    uint8_t answer, data;

    for (uint8_t bindex=0; bindex < 5; ++bindex) {
       data = inData[bindex];
       OpenSmartBattery::Utils::addByteToCRC(CRC, data);

       printf("\n[%i] answer/got:\n", bindex);
       for (uint8_t i = 0; i < 8; ++i) {
           printf("%x", expectedCRC[bindex][i]);
           if (i == 3) { printf("_"); }
       }

       printf("\n");
       for (uint8_t i = 0; i < 8; ++i) {
           printf("%x", CRC[i]);
           if (i == 3) { printf("_"); }
       }
    }
}
