/*******************************************************************************
Project  : FaceKey(Foshan)
Module   : drv-gpio.h
Version  : 0.3
Date     : 2014.07.17.
Authors  : Liang HaiDong
Company  : EASEN
Comments :
Chip type: A10
*******************************************************************************/
#ifndef _DRV_GPIO_H
#define _DRV_GPIO_H

#include "appdef.h"

#ifdef __cplusplus
extern	"C"
{
#endif

#if (DEFAULT_BOARD_TYPE == BD_TY_CV180xB_DEMO_V1v0)
#define IR_LED                  420 /* PWR_GPIO[20] Group:4 Num:20 */
#define CAM_MIPI0_PWDN          208 /* XGPIOC[8] Group:2 Num:8*/
#define CAM_MIPI1_PWDN          207 /* XGPIOC[7] Group:2 Num:7*/
#elif (DEFAULT_BOARD_TYPE == BD_TY_FSDB_1V0)
#define IR_LED                  421 /* PWR_GPIO[21] Group:4 Num:21 */
#define CAM_MIPI0_PWDN          208 /* XGPIOC[8] Group:2 Num:8*/
#define CAM_MIPI1_PWDN          207 /* XGPIOC[7] Group:2 Num:7*/
#elif (DEFAULT_BOARD_TYPE == BD_TY_CV181xC_DEMO_V1v0)
#define IR_LED                  19 /* XGPIOA[19] Group:0 Num:19 */
#define WHITE_LED               18 /* XGPIOA[18] Group:0 Num:18 */
#define CAM_MIPI0_PWDN		    213 /* XGPIOC_13 Group:2 Num:13*/
#define CAM_MIPI1_PWDN          CAM_MIPI0_PWDN
//#define PSENSE_DET              419 /*PWR_GPIO[19] Group:4 Num:19 */
#elif (DEFAULT_BOARD_TYPE == BD_TY_FMDASS_1V0J)
#define IR_LED                  19 /* XGPIOA[19] Group:0 Num:19 */
#define WHITE_LED               18 /* XGPIOA[18] Group:0 Num:18 */
#define CAM_MIPI0_PWDN          217 /* XGPIOC_17 Group:2 Num:17*/
#define CAM_MIPI1_PWDN          216 /* XGPIOC_17 Group:2 Num:16*/
#define PSENSE_DET              419 /*PWR_GPIO[19] Group:4 Num:19 */
#define AUDIO_EN                15 /* XGPIOA[15] Group:0 Num:15*/
#else // DEFAULT_BOARD_TYPE
	#error "Board Type Error!"
#endif // DEFAULT_BOARD_TYPE

#define IN  0
#define OUT 1

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64

#define OFF     0
#define ON      1
#define FLICK   2

int GPIO_fast_init();
void GPIO_fast_deinit();
int GPIO_fast_config(int, int);
int GPIO_fast_setvalue(int gpio_pin, int value);
int GPIO_fast_getvalue(int);
#define gpio_irled_on(on) GPIO_fast_setvalue(IR_LED, on)
#define gpio_whiteled_on(on) GPIO_fast_setvalue(WHITE_LED, on)

#ifdef __cplusplus
}
#endif

#endif /* _DRV_GPIO_H */

