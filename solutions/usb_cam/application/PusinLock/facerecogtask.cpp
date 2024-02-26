#include "facerecogtask.h"
#include "drv_gpio.h"
#include "appdef.h"
#include "shared.h"
#include "DBManager.h"
#include "engineparam.h"
#include "ImageProcessing.h"
#include "FaceRetrievalSystem.h"
#include "settings.h"
#include "camerasurface.h"
#include "senselockmessage.h"
#include "faceengine.h"
#include "jpge.h"
#include "check_camera_pattern.h"
#include "sn.h"
#include "senselocktask.h"
#include <vfs.h>

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#if (USE_FP16_ENGINE)
#if (N_MAX_HAND_NUM)
#include "hand/HandRetrival_.h"
#endif // N_MAX_HAND_NUM
#include "ComboRetrievalSystem.h"
#endif // USE_FP16_ENGINE
#include "senselocktask.h"
#include "upgradebase.h"

//#define USE_STATIC_IMAGE
unsigned char *g_abCapturedFace = NULL;

int FaceRecogTask::m_iCounter = 0;
int FaceRecogTask::m_iImageMode = 0;

#define N_MAX_SAVE_IMG_NUM      SI_MAX_IMAGE_COUNT
static int SaveImage(unsigned char* pbImage, int iSaveIdx, int iRotate,
                     int width = IR_CAM_WIDTH, int height = IR_CAM_HEIGHT);
void* faceRecogTask_ThreadProc1(void* param);

message_queue g_queue_face;

extern void EndIns();
extern "C" int g_sc201cs_aec_exp;
extern "C" int g_sc201cs_aec_gain;
extern "C" int g_sc201cs_aec_dig_gain;
extern "C" int g_sc201cs_aec_fine_gain;

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
    g_xSS.iFirstFlag = 0;
    g_xSS.rFaceEngineTime = Now();
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
    fr_SetCameraFlip(g_xSS.iCameraRotate);
#if (USE_3M_MODE == U3M_SEMI)
    fr_SetColorLed(2);
#endif
    m_isFaceOcculution = 0;
    memset(m_iFaceNearFar, 0, sizeof(m_iFaceNearFar));
    memset(m_iFacePosition, 0, sizeof(m_iFacePosition));
    memset(m_isFaceDetected, 0, sizeof(m_isFaceDetected));
    memset(m_rDetectTime, 0, sizeof(m_rDetectTime));
    m_iLastDetMode = FMI_FACE;

    message_queue_init(&g_queue_face, sizeof(MSG), MAX_MSG_NUM);

#if 1
    if(my_thread_create_ext(&m_thread, NULL, faceRecogTask_ThreadProc1, this, (char*)"fatk", 16384, MYTHREAD_PRIORITY_HIGH))
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

void FaceRecogTask::SendFaceDetectMsg()
{
    int iFaceNearFar = 0;
    int iFacePosition = 0;
    int idx = m_rDetectTime[FMI_FACE] >= m_rDetectTime[FMI_HAND] ? FMI_FACE : FMI_HAND;
    int isFaceDetected = 0;
    int iFaceNearFarIdx = (m_iFaceNearFar[idx] != 0) ? idx : ((idx + 1) % FMI_END);
    int iFacePositionIdx = (m_iFacePosition[idx] != 0) ? idx : ((idx + 1) % FMI_END);
    iFaceNearFar = m_iFaceNearFar[iFaceNearFarIdx];
    iFacePosition = m_iFacePosition[iFacePositionIdx];
    isFaceDetected = (m_isFaceDetected[idx] != FDS_NONE) ? m_isFaceDetected[idx] : m_isFaceDetected[(idx + 1) % FMI_END];
    if(g_xSS.iOcclusionFlag)
    {
        if (m_isFaceOcculution)
        {
            g_xSS.note_data_face.state = FACE_STATE_FACE_OCCLUSION;
            SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, 0, m_iCounter);
        }
    }
    else if(iFaceNearFar)
    {
        if(iFaceNearFar == 1)
            g_xSS.note_data_face.state = (iFaceNearFarIdx == FMI_FACE) ? FACE_STATE_TOOCLOSE : FACE_STATE_HAND_CLOSE;
        else if(iFaceNearFar == 2)
            g_xSS.note_data_face.state = (iFaceNearFarIdx == FMI_FACE) ? FACE_STATE_TOOFAR : FACE_STATE_HAND_FAR;
        SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, 0, m_iCounter);
    }
    else if(iFacePosition)
    {
        if(iFacePosition == 1)
            g_xSS.note_data_face.state = (iFacePositionIdx == FMI_FACE) ? FACE_STATE_TOOLEFT : FACE_STATE_HAND_TOOLEFT;
        else if(iFacePosition == 2)
            g_xSS.note_data_face.state = (iFacePositionIdx == FMI_FACE) ? FACE_STATE_TOORIGHT : FACE_STATE_HAND_TOORIGHT;
        else if(iFacePosition == 3)
            g_xSS.note_data_face.state = (iFacePositionIdx == FMI_FACE) ? FACE_STATE_TOOUP : FACE_STATE_HAND_TOOUP;
        else if(iFacePosition == 4)
            g_xSS.note_data_face.state = (iFacePositionIdx == FMI_FACE) ? FACE_STATE_TOODOWN : FACE_STATE_HAND_TOODOWN;
        SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, 0, m_iCounter);
    }
    else if(isFaceDetected != FDS_NONE)
    {
        g_xSS.note_data_face.state = (isFaceDetected == FDS_FACE_DETECTED) ? FACE_STATE_NORMAL : FACE_STATE_HAND_NORMAL;
        SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, 0, m_iCounter);
    }
    else
    {
        g_xSS.note_data_face.state = FACE_STATE_NOFACE;
        SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, 0, m_iCounter);
    }
}

void FaceRecogTask::run()
{
    int iLoopCount = 0;
    int iTimeout = DETECTION_TIMEOUT;
#if (ENGINE_USE_TWO_CAM == EUTC_2V0_MODE || ENGINE_USE_TWO_CAM == EUTC_3V4_MODE)
    int i2ndCheck = 0;
#endif
    if(m_iCmd == E_REGISTER)
    {
        iTimeout = g_xSS.msg_enroll_itg_data.timeout;
    }
    else if(m_iCmd == E_VERIFY)
        iTimeout = g_xSS.msg_verify_data.timeout;
    else if(m_iCmd == E_EYE_CHECK)
        iTimeout = 2;
#if (USE_RENT_ENGINE)
    else if(m_iCmd == E_GET_IMAGE && g_xSS.iSnapImageFace)
        iTimeout = g_xSS.msg_verify_data.timeout;
#endif // USE_RENT_ENGINE

    if(iTimeout == 0)
        iTimeout = DETECTION_TIMEOUT;

    dbug_printf("Timeout: %d\n", iTimeout);

    if(g_xSS.iRestoreRootfs)
    {
        iTimeout = 30;
        //StopCamSurface();
    }

    memset(&g_xSS.xFaceRect, 0, sizeof(g_xSS.xFaceRect));

    m_rFaceFailTime = 0;

    m_rStartTime = Now();
    m_iCaptureCount = 0;
    m_iHasIrError = 0;

#if (NOTE_INTERVAL_MS)
    g_xSS.note_data_face.state = FACE_STATE_NOFACE;
    SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, 0, m_iCounter);
#endif
    while(m_iRunning)
    {
        if(g_xSS.iResetFlag == 1)
            break;

#if (USE_TONGXIN_PROTO == 0 && UVC_ENC_TYPE != 2)
        if (iLoopCount == 0)
            fr_LoadCreateMachine_2(1);
#endif

        if (ProcessEnrollFile1Step())
            break;

        iLoopCount ++;
        g_xSS.rFaceEngineTime = Now();

        ///적외선켜기지령을 내려보낸후 Led On/Off화상을 얻는다.
        int iFlag = 0;
        if (GetLeftIrFrame(&iFlag))
            break;

        if(!m_iRunning)
            break;

        if(m_iCmd == E_GET_IMAGE && g_xSS.iSnapImageFace == 0)
        {
#if (USE_3M_MODE == 0)
            GetRightIrFrame(NULL, iLoopCount == 0);
#endif
            if (ProcessGetImage1Step(iLoopCount))
                break;
            continue;
        }

        if(g_xSS.iResetFlag == 1)
            break;

        ///얼굴검출을 진행한다.
        int iSecondImageReCheck = 0;

        unsigned char* pInputImageBuffer1 = fr_GetInputImageBuffer1();

        fr_BackupIRCamera_ExpGain();

        int nProcessModeIndex = 0;
        int nProcessModeIndexStart = 0;
        int nProcessModeIndexEnd = 0;

        if(m_iCmd == E_VERIFY)
        {
            nProcessModeIndexStart = 0;
            nProcessModeIndexEnd = (N_MAX_HAND_NUM > 0) ? 1: 0;
            if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE)
            {
                nProcessModeIndexEnd = 0;
            }
        }
        else
        {
            nProcessModeIndexStart = -1;
            nProcessModeIndexEnd = -1;
        }

        if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE)
        {
            ProcessCheckCamera1Step();
        }

        int nBreaks = 0;
#if (ENGINE_USE_TWO_CAM == EUTC_2V0_MODE || ENGINE_USE_TWO_CAM == EUTC_3V4_MODE)
        int nGotRightFrame = 0;
#endif
        for(nProcessModeIndex = nProcessModeIndexStart; nProcessModeIndex <= nProcessModeIndexEnd; nProcessModeIndex ++)
        {
            //g_irOnData1 is required
            //caution: must fill buffer every time.
            if (g_xSS.iDemoMode != N_DEMO_FACTORY_MODE)
            {
                lockIRBuffer();
                memcpy(pInputImageBuffer1, g_irOnData1, IR_BUFFER_SIZE);
                unlockIRBuffer();
            }
            fr_PreExtractCombo(pInputImageBuffer1, nProcessModeIndex);

            if(g_xSS.iResetFlag == 1)
                break;

#if (ENGINE_USE_TWO_CAM == EUTC_2V0_MODE || ENGINE_USE_TWO_CAM == EUTC_3V4_MODE)
            if (i2ndCheck == 0)
            {
                //g_irOnData2 is required here.
                if (nGotRightFrame == 0 && fr_GetNeedSecondImageCheck())
                {
                    GetRightIrFrame(pInputImageBuffer1, 1);
                    nGotRightFrame = 1;
                }
                fr_PreExtractCombo2(pInputImageBuffer1);
#if (ENGINE_USE_TWO_CAM == 3)
                i2ndCheck = 1;
#endif
            }
#endif // ENGINE_USE_TWO_CAM

            int iNeedExp = fr_GetNeedToCalcNextExposure();
            iSecondImageReCheck = fr_GetSecondImageNeedReCheck();

#if (USE_TWIN_ENGINE)
            if(fr_GetFullOffImageBuffer())
            {
                if (!(g_iLedOffFrameFlag & LEFT_IROFF_CAM_RECVED))
                {
                    if (nGotOffFrame == 0)
                    {
                        WAIT_CAM_FRAME(1000, WaitIROffTimeout);
                        nGotOffFrame = 1;
                    }
                    if (g_xSS.iResetFlag == 1)
                        break;
                }
                fr_calc_Off(fr_GetFullOffImageBuffer());
            }
#endif // USE_TWIN_ENGINE

            GetFaceState();
            if ((m_isFaceDetected[FMI_FACE] != FDS_NONE || m_isFaceDetected[FMI_HAND] != FDS_NONE) && m_rFaceFailTime == 0)
            {
                m_rFaceFailTime = Now();
            }

            if(g_xSS.iResetFlag == 1)
                break;

            dbug_printf("Face: x=%d,y=%d,width=%d,heigh=%d, %f\n", g_xSS.xFaceRect.x, g_xSS.xFaceRect.y, g_xSS.xFaceRect.width, g_xSS.xFaceRect.height, Now());

            if(Now() - m_rStartTime > (iTimeout * 1000))
            {
                m_iResult = FACE_RESULT_TIMEOUT;
                nBreaks = 1;
                break;
            }

            if(!m_iRunning)
                break;

#if (USE_FUSHI_PROTO)
            if (g_xSS.bVerifying && g_xSS.rLastVerifyAckSendTime > 0 && ((Now() - g_xSS.rLastVerifyAckSendTime) >= 500))
            {
                g_xSS.bVerifying = 0;
                MarkSenseResetFlag();
            }
#endif // USE_FUSHI_PROTO

            if(iNeedExp && nProcessModeIndex == nProcessModeIndexEnd)
            {
                CalcNextExposure();
                fr_SetNeedDelayForCameraControl(1);
            }

#if (ENGINE_USE_TWO_CAM == EUTC_2V0_MODE)
            if(iSecondImageReCheck)
            {
                iFlag = 0;
                GetLeftIrFrame(&iFlag);
            }
#endif // ENGINE_USE_TWO_CAM

#if (USE_RENT_ENGINE)
            if (ProcessSaveFaceImage1Step())
            {
                nBreaks = 1;
                break;
            }
#endif // USE_RENT_ENGINE

            if(g_xSS.iResetFlag == 1)
                break;

            if(m_iCmd == E_REGISTER)
            {
                if (ProcessEnroll1Step(iSecondImageReCheck))
                {
                    nBreaks = 1;
                    break;
                }
            }
            else if(m_iCmd == E_VERIFY)
            {
                if (ProcessVerify1Step(iSecondImageReCheck))
                {
                    nBreaks = 1;
                    break;
                }
            }
            else if(m_iCmd == E_EYE_CHECK)
            {
                if (ProcessCheckEye1Step())
                {
                    nBreaks = 1;
                    break;
                }
            }
            if(iFlag)
            {
                EndIns();
                StartCamSurface(1);
            }
        }//for(nProcessModeIndex

        if(!m_iRunning)
            break;

        if(g_xSS.iResetFlag == 1)
            break;

        if(nBreaks)
        {
            break;
        }
    }

    memset(&g_xSS.xFaceRect, 0, sizeof(g_xSS.xFaceRect));

#if (DEFAULT_SECURE_MODE == 1)
    unsigned char _tmp_buf_cs[sizeof(g_xCS)];
    unsigned char _tmp_buf_hd2[sizeof(g_xHD2)];
    memcpy(_tmp_buf_cs, &g_xCS, sizeof(g_xCS));
    memcpy(_tmp_buf_hd2, &g_xHD2, sizeof(g_xHD2));
    fr_setSuitableThreshold(&g_xHD, &g_xCS, &g_xHD2);
    if (memcmp(_tmp_buf_cs, &g_xCS, sizeof(g_xCS)))
        UpdateCommonSettings();
    if (memcmp(_tmp_buf_hd2, &g_xHD2, sizeof(g_xHD2)))
        UpdateHeadInfos2();
#endif // DEFAULT_SECURE_MODE

#if (USE_3M_MODE == U3M_DEFAULT || USE_3M_MODE == U3M_SEMI)
    if (g_xSS.iUvcSensor == DEFAULT_SNR4UVC && 0)
    {
        camera_clr_set_exp(g_sc201cs_aec_exp);
        camera_clr_set_gain(g_sc201cs_aec_gain, g_sc201cs_aec_dig_gain, g_sc201cs_aec_fine_gain);
        camera_clr_start_aec();
    }
#endif // USE_3M_MODE
    SendMsg(MSG_RECOG_FACE, FACE_TASK_FINISHED, 0, m_iCounter);
    dbug_printf("[Recog] Verify Face End!\n");
    g_xSS.rFaceEngineTime = 0;
    g_iFirstCamFlag = 0;
    reset_ir_exp_gain();
}


int SaveImage(unsigned char* pbImage, int iSaveIdx, int iRotate,
              int width, int height)
{
    int iMaxLen = 128 * 1024;
    unsigned char* pbJpgData = NULL;
    unsigned char* g_abCapturedFace = NULL;

    g_abCapturedFace = (unsigned char*)my_tpu_malloc(g_xSS.iCapWidth * g_xSS.iCapHeight * 3);
    if (!g_abCapturedFace)
    {
        my_printf("[%s] malloc fail1.\n", __func__);
        return -1;
    }

    rotateImage_inner(pbImage, width, height, iRotate);
    Shrink_Grey(pbImage, width, height, g_abCapturedFace, g_xSS.iCapHeight, g_xSS.iCapWidth);

    jpge::params params;
#if (USE_NEW_SNAPIMAGE_MODE)
    if (g_xSS.bSnapImageData == NULL)
        g_xSS.bSnapImageData = (unsigned char*)my_tpu_malloc(SI_MAX_IMAGE_SIZE * SI_MAX_IMAGE_COUNT);
    if (g_xSS.bSnapImageData == NULL)
    {
        my_printf("[%s] malloc fail2.\n", __func__);
        return -1;
    }
#endif
    pbJpgData = (unsigned char*)my_malloc(iMaxLen);
    if (!pbJpgData)
    {
        my_printf("[%s] malloc fail3.\n", __func__);
        my_tpu_free(g_abCapturedFace);
        return -1;
    }

    int iWriteLen = 0;
    float rOld = Now();
    for(int i = 60; i >= 10; i -= 10)
    {
        jpge::params params;
        params.m_quality = i;
        params.m_subsampling = jpge::Y_ONLY;

        iWriteLen = iMaxLen;
        if(!jpge::compress_image_to_jpeg_file_in_memory(pbJpgData, iWriteLen, g_xSS.iCapWidth, g_xSS.iCapHeight, 1, g_abCapturedFace, params, NULL))
        {
            iWriteLen = 0;
            break;
        }

        if(iWriteLen < SI_MAX_IMAGE_SIZE)
        {
            dbug_printf("Jpeg Quality: %d\n", i);
            break;
        }
    }
    if (iWriteLen > 0 && iWriteLen < SI_MAX_IMAGE_SIZE)
    {
        memcpy((unsigned char*)g_xSS.bSnapImageData + (iSaveIdx - 1) * SI_MAX_IMAGE_SIZE, pbJpgData, iWriteLen);
        g_xSS.iSnapImageLen[iSaveIdx - 1] = iWriteLen;
        if (rOld > 0)
            dbug_printf("[%s]: %d, %d, time=%0.3f\n", __func__, iSaveIdx, iWriteLen, Now() - rOld);
    }
    else
    {
        my_printf("[%s]: %d, exceed size, time=%0.3f\n", __func__, iSaveIdx, Now() - rOld);
        iWriteLen = 0;
    }
    if (pbJpgData)
        my_free(pbJpgData);
    if (g_abCapturedFace)
        my_tpu_free(g_abCapturedFace);

    return iWriteLen;
}

int FaceRecogTask::ReadStaticIRImage(void* dst, int flip)
{
    unsigned char* test_tmp_buff = g_irOnData1;
    //if (test_tmp_buff)
    {
        fr_ReadFileData(FN_FACE_IR_BIN_PATH, 0, test_tmp_buff, IR_TEST_BIN_WIDTH * IR_TEST_BIN_HEIGHT);
        memset(dst, 0, IR_CAM_WIDTH * IR_CAM_HEIGHT);
        for (int y = 0; y < IR_TEST_BIN_HEIGHT ; y++)
        {
            memcpy(((char*)dst) + (y + IR_TEST_BIN_H_START) * IR_CAM_WIDTH + IR_TEST_BIN_W_START, test_tmp_buff + y * IR_TEST_BIN_WIDTH, IR_TEST_BIN_WIDTH);
        }
        if (flip)
        {
            int n = IR_CAM_WIDTH * IR_CAM_HEIGHT;
            unsigned char* ptr1 = (unsigned char*)dst;
            unsigned char* ptr2 = (unsigned char*)dst + n - 1;
            n /= 2;
            for (int i = 0; i < n; i++)
            {
                unsigned char tmp = *ptr1;
                *ptr1 = *ptr2;
                *ptr2 = tmp;
                ptr1++;
                ptr2--;
            }
        }
        dbug_printf("@@@ read IR static image ok\n");
    }
    // else
    //     my_printf("@@@ read IR static image fail\n");
    return 0;
}

int FaceRecogTask::ProcessGetImage1Step(int iFrameCount)
{
    int ret = 0;
    lockIRBuffer();
    fr_CalcScreenValue(g_irOnData1, IR_SCREEN_GETIMAGE_MODE);
    unlockIRBuffer();
    CalcNextExposure();
    if (iFrameCount >= DEFAULT_SNAPIMG_CTRL_CNT)
    {
        if(m_iCaptureCount < g_xSS.msg_snap_image_data.image_counts &&
                g_xSS.msg_snap_image_data.start_number + m_iCaptureCount <= N_MAX_SAVE_IMG_NUM)
        {
            lockIRBuffer();
            remove_white_point_riscv(g_irOnData1, IR_CAM_WIDTH, IR_CAM_HEIGHT);
            gammaCorrection_screen(g_irOnData1, IR_CAM_WIDTH, IR_CAM_HEIGHT);
            ret += SaveImage(g_irOnData1, g_xSS.msg_snap_image_data.start_number + m_iCaptureCount, g_xSS.iCameraRotate == 0 ? 270: 90);
            unlockIRBuffer();
            m_iCaptureCount ++;
        }

        if(m_iCaptureCount < g_xSS.msg_snap_image_data.image_counts &&
                g_xSS.msg_snap_image_data.start_number + m_iCaptureCount <= N_MAX_SAVE_IMG_NUM)
        {
            lockIRBuffer();
#if (!USE_3M_MODE)
            remove_white_point_riscv(g_irOnData2, IR_CAM_WIDTH, IR_CAM_HEIGHT);
            gammaCorrection_screen(g_irOnData2, IR_CAM_WIDTH, IR_CAM_HEIGHT);
#else
            memset(g_irOnData2, 0, IR_BUFFER_SIZE);
#endif
            ret += SaveImage(g_irOnData2, g_xSS.msg_snap_image_data.start_number + m_iCaptureCount, g_xSS.iCameraRotate == 0 ? 90 : 270);
            unlockIRBuffer();
            m_iCaptureCount ++;
        }

        if(!(m_iCaptureCount < g_xSS.msg_snap_image_data.image_counts &&
             g_xSS.msg_snap_image_data.start_number + m_iCaptureCount <= N_MAX_SAVE_IMG_NUM))
        {
            if (ret > 0)
                m_iResult = FACE_RESULT_CAPTURED_FACE;
            else
                m_iResult = FACE_RESULT_CAPTURED_FACE_FAILED;
            return 1;
        }
    }
    return 0;
}

int FaceRecogTask::ProcessCheckCamera1Step()
{
    //check camera
    int res1 = 0, res2 = 0;
    unsigned char* pInputImageBuffer1 = fr_GetInputImageBuffer1();
    unsigned char* pInputImageBuffer2 = fr_GetInputImageBuffer2();
#if (MY_SAVE_ERROR_IMG)
    FILE *_fp = NULL;
#endif // MY_SAVE_ERROR_IMG
    lockIRBuffer();
    memcpy(pInputImageBuffer1, g_irOnData1, IR_BUFFER_SIZE);
    unlockIRBuffer();
    if (m_iCmd == E_VERIFY)
    {
        res1 = checkCameraPattern(pInputImageBuffer1);
        lockIRBuffer();
        memcpy(pInputImageBuffer2, g_irOnData2, IR_BUFFER_SIZE);
        unlockIRBuffer();
#if (!USE_3M_MODE)
        res2 = (ENGINE_USE_TWO_CAM == 1) ? checkCameraPattern(pInputImageBuffer2) : 0;
#else // !USE_3M_MODE
        res2 = 0;
#endif // !USE_3M_MODE
        my_printf("** check camera=%d,%d\n", res1, res2);
        if (res1 != 0 || res2 != 0)
        {
            if (m_iHasIrError == 1)
            {
                if (res1 == CAMERA_ERROR || res2 == CAMERA_ERROR)
                    g_xSS.iCamError |= CAM_ERROR_IR_PATTERN;
                else
                    g_xSS.iCamError |= CAM_ERROR_IR_PATTERN_2;
            }
        }

        if (res1 != 0 && m_iHasIrError)
        {
#if (MY_SAVE_ERROR_IMG)
            mount(TMP_DEVNAME, "/db1", TMP_FSTYPE, MS_NOATIME, "");
            _fp = fopen("/db1/f1.bin", "w");
            my_printf("going to save ir cam1, %p.\n", _fp);
            if (_fp != NULL)
            {
                fwrite(pInputImageBuffer1, 1, IR_CAM_HEIGHT * IR_CAM_WIDTH, _fp);
                fsync(fileno(_fp));
                fclose(_fp);
            }
#endif // MY_SAVE_ERROR_IMG
        }

        if (res2 != 0 && m_iHasIrError)
        {
#if (MY_SAVE_ERROR_IMG)
            mount(TMP_DEVNAME, "/db1", TMP_FSTYPE, MS_NOATIME, "");
            _fp = fopen("/db1/f2.bin", "w");
            my_printf("going to save ir cam2, %p.\n", _fp);
            if (_fp != NULL)
            {
                fwrite(pInputImageBuffer2, 1, IR_CAM_HEIGHT * IR_CAM_WIDTH, _fp);
                fsync(fileno(_fp));
                fclose(_fp);
            }
#endif // MY_SAVE_ERROR_IMG
        }
    }

    if ((res1 == 0 && res2 == 0) || m_iHasIrError == 1)
    {
        //read static image.
        ReadStaticIRImage(pInputImageBuffer1, 0);
    }
    else
    {
        memset(pInputImageBuffer1, 0, IR_BUFFER_SIZE);
        m_iHasIrError = 1;
    }
    return 0;
}

int FaceRecogTask::GetRightIrFrame(void* pBuffer, int iUseFirstFrame)
{
    if (!pBuffer)
        return 0;
    //caution: you should not use pInputImageBuffer2 here!!!
    if (g_xSS.iDemoMode != N_DEMO_FACTORY_MODE)
    {
        int buf_len = DumpFromVpss(DEFAULT_SNR4UVC, 0, 1, (unsigned char*)pBuffer, 1);
        if (buf_len <= 0)
        {
            my_printf("dump vpss fail\n");
            unlockIRBuffer();
            return -1;
        }
        dbug_printf("dump vpss ok\n");
    }
    else
    {
        //read static image.
        ReadStaticIRImage(pBuffer, 1);
    }
    return 0;
}

int FaceRecogTask::GetFaceState()
{
    int isFaceDetected = 0;
    int iNext = ES_FAILED;
    int nProcessMode = 0;   //if 0:face process, 1:hand process
    iNext = fr_ExtractCombo();

    SEngineResult* pxEngineResult;
    pxEngineResult =  fr_GetEngineResultCombo(&nProcessMode);
    SEngineParam* pxEngineParam = fr_GetEngineParam();
    memset(&g_xSS.xFaceRect, 0, sizeof(g_xSS.xFaceRect));
    memset(&g_xSS.note_data_face, 0, sizeof(g_xSS.note_data_face));
    m_isFaceDetected[nProcessMode] = FDS_NONE;
    m_isFaceOcculution = 0;
    m_iFaceNearFar[nProcessMode] = 0;
    m_iFacePosition[nProcessMode] = 0;
    m_rDetectTime[nProcessMode] = Now();
    if(iNext == ES_FAILED)
    {
        if(pxEngineResult && pxEngineParam)
        {
            m_isFaceOcculution = pxEngineResult->nOcclusion;
            m_iFaceNearFar[nProcessMode] = pxEngineResult->nFaceNearFar;
            m_iFacePosition[nProcessMode] = pxEngineResult->nFacePosition;
            if (m_isFaceOcculution + m_iFaceNearFar[nProcessMode] + m_iFacePosition[nProcessMode] != 0)
            {
                m_iLastDetMode = nProcessMode;
            }
        }
    }
    else if (iNext == ES_SUCCESS)
    {
        if (nProcessMode == FMI_FACE)
            m_isFaceDetected[nProcessMode] = FDS_FACE_DETECTED;
        else
            m_isFaceDetected[nProcessMode] = FDS_HAND_DETECTED;
        if (m_isFaceDetected[nProcessMode] != FDS_NONE)
            m_iLastDetMode = nProcessMode;
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
    }
    if (nProcessMode == 0) //face process
    {
        if (m_isFaceOcculution)
            g_xSS.iOcclusionFlag = 1;
        else
            g_xSS.iOcclusionFlag = 0;
    }
    return isFaceDetected;
}

int FaceRecogTask::ProcessVerify1Step(int iSecondImageReCheck)
{
    float arEngineResult[10] = { 0 };
    SendFaceDetectMsg();

    if(g_xSS.iResetFlag == 1)
        return 1;

#if (USE_DEMOMODE2)
    if (g_xSS.iDemoMode == N_DEMO_VERIFY_MODE_OFF &&
            dbm_GetTotalUserCount() == 0 &&
            (m_isFaceDetected[FMI_FACE] == FDS_FACE_DETECTED || m_isFaceDetected[FMI_HAND] == FDS_HAND_DETECTED))
    {
        m_iResult = m_isFaceDetected[FMI_FACE] == FDS_FACE_DETECTED ? FACE_RESULT_DETECTED : HAND_RESULT_DETECTED;
#if (USE_FUSHI_PROTO)
        g_xSS.bVerifying = 0;
#endif
        //검출결과와 인증결과가 간격이 없어 조종기판쪽에서 받지 못하는 문제가 있어 지연을 주었음.
        my_usleep(20*1000);
        return 1;
    }
#endif // USE_DEMOMODE2

#if (USE_DEMOMODE4HAND == 0)
    if (g_xSS.iDemoMode == N_DEMO_VERIFY_MODE_ON &&
            (m_isFaceDetected[FMI_HAND] == FDS_HAND_DETECTED))
    {
        m_iResult = HAND_RESULT_SUCCESS;
        m_iRecogIndex = 0;
        //검출결과와 인증결과가 간격이 없어 조종기판쪽에서 받지 못하는 문제가 있어 지연을 주었음.
        usleep(20*1000);
        return 1;
    }
#endif // USE_DEMOMODE4HAND

    memset(arEngineResult, 0, sizeof(arEngineResult));
    int iFindIndex = FaceEngine::VerifyFace(arEngineResult);

    if(g_xSS.iResetFlag == 1)
        return 1;

#if (ENGINE_USE_TWO_CAM == EUTC_3M_MODE)
#if (USE_3M_MODE == U3M_DEFAULT || USE_3M_MODE == U3M_SEMI)
    if((arEngineResult[0] == ES_SUCCESS || arEngineResult[0] == ES_UPDATE) && arEngineResult[1] == 0 && g_xSS.iUvcSensor == DEFAULT_SNR4UVC) //face mode only
    {
        unsigned char* pInputImageBuffer1 = fr_GetInputImageBuffer1();
        if (g_xSS.iFirstFlag == 2) // use first clr frame
        {
            g_xSS.iFirstFlag = 1;
            GetRightIrFrame(pInputImageBuffer1, 1);
        }
        else
            GetRightIrFrame(pInputImageBuffer1, 0);
        int w = GetDumpVpssWidth();
        int h = GetDumpVpssHeight();
        if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE)
        {
            memset(pInputImageBuffer1, 0, IR_BUFFER_SIZE);
            ReadStaticIRImage(pInputImageBuffer1, 1);
            w = IR_CAM_WIDTH;
            h = IR_CAM_HEIGHT;
        }

        int iCheck = fr_PreExtractFaceClr(pInputImageBuffer1, w, h, 1);
        if(iCheck != ES_SUCCESS)
        {
            arEngineResult[0] = ES_PROCESS;
        }
    }
#endif // USE_3M_MODE
#elif (ENGINE_USE_TWO_CAM != EUTC_2D_MODE) // ENGINE_USE_TWO_CAM
    unsigned char* pInputImageBuffer1 = fr_GetInputImageBuffer1();
    if((arEngineResult[0] == ES_SUCCESS || arEngineResult[0] == ES_UPDATE) && iSecondImageReCheck)
    {
        //verify time will be greater than 300ms
        if(fr_GetSecondImageIsRight())
        {
            GetRightIrFrame(pInputImageBuffer1, 0);
        }
        else
        {
            lockIRBuffer();
            memcpy(pInputImageBuffer1, g_irOnData1, IR_BUFFER_SIZE);
            unlockIRBuffer();
        }

        int iCheck = fr_CheckFaceInSecondImage(pInputImageBuffer1);
        if(iCheck != ES_SUCCESS)
        {
            arEngineResult[0] = ES_PROCESS;
        }
    }
#endif // ENGINE_USE_TWO_CAM
    if(arEngineResult[0] == ES_PROCESS && m_rFaceFailTime != 0)
    {
        if(Now() - m_rFaceFailTime >= N_MAX_FAILED_TIME * 1000)
            arEngineResult[0] = ES_FAILED;
    }

    if(arEngineResult[0] != ES_PROCESS)
    {
        if(arEngineResult[0] != ES_FAILED)
        {
            dbug_printf("FRS %d, %f\n", iFindIndex, Now());
            if (arEngineResult[1] == 0) //face
            {
                m_iResult = FACE_RESULT_SUCCESS;
#if (USE_WHITE_LED == UWL_DISABLE)
                if (fr_GetClrIsDarkGotoIr())
                    g_xSS.iForceUvcIR = 1;
#endif
            }
            else if (arEngineResult[1] == 1) //hand
                m_iResult = HAND_RESULT_SUCCESS;
            m_iEyeOpened = arEngineResult[6];
            m_iRecogIndex = iFindIndex;

            if(!m_iEyeOpened && (m_iResult == FACE_RESULT_SUCCESS))
            {
                m_iCmd = E_EYE_CHECK;
                m_rStartTime = Now();
            }
            else
            {
                return 1;
            }
        }
        else
        {
            dbug_printf("Face Recog Failed!\n");
            m_iResult = m_iLastDetMode == FMI_HAND ? HAND_RESULT_FAILED : FACE_RESULT_FAILED;
#if (USE_FUSHI_PROTO)
            if (g_xSS.iProtocolHeader != FUSHI_HEAD1)
#endif
            {
                return 1;
            }
        }
    }
    return 0;
}
int FaceRecogTask::ProcessEnroll1Step(int iSecondImageReCheck)
{
    float arEngineResult[10] = { 0 };
    int ret = 0;
    SendFaceDetectMsg();
    FaceEngine::RegisterFace(arEngineResult, g_xSS.msg_enroll_itg_data.face_direction);

    if(g_xSS.iResetFlag == 1)
        return 1;
#if (ENGINE_USE_TWO_CAM == EUTC_3M_MODE)
#if (USE_3M_MODE == U3M_DEFAULT || USE_3M_MODE == U3M_SEMI)
    if((arEngineResult[0] == ES_SUCCESS || arEngineResult[0] == ES_ENEXT) && arEngineResult[1] == 0 && g_xSS.iUvcSensor == DEFAULT_SNR4UVC) //face mode only
    {
        unsigned char* pInputImageBuffer1 = fr_GetInputImageBuffer1();
        GetRightIrFrame(pInputImageBuffer1, 0);
        int w = GetDumpVpssWidth();
        int h = GetDumpVpssHeight();
        if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE)
        {
            memset(pInputImageBuffer1, 0, IR_BUFFER_SIZE);
            ReadStaticIRImage(pInputImageBuffer1, 1);
            w = IR_CAM_WIDTH;
            h = IR_CAM_HEIGHT;
        }

        int iCheck = fr_PreExtractFaceClr(pInputImageBuffer1, w, h, 1);
        if(iCheck != ES_SUCCESS)
        {
            arEngineResult[0] = ES_PROCESS;
        }
    }
#endif // USE_3M_MODE
#elif (ENGINE_USE_TWO_CAM != EUTC_2D_MODE) // ENGINE_USE_TWO_CAM
    unsigned char* pInputImageBuffer1 = fr_GetInputImageBuffer1();
    if((arEngineResult[0] == ES_SUCCESS || arEngineResult[0] == ES_ENEXT) && iSecondImageReCheck)
    {
        //register time will be greater than 300ms
        if(fr_GetSecondImageIsRight())
        {
            GetRightIrFrame(pInputImageBuffer1, 0);
        }
        else
        {
            lockIRBuffer();
            memcpy(pInputImageBuffer1, g_irOnData1, IR_BUFFER_SIZE);
            unlockIRBuffer();
        }

        int iCheck = fr_CheckFaceInSecondImage(pInputImageBuffer1);
        if(iCheck != ES_SUCCESS)
        {
            arEngineResult[0] = ES_PROCESS;
        }
    }
#endif // ENGINE_USE_TWO_CAM
    if(arEngineResult[0] != ES_PROCESS)
    {
        if(arEngineResult[1] ==  0) //face mode only
        {
            if(arEngineResult[0] == ES_SUCCESS)
            {
                m_iResult = FACE_RESULT_ENROLLED_FACE;
#if (USE_WHITE_LED == UWL_DISABLE)
                if (fr_GetClrIsDarkGotoIr())
                    g_xSS.iForceUvcIR = 1;
#endif
                ret = 1;
            }
            else if(arEngineResult[0] == ES_DUPLICATED)
            {
                m_iResult = FACE_RESULT_DUPLICATED_FACE;
                ret = 1;
            }
            else if(arEngineResult[0] == ES_SPOOF_FACE)
            {
                m_iResult = FACE_RESULT_SPOOF_FACE;
                ret = 1;
            }
            if(arEngineResult[0] == ES_ENEXT)
            {
                m_iResult = FACE_RESULT_ENROLLED_NEXT;
#if (USE_WHITE_LED == UWL_DISABLE)
                if (fr_GetClrIsDarkGotoIr())
                    g_xSS.iForceUvcIR = 1;
#endif
                ret = 1;
            }
            else if(arEngineResult[0] == ES_DIRECTION_ERROR)
            {
                g_xSS.note_data_face.state = FACE_STATE_DIRECTION_ERROR;
                SendMsg(MSG_RECOG_FACE, FACE_TASK_DETECTED, 0, m_iCounter);
            }
        }
#if (N_MAX_HAND_NUM)
        else
        {
            if(arEngineResult[0] == ES_SUCCESS)
            {
                my_usleep(20*1000);
                m_iResult = HAND_RESULT_ENROLLED;
                if (USE_TONGXIN_PROTO == 1 && g_xSS.msg_enroll_itg_data.face_direction != FACE_DIRECTION_MIDDLE && g_xSS.iEnrollMutiDirMode)
                {
                    m_iResult = FACE_RESULT_TIMEOUT;
                }
                else if (g_xSS.msg_enroll_itg_data.face_direction != FACE_DIRECTION_MIDDLE && g_xSS.iEnrollMutiDirMode)
                {
                    for (int t = 0; t < 100; t ++)
                    {
                        my_usleep(20*1000);
                        if (t % 5 == 0)
                            SendFaceDetectMsg();
                        if (g_xSS.iResetFlag)
                            break;
                    }
                }
                ret = 1;
            }
            else if(arEngineResult[0] == ES_DUPLICATED)
            {
                my_usleep(20*1000);
                m_iResult = HAND_RESULT_ENROLL_DUPLICATED;
                ret = 1;
            }
            else if(arEngineResult[0] == ES_ENEXT)
            {
                my_usleep(20*1000);
#if (USE_SANJIANG3_MODE)
                if ((SenseLockTask::m_encMode == SenseLockTask::EM_XOR && g_xSS.iProtoMode == PROTO_MODE_SANJIANG) || (g_xSS.iRegisterMixMode == ENROLL_FACE_HAND_MIX && USE_TONGXIN_PROTO == 1))
                {
                    m_iResult = HAND_RESULT_ENROLLED;
                    if (USE_TONGXIN_PROTO == 1 && g_xSS.msg_enroll_itg_data.face_direction != FACE_DIRECTION_MIDDLE)
                        m_iResult = FACE_RESULT_TIMEOUT;
                }
                else
                {
                    m_iResult = HAND_RESULT_ENROLLED_NEXT;
                    if (g_xSS.msg_enroll_itg_data.face_direction != FACE_DIRECTION_MIDDLE && g_xSS.iEnrollMutiDirMode)
                    {
                        for (int t = 0; t < 100; t ++)
                        {
                            my_usleep(20*1000);
                            if (t % 5 == 0)
                                SendFaceDetectMsg();
                            if (g_xSS.iResetFlag)
                                break;
                        }
                    }
                }
#else // USE_SANJIANG3_MODE
                m_iResult = HAND_RESULT_ENROLLED_NEXT;
#endif // USE_SANJIANG3_MODE
                ret = 1;
            }
        }
#endif // N_MAX_HAND_NUM
    }

    return ret;
}
int FaceRecogTask::ProcessCheckEye1Step()
{
    float arEngineResult[10] = { 0 };
    if(m_isFaceDetected[FMI_FACE] == FDS_FACE_DETECTED)
    {
        FaceEngine::VerifyFace(arEngineResult);

        if(g_xSS.iResetFlag == 1)
            return 1;

        if(arEngineResult[6] == 1)
        {
            //eye_close_open
            g_xSS.note_data_eye.eye_state = FACE_STATE_EYE_CLOSE_STATUS_OPEN_EYE;
            m_iEyeOpened = arEngineResult[6];
            //                    m_iResult = FACE_RESULT_TIMEOUT;
            return 1;
        }
        else if(Now() - m_rStartTime > 1000)
        {
            //eye_close
            g_xSS.note_data_eye.eye_state = FACE_STATE_EYE_CLOSE_STATUS;
            m_iEyeOpened = 0;
            //                    m_iResult = FACE_RESULT_TIMEOUT;
            return 1;
        }
    }
    else if(Now() - m_rStartTime > 1000)
    {
        //eye unkown
        g_xSS.note_data_eye.eye_state = FACE_STATE_EYE_CLOSE_STATUS;
        m_iEyeOpened = 0;
        //                m_iResult = FACE_RESULT_TIMEOUT;
        return 1;
    }
    return 0;
}

int FaceRecogTask::GetLeftIrFrame(int* p_iUseFirstFrame)
{
    if (p_iUseFirstFrame == NULL)
        return 1;
    if((g_iFirstCamFlag & LEFT_IR_CAM_RECVED) && !(g_iFirstCamFlag & FIRST_IR_PROCESSED))
    {
        g_iFirstCamFlag |= FIRST_IR_PROCESSED;
        *p_iUseFirstFrame = 1;
    }
    else
    {
#if (ENGINE_USE_TWO_CAM == 1)
        if((g_iMipiCamInited == -1 || g_iDvpCamInited == -1) &&
                !((g_iFirstCamFlag & LEFT_IR_CAM_RECVED) && (g_iFirstCamFlag & RIGHT_IR_CAM_RECVED)))
#else // ENGINE_USE_TWO_CAM
        if(g_iDvpCamInited == -1 && !(g_iFirstCamFlag & LEFT_IR_CAM_RECVED))
#endif // ENGINE_USE_TWO_CAM
        {
            my_usleep(30 * 1000);
            m_iResult = FACE_RESULT_FAILED_CAMERA;
            return 1;
        }
        if (g_xSS.iCamError != CAM_ERROR_NONE)
        {
            my_usleep(30 * 1000);
            m_iResult = FACE_RESULT_FAILED_CAMERA;
            return 1;
        }

// #if (ENGINE_USE_TWO_CAM == 1)
//         if (g_iTwoCamFlag != -1 && !(g_iLedOffFrameFlag & LEFT_IROFF_CAM_RECVED))
//         {
//             WAIT_CAM_FRAME(1000, WaitIROffTimeout);
//         }
//         dbug_printf("g_iTwoCamFlag=%d\n", g_iTwoCamFlag);
// #endif // ENGINE_USE_TWO_CAM

#if (IR_LED_ONOFF_MODE == 1)
        camera_set_irled_on(1);
        dbug_printf("irled on, %s:%d\n", __FILE__, __LINE__);
        g_iTwoCamFlag = IR_CAMERA_STEP0;
        g_iLedOffFrameFlag = 0;
#endif
        WAIT_CAM_FRAME(1000, WaitIRTimeout);
    }
    return 0;
}

int FaceRecogTask::ProcessEnrollFile1Step()
{
    int ret = 0;
#if (USE_RENT_ENGINE)
    float arEngineResult[10] = { 0 };
    if (m_iCmd == E_REGISTER && g_xSS.iEFIFlag)
    {
        dbug_printf("[Color Image] Enroll\n");
        int width = 0, height = 0;

        dbug_printf("g_xSS.pbOtaData = %p, g_xSS.iUpgradeLen = %d\n", g_xSS.pbOtaData, g_xSS.iUpgradeLen);
        if (g_xSS.pbOtaData == NULL || g_xSS.iUpgradeLen <= (int)sizeof(int)*2)
        {
            arEngineResult[0] = ES_FAILED;
        }
        else
        {
            if (memcmp(g_xSS.pbOtaData, ENROLL_FACE_IMG_MAGIC, sizeof(ENROLL_FACE_IMG_MAGIC)) == 0)
            {
                memcpy(&width, g_xSS.pbOtaData + UF_HEADER_MAGIC_SIZE, 4);
                memcpy(&height, g_xSS.pbOtaData + UF_HEADER_MAGIC_SIZE + 4, 4);
                dbug_printf("w=%d, h=%d, sz=%d\n", width, height, UF_HEADER_MAGIC_SIZE);
                if (width * height <= 0 || width * height > IR_BUFFER_SIZE || g_xSS.iUpgradeLen - 8 < width*height*3)
                {
                    arEngineResult[0] = ES_FAILED;
                }
                else
                {
                    FaceEngine::RegisterImage(arEngineResult, g_xSS.pbOtaData + UF_HEADER_MAGIC_SIZE + 8, width, height);
                    dbug_printf("[Color Image] Engine State = %d\n", (int)arEngineResult[0]);
                }
            }
            else if (memcmp(g_xSS.pbOtaData, ENROLL_FACE_IMG_MAGIC2, sizeof(ENROLL_FACE_IMG_MAGIC2)) == 0)
            {
                xor_encrypt(g_xSS.pbOtaData + UF_HEADER_MAGIC_SIZE, g_xSS.iUpgradeLen - UF_HEADER_MAGIC_SIZE,
                            (unsigned char*)ENROLL_FACE_IMG_MAGIC2, sizeof(ENROLL_FACE_IMG_MAGIC2));
                FaceEngine::RegisterFeat(arEngineResult,
                                         g_xSS.pbOtaData + UF_HEADER_MAGIC_SIZE,
                                         g_xSS.iUpgradeLen - UF_HEADER_MAGIC_SIZE);
            }
            else
                arEngineResult[0] = ES_FAILED;
            free(g_xSS.pbOtaData);
            g_xSS.pbOtaData = NULL;
            g_xSS.iUpgradeLen = 0;
        }

        if(arEngineResult[0] == ES_SUCCESS)
        {
            m_iResult = FACE_RESULT_ENROLLED_FACE;
            ret = 1;
        }
        else
        {
            m_iResult = FACE_RESULT_TIMEOUT;
            ret = 1;
        }
    }
#endif // USE_RENT_ENGINE
    return ret;
}

extern "C" int uvc_media_update();

int FaceRecogTask::ProcessSaveFaceImage1Step()
{
    int ret = 0;
#if (USE_RENT_ENGINE)
    if (g_xSS.iSnapImageFace == 1 && m_isFaceDetected[FMI_FACE])
    {
#if 0
        int face_w = 160, face_h = 160;
        unsigned char* tmpImageBuffer = (unsigned char*)malloc(IR_BUFFER_SIZE);
        if (tmpImageBuffer)
        {
            fr_GetFaceRegionImage(tmpImageBuffer, face_w, face_h);
            g_xSS.iCapWidth = face_w;
            g_xSS.iCapHeight = face_h;
            SaveImage(tmpImageBuffer, g_xSS.msg_snap_image_data.start_number + m_iCaptureCount, 0,
                      face_w, face_h);
            free(tmpImageBuffer);
            m_iResult = FACE_RESULT_CAPTURED_FACE;
            ret = 1;
        }
        else
        {
            m_iResult = FACE_RESULT_TIMEOUT;
            ret = 1;
        }
#else
        uvc_media_update();
        m_iResult = FACE_RESULT_CAPTURED_FACE;
        ret = 1;
#endif
    }
#endif // USE_RENT_ENGINE
    return ret;
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