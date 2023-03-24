/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#ifndef __MESH_PROVISIONER_H_
#define __MESH_PROVISIONER_H_

#include "aos/ble.h"

#define DEVICE_NAME_MAX_LENGTH 28
#define CID_NVAL 0xFFFF
#define DEF_MAX_PROV_RETRY 5
#define DEF_DEV_REPORT_TIMEOUT (500)//ms
#define DEF_REPORT_DEV_SURVIVE_TIME (24000)//ms
#define DEF_REPORT_QUEUE_LENGTH (25)


typedef struct {
    uint8_t  uuid[16];
    uint8_t  dev_addr[6];
    uint8_t  addr_type;
    uint16_t prim_unicast;
    uint16_t oob_info;
    uint16_t net_idx;
    uint8_t  element_num;
    uint8_t  bearer;
    uint8_t flags;
    uint32_t iv_index;
    uint8_t *node_name;
} mesh_node_t;


typedef enum {
    BT_MESH_EVENT_RECV_UNPROV_DEV_ADV,
    BT_MESH_EVENT_PROV_COMP,
    BT_MESH_EVENT_FOUND_DEV_TIMEOUT,
    BT_MESH_EVENT_PROV_FAILED,
    BT_MESH_EVENT_PROV_FAILED_WITH_INFO,
    BT_MESH_EVENT_OOB_INPUT_NUM,
    BT_MESH_EVENT_OOB_INPUT_NUM_WITH_INFO,
    BT_MESH_EVENT_OOB_INPUT_STRING,
    BT_MESH_EVENT_OOB_INPUT_STRING_WITH_INFO,
    BT_MESH_EVENT_OOB_INPUT_STATIC_OOB,
} mesh_provisioner_event_en;



typedef void (*provisioner_cb)(mesh_provisioner_event_en event, void *p_arg);

typedef struct {
    uint8_t addr[6];
    uint8_t addr_type;
    uint8_t uuid[16];
    uint8_t size;
} oob_input_info_t;

typedef struct {
    uint8_t addr[6];
    uint8_t addr_type;
    uint8_t uuid[16];
    uint8_t reason;
    uint8_t bearer;
} prov_failed_info_t;


typedef struct {
    uint8_t *uuid;
    uint8_t uuid_length;
    uint8_t filter_start;
} uuid_filter_t;

typedef struct {
    dev_addr_t *addr;
    uint8_t     size;
} mac_filter_t;


typedef struct {
    uint16_t unicast_addr_start;
    uint16_t unicast_addr_end;
    uint8_t  attention_time;
    provisioner_cb cb;
} provisioner_config_t;

typedef struct {
    klist_t    list;
    dev_addr_t addr;
} mac_filters_dev;


typedef struct {
    struct bt_mesh_provisioner *provisioner;
} ble_mesh_provisioner_t;

int ble_mesh_provisioner_init(provisioner_config_t *param);

int ble_mesh_provisioner_enable();

int ble_mesh_provisioner_disable();

int ble_mesh_provisioner_dev_filter(uint8_t enable, uuid_filter_t *filter);

int ble_mesh_provisioner_mac_filter_enable();

int ble_mesh_provisioner_mac_filter_disable();

int ble_mesh_provisioner_mac_filter_clear();

int ble_mesh_provisioner_mac_filter_dev_add(uint8_t           mac_size,dev_addr_t *mac);

int ble_mesh_provisioner_mac_filter_dev_rm(uint8_t            mac_size,dev_addr_t *mac);

int ble_mesh_provisioner_show_dev(uint8_t enable, uint32_t timeout);

int ble_mesh_provisioner_dev_add(mesh_node_t *node, uint8_t add_appkey);

int ble_mesh_provisioner_get_add_appkey_flag(u16_t unicast_addr);

int ble_mesh_provisioner_dev_del(uint8_t addr[6], uint8_t addr_type, uint8_t uuid[16]);

int ble_mesh_provisioner_OOB_input_num(uint32_t num);

int ble_mesh_provisioner_OOB_input_string(const char *str);

int ble_mesh_provisioner_static_OOB_set(const uint8_t *oob, uint16_t oob_size);

mesh_node_t *ble_mesh_provisioner_get_node_info(u16_t unicast_addr);

const ble_mesh_provisioner_t *ble_mesh_provisioner_get_provisioner_data();

#endif

