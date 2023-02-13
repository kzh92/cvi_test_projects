
#ifndef MATD_H__INCLUDED
#define MATD_H__INCLUDED

#include <math.h>
#include <memory>
#include <stdlib.h>
#include <string.h>
typedef unsigned long long uint64;

#define MAX(a, b) (((a) < (b)) ? (b) : (a))
inline void swap(float& a, float& b)
{
	float t = a;
	a = b;
	b = t;
}

void transpose(float *a,int rows, int cols, float *b)
{
	for (int mm = 0; mm < rows; mm++)
	{
		for (int nn = 0; nn < cols; nn++)
		{
			b[nn * rows + mm] = a[mm *cols+ nn];
		}
	}
}

// #define det2(m)   ((float)m(0,0)*m(1,1) - (float)m(0,1)*m(1,0))
/* #define det3(m)   (m(0,0)*((float)m(1,1)*m(2,2) - (float)m(1,2)*m(2,1)) -  \
    m(0, 1)*((float)m(1, 0)*m(2, 2) - (float)m(1, 2)*m(2, 0)) + \
    m(0, 2)*((float)m(1, 0)*m(2, 1) - (float)m(1, 1)*m(2, 0)))
*/
float determinant(float *mat, int rows)
{
	float result;
	if (rows == 2)
		result = mat[0] * mat[1*rows +1] - mat[1] * mat[rows];
	else if (rows == 3)
		result = mat[0] * (mat[rows+1] * mat[2 *rows +2] - mat[1 *rows +2] * mat[2*rows+1]) -
		mat[1] * (mat[rows] * mat[2 *rows +2] - mat[1*rows+2] * mat[2*rows+0]) +
		mat[2] * (mat[rows] * mat[2*rows+1] - mat[1 *rows+1] * mat[2*rows + 0]);
	else if (rows == 1)
		result = mat[0];
	else
	{
		return 0.0;
	}

	return result;
}

static inline
int  CholImpl(float* A, int astep, int m, float* b, int bstep, int n)
{
	float* L = A;
	int i, j, k;
	float s;
// 	astep /= sizeof(float);
// 	bstep /= sizeof(float);

	for (i = 0; i < m; i++)
	{
		for (j = 0; j < i; j++)
		{
			s = A[i*astep + j];
			for (k = 0; k < j; k++)
				s -= L[i*astep + k] * L[j*astep + k];
			L[i*astep + j] = (float)(s*L[j*astep + j]);
		}
		s = A[i*astep + i];
		for (k = 0; k < j; k++)
		{
			float t = L[i*astep + k];
			s -= t*t;
		}
		if (s < 2.2204460492503131e-016)
			return 0;
		L[i*astep + i] = (float)(1. / sqrt(s));
	}

	if (!b)
		return 1;

	// LLt x = b
	// 1: L y = b
	// 2. Lt x = y

	/*
	[ L00             ]  y0   b0
	[ L10 L11         ]  y1 = b1
	[ L20 L21 L22     ]  y2   b2
	[ L30 L31 L32 L33 ]  y3   b3

	[ L00 L10 L20 L30 ]  x0   y0
	[     L11 L21 L31 ]  x1 = y1
	[         L22 L32 ]  x2   y2
	[             L33 ]  x3   y3
	*/

	for (i = 0; i < m; i++)
	{
		for (j = 0; j < n; j++)
		{
			s = b[i*bstep + j];
			for (k = 0; k < i; k++)
				s -= L[i*astep + k] * b[k*bstep + j];
			b[i*bstep + j] = (float)(s*L[i*astep + i]);
		}
	}

	for (i = m - 1; i >= 0; i--)
	{
		for (j = 0; j < n; j++)
		{
			s = b[i*bstep + j];
			for (k = m - 1; k > i; k--)
				s -= L[k*astep + i] * b[k*bstep + j];
			b[i*bstep + j] = (float)(s*L[i*astep + i]);
		}
	}

	return 1;
}

int solve_CHOLESKY(float *src, int srcRows, int srcCols, float *src2arg, int dstCols, float* dst)
{
	// dstRows == srcRows
	int result = 1;
	memcpy(dst, src2arg, srcRows* dstCols * sizeof(float));
	result = CholImpl(src,  srcCols, srcRows, dst, dstCols, dstCols);
	if (!result)
	{
		memset( dst, 0, sizeof(float) * srcRows * dstCols); // dstRows == srcRows
	}
	return result;
}

//template<typename _Tp>
void JacobiSVDImpl_(float* At, int astep, float* _W, float* Vt, int vstep, int m, int n, int n1, float minval, float eps)
{
	// VBLAS<_Tp> vblas;
	// AutoBuffer<float> Wbuf(n);
	// float* W = Wbuf;
	float W[3] = { 0 };

	int i, j, k, iter, max_iter = MAX(m, 30);
	float c, s;
	float sd;
	astep /= sizeof(At[0]);
	vstep /= sizeof(Vt[0]);

	for (i = 0; i < n; i++)
	{
		for (k = 0, sd = 0; k < m; k++)
		{
			float t = At[i*astep + k];
			sd += (float)t*t;
		}
		W[i] = sd;

		if (Vt)
		{
			for (k = 0; k < n; k++)
				Vt[i*vstep + k] = 0;
			Vt[i*vstep + i] = 1;
		}
	}

	for (iter = 0; iter < max_iter; iter++)
	{
		bool changed = false;

		for (i = 0; i < n - 1; i++)
		for (j = i + 1; j < n; j++)
		{
			float *Ai = At + i*astep, *Aj = At + j*astep;
			float a = W[i], p = 0, b = W[j];

			for (k = 0; k < m; k++)
				p += (float)Ai[k] * Aj[k];

			if (abs(p) <= eps*sqrt((float)a*b))
				continue;

			p *= 2;
			float beta = a - b, gamma = hypot((float)p, beta);
			if (beta < 0)
			{
				float delta = (gamma - beta)*0.5f;
				s = (float)sqrt(delta / gamma);
				c = (float)(p / (gamma*s * 2));
			}
			else
			{
				c = (float)sqrt((gamma + beta) / (gamma * 2));
				s = (float)(p / (gamma*c * 2));
			}

			a = b = 0;
			for (k = 0; k < m; k++)
			{
				float t0 = c*Ai[k] + s*Aj[k];
				float t1 = -s*Ai[k] + c*Aj[k];
				Ai[k] = t0; Aj[k] = t1;

				a += (float)t0*t0; b += (float)t1*t1;
			}
			W[i] = a; W[j] = b;

			changed = true;

			if (Vt)
			{
				float *Vi = Vt + i*vstep, *Vj = Vt + j*vstep;
				k = 0;// vblas.givens(Vi, Vj, n, c, s);

				for (; k < n; k++)
				{
					float t0 = c*Vi[k] + s*Vj[k];
					float t1 = -s*Vi[k] + c*Vj[k];
					Vi[k] = t0; Vj[k] = t1;
				}
			}
		}
		if (!changed)
			break;
	}

	for (i = 0; i < n; i++)
	{
		for (k = 0, sd = 0; k < m; k++)
		{
			float t = At[i*astep + k];
			sd += (float)t*t;
		}
		W[i] = sqrt(sd);
	}

	for (i = 0; i < n - 1; i++)
	{
		j = i;
		for (k = i + 1; k < n; k++)
		{
			if (W[j] < W[k])
				j = k;
		}
		if (i != j)
		{
			swap(W[i], W[j]);
			if (Vt)
			{
				for (k = 0; k < m; k++)
					swap(At[i*astep + k], At[j*astep + k]);

				for (k = 0; k < n; k++)
					swap(Vt[i*vstep + k], Vt[j*vstep + k]);
			}
		}
	}

	for (i = 0; i < n; i++)
		_W[i] = (float)W[i];

	if (!Vt)
		return;

	uint64 state = 0x12345678;
	for (i = 0; i < n1; i++)
	{
		sd = i < n ? W[i] : 0;

		for (int ii = 0; ii < 100 && sd <= minval; ii++)
		{
			// if we got a zero singular value, then in order to get the corresponding left singular vector
			// we generate a random vector, project it to the previously computed left singular vectors,
			// subtract the projection and normalize the difference.
			const float val0 = (float)(1. / m);
			for (k = 0; k < m; k++)
			{

				state = (uint64)(unsigned)state * 4164903690U + (unsigned)(state >> 32);
				float val = (((unsigned)state) & 256) != 0 ? val0 : -val0;
				At[i*astep + k] = val;
			}
			for (iter = 0; iter < 2; iter++)
			{
				for (j = 0; j < i; j++)
				{
					sd = 0;
					for (k = 0; k < m; k++)
						sd += At[i*astep + k] * At[j*astep + k];
					float asum = 0;
					for (k = 0; k < m; k++)
					{
						float t = (float)(At[i*astep + k] - sd*At[j*astep + k]);
						At[i*astep + k] = t;
						asum += abs(t);
					}
					asum = asum > eps * 100 ? 1 / asum : 0;
					for (k = 0; k < m; k++)
						At[i*astep + k] *= asum;
				}
			}
			sd = 0;
			for (k = 0; k < m; k++)
			{
				float t = At[i*astep + k];
				sd += (float)t*t;
			}
			sd = sqrt(sd);
		}

		s = (float)(sd > minval ? 1 / sd : 0.);
		for (k = 0; k < m; k++)
			At[i*astep + k] *= s;
	}
}

#define FLT_MIN         1.175494351e-38F
#define FLT_EPSILON     1.192092896e-07F 

void _SVDcompute(float *src, int rows, int cols,float *_w, float *_u, float *_vt, int flags)
{
	int m = rows, n = cols;// m = n

	//Matd<_Tp> temp_a;
	//Matd<_Tp> temp_u(n, m);
	float temp_a[3][3];
	transpose(src, rows,cols, (float *)temp_a);
	JacobiSVDImpl_((float *)temp_a, m * sizeof(float), _w, _vt, n * sizeof(float), m, n, n, FLT_MIN, FLT_EPSILON * 10);
	transpose((float *)temp_a,cols,rows, _u);

}

void matSVD(float *pa, float *pw, float *uarr, float *varr, int flags)
{
	//Matd<_Tp>  u, vt;
	float  u[3][3] = { 0 }, vt[3][3] = { 0 };
	
	_SVDcompute(pa,3,3, pw, (float *)u, (float *)vt, 1);

	//if (u.fAlloced)
	//{
		if (flags & 2) transpose((float *)u,3,3, uarr);
		else memcpy(uarr, u, sizeof(u));
	//}

	//if (vt.fAlloced)
	//{
		if (!(flags & 4)) transpose((float *)vt,3,3, varr);
		else memcpy(varr,vt, sizeof(vt));
	//}

	//*pw = w;
}

int kvRodrigues2(const float* src, int srcRows, int srcCols,float* dst, int dstRows, int dstCols, float* jacobian)
{
	int k;
	//Matd<float> matJ(3, 9, J);
// 	float matJ[3][9];
// 	float *J = (float *)matJ;

	if (srcCols == 1 || srcRows == 1)
	{
		float rx, ry, rz, theta;
		// int step = 1;

		rx = src[0];// src->val[0][0]; // 0,0
		ry = src[1 * srcCols]; // (1,0)
		rz = src[2 * srcCols]; // (2,0)

		theta = sqrt(rx*rx + ry*ry + rz*rz);

		if (theta < FLT_EPSILON)
		{
			for (int i = 0; i < dstRows; i++)
			for (int j = 0; j < dstCols; j++)
				dst[i*dstCols + j] = (i == j) ? 1.0f : 0.0f;
		}
		else
		{
			const float I[] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };

			float c = cos(theta);
			float s = sin(theta);
			float c1 = 1.0f - c;
			float itheta = theta ? 1.0f / theta : 0.0f;

			rx *= itheta; ry *= itheta; rz *= itheta;

			float rrt[] = { rx*rx, rx*ry, rx*rz, rx*ry, ry*ry, ry*rz, rx*rz, ry*rz, rz*rz };
			float _r_x_[] = { 0, -rz, ry, rz, 0, -rx, -ry, rx, 0 };

			// R = cos(theta)*I + (1 - cos(theta))*r*rT + sin(theta)*[r_x]
			// where [r_x] is [0 -rz ry; rz 0 -rx; -ry rx 0]
			for (k = 0; k < 9; k++)
				//dst[0*dstCols + k] = c*I[k] + c1*rrt[k] + s*_r_x_[k];
				dst[k] = c*I[k] + c1*rrt[k] + s*_r_x_[k];
		}
	}
	else if (srcCols == 3 && srcRows == 3)
	{
		float R[9], U[9], V[9], W[3], rx, ry, rz;
		float (*matR)[3] = (float(*)[3])R;
		float (*matU)[3] = (float(*)[3])U;
		float (*matV)[3] = (float(*)[3])V;
		float (*matW)[1] = (float(*)[1])W;
// 		Matd<float> matR(3, 3, R);
// 		Matd<float> matU(3, 3, U);
// 		Matd<float> matV(3, 3, V);
// 		Matd<float> matW(3, 1, W);
		float theta, s, c;
		// int step = 1;

		for (int i = 0; i < srcRows; i++)
		for (int j = 0; j < srcCols; j++)
			matR[i][j] = src[i * srcCols + j];

		matSVD((float *)matR, (float *)matW, (float *)matU, (float *)matV, 1 + 2 + 4);
		// matGEMM(&matU, &matV, 1, (Matd<float>*)0, 0, &matR, /*KV_GEMM_A_T*/1);
		float matUt[3][3];
		transpose((float *)matU,3,3,(float *)matUt);
		//matR = matUt * matV;
		for (int row = 0; row < 3; row ++)
		for (int col = 0; col < 3; col++)
		{
			float sum = 0.0f;
			for (int k = 0; k < 3; k++)
				sum += matUt[row][k] * matV[k][col];
			matR[row][col] = sum;
		}

		rx = R[7] - R[5];
		ry = R[2] - R[6];
		rz = R[3] - R[1];

		s = sqrt((rx*rx + ry*ry + rz*rz)*0.25f);
		c = (R[0] + R[4] + R[8] - 1)*0.5f;
		c = c > 1.0f ? 1.0f : c < -1.0f ? -1.0f : c;
		theta = acos(c);

		if (s < 1e-5)
		{
			float t;

			if (c > 0)
				rx = ry = rz = 0;
			else
			{
				t = (R[0] + 1)*0.5f;
				rx = sqrt(MAX(t, 0.0f));
				t = (R[4] + 1)*0.5f;
				ry = sqrt(MAX(t, 0.0f))*(R[1] < 0.0f ? -1.0f : 1.0f);
				t = (R[8] + 1)*0.5f;
				rz = sqrt(MAX(t, 0.0f))*(R[2] < 0.0f ? -1.0f : 1.0f);
				if (fabs(rx) < fabs(ry) && fabs(rx) < fabs(rz) && (R[5] > 0) != (ry*rz > 0))
					rz = -rz;
				theta /= sqrt(rx*rx + ry*ry + rz*rz);
				rx *= theta;
				ry *= theta;
				rz *= theta;
			}
		}
		else
		{
			float vth = 1 / (2 * s);

			vth *= theta;
			rx *= vth; ry *= vth; rz *= vth;
		}

// 		dst->val[0][0] = (_Tp)rx;
// 		dst->val[1][0] = (_Tp)ry;
// 		dst->val[2][0] = (_Tp)rz;
		dst[0] = (float)rx;
		dst[dstCols] = (float)ry;
		dst[2*dstCols] = (float)rz;
	}

	return 1;
}

void Rodrigues(const float* src, int rows, int cols, float *dst)
{
	//dst.create(3, (src.cols == 1 || src.rows == 1) ? 3 : 1);
	int dstRows = 3;
	int dstCols = (cols == 1 || rows == 1) ? 3 : 1;
	if (kvRodrigues2(src, rows, cols, dst,dstRows, dstCols, 0) <= 0.0f)
	{
		memset(dst, 0, sizeof(float)* dstRows* dstCols);
	}
}


#endif //MATD_H__INCLUDED
