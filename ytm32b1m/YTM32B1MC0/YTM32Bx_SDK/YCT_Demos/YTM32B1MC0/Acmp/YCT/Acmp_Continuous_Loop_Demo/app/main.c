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
#define CHANNEL_NUM        2
#define BOARD_RED_LED      (GPIOD)
#define BOARD_RED_LED_PIN  (5)
#define BOARD_BLUE_LED     (GPIOD)
#define BOARD_BLUE_LED_PIN (7)
/* Red led */
#define RED_LED_OFF()      PINS_DRV_SetPins(BOARD_RED_LED, (1 << BOARD_RED_LED_PIN))
#define RED_LED_ON()       PINS_DRV_ClearPins(BOARD_RED_LED, (1 << BOARD_RED_LED_PIN))
/* Blue led */
#define BLUE_LED_OFF()     PINS_DRV_SetPins(BOARD_BLUE_LED, (1 << BOARD_BLUE_LED_PIN))
#define BLUE_LED_ON()      PINS_DRV_ClearPins(BOARD_BLUE_LED, (1 << BOARD_BLUE_LED_PIN))
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
void ACMP0_IRQHandler(void)
{
    for (uint8_t channel = 0; channel < CHANNEL_NUM; channel++)
    {
        /* If channel's voltage is greater than DAC voltage */
        if (ACMP_DRV_GetChannelOutput(0, channel))
        {
            if (channel == 0)
            {
                /* Red led light on */
                RED_LED_ON();
            }
            else
            {
                /* Blue led light on */
                BLUE_LED_ON();
            }
        }
    }
    /* Clear channel flags to exit interrupt normally */
    ACMP_DRV_ClearChannelFlags(0);
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
    /* Enable ACMP interrupt */
    INT_SYS_EnableIRQ(ACMP0_IRQn);
    /* Enable ACMP */
    ACMP_DRV_Enable(0);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* Light off red and blue led if channel's voltage is less than DAC voltage */
        RED_LED_OFF();
        BLUE_LED_OFF();
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
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
