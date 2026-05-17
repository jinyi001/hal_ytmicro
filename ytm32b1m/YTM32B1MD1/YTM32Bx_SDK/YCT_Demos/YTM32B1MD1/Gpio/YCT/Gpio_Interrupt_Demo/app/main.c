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
#define SW2_PORT       (GPIOB)
#define SW2_PORT_INDEX (2U)
#define LED_PORT       (GPIOB)
#define LED_PORT_INDEX (5U)

#define TEST_COUNT    (5U)
#define TEST_FAIL_CNT (20U)

#define TOGGLE_PIN_FOR_TEST() (PINS_DRV_TogglePins(GPIOC, 1 << 13))
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint8_t gpioIrqCount = 0;
volatile uint32_t testCnt = 0;
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void GPIOB_IRQHandler(void)
{
    if (PINS_DRV_GetPortIntFlag(SW2_PORT) & (1 << SW2_PORT_INDEX))
    {
        /* SW2 is pressed, toggle LED */
        PINS_DRV_TogglePins(LED_PORT, (1 << LED_PORT_INDEX));
        /* Clear pin interrupt flag.*/
        PINS_DRV_ClearPinIntFlagCmd(SW2_PORT, SW2_PORT_INDEX);
    }
    gpioIrqCount++;
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
    PRINTF("Hello GPIO interrupt!\r\n");
    PRINTF("PRESSING SW2 will toggle LED in GPIO interrupt!\r\n");
    PRINTF("You can short PTC13 and PTB2(SW2) for self test.\n");
    /* enable the SW GPIO IRQ interrupt */
    INT_SYS_EnableIRQ(GPIOB_IRQn);
    while (gpioIrqCount < TEST_COUNT)
    {
        /* Short PTC13 and PTB2 for self-test */
        TOGGLE_PIN_FOR_TEST();
        OSIF_TimeDelay(1);
        if (testCnt++ >= TEST_FAIL_CNT)
        {
            PRINTF("GPIO Interrupt Self Test Timeout. Please check pin connection.\n");
        }
    }
    if (testCnt > (TEST_COUNT * 2))
    {
        PRINTF("GPIO Interrupt Demo Fail!\n");
    }
    else
    {
        PRINTF("GPIO Interrupt Demo Pass!\n");
    }
    PRINTF("GPIO Interrupt Demo End!\n");
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
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
