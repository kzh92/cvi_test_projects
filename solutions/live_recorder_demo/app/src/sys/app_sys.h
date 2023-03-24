/*
 * Copyright (C) 2019-2022 Alibaba Group Holding Limited
 */
#ifndef _APP_SYS_H_
#define _APP_SYS_H_

/* boot reason */
#define BOOT_REASON_POWER_ON        0
#define BOOT_REASON_SOFT_RESET      1
#define BOOT_REASON_POWER_KEY       2
#define BOOT_REASON_WAKE_STANDBY    3
#define BOOT_REASON_WIFI_CONFIG     4
#define BOOT_REASON_NONE            5

void app_sys_init();
void app_sys_reboot(int reason);
int app_sys_set_boot_reason(int reason);
int app_sys_get_boot_reason();
int app_sys_get_device_name(char dev_name[9]);

#define APPEXP_MODE_DEV 1
#define APPEXP_MODE_FCT 2
#define APPEXP_MODE_RLS 3
/**
 * @brief 系统异常处理
 * @param debug
 *        1:开发模式,启动软狗,异常后不复位
 *        2:开发模式,关闭软狗,产测模式
 *        其他:release模式，开启软狗及硬狗
 */
void app_sys_except_init(int debug);
#endif
