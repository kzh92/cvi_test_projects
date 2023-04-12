#ifndef SYSTEM_BASE_H
#define SYSTEM_BASE_H

#include "common_types.h"

int     face_engine_create(int argc);
int     face_engine_create_again();
void    ResetTmp(int  iUpgradeFlag);
void    StartBattLog();
void    EndBattLog();
int     GetKeyID(unsigned char* abKey);

#endif // SOUND_BASE_H
