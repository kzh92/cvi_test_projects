// Onur G. Guleryuz 1995, 1996, 1997,
// University of Illinois at Urbana-Champaign,
// Princeton University,
// Polytechnic University.

#if !defined(_WAV_BASIC_)
#define _WAV_BASIC_

void filt_n_dec_all_rows(float **image,int Ni,int Nj,float *lp,int Nl,
							float *hp,int Nh);

void filt_n_dec_all_cols(float **image,int Ni,int Nj,float *lp,int Nl,
							float *hp,int Nh);

void ups_n_filt_all_rows(float **image,int Ni,int Nj,float *lp,int Nl,
							float *hp,int Nh,int lev,int *shift_arr);

void ups_n_filt_all_cols(float **image,int Ni,int Nj,float *lp,int Nl,
							float *hp,int Nh,int lev,int *shift_arr);

#endif
