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
/* The instance of CAN1 silk-screen on EVB */
#define CAN1_INST (1U)
/* The instance of CAN2 silk-screen on EVB */
#define CAN2_INST (0U)

#define TX_MSG_ID1     (0x0C1001F0U)
#define TX_MSG_ID2     (0x081002F0U)
#define TX_MAILBOX     (0x08UL)
#define RX_LEGACY_FIFO (0UL) /* can not modify */

#define LED_B_TOGGLE()            (PINS_DRV_TogglePins(GPIOB, (pins_channel_type_t)1 << 4U))
#define LED_R_TOGGLE()            (PINS_DRV_TogglePins(GPIOE, (pins_channel_type_t)1 << 8U))
#define CAN0_RX_FIFO_FILTER_COUNT (8 * (FLEXCAN_RX_FIFO_ID_FILTERS_8 + 1))

#define TEST_COUNT (10U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* Define rx FIFO filter acceptance ID */
const uint32_t rxFifoAccId[CAN0_RX_FIFO_FILTER_COUNT] = {
    0x0C1001F0U, 0x081002F0U, 0x0C0803A5U, 0x0C1004F0U, 0x0C1005E0U, 0x0C1006F3U, 0x0C1007F0U, 0x0C10080FU,
};
flexcan_id_table_t rxFifoFilterAccInfo[CAN0_RX_FIFO_FILTER_COUNT];

/* CAN1 configuration */
const flexcan_data_info_t rxMbExtInfo1 = {
    .msg_id_type = FLEXCAN_MSG_ID_EXT,
    .data_length = 8,
    .fd_enable = false,
    .fd_padding = 0,
    .enable_brs = false,
    .is_remote = false,
};
const flexcan_data_info_t txMbExtInfo1 = {
    .msg_id_type = FLEXCAN_MSG_ID_EXT,
    .data_length = 8,
    .fd_enable = false,
    .fd_padding = 0,
    .enable_brs = false,
    .is_remote = false,
};

/* Define receive buffer */
flexcan_msgbuff_t rxMsg1;
/* Prepare message to be sent */
flexcan_msgbuff_t txMsg1 = {
    .cs = 0U,
    .msgId = TX_MSG_ID1,
    .data[0] = 0x00,
    .dataLen = 8U,
};

/* CAN2 configuration */
const flexcan_data_info_t rxMbExtInfo2 = {
    .msg_id_type = FLEXCAN_MSG_ID_EXT,
    .data_length = 8,
    .fd_enable = false,
    .fd_padding = 0,
    .enable_brs = false,
    .is_remote = false,
};
const flexcan_data_info_t txMbExtInfo2 = {
    .msg_id_type = FLEXCAN_MSG_ID_EXT,
    .data_length = 8,
    .fd_enable = false,
    .fd_padding = 0,
    .enable_brs = false,
    .is_remote = false,
};

/* Define receive buffer */
flexcan_msgbuff_t rxMsg2;
/* Prepare message to be sent */
flexcan_msgbuff_t txMsg2 = {
    .cs = 0U,
    .msgId = TX_MSG_ID2,
    .data[0] = 0x00,
    .dataLen = 8U,
};
volatile bool canError = false;
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
    uint32_t offset1 = 0;
    uint32_t offset2 = 0x80;
    uint8_t fifoFrameCnt1 = 0;
    uint8_t fifoFrameCnt2 = 0;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("FlexCAN use legacy fifo transmit between CAN1 and CAN2 on EVB.\n");
    PRINTF("Short the CAN_H and CAN_L of CAN1 to CAN2 on EVB.\n");
    /* Init rx fifo filter table acceptance code */
    for (uint8_t i = 0; i < (uint8_t)CAN0_RX_FIFO_FILTER_COUNT; i++)
    {
        rxFifoFilterAccInfo[i].isRemoteFrame = false;
        rxFifoFilterAccInfo[i].isExtendedFrame = true;
        rxFifoFilterAccInfo[i].id = rxFifoAccId[i];
    }
    /* Can1 init */
    FLEXCAN_DRV_ConfigRxFifo(CAN1_INST, FLEXCAN_RX_FIFO_ID_FORMAT_A, rxFifoFilterAccInfo);
    /* Configure TX buffer with index TX_MAILBOX*/
    status |= FLEXCAN_DRV_ConfigTxMb(CAN1_INST, TX_MAILBOX, &txMbExtInfo1, TX_MSG_ID1);
    /* Start receiving data in RX_LEGACY_FIFO. */
    status |= FLEXCAN_DRV_RxFifo(CAN1_INST, &rxMsg1);

    /* Can3 init */
    FLEXCAN_DRV_ConfigRxFifo(CAN2_INST, FLEXCAN_RX_FIFO_ID_FORMAT_A, rxFifoFilterAccInfo);
    /* Configure TX buffer with index TX_MAILBOX*/
    status |= FLEXCAN_DRV_ConfigTxMb(CAN2_INST, TX_MAILBOX, &txMbExtInfo2, TX_MSG_ID2);
    /* Start receiving data in RX_LEGACY_FIFO. */
    status |= FLEXCAN_DRV_RxFifo(CAN2_INST, &rxMsg2);
    while (1)
    {
        /* CAN1 transmit */
        if (FLEXCAN_DRV_GetTransferStatus(CAN1_INST, TX_MAILBOX) != STATUS_BUSY)
        {
            for (int i = 0; i < 8; i++)
            {
                txMsg1.data[i] = offset1 + i;
            }
            offset1++;
            /* Send the information via CAN */
            status |= FLEXCAN_DRV_Send(CAN1_INST, TX_MAILBOX, &txMbExtInfo1, TX_MSG_ID1, txMsg1.data);
        }
        if (FLEXCAN_DRV_GetTransferStatus(CAN1_INST, RX_LEGACY_FIFO) != STATUS_BUSY)
        {
            fifoFrameCnt1++;
            LED_B_TOGGLE();
            /* Print the received data information */
            PRINTF("CAN1 Rx length= %d, id = %x\n", rxMsg1.dataLen, rxMsg1.msgId);
            for (int i = 0; i < rxMsg1.dataLen; i++)
            {
                PRINTF("%x ", rxMsg1.data[i]);
                if (rxMsg1.data[i] != txMsg2.data[i])
                {
                    canError = true;
                    PRINTF("\nCAN1 receive mismatch. Expected %x, actual %x.\n", txMsg2.data[i], rxMsg1.data[i]);
                }
            }
            PRINTF("\n");
            /* start to receive new data */
            status |= FLEXCAN_DRV_RxFifo(CAN1_INST, &rxMsg1);
        }
        /* Delay some time for loopback receive */
        OSIF_TimeDelay(10);
        /* CAN2 transmit */
        if (FLEXCAN_DRV_GetTransferStatus(CAN2_INST, TX_MAILBOX) != STATUS_BUSY)
        {
            for (int i = 0; i < 8; i++)
            {
                txMsg2.data[i] = offset2 + i;
            }
            offset2++;
            /* Send the information via CAN */
            status |= FLEXCAN_DRV_Send(CAN2_INST, TX_MAILBOX, &txMbExtInfo2, TX_MSG_ID2, txMsg2.data);
        }
        if (FLEXCAN_DRV_GetTransferStatus(CAN2_INST, RX_LEGACY_FIFO) != STATUS_BUSY)
        {
            fifoFrameCnt2++;
            LED_R_TOGGLE();
            /* Print the received data information */
            PRINTF("CAN2 Rx length= %d, id = %x\n", rxMsg2.dataLen, rxMsg2.msgId);
            for (int i = 0; i < rxMsg2.dataLen; i++)
            {
                PRINTF("%x ", rxMsg2.data[i]);
                if (rxMsg2.data[i] != txMsg1.data[i])
                {
                    canError = true;
                    PRINTF("\nCAN2 receive mismatch. Expected %x, actual %x.\n", txMsg1.data[i], rxMsg2.data[i]);
                }
            }
            PRINTF("\n");
            /* start to receive new data */
            status |= FLEXCAN_DRV_RxFifo(CAN2_INST, &rxMsg2);
        }
        /* If received 10 messages, stop demo routine */
        if ((fifoFrameCnt1 >= TEST_COUNT) && (fifoFrameCnt2 >= TEST_COUNT))
        {
            break;
        }
        /* send data every 100ms */
        OSIF_TimeDelay(100);
        /* Only for self test, user can delete it */
        if (txMsg1.data[0] > TEST_COUNT)
        {
            canError = true;
            PRINTF("FlexCAN Legacy Demo Timeout! Please check connection\n");
        }
    }
    PRINTF("CAN1 Total %d frames received.\n", fifoFrameCnt1);
    PRINTF("CAN2 Total %d frames received.\n", fifoFrameCnt2);
    /* Only for self test, user can delete it */
    if (!canError)
    {
        PRINTF("FlexCAN Legacy Demo Pass!\n");
    }
    else
    {
        PRINTF("FlexCAN Legacy Demo Fail!\n");
    }
    FLEXCAN_DRV_Deinit(CAN1_INST);
    FLEXCAN_DRV_Deinit(CAN2_INST);
    PRINTF("FlexCAN Legacy Demo End!\n");
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
    if (STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX, CLOCK_MANAGER_POLICY_AGREEMENT))
    {
        /* USER CODE BEGIN ERROR_HANDLER 1 */
        SystemSoftwareReset();
        /* USER CODE END ERROR_HANDLER 1 */
    }
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    FLEXCAN_DRV_Init(CAN1_INST, &flexcanInitConfig1_State, &flexcanInitConfig1);
    FLEXCAN_DRV_Init(CAN2_INST, &flexcanInitConfig2_State, &flexcanInitConfig2);
    UTILITY_PRINT_Init();
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
