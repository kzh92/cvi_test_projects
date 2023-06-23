#ifndef FACERECOG_TASK_H
#define FACERECOG_TASK_H

#include "msg.h"
#include "camerasurface.h"
#include "common_types.h"

#define FACE_TASK_FINISHED          1
#define FACE_TASK_DETECTED          2
#define FACE_TASK_REGISTER          3
#define FACE_TASK_UPDATE_CAM        4

enum {
    FACE_RESULT_NONE,
    FACE_RESULT_SUCCESS,
    FACE_RESULT_FAILED,
    FACE_RESULT_TIMEOUT,
    FACE_RESULT_ENROLLED_FACE,
    FACE_RESULT_DUPLICATED_FACE,
    FACE_RESULT_SPOOF_FACE,
    FACE_RESULT_CAPTURED_FACE,
    FACE_RESULT_CAPTURED_FACE_FAILED,
    FACE_RESULT_ENROLLED_NEXT,
    FACE_RESULT_FAILED_CAMERA,
    FACE_RESULT_DETECTED,
    HAND_RESULT_SUCCESS,
    HAND_RESULT_ENROLLED,
    HAND_RESULT_ENROLL_DUPLICATED,
    HAND_RESULT_FAILED,
    HAND_RESULT_ENROLLED_NEXT,
    HAND_RESULT_DETECTED,
};

#define FACE_REGISTER_NEXT          0
#define FACE_REGISTER_FACE_NORMAL   1
#define FACE_REGISTER_NO_FACE       2

//face detection state,
typedef enum {
    FDS_NONE = 0x0,
    FDS_FACE_DETECTED = 0x01,
    FDS_HAND_DETECTED = 0x02,
} e_face_detection_states;

//face recognition mode index
typedef enum {
    FMI_FACE,
    FMI_HAND,
    FMI_END
} e_face_recog_modes;

class FaceRecogTask
{
public:
    FaceRecogTask();
    ~FaceRecogTask();

    enum{ E_VERIFY, E_REGISTER, E_GET_IMAGE, E_EYE_CHECK };

    void    Start(int iCmd = 0);
    void    Stop();
    void    Pause();
    void    ThreadProc();
    void    Wait();

    int     GetResult(){return m_iResult;}
    int     GetRecogIndex(){return m_iRecogIndex;}
    int     GetRecogID() {return m_iRecogID;}
    int     GetCounter() {return m_iCounter;}
    int     GetEyeOpened() {return m_iEyeOpened;}

    void    SendMsg(int type, int data1, int data2, int data3);

    int     IsRunning(){return m_iRunning;}
protected:
    void    run();

    void    SendFaceDetectMsg();
    int     ProcessGetImage1Step(int iFrameCount);
    int     ProcessCheckCamera1Step();
    int     ProcessVerify1Step(int iSecondImageReCheck);
    int     ProcessEnroll1Step(int iSecondImageReCheck);
    int     ProcessEnrollFile1Step();
    int     ProcessSaveFaceImage1Step();
    int     ProcessCheckEye1Step();
    int     ReadStaticIRImage(void* dst, int flip);
    int     GetLeftIrFrame(int* p_iUseFirstFrame);
    int     GetRightIrFrame(void* pBuffer, int iUseFirstFrame);
    int     GetFaceState();

    static int m_iCounter;
    static int m_iImageMode;

    unsigned char m_iRunning;
    unsigned char m_iCmd;
    unsigned char m_iCaptureCount;
    unsigned char m_iHasIrError;
    unsigned char m_isFaceOcculution;
    unsigned char m_iFaceNearFar[FMI_END];
    unsigned char m_iFacePosition[FMI_END];
    unsigned char m_isFaceDetected[FMI_END];
    unsigned char m_iLastDetMode;   //last recognition mode
    float m_rDetectTime[FMI_END];
    float   m_rFaceFailTime;
    float   m_rStartTime;
    int     m_iResult;
    int     m_iRecogIndex;
    int     m_iRecogID;
    int     m_iEyeOpened;

    mythread_ptr m_thread;
};

extern message_queue g_queue_face;

#endif // WATCHTASK_H
