// Onur G. Guleryuz 1995, 1996, 1997,
// University of Illinois at Urbana-Champaign,
// Princeton University,
// Polytechnic University.

#if !defined(_WAV_FILTERS_)
#define _WAV_FILTERS_

// beg parameters may effect mirror symmetric extension (PS=0 case) for
// symmetric banks. Check the comments in ups_n_filt_all_rows in
// wav_basic.c as needed.

// Daubechies 7-9 biorth filters.

// FLP : forward low pass.
// FHP : forward high pass.
// ILP : inverse low pass.
// IHP : inverse high pass.
float D79_FLP[9] =
        {(float)0.0378287, (float)-0.0238492, (float)-0.110624, (float)0.3774026, \
		 (float)0.8526986, (float)0.3774026, (float)-0.110624, (float)-0.0238492, \
		 (float)0.0378287};

float D79_FHP[7] =
        {(float)0.064539, (float)-0.0406897, (float)-0.4180924, (float)0.7884848, \
		 (float)-0.4180924, (float)-0.0406897, (float)0.064539};

float D79_ILP[7] =
        {(float)-0.064539, (float)-0.0406897, (float)0.4180924, (float)0.7884848, \
		 (float)0.4180924, (float)-0.0406897, (float)-0.064539};

float D79_IHP[9] =
        {(float)0.0378287, (float)0.0238492, (float)-0.110624, (float)-0.3774026, \
		 (float)0.8526986, (float)-0.3774026, (float)-0.110624, (float)0.0238492, \
		 (float)0.0378287};

// dimensions of the filters
int D79_Nilp=7,D79_Nihp=9;
int D79_Nflp=9,D79_Nfhp=7;

// symmetric bank, so data extensions at the boundaries should use
// mirror symmetry and NOT periodic symmetry.
int D79_PS=0;

// How the filters should be placed on data (the default shifts)
// see filter_n_decimate() in wav_basic.c .
int D79_begilp=3,D79_begihp=5;
int D79_begflp=4,D79_begfhp=4;


/**************************************************************************/
// Antonini 7-9 biorth filters.

float Antonini_FLP[9] = {(float)0.037828798579918, (float)-0.023849297515860, \
							(float)-0.110624027489511, (float)0.377402688109134, \
							(float)0.852698653219295, (float)0.377402688109134, \
		   					(float)-0.110624027489511, (float)-0.023849297515860, \
							(float)0.037828798579918};

float Antonini_FHP[7] = {(float)0.064539050132459, (float)-0.040689752616599, \
							(float)-0.418092440725732, (float)0.788484872206183, \
							(float)-0.418092440725732, (float)-0.040689752616599, \
							(float)0.064539050132459};

float Antonini_ILP[7] = {(float)-0.064539050132459, (float)-0.040689752616599, \
							(float)0.418092440725732, (float)0.788484872206183, \
							(float)0.418092440725732, (float)-0.040689752616599, \
							(float)-0.064539050132459};

float Antonini_IHP[9] = {(float)0.037828798579918, (float)0.023849297515860, \
							(float)-0.110624027489511, (float)-0.377402688109134, \
							(float)0.852698653219295, (float)-0.377402688109134, \
		   					(float)-0.110624027489511, (float)0.023849297515860, \
							(float)0.037828798579918};

int Antonini_Nilp=7,Antonini_Nihp=9;
int Antonini_Nflp=9,Antonini_Nfhp=7; 

int Antonini_PS=0;

int Antonini_begilp=3,Antonini_begihp=5;
int Antonini_begflp=4,Antonini_begfhp=4;

/**************************************************************************/
// Haar.

float Haar_FLP[2] ={(float)0.70711,(float)0.70711};

float Haar_FHP[2] ={(float)0.70711,(float)-0.70711};

float Haar_ILP[2] ={(float)0.70711,(float)0.70711};

float Haar_IHP[2] ={(float)-0.70711,(float)0.70711};

int Haar_Nilp=2,Haar_Nihp=2;
int Haar_Nflp=2,Haar_Nfhp=2;

// orthnormal bank use periodic symmetry for perfect reconstruction.
int Haar_PS=1;

int Haar_begilp=0,Haar_begihp=0;
int Haar_begflp=0,Haar_begfhp=0;

/**************************************************************************/
// Daubechies 8 tap orthogonal.

float D8_FLP[8]={(float)0.2303778133088971, (float)0.7148465705529174, (float)0.6308807679298597, \
				(float)-0.0279837694168613, (float)-0.1870348117190946, (float)0.0308413818355606, \
				(float)0.0328830116668853, (float)-0.0105974017850690};

float D8_FHP[8]={(float)-0.0105974017850690, (float)-0.0328830116668853, (float)0.0308413818355606, \
				(float)0.1870348117190946, (float)-0.0279837694168613, (float)-0.6308807679298597, \
				(float)0.7148465705529174, (float)-0.2303778133088971};

float D8_ILP[8]={(float)-0.0105974017850690, (float)0.0328830116668853, (float)0.0308413818355606, \
				(float)-0.1870348117190946, (float)-0.0279837694168613, (float)0.6308807679298597, \
				(float)0.7148465705529174, (float)0.2303778133088971};

float D8_IHP[8]={(float)-0.2303778133088971, (float)0.7148465705529174, (float)-0.6308807679298597, \
				(float)-0.0279837694168613, (float)0.1870348117190946, (float)0.0308413818355606, \
				(float)-0.0328830116668853, (float)-0.0105974017850690};

int D8_Nilp=8,D8_Nihp=8;
int D8_Nflp=8,D8_Nfhp=8; 

int D8_PS=1;

int D8_begilp=3,D8_begihp=3;
int D8_begflp=3,D8_begfhp=3; 

/**************************************************************************/
// Kingsbury's orthogonal bank for complex wavelets.
// For complex wavelet transform ILP, IHP give the flipped version.

float Nick_FLP[8]={(float)0.035164, (float)0, (float)-0.088329, (float)0.233890, (float)0.760272, \
			(float)0.587518, (float)0, (float)-0.114302};

float Nick_FHP[8]={(float)-0.114302, (float)0, (float)0.587518, (float)-0.760272, (float)0.233890, \
			(float)0.088329, (float)0, (float)-0.035164};

float Nick_ILP[8]={(float)-0.114302, (float)0, (float)0.587518, (float)0.760272, (float)0.233890, \
			(float)-0.088329, (float)0, (float)0.035164};

float Nick_IHP[8]={(float)-0.035164, (float)0, (float)0.088329, (float)0.233890, (float)-0.760272, \
			(float)0.587518, (float)0, (float)-0.114302};

int Nick_Nflp=8,Nick_Nfhp=8;
int Nick_Nilp=8,Nick_Nihp=8;

int Nick_PS=1;

int Nick_begflp=5,Nick_begfhp=3;
int Nick_begilp=5,Nick_begihp=3;

/**************************************************************************/
// Kingsbury's orthogonal bank for complex wavelets flipped (inverse of above).
// Listed for convenience.

float Nick_flip_FLP[8]={(float)-0.114302, (float)0, (float)0.587518, (float)0.760272, (float)0.233890, \
			(float)-0.088329, (float)0, (float)0.035164};

float Nick_flip_FHP[8]={(float)-0.035164, (float)0, (float)0.088329, (float)0.233890, (float)-0.760272, \
			(float)0.587518, (float)0, (float)-0.114302};

float Nick_flip_ILP[8]={(float)0.035164, (float)0, (float)-0.088329, (float)0.233890, (float)0.760272, \
			(float)0.587518, (float)0, (float)-0.114302};

float Nick_flip_IHP[8]={(float)-0.114302, (float)0, (float)0.587518, (float)-0.760272, (float)0.233890, \
			(float)0.088329, (float)0, (float)-0.035164};

int Nick_flip_Nflp=8,Nick_flip_Nfhp=8;
int Nick_flip_Nilp=8,Nick_flip_Nihp=8;

int Nick_flip_PS=1;

int Nick_flip_begflp=3,Nick_flip_begfhp=5;
int Nick_flip_begilp=3,Nick_flip_begihp=5;

/**************************************************************************/
// Generic filter pointers and corresponding size/symmetry/starting point variables.
float *MFLP,*MFHP;
float *MILP,*MIHP;

int Nflp,Nfhp; 
int Nilp,Nihp;

int PS;

int begflp,begfhp;
int mfl,mfh;

int begilp,begihp;
int mil,mih;

#endif
