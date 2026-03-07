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
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
typedef enum
{
    DATA_SAVE_WRITE = 0,
    DATA_SAVE_READ,
    DATA_SAVE_CHECK,
} data_save_state_t;

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint32_t Task1msCnt = 0;

uint32_t PowerUpCnt = 0;
uint32_t DataWriteCnt = 0;
uint8_t ReadDataBuffer[50] = { 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t WriteDataBuffer[50] = { 0x00 };
data_save_state_t DataSaveState = DATA_SAVE_WRITE;
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void pTMR0_Ch0_IRQHandler(void)
{
    Task1msCnt++;
    pTMR_DRV_ClearInterruptFlagTimerChannels(pTMR0_INST, pTMR0_CH0);
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
    PRINTF("Fee demo!\r\n");

    pTMR_DRV_InitChannel(pTMR0_INST, pTMR0_CH0, &ptmr_channel_0);
    pTMR_DRV_StartTimerChannels(pTMR0_INST, pTMR0_CH0);

    Fee_Init(&FEEGenConfig);
    do
    {
        Fee_MainFunction();
    } while (MEMIF_IDLE != Fee_GetStatus());

    if (MEMIF_JOB_OK != Fee_GetJobResult())
    {
        PRINTF("The Fee initialization is failed, job status is %d\r\n", Fee_GetJobResult());
    }
    else
    {
        PRINTF("The Fee initialization is successful.\r\n");
    }

    /* Read PowerUpCnt from PowerUpCnt_Block */
    Fee_Read(PowerUpCnt_Block, 0, (uint8_t *)&PowerUpCnt, 4);
    /* Wait Fee_Read done */
    do
    {
        Fee_MainFunction();
    } while (MEMIF_IDLE != Fee_GetStatus());

    if (MEMIF_JOB_FAILED == Fee_GetJobResult())
    {
        /* PowerUpCnt_Block never been written */
        /* Also not set PowerUpCnt to 0, because PowerUpCnt would not be changed if job failed */
        PowerUpCnt = 0;
    }
    PowerUpCnt++;
    PRINTF("It's %d times to power up!\r\n", PowerUpCnt);

    /* Write PowerUpCnt back to PowerUpCnt_Block */
    Fee_Write(PowerUpCnt_Block, (uint8_t *)&PowerUpCnt);
    /*Perform write data to Block 0*/
    do
    {
        Fee_MainFunction();
    } while (MEMIF_IDLE != Fee_GetStatus());

    if (MEMIF_JOB_OK != Fee_GetJobResult())
    {
        PRINTF("The PowerUpCnt_Block write error!\r\n");
    }
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* Task 2ms: always call Fee_MainFunction */
        if (Task1msCnt % 2 == 0)
        {
            /* Fee_MainFunction should be called all the time as a background task  */
            Fee_MainFunction();
        }

        /* Task 500ms: Write DataSave_Block */
        if (Task1msCnt % 500 == 0)
        {
            if (MEMIF_IDLE == Fee_GetStatus() && (DataSaveState == DATA_SAVE_WRITE))
            {
                for (int i = 0; i < 50; i++)
                {
                    WriteDataBuffer[i] = (uint8_t)(DataWriteCnt + i);
                }
                Fee_Write(DataSave_Block, WriteDataBuffer);
                DataSaveState = DATA_SAVE_READ;
                DataWriteCnt++;
            }
        }

        /* Task 200ms: Read DataSave_Block */
        if (Task1msCnt % 200 == 0)
        {
            if (MEMIF_IDLE == Fee_GetStatus() && (DataSaveState == DATA_SAVE_READ))
            {
                if (MEMIF_JOB_OK != Fee_GetJobResult())
                {
                    PRINTF("The DataSave_Block write error!\r\n");
                }
                Fee_Read(DataSave_Block, 0, ReadDataBuffer, 50);
                DataSaveState = DATA_SAVE_CHECK;
            }
        }

        /* Task 100ms: Check DataSave_Block */
        if (Task1msCnt % 100 == 0)
        {
            if (MEMIF_IDLE == Fee_GetStatus() && (DataSaveState == DATA_SAVE_CHECK))
            {
                if (MEMIF_JOB_OK != Fee_GetJobResult())
                {
                    PRINTF("The DataSave_Block read error!\r\n");
                }
                DataSaveState = DATA_SAVE_WRITE;
                /* Check data with WriteDataBuffer */
                for (int i = 0; i < 50; i++)
                {
                    if (WriteDataBuffer[i] != ReadDataBuffer[i])
                    {
                        PRINTF("The DataSave_Block check error!\r\n");
                        break;
                    }
                }
            }
        }
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
    pTMR_DRV_Init(0, &PTMR_Config);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
