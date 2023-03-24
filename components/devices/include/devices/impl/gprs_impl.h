/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#ifndef _DEVICE_GPRS_MODULE_IMPL_H
#define _DEVICE_GPRS_MODULE_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <devices/impl/net_impl.h>
#include <devices/gprs.h>

typedef struct gprs_driver {
    /*common*/
    int (*set_mode)(rvm_dev_t *dev, rvm_hal_gprs_mode_t mode);
    int (*get_mode)(rvm_dev_t *dev, rvm_hal_gprs_mode_t *mode);
    int (*reset)(rvm_dev_t *dev);
    int (*start)(rvm_dev_t *dev);
    int (*stop)(rvm_dev_t *dev);
    /*configuration*/
    int (*set_if_config)(rvm_dev_t *dev, uint32_t baud, uint8_t flow_control);
    /*connection*/
    int (*module_init_check)(rvm_dev_t *dev);
    int (*connect_to_gprs)(rvm_dev_t *dev);
    int (*disconnect_from_gprs)(rvm_dev_t *dev);
    int (*get_link_status)(rvm_dev_t *dev, rvm_hal_gprs_status_link_t *link_status);
    int (*get_ipaddr)(rvm_dev_t *dev, char ip[16]);
    int (*sms_send)(rvm_dev_t *dev, char *sca, char *da, char *content);
    int (*sms_set_cb)(rvm_dev_t *dev, recv_sms_cb cb);
    int (*get_imsi)(rvm_dev_t *dev, char *imsi);
    int (*get_imei)(rvm_dev_t *dev, char *imei);
    int (*get_csq)(rvm_dev_t *dev, int *csq);
    int (*get_simcard_info)(rvm_dev_t *dev, char ccid[21], int *insert);
} gprs_driver_t;

#ifdef __cplusplus
}
#endif

#endif /*_WIFI_MODULE*/
