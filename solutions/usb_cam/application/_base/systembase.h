#ifndef SYSTEM_BASE_H
#define SYSTEM_BASE_H

#include "common_types.h"

int     face_engine_create(int argc);
int     face_engine_create_again();
int     RTCInit(int argc);
int     RTCInit();
void    ResetTmp(int  iUpgradeFlag);
void    StartBattLog();
void    EndBattLog();
int     GetKeyID(unsigned char* abKey);
void    LedConfig();
void    SetLed(int iLedPort);
//void    SetStatus(int iStatus);


#endif // SOUND_BASE_H
