#ifndef CHECK_FIRMWARE_H
#define CHECK_FIRMWARE_H

#include "appdef.h"

#define DICT_START_ADDR 0x00450000

#if (CHECK_FIRMWARE == 1)
void doCheckFirmware();
#endif // CHECK_FIRMWARE == 1

#endif // !CHECK_FIRMWARE_H