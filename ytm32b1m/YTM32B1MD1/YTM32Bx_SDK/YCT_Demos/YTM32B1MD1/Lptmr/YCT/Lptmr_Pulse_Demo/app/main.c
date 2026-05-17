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
#define LPTMR_INST (0U)
#define TEST_CNT   (3U)
#define ERROR      (10U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint8_t lptmrIsrCnt = 0;
uint32_t cnt = 0;

/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
void lpTMR0_IRQHandler(void)
{
    PRINTF("LpTMR ISR enter!\n");
    /* Clear compare flag */
    lpTMR_DRV_ClearCompareFlag(0);
    lptmrIsrCnt++;
    /* Note: Debug output inserted into interrupt routine for demo clarity. Might introduce delay. */
    PRINTF("LpTMR ISR exit!\n");
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
    PRINTF("Lptmr Demo!\r\n");
    PRINTF("Please Short PTE11 and PTD14 for self test.\n");
    /* Enable IRQ for LPTMR */
    INT_SYS_EnableIRQ(lpTMR0_IRQn);
    /* Start LPTMR counter */
    lpTMR_DRV_StartCounter(LPTMR_INST);
    while (1)
    {
        PINS_DRV_WritePin(GPIOD, 14, 1);
        OSIF_TimeDelay(1);
        PINS_DRV_WritePin(GPIOD, 14, 0);
        OSIF_TimeDelay(1);
        if (lptmrIsrCnt > TEST_CNT)
        {
            /* Stop LPTMR counter */
            lpTMR_DRV_StopCounter(LPTMR_INST);
            break;
        }
        cnt++;
    }
    PRINTF("Input %d pulse and enter %d lpTMR irq.\n", cnt, lptmrIsrCnt);
    /* Self Test, expected is 2000, enter irq may loss pulse */
    if (cnt > ((TEST_CNT + 1) * LPTMR_Config.compareValue + ERROR))
    {
        PRINTF("lpTMR Pulse Demo Fail!\n");
    }
    else
    {
        PRINTF("lpTMR Pulse Demo Pass!\n");
    }
    lpTMR_DRV_Deinit(LPTMR_INST);
    PRINTF("lpTMR Pulse Demo End!\n");
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
    UTILITY_PRINT_Init();
    lpTMR_DRV_Init(0, &LPTMR_Config, false);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
