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
#define SW1_PORT        (GPIOE)
#define SW1_PORT_INDEX  (0U)
#define SW2_PORT        (GPIOE)
#define SW2_PORT_INDEX  (1U)
#define LED1_PORT       (GPIOD)
#define LED1_PORT_INDEX (7U)
#define LED2_PORT       (GPIOD)
#define LED2_PORT_INDEX (6U)

/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
void GPIOE_IRQHandler(void)
{
    if (PINS_DRV_GetPortIntFlag(SW1_PORT) & (1 << SW1_PORT_INDEX))
    {
        /* SW1 is pressed, toggle LED */
        PINS_DRV_TogglePins(LED1_PORT, (1 << LED1_PORT_INDEX));

        /* Clear interrupt flag. */
        PINS_DRV_ClearPinIntFlagCmd(SW1_PORT, SW1_PORT_INDEX);
    }
    if (PINS_DRV_GetPortIntFlag(SW2_PORT) & (1 << SW2_PORT_INDEX))
    {
        /* SW2 is pressed, toggle LED2 */
        PINS_DRV_TogglePins(LED2_PORT, (1 << LED2_PORT_INDEX));
        /* Clear interrupt flag. */
        PINS_DRV_ClearPinIntFlagCmd(SW2_PORT, SW2_PORT_INDEX);
    }
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
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Hello GPIO interrupt!\r\n");
    PRINTF("PRESSING SW1 will toggle LED in GPIO interrupt!\r\n");

    /* enable the SW GPIO IRQ interrupt */
    INT_SYS_EnableIRQ(GPIOE_IRQn);
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
