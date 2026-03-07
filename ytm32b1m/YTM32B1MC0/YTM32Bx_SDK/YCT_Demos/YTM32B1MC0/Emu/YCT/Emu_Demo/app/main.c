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
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
void EMU0_SB_IRQHandler(void)
{
    if (EMU_DRV_GetChannelSBInterruptFlag(0, 0))
    {
        EMU_DRV_ClearInterruptFlagSignalBit(0, 0);
    }
}

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
    uint32_t value = 0;
    uint32_t reportErrAddr = 0;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */

    PRINTF("Build %s %s\n", __DATE__, __TIME__);

    /* write a value to the inject address. */
    *(volatile uint32_t *)(EMU_Config0.injectErrAddr) = 0xBEEF;

    INT_SYS_EnableIRQ(EMU0_SB_IRQn);
    /* Initialize */
    EMU_DRV_Init(0, &EMU_Config0, 1);
    /* Read memory,it will trigger interrupt. */
    value = *(volatile uint32_t *)(EMU_Config0.injectErrAddr);

    /* Deint emu module. */
    EMU_DRV_DeInit(0, 0);
    /* Rewrite the inject address value again to clear the inject error, it can be any value.
    Ortherwise , the inject error will always be there, and read a time will trigger ecc error. */
    *(volatile uint32_t *)(EMU_Config0.injectErrAddr) = 0x01;

    if (value == 0xBEEF)
    {
        PRINTF("Injection error has been corrected!\n");
    }
    else
    {
        PRINTF("Injection error has not been corrected!\n");
    }

    reportErrAddr = EMU_DRV_GetErrReportAddr(0, EMU_Config0.channel);
    PRINTF("injectErrAddr:%x\n", EMU_Config0.injectErrAddr);
    PRINTF("reportErrAddr:%x\n", reportErrAddr);

    PRINTF("<----DEMO_END---->\n");
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
