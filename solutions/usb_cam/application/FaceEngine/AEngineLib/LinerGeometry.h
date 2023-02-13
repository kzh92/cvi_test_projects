#ifndef _LINER_GEOMETRY_H_
#define _LINER_GEOMETRY_H_

//L1:(X1, Y1),(X2, Y2). Point:(PointX, PointY) 
bool isPointAboveLineByPoint(float rX1, float rY1, float rX2, float rY2, float rPointX, float rPointY);

//L1: Ax + By + c. Point:(PointX, PointY)
bool isPointAboveLineByEqual(float rA, float rB, float rC, float rPointX, float rPointY);

//LineSegments Points
bool isPointAboveLineSegments(float* prXY, int nPointNum, float rPointX, float rPointY);

//LineSegment intersection check
//prXYs:Line Segment gruop, prSegment2 compare segment
int LineSegmentIntersectionsCount(float* prXYs, int nPointNum, float *prSegment2);


bool getIntersectionPointYinLineSegments(float nIntersectionX, float& nInterasectionY, float* prXY, int nPointNum);

float getAngleBetweenTwoVector(float* rVector1, float *rVector2, int nVecSize);


#endif // !_LINER_GEOMETRY_H_
