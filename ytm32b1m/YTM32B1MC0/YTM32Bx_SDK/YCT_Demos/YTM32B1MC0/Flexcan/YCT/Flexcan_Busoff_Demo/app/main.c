/* USER CODE BEGIN Header */
/* you can remove the copyright */
/*
 *  Copyright 2020-2024 Yuntu Microelectronics Co., Ltd.
 *  All rights reserved.
 * 
 *  SPDX-License-Identifier: BSD-3-Clause
 * 
 * @file main.c
 * @brief 
 * 
 */

/* USER CODE END Header */
#include "sdk_project_config.h"
/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "printf.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAN_INST       (1U)

#define LPTMR_INST      (0U)
/* Receive Standard ID*/
#define RX_STD_MSG_ID  (0x02UL)
/* Receive Standard MB*/
#define RX_STD_MAILBOX (0x00UL)

/* Receive Extended ID*/
#define RX_EXTMSG_ID   (0x70012UL)
/* Receive Standard MB*/
#define RX_EXT_MAILBOX (0x02UL)

#define TX_MSG_ID      (0x03UL)
#define TX_MAILBOX     (0x01UL)

#define MAX_QUICKRECOVER_COUNT     (10U)

#define LED5_TOGGLE()  (PINS_DRV_TogglePins(GPIOD, (pins_channel_type_t)1 << 7U))
#define LED6_TOGGLE()  (PINS_DRV_TogglePins(GPIOD, (pins_channel_type_t)1 << 6U))
#define LED7_TOGGLE()  (PINS_DRV_TogglePins(GPIOD, (pins_channel_type_t)1 << 5U))

#define QUICKRECOVER_DELAY_100MS      (100U)
#define SLOWRECOVER_DELAY_10S         (10000U)
#define SEND_DELAY_1S                 (1000U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

volatile uint32_t tmrCnt = 0U;
/* BusOff time count */
static uint32_t BusOffCount = 0U;
/* Quick recover flag */
static uint8_t QuickRecover = false;
/* Slow recover flag */
static uint8_t SlowRecover = false;

/* Standard receive message buffer */
flexcan_msgbuff_t rxStdMsg;
/* Extended receive message buffer */
flexcan_msgbuff_t rxExtMsg;

static uint8_t successful_flag = false;

/* Prepare message to be sent */
flexcan_msgbuff_t txMsg = {
    .cs = 0U,
    .msgId = TX_MSG_ID,
    .data[0] = 0x00U,
    .dataLen = 1U,
};

const flexcan_data_info_t rxMbExtInfo = {
    .msg_id_type = FLEXCAN_MSG_ID_EXT,
    .data_length = 8U,
    .fd_enable = false,
    .fd_padding = 0U,
    .enable_brs = false,
    .is_remote = false,
};
const flexcan_data_info_t rxMbStdInfo = {
    .msg_id_type = FLEXCAN_MSG_ID_STD,
    .data_length = 8U,
    .fd_enable = false,
    .fd_padding = 0U,
    .enable_brs = false,
    .is_remote = false,
};
const flexcan_data_info_t txMbStdInfo = {
    .msg_id_type = FLEXCAN_MSG_ID_STD,
    .data_length = 8U,
    .fd_enable = false,
    .fd_padding = 0U,
    .enable_brs = false,
    .is_remote = false,
};
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
static void Flexcan_Init(void);
static void SlowRecoverFun(void);
static void QuickRecoverFun(void);
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void lpTMR0_IRQHandler(void)
{
    /* Clear compare flag */
    lpTMR_DRV_ClearCompareFlag(LPTMR_INST);
    tmrCnt++;
}

void DRV_CAN_RxEventCall(uint8_t instance, flexcan_event_type_t eventType, uint32_t buffIdx, flexcan_state_t *flexcanState)
{
    switch (eventType)
	{
        case FLEXCAN_EVENT_RX_COMPLETE:
            if(buffIdx == RX_STD_MAILBOX)
            {
                FLEXCAN_DRV_Receive(CAN_INST, RX_STD_MAILBOX, &rxStdMsg);
                LED6_TOGGLE();
            }
            if(buffIdx == RX_EXT_MAILBOX)
            {
                FLEXCAN_DRV_Receive(CAN_INST, RX_EXT_MAILBOX, &rxExtMsg);
                LED6_TOGGLE();
            }
        break;

        case FLEXCAN_EVENT_TX_COMPLETE:
            {
                /* Send successful clear try recovery flag */
                BusOffCount = 0U;

                if(successful_flag == false)
                {
                    successful_flag = true;
                }
            }
            break;

        default:
            /* Default */
            break;
	}
}

void DRV_CAN_ErrorCall(uint8_t instance, flexcan_error_event_type_t eventType, flexcan_state_t *flexcanState) 
{
    switch (eventType)
	{
        case FLEXCAN_BUS_OFF_ENTER_EVENT:
        {
            /* recovery use reset can0 by deinitialization and initialization */
            FLEXCAN_DRV_Deinit(CAN_INST);

            BusOffCount++;
            /* Quick recover */
            if (BusOffCount <= MAX_QUICKRECOVER_COUNT) 
            {
                QuickRecover = true;
                SlowRecover = false;
            } 
            else
            {
                SlowRecover = true;
                QuickRecover = false;
            }
        }
        default:
            /* default */
        break;
    }
}
/* USER CODE END 0 */


/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */
    /* note:
     * bus off Manual or automatic recovery depends on CAN0 register CTRL1_BOFFREC
     * 0b - Automatic recovering from Bus Off state enabled.
     * 1b - Automatic recovering from Bus Off state disabled. 
     * In this demo,  uses the manual recovery busoff mode.
     * The manual recovery use reset can0 by deinitialization and initialization
     */
    status_t status = STATUS_SUCCESS;
    /* USER CODE END 1 */ 
    Board_Init();
    /* USER CODE BEGIN 2 */
    Flexcan_Init();

    PRINTF("<----FlexCAN manual recovery busoff demo---->\n");

    while (1)
    {
        txMsg.data[0]++;
        /* Send the information via CAN */

        /* When enter FLEXCAN_EVENT_TX_COMPLETE event interrupt change this flag to true */
        if(successful_flag == true)
        {
            PRINTF("<--- send ok --->\n");
            successful_flag = false;
        }

        if (FLEXCAN_DRV_GetTransferStatus(CAN_INST, TX_MAILBOX) != STATUS_BUSY)
        {
            status |= FLEXCAN_DRV_Send(CAN_INST, TX_MAILBOX, &txMbStdInfo, TX_MSG_ID, txMsg.data);
            LED5_TOGGLE();
        }
        if (status != STATUS_SUCCESS)
        {
            break;
        }

        /* busoff did not happen when BusOffCount == 0 */
        if(BusOffCount == 0U)
        {
            /* The interval for sending frames successfully is 1s */
            OSIF_TimeDelay(SEND_DELAY_1S);
        }
        else/* busoff error occur */
        {
            lpTMR_DRV_StartCounter(LPTMR_INST);

            if(SlowRecover == true)
            {
                SlowRecoverFun();
            }
            if(QuickRecover == true)
            {
                QuickRecoverFun();
            }
        }
    }
    PRINTF("<----Demo end---->\n");
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */
        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

static void Board_Init(void)
{
    CLOCK_SYS_Init(g_clockManConfigsArr,CLOCK_MANAGER_CONFIG_CNT,g_clockManCallbacksArr,CLOCK_MANAGER_CALLBACK_CNT);
    CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX,CLOCK_MANAGER_POLICY_AGREEMENT);
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0,g_pin_mux_InitConfigArr0);
    UTILITY_PRINT_Init();
    lpTMR_DRV_Init(LPTMR_INST,&LPTMR_Config,false);
    INT_SYS_EnableIRQ(lpTMR0_IRQn);
}

/* USER CODE BEGIN 4 */
static void Flexcan_Init(void)
{
    FLEXCAN_DRV_Init(CAN_INST, &flexcanInitConfig0_State, &flexcanInitConfig0);

    /* Automatic recovering from Bus Off state disabled */
    CAN1->CTRL1 |= CAN_CTRL1_BOFFREC_MASK;

    FLEXCAN_DRV_ConfigRxMb(CAN_INST, RX_STD_MAILBOX, &rxMbStdInfo, RX_STD_MSG_ID);
    FLEXCAN_DRV_ConfigRxMb(CAN_INST, RX_EXT_MAILBOX, &rxMbExtInfo, RX_EXTMSG_ID);
    
    /* Install tx/rx callback */
    FLEXCAN_DRV_InstallEventCallback(CAN_INST, DRV_CAN_RxEventCall, NULL);
    FLEXCAN_DRV_InstallErrorCallback(CAN_INST, DRV_CAN_ErrorCall, NULL);

    /* Start receiving data in RX_STD_MAILBOX. */
    FLEXCAN_DRV_Receive(CAN_INST, RX_STD_MAILBOX, &rxStdMsg);
    FLEXCAN_DRV_Receive(CAN_INST, RX_EXT_MAILBOX, &rxExtMsg);
}

static void QuickRecoverFun(void)
{
    /* Slow recover，the interval for slow recovery is 100ms */
    if(tmrCnt >= QUICKRECOVER_DELAY_100MS)
    {
        Flexcan_Init();

        PRINTF("<----Try Quick recovery times %d---->\n", BusOffCount);

        QuickRecover = false;
        /* Stop tmrCnt increase */
        lpTMR_DRV_StopCounter(LPTMR_INST);

        tmrCnt = 0U;

        LED7_TOGGLE();
    }
}

static void SlowRecoverFun(void)
{
    /* Slow recover，the interval for slow recovery is 10S */
    if(tmrCnt >= SLOWRECOVER_DELAY_10S)
    {
        Flexcan_Init();

        PRINTF("<----Try Slow recovery  times %d---->\n", (BusOffCount - 10U));

        SlowRecover = false;

        /* Stop tmrCnt increase */
        lpTMR_DRV_StopCounter(LPTMR_INST);

        tmrCnt = 0U;

        LED7_TOGGLE();
    }

}
/* USER CODE END 4 */
