#ifndef UPGRADE_FIRMWARE_H
#define UPGRADE_FIRMWARE_H

#include "appdef.h"

#define RTOS_START_ADDR			0x250000
#define NEW_RTOS_ADDR			0x251000
typedef struct _tagUpInfo
{
    unsigned int offset;
    unsigned int size;
    unsigned int reserved[14];
} stUpInfo;

#define REH_MAGIC_LEN   8
#define REH_MAX_SLOTS   64
#define REH_KEY_LEN     32
#define REH_MAGIC       "ESNRT1"

typedef struct _tagREHSlot
{
    unsigned int offset;
    unsigned int size;
} stREHSlot;

typedef struct _tagRtosEasenHeader
{
    char magic[REH_MAGIC_LEN];
    unsigned int rtos_length;
    unsigned int slot_count;
    stREHSlot slots[REH_MAX_SLOTS];
    unsigned char key[REH_KEY_LEN];
} stRtosEasenHeader;

void doUpgradeFirmware();
void resetUpgradeInfo();

#endif // !CHECK_FIRMWARE_H