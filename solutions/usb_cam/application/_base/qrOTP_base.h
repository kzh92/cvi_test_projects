#ifndef QROTP_BASE_H_
#define QROTP_BASE_H_

#include "appdef.h"

#if (USE_WIFI_MODULE)

#define OPT_LEN_MIN 0
#define SCAN_LIMIT_NUM 1

void    create_qrreader(int src_w, int src_h);
void    destroy_qrreader();
int     scan_candidate4OTP(unsigned char* src_buf, char* result);

#endif // USE_WIFI_MODULE

#endif
