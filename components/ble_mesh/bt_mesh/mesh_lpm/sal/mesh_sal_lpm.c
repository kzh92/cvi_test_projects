/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#include <stdint.h>
#include <string.h>
#include <mesh_sal_lpm.h>
#ifdef BOARD_PHY6220_EVB
#include "gpio.h"
#include <pm.h>
#endif

extern void drv_pm_sleep_enable();
extern void drv_pm_sleep_disable();
extern int  drv_pm_enter_standby();

void mesh_sal_sleep_enable()
{
    drv_pm_sleep_enable();
}

void mesh_sal_sleep_disable()
{
    drv_pm_sleep_disable();
}

int mesh_sal_sleep_enter_standby()
{
    return drv_pm_enter_standby();
}

int mesh_sal_sleep_wakup_io_set(uint8_t port, uint8_t pol)
{
#if defined(BOARD_TG7100B) || defined(BOARD_PHY6220_EVB) || defined(BOARD_TG7120B_EVB)
    uint8_t type = (pol >= 2) ? pol - 2 : pol;
    phy_gpio_pull_set(port, !type);
    phy_gpio_wakeup_set(port, type);
    return 0;
#endif
    return -1;
}

__attribute__((section(".__sram.code.mesh_sal_sleep_wakeup_io_get_status"))) int
mesh_sal_sleep_wakeup_io_get_status(uint8_t port)
{
#if defined(BOARD_TG7100B) || defined(BOARD_PHY6220_EVB) || defined(BOARD_TG7120B_EVB)
    return phy_gpio_read(port);
#endif
    return 0;
}

int mesh_sal_io_wakeup_cb_register(mesh_io_wakeup_cb cb)
{
    if (!cb) {
        return -1;
    }
#if defined(BOARD_TG7100B) || defined(BOARD_PHY6220_EVB) || defined(BOARD_TG7120B_EVB)
    drv_pm_io_wakeup_handler_register(cb);
#endif
    return 0;
}

int mesh_sal_io_wakeup_cb_unregister()
{
#if defined(BOARD_TG7100B) || defined(BOARD_PHY6220_EVB) || defined(BOARD_TG7120B_EVB)
    drv_pm_io_wakeup_handler_unregister();
#endif
    return 0;
}
