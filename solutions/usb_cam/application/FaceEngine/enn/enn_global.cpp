
#include "common_types.h"
#include <stddef.h>
#include "enn_global.h"

namespace ENN
{
	int get_blob_size(int size, int align_size)
	{
		return (size + (align_size - 1)) & (-align_size);
	}

	unsigned char* aligned_malloc(int sz, int align_size)
	{
        unsigned char* mem_real = (unsigned char*)my_malloc(sz + align_size + sizeof(unsigned char*));
		if (!mem_real) return 0;

		unsigned char** adata = (unsigned char**)((((size_t)((unsigned char**)mem_real + 1)) + align_size - 1) & (-align_size));
		adata[-1] = mem_real;
		return (unsigned char*)adata;
	}

	void aligned_free(void* adata)
	{
		if (!adata) return;
        my_free(((unsigned char**)adata)[-1]);
	}
}
