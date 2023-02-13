#ifndef TYPE_H_
#define TYPE_H_

#ifndef CV_PI
#define CV_PI   3.1415926535897932384626433832795
#endif

#ifndef NULL
#define NULL 0
#endif // !NULL

#ifndef SWAP
#define SWAP(a,b,t) ((t) = (a), (a) = (b), (b) = (t))
#endif // !SWAP


#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

#ifndef uchar
typedef unsigned char uchar;
#endif

enum MAT_TYPE
{
	TYPE_BYTE,
	TYPE_INT,
	TYPE_SHORT,
	TYPE_DOUBLE,
	TYPE_FLOAT,
	TYPE_MAX
};

const unsigned int TWO = 2;
const unsigned int TRIPLET = 3;
const unsigned int QUADLET = 4;

#define TERMCRIT_ITER    1
#define TERMCRIT_NUMBER  TERMCRIT_ITER
#define TERMCRIT_EPS     2

typedef struct TermCriteria
{
    int    type;  /* may be combination of
                     CV_TERMCRIT_ITER
                     CV_TERMCRIT_EPS */
    int    max_iter;
    double epsilon;
}
TermCriteria;

TermCriteria  termCriteria( int type, int max_iter, double epsilon );


struct FacePoseAngle
{
	float rXAngle;
	float rYAngle;
	float rZAngle;
};

int sizeofMatElement(enum MAT_TYPE nType);

typedef struct _tagSImage
{
	unsigned char* pBuffer;
	int nWidth;
	int nHeight;
}SImage;

unsigned char getPixelInSImage(SImage* pImage, int nY, int nX);
void setPixelInSImage(SImage* pImage, int nY, int nX, unsigned char value);
SImage* newImage(int nWidth, int nHeight);
void deleteImage(SImage* pImage);
void ConvertSImageToBuffer(SImage* pImage, unsigned char** pBuffer);
void ConvertBufferToSImage(unsigned char* pbBuffer, int nWidth, int nHeight, SImage** pSrcImage);
SImage* cropSImage(SImage* pImage, int nX, int nY, int nWidth, int nHeight);
void pasteImage(SImage* pDesImage, SImage* pSrcImage, int nX, int nY, int nWidth, int nHeight);


typedef struct _tagSMat
{
	void* pData;
	int nRows;
	int nCols;
	enum MAT_TYPE nType;
	int nStep;
	int nElementSize;
}SMat;

/*
#define MAT_ELEM( mat, elemtype, row, col )           \
    (*(elemtype*)((elemtype*)(mat).pData + (size_t)(mat).nStep* (row) + (sizeof(elemtype))*(col)))
*/
#define MAT_ELEM( mat, elemtype, row, col )           \
	(*(elemtype*)((elemtype*)(mat).pData + (mat).nCols* (row) + (col)))


SMat* createMat(int nRows, int nCols, enum MAT_TYPE nType);
void releaseMat(SMat* pMat);
int countNonZero( SMat* src );


typedef struct Point
{
	int x;
	int y;
}
Point;


__inline  Point  point( int x, int y )
{
	Point p;

	p.x = x;
	p.y = y;

	return p;
}

typedef struct Size
{
	int width;
	int height;
}
Size;

__inline Size  size( int width, int height )
{
	Size s;

	s.width = width;
	s.height = height;

	return s;
}

typedef struct Rect
{
	int x;
	int y;
	int width;
	int height;
}
Rect;

__inline  Rect  rect(int x, int y, int width, int height)
{
	Rect r;

	r.x = x;
	r.y = y;
	r.width = width;
	r.height = height;

	return r;
}


#endif//TYPE_H_
