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
typedef enum
{
    STANDBY_MODE,
    ACTIVE_MODE,
} Sys_Mode_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

//#define LOW_POWER_MODE STANDBY_MODE
#define CAN_INST (1U)

#define WKU_INST (0U)

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

#define LED5_TOGGLE() (PINS_DRV_TogglePins(GPIOD, (pins_channel_type_t)1 << 5U))
#define LED6_TOGGLE() (PINS_DRV_TogglePins(GPIOD, (pins_channel_type_t)1 << 6U))
#define LED7_TOGGLE() (PINS_DRV_TogglePins(GPIOD, (pins_channel_type_t)1 << 7U))

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
Sys_Mode_t sys_mode_sele = ACTIVE_MODE;

static uint8_t successful_flag = false;

static uint8_t en_lp_flag = false;
/* Standard receive message buffer */
flexcan_msgbuff_t rxStdMsg;
/* Extended receive message buffer */
flexcan_msgbuff_t rxExtMsg;

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
static void APP_Set_Mode_Fun(void);
static void Flexcan_Send_Fun(status_t send_status);
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void DRV_CAN_RxEventCall(uint8_t instance,
                         flexcan_event_type_t eventType,
                         uint32_t buffIdx,
                         flexcan_state_t *flexcanState)
{
    switch (eventType)
    {
        case FLEXCAN_EVENT_RX_COMPLETE:
        {
            if (buffIdx == RX_STD_MAILBOX)
            {
                en_lp_flag = true;

                FLEXCAN_DRV_Receive(CAN_INST, RX_STD_MAILBOX, &rxStdMsg);

                /* message data[0] == 0x01 will change system mode to standby mode, 
                     * others data don't care
                    */
                if (rxStdMsg.data[0] == 0x01U)
                {
                    sys_mode_sele = STANDBY_MODE;
                }
                else
                {}
                LED6_TOGGLE();
            }
            if (buffIdx == RX_EXT_MAILBOX)
            {
                FLEXCAN_DRV_Receive(CAN_INST, RX_EXT_MAILBOX, &rxExtMsg);
                LED7_TOGGLE();
            }
            else
            {}
        }
        break;

        case FLEXCAN_EVENT_TX_COMPLETE:
        {
            if (successful_flag == false)
            {
                successful_flag = true;
            }
            else
            {}
        }
        break;

        case FLEXCAN_EVENT_SELF_WAKEUP:
        {
            PRINTF("<----FlexCAN WakeUp from Standby mode---->\n\r");
        }

        break;
        default:
            /* Default */
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
    /* Note: Use **RX_STD_MAILBOX** defined mailbox number(mailbox-0) to receive a standard message id = **RX_STD_MSG_ID**(0x002)- 
     * Use **RX_EXT_MAILBOX** defined mailbox number(mailbox-2) to receive a Extended message id = **RX_EXTMSG_ID**(0x70012)-
     * received message with can0 event interupt(FLEXCAN_EVENT_RX_COMPLETE). 
     * User can choose the standby mode by standard can message received. 
     * Any frame can wake up the chip to active mode.
     */
    status_t status = STATUS_SUCCESS;
    /* USER CODE END 1 */ 
    Board_Init();
    /* USER CODE BEGIN 2 */

    PRINTF(" Build %s %s\r\n", __DATE__, __TIME__);

    PRINTF(" Flexcan wake up demo start\r\n");

    OSIF_TimeDelay(1000U);

    Flexcan_Init();

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        Flexcan_Send_Fun(status);

        if (en_lp_flag == true)
        {
            APP_Set_Mode_Fun();
            en_lp_flag = false;
            sys_mode_sele = ACTIVE_MODE;
        }
        else
        {}
        /* USER CODE END WHILE */
        /* USER CODE BEGIN 3 */
        if (status != STATUS_SUCCESS)
        {
            break;
        }
        else
        {}
    }

    return status;
    /* USER CODE END 3 */
}

static void Board_Init(void)
{
    CLOCK_SYS_Init(g_clockManConfigsArr,CLOCK_MANAGER_CONFIG_CNT,g_clockManCallbacksArr,CLOCK_MANAGER_CALLBACK_CNT);
    CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX,CLOCK_MANAGER_POLICY_AGREEMENT);
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0,g_pin_mux_InitConfigArr0);
    UTILITY_PRINT_Init();
    POWER_SYS_Init(&powerConfigsArr,POWER_MANAGER_CONFIG_CNT,NULL,POWER_MANAGER_CALLBACK_CNT);
}

/* USER CODE BEGIN 4 */
static void Flexcan_Init(void)
{
    FLEXCAN_DRV_Init(CAN_INST, &flexcanInitConfig0_State, &flexcanInitConfig0);

    FLEXCAN_DRV_ConfigRxMb(CAN_INST, RX_STD_MAILBOX, &rxMbStdInfo, RX_STD_MSG_ID);
    FLEXCAN_DRV_ConfigRxMb(CAN_INST, RX_EXT_MAILBOX, &rxMbExtInfo, RX_EXTMSG_ID);

    /* Enable can0 self wakeup by any frame */
    FLEXCAN_DRV_EnableSelfWakeUp(CAN_INST);
    /* Install tx/rx callback */
    FLEXCAN_DRV_InstallEventCallback(CAN_INST, DRV_CAN_RxEventCall, NULL);

    /* Start receiving data in RX_STD_MAILBOX. */
    FLEXCAN_DRV_Receive(CAN_INST, RX_STD_MAILBOX, &rxStdMsg);
    FLEXCAN_DRV_Receive(CAN_INST, RX_EXT_MAILBOX, &rxExtMsg);
}

static void Flexcan_Send_Fun(status_t send_status)
{
    /* When enter FLEXCAN_EVENT_TX_COMPLETE event interrupt change this flag to true */
    if (successful_flag == true)
    {
        PRINTF("<--- send ok --->\r\n");
        LED5_TOGGLE();
        OSIF_TimeDelay(1000U);
        successful_flag = false;
    }
    else
    {}

    if (FLEXCAN_DRV_GetTransferStatus(CAN_INST, TX_MAILBOX) != STATUS_BUSY)
    {
        txMsg.data[0]++;
        send_status |= FLEXCAN_DRV_Send(CAN_INST, TX_MAILBOX, &txMbStdInfo, TX_MSG_ID, txMsg.data);
    }
    else
    {}
}

static void APP_Set_Mode_Fun(void)
{
    switch (sys_mode_sele)
    {
        case STANDBY_MODE:
        {
            PRINTF("<----goto Standby mode ---->\n\r");

            /* Enter into Standby mode */
            POWER_SYS_SetMode(sys_mode_sele, POWER_MANAGER_POLICY_AGREEMENT);
        }
        break;

        default:
            /* default */
            break;
    }
}

/* USER CODE END 4 */
