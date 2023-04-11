#ifndef _MANAGE_IR_CAMERA_H_
#define _MANAGE_IR_CAMERA_H_

void    CalcNextExposure_inner();
void    CalcNextExposure_inner_hand();

//nMode:0-GetImageMode, 1:IR_CameraViewMode
void    CalcNextExposure_ir_screen_inner(int nMode);
void    InitIRCamera_ExpGain();
void    BackupIRCamera_ExpGain();

#endif//_MANAGE_IR_CAMERA_H_
