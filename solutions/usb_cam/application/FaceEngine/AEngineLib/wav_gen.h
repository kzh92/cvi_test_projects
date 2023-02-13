// Onur G. Guleryuz 1995, 1996, 1997,
// University of Illinois at Urbana-Champaign,
// Princeton University,
// Polytechnic University.

#if !defined(_WAV_GEN_)
#define _WAV_GEN_

// Grow only the LL band after this level 
// (relevant for packet decompositions).
#define LL_ONLY_AFTER_LEV 6

// Maximum level possible in all decompositions.
// Mainly because we are using static allocation in some
// places.
#define MAX_LEV 6
#define MAX_LEV_POW 64 /* 2^MAX_LEV */

#define MAX_ARR_SIZE (MAX_LEV_POW)

#ifndef max
#define max(a,b) \
	((a)>(b)?(a):(b))
#endif

#ifndef min
#define min(a,b) \
	((a)<(b)?(a):(b))
#endif

#endif
