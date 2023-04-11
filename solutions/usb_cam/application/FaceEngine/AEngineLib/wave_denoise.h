// Onur G. Guleryuz 1995, 1996, 1997,
// University of Illinois at Urbana-Champaign,
// Princeton University,
// Polytechnic University.

#if !defined(_WAVE_DENOISE_)
#define _WAVE_DENOISE_
#ifdef __cplusplus
extern "C" {
#endif
    void wavelet_denoising(unsigned char* pbIn, int Ni, int Nj, float rThreshold);
    void wavelet_complex_denoising(unsigned char* pbIn, int Ni, int Nj, float rThreshold);
#ifdef __cplusplus
}
#endif

#endif //_WAVE_DENOISE_
