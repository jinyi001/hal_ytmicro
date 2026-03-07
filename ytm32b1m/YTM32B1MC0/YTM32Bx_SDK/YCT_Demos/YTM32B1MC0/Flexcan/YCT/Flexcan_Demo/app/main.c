/* USER CODE BEGIN Header */
/* you can remove the copyright */

/*
 *  Copyright 2020-2023 Yuntu Microelectronics Co., Ltd.
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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAN_INST (1U)

#define RX_STD_MSG_ID  (0x102UL)
#define RX_STD_MAILBOX (0x00UL)
#define RX_EXT_MSG_ID  (0x70012UL)
#define RX_EXT_MAILBOX (0x02UL)
#define TX_MSG_ID      (0x103UL)
#define TX_MAILBOX     (0x01UL)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* Define receive buffer */
flexcan_msgbuff_t rxStdMsg;
flexcan_msgbuff_t rxExtMsg;
/* Prepare message to be sent */
flexcan_msgbuff_t txMsg = {
    .cs = 0U,
    .msgId = TX_MSG_ID,
    .data[0] = 0x00,
    .dataLen = 1U,
};
const flexcan_data_info_t rxMbExtInfo = {
    .msg_id_type = FLEXCAN_MSG_ID_EXT,
    .data_length = 8,
    .fd_enable = false,
    .fd_padding = 0,
    .enable_brs = false,
    .is_remote = false,
};
const flexcan_data_info_t rxMbStdInfo = {
    .msg_id_type = FLEXCAN_MSG_ID_STD,
    .data_length = 8,
    .fd_enable = false,
    .fd_padding = 0,
    .enable_brs = false,
    .is_remote = false,
};
const flexcan_data_info_t txMbStdInfo = {
    .msg_id_type = FLEXCAN_MSG_ID_STD,
    .data_length = 8,
    .fd_enable = false,
    .fd_padding = 0,
    .enable_brs = false,
    .is_remote = false,
};
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */
    status_t status = STATUS_SUCCESS;
    uint8_t canRxFrameCnt = 0;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("<----FlexCAN transmit receive demo---->\r\n");

    status |= FLEXCAN_DRV_Init(CAN_INST, &flexcanInitConfig0_State, &flexcanInitConfig0);
    status |= FLEXCAN_DRV_ConfigRxMb(CAN_INST, RX_STD_MAILBOX, &rxMbStdInfo, RX_STD_MSG_ID);
    status |= FLEXCAN_DRV_ConfigRxMb(CAN_INST, RX_EXT_MAILBOX, &rxMbExtInfo, RX_EXT_MSG_ID);
    status |= FLEXCAN_DRV_ConfigTxMb(CAN_INST, TX_MAILBOX, &txMbStdInfo, TX_MSG_ID);

    /* Start receiving data in RX_MAILBOX. */
    status |= FLEXCAN_DRV_Receive(CAN_INST, RX_STD_MAILBOX, &rxStdMsg);
    status |= FLEXCAN_DRV_Receive(CAN_INST, RX_EXT_MAILBOX, &rxExtMsg);

    while (1)
    {
        txMsg.data[0]++;
        /* Send the information via CAN */
        if (FLEXCAN_DRV_GetTransferStatus(CAN_INST, TX_MAILBOX) != STATUS_BUSY)
        {
            status |= FLEXCAN_DRV_Send(CAN_INST, TX_MAILBOX, &txMbStdInfo, TX_MSG_ID, txMsg.data);
        }

        if (FLEXCAN_DRV_GetTransferStatus(CAN_INST, RX_STD_MAILBOX) != STATUS_BUSY)
        {
            canRxFrameCnt++;
            PRINTF("Received MSG ID = %x, data[0] = %x\n", rxStdMsg.msgId, rxStdMsg.data[0]);
            /* Start receiving data in RX_STD_MAILBOX. */
            status |= FLEXCAN_DRV_Receive(CAN_INST, RX_STD_MAILBOX, &rxStdMsg);
        }

        if (FLEXCAN_DRV_GetTransferStatus(CAN_INST, RX_EXT_MAILBOX) != STATUS_BUSY)
        {
            canRxFrameCnt++;
            PRINTF("Received EXT MSG ID = %x, data[0] = %x\n", rxExtMsg.msgId, rxExtMsg.data[0]);
            /* Start receiving data in RX_STD_MAILBOX. */
            status |= FLEXCAN_DRV_Receive(CAN_INST, RX_EXT_MAILBOX, &rxExtMsg);
        }

        if (canRxFrameCnt >= 10)
        {
            break;
        }
        /* If received 10 messages, stop demo routine */
        OSIF_TimeDelay(100);
    }

    PRINTF("Total %d frames received.\n", canRxFrameCnt);
    PRINTF("<----Demo end---->\n");
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */
        /* USER CODE BEGIN 3 */
        if (status != STATUS_SUCCESS)
        {
            break;
        }
    }
    /* USER CODE END 3 */
}

static void Board_Init(void)
{
    CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT, g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
    if(STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX,CLOCK_MANAGER_POLICY_AGREEMENT))
    {
        /* USER CODE BEGIN ERROR_HANDLER 1 */
        SystemSoftwareReset();
        /* USER CODE END ERROR_HANDLER 1 */
    }
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    UTILITY_PRINT_Init();
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
