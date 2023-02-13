// Onur G. Guleryuz 1995, 1996, 1997,
// University of Illinois at Urbana-Champaign,
// Princeton University,
// Polytechnic University.

#if !defined(_WAV_TRF_)
#define _WAV_TRF_

void choose_filter(char name,int tap);

void wav2d_inpl(float **image,int Ni,int Nj,int levs,float *lp,int Nl,
						float *hp,int Nh,char forw,int *shift_arr_row,int *shift_arr_col);

void wavpack2d_inpl(float **image,int Ni,int Nj,int levs,float *lp,int Nl,
						float *hp,int Nh,char forw,int *shift_arr_row,int *shift_arr_col);

void complex_wav_forw(float **im,float ***trf,int Ni,int Nj,int levs);

float **complex_wav_inv(float ***trf,int Ni,int Nj,int levs);

void complex_wav_pack_forw(float **im,float ***trf,int Ni,int Nj,int levs);

float **complex_wav_pack_inv(float ***trf,int Ni,int Nj,int levs);

#endif
