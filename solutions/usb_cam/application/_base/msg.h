#ifndef MSG_H
#define MSG_H

#include "message_queue.h"

enum MSG_TYPE
{
    MSG_KEY,
    MSG_BUTTON_UPDATE,
    MSG_BUTTON_CLICKED,
    MSG_WATCH,
    MSG_RECOG_FACE,
    MSG_RECOG_CARD,
    MSG_RECOG_FP,
    MSG_ERROR,
    MSG_CAMERA,
    MSG_BACK_RESP,
    MSG_LOGO_DOWN,
    MSG_UPG_DOWN,
    MSG_ZIGBEE,
    MSG_SOUND,
    MSG_FM,
    MSG_SENSE,
    MSG_VDB_TASK,
};

enum TOUCH_TYPE
{
    BUTTON_UPDATE,
    BUTTON_CLICKED
};

typedef struct _tagMSG
{
    int type;
    int data1;
    int data2;
    int data3;
    int touchType;
    int posX;
    int posY;
} MSG;

#define MAX_MSG_NUM 100

extern message_queue g_worker;
extern message_queue g_uart;

void SendGlobalMsg(int type, int data1, int data2, int data3);


#endif // MSG_H
