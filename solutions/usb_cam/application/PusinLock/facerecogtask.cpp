#include "facerecogtask.h"
#include "drv_gpio.h"
#include "appdef.h"
#include "shared.h"
// #include "msg.h"
#include "DBManager.h"
// #include "msg.h"
#include "engineparam.h"
#include "ImageProcessing.h"
#include "FaceRetrievalSystem.h"
#include "settings.h"
#include "camerasurface.h"
#include "senselockmessage.h"
// #include "removebackground.h"
// #include "countbase.h"
#include "faceengine.h"
// #include "i2cbase.h"
#include "jpge.h"
// #include "shared.h"
#include "check_camera_pattern.h"
#include "sn.h"

// #include <unistd.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <time.h>
#include <string.h>
// #include <math.h>
#include <fcntl.h>
#include <errno.h>

//#define USE_STATIC_IMAGE

#define CAPTURE_WIDTH   (360)
#define CAPTURE_HEIGHT  (640)

unsigned char *g_abCapturedFace = NULL;

int FaceRecogTask::m_iCounter = 0;
int FaceRecogTask::m_iImageMode = 0;

#define N_MAX_SAVE_IMG_NUM      SI_MAX_IMAGE_COUNT
int SaveImage(unsigned char* pbImage, int iSaveIdx, int iRotate);
void* faceRecogTask_ThreadProc1(void* param);

message_queue g_queue_face;

extern void EndIns();

FaceRecogTask::FaceRecogTask()
{
    m_iRunning = 0;
    m_iRecogID = 0;
    m_iRecogIndex = 0;
    m_iEyeOpened = 0;
}

FaceRecogTask::~FaceRecogTask()
{

}

void FaceRecogTask::Start(int iCmd)
{
    m_iResult = FACE_RESULT_NONE;
    m_iCounter ++;
    m_iRecogIndex = -1;
    m_iRecogID = 0;
    m_iEyeOpened = 0;
    ResetDetectTimeout();

    //added by KSB 20180711
    g_xSS.rMainLoopTime = Now();
    m_iRunning = 1;

    m_iCmd = iCmd;
    if(m_iCmd == E_VERIFY)
        FaceEngine::VerifyInit(g_xSS.iVerifyMode, g_xSS.iDemoMode);
    else if(m_iCmd == E_GET_IMAGE)
        FaceEngine::VerifyInit(0, 0);
    else if(m_iCmd == E_EYE_CHECK)
    {

    }

    message_queue_init(&g_queue_face, sizeof(MSG), MAX_MSG_NUM);

#if 1
    if(my_thread_create_ext(&m_thread, NULL, faceRecogTask_ThreadProc1, this, (char*)"fatk", 8192, MYTHREAD_PRIORITY_HIGH))
        my_printf("[FRTask]create thread error.\n");
#else
    run();
#endif
}

void FaceRecogTask::Stop()
{
    m_iRunning = 0;

    WaitIRCancel();

    Wait();

    message_queue_destroy(&g_queue_face);
}

void FaceRecogTask::Wait()
{
    if (m_thread != NULL)
    {
        my_thread_join(&m_thread);
        m_thread = NULL;
    }
}

void FaceRecogTask::Pause()
{
    m_iRunning = 0;

    WaitIRCancel();
}

void FaceRecogTask::SendMsg(int type, int data1, int data2, int data3)
{
    MSG* pxMsg = (MSG*)message_queue_message_alloc(&g_queue_face);
    memset(pxMsg, 0, sizeof(MSG));
    pxMsg->type = type;
    pxMsg->data1 = data1;
    pxMsg->data2 = data2;
    pxMsg->data3 = data3;

    message_queue_write(&g_queue_face, pxMsg);
}

void FaceRecogTask::SendFaceDetectMsg(int isFaceOcculution, int iFaceNearFar, int iFacePosition, int isFaceDetected)
{
    if(isFaceOcculution)
    {
        g_xSS.note_data_face.state = FACE_STATE_FACE_OCCLUSION;
        SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, isFaceDetected, m_iCounter);
    }
    else if(iFaceNearFar)
    {
        if(iFaceNearFar == 1)
            g_xSS.note_data_face.state = FACE_STATE_TOOCLOSE;
        else if(iFaceNearFar == 2)
            g_xSS.note_data_face.state = FACE_STATE_TOOFAR;
        SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, isFaceDetected, m_iCounter);
    }
    else if(iFacePosition)
    {
        if(iFacePosition == 1)
            g_xSS.note_data_face.state = FACE_STATE_TOOLEFT;
        else if(iFacePosition == 2)
            g_xSS.note_data_face.state = FACE_STATE_TOORIGHT;
        else if(iFacePosition == 3)
            g_xSS.note_data_face.state = FACE_STATE_TOOUP;
        else if(iFacePosition == 4)
            g_xSS.note_data_face.state = FACE_STATE_TOODOWN;
        SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, isFaceDetected, m_iCounter);
    }
    else if(isFaceDetected)
    {
        g_xSS.note_data_face.state = FACE_STATE_NORMAL;
        SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, isFaceDetected, m_iCounter);
    }
    else
    {
        g_xSS.note_data_face.state = FACE_STATE_NOFACE;
        SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, isFaceDetected, m_iCounter);
    }
}

void FaceRecogTask::run()
{
    int iLoopCount = 0;
    float rFailedDetectTime = 0;
    float arEngineResult[10] = { 0 };
    int iTimeout = DETECTION_TIMEOUT;
    int iCaptureCount = 0;

    m_irOnData1 = (unsigned char*)my_malloc(IR_BUFFER_SIZE);
    m_irOnData2 = (unsigned char*)my_malloc(IR_BUFFER_SIZE);
    if (m_irOnData1 == NULL || m_irOnData2 == NULL)
    {
        my_printf("failed to malloc, ft. %p, %p\n", m_irOnData1, m_irOnData2);
        SendMsg(MSG_RECOG_FACE, FACE_TASK_FINISHED, 0, m_iCounter);
        return;
    }

    g_abCapturedFace = (unsigned char*)my_malloc(CAPTURE_WIDTH * CAPTURE_HEIGHT * 3);

    if(m_iCmd == E_REGISTER)
        iTimeout = g_xSS.msg_enroll_itg_data.timeout;
    else if(m_iCmd == E_VERIFY)
    {
        iTimeout = g_xSS.msg_verify_data.timeout;
    }
    else if(m_iCmd == E_EYE_CHECK)
        iTimeout = 2;

    if(iTimeout == 0)
        iTimeout = DETECTION_TIMEOUT;

    dbug_printf("Timeout: %d\n", iTimeout);

    if(g_xSS.iRestoreRootfs)
    {
        iTimeout = 30;
        //StopCamSurface();
    }

    memset(&g_xSS.xFaceRect, 0, sizeof(g_xSS.xFaceRect));

    float rFaceFailTime = 0;
    float rStartTime = Now();
    int hasIrError = 0;
#if (NOTE_INTERVAL_MS)
    g_xSS.note_data_face.state = FACE_STATE_NOFACE;
#endif
    while(m_iRunning)
    {
        if(g_xSS.iResetFlag == 1)
            break;

        iLoopCount ++;
        g_xSS.rFaceEngineTime = Now();

#if 0
        if(g_xSS.iRestoreRootfs)
        {
            if(g_xSS.iRestoreRootfs == 2)
            {
                m_iResult = FACE_RESULT_TIMEOUT;
                break;
            }

            g_xSS.note_data_face.state = FACE_STATE_NOFACE;
            SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, 0, m_iCounter);

            my_usleep(500 * 1000);
            my_printf("R: %f\n", Now());
            continue;
        }
#endif

        if(g_iMipiCamInited == -1)
        {            
            if(!((g_iFirstCamFlag & LEFT_IR_CAM_RECVED) && (g_iFirstCamFlag & RIGHT_IR_CAM_RECVED)))
            {
                my_usleep(30 * 1000);

#if (FM_PROTOCOL == FM_EASEN)
                if(Now() - g_rLastDetectTime > iTimeout * 1000)
                {
                    m_iResult = FACE_RESULT_NONE;
                    break;
                }
                continue;
#elif (FM_PROTOCOL == FM_DESMAN)
                m_iResult = FACE_RESULT_FAILED_CAMERA;
                break;
#endif
            }
        }

        ///적외선켜기지령을 내려보낸후 Led On/Off화상을 얻는다.
#if 1
        int iFlag = 0;
        if((g_iFirstCamFlag & LEFT_IR_CAM_RECVED) && !(g_iFirstCamFlag & FIRST_IR_PROCESSED))
        {
            //???? ?? ???????? ???? ??????? ?? ????.
            g_iFirstCamFlag |= FIRST_IR_PROCESSED;

            fr_InitIRCamera_ExpGain();

            iFlag = 1;
        }
        else
        {
            //
            for(int i = 0; i < 10; i ++)
            {
                if(g_iTwoCamFlag == -1)
                    break;

                if(g_xSS.iResetFlag == 1)
                    break;

                my_usleep(10 * 1000);
            }
            g_iTwoCamFlag = 0;
#if (IR_LED_ONOFF_MODE == 1)
            camera_set_irled_on(1);
#endif
            float rOldTime = Now();
            dbug_printf("--------- before wait %0.3f\n", Now());
            while(Now() - rOldTime < 500)
            {
                int ret = WaitIRTimeout(10);
                if (ret != ETIMEDOUT)
                    break;
                if (g_xSS.iResetFlag == 1)
                    break;
            }
            dbug_printf("--------- after wait %0.3f\n", Now());

            if(!m_iRunning)
                break;
        }

#if (NOTE_INTERVAL_MS)
        if(g_xSS.note_data_face.state == FACE_STATE_NOFACE)
            SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, 0, m_iCounter);
        else if(g_xSS.note_data_face.state == FACE_STATE_FACE_OCCLUSION)
            SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, 0, m_iCounter);
        else if(g_xSS.note_data_face.state == FACE_STATE_TOOFAR)
            SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, 0, m_iCounter);
        else if(g_xSS.note_data_face.state == FACE_STATE_TOOCLOSE)
            SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, 0, m_iCounter);
#endif // NOTE_INTERVAL_MS

        if(!m_iRunning)
            break;

        if(m_iCmd == E_GET_IMAGE)
        {
            if(iFlag == 1)
            {
                EndIns();
                StartCamSurface(1);
            }
            lockIRBuffer();
            fr_CalcScreenValue(g_irOnData1, IR_SCREEN_GETIMAGE_MODE);
            unlockIRBuffer();
            CalcNextExposure();
            my_usleep(30*1000);
            if (iLoopCount >= 3)
            {
                unsigned char* pbTmp = (unsigned char*)my_malloc(WIDTH_1280*HEIGHT_720);
                if(iCaptureCount < g_xSS.msg_snap_image_data.image_counts &&
                        g_xSS.msg_snap_image_data.start_number + iCaptureCount <= N_MAX_SAVE_IMG_NUM)
                {
                    lockIRBuffer();
                    memcpy(pbTmp, g_irOnData1, IR_BUFFER_SIZE);
                    unlockIRBuffer();
                    gammaCorrection_screen(pbTmp, WIDTH_1280, HEIGHT_720);
                    SaveImage(pbTmp, g_xSS.msg_snap_image_data.start_number + iCaptureCount, g_xSS.iCameraRotate == 0 ? 270: 90);
                    iCaptureCount ++;
                }

                if(iCaptureCount < g_xSS.msg_snap_image_data.image_counts &&
                        g_xSS.msg_snap_image_data.start_number + iCaptureCount <= N_MAX_SAVE_IMG_NUM)
                {
                    lockIRBuffer();
                    memcpy(pbTmp, g_irOnData2, IR_BUFFER_SIZE);
                    unlockIRBuffer();
                    gammaCorrection_screen(pbTmp, WIDTH_1280, HEIGHT_720);
                    SaveImage(pbTmp, g_xSS.msg_snap_image_data.start_number + iCaptureCount, g_xSS.iCameraRotate == 0 ? 90 : 270);
                    iCaptureCount ++;
                }
                my_free(pbTmp);

                if(!(iCaptureCount < g_xSS.msg_snap_image_data.image_counts &&
                        g_xSS.msg_snap_image_data.start_number + iCaptureCount <= N_MAX_SAVE_IMG_NUM))
                {
                    m_iResult = FACE_RESULT_CAPTURED_FACE;
                    break;
                }
            }
            // iLoopCount ++;
            continue;
        }

        if(g_xSS.iResetFlag == 1)
            break;

        ///얼굴검출을 진행한다.
        int isFaceDetected = 0;
        int isFaceOcculution = 0;
        int iFaceNearFar = 0;
        int iFacePosition = 0;
        int iVerifyFlag = 1;
        int iSecondImageReCheck = 0;

        //my_printf("E: %f\n", Now());
#if (USE_VDBTASK)
        g_iCapturedClrFlag = CLR_CAPTURE_START;
#endif // USE_VDBTASK
#ifndef USE_STATIC_IMAGE
        if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE)
#endif // !USE_STATIC_IMAGE
        {
            lockIRBuffer();
            memcpy(m_irOnData1, g_irOnData1, IR_BUFFER_SIZE);
            memcpy(m_irOnData2, g_irOnData2, IR_BUFFER_SIZE);
            unlockIRBuffer();
            //check camera
            int res1 = 0, res2 = 0;
            myfdesc_ptr fd = NULL;
#ifndef USE_STATIC_IMAGE
            if (m_iCmd == E_VERIFY)
#endif // !USE_STATIC_IMAGE
            {
#ifndef USE_STATIC_IMAGE
                res1 = checkCameraPattern(m_irOnData1);
                res2 = checkCameraPattern(m_irOnData2);
                my_printf("** check camera=%d,%d, %0.3f\n", res1, res2, Now());
                if (res1 != 0 || res2 != 0)
                {
                    if (hasIrError == 1)
                    {
                        if (res1 == CAMERA_ERROR || res2 == CAMERA_ERROR)
                            g_xSS.iCamError |= CAM_ERROR_IR_PATTERN;
                        else
                            g_xSS.iCamError |= CAM_ERROR_IR_PATTERN_2;
                    }
                }

                if (res1 != 0 && hasIrError)
                {
                    float _tmpTime = Now();
                    SaveImage(m_irOnData1, 1, g_xSS.iCameraRotate == 0 ? 270: 90);
                    if (g_xSS.bSnapImageData != NULL)
                        my_flash_write(IR_ERROR_SAVE_ADDR, (unsigned int)g_xSS.bSnapImageData, g_xSS.iSnapImageLen[0]);
                    my_printf("write ir1=%d, jpg %0.3f\n", g_xSS.iSnapImageLen[0], Now() - _tmpTime);
                }
#endif // !USE_STATIC_IMAGE
                if (res2 != 0 && hasIrError)
                {
                    float _tmpTime = Now();
                    SaveImage(m_irOnData2, 2, g_xSS.iCameraRotate == 0 ? 270: 90);
                    if (g_xSS.bSnapImageData != NULL)
                        my_flash_write(IR_ERROR_SAVE_ADDR + SI_MAX_IMAGE_SIZE, (unsigned int)g_xSS.bSnapImageData + SI_MAX_IMAGE_SIZE, g_xSS.iSnapImageLen[1]);
                    my_printf("write ir2=%d, jpg %0.3f\n", g_xSS.iSnapImageLen[1], Now() - _tmpTime);
                }
            }
            if ((res1 == 0 && res2 == 0) || hasIrError == 1)
            {
                my_mount_misc();
                //read static image.
                fd = my_open(FN_FACE_BIN, O_RDONLY, 0);
                if (is_myfdesc_ptr_valid(fd))
                {
                    my_read(fd, m_irOnData1, HEIGHT_720*WIDTH_1280);
                    my_close(fd);
                    for (int x = 0; x < WIDTH_1280; x++)
                        for (int y = 0; y < HEIGHT_720; y++)
                        {
                            m_irOnData2[(WIDTH_1280 - x - 1) + WIDTH_1280 * (HEIGHT_720 - y - 1)] = m_irOnData1[x + y*WIDTH_1280];
                        }
                    dbug_printf("-------------------- read static image ok.\n");
                }
                else
                {
                    dbug_printf("-------------------- read static image fail.\n");
                }
            }
            else
            {
                memset(m_irOnData1, 0, IR_BUFFER_SIZE);
                memset(m_irOnData2, 0, IR_BUFFER_SIZE);
                hasIrError = 1;
            }
        }
#ifndef USE_STATIC_IMAGE
        else
        {
            lockIRBuffer();
            memcpy(m_irOnData1, g_irOnData1, IR_BUFFER_SIZE);
            unlockIRBuffer();
        }
#endif // !USE_STATIC_IMAGE

        FaceEngine::ExtractFace(NULL, m_irOnData1, arEngineResult);

        if(g_xSS.iResetFlag == 1)
            break;

        if(arEngineResult[0] == ES_FAILED)
            iVerifyFlag = 0;

        if(iFlag == 1)
            EndIns();
#ifndef USE_STATIC_IMAGE
        if (g_xSS.iDemoMode != N_DEMO_FACTORY_MODE)
        {
            lockIRBuffer();
            memcpy(m_irOnData2, g_irOnData2, IR_BUFFER_SIZE);
            unlockIRBuffer();
        }
#endif // !USE_STATIC_IMAGE

        fr_PreExtractFace2(m_irOnData2);
        iSecondImageReCheck = fr_GetSecondImageNeedReCheck();
        int iNeedExp = fr_GetNeedToCalcNextExposure();

        ///카메라조종에 얼굴검출정보를 넘겨준다.
//        g_nFaceRectValid = 0;
//        if(fr_GetFaceDetected())
//            g_nFaceRectValid = *fr_GetFaceDetected();

        if(iFlag == 0)
        {
            fr_calc_Off(g_irOffData);
        }

        if(iNeedExp)
        {            
            CalcNextExposure();
        //my_printf("CalcNextExposure: %f\n", Now());
        }
#endif

        memset(&g_xSS.note_data_face, 0, sizeof(g_xSS.note_data_face));
        if(iVerifyFlag == 1)
        {
            int iNext = fr_ExtractFace();

            if(g_xSS.iResetFlag == 1)
                break;

            if(iNext == ES_FAILED)
            {
                iVerifyFlag = 0;

                SEngineResult* pxEngineResult =  fr_GetEngineResult();
                SEngineParam* pxEngineParam = fr_GetEngineParam();
                if(pxEngineResult && pxEngineParam)
                {
                    isFaceOcculution = pxEngineResult->nOcclusion;
                    iFaceNearFar = pxEngineResult->nFaceNearFar;
                    iFacePosition = pxEngineResult->nFacePosition;
                }

                memset(&g_xSS.xFaceRect, 0, sizeof(g_xSS.xFaceRect));
                memset(&g_xSS.note_data_face, 0, sizeof(g_xSS.note_data_face));
            }
            else
            {
                if(iSecondImageReCheck)
                {
                    camera_set_irled_on(1);
                    dbug_printf("irled on, %s:%d\n", __FILE__, __LINE__);
                }
                isFaceDetected = 1;

                //added by KSB 20180711
                //reset recognition mode to face
#if 0
                g_xSS.xFaceRect.x = (int)arEngineResult[2];
                g_xSS.xFaceRect.y = (int)arEngineResult[3];
                g_xSS.xFaceRect.width = (int)arEngineResult[4];
                g_xSS.xFaceRect.height = (int)arEngineResult[5];
#else
                SEngineResult* pxEngineResult =  fr_GetEngineResult();
                SEngineParam* pxEngineParam = fr_GetEngineParam();
                if(pxEngineResult && pxEngineParam)
                {
                    g_xSS.xFaceRect.x = ((float)pxEngineResult->xFaceRect.x + pxEngineParam->nOffsetX) / 2;
                    g_xSS.xFaceRect.y = ((float)pxEngineResult->xFaceRect.y + pxEngineParam->nOffsetY) / 2;
                    g_xSS.xFaceRect.width = ((float)pxEngineResult->xFaceRect.width) / 2;
                    g_xSS.xFaceRect.height = ((float)pxEngineResult->xFaceRect.height) / 2;

                    g_xSS.note_data_face.left = g_xSS.xFaceRect.x;
                    g_xSS.note_data_face.top = g_xSS.xFaceRect.y;
                    g_xSS.note_data_face.right = g_xSS.xFaceRect.x + g_xSS.xFaceRect.width - 1;
                    g_xSS.note_data_face.bottom = g_xSS.xFaceRect.y + g_xSS.xFaceRect.height - 1;
                }
#endif

#if (FM_PROTOCOL == FM_EASEN)
                ResetDetectTimeout();
#endif

                if (rFaceFailTime == 0)
                    rFaceFailTime = Now();

                if(rFailedDetectTime == 0)
                    rFailedDetectTime = g_xSS.rFaceEngineTime;

                if(isFaceOcculution == 1)
                    rFailedDetectTime = 0;
            }
        }

        dbug_printf("Face: x=%d,y=%d,width=%d,heigh=%d, %f\n", g_xSS.xFaceRect.x, g_xSS.xFaceRect.y, g_xSS.xFaceRect.width, g_xSS.xFaceRect.height, Now());

        if(Now() - rStartTime > (iTimeout * 1000))
        {
            m_iResult = FACE_RESULT_TIMEOUT;
            break;
        }

        if(!m_iRunning)
            break;

        if(g_xSS.iResetFlag == 1)
            break;

        if(m_iCmd == E_REGISTER)
        {
            FaceEngine::RegisterFace(arEngineResult, g_xSS.msg_enroll_itg_data.face_direction);

            if(g_xSS.iResetFlag == 1)
                break;

            if((arEngineResult[0] == ES_SUCCESS || arEngineResult[0] == ES_ENEXT) && iSecondImageReCheck)
            {
                //register time will be greater than 300ms
                if (g_xSS.iDemoMode != N_DEMO_FACTORY_MODE)
                {
                    lockIRBuffer();
                    memcpy(m_irOnData1, g_irOnData1, IR_BUFFER_SIZE);
                    memcpy(m_irOnData2, g_irOnData2, IR_BUFFER_SIZE);
                    unlockIRBuffer();
                }

                int iCheck = fr_CheckFaceInSecondImage(m_irOnData1, m_irOnData2);
                if(iCheck != ES_SUCCESS)
                {
                    arEngineResult[0] = ES_PROCESS;
                }
            }

            if(arEngineResult[0] != ES_PROCESS)
            {
//                my_printf("Register: %d\n", (int)arEngineResult[0]);
                if(arEngineResult[0] == ES_ENEXT)
                    SendMsg(MSG_RECOG_FACE, FACE_TASK_REGISTER, FACE_REGISTER_NEXT, m_iCounter);

                if(arEngineResult[0] == ES_SUCCESS)
                {
                    my_usleep(20*1000);
                    m_iResult = FACE_RESULT_ENROLLED_FACE;
                    break;
                }
                else if(arEngineResult[0] == ES_DUPLICATED)
                {
                    my_usleep(20*1000);
                    m_iResult = FACE_RESULT_DUPLICATED_FACE;
                    break;
                }
                else if(arEngineResult[0] == ES_SPOOF_FACE)
                {
                    my_usleep(20*1000);
                    m_iResult = FACE_RESULT_SPOOF_FACE;
                    break;
                }
#if (FM_PROTOCOL == FM_DESMAN)
                if(arEngineResult[0] == ES_ENEXT)
                {
                    SendFaceDetectMsg(isFaceOcculution, iFaceNearFar, iFacePosition, isFaceDetected);
                    my_usleep(20*1000);
                    m_iResult = FACE_RESULT_ENROLLED_NEXT;
                    break;
                }
                else if(arEngineResult[0] == ES_DIRECTION_ERROR)
                {
                    g_xSS.note_data_face.state = FACE_STATE_DIRECTION_ERROR;
                    SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, isFaceDetected, m_iCounter);
                }
#endif
            }
            else
            {
                SendFaceDetectMsg(isFaceOcculution, iFaceNearFar, iFacePosition, isFaceDetected);
                my_usleep(20*1000);
            }
        }
        else if(m_iCmd == E_VERIFY)
        {
            SendFaceDetectMsg(isFaceOcculution, iFaceNearFar, iFacePosition, isFaceDetected);
            my_usleep(10*1000);
            if(g_xSS.iResetFlag == 1)
                break;

#if (USE_DEMOMODE2)
            if (g_xSS.iDemoMode == N_DEMO_VERIFY_MODE_OFF &&
                    arEngineResult[0] == ES_SUCCESS &&
                    dbm_GetUserCount() == 0 &&
                    isFaceDetected)
            {
                m_iResult = FACE_RESULT_DETECTED;
                break;
            }
#endif // USE_DEMOMODE2

            int iFindIndex = FaceEngine::VerifyFace(arEngineResult);

            if(g_xSS.iResetFlag == 1)
                break;

            if((arEngineResult[0] == ES_SUCCESS || arEngineResult[0] == ES_UPDATE) && iSecondImageReCheck)
            {
                //verify time will be greater than 300ms
                if (g_xSS.iDemoMode != N_DEMO_FACTORY_MODE)
                {
                    lockIRBuffer();
                    memcpy(m_irOnData1, g_irOnData1, IR_BUFFER_SIZE);
                    memcpy(m_irOnData2, g_irOnData2, IR_BUFFER_SIZE);
                    unlockIRBuffer();
                }

                int iCheck = fr_CheckFaceInSecondImage(m_irOnData1, m_irOnData2);
                if(iCheck != ES_SUCCESS)
                {
                    arEngineResult[0] = ES_PROCESS;
                }
            }

            if(arEngineResult[0] == ES_PROCESS && rFaceFailTime != 0)
            {
                if(Now() - rFaceFailTime >= N_MAX_FAILED_TIME * 1000)
                    arEngineResult[0] = ES_FAILED;
            }


            if(arEngineResult[0] != ES_PROCESS)
            {
                if(arEngineResult[0] != ES_FAILED)
                {
                    dbug_printf("Face Recog Success! %d, %f\n", iFindIndex, Now());
                    m_iResult = FACE_RESULT_SUCCESS;
                    m_iEyeOpened = arEngineResult[6];
                    m_iRecogIndex = iFindIndex;

                    if(!m_iEyeOpened)
                    {
                        m_iCmd = E_EYE_CHECK;
                        rStartTime = Now();
                    }
                    else
                        break;
                }
                else
                {
                    dbug_printf("Face Recog Failed!\n");
                    m_iResult = FACE_RESULT_FAILED;
                    break;
                }
            }
        }
        else if(m_iCmd == E_EYE_CHECK)
        {
            if(isFaceDetected)
            {
                if(g_xSS.iResetFlag == 1)
                    break;

                FaceEngine::VerifyFace(arEngineResult);

                if(g_xSS.iResetFlag == 1)
                    break;

                if(arEngineResult[6] == 1)
                {
                    //eye_close_open
                    g_xSS.note_data_eye.eye_state = FACE_STATE_EYE_CLOSE_STATUS_OPEN_EYE;
                    m_iEyeOpened = arEngineResult[6];
//                    m_iResult = FACE_RESULT_TIMEOUT;
                    break;
                }
                else if(Now() - rStartTime > 1000)
                {
                    //eye_close
                    g_xSS.note_data_eye.eye_state = FACE_STATE_EYE_CLOSE_STATUS;
                    m_iEyeOpened = 0;
//                    m_iResult = FACE_RESULT_TIMEOUT;
                    break;
                }
            }
            else if(Now() - rStartTime > 1000)
            {
                //eye unkown
                g_xSS.note_data_eye.eye_state = FACE_STATE_EYE_CLOSE_STATUS;
                m_iEyeOpened = 0;
//                m_iResult = FACE_RESULT_TIMEOUT;
                break;
            }
        }
        else if(m_iCmd == E_GET_IMAGE)
        {            
            if((isFaceDetected && g_xSS.iFaceImage) || (g_xSS.iFaceImage == 0))
            {
#if (FM_PROTOCOL == FM_EASEN)
                if((m_iImageMode % 2) == 0)
                {
                    rotateImage_inner(m_irOnData1, WIDTH_1280, HEIGHT_720, g_xSS.iCameraRotate == 0 ? 270: 90);
                    Shrink_Grey(m_irOnData1, WIDTH_1280, HEIGHT_720, g_abCapturedFace, WIDTH_1280 / 4, HEIGHT_720 / 4);
                }
                else
                {
                    rotateImage_inner(m_irOnData2, WIDTH_1280, HEIGHT_720, g_xSS.iCameraRotate == 0 ? 90 : 270);
                    Shrink_Grey(m_irOnData2, WIDTH_1280, HEIGHT_720, g_abCapturedFace, WIDTH_1280 / 4, HEIGHT_720 / 4);
                }

                m_iResult = FACE_RESULT_CAPTURED_FACE;
                m_iImageMode ++;
                break;
#elif (FM_PROTOCOL == FM_DESMAN)
                if(iCaptureCount < g_xSS.msg_snap_image_data.image_counts &&
                        g_xSS.msg_snap_image_data.start_number + iCaptureCount <= N_MAX_SAVE_IMG_NUM)
                {
                    SaveImage(m_irOnData1, g_xSS.msg_snap_image_data.start_number + iCaptureCount, g_xSS.iCameraRotate == 0 ? 270: 90);
                    iCaptureCount ++;
                }

                if(iCaptureCount < g_xSS.msg_snap_image_data.image_counts &&
                        g_xSS.msg_snap_image_data.start_number + iCaptureCount <= N_MAX_SAVE_IMG_NUM)
                {
                    SaveImage(m_irOnData2, g_xSS.msg_snap_image_data.start_number + iCaptureCount, g_xSS.iCameraRotate == 0 ? 90 : 270);
                    iCaptureCount ++;
                }

                if(!(iCaptureCount < g_xSS.msg_snap_image_data.image_counts &&
                        g_xSS.msg_snap_image_data.start_number + iCaptureCount <= N_MAX_SAVE_IMG_NUM))
                {
                    m_iResult = FACE_RESULT_CAPTURED_FACE;
                    break;
                }
#endif
            }
        }

        if(iFlag)
        {
#if (IR_LED_ONOFF_MODE == 1 && 0)
            fr_calc_Off(g_irOffData);
#endif
            StartCamSurface(1);
            if(iNeedExp)
            {
                my_usleep(70 * 1000);
            }

            //CalcNextExposure();
        }

#if 0
#if (IR_LED_ONOFF_MODE == 1)
        ///얼굴검출에서 실패하였으면 800ms지연준다.
        if(isFaceDetected == 0)
        {
            if(iLoopCount <= 1)
                my_usleep(100 * 1000);
            else
                my_usleep(100 * 1000);
        }
#endif
#endif
        // iLoopCount ++;
    }

    memset(&g_xSS.xFaceRect, 0, sizeof(g_xSS.xFaceRect));

#ifdef PROTECT_ENGINE
    unsigned char _tmp_buf_cs[sizeof(g_xCS)];
    unsigned char _tmp_buf_hd2[sizeof(g_xHD2)];
    memcpy(_tmp_buf_cs, &g_xCS, sizeof(g_xCS));
    memcpy(_tmp_buf_hd2, &g_xHD2, sizeof(g_xHD2));
    fr_setSuitableThreshold(&g_xHD, &g_xCS, &g_xHD2);
    if (memcmp(_tmp_buf_cs, &g_xCS, sizeof(g_xCS)))
        UpdateCommonSettings();
    if (memcmp(_tmp_buf_hd2, &g_xHD2, sizeof(g_xHD2)))
        UpdateHeadInfos2();
#endif // PROTECT_ENGINE

    SendMsg(MSG_RECOG_FACE, FACE_TASK_FINISHED, 0, m_iCounter);
    dbug_printf("[Recog] Verify Face End!\n");
    g_iFirstCamFlag = 0;

    if (m_irOnData1)
    {
        my_free(m_irOnData1);
        m_irOnData1 = NULL;
    }
    if (m_irOnData2)
    {
        my_free(m_irOnData2);
        m_irOnData2 = NULL;
    }

    if (g_abCapturedFace)
    {
        my_free(g_abCapturedFace);
        g_abCapturedFace = NULL;
    }
    g_xSS.rFaceEngineTime = 0;
}

int SaveImage(unsigned char* pbImage, int iSaveIdx, int iRotate)
{
    int iMaxLen = 128*1024;
    static unsigned char* pbJpgData = NULL;

    if (iSaveIdx < 1 || iSaveIdx > SI_MAX_IMAGE_COUNT)
    {
        //invalid index
        return -1;
    }

    rotateImage_inner(pbImage, WIDTH_1280, HEIGHT_720, iRotate);
    Shrink_Grey(pbImage, WIDTH_1280, HEIGHT_720, g_abCapturedFace, CAPTURE_HEIGHT, CAPTURE_WIDTH);

    jpge::params params;
    params.m_quality = 90;
    params.m_subsampling = jpge::Y_ONLY;

#ifndef __RTK_OS__
    if(pbJpgData == NULL)
        pbJpgData = (unsigned char*)my_malloc(SI_MAX_IMAGE_SIZE);
#else // ! __RTK_OS__
    if (g_xSS.bSnapImageData == NULL)
        g_xSS.bSnapImageData = (unsigned char*)my_malloc(SI_MAX_IMAGE_SIZE * SI_MAX_IMAGE_COUNT);
    if (g_xSS.bSnapImageData == NULL)
    {
        my_printf("@@@ SaveImage malloc fail\n");
        return 0;
    }
    pbJpgData = (unsigned char*)g_xSS.bSnapImageData + (iSaveIdx - 1) * SI_MAX_IMAGE_SIZE;
#endif // ! __RTK_OS__

    int iWriteLen = iMaxLen;
    for(int i = 70; i >= 10; i -= 10)
    {
        jpge::params params;
        params.m_quality = i;
        params.m_subsampling = jpge::Y_ONLY;

        iWriteLen = iMaxLen;
        if(!jpge::compress_image_to_jpeg_file_in_memory(pbJpgData, iWriteLen, CAPTURE_WIDTH, CAPTURE_HEIGHT, 1, g_abCapturedFace, params))
        {
            iWriteLen = 0;
            break;
        }

        if(iWriteLen < SI_MAX_IMAGE_SIZE)
        {
            my_printf("Jpeg Quality: %d\n", i);
            break;
        }
    }
#ifndef __RTK_OS__
    my_mount(TMP_DEVNAME, "/db1", TMP_FSTYPE, MS_NOATIME, "");

    char szPath[256] = { 0 };
    sprintf(szPath, "/db1/img_%d.jpg", iSaveIdx);
    FILE* fp = my_fopen(szPath, "wb");
    my_printf("SaveImae: %s, %p\n", szPath, fp);

    if(fp)
    {
        my_fwrite(pbJpgData, iWriteLen, 1, fp);
        fsync(fileno(fp));
        fclose(fp);
    }
#else // !__RTK_OS__
    g_xSS.iSnapImageLen[iSaveIdx - 1] = iWriteLen;
    dbug_printf("[%s] save, %d, %d, %0.3f------------\n", __func__, iSaveIdx, iWriteLen, Now());
#endif // !__RTK_OS__
    return 0;
}

void FaceRecogTask::ThreadProc()
{
    run();
}

void* faceRecogTask_ThreadProc1(void* param)
{
    FaceRecogTask* pThread = (FaceRecogTask*)(param);
    pThread->ThreadProc();
    return NULL;
}
