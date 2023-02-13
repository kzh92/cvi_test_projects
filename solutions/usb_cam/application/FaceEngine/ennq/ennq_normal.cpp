
#include <math.h>

#include "ennq_normal.h"

namespace ENNQ
{
	void normalize(float* feat, int sz)
	{
		int i;
		float sum = 0.0f;
		for (i = 0; i < sz; i++)
		{
			sum += feat[i] * feat[i];
		}

		sum = sqrtf(sum);
		for (i = 0; i < sz; i++)
		{
			feat[i] = feat[i] / sum;
		}
	}
}