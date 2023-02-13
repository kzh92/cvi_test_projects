
#ifndef _ENNQ_GLOBAL_H__INCLUDED_
#define _ENNQ_GLOBAL_H__INCLUDED_

namespace ENNQ
{
	typedef struct _tag_ennq_blob_
	{
		int w;
		int h;
		unsigned short c;
		unsigned char packp; 		// channel pack size
		unsigned char elemc;		// 1 : signed char, 4 : float
		int cstep;
		char mem[16];
	} ennq_blob;

	int get_blob_size(int size, int align_size = 16);
	unsigned char* aligned_malloc(int sz, int align_size = 16);
	void aligned_free(void* adata);
}

extern int g_nThreadCount;

#endif //_ENNQ_GLOBAL_H__INCLUDED_
