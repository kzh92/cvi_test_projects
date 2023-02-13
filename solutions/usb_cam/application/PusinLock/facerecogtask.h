#ifndef FACERECOG_TASK_H
#define FACERECOG_TASK_H

#include "msg.h"
#include "camerasurface.h"
#include "common_types.h"

#define FACE_TASK_FINISHED          1
#define FACE_TASK_DETECTED          2
#define FACE_TASK_REGISTER          3
#define FACE_TASK_UPDATE_CAM        4

#define FACE_RESULT_NONE 0
#define FACE_RESULT_SUCCESS 1
#define FACE_RESULT_FAILED 2
#define FACE_RESULT_TIMEOUT 3
#define FACE_RESULT_ENROLLED_FACE 4
#define FACE_RESULT_DUPLICATED_FACE 5
#define FACE_RESULT_SPOOF_FACE 6
#define FACE_RESULT_CAPTURED_FACE 7
#define FACE_RESULT_ENROLLED_NEXT 8
#define FACE_RESULT_FAILED_CAMERA 9
#define FACE_RESULT_DETECTED 10

#define FACE_REGISTER_NEXT          0
#define FACE_REGISTER_FACE_NORMAL   1
#define FACE_REGISTER_NO_FACE       2

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

    void    SendFaceDetectMsg(int isFaceOcculution, int iFaceNearFar, int iFacePosition, int isFaceDetected);

    static int m_iCounter;
    static int m_iImageMode;

    int     m_iRunning;
    int     m_iResult;
    int     m_iRecogIndex;
    int     m_iRecogID;
    int     m_iEyeOpened;

    int     m_iCmd;
    unsigned char *m_irOnData1;
    unsigned char *m_irOnData2;
    mythread_ptr m_thread;
};

extern message_queue g_queue_face;

#endif // WATCHTASK_H
