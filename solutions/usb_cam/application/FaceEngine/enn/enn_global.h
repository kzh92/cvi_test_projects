
#ifndef _ENN_GLOBAL_H__INCLUDED_
#define _ENN_GLOBAL_H__INCLUDED_

namespace ENN
{
	typedef struct _tag_enn_blob_
	{
		short c;
		short pack;
		int w;
		int h;
		int cstep;
		char mem[16];
	} enn_blob;

	int get_blob_size(int size, int align_size = 4);
	void aligned_free(void* adata);
	unsigned char* aligned_malloc(int sz, int align_size = 16);
}

extern int g_nThreadCount;

#endif // _ENN_GLOBAL_H__INCLUDED_
