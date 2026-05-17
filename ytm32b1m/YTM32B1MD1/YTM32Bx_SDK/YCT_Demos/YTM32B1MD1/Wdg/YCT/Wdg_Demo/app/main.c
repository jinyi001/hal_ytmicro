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
#define FEED_DOG_COUNT (10U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint32_t lptmrIsrCnt = 0;

/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
void LPTMR0_TriggerIRQHandler(void)
{
    /* Clear lptmr interrupt flag. */
    lpTMR_DRV_ClearCompareFlag(0);
    lptmrIsrCnt++;
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
    uint32_t lastTimer = 0;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    /* Check whether the reset is caused by WDG. */
    if (RCU->RSSR & RCU_RSSR_WDG_MASK)
    {
        PRINTF("System reset by WDG reset!\n");
        /* Clear all reset flags. */
        RCU->RSSR = RCU->RSSR;
        PRINTF("WDG Demo Pass.\n");
        PRINTF("WDG Demo End.\n");
    }
    else
    {
        PRINTF("Build %s %s\n", __DATE__, __TIME__);
        PRINTF("WDG Demo!\r\n");
        PRINTF("Setup WDG and wait for WDG timeout(60ms)!\n");
        INT_SYS_InstallHandler(lpTMR0_IRQn, LPTMR0_TriggerIRQHandler, NULL);
        /* Enable lptmr interrupt. */
        INT_SYS_EnableIRQ(lpTMR0_IRQn);
        /* Init wdg module. */
        WDG_DRV_Init(0, &wdg_config0);

        /* Start lptmr timer. */
        lpTMR_DRV_StartCounter(0);
        /* Feed dog in 20ms */
        while (lptmrIsrCnt < FEED_DOG_COUNT)
        {
            if (lastTimer != lptmrIsrCnt)
            {
                WDG_DRV_Trigger(0);
                lastTimer = lptmrIsrCnt;
            }
        }
        /* Close lptmr timer. */
        lpTMR_DRV_StopCounter(0);
        lpTMR_DRV_Deinit(0);
        PRINTF("Stop feed and wait for WDG reset.\n");
        /* If 100ms, MCU not reset, FAIL */
        OSIF_TimeDelay(100);
        PRINTF("WDG Demo Fail.\n");
        PRINTF("WDG Demo End.\n");
    }
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
    if (STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX, CLOCK_MANAGER_POLICY_AGREEMENT))
    {
        /* USER CODE BEGIN ERROR_HANDLER 1 */
        SystemSoftwareReset();
        /* USER CODE END ERROR_HANDLER 1 */
    }
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    UTILITY_PRINT_Init();
    lpTMR_DRV_Init(0, &LPTMR_Config, false);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
