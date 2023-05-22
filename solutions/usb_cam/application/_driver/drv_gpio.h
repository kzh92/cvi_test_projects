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

// #if (USE_SSD210)
// #define IR_LED          23 /* PAD_TTL0 */
// #define M24C64_WP       34 /* PAD_GPIO0 */
// #define PSENSE_DET      33 /* PAD_SD_GPIO0 */
// #define GPIO_USBSense   32 /* PAD_SD_GPIO1 */
// #else
// #if (!NFS_DEBUG_EN)
// #define IR_LED          17 /* PAD_TTL0 */
// #else
// #define IR_LED          16 /* PAD_TTL0 */
// #endif
// #define M24C64_WP       61 /* PAD_GPIO0 */
// #define PSENSE_DET      59 /* PAD_SD_GPIO0 */
// #define GPIO_USBSense   60 /* PAD_SD_GPIO1 */
// #define UART_EN     	44 /* PAD_KEY5 */
// #if (USE_WIFI_MODULE)
// #define IOCtl           43 /* PAD_KEY4 */
// #define SPI_CS          49 /* PAD_KEY10 */
// #define AUDIO_EN        69 /* PAD_GPIO8 */
// #endif // USE_WIFI_MODULE
// #endif

#if (DEFAULT_BOARD_TYPE == BD_TY_CV180xB_DEMO_V1v0)
#define IR_LED                  19 /* XGPIOA[19] Group:0 Num:19 */
#define CAM_PWDN		        213 /* XGPIOC_13 Group:2 Num:13*/
#elif (DEFAULT_BOARD_TYPE == BD_TY_FSDB_1V0)
#define IR_LED                  421 /* PWR_GPIO[21] Group:4 Num:21 */
#else // DEFAULT_BOARD_TYPE
	#error "Board Type Error!"
#endif // DEFAULT_BOARD_TYPE

#define IR_LED                  421 /* PWR_GPIO_21 Group:4 Num:21 */
// #define PSENSE_DET      106 /* XGPIOB_6 Group:1 Num:6*/
// #define GPIO_USBSense   106 /* XGPIOB_6 Group:1 Num:6*/
#define CAM_MIPI0_PWDN          208 /* XGPIOC[8] Group:2 Num:8*/
#define CAM_MIPI1_PWDN          207 /* XGPIOC[7] Group:2 Num:7*/

#define IN  0
#define OUT 1

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64

#define OFF     0
#define ON      1
#define FLICK   2

#if 0
//----------------------------------//
//       PORT BASE DEFINITIONS      //
//----------------------------------//

#define SUNXI_PORT_A_BASE      (0*0x24)
#define SUNXI_PORT_B_BASE      (1*0x24)
#define SUNXI_PORT_C_BASE      (2*0x24)
#define SUNXI_PORT_D_BASE      (3*0x24)
#define SUNXI_PORT_E_BASE      (4*0x24)
#define SUNXI_PORT_F_BASE      (5*0x24)
#define SUNXI_PORT_G_BASE      (6*0x24)
#define SUNXI_PORT_H_BASE      (7*0x24)
#define SUNXI_PORT_I_BASE      (8*0x24)


//----------------------------------//
//         PIO DEFINITIONS          //
//----------------------------------//

#define SUNXI_PIO_00           (0x00000001L <<  0)
#define SUNXI_PIO_01           (0x00000001L <<  1)
#define SUNXI_PIO_02           (0x00000001L <<  2)
#define SUNXI_PIO_03           (0x00000001L <<  3)
#define SUNXI_PIO_04           (0x00000001L <<  4)
#define SUNXI_PIO_05           (0x00000001L <<  5)
#define SUNXI_PIO_06           (0x00000001L <<  6)
#define SUNXI_PIO_07           (0x00000001L <<  7)
#define SUNXI_PIO_08           (0x00000001L <<  8)
#define SUNXI_PIO_09           (0x00000001L <<  9)
#define SUNXI_PIO_10           (0x00000001L <<  10)
#define SUNXI_PIO_11           (0x00000001L <<  11)
#define SUNXI_PIO_12           (0x00000001L <<  12)
#define SUNXI_PIO_13           (0x00000001L <<  13)
#define SUNXI_PIO_14           (0x00000001L <<  14)
#define SUNXI_PIO_15           (0x00000001L <<  15)
#define SUNXI_PIO_16           (0x00000001L <<  16)
#define SUNXI_PIO_17           (0x00000001L <<  17)
#define SUNXI_PIO_18           (0x00000001L <<  18)
#define SUNXI_PIO_19           (0x00000001L <<  19)
#define SUNXI_PIO_20           (0x00000001L <<  20)
#define SUNXI_PIO_21           (0x00000001L <<  21)
#define SUNXI_PIO_22           (0x00000001L <<  22)
#define SUNXI_PIO_23           (0x00000001L <<  23)
#define SUNXI_PIO_24           (0x00000001L <<  24)
#define SUNXI_PIO_25           (0x00000001L <<  25)
#define SUNXI_PIO_26           (0x00000001L <<  26)
#define SUNXI_PIO_27           (0x00000001L <<  27)
#define SUNXI_PIO_28           (0x00000001L <<  28)
#define SUNXI_PIO_29           (0x00000001L <<  29)
#define SUNXI_PIO_30           (0x00000001L <<  30)
#define SUNXI_PIO_31           (0x00000001L <<  31)



//----------------------------------//
//       CONSTANT DEFINITIONS       //
//----------------------------------//


#define SUNXI_SW_PORTC_IO_BASE  (0x01c20800)
#define SUNXI_GPIO_DATA_OFFSET  (0x10)
#define SUNXI_GPIO_PULL_OFFSET	(0x1C)
#define SUNXI_GPIO_INPUT        (0)
#define SUNXI_GPIO_OUTPUT       (1)

#define DISABLE 	0
#define FULL_UP		1
#define FULL_DOWN	2
#define RESERVED	3
// Debug function
//#define SUNXI_GPIO_DEBUG

//----------------------------------//
//        METHOD DEFINITIONS        //
//----------------------------------//

#define SUNXI_PIO_GET_BIT_INDEX(a) ( (a&SUNXI_PIO_00)?0:(a&SUNXI_PIO_01)?1:(a&SUNXI_PIO_02)?2:(a&SUNXI_PIO_03)?3:(a&SUNXI_PIO_04)?4:(a&SUNXI_PIO_05)?5:(a&SUNXI_PIO_06)?6:(a&SUNXI_PIO_07)?7:(a&SUNXI_PIO_08)?8:(a&SUNXI_PIO_09)?9:(a&SUNXI_PIO_10)?10:(a&SUNXI_PIO_11)?11:(a&SUNXI_PIO_12)?12:(a&SUNXI_PIO_13)?13:(a&SUNXI_PIO_14)?14:(a&SUNXI_PIO_15)?15:(a&SUNXI_PIO_16)?16:(a&SUNXI_PIO_17)?17:(a&SUNXI_PIO_18)?18:(a&SUNXI_PIO_19)?19:(a&SUNXI_PIO_20)?20:(a&SUNXI_PIO_21)?21:(a&SUNXI_PIO_22)?22:(a&SUNXI_PIO_23)?23:(a&SUNXI_PIO_24)?24:(a&SUNXI_PIO_25)?25:(a&SUNXI_PIO_26)?26:(a&SUNXI_PIO_27)?27:(a&SUNXI_PIO_28)?28:(a&SUNXI_PIO_29)?29:(a&SUNXI_PIO_30)?30:(a&SUNXI_PIO_31)?31:0)

int GPIO_fast_init();

int Get_addr_value(int iOffset);

void Set_addr_value(int iOffset, int iValue);

int GPIO_fast_config(int gpio, int inout);

int GPIO_fast_pull_config(int gpio, int value);

int GPIO_fast_setvalue(int gpio, int value);

int GPIO_fast_getvalue(int gpio);

int APB_fast_init();

unsigned int APB_fast_getvalue();

int APB_fast_setvalue(unsigned int value);
#else
int GPIO_fast_init();

void GPIO_fast_deinit();

int GPIO_fast_config(int gpio, int inout);

int GPIO_fast_setvalue(int gpio_pin, int value);

int GPIO_fast_getvalue(int gpio);
#define gpio_irled_on(on) GPIO_fast_setvalue(IR_LED, on)

#endif
#ifdef __cplusplus
}
#endif

#endif /* _DRV_GPIO_H */

