
#ifndef POSE_H__INCLUDED
#define POSE_H__INCLUDED

void getPose(float rFaceLeft, float rFaceTop, float rFaceRate, float rFacePan, float *pModelPoints, int nModelPointNum, float &rRotateX, float &rRotateY, float &rRotateZ);
void getPose68(float rFaceLeft, float rFaceTop, float rFaceSize, float *pModelPoints, int nModelPointNum, float &rRotateX, float &rRotateY, float &rRotateZ);

#endif //POSE_H__INCLUDED


