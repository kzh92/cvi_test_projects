// Onur G. Guleryuz 1995, 1996, 1997,
// University of Illinois at Urbana-Champaign,
// Princeton University,
// Polytechnic University.

#if !defined(_WAV_FILTERS_EXTERN_)
#define _WAV_FILTERS_EXTERN_

// beg parameters may effect mirror symmetric extension (PS=0 case) for
// symmetric banks. Check the comments in ups_n_filt_all_rows as needed.

// Daubechies 7-9 biorth filters.

extern float D79_FLP[9];

extern float D79_FHP[7];

extern float D79_ILP[7];

extern float D79_IHP[9];

extern int D79_Nilp,D79_Nihp;
extern int D79_Nflp,D79_Nfhp;

extern int D79_PS;

extern int D79_begilp,D79_begihp;
extern int D79_begflp,D79_begfhp;

// Antonini 7-9 biorth filters.

extern float Antonini_FLP[9];

extern float Antonini_FHP[7];

extern float Antonini_ILP[7];

extern float Antonini_IHP[9];

extern int Antonini_Nilp,Antonini_Nihp;
extern int Antonini_Nflp,Antonini_Nfhp; 

extern int Antonini_PS;

extern int Antonini_begilp,Antonini_begihp;
extern int Antonini_begflp,Antonini_begfhp;

// Haar.

extern float Haar_FLP[2];

extern float Haar_FHP[2];

extern float Haar_ILP[2];

extern float Haar_IHP[2];

extern int Haar_Nilp,Haar_Nihp;
extern int Haar_Nflp,Haar_Nfhp;

extern int Haar_PS;

extern int Haar_begilp,Haar_begihp;
extern int Haar_begflp,Haar_begfhp;

// Daubechies 8 tap orthogonal.

extern float D8_FLP[8];

extern float D8_FHP[8];

extern float D8_ILP[8];

extern float D8_IHP[8];

extern int D8_Nilp,D8_Nihp;
extern int D8_Nflp,D8_Nfhp; 

extern int D8_PS;

extern int D8_begilp,D8_begihp;
extern int D8_begflp,D8_begfhp; 

// Kingsbury's orthogonal bank for complex wavelets.
// For complex wavelet transform ILP,IHP give the flipped version.

extern float Nick_FLP[8];

extern float Nick_FHP[8];

extern float Nick_ILP[8];

extern float Nick_IHP[8];

extern int Nick_Nflp,Nick_Nfhp;
extern int Nick_Nilp,Nick_Nihp;

extern int Nick_PS;

extern int Nick_begflp,Nick_begfhp;
extern int Nick_begilp,Nick_begihp;

// Kingsbury's orthogonal bank for complex wavelets flipped (inverse of above).
// Listed for convenience.

extern float Nick_flip_FLP[8];

extern float Nick_flip_FHP[8];

extern float Nick_flip_ILP[8];

extern float Nick_flip_IHP[8];

extern int Nick_flip_Nflp,Nick_flip_Nfhp;
extern int Nick_flip_Nilp,Nick_flip_Nihp;

extern int Nick_flip_PS;

extern int Nick_flip_begflp,Nick_flip_begfhp;
extern int Nick_flip_begilp,Nick_flip_begihp;

// Generic filter pointers and corresponding size/symmetry/starting point variables.
extern float *MFLP,*MFHP;
extern float *MILP,*MIHP;

extern int Nflp,Nfhp; 
extern int Nilp,Nihp;

extern int PS;

extern int begflp,begfhp;
extern int mfl,mfh;

extern int begilp,begihp;
extern int mil,mih;


#endif
