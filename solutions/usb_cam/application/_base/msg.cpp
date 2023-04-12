#include "msg.h"
#include "common_types.h"

message_queue g_worker;
message_queue g_uart;

void SendGlobalMsg(int type, long data1, long data2, long data3)
{
    MSG* msg = (MSG*)message_queue_message_alloc(&g_worker);
    if (msg == NULL)
    {
    	my_printf("[%s] alloc fail\n", __func__);
    	return;
    }
    msg->type = type;
    msg->data1 = data1;
    msg->data2 = data2;
    msg->data3 = data3;
    msg->time_ms = Now();
    message_queue_write(&g_worker, (MSG*)msg);
}
