
#if __ARM_NEON
#include <arm_neon.h>
#endif
#include <math.h>

#include "enn_softmax.h"

namespace ENN
{
	void softmax(float* inout, int count)
	{
#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int i = 0; i < count; i++)
		{
			float* pt = inout + i * 2;

			float a = pt[0];
			float b = pt[1];

			if (a < b)
			{
				float c = (float)exp(a - b);
				//float c = exp_quant(a - b);

				c = 1.0f / (1.0f + c);
				pt[0] = 1 - c;
				pt[1] = c;
			}
			else
			{
				float c = (float)exp(b - a);
				//float c = exp_quant(b - a);

				c = 1.0f / (1.0f + c);
				pt[0] = c;
				pt[1] = 1 - c;
			}
		}
	}
}
