#ifndef CONVERTBAYER2Y_CM_RISCV_H
#define CONVERTBAYER2Y_CM_RISCV_H

void convert_bayer2y_rotate_cm_riscv(const unsigned char* bayer, unsigned char* yuv, int width, int height, int iCamFlip);

#endif // CONVERTBAYER2Y_CM_RISCV_H
