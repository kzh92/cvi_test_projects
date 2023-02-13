#ifndef COUNT_BASE_H
#define COUNT_BASE_H

#include "EngineStruct.h"
#if 0
int     ReadSystemRunningCount();
void    IncreaseSystemRunningCount();

int     ReadSystemRunningCount1();
void    IncreaseSystemRunningCount1();

int     ReadClrCamErrorCount();
void    IncClrCamErrorCount();
int     ReadIRCamErrorCount();
void    IncIRCamErrorCount();


int     ReadSuccessSendCount();
void    IncreaseSuccessSendCount();

int     ReadFailedSendCount();
void    IncreaseFailedSendCount();

void    IncreaseFaceFailedCount();
int     ReadFaceFailedCount();
void    ResetFaceFailedCount();
void    SetLastFaceFailedTime();
DATETIME_32   GetLastFaceFailedTime();

void    IncreaseHandFailedCount();
int     ReadHandFailedCount();
void    ResetHandFailedCount();
void    SetLastHandFailedTime();
DATETIME_32   GetLastHandFailedTime();

void    IncreasePasscodeFailedCount();
int     ReadPasscodeFailedCount();
void    ResetPasscodeFailedCount();
void    SetLastPasscodeFailedTime();
DATETIME_32   GetLastPasscodeFailedTime();
int     CheckPasscodeError();

void    IncreaseRequestCount();
int     ReadRequestCount();
void    ResetRequestCount();
void    SetLastRequestTime();
DATETIME_32   GetLastRequestTime();
#endif
int ReadSystemRunningCount();
void IncreaseSystemRunningCount();
int ReadSystemRunningCount1();
void IncreaseSystemRunningCount1();

#endif // COUNT_BASE_H
