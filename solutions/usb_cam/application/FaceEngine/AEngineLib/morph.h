#include "type.h"
#include <stdlib.h>
#include <vector>

//! various border interpolation methods
enum {	BORDER_REPLICATE,
		BORDER_CONSTANT,
		BORDER_REFLECT,
		BORDER_WRAP,
		BORDER_REFLECT_101,
		BORDER_REFLECT101,
		BORDER_TRANSPARENT,
		BORDER_DEFAULT=BORDER_REFLECT_101,
		BORDER_ISOLATED=16 };

enum { 
	MORPH_ERODE,
	MORPH_DILATE,
	MORPH_OPEN,
	MORPH_CLOSE,
	MORPH_GRADIENT,
	MORPH_TOPHAT,
	MORPH_BLACKHAT};

//! shape of the structuring element
enum { 
	MORPH_RECT=0, 
	MORPH_CROSS=1, 
	MORPH_ELLIPSE=2 
};

#define  SHAPE_RECT      0
#define  SHAPE_CROSS     1
#define  SHAPE_ELLIPSE   2
#define  SHAPE_CUSTOM    100
//! 1D interpolation function: returns coordinate of the "donor" pixel for the specified location p.
int borderInterpolate( int p, int len, int borderType );

/*!
 The Base Class for 1D or Row-wise Filters

 This is the base class for linear or non-linear filters that process 1D data.
 In particular, such filters are used for the "horizontal" filtering parts in separable filters.

 Several functions in OpenCV return Ptr<BaseRowFilter> for the specific types of filters,
 and those pointers can be used directly or within cv::FilterEngine.
*/
class BaseRowFilter
{
public:
    //! the default constructor
    BaseRowFilter();
    //! the destructor
    virtual ~BaseRowFilter();
    //! the filtering operator. Must be overrided in the derived classes. The horizontal border interpolation is done outside of the class.
    virtual void operator()(const uchar* src, uchar* dst,
                            int width, int cn) = 0;
    int ksize, anchor;
};


/*!
 The Base Class for Column-wise Filters

 This is the base class for linear or non-linear filters that process columns of 2D arrays.
 Such filters are used for the "vertical" filtering parts in separable filters.

 Several functions in OpenCV return Ptr<BaseColumnFilter> for the specific types of filters,
 and those pointers can be used directly or within cv::FilterEngine.

 Unlike cv::BaseRowFilter, cv::BaseColumnFilter may have some context information,
 i.e. box filter keeps the sliding sum of elements. To reset the state BaseColumnFilter::reset()
 must be called (e.g. the method is called by cv::FilterEngine)
 */
class BaseColumnFilter
{
public:
    //! the default constructor
    BaseColumnFilter();
    //! the destructor
    virtual ~BaseColumnFilter();
    //! the filtering operator. Must be overrided in the derived classes. The vertical border interpolation is done outside of the class.
    virtual void operator()(const uchar** src, uchar* dst, int dststep,
                            int dstcount, int width) = 0;
    //! resets the internal buffers, if any
    virtual void reset();
    int ksize, anchor;
};

/*!
 The Base Class for Non-Separable 2D Filters.

 This is the base class for linear or non-linear 2D filters.

 Several functions in OpenCV return Ptr<BaseFilter> for the specific types of filters,
 and those pointers can be used directly or within cv::FilterEngine.

 Similar to cv::BaseColumnFilter, the class may have some context information,
 that should be reset using BaseFilter::reset() method before processing the new array.
*/
class BaseFilter
{
public:
    //! the default constructor
    BaseFilter();
    //! the destructor
    virtual ~BaseFilter();
    //! the filtering operator. The horizontal and the vertical border interpolation is done outside of the class.
    virtual void operator()(const uchar** src, uchar* dst, int dststep,
                            int dstcount, int width, int cn) = 0;
    //! resets the internal buffers, if any
    virtual void reset();
    Size ksize;
    Point anchor;
};

/*!
 The Main Class for Image Filtering.

 The class can be used to apply an arbitrary filtering operation to an image.
 It contains all the necessary intermediate buffers, it computes extrapolated values
 of the "virtual" pixels outside of the image etc.
 Pointers to the initialized cv::FilterEngine instances
 are returned by various OpenCV functions, such as cv::createSeparableLinearFilter(),
 cv::createLinearFilter(), cv::createGaussianFilter(), cv::createDerivFilter(),
 cv::createBoxFilter() and cv::createMorphologyFilter().

 Using the class you can process large images by parts and build complex pipelines
 that include filtering as some of the stages. If all you need is to apply some pre-defined
 filtering operation, you may use cv::filter2D(), cv::erode(), cv::dilate() etc.
 functions that create FilterEngine internally.

 Here is the example on how to use the class to implement Laplacian operator, which is the sum of
 second-order derivatives. More complex variant for different types is implemented in cv::Laplacian().

 \code
 void laplace_f(const Mat& src, Mat& dst)
 {
     CV_Assert( src.type() == CV_32F );
     // make sure the destination array has the proper size and type
     dst.create(src.size(), src.type());

     // get the derivative and smooth kernels for d2I/dx2.
     // for d2I/dy2 we could use the same kernels, just swapped
     Mat kd, ks;
     getSobelKernels( kd, ks, 2, 0, ksize, false, ktype );

     // let's process 10 source rows at once
     int DELTA = std::min(10, src.rows);
     Ptr<FilterEngine> Fxx = createSeparableLinearFilter(src.type(),
     dst.type(), kd, ks, Point(-1,-1), 0, borderType, borderType, Scalar() );
     Ptr<FilterEngine> Fyy = createSeparableLinearFilter(src.type(),
     dst.type(), ks, kd, Point(-1,-1), 0, borderType, borderType, Scalar() );

     int y = Fxx->start(src), dsty = 0, dy = 0;
     Fyy->start(src);
     const uchar* sptr = src.data + y*src.step;

     // allocate the buffers for the spatial image derivatives;
     // the buffers need to have more than DELTA rows, because at the
     // last iteration the output may take max(kd.rows-1,ks.rows-1)
     // rows more than the input.
     Mat Ixx( DELTA + kd.rows - 1, src.cols, dst.type() );
     Mat Iyy( DELTA + kd.rows - 1, src.cols, dst.type() );

     // inside the loop we always pass DELTA rows to the filter
     // (note that the "proceed" method takes care of possibe overflow, since
     // it was given the actual image height in the "start" method)
     // on output we can get:
     //  * < DELTA rows (the initial buffer accumulation stage)
     //  * = DELTA rows (settled state in the middle)
     //  * > DELTA rows (then the input image is over, but we generate
     //                  "virtual" rows using the border mode and filter them)
     // this variable number of output rows is dy.
     // dsty is the current output row.
     // sptr is the pointer to the first input row in the portion to process
     for( ; dsty < dst.rows; sptr += DELTA*src.step, dsty += dy )
     {
         Fxx->proceed( sptr, (int)src.step, DELTA, Ixx.data, (int)Ixx.step );
         dy = Fyy->proceed( sptr, (int)src.step, DELTA, d2y.data, (int)Iyy.step );
         if( dy > 0 )
         {
             Mat dstripe = dst.rowRange(dsty, dsty + dy);
             add(Ixx.rowRange(0, dy), Iyy.rowRange(0, dy), dstripe);
         }
     }
 }
 \endcode
*/
class FilterEngine
{
public:
    //! the default constructor
    FilterEngine();
    //! the full constructor. Either _filter2D or both _rowFilter and _columnFilter must be non-empty.
    FilterEngine(BaseFilter* _filter2D,
                 BaseRowFilter* _rowFilter,
                 BaseColumnFilter* _columnFilter,
                 int srcType, int dstType, int bufType,
                 int _rowBorderType = BORDER_REPLICATE,
                 int _columnBorderType=-1,
                 const double _borderValue = 0);
    //! the destructor
    virtual ~FilterEngine();
    //! reinitializes the engine. The previously assigned filters are released.
    void init(BaseFilter* _filter2D,
              BaseRowFilter* _rowFilter,
              BaseColumnFilter* _columnFilter,
              int srcType, int dstType, int bufType,
              int _rowBorderType=BORDER_REPLICATE, int _columnBorderType=-1,
              double _borderValue = 0);
    //! starts filtering of the specified ROI of an image of size wholeSize.
    virtual int start(Size wholeSize, Rect roi, int maxBufRows=-1);
    //! starts filtering of the specified ROI of the specified image.
    virtual int start(SMat* src, Rect srcRoi = rect(0,0,-1,-1),
                      bool isolated=false, int maxBufRows=-1);
    //! processes the next srcCount rows of the image.
    virtual int proceed(uchar* src, int srcStep, int srcCount,
                        uchar* dst, int dstStep);
    //! applies filter to the specified ROI of the image. if srcRoi=(0,0,-1,-1), the whole image is filtered.
    virtual void apply( SMat* src, SMat* dst,
                        Rect srcRoi= rect(0,0,-1,-1),
                        Point dstOfs = point(0,0),
                        bool isolated=false);
    //! returns true if the filter is separable
    bool isSeparable() const { return (const BaseFilter*)filter2D == 0; }
    //! returns the number
    int remainingInputRows() const;
    int remainingOutputRows() const;

    int srcType, dstType, bufType;
    Size ksize;
    Point anchor;
    int maxWidth;
    Size wholeSize;
    Rect roi;
    int dx1, dx2;
    int rowBorderType, columnBorderType;
    std::vector<int> borderTab;
    int borderElemSize;
    std::vector<uchar> ringBuf;
    std::vector<uchar> srcRow;
    std::vector<uchar> constBorderValue;
    std::vector<uchar> constBorderRow;
    int bufStep, startY, startY0, endY, rowCount, dstY;
    std::vector<uchar*> rows;

    BaseFilter* filter2D;
    BaseRowFilter* rowFilter;
    BaseColumnFilter* columnFilter;
};

void dilate( SMat* src, SMat* dst, SMat* kernel,
				Point anchor, int iterations,
				int borderType, const double borderValue);

SMat* getStructuringElement(int shape, Size ksize, Point anchor);

typedef struct _ConvKernel
{
	int  nCols;
	int  nRows;
	int  anchorX;
	int  anchorY;
	int *values;
	int  nShiftR;
}ConvKernel;

ConvKernel *
	CreateStructuringElementEx( int cols, int rows,
	int anchorX, int anchorY,
	int shape, int *values );
