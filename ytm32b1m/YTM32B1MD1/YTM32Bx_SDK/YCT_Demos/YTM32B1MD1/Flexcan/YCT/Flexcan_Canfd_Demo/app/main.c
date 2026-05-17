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

/* If self-test, TX ID need match RX ID */
#define TX_MAILBOX (1UL)
#define RX_MAILBOX (0UL)
#define TX_MSG_ID1 (2UL)
#define RX_MSG_ID1 (3UL)
#define TX_MSG_ID2 (3UL)
#define RX_MSG_ID2 (2UL)

#define LED_B_TOGGLE() (PINS_DRV_TogglePins(GPIOB, (pins_channel_type_t)1 << 4U))
#define LED_R_TOGGLE() (PINS_DRV_TogglePins(GPIOE, (pins_channel_type_t)1 << 8U))

#define TEST_COUNT (10U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* CAN1 configuration */
/* Define receive buffer */
flexcan_msgbuff_t rxMsg1;
/* Prepare message to be sent */
flexcan_msgbuff_t txMsg1 = {
    .cs = 0U,
    .msgId = TX_MSG_ID1,
    .data[0] = 0x00,
    .dataLen = 64U,
};

const flexcan_data_info_t rxMbStdInfo1 = {
    .msg_id_type = FLEXCAN_MSG_ID_STD,
    .data_length = 64,
    .fd_enable = true,
    .fd_padding = 0,
    .enable_brs = true,
    .is_remote = false,
};
const flexcan_data_info_t txMbStdInfo1 = {
    .msg_id_type = FLEXCAN_MSG_ID_STD,
    .data_length = 64,
    .fd_enable = true,
    .fd_padding = 0,
    .enable_brs = true,
    .is_remote = false,
};

/* Define receive buffer */
flexcan_msgbuff_t rxMsg2;
/* Prepare message to be sent */
flexcan_msgbuff_t txMsg2 = {
    .cs = 0U,
    .msgId = TX_MSG_ID2,
    .data[0] = 0x80,
    .dataLen = 64U,
};

const flexcan_data_info_t rxMbStdInfo2 = {
    .msg_id_type = FLEXCAN_MSG_ID_STD,
    .data_length = 64,
    .fd_enable = true,
    .fd_padding = 0,
    .enable_brs = true,
    .is_remote = false,
};
const flexcan_data_info_t txMbStdInfo2 = {
    .msg_id_type = FLEXCAN_MSG_ID_STD,
    .data_length = 64,
    .fd_enable = true,
    .fd_padding = 0,
    .enable_brs = true,
    .is_remote = false,
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
    uint8_t can1RxFrameCnt = 0;
    uint8_t can2RxFrameCnt = 0;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("FlexCAN FD transmit between CAN1 and CAN2 on EVB.\n");
    PRINTF("Short the CAN_H and CAN_L of CAN1 to CAN2 on EVB.\n");

    /* Configure RX buffer with index RX_MAILBOX */
    status |= FLEXCAN_DRV_ConfigRxMb(CAN1_INST, RX_MAILBOX, &rxMbStdInfo1, RX_MSG_ID1);
    /* Configure TX buffer with index TX_MAILBOX*/
    status |= FLEXCAN_DRV_ConfigTxMb(CAN1_INST, TX_MAILBOX, &txMbStdInfo1, TX_MSG_ID1);
    /* Start receiving data in RX_MAILBOX. */
    status |= FLEXCAN_DRV_Receive(CAN1_INST, RX_MAILBOX, &rxMsg1);

    /* Configure RX buffer with index RX_MAILBOX */
    status |= FLEXCAN_DRV_ConfigRxMb(CAN2_INST, RX_MAILBOX, &rxMbStdInfo2, RX_MSG_ID2);
    /* Configure TX buffer with index TX_MAILBOX*/
    status |= FLEXCAN_DRV_ConfigTxMb(CAN2_INST, TX_MAILBOX, &txMbStdInfo2, TX_MSG_ID2);
    /* Start receiving data in RX_MAILBOX. */
    status |= FLEXCAN_DRV_Receive(CAN2_INST, RX_MAILBOX, &rxMsg2);

    while (1)
    {
        /* CAN1 transmit */
        if (FLEXCAN_DRV_GetTransferStatus(CAN1_INST, TX_MAILBOX) != STATUS_BUSY)
        {
            /* Configure sending data bytes */
            for (int i = 0; i < 64; i++)
            {
                txMsg1.data[i] = offset1 + i;
            }
            offset1++;
            /* Send the information via CAN */
            status |= FLEXCAN_DRV_Send(CAN1_INST, TX_MAILBOX, &txMbStdInfo1, TX_MSG_ID1, txMsg1.data);
        }
        if (FLEXCAN_DRV_GetTransferStatus(CAN1_INST, RX_MAILBOX) != STATUS_BUSY)
        {
            can1RxFrameCnt++;
            LED_B_TOGGLE();
            /* Print the received data information */
            PRINTF("CAN1 Rx length= %d, id = %x\n", rxMsg1.dataLen, rxMsg1.msgId);
            for (int i = 0; i < rxMsg1.dataLen; i++)
            {
                PRINTF("%02x ", rxMsg1.data[i]);
                if (rxMsg1.data[i] != txMsg2.data[i])
                {
                    canError = true;
                    PRINTF("\nCAN1 receive mismatch. Expected %02x, actual %02x.\n", txMsg2.data[i], rxMsg1.data[i]);
                }
                /* Format data */
                if (i % 8 == 7)
                {
                    PRINTF("\n");
                }
            }
            /* Start to receive new data */
            status |= FLEXCAN_DRV_Receive(CAN1_INST, RX_MAILBOX, &rxMsg1);
        }
        /* Delay some time for loopback receive */
        OSIF_TimeDelay(10);
        /* CAN2 transmit */
        if (FLEXCAN_DRV_GetTransferStatus(CAN2_INST, TX_MAILBOX) != STATUS_BUSY)
        {
            /* Configure sending data bytes */
            for (int i = 0; i < 64; i++)
            {
                txMsg2.data[i] = offset2 + i;
            }
            offset2++;
            /* Send the information via CAN */
            status |= FLEXCAN_DRV_Send(CAN2_INST, TX_MAILBOX, &txMbStdInfo2, TX_MSG_ID2, txMsg2.data);
        }
        if (FLEXCAN_DRV_GetTransferStatus(CAN2_INST, RX_MAILBOX) != STATUS_BUSY)
        {
            can2RxFrameCnt++;
            LED_R_TOGGLE();
            /* Print the received data information */
            PRINTF("CAN2 Rx length= %d, id = %x\n", rxMsg2.dataLen, rxMsg2.msgId);
            for (int i = 0; i < rxMsg2.dataLen; i++)
            {
                PRINTF("%02x ", rxMsg2.data[i]);
                if (rxMsg2.data[i] != txMsg1.data[i])
                {
                    canError = true;
                    PRINTF("\nCAN2 receive mismatch. Expected %02x, actual %02x.\n", txMsg1.data[i], rxMsg2.data[i]);
                }
                /* Format data */
                if (i % 8 == 7)
                {
                    PRINTF("\n");
                }
            }
            /* Start to receive new data */
            status |= FLEXCAN_DRV_Receive(CAN2_INST, RX_MAILBOX, &rxMsg2);
        }

        /* If received 10 messages, stop demo routine */
        if ((can1RxFrameCnt >= TEST_COUNT) && (can2RxFrameCnt >= TEST_COUNT))
        {
            break;
        }
        /* Send data every 100ms */
        OSIF_TimeDelay(100);
        /* Only for self test, user can delete it */
        if (txMsg1.data[0] > TEST_COUNT)
        {
            canError = true;
            PRINTF("FlexCAN FD Demo Timeout! Please check connection\n");
        }
    }

    PRINTF("CAN1 Total %d frames received.\n", can1RxFrameCnt);
    PRINTF("CAN2 Total %d frames received.\n", can2RxFrameCnt);
    /* Only for self test, user can delete it */
    if (!canError)
    {
        PRINTF("FlexCAN FD Demo Pass!\n");
    }
    else
    {
        PRINTF("FlexCAN FD Demo Fail!\n");
    }
    FLEXCAN_DRV_Deinit(CAN1_INST);
    FLEXCAN_DRV_Deinit(CAN2_INST);
    PRINTF("FlexCAN FD Demo End!\n");
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
