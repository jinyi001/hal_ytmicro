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
#include "printf.h"
#include <stdbool.h>
#include <stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* The instance of CAN0 silk-screen on EVB */
#define CAN0_INST (0U)
/* The instance of CAN1 silk-screen on EVB */
#define CAN1_INST (1U)

#define TX_MAILBOX_STD (0UL)
#define TX_MAILBOX_EXT (0UL)
#define TX_MSG_ID2_STD (0x200UL)
#define TX_MSG_ID2_EXT (0x180F1000U)

#define RX_MAILBOX_STD (1UL)
#define RX_MSG_ID2_STD (0x4FEUL)
#define RX_MAILBOX_EXT (2UL)
#define RX_MSG_ID2_EXT (0x0CF003FEU)

#define TX_MSG_ID1_STD (0x4FEUL)
#define TX_MSG_ID1_EXT (0x0CF003FEU)

#define LED0_TOGGLE() (PINS_DRV_TogglePins(GPIOB, (pins_channel_type_t)1 << 4U))
#define LED1_TOGGLE() (PINS_DRV_TogglePins(GPIOB, (pins_channel_type_t)1 << 5U))

#define TEST_COUNT (10U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* CAN0 configuration */
/* Define receive buffer */
flexcan_msgbuff_t rxMsg1;
/* Prepare message to be sent */
flexcan_msgbuff_t txMsg1Std = {
    .cs = 0U,
    .msgId = TX_MSG_ID1_STD,
    .data[0] = 0x00,
    .dataLen = 64U,
};

flexcan_msgbuff_t txMsg1Ext = {
    .cs = 0U,
    .msgId = TX_MSG_ID1_EXT,
    .data[0] = 0x00,
    .dataLen = 64U,
};

const flexcan_data_info_t txMbStdInfo1 = {
    .msg_id_type = FLEXCAN_MSG_ID_STD,
    .data_length = 64,
    .fd_enable = true,
    .fd_padding = 0,
    .enable_brs = true,
    .is_remote = false,
};
const flexcan_data_info_t txMbExtInfo1 = {
    .msg_id_type = FLEXCAN_MSG_ID_EXT,
    .data_length = 64,
    .fd_enable = true,
    .fd_padding = 0,
    .enable_brs = true,
    .is_remote = false,
};

/* CAN1 configuration */
/* Define receive buffer */
flexcan_msgbuff_t rxMsg2;
/* Prepare message to be sent */
flexcan_msgbuff_t txMsg2Std = {
    .cs = 0U,
    .msgId = TX_MSG_ID2_STD,
    .data[0] = 0x00,
    .dataLen = 64U,
};

flexcan_msgbuff_t txMsg2Ext = {
    .cs = 0U,
    .msgId = TX_MSG_ID2_EXT,
    .data[0] = 0x00,
    .dataLen = 64U,
};

const flexcan_data_info_t txMbStdInfo2 = {
    .msg_id_type = FLEXCAN_MSG_ID_STD,
    .data_length = 64,
    .fd_enable = true,
    .fd_padding = 0,
    .enable_brs = false,
    .is_remote = false,
};
const flexcan_data_info_t txMbExtInfo2 = {
    .msg_id_type = FLEXCAN_MSG_ID_EXT,
    .data_length = 64,
    .fd_enable = true,
    .fd_padding = 0,
    .enable_brs = false,
    .is_remote = false,
};
const flexcan_data_info_t rxMbStdInfo2 = {
    .msg_id_type = FLEXCAN_MSG_ID_STD,
    .data_length = 64,
    .fd_enable = true,
    .fd_padding = 0,
    .enable_brs = false,
    .is_remote = false,
};
const flexcan_data_info_t rxMbExtInfo2 = {
    .msg_id_type = FLEXCAN_MSG_ID_EXT,
    .data_length = 64,
    .fd_enable = true,
    .fd_padding = 0,
    .enable_brs = false,
    .is_remote = false,
};

const flexcan_enhance_rx_fifo_filter_table_t enhanceFifoFilterTable[2] = 
{
    {
        .isRemoteFrame = false,
        .isExtendedFrame = false,
        .filterScheme = FLEXCAN_ENAHNCE_RXFIFO_FSCH_FILTER_MASK,
        .filter_table.scheme_filter_mask.filter = 0x200U,
        .filter_table.scheme_filter_mask.mask  = 0x7FFU,
    },
    {
        .isRemoteFrame = false,
        .isExtendedFrame = true,
        .filterScheme = FLEXCAN_ENAHNCE_RXFIFO_FSCH_FILTER_MASK,
        .filter_table.scheme_filter_mask.filter = 0x180F1000U,
        .filter_table.scheme_filter_mask.mask  = 0x1FFFFFFFU,
    },
};

volatile uint16_t canTimeoutCnt = 0U;
volatile bool canError = false;
volatile uint8_t can0RxFrameCnt = 0U;
volatile uint8_t can1RxFrameCnt = 0U;
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void FlexCAN_RxEventCall(uint8_t instance,
                         flexcan_event_type_t eventType,
                         uint32_t buffIdx,
                         flexcan_state_t *flexcanState)
{
    if (instance == CAN0_INST)
    {
        switch (eventType)
        {
            case FLEXCAN_EVENT_DMA_COMPLETE:
            {
                can0RxFrameCnt++;
                FLEXCAN_DRV_EnhanceRxFifo(CAN0_INST, &rxMsg1);
                LED0_TOGGLE();
                if(rxMsg1.msgId == TX_MSG_ID2_STD)
                {
                    for (uint32_t i = 0U; i < 64U; i++)
                    {   
                        txMsg1Ext.data[i] = rxMsg1.data[63U - i];
                    }
                    /* Send the information via CAN */
                    if (FLEXCAN_DRV_GetTransferStatus(CAN0_INST, TX_MAILBOX_EXT) != STATUS_BUSY)
                    {
                        FLEXCAN_DRV_Send(CAN0_INST, TX_MAILBOX_EXT, &txMbExtInfo1, TX_MSG_ID1_EXT, txMsg1Ext.data);
                    }
                }
                else if(rxMsg1.msgId == TX_MSG_ID2_EXT)
                {
                    for (uint32_t i = 0U; i < 64U; i++)
                    {   
                        txMsg1Std.data[i] = rxMsg1.data[63U - i];
                    }
                    /* Send the information via CAN */
                    if (FLEXCAN_DRV_GetTransferStatus(CAN0_INST, TX_MAILBOX_STD) != STATUS_BUSY)
                    {
                        FLEXCAN_DRV_Send(CAN0_INST, TX_MAILBOX_STD, &txMbStdInfo1, TX_MSG_ID1_STD, txMsg1Std.data);
                    }

                }
                else
                {
                    /* Do nothing */
                }
            }
            break;

            default:
                /* Default */
                break;
        }
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
    status_t status = STATUS_SUCCESS;

    /* USER CODE END 1 */ 
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Flexcan Enhanced Fifo use DMA Demo.\n");
    PRINTF("FlexCAN transmit between CAN0 and CAN1 on EVB.\n");
    PRINTF("Short the CAN_H and CAN_L of CAN0 to CAN1 on EVB.\n");

    FLEXCAN_DRV_ConfigEnhanceRxFifo(CAN0_INST, &enhanceFifoFilterTable[0], 2U);
    status |= FLEXCAN_DRV_ConfigRxMb(CAN1_INST, RX_MAILBOX_STD, &rxMbStdInfo2, RX_MSG_ID2_STD);
    status |= FLEXCAN_DRV_ConfigRxMb(CAN1_INST, RX_MAILBOX_EXT, &rxMbExtInfo2, RX_MSG_ID2_EXT);

    /* Install tx/rx callback */
    FLEXCAN_DRV_InstallEventCallback(CAN0_INST, FlexCAN_RxEventCall, NULL);

    if (FLEXCAN_DRV_GetTransferStatus(CAN0_INST, (uint32_t)FLEXCAN_MB_HANDLE_ENHANCE_RXFIFO) != STATUS_BUSY)
    {
        /* start to receive new data */
        status |= FLEXCAN_DRV_EnhanceRxFifo(CAN0_INST, &rxMsg1);
    }
    
    status |= FLEXCAN_DRV_Receive(CAN1_INST, RX_MAILBOX_STD, &rxMsg2);
    status |= FLEXCAN_DRV_Receive(CAN1_INST, RX_MAILBOX_EXT, &rxMsg2);

    for (uint32_t i = 0U; i < 64U; i++)
    {   
        txMsg2Std.data[i] = i;
    }

    if (FLEXCAN_DRV_GetTransferStatus(CAN1_INST, TX_MAILBOX_STD) != STATUS_BUSY)
    {
        status |= FLEXCAN_DRV_Send(CAN1_INST, TX_MAILBOX_STD, &txMbStdInfo2, TX_MSG_ID2_STD, txMsg2Std.data);
    }

    while (1)
    {
        canTimeoutCnt++;
        if (FLEXCAN_DRV_GetTransferStatus(CAN1_INST, RX_MAILBOX_STD) != STATUS_BUSY)
        {
            can1RxFrameCnt++;
            FLEXCAN_DRV_Receive(CAN1_INST, RX_MAILBOX_STD, &rxMsg2);
            LED1_TOGGLE();
            PRINTF("CAN0 Rx length= %d, id = %x, data[0] = %x\n", 
                rxMsg1.dataLen, rxMsg1.msgId, rxMsg1.data[0]);
            PRINTF("CAN0 Tx length= %d, id = %x, data[0] = %x\n", 
                txMsg1Ext.dataLen, txMsg1Ext.msgId, txMsg1Ext.data[0]);
            PRINTF("CAN1 Rx length= %d, id = %x, data[0] = %x\n", 
                rxMsg2.dataLen, rxMsg2.msgId, rxMsg2.data[0]);
            if(rxMsg2.msgId == RX_MSG_ID2_STD)
            {
                for (uint32_t i = 0U; i < 64U; i++)
                {   
                    txMsg2Std.data[i] = rxMsg2.data[i];
                }
                /* Send the information via CAN */
                if (FLEXCAN_DRV_GetTransferStatus(CAN1_INST, TX_MAILBOX_STD) != STATUS_BUSY)
                {
                    FLEXCAN_DRV_Send(CAN1_INST, TX_MAILBOX_STD, &txMbStdInfo2, TX_MSG_ID2_STD, txMsg2Std.data);
                }
                PRINTF("CAN1 Tx length= %d, id = %x, data[0] = %x\n", 
                    txMsg2Std.dataLen, txMsg2Std.msgId, txMsg2Std.data[0]);
            }
        }

        OSIF_TimeDelay(50);
        if(FLEXCAN_DRV_GetTransferStatus(CAN1_INST, RX_MAILBOX_EXT) != STATUS_BUSY)
        {
            can1RxFrameCnt++;
            FLEXCAN_DRV_Receive(CAN1_INST, RX_MAILBOX_EXT, &rxMsg2);
            LED1_TOGGLE();
            PRINTF("CAN0 Rx length= %d, id = %x, data[0] = %x\n", 
                rxMsg1.dataLen, rxMsg1.msgId, rxMsg1.data[0]);
            PRINTF("CAN0 Tx length= %d, id = %x, data[0] = %x\n", 
                txMsg1Std.dataLen, txMsg1Std.msgId, txMsg1Std.data[0]);
            PRINTF("CAN1 Rx length= %d, id = %x, data[0] = %x\n", 
                rxMsg2.dataLen, rxMsg2.msgId, rxMsg2.data[0]);
            if(rxMsg2.msgId == RX_MSG_ID2_EXT)
            {
                for (uint32_t i = 0U; i < 64U; i++)
                {   
                    txMsg2Ext.data[i] = rxMsg2.data[i];
                }
                /* Send the information via CAN */
                if (FLEXCAN_DRV_GetTransferStatus(CAN1_INST, TX_MAILBOX_EXT) != STATUS_BUSY)
                {
                    FLEXCAN_DRV_Send(CAN1_INST, TX_MAILBOX_EXT, &txMbExtInfo2, TX_MSG_ID2_EXT, txMsg2Ext.data);
                }
                PRINTF("CAN1 Tx length= %d, id = %x, data[0] = %x\n", 
                    txMsg2Ext.dataLen, txMsg2Ext.msgId, txMsg2Ext.data[0]);
            }
        }

        OSIF_TimeDelay(50);
        /* If received 10 messages, stop demo routine */
        if ((can0RxFrameCnt >= TEST_COUNT) && (can1RxFrameCnt >= TEST_COUNT))
        {
            break;
        }
        /* Only for self test, user can delete it */
        if (canTimeoutCnt >= 1000U)
        {
            canError= true;
            PRINTF("FlexCAN FD Demo Timeout! Please check connection\n");
        }
    }
    PRINTF("CAN0 Total %d frames received.\n", can0RxFrameCnt);
    PRINTF("CAN1 Total %d frames received.\n", can1RxFrameCnt);
    /* Only for self test, user can delete it */
    if (!canError)
    {
        PRINTF("FlexCAN Enhance FIFO Demo Pass!\n");
    }
    else
    {
        PRINTF("FlexCAN Enhance FIFO Demo Fail!\n");
    }
    FLEXCAN_DRV_Deinit(CAN0_INST);
    FLEXCAN_DRV_Deinit(CAN1_INST);
    PRINTF("FlexCAN Enhance FIFO Demo End!\n");
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
    return status;
    /* USER CODE END 3 */
}

static void Board_Init(void)
{
    CLOCK_SYS_Init(g_clockManConfigsArr,CLOCK_MANAGER_CONFIG_CNT,g_clockManCallbacksArr,CLOCK_MANAGER_CALLBACK_CNT);
    if(STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX,CLOCK_MANAGER_POLICY_AGREEMENT))
    {
        /* USER CODE BEGIN ERROR_HANDLER 1 */
        SystemSoftwareReset();
        /* USER CODE END ERROR_HANDLER 1 */
    }
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0,g_pin_mux_InitConfigArr0);
    FLEXCAN_DRV_Init(CAN0_INST,&flexcanInitConfig0_State,&flexcanInitConfig0);
    FLEXCAN_DRV_Init(CAN1_INST,&flexcanInitConfig1_State,&flexcanInitConfig1);
    DMA_DRV_Init(&dmaState,&dmaController_InitConfig,dmaChnState,dmaChnConfigArray,NUM_OF_CONFIGURED_DMA_CHANNEL);
    UTILITY_PRINT_Init();
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
