#ifndef _MANAGE_IR_CAMERA_H_
#define _MANAGE_IR_CAMERA_H_

void    CalcNextExposure_inner();
//nMode:0-GetImageMode, 1:IR_CameraViewMode
void    CalcNextExposure_ir_screen_inner(int nMode);
void    InitIRCamera_ExpGain();
#endif//_MANAGE_IR_CAMERA_H_
