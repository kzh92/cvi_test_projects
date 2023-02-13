// Onur G. Guleryuz 1995, 1996, 1997,
// University of Illinois at Urbana-Champaign,
// Princeton University,
// Polytechnic University.

#if !defined(_ALLOC_)
#define _ALLOC_


float **allocate_2d_float(int N,int M,char zero);

void free_2d_float(float **a,int N);

double **allocate_2d_double(int N,int M,char zero);

void free_2d_double(double **a,int N);

int **allocate_2d_int(int N,int M,char zero);

void free_2d_int(int **a,int N);

float *allocate_1d_float(int N,char zero);

double *allocate_1d_double(int N,char zero);

int *allocate_1d_int(int N,char zero);

unsigned char *allocate_1d_uchar(int N,char zero);

#endif
