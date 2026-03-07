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
#include "osif.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BOARD_RED_LED     (GPIOD)
#define BOARD_RED_LED_PIN (5)
/* Red led */
#define RED_LED_OFF()     PINS_DRV_SetPins(BOARD_RED_LED, (1 << BOARD_RED_LED_PIN))
#define RED_LED_ON()      PINS_DRV_ClearPins(BOARD_RED_LED, (1 << BOARD_RED_LED_PIN))
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t flag = 0;

void ACMP0_IRQHandler(void)
{
    ACMP_DRV_GetOutputFlags(0, &flag);
    if ((flag & 0x3) && ACMP_DRV_GetOutput(0))
    {
        /* Compare the voltage of channel 0 and channel 1 each second.
         * Once detect that channel 0 voltage is greater than channel 1,
         * red led will light on to send warning. */
        RED_LED_ON();
    }
    else
    {
        /* If channel 0 voltage is less than channel 1, issue is clear, exit interrupt */
        ACMP_DRV_ClearOutputFlags(0);
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
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    /* Enable ACMP0_IRQn */
    INT_SYS_EnableIRQ(ACMP0_IRQn);
    /* Enable ACMP module */
    ACMP_DRV_Enable(0);
    /* Start lpTMR0 */
    lpTMR_DRV_StartCounter(0);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* If voltage of channel 0 is less than channel 1, red led light off */
        RED_LED_OFF();
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
    ACMP_DRV_Init(0, &acmp_config0);
    lpTMR_DRV_Init(0, &LPTMR_Config, true);
    TMU_DRV_Init(0, &tmu_config0);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
