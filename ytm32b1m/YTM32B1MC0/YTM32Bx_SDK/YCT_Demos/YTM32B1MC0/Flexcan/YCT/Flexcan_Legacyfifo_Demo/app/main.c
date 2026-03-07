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

#define TX_MSG_ID      (0x03UL)
#define TX_MAILBOX     (0x08UL)
#define RX_LEGACY_FIFO (0UL) /* can not modify */

#define CAN0_RX_FIFO_FILTER_COUNT (8 * (FLEXCAN_RX_FIFO_ID_FILTERS_8 + 1))
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
const flexcan_data_info_t rxMbStdInfo = {
    .msg_id_type = FLEXCAN_MSG_ID_EXT,
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

/* Define rx FIFO filter acceptance ID */
const uint32_t rxFifoAccId[CAN0_RX_FIFO_FILTER_COUNT] = {
    0x0C1001F0U, 0x081002F0U, 0x0C0803A5U, 0x0C1004F0U, 0x0C1005E0U, 0x0C1006F3U, 0x0C1007F0U, 0x0C10080FU,
};
/* Define receive buffer */
flexcan_msgbuff_t rxMsg;
/* Prepare message to be sent */
flexcan_msgbuff_t txMsg = {
    .cs = 0U,
    .msgId = TX_MSG_ID,
    .data[0] = 0x00,
    .dataLen = 8U,
};
flexcan_id_table_t rxFifoFilterAccInfo[CAN0_RX_FIFO_FILTER_COUNT];
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
    uint8_t fifoFrameCnt = 0;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("<----Flexcan legacy fifo demo---->\n");
    /* Init rx fifo filter table acceptance code */
    for (uint8_t i = 0; i < (uint8_t)CAN0_RX_FIFO_FILTER_COUNT; i++)
    {
        rxFifoFilterAccInfo[i].isRemoteFrame = false;
        rxFifoFilterAccInfo[i].isExtendedFrame = true;
        rxFifoFilterAccInfo[i].id = rxFifoAccId[i];
    }
    /* Can init */
    status |= FLEXCAN_DRV_Init(CAN_INST, &flexcanInitConfig0_State, &flexcanInitConfig0);
    FLEXCAN_DRV_ConfigRxFifo(CAN_INST, FLEXCAN_RX_FIFO_ID_FORMAT_A, rxFifoFilterAccInfo);
    /* Configure TX buffer with index TX_MAILBOX*/
    status |= FLEXCAN_DRV_ConfigTxMb(CAN_INST, TX_MAILBOX, &txMbStdInfo, TX_MSG_ID);
    /* Start receiving data in RX_LEGACY_FIFO. */
    status |= FLEXCAN_DRV_RxFifo(CAN_INST, &rxMsg);
    while (1)
    {
        if (FLEXCAN_DRV_GetTransferStatus(CAN_INST, TX_MAILBOX) != STATUS_BUSY)
        {
            for (int i = 0; i < 8; i++)
            {
                txMsg.data[i] = offset + i;
            }
            offset++;
            /* Send the information via CAN */
            status |= FLEXCAN_DRV_Send(CAN_INST, TX_MAILBOX, &txMbStdInfo, TX_MSG_ID, txMsg.data);
        }
        if (FLEXCAN_DRV_GetTransferStatus(CAN_INST, RX_LEGACY_FIFO) != STATUS_BUSY)
        {
            fifoFrameCnt++;
            /* For concise output on UART, only print data[0] */
            PRINTF("\nRx length= %d, id = %x, data[0] = %x:\n", rxMsg.dataLen, rxMsg.msgId, rxMsg.data[0]);
            /* start to receive new data */
            status |= FLEXCAN_DRV_RxFifo(CAN_INST, &rxMsg);
        }
        if (fifoFrameCnt >= 10)
        {
            break;
        }
        /* send data every 100ms */
        OSIF_TimeDelay(100);
    }
    PRINTF("\n\n");
    PRINTF("Total %d frames legacy fifo has received.\n", fifoFrameCnt);
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
