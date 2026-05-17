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
#define TMR_INST (0U)
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
void TMR0_Ch0_IRQHandler(void)
{
    TMR_DRV_ClearStatusFlags(TMR_INST, 0);
    TMR_DRV_IncrementTicks(TMR_INST, tmr_channel_0.channel, tmr_channel_0.compareValue);
    tmrCnt++;
    /* Note: Debug output inserted into interrupt routine for demo clarity. Might introduce delay. */
    PRINTF("TMR CH0 interrupt happened!\n");
}

void TMR0_Ch1_IRQHandler(void)
{
    TMR_DRV_ClearStatusFlags(TMR_INST, 1);
    TMR_DRV_IncrementTicks(TMR_INST, tmr_channel_1.channel, tmr_channel_1.compareValue);
    tmrCnt++;
    /* Note: Debug output inserted into interrupt routine for demo clarity. Might introduce delay. */
    PRINTF("TMR CH1 interrupt happened!\n");
}

void TMR0_Ch2_IRQHandler(void)
{
    TMR_DRV_ClearStatusFlags(TMR_INST, 2);
    TMR_DRV_IncrementTicks(TMR_INST, tmr_channel_2.channel, tmr_channel_2.compareValue);
    tmrCnt++;
    /* Note: Debug output inserted into interrupt routine for demo clarity. Might introduce delay. */
    PRINTF("TMR CH2 interrupt happened!\n");
}

void TMR0_Ch3_IRQHandler(void)
{
    TMR_DRV_ClearStatusFlags(TMR_INST, 3);
    TMR_DRV_IncrementTicks(TMR_INST, tmr_channel_3.channel, tmr_channel_3.compareValue);
    tmrCnt++;
    /* Note: Debug output inserted into interrupt routine for demo clarity. Might introduce delay. */
    PRINTF("TMR CH3 interrupt happened!\n");
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
    PRINTF("TMR demo!\n");

    TMR_DRV_StartTimer(TMR_INST);

    while (1)
    {
        if (tmrCnt > 32)
        {
            INT_SYS_DisableIRQ(TMR0_Ch0_IRQn);
            INT_SYS_DisableIRQ(TMR0_Ch1_IRQn);
            INT_SYS_DisableIRQ(TMR0_Ch2_IRQn);
            INT_SYS_DisableIRQ(TMR0_Ch3_IRQn);
            TMR_DRV_StopTimer(TMR_INST);
            break;
        }
    }
    PRINTF("TMR Demo Pass.\n");
    TMR_DRV_Deinit(TMR_INST);
    PRINTF("TMR Demo end!\n");
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
    UTILITY_PRINT_Init();
    TMR_DRV_Init(0, &TMR_Config);
    INT_SYS_EnableIRQ(TMR0_Ch0_IRQn);
    INT_SYS_EnableIRQ(TMR0_Ch1_IRQn);
    INT_SYS_EnableIRQ(TMR0_Ch2_IRQn);
    INT_SYS_EnableIRQ(TMR0_Ch3_IRQn);
    TMR_DRV_InitChannel(TMR_INST, &tmr_channel_0);
    TMR_DRV_InitChannel(TMR_INST, &tmr_channel_1);
    TMR_DRV_InitChannel(TMR_INST, &tmr_channel_2);
    TMR_DRV_InitChannel(TMR_INST, &tmr_channel_3);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
