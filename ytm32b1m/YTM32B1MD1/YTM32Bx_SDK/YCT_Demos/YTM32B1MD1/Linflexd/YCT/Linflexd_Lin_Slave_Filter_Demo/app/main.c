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
typedef enum
{
    LINMasterIDLE = 0,     /* LIN master IDLE */
    LINMasterRecvDone = 1, /* LIN master recv done */
    LINMasterSendDone = 2, /* LIN master send done */
    LINMasterError = 3,    /* LIN master error happens */
    LINSlaverIDLE = 4,     /* LIN slave IDLE */
    LINSlaveRecvDone = 5,  /* LIN slave recv done */
    LINSlaverSendDone = 6, /* LIN slave send done */
    LINSlaverError = 7,    /* LIN slave error happens */
} LINFlexD_user_state_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LIN_MASTER_INST   (0)
#define LIN_SLAVE_INST    (1)
#define MILLISECOND_DELAY (100)

/* Two LIN ID for master send and recv */
#define MASTER_SEND_SLAVE_RECV (0x22)
#define MASTER_RECV_SLAVE_SEND (0x11)
#define MISMATCH_LIN_ID        (0x33)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile LINFlexD_user_state_t slaveEvent = LINSlaverIDLE;
/* Self test flag */
volatile bool linError = false;

linflexd_frame_t linMasterSendFrame = { .id = MASTER_SEND_SLAVE_RECV,
                                        .checksumType = LIN_ENHANCED_CHECKSUM,
                                        .responseType = LIN_MASTER_RESPONSE,
                                        .dataLength = 8,
                                        .data = { 0x69, 0x96, 0x55, 0xAA, 0xAA, 0x55, 0x96, 0x69 } };

linflexd_frame_t linMasterSendMismatchFrame = { .id = MISMATCH_LIN_ID,
                                                .checksumType = LIN_ENHANCED_CHECKSUM,
                                                .responseType = LIN_MASTER_RESPONSE,
                                                .dataLength = 8,
                                                .data = { 0x69, 0x96, 0x55, 0xAA, 0xAA, 0x55, 0x96, 0x69 } };

/* Frame configuration structure, slave sends response */
linflexd_frame_t linMasterRecvFrame = {
    .id = MASTER_RECV_SLAVE_SEND,
    .checksumType = LIN_ENHANCED_CHECKSUM,
    .responseType = LIN_SLAVE_RESPONSE,
    .dataLength = 8,
};
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void linflexd_process_callback(uint32_t instance, void *state)
{
    linflexd_state_t *linState = (linflexd_state_t *)state;
    if (LINFlexD_DATA_RECEIVED_EVENT == linState->currentEventId)
    {
        if (MASTER_SEND_SLAVE_RECV == LINFlexD_GetFilterMatchId(instance))
        {
            slaveEvent = LINSlaveRecvDone;
        }
    }
    if (LINFlexD_DATA_SENT_EVENT == linState->currentEventId)
    {
        if (MASTER_RECV_SLAVE_SEND == LINFlexD_GetFilterMatchId(instance))
        {
            slaveEvent = LINSlaverSendDone;
        }
    }
    if (LINFlexD_OUTPUT_COMPARE_EVENT == linState->currentEventId)
    {}
    /* If any error happened, deal with it following */
    if (linState->errorStatus)
    {
        slaveEvent = LINSlaverError;
        linState->errorStatus = 0;
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
    uint32_t cnt = 5;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\r\n", __DATE__, __TIME__);
    PRINTF("LIN Slave filter Demo!\r\n");
    PRINTF("LIN1 as master and LIN2 as slave. LIN transfer on EVB.\n");
    PRINTF("Please short LIN1 and LIN2 for test.\n");

    LINFlexD_DRV_InstallCallback(LIN_SLAVE_INST, linflexd_process_callback);
    while (cnt--)
    {
        /* Clear transmit status */
        slaveEvent = LINSlaverIDLE;
        /* Master send and slave receive */
        status |= LINFlexD_DRV_MasterTransfer(LIN_MASTER_INST, &linMasterSendFrame);
        /* Wait LIN transmission completed */
        while (LINSlaverIDLE == slaveEvent)
            ;

        /* Handler LIN slave */
        if (LINSlaveRecvDone == slaveEvent)
        {
            PRINTF("LIN Slave Receive and id is 0x%x: \n", linflexd_lin_config2.slaveFilterCfgPtr[1].id);
            for (uint8_t i = 0; i < linflexd_lin_config2.slaveFilterCfgPtr[1].dataFieldLength; i++)
            {
                PRINTF("0x%x, ", linflexd_lin_config2.slaveFilterCfgPtr[1].data[i]);
                if (linflexd_lin_config2.slaveFilterCfgPtr[1].data[i] != linMasterSendFrame.data[i])
                {
                    linError = true;
                    PRINTF("\nLIN Slave receive mismatch. Expected %x, actual %x\n",
                           linMasterSendFrame.data[i],
                           linflexd_lin_config2.slaveFilterCfgPtr[1].data[i]);
                }
            }
        }
        else
        {
            linError = true;
            PRINTF("LIN Slave Receive error!\n");
        }
        PRINTF("\r\n");
        /* Delay several millisecond */
        OSIF_TimeDelay(MILLISECOND_DELAY);

        slaveEvent = LINSlaverIDLE;
        /* Master send and slave receive */
        status |= LINFlexD_DRV_MasterTransfer(LIN_MASTER_INST, &linMasterRecvFrame);
        /* Wait LIN transmission completed */
        while (LINSlaverIDLE == slaveEvent)
            ;
        /* Handler LIN slave */
        if (LINSlaverSendDone == slaveEvent)
        {
            PRINTF("LIN Slave Send and id is 0x%x: \n", linflexd_lin_config2.slaveFilterCfgPtr[0].id);
            for (uint8_t i = 0; i < linflexd_lin_config2.slaveFilterCfgPtr[0].dataFieldLength; i++)
            {
                PRINTF("0x%x, ", linflexd_lin_config2.slaveFilterCfgPtr[0].data[i]);
                if (linflexd_lin_config2.slaveFilterCfgPtr[0].data[i] != linMasterRecvFrame.data[i])
                {
                    linError = true;
                    PRINTF("\nLIN Master receive mismatch. Expected %x, actual %x\n",
                           linflexd_lin_config2.slaveFilterCfgPtr[0].data[i],
                           linMasterRecvFrame.data[i]);
                }
                linflexd_lin_config2.slaveFilterCfgPtr[0].data[i] += 8;
            }
        }
        else
        {
            linError = true;
            PRINTF("LIN Slave Send error!\n");
        }
        PRINTF("\r\n");
        /* Delay several millisecond */
        OSIF_TimeDelay(MILLISECOND_DELAY);

        slaveEvent = LINSlaverIDLE;
        /* Master send mismatch ID and slave will discard */
        status |= LINFlexD_DRV_MasterTransfer(LIN_MASTER_INST, &linMasterSendMismatchFrame);
        /* Delay several millisecond */
        OSIF_TimeDelay(MILLISECOND_DELAY);
        /* Slave will ignore this frame */
        if (slaveEvent != LINSlaverIDLE)
        {
            linError = true;
        }
    }
    /* Self test */
    if (!linError)
    {
        PRINTF("LINFlexD Slave Filter Demo Pass!\n");
    }
    else
    {
        PRINTF("LINFlexD Slave Filter Demo Fail!\n");
    }
    LINFlexD_DRV_Deinit(LIN_MASTER_INST);
    LINFlexD_DRV_Deinit(LIN_SLAVE_INST);
    PRINTF("LINFlexD Slave Filter Demo End.\n");
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
    CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT, g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
    if (STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX, CLOCK_MANAGER_POLICY_AGREEMENT))
    {
        /* USER CODE BEGIN ERROR_HANDLER 1 */
        SystemSoftwareReset();
        /* USER CODE END ERROR_HANDLER 1 */
    }
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    LINFlexD_DRV_Init(LIN_MASTER_INST, &linflexd_lin_config1, &linflexd_lin_config1_State);
    LINFlexD_DRV_Init(LIN_SLAVE_INST, &linflexd_lin_config2, &linflexd_lin_config2_State);
    UTILITY_PRINT_Init();
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
