/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */
#include <errno.h>
#include <string.h>
#include "yoc/partition.h"
#include "yoc/partition_device.h"
#ifndef CONFIG_KERNEL_NONE
#include <devices/blockdev.h>
#else
#ifdef CONFIG_COMP_SDMMC
#include <mmc.h>
#endif
#endif /*CONFIG_KERNEL_NONE*/

#define CONFIG_PARTITION_DEVICE_MAX_NUM (CONFIG_PARTITION_SUPPORT_EFLASH + \
                                        CONFIG_PARTITION_SUPPORT_SPINORFLASH + \
                                        CONFIG_PARTITION_SUPPORT_SPINANDFLASH + \
                                        CONFIG_PARTITION_SUPPORT_EMMC + \
                                        CONFIG_PARTITION_SUPPORT_SD + \
                                        CONFIG_PARTITION_SUPPORT_USB)

#if CONFIG_PARTITION_DEVICE_MAX_NUM > 6
#error "CONFIG_PARTITION_DEVICE_MAX_NUM define error"
#endif

static int g_part_dev_num = 0;
static partition_device_ops_t *g_part_dev_ops[CONFIG_PARTITION_DEVICE_MAX_NUM];

int partition_device_register(partition_device_ops_t *dev_ops)
{
    if (!dev_ops) {
        return -EINVAL;
    }
    if (g_part_dev_num > CONFIG_PARTITION_DEVICE_MAX_NUM) {
        return -1;
    }
    for (int i = 0; i < g_part_dev_num; i++) {
        if (dev_ops->storage_info.type == g_part_dev_ops[i]->storage_info.type
            && dev_ops->storage_info.id == g_part_dev_ops[i]->storage_info.id) {
            // printf("%s, %d, register type:%d,id:%d already\n", __func__, __LINE__,
            //       dev_ops->storage_info.type, dev_ops->storage_info.id);
            return 0;
        }
    }
    g_part_dev_ops[g_part_dev_num] = (partition_device_ops_t *)dev_ops;
    g_part_dev_num++;
    return 0;
}

partition_device_ops_t *partition_device_find(storage_info_t *storage_info)
{
    partition_device_ops_t *p;

    if (!storage_info) {
        return NULL;
    }
    // printf("%s, %d, %d, %d, %d\n", __func__, __LINE__, storage_info->type, storage_info->id, storage_info->area);
    for (int i = 0; i < g_part_dev_num; i++) {
        p = g_part_dev_ops[i];
        if (p && p->storage_info.type == storage_info->type && p->storage_info.id == storage_info->id) {
            if (p->find) {
                p->dev_hdl = p->find(p->storage_info.id);
                p->storage_info.area = storage_info->area;
                return p;
            }
        }
    }
    // printf("%s, %d, %d, %d, find NULL.\n", __func__, __LINE__, storage_info->type, storage_info->id);
    return NULL;
}

int partition_device_close(partition_device_ops_t *dev_ops)
{
    partition_device_ops_t *p = dev_ops;

    if (!(dev_ops)) {
        return -EINVAL;
    }
    if (p->close) {
        return p->close(p->dev_hdl);
    }
    return -1;
}

int partition_device_info_get(partition_device_ops_t *dev_ops, partition_device_info_t *info)
{
    partition_device_ops_t *p = dev_ops;
    // printf("%s, %d, 0x%x, 0x%x\n", __func__, __LINE__, handle, info);
    if (!(dev_ops && info)) {
        return -EINVAL;
    }
    if (p->info_get) {
        int ret = p->info_get(p->dev_hdl, info);
        // printf("%s, %d, 0x%x, 0x%x\n", __func__, __LINE__, handle, ret);
        return ret;
    }
    // printf("%s, %d, 0x%x, 0x%x\n", __func__, __LINE__, handle, info);
    return -1;
}

int partition_device_read(partition_device_ops_t *dev_ops, off_t offset, void *data, size_t data_len)
{
    partition_device_ops_t *p = dev_ops;

    if (data_len == 0) {
        return 0;
    }

    if (!(dev_ops && data)) {
        return -EINVAL;
    }
    if (p->read) {
#if CONFIG_PARTITION_SUPPORT_EMMC
        if (p->storage_info.type == MEM_DEVICE_TYPE_EMMC) {
#ifndef CONFIG_KERNEL_NONE
            if (rvm_hal_blockdev_mmc_select_area(p->dev_hdl, p->storage_info.area)) {
                // printf("%s, %d, select area:%d failed.\n", __func__, __LINE__, p->storage_info.area);
                return -1;
            }
#else
            mmc_card_t *card = (mmc_card_t *)p->dev_hdl;
            if (card->currentPartition != p->storage_info.area) {
                if (MMC_SelectPartition(p->dev_hdl, p->storage_info.area)) {
                    // printf("%s, %d, select area:%d failed.\n", __func__, __LINE__, p->storage_info.area);
                    return -1;
                }
            }
#endif
        }
#endif /*CONFIG_PARTITION_SUPPORT_EMMC*/
        return p->read(p->dev_hdl, offset, data, data_len);
    }
    return -1;
}

int partition_device_write(partition_device_ops_t *dev_ops, off_t offset, void *data, size_t data_len)
{
    partition_device_ops_t *p = dev_ops;

    if (data_len == 0) {
        return 0;
    }

    if (!(dev_ops && data)) {
        return -EINVAL;
    }
    if (p->write) {
#if CONFIG_PARTITION_SUPPORT_EMMC
        if (p->storage_info.type == MEM_DEVICE_TYPE_EMMC) {
#ifndef CONFIG_KERNEL_NONE
            if (rvm_hal_blockdev_mmc_select_area(p->dev_hdl, p->storage_info.area)) {
                // printf("%s, %d, select area:%d failed.\n", __func__, __LINE__, p->storage_info.area);
                return -1;
            }
#else
            mmc_card_t *card = (mmc_card_t *)p->dev_hdl;
            if (card->currentPartition != p->storage_info.area) {
                if (MMC_SelectPartition(p->dev_hdl, p->storage_info.area)) {
                    // printf("%s, %d, select area:%d failed.\n", __func__, __LINE__, p->storage_info.area);
                    return -1;
                }
            }
#endif
        }
#endif /*CONFIG_PARTITION_SUPPORT_EMMC*/
        return p->write(p->dev_hdl, offset, data, data_len);
    }
    return -1;
}

int partition_device_erase(partition_device_ops_t *dev_ops, off_t offset, size_t len)
{
    partition_device_ops_t *p = dev_ops;

    if (len == 0) {
        return 0;
    }

    if (!dev_ops) {
        return -EINVAL;
    }
    if (p->erase) {
#if CONFIG_PARTITION_SUPPORT_EMMC
        if (p->storage_info.type == MEM_DEVICE_TYPE_EMMC) {
#ifndef CONFIG_KERNEL_NONE
            if (rvm_hal_blockdev_mmc_select_area(p->dev_hdl, p->storage_info.area)) {
                // printf("%s, %d, select area:%d failed.\n", __func__, __LINE__, p->storage_info.area);
                return -1;
            }
#else
            mmc_card_t *card = (mmc_card_t *)p->dev_hdl;
            if (card->currentPartition != p->storage_info.area) {
                if (MMC_SelectPartition(p->dev_hdl, p->storage_info.area)) {
                    // printf("%s, %d, select area:%d failed.\n", __func__, __LINE__, p->storage_info.area);
                    return -1;
                }
            }
#endif
        }
#endif /*CONFIG_PARTITION_SUPPORT_EMMC*/
        return p->erase(p->dev_hdl, offset, len);
    }
    return -1;
}