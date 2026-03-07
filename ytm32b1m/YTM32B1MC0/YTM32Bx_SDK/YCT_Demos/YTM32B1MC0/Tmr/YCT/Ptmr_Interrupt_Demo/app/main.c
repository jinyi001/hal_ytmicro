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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PTMR_INST 0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint32_t tmrCnt = 0;

/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
void pTMR0_Ch0_IRQHandler(void)
{
    pTMR_DRV_ClearInterruptFlagTimerChannels(PTMR_INST, pTMR0_CH0);
    /* Note: Debug output inserted into interrupt routine for demo clarity. Might introduce delay. */
    PRINTF("Ptmr CH0 ISR \n");
    tmrCnt++;
}

void pTMR0_Ch1_IRQHandler(void)
{
    pTMR_DRV_ClearInterruptFlagTimerChannels(PTMR_INST, pTMR0_CH1);
    /* Note: Debug output inserted into interrupt routine for demo clarity. Might introduce delay. */
    PRINTF("Ptmr CH1 ISR\n");
    tmrCnt++;
}

void pTMR0_Ch2_IRQHandler(void)
{
    pTMR_DRV_ClearInterruptFlagTimerChannels(PTMR_INST, pTMR0_CH2);
    /* Note: Debug output inserted into interrupt routine for demo clarity. Might introduce delay. */
    PRINTF("Ptmr CH2 ISR\n");
    tmrCnt++;
}

void pTMR0_Ch3_IRQHandler(void)
{
    pTMR_DRV_ClearInterruptFlagTimerChannels(PTMR_INST, pTMR0_CH3);
    /* Note: Debug output inserted into interrupt routine for demo clarity. Might introduce delay. */
    PRINTF("Ptmr CH3 ISR\n");
    tmrCnt++;
}

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
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Ptmr Demo!\r\n");

    pTMR_DRV_StartTimerChannels(PTMR_INST, pTMR0_CH0);
    pTMR_DRV_StartTimerChannels(PTMR_INST, pTMR0_CH1);
    pTMR_DRV_StartTimerChannels(PTMR_INST, pTMR0_CH2);
    pTMR_DRV_StartTimerChannels(PTMR_INST, pTMR0_CH3);

    while (1)
    {
        if (tmrCnt >= 32)
        {
            INT_SYS_DisableIRQ(pTMR0_Ch0_IRQn);
            INT_SYS_DisableIRQ(pTMR0_Ch1_IRQn);
            INT_SYS_DisableIRQ(pTMR0_Ch2_IRQn);
            INT_SYS_DisableIRQ(pTMR0_Ch3_IRQn);
            tmrCnt = 0;
            pTMR_DRV_StopTimerChannels(PTMR_INST, pTMR0_CH0);
            pTMR_DRV_StopTimerChannels(PTMR_INST, pTMR0_CH1);
            pTMR_DRV_StopTimerChannels(PTMR_INST, pTMR0_CH2);
            pTMR_DRV_StopTimerChannels(PTMR_INST, pTMR0_CH3);
            pTMR_DRV_Deinit(PTMR_INST);
            PRINTF("Demo end!\r\n");
            break;
        }
    }
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
    if(STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX,CLOCK_MANAGER_POLICY_AGREEMENT))
    {
        /* USER CODE BEGIN ERROR_HANDLER 1 */
        SystemSoftwareReset();
        /* USER CODE END ERROR_HANDLER 1 */
    }
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    UTILITY_PRINT_Init();
    pTMR_DRV_Init(0, &PTMR_Config);
    pTMR_DRV_InitChannel(PTMR_INST, pTMR0_CH0, &ptmr_channel_0);
    pTMR_DRV_InitChannel(PTMR_INST, pTMR0_CH1, &ptmr_channel_1);
    pTMR_DRV_InitChannel(PTMR_INST, pTMR0_CH2, &ptmr_channel_2);
    pTMR_DRV_InitChannel(PTMR_INST, pTMR0_CH3, &ptmr_channel_3);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
