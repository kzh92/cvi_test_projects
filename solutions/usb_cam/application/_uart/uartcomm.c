#include "uartcomm.h"
#include "appdef.h"
//#include "drv_uart_api.h"
#include "common_types.h"

#include <string.h>

#include "vm_types.ht"
#include "sys_vm_v24.hi"
#include "hal_fuart.h"
#include "cam_os_wrapper.h"
#include "drv_gpio.h"

//#define USE_UART_READ
//#define PRINT_DEBUG

/**
 * @brief UART port
 *
 *  Used in all uart functions.
 */
typedef enum
{
    UART_1,
    UART_2,
    FUART,
    NB_UART
} Uart_Num_t;

/**
 * @brief Type of UART Indication
 *
 *  Used in @ref UartCallbackInd_t callback function to specify the type of UART Indication.
 */
typedef enum 
{
    UART_BREAK_IND=0,
    UART_OVERRUN_IND,
    UART_MSR_CHANGE_IND,
    UART_RX_FLOW_CTRL_IND
} Uart_Ind_t;

/**
 * @brief Data Callback function type
 *
 *  Used in @ref Uart_cfg_t structure to specify a callback for RX or TX data events.
 */
typedef void (* UartCallbackData_t)(Uart_Num_t num, u8 *buffer, u8 size);

/**
 * @brief Indication Callback function type
 *
 *  Used in @ref UartCallbackInd_t callback function to specify the callback used for UART non data events.
 */
typedef void (* UartCallbackInd_t)(Uart_Num_t num, Uart_Ind_t ind);

/**
 * @brief UART configuration type
 *
 *  Used in @ref uart_Open function to specify the parameters of the UART to use.
 */
typedef struct
{
    vm_v24_SerialLength_t   length;             ///< length in number of bits
    vm_v24_SerialParity_t   parity;     ///< parity
    vm_v24_SerialStop_t     stop;   ///< stop bit
    vm_v24_Rate_t           rate;   ///< bit rate
    vm_v24_FlowControl_t    RxFlowCtrlMethod;   ///< type of Rx flow control
    vm_v24_FlowControl_t    TxFlowCtrlMethod;   ///< type of Tx flow control
    u8                      signal_mask;        ///< signals to use
    UartCallbackData_t      rxCallback;         ///< callback called when data received in hardware FIFO
    UartCallbackData_t      txCallback;         ///< callback called when all data of software FIFO sent
    UartCallbackInd_t       EvtCallback;            ///< callback called when UART event raise of type @ref Uart_Ind_t
} Uart_cfg_t;

extern volatile FURUart_t * const FURUart[NB_UART];

/****************************************************************************************************/
/*************************** V24 Action Macros on Uart Signals **************************************/
/****************************************************************************************************/

#define UART_GET_HAL_NUM(Uart_num)      (Uart_num+HAL_UART1)

#define UART_GET_OBJ(Uart_num)          HAL_UART_GET_OBJ(UART_GET_HAL_NUM(Uart_num))

#define UART_SET_RTS_ON(Uart_num)       HAL_UART_SET_RTS_ON(UART_GET_OBJ(Uart_num))

#define UART_SET_RTS_OFF(Uart_num)      HAL_UART_SET_RTS_OFF(UART_GET_OBJ(Uart_num))

#define UART_SET_DTR_ON(Uart_num)       HAL_UART_SET_DTR_ON(UART_GET_OBJ(Uart_num))

#define UART_SET_DTR_OFF(Uart_num)      HAL_UART_SET_DTR_OFF(UART_GET_OBJ(Uart_num))

#define UART_IS_CTS_CHANGE( msr, available )      HAL_UART_IS_CTS_CHANGE( msr, available )
#define UART_IS_DSR_CHANGE( msr )       HAL_UART_IS_DSR_CHANGE(msr)

#define UART__IS_CTS_ON( msr, available )  HAL_UART__IS_CTS_ON( msr, available )
#define UART__IS_DSR_ON( msr )          HAL_UART__IS_DSR_ON(msr)

#define UART_IS_CTS_ON(Uart_num )       HAL_UART_IS_CTS_ON(UART_GET_OBJ(Uart_num))

/****************************************************************************************************/
/*************************** V24 general functionnalities MACROS ************************************/
/****************************************************************************************************/

#define UART_CLEAR_IIR( Uart_num )      //(uart_WriteIIR_Clear(HAL_UART_GET_OBJ(Uart_num),0))

#define UART_IS_BREAK( lsr )        HAL_UART_IS_BREAK( lsr )
#define UART_IS_OVERRUN( lsr )      HAL_UART_IS_OVERRUN( lsr )

#define UART_SET_IT_MASK( Uart_num, m ) HAL_UART_SET_IT_MASK( UART_GET_OBJ(Uart_num), m )
#define UART_SET_IT_SRC( Uart_num, m )  HAL_UART_SET_IT_SRC( UART_GET_OBJ(Uart_num), m)
#define UART_CLR_IT_SRC( Uart_num, m )  HAL_UART_CLR_IT_SRC( UART_GET_OBJ(Uart_num), m )

#define UART_RX_FIFO_NOT_EMPTY( Uart_num )  HAL_UART_RX_FIFO_NOT_EMPTY( UART_GET_OBJ(Uart_num))
#define UART_RX_FIFO_EMPTY(Uart_num)        HAL_UART_RX_FIFO_EMPTY(UART_GET_OBJ(Uart_num))

/**
 * @brief following macro tests transmit register empty bit. If so it means that
 * fifo (or transmit holding reg) is also empty.
*/
#define UART_TX_FIFO_EMPTY( Uart_num )      HAL_UART_TX_FIFO_EMPTY(UART_GET_OBJ(Uart_num))

#define UART_CLR_RX_FIFO( Uart_num, lvl )   HAL_UART_CLR_RX_FIFO( UART_GET_OBJ(Uart_num), lvl )
#define UART_CLR_TX_FIFO( Uart_num, lvl )   HAL_UART_CLR_TX_FIFO( UART_GET_OBJ(Uart_num), lvl )

/**
 * @brief CAUTION: following two macro clear also the trigger level
*/
#define UART_ENABLE_FIFOs( Uart_num )       HAL_UART_ENABLE_FIFOs( UART_GET_OBJ(Uart_num) )
#define UART_RESET_FIFOs( Uart_num )        HAL_UART_RESET_FIFOs( UART_GET_OBJ(Uart_num))

#define UART_SET_TRIGGER_LVL( Uart_num,lvl )    HAL_UART_SET_TRIGGER_LVL( UART_GET_OBJ(Uart_num),lvl )

/****************************************************************************************************/
/*************************** V24 special functionnalities MACROS ************************************/
/****************************************************************************************************/
#define UART_START_BREAK( Uart_num )   HAL_UART_START_BREAK(UART_GET_OBJ(Uart_num))
#define UART_STOP_BREAK( Uart_num )    HAL_UART_STOP_BREAK(UART_GET_OBJ(Uart_num))


#define UART_RX_IT_BUF_LEN     256   /* must be a power of 2 */
#define UART_TX_IT_BUF_LEN     384

s32 uart_Open( Uart_Num_t uart_num, Uart_cfg_t *uart_cfg );
void uart_SetRate(Uart_Num_t uart_num, vm_v24_Rate_t Rate );
void uart_SetFraming( Uart_Num_t uart_num, vm_v24_Framing_t Framing );
s32 uart_Write( Uart_Num_t uart_num, u8 *string,  u32 length);
s32 uart_Read( Uart_Num_t uart_num, u8 *string,  u32 length);
s32 uart_GetRxFifoLevel( Uart_Num_t uart_num);
bool uart_IsTxFifoEmpty( Uart_Num_t uart_num);
s32 uart_ClearRxBuffer( Uart_Num_t uart_num);

#if defined(__SYSDRIVER_REDUCE_MEM_USAGE__)
#define UART_RX_IT_FC_THRES   32
#else
#define UART_RX_IT_FC_THRES   64
#endif

#define UART_XOFF_SENT          ((u8)1<<0)
#define UART_XON_SENT           ((u8)1<<1)

#define UART_XOFF 0xFF

typedef struct
{
    u8          *TxBuffer;
    u8          RxBuffer[UART_RX_IT_BUF_LEN];

    u32         RxFifoPurgeIndex;
    u32         RxFifoFillIndex;
    u32         TxFifoFillIndex;
    u32         TxFifoPurgeIndex;
    u8          FillingSem; /* bit field */
    bool        UartTxItRunning;
    u8          RxFlowCtrl;
    Uart_cfg_t  cfg;
} UartContext_t;

UartContext_t UartContext_tab[NB_UART];

static void uart_IntBarReceiving( Uart_Num_t uart_num );

Uart_Num_t g_uartNums[2] = {UART_1, UART_2};

#define UART_FIFO_LEN   16
/*
 *
 *   @fn void TxUartHandler (void)
 *
 *   @brief Tx UART handler
 *
 *
 *
 *   @param [in] UartContext  Describes Uart/Port on which to apply the request
 *
 *   @return void
 */
void TxUartHandler( Uart_Num_t uart_num )
{
    UartContext_t *UartContext = &(UartContext_tab[uart_num]);
    //
    // 'TxFifo' is _not_ a circular buffer. Basically when all bytes stored
    // in this buffer have been written in the UART, fill and purge indexes are
    // reset to zero. By doing so, applying a 256-modulo on indexes is no longer
    // required. Futhermore it allows the plain use of the post-indexed
    // addressing mode of ARM resulting in a significant increase in speed.
    //
    // C version (on target, takes up to 14 us to fill 16 bytes in fifo)
    //
    u8 * src = (u8 *) &UartContext->TxBuffer[ UartContext->TxFifoPurgeIndex ];
    u32 NbToTx = UartContext->TxFifoFillIndex - UartContext->TxFifoPurgeIndex;

    // On target, uart fifo length is worth 16 bytes, whereas
    // on PC it's worth 1 or 16 depending on uart fifo capabilities.
    //
    if ( NbToTx > UART_FIFO_LEN )
        NbToTx = UART_FIFO_LEN;

    switch (NbToTx)
    {
    case 16:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
    case 15:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
    case 14:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
    case 13:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
    case 12:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
    case 11:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
    case 10:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
    case 9:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
    case 8:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
    case 7:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
    case 6:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
    case 5:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
    case 4:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
    case 3:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
    case 2:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
    case 1:
        hal_uart_write_char(UART_GET_OBJ(uart_num),*src++);
        UartContext->TxFifoPurgeIndex += NbToTx;

        if ( (UartContext->TxFifoFillIndex == UartContext->TxFifoPurgeIndex) && !UartContext->FillingSem )
        {
            UartContext->TxFifoFillIndex = UartContext->TxFifoPurgeIndex = 0;
        }
        UartContext->UartTxItRunning = TRUE;
        break;
    case 0:
        UartContext->UartTxItRunning = FALSE;
        if (UartContext->cfg.txCallback != NULL)
            UartContext->cfg.txCallback(uart_num, UartContext->TxBuffer, UartContext->TxFifoFillIndex);
        break;
    }
}

/**
 * @brief Uart interrupt routine. UART ONLY
 * @param [in] UartContext  Describes Uart/Port on which to apply the request
 * @return None.
 */
static void CtxIsr( Uart_Num_t uart_num )
{
    UartContext_t *UartContext = &(UartContext_tab[uart_num]);
    u32 Source;
    u8 count=0, retry=100;

    // To work properly on target this routine _must_ be called with all IRQs
    // disabled. Moreover the FINT1 (Frame interrupt on IRQ1) must _not_ disable
    // uart interrupts to react as fast as possible.
    //
    // The time period from entrance to interrupt to this stage is 25 us.
    Source = HAL_UART_GET_IT_SOURCE( UART_GET_OBJ(uart_num) );

    while ( HAL_UART_IS_IT_PENDING( Source ) )
    {
        if ( HAL_IS_UART_RX_FIFO_AVALIABLE(Source))
        {
            //
            // Read the uart fifo out
            //

            //
            // Fast reading. takes 62 us* to read 14 bytes. Only for target
            // Don't alter it, as it has been finely optimized for ARM.
            // *: from entry to this function to exit from it.
            // The read duration is only valid when indexes are u8 (when
            // interrupt buffer is 256, or less, bytes long).
            // In support of u32 indexes, add 7us.
            //
            u8*  Start, *RxFifo = UartContext->RxBuffer+UartContext->RxFifoFillIndex;
            u32    Count  = HUC_UART_RX_FIFO_SIZE;  // to read out 16 Bytes from the uart fifo

            Start = RxFifo;
            //
            // if 'Count' was init'ed to 1 and 'READ()' was called 16 times
            // then time would be decreased by 3us. Therefore I think it's not
            // worth taking twice as much memory just for the sake of 3us...
            //
            do
            {
                if ( UART_RX_FIFO_NOT_EMPTY( uart_num ) )
                {
                    *RxFifo++= hal_uart_read_char(UART_GET_OBJ(uart_num) );
                    UartContext->RxFifoFillIndex++;
                }
                else
                    break;
            }
            while ( --Count );

            // call RX callbackprovided by the client
            if (UartContext->cfg.rxCallback != NULL)
                UartContext->cfg.rxCallback(uart_num, Start, RxFifo-Start);

            // Compute how much room is left now in the receive buffer.
            {
                u32 NewRoomLeft = UART_RX_IT_BUF_LEN - UartContext->RxFifoFillIndex;

                if ( NewRoomLeft < UART_RX_IT_FC_THRES )
                {
                    // The interrupt receive buffer has reached the threshold so
                    // invoke flow control toward host.
                    uart_IntBarReceiving( uart_num );
                }
            }
        }
        else if (HAL_IS_UART_TX_FIFO_EMPTY(Source))
        {
            //must be called before TxUartHandler of course
            TxUartHandler(uart_num);
        }
        else if (HAL_IS_UART_LSR_EVENT(Source)) // error or break detected
        {
            Source = hal_uart_ReadLSR(UART_GET_OBJ(uart_num));
            if ( UART_IS_BREAK( Source ) )
                // call break callback provided by the client
                if (UartContext->cfg.EvtCallback != NULL)
                    UartContext->cfg.EvtCallback(uart_num, UART_BREAK_IND);

            if ( UART_IS_OVERRUN( Source ) )
                // call overrun callback provided by the client
                if (UartContext->cfg.EvtCallback != NULL)
                    UartContext->cfg.EvtCallback(uart_num, UART_OVERRUN_IND);

        }
        else if (HAL_IS_UART_MSR_EVENT(Source))
        {
            //UART_CLR_IT_SRC(uart_num,UART_MSR_EVENT);
            Source = hal_uart_ReadMSR(UART_GET_OBJ(uart_num));
            if (UartContext->cfg.EvtCallback != NULL)
                UartContext->cfg.EvtCallback(uart_num, UART_MSR_CHANGE_IND);
        }
        else if( HAL_IS_UART_BUSY_DETECT(Source)) /* Busy detect indication */
        {
            // Read USR to clear
            Source = hal_uart_ReadUSR(UART_GET_OBJ(uart_num));

            while((HAL_IS_UART_BUSY_DETECT(HAL_UART_GET_IT_SOURCE( UART_GET_OBJ(uart_num)))) && (count < retry))
            {
                // Read USR to clear
                Source = hal_uart_ReadUSR(UART_GET_OBJ(uart_num));
                count++;
            }
            if (count == retry)
                UartSendTrace("UART Interrupt: UART_IIR_BUSY\n");
        }
        else
        {
            //clear the source...
            UartSendTrace("UART Interrupt: unknown interrupt %ld\n", Source);
        }
        Source = HAL_UART_GET_IT_SOURCE( UART_GET_OBJ(uart_num) );
    }
}

/**
 * @brief Called under uart interrupt, whenever the interrupt receive
 * buffer is almost full, to drop RTS and send XOFF according to
 * flow control settings.
 * In order to run as fast as possible it bypasses functions
 * called in idle-task.
 * @param [in] uart_num  Describes Uart/Port on which to apply the request
 * @return None.
 */
static void uart_IntBarReceiving( Uart_Num_t uart_num )
{
    UartContext_t *UartContext = &(UartContext_tab[uart_num]);

    if ( UartContext->cfg.RxFlowCtrlMethod & VM_V24_HARD)
    {
        UART_SET_RTS_OFF( uart_num );
        //Doing that makes sure that RxIt buffer won't be overloaded under Rx IT.
        //When Bluetooth can't accept more data it simply stops Polling in Idle task the
        //Rx It buffer so that we need to leave RTS down (Before we just set RTS On each time
        //we were going in Idle Task assuming the Rx it Buffer will be polled : it is not more the case...)
    }
    if (UartContext->cfg.RxFlowCtrlMethod & VM_V24_SOFT)
    {
        //
        // Emergency case: if the send uart fifo is not empty then it's
        // simply cleared to ensure that XOFF will really be sent.
        // I agree that it's a little bit rough though usually
        // software flow control operates on half-duplex data transfers,
        // entailing the send fifo is usually empty when receiving.
        // If not then PC application should cope with data loss...
        //
        if ( (UartContext->RxFlowCtrl & UART_XOFF_SENT) == 0 )
        {
            if ( !UART_TX_FIFO_EMPTY( uart_num ) )
                UART_CLR_TX_FIFO( uart_num, HUC_UART_TRIGGER_LEVEL );
            //uart_WriteChar( uart_num, UART_XOFF );
            hal_uart_write_char(UART_GET_OBJ(uart_num),UART_XOFF);

            UartContext->RxFlowCtrl &= ~UART_XON_SENT;
            UartContext->RxFlowCtrl |= UART_XOFF_SENT;
        }
    }
    // call break callback provided by the client // to force a RTS raise in idle-task.
    if (UartContext->cfg.EvtCallback != NULL)
        UartContext->cfg.EvtCallback(uart_num, UART_RX_FLOW_CTRL_IND);

}

/**
 * @brief Uart interrupt routine.
 * @return None.
 */
void IsrUart1( void )
{
    CtxIsr( UART_1);
}

/**
 * @brief Uart interrupt routine.
 * @return None.
 */
void IsrUart2( void )
{
    CtxIsr( UART_2 );
}

void IsrFuart( void )
{
    CtxIsr( FUART );
}

/**
 *
 *   @fn s32 uart_Open( Uart_Num_t uart_num, Uart_cfg_t *uart_cfg );
 *
 *   @brief Open the given UART with the given config
 *
 *   @param [in] uart_num:  Describes Uart/Port on which to apply the request
 *   @param [in] uart_cfg:  configuretion to use with the given uart
 *
 *   @return void
 */
s32 uart_Open( Uart_Num_t uart_num, Uart_cfg_t *uart_cfg )
{
    memcpy(&(UartContext_tab[uart_num].cfg), uart_cfg, sizeof(Uart_cfg_t));

    uart_SetRate(uart_num,uart_cfg->rate);
    uart_SetFraming(uart_num,VM_V24_FRAMING(uart_cfg->length, uart_cfg->parity, uart_cfg->stop));

    HAL_UART_SET_RTS_OFF(UART_GET_OBJ(uart_num));

    UART_ENABLE_FIFOs(uart_num);

    UART_SET_TRIGGER_LVL(uart_num, HUC_UART_TRIGGER_LEVEL); // UART_FCR_TRIGGER_RX_L3 | UART_FCR_TRIGGER_TX_L0

    UartContext_tab[uart_num].TxFifoFillIndex = 0;
    UartContext_tab[uart_num].TxFifoPurgeIndex = 0;
    UartContext_tab[uart_num].RxFifoFillIndex = 0;
    UartContext_tab[uart_num].RxFifoPurgeIndex = 0;
    UartContext_tab[uart_num].FillingSem  = FALSE;

    // Enable all interrupt sources on target (namely RX, TX, LSR & MSR).
    // UART_SET_IT_MASK(uart_num, HUC_V24_IT_MASK );
    // UART_CLEAR_IIR( uart_num);

    // switch(uart_num)
    // {
    // case UART_1:
    //     hal_uart_int_init(HAL_UART1, (PfnIntcISR)IsrUart1);
    //     hal_uart_int_unmask(HAL_UART1);
    //     break;
    // case UART_2:
    //     hal_uart_int_init(HAL_UART2, (PfnIntcISR)IsrUart2);
    //     hal_uart_int_unmask(HAL_UART2);
    //     break;
    // case FUART:
    //     hal_uart_int_init(HAL_FUART, (PfnIntcISR)IsrFuart);
    //     hal_uart_int_unmask(HAL_FUART);
    //     break;
    // default:
    //     break;
    // }

    // UART_SET_IT_SRC( uart_num, UART_IER_RDICTI | UART_IER_RLSI );

#ifdef __BARCODE__
    UART_SET_RTS_OFF( uart_num );
#else
    UART_SET_RTS_ON( uart_num );
#endif

    return uart_num;
}

/**
 *
 *   @fn void uart_SetRate(Uart_Num_t uart_num, vm_v24_Rate_t Rate );
 *
 *   @brief Set the baud rate for the given uart
 *
 *   @param [in] uart_num  Describes Uart/Port on which to apply the request
 *   @param [in] Rate  new baud rate or autobaud
 *   @return None.
 */
void uart_SetRate(Uart_Num_t uart_num, vm_v24_Rate_t Rate )
{
    UartContext_t *UartContext = &(UartContext_tab[uart_num]);
#ifdef SUPPORT_UART_SET_HW
    hal_uart_enable_clk(UART_GET_HAL_NUM(uart_num));
    hal_uart_set_padmux(UART_GET_HAL_NUM(uart_num));
#endif
    hal_uart_set_rate(UART_GET_HAL_NUM(uart_num),Rate);

    UartContext->cfg.rate = Rate;
}

/**
 *
 *   @fn void uart_SetFraming( Uart_Num_t uart_num, vm_v24_Framing_t Framing );
 *
 *   @brief Set the framing type for the given uart
 *
 *   @param [in] uart_num  Describes Uart/Port on which to apply the request
 *   @param [in] Framing  new framing
 *   @return none
 */
void uart_SetFraming( Uart_Num_t uart_num, vm_v24_Framing_t Framing )
{
    hal_uart_set_framing(UART_GET_HAL_NUM(uart_num),Framing);
}

/**
 *
 *   @fn const s32 uart_Write( Uart_Num_t uart_num, u8 *string,  u32 length);
 *
 *   @brief Write data in the given UART. This function returns
 *
 *   @param [in] uart_num  Describes Uart/Port on which to apply the request
 *   @param [out] string  pointer on the string to write
 *   @param [in] length  number of char to write in the given uart
 *
 *   @return length
 */
s32 uart_Write( Uart_Num_t uart_num, u8 *string,  u32 length)
{
    UartContext_t *UartContext = &(UartContext_tab[uart_num]);

    // check if a transfer is currently ongoing
    if (UartContext->TxFifoFillIndex > 0)
        return -1;

    UartContext->TxBuffer = string+1;
    UartContext->TxFifoFillIndex = length-1;
    UartContext->TxFifoPurgeIndex= 0;

    // send first char to trigger the Tx Fifo empty IRQ
    hal_uart_write_char(UART_GET_OBJ(uart_num),*string);

    return length;
}

/**
 *
 *   @fn const s32 uart_Read( Uart_Num_t uart_num, u8 *string,  u32 length);
 *
 *   @brief Read data from the given UART. The given string must have been allocated by the caller
 *
 *   @param [in] uart_num  Describes Uart/Port on which to apply the request
 *   @param [out] string  pointer on the string to read
 *   @param [in] length  max length to read from the given uart
 *
 *   @return number of character really read from the UART
 */
s32 uart_Read( Uart_Num_t uart_num, u8 *string,  u32 length)
{
    UartContext_t *UartContext = &(UartContext_tab[uart_num]);

    // check if the Rx Fifo is not empty
    if ((UartContext->RxFifoFillIndex == 0) || (string== 0))
        return 0;

    if (length > UartContext->RxFifoFillIndex)
        length = UartContext->RxFifoFillIndex;

    // during this operation, let's mask Rx IRQ
    HAL_UART_CLR_IT_SRC(UART_GET_OBJ(uart_num),UART_IER_RDICTI);
    UartContext->RxFifoFillIndex -= length;
    memcpy(string, UartContext->RxBuffer,length);

    // if there are still data remaining in the Fifo, let's place it on the top of it
    if (UartContext->RxFifoFillIndex > 0)
    {
        memmove(UartContext->RxBuffer, &(UartContext->RxBuffer[length]),UartContext->RxFifoFillIndex);
    }

    // after this operation, let's unmask Rx IRQ
    HAL_UART_SET_IT_SRC(UART_GET_OBJ(uart_num),UART_IER_RDICTI);

    return length;
}

/**
 *
 *   @fn const s32 uart_GetRxFifoLevel( Uart_Num_t uart_num);
 *
 *   @brief Returns the number of cher currently in the RX fifo
 *
 *   @param [in] uart_num  Describes Uart/Port on which to apply the request
 *
 *   @return number of character in the Rx Fifo of the given UART UART
 */
s32 uart_GetRxFifoLevel( Uart_Num_t uart_num)
{
    UartContext_t *UartContext = &(UartContext_tab[uart_num]);

    return (UartContext->RxFifoFillIndex);
}

/**
 *
 *   @fn const bool uart_IsTxFifoEmpty( Uart_Num_t uart_num);
 *
 *   @brief Returns TRUE if the physical TX Fifo is empty
 *
 *   @param [in] uart_num  Describes Uart/Port on which to apply the request
 *
 *   @return TRUE if the physical TX Fifo is empty
 */
bool uart_IsTxFifoEmpty( Uart_Num_t uart_num)
{
    UartContext_t *UartContext = &(UartContext_tab[uart_num]);

    if (( UartContext->TxFifoFillIndex != 0 )|| (!UART_TX_FIFO_EMPTY( uart_num )))
        return FALSE;

    return TRUE;
}

/**
 *
 *   @fn const s32 uart_ClearRxBuffer( Uart_Num_t uart_num);
 *
 *   @brief Clear the RX buffer of the given UART. Typically called by the client in it's RX callback
 *
 *   @param [in] uart_num  Describes Uart/Port on which to apply the request
 *
 *   @return UART_OK if OK
 */
s32 uart_ClearRxBuffer( Uart_Num_t uart_num)
{
    UartContext_t *UartContext = &(UartContext_tab[uart_num]);

    //my_printf((_HWL | LEVEL_31, "uart_ClearRxBuffer, RxFifoFillIndex = %d",UartContext->RxFifoFillIndex));
    UartContext->RxFifoFillIndex = 0;

    return 0;
}

#if 0
int uart_fd;
struct termios uart_oldtio,uart_newtio;

#define UART_BAUDRATE         (B115200)
#define UART_MODEMDEVICE      "/dev/ttyS1"

#ifndef __min
#define __min(a,b)  ((a) > (b) ? (b) : (a))
#endif  //	MIN

#ifndef __max
#define __max(a,b)  ((a) < (b) ? (b) : (a))
#endif  //	MAX
#endif

void init_uart(int idx)
{
    if (idx < 0 || idx >= sizeof(g_uartNums) / sizeof(Uart_Num_t))
        return;

    Uart_cfg_t uart_cfg = {0};

    //uart_cfg.EvtCallback = NULL;
    uart_cfg.length = VM_V24_8BIT_LENGTH;
    uart_cfg.parity = VM_V24_NO_PARITY;
    uart_cfg.stop = VM_V24_1STOP_BIT;
    uart_cfg.rate = VM_V24_115200;
    //uart_cfg.signal_mask = (VM_V24_SIGNAL_RX | VM_V24_SIGNAL_TX |VM_V24_SIGNAL_RTS |VM_V24_SIGNAL_CTS);
    //uart_cfg.rxCallback = uart_rxCallback;
    //uart_cfg.txCallback = uart_txCallback;
    uart_Open(g_uartNums[idx], &uart_cfg);

    my_usleep(10);
}

void init_uart1(void)
{
    init_uart(0);
}

void set_rate_uart(int idx, int rate)
{
    if (idx < 0 || idx >= sizeof(g_uartNums) / sizeof(Uart_Num_t))
        return;
    if (rate == 115200)
        uart_SetRate(g_uartNums[idx], VM_V24_115200);
    else if (rate == 230400)
        uart_SetRate(g_uartNums[idx], VM_V24_230400);
    else if (rate == 460800)
        uart_SetRate(g_uartNums[idx], VM_V24_460800);
    else
        uart_SetRate(g_uartNums[idx], VM_V24_115200);

    my_usleep(10);
}

void set_rate_uart1(int rate)
{
    set_rate_uart(0, rate);
}

int recv_buffer(int idx, char* buf, int len, int ms_timeout, int us_interval)
{
#ifndef USE_UART_READ
  char rx_data = 0;
  int index = 0;
  CamOsTimespec_t s;
  float f_starttime, f_endtime;
  while (1)
  {
    if (index >= len)
      break;
    CamOsGetMonotonicTime(&s);
    f_starttime = s.nSec*1000.f + s.nNanoSec/1000000.f;
    while(HAL_UART_RX_FIFO_EMPTY(UART_GET_OBJ(g_uartNums[idx]))) // wait for a char
    {
      CamOsGetMonotonicTime(&s);
      f_endtime = s.nSec*1000.f + s.nNanoSec/1000000.f;
      if (ms_timeout > 0 && f_endtime - f_starttime >= ms_timeout)
        return index;
      if (us_interval > 0)
        CamOsUsSleep(us_interval);
    }
    rx_data = hal_uart_read_char(UART_GET_OBJ(g_uartNums[idx]));
    buf[index] = rx_data;
    index++;
  }

  return index;
#else // ! USE_UART_READ
    int index = 0;
    CamOsTimespec_t s;
    float f_starttime, f_endtime;
    while (1)
    {
        if (index >= len)
            break;
        CamOsGetMonotonicTime(&s);
        f_starttime = s.nSec*1000.f + s.nNanoSec/1000000.f;
        if (uart_Read(g_uartNums[idx], (u8*)buf + index, 1) == 1)
        {
        }
        else
        {
            CamOsGetMonotonicTime(&s);
            f_endtime = s.nSec*1000.f + s.nNanoSec/1000000.f;
            if (ms_timeout > 0 && f_endtime - f_starttime >= ms_timeout)
                return index;
        }
        if (us_interval > 0)
            CamOsUsSleep(us_interval);
        index++;
    }

    return index;
#endif // ! USE_UART_READ
}

int recv_buffer1(char* buf, int len, int ms_timeout, int us_interval)
{
    return recv_buffer(0, buf, len, ms_timeout, us_interval);
}

static int send_buffer(int idx, char *buffer, u32 len)
{
#ifndef USE_UART_READ
    int ret = len;
    while (len > 0)
    {
        while(!HAL_UART_TX_FIFO_NOT_FULL(UART_GET_OBJ(g_uartNums[idx])))
            CamOsUsSleep(2); // poll tx fifo full
        hal_uart_write_char(UART_GET_OBJ(g_uartNums[idx]),*buffer);
        buffer++;
        len--;
    }
    return ret - len;
#else // ! USE_UART_READ
    return uart_Write(g_uartNums[idx], (u8*)buffer, len);
#endif // ! USE_UART_READ
}

static int send_buffer1(char *buffer, u32 len)
{
    return send_buffer(0, buffer, len);
}

float UART_Now(void)
{
    return 0;
}

void UART_SetBlocking (int fd, int should_block)
{
#if 0
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 10;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
    }
#endif
}

int UART_Init()
{
#ifndef __RTK_OS__
    struct termios cfg;
    uart_fd = open(UART_MODEMDEVICE, O_RDWR | O_NOCTTY | O_SYNC);
    if (uart_fd < 0) {
        my_printf("[Zigbee] serial dev file open error\n");
        return -1;
    }

    tcgetattr(uart_fd, &cfg); /* save current port settings */

    cfsetospeed (&cfg, UART_BAUDRATE);
    cfsetispeed (&cfg, UART_BAUDRATE);

    cfg.c_cflag = (cfg.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
     // disable IGNBRK for mismatched speed tests; otherwise receive break
     // as \000 chars
//    cfg.c_iflag &= ~IGNBRK;         // disable break processing
    cfg.c_iflag = IGNPAR;
    cfg.c_lflag = 0;                // no signaling chars, no echo,
                                    // no canonical processing
    cfg.c_oflag = 0;                // no remapping, no delays
    cfg.c_cc[VMIN]  = 0;            // read doesn't block
    cfg.c_cc[VTIME] = 0;             // 0.1 seconds read timeout

    cfg.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    cfg.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                     // enable readingdr
    cfg.c_cflag &= ~(PARENB | PARODD);      // shut off parity

#if 0
    cfg.c_cflag &= ~CSTOPB;
    cfg.c_cflag &= ~CRTSCTS;
#else
    cfg.c_cflag |= CSTOPB;
    cfg.c_cflag &= ~CRTSCTS;
#endif

    cfg.c_cflag &= ~CRTSCTS;

    tcsetattr(uart_fd, TCSANOW, &cfg);

//    UART_SetBlocking (uart_fd, 0);
#else // ! __RTK_OS__
    init_uart1();
#endif // ! __RTK_OS__
    return 0;
}

int UART_Init2(void)
{
    init_uart(1);
    return 0;
}

void UART_Quit(void)
{
#ifndef __RTK_OS__
    close(uart_fd);
#else // ! __RTK_OS__
#endif // ! __RTK_OS__
}

void UART_Quit2(void)
{
}

void UART_SetBaudrate(int iBaudrate)
{
#ifndef __RTK_OS__
    if(uart_fd <= 0)
        return;

    struct termios cfg;
    tcgetattr(uart_fd, &cfg); /* save current port settings */

    cfsetospeed (&cfg, iBaudrate);
    cfsetispeed (&cfg, iBaudrate);

    tcsetattr(uart_fd, TCSADRAIN, &cfg);
#else // !__RTK_OS__
    if (iBaudrate == B115200)
        set_rate_uart1(115200);
    else if (iBaudrate == B230400)
        set_rate_uart1(230400);
    else if (iBaudrate == B460800)
        set_rate_uart1(460800);
    else
        set_rate_uart1(115200);
#endif // !__RTK_OS__
}

void UART_SetBaudrate2(int iBaudrate)
{
    if (iBaudrate == B115200)
        set_rate_uart(1, 115200);
    else if (iBaudrate == B230400)
        set_rate_uart(1, 230400);
    else if (iBaudrate == B460800)
        set_rate_uart(1, 460800);
    else
        set_rate_uart(1, 115200);
}

int UART_Send(unsigned char * pBuf, int nBufLen)
{
#ifndef __RTK_OS__
    int i, nTxLen;
    if(uart_fd < 0)
        return 0;

    tcflush(uart_fd, TCIOFLUSH);

#ifdef PRINT_DEBUG
    my_printf("[UART] Send Len = %d: ", nBufLen);
    for (i = 0; i < nBufLen; i++)
        my_printf("0x%02x,", pBuf[i]);
    my_printf("\n");
#endif

    nTxLen = write(uart_fd, pBuf, nBufLen);

//    tcdrain(uart_fd);
    my_usleep(2 * 1000);

    if(nTxLen < 0)
    {
        LOG_PRINT("[UART] UART TX error\n");
    }

    return nTxLen;
#else // ! __RTK_OS__
    int ret = 0;
#ifdef UART_EN
    GPIO_fast_setvalue(UART_EN, ON);
#endif
    ret = send_buffer1((char*)pBuf, nBufLen);
#ifdef UART_EN
    my_usleep(5000);
    GPIO_fast_setvalue(UART_EN, OFF);
#endif
    return ret;
#endif // ! __RTK_OS__
}

int UART_Send2(unsigned char * pBuf, int nBufLen)
{
    return send_buffer(1, (char*)pBuf, nBufLen);
}

int UART_NDelay()
{
#if 0
    if(uart_fd < 0)
        return -1;

    fcntl(uart_fd, F_SETFL,  fcntl(uart_fd, F_GETFL) | O_NONBLOCK);
//    fcntl(uart_fd, F_SETFL, FNDELAY);
#endif
    return 0;
}

int UART_Recv(unsigned char * pBuf, int nBufLen)
{
#if 0
    int nRxLen, i, r;

    if(uart_fd < 0)
        return -1;

//    fcntl(uart_fd, F_SETFL, FNDELAY);       //add

    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    FD_ZERO(&rfds);
    FD_SET(uart_fd,&rfds);

    r = select(uart_fd + 1, &rfds, NULL, NULL, &tv);
    if(r == -1)
    {
        my_printf("uart select timeout\n");
        nRxLen = 0;
    }
    else if(r)
        nRxLen = read(uart_fd, pBuf, nBufLen);
    else
        nRxLen = 0;


#ifdef PRINT_DEBUG
    if(nRxLen  > 0)
    {
        my_printf("[UART] Recv Len = %d: ", nRxLen);
        for (i = 0;i < nRxLen; i++)
            my_printf("0x%02x,", pBuf[i]);
        my_printf("\n");
    }
#endif
    if (nRxLen < 0)
    {
        //An error occured (will occur if there are no bytes)
    }
    else if (nRxLen == 0)
    {
        //No data waiting
    }

    return nRxLen;
#else //0
    int ret = 0;
    ret = recv_buffer1((char*)pBuf, nBufLen, 1, 0);
#ifdef PRINT_DEBUG
    int i;
    if (ret > 0)
    {
        my_printf("[%s] len=%d:\n", __func__, nBufLen);
        for (i = 0; i < ret; i++)
            my_printf("%02x ", pBuf[i]);
        my_printf("\n");
    }
#endif // PRINT_DEBUG
    return ret;
#endif // 0
}

int UART_Recv2(unsigned char * pBuf, int nBufLen)
{
    int ret = 0;
    ret = recv_buffer(1, (char*)pBuf, nBufLen, 1, 0);
#ifdef PRINT_DEBUG
    int i;
    if (ret > 0)
    {
        my_printf("[%s] len=%d:\n", __func__, nBufLen);
        for (i = 0; i < ret; i++)
            my_printf("%02x ", pBuf[i]);
        my_printf("\n");
    }
#endif // PRINT_DEBUG
    return ret;
}

int UART_RecvDataForWait(unsigned char* pBuf, int iBufLen, int iTimeOut, int iInterval)
{
#if 0
    int iReceiveOff = 0;
    int iReceiveSize = 0;
    unsigned char wakup[10] = { 0 };
    int iRet = 0;

    float rOldTime = UART_Now();

    while(1)
    {
        iReceiveSize = __min(128, iBufLen -  iReceiveOff);
        iRet = UART_Recv(pBuf + iReceiveOff, iReceiveSize);
        if(iRet > 0)
        {
            iReceiveOff += iRet;

            rOldTime = UART_Now();
        }

        if(iReceiveOff == iBufLen)
            break;

        if(iReceiveOff > iBufLen) {
            LOG_PRINT("[Zigbee] over data received\n\n");
            break;
        }

        if (iTimeOut > 0) {
            if(UART_Now() - rOldTime >= iTimeOut)
            {
                return -1;
            }
        }

        if (iInterval > 0)
            my_usleep(iInterval);
    }

    return iReceiveOff;
#else
    return recv_buffer1((char*)pBuf, iBufLen, iTimeOut, iInterval);
#endif
}

int UART_RecvDataForWait2(unsigned char* pBuf, int iBufLen, int iTimeOut, int iInterval)
{
    return recv_buffer(1, (char*)pBuf, iBufLen, iTimeOut, iInterval);
}

int uart3Open()
{
    UART_Init2();
    return 0;
}

void uart3Close()
{
    UART_Quit2();
}

int uart3Read(char *buffer, int length, int timeout)
{
    return UART_RecvDataForWait2((unsigned char*)buffer, length, timeout, 0);
}

int uart3Write(char *buffer, int length)
{
    return UART_Send2((unsigned char*)buffer, length);
}