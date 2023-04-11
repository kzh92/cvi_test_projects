
#ifndef _ENN_GLOBAL_H__INCLUDED_
#define _ENN_GLOBAL_H__INCLUDED_

#ifdef __cplusplus
extern "C"{
#endif

	typedef struct _tag_enn_blob_
	{
		short c;
		short pack;
		int w;
		int h;
		int cstep;
		char mem[16];
	} enn_blob;

    int enn_get_blob_size(int size, int align_size); // int align_size = 4
    void enn_aligned_free(void* adata);
    unsigned char* enn_aligned_malloc(int sz, int align_size); // int align_size = 16

	float _gnu_h2f_internal_f(unsigned short a);
	unsigned short _gnu_f2h_internal_f(float v);
	void _gnu_h2f_internal_vector(const unsigned short* src, float* dst, int n);
    void _gnu_f2h_internal_vector(const float* src, unsigned short* dst, int n);

#ifdef __cplusplus
}
#endif

#endif // _ENN_GLOBAL_H__INCLUDED_
