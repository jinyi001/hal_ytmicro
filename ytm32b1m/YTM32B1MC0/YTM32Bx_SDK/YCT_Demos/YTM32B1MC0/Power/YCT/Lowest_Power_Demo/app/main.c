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
#define LPTMR_INST 0

#define RUN_MODE       0
#define SLEEP_MODE     1
#define DEEPSLEEP_MODE 2
#define STANDBY_MODE   3

#define LOW_POWER_MODE STANDBY_MODE

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define SW1_PORT       (GPIOE)
#define SW1_PORT_INDEX (0U)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

volatile uint32_t g_lptmrIrqCnt = 0U;
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void lpTMR0_IRQHandler(void)
{
    lpTMR_DRV_ClearCompareFlag(0);
    g_lptmrIrqCnt++;
}

void GPIOE_IRQHandler(void)
{
    if (PINS_DRV_GetPortIntFlag(SW1_PORT) & (1 << SW1_PORT_INDEX))
    {
        /* Note: Debug output inserted into interrupt routine for demo clarity. Might introduce delay. */
        PRINTF("SW1 pressed.\r\n");
        /* Clear interrupt flag.*/
        PINS_DRV_ClearPinIntFlagCmd(SW1_PORT, SW1_PORT_INDEX);
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
    PRINTF("Lowest Power Demo!\r\n");

    POWER_SYS_Init(&powerConfigsArr, POWER_MANAGER_CONFIG_CNT, NULL, POWER_MANAGER_CALLBACK_CNT);

    /* When debugging low power mode, it is recommended to add a delay to wait to 
       prevent debugger cannot be connected as soon as the power is turned on */
    OSIF_TimeDelay(2000);

    lpTMR_DRV_StartCounter(LPTMR_INST);

    PRINTF("Wait to enter low power mode.\n");
    while (g_lptmrIrqCnt == 0)
        ;
    PRINTF("Start to enter low power mode.\n");
    // Enter into low power mode
    PRINTF("Because SIRC disable Standby/Deepsleep, it cannot wakeup by Lptmr.\r\n");
    PRINTF("Please press SW1 to wakeup device.\r\n");
    POWER_SYS_SetMode(LOW_POWER_MODE, POWER_MANAGER_POLICY_AGREEMENT);
    PRINTF("Device has been waken up from standby or deepsleep mode!\n");
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
    lpTMR_DRV_Init(LPTMR_INST, &LPTMR_Config, false);
    INT_SYS_ConfigInit();
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
