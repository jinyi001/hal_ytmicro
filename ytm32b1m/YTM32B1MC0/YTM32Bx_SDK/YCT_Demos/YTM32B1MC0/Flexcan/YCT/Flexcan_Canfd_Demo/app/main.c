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

#define TX_MAILBOX (1UL)
#define TX_MSG_ID  (3UL)
#define RX_MAILBOX (0UL)
#define RX_MSG_ID  (2UL)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* Define receive buffer */
flexcan_msgbuff_t rxMsg;
/* Prepare message to be sent */
flexcan_msgbuff_t txMsg = {
    .cs = 0U,
    .msgId = TX_MSG_ID,
    .data[0] = 0x00,
    .dataLen = 64U,
};

const flexcan_data_info_t rxMbStdInfo = {
    .msg_id_type = FLEXCAN_MSG_ID_STD,
    .data_length = 64,
    .fd_enable = true,
    .fd_padding = 0,
    .enable_brs = true,
    .is_remote = false,
};
const flexcan_data_info_t txMbStdInfo = {
    .msg_id_type = FLEXCAN_MSG_ID_STD,
    .data_length = 64,
    .fd_enable = true,
    .fd_padding = 0,
    .enable_brs = true,
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
    int32_t offset = 0;
    uint8_t canRxFrameCnt = 0;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("<----Flexcan CAN-FD mode demo---->\n");
    /* Can init */
    status |= FLEXCAN_DRV_Init(CAN_INST, &flexcanInitConfig0_State, &flexcanInitConfig0);
    /* Configure RX buffer with index RX_MAILBOX */
    status |= FLEXCAN_DRV_ConfigRxMb(CAN_INST, RX_MAILBOX, &rxMbStdInfo, RX_MSG_ID);
    /* Configure TX buffer with index TX_MAILBOX*/
    status |= FLEXCAN_DRV_ConfigTxMb(CAN_INST, TX_MAILBOX, &txMbStdInfo, TX_MSG_ID);
    /* Start receiving data in RX_STD_MAILBOX. */
    status |= FLEXCAN_DRV_Receive(CAN_INST, RX_MAILBOX, &rxMsg);

    while (1)
    {
        if (FLEXCAN_DRV_GetTransferStatus(CAN_INST, TX_MAILBOX) != STATUS_BUSY)
        {
            /* Configure sending data bytes */
            for (int i = 0; i < 64; i++)
            {
                txMsg.data[i] = offset + i;
            }
            offset++;
            /* Send the information via CAN */
            status |= FLEXCAN_DRV_Send(CAN_INST, TX_MAILBOX, &txMbStdInfo, TX_MSG_ID, txMsg.data);
        }
        if (FLEXCAN_DRV_GetTransferStatus(CAN_INST, RX_MAILBOX) != STATUS_BUSY)
        {
            canRxFrameCnt++;
            /* Print the received data information */
            PRINTF("\nRx length= %d, id = %x\n", rxMsg.dataLen, rxMsg.msgId);
            for (int i = 0; i < rxMsg.dataLen; i++)
            {
                PRINTF("%x ", rxMsg.data[i]);
            }
            /* Start to receive new data */
            status |= FLEXCAN_DRV_Receive(CAN_INST, RX_MAILBOX, &rxMsg);
        }
        /* If received 10 messages, stop demo routine */
        if (canRxFrameCnt >= 10)
        {
            break;
        }
        /* Send data every 100ms */
        OSIF_TimeDelay(100);
    }

    PRINTF("\n\n");
    PRINTF("Total %d frames received.\n", canRxFrameCnt);
    PRINTF("<----Demo end---->\n");
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        if (status != STATUS_SUCCESS)
        {
            break;
        }
        /* USER CODE END WHILE */
        /* USER CODE BEGIN 3 */
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
