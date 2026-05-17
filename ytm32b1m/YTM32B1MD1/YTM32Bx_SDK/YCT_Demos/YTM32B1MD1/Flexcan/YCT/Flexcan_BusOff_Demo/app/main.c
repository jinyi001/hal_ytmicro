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
#define CAN_INST (2U)

#define TMR_INST       (0U)
/* Receive Standard ID*/
#define RX_STD_MSG_ID  (0x02UL)
/* Receive Standard MB*/
#define RX_STD_MAILBOX (0x00UL)

/* Receive Extended ID*/
#define RX_EXTMSG_ID   (0x70012UL)
/* Receive Standard MB*/
#define RX_EXT_MAILBOX (0x02UL)

#define TX_MSG_ID  (0x03UL)
#define TX_MAILBOX (0x01UL)

#define MAX_QUICKRECOVER_COUNT (10U)

#define LED5_TOGGLE() (PINS_DRV_TogglePins(GPIOB, (pins_channel_type_t)1 << 4U))
#define LED6_TOGGLE() (PINS_DRV_TogglePins(GPIOB, (pins_channel_type_t)1 << 5U))
#define LED7_TOGGLE() (PINS_DRV_TogglePins(GPIOE, (pins_channel_type_t)1 << 8U))

#define QUICKRECOVER_DELAY_100MS (100U)
#define SLOWRECOVER_DELAY_10S    (10000U)
#define SEND_DELAY_1S            (1000U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* test done */
volatile bool testDone = false;
volatile bool errorOccur = false;
volatile uint32_t tmrCnt = 0U;
/* BusOff time count */
static uint32_t busoffCount = 0U;
/* Quick recover flag */
static uint8_t quickRecover = false;
/* Slow recover flag */
static uint8_t slowRecover = false;

/* Standard receive message buffer */
flexcan_msgbuff_t rxStdMsg;
/* Extended receive message buffer */
flexcan_msgbuff_t rxExtMsg;

static uint8_t successfulFlag = false;

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
static void FlexcanInit(void);
static void SlowRecoverFun(void);
static void QuickRecoverFun(void);
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void TMR0_Ch0_IRQHandler(void)
{
    TMR_DRV_ClearStatusFlags(TMR_INST, 0U);
    TMR_DRV_IncrementTicks(TMR_INST, tmr_channel_0.channel, tmr_channel_0.compareValue);
    tmrCnt++;
}

void DRV_CAN_RxEventCall(uint8_t instance,
                         flexcan_event_type_t eventType,
                         uint32_t buffIdx,
                         flexcan_state_t *flexcanState)
{
    (void)instance;
    (void)flexcanState;
    switch (eventType)
    {
        case FLEXCAN_EVENT_RX_COMPLETE:
            if (buffIdx == RX_STD_MAILBOX)
            {
                FLEXCAN_DRV_Receive(CAN_INST, RX_STD_MAILBOX, &rxStdMsg);
                LED6_TOGGLE();
            }
            if (buffIdx == RX_EXT_MAILBOX)
            {
                FLEXCAN_DRV_Receive(CAN_INST, RX_EXT_MAILBOX, &rxExtMsg);
                LED6_TOGGLE();
            }
            break;

        case FLEXCAN_EVENT_TX_COMPLETE:
        {
            /* Send successful clear try recovery flag */
            busoffCount = 0U;

            if (successfulFlag == false)
            {
                successfulFlag = true;
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
    (void)instance;
    (void)flexcanState;
    errorOccur = true;
    switch (eventType)
    {
        case FLEXCAN_BUS_OFF_ENTER_EVENT:
        {
            /* recovery use reset can0 by deinitialization and initialization */
            FLEXCAN_DRV_Deinit(CAN_INST);

            busoffCount++;
            /* Quick recover */
            if (busoffCount <= MAX_QUICKRECOVER_COUNT)
            {
                quickRecover = true;
                slowRecover = false;
            }
            else
            {
                slowRecover = true;
                quickRecover = false;
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
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("FlexCAN Bus Off Demo\n");
    PRINTF("Self test can short CAN3_H and CAN3_L on EVB.\n");
    FlexcanInit();

    PRINTF("<----FlexCAN manual recovery busoff demo---->\n");

    while (1)
    {
        txMsg.data[0]++;
        /* Send the information via CAN */

        /* When enter FLEXCAN_EVENT_TX_COMPLETE event interrupt change this flag to true */
        if (successfulFlag == true)
        {
            PRINTF("<--- send ok --->\n");
            successfulFlag = false;
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

        /* busoff did not happen when busoffCount == 0 */
        if (busoffCount == 0U)
        {
            /* The interval for sending frames successfully is 1s */
            OSIF_TimeDelay(SEND_DELAY_1S);
        }
        else /* busoff error occur */
        {
            TMR_DRV_StartTimer(TMR_INST);

            if (slowRecover == true)
            {
                SlowRecoverFun();
            }
            if (quickRecover == true)
            {
                QuickRecoverFun();
            }
            /* Self test check */
            if (!testDone)
            {
                testDone = true;
                if (errorOccur)
                {
                    PRINTF("FlexCAN Bus Off demo Pass\n");
                }
                else
                {
                    PRINTF("FlexCAN Bus Off demo Fail\n");
                }
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
    CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT, g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
    CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX, CLOCK_MANAGER_POLICY_AGREEMENT);
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    UTILITY_PRINT_Init();
    TMR_DRV_Init(0, &TMR_Config);
    INT_SYS_EnableIRQ(TMR0_Ch0_IRQn);
    TMR_DRV_InitChannel(TMR_INST, &tmr_channel_0);
}

/* USER CODE BEGIN 4 */
static void FlexcanInit(void)
{
    FLEXCAN_DRV_Init(CAN_INST, &flexcanInitConfig0_State, &flexcanInitConfig0);

    /* Automatic recovering from Bus Off state disabled */
    CAN2->CTRL1 |= CAN_CTRL1_BOFFREC_MASK;

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
    if (tmrCnt >= QUICKRECOVER_DELAY_100MS)
    {
        FlexcanInit();

        PRINTF("<----Try Quick recovery times %d---->\n", busoffCount);

        quickRecover = false;
        /* Stop tmrCnt increase */
        TMR_DRV_StopTimer(TMR_INST);

        LED7_TOGGLE();

        tmrCnt = 0U;
    }
}

static void SlowRecoverFun(void)
{
    /* Slow recover，the interval for slow recovery is 10S */
    if (tmrCnt >= SLOWRECOVER_DELAY_10S)
    {
        FlexcanInit();

        PRINTF("<----Try Slow recovery  times %d---->\n", (busoffCount - 10U));

        slowRecover = false;

        /* Stop tmrCnt increase */
        TMR_DRV_StopTimer(TMR_INST);

        LED7_TOGGLE();

        tmrCnt = 0U;
    }
}

/* USER CODE END 4 */
