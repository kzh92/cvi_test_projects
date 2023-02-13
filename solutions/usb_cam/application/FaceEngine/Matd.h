
#ifndef MATD_H__INCLUDED
#define MATD_H__INCLUDED

int solve_CHOLESKY(float *src, int srcRows, int srcCols, float *src2arg, int dstCols, float* dst);
void _SVDcompute(float *src, int rows, int cols, float *_w, float *_u, float *_vt, int flags);
void Rodrigues(const float* src, int rows, int cols, float *dst);
float determinant(float *mat, int rows);
#endif //MATD_H__INCLUDED
