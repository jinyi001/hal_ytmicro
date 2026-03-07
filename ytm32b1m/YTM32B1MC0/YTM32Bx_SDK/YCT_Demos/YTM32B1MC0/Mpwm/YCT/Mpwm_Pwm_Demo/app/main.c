/* USER CODE BEGIN Header */
/* you can remove the copyright */

/*
 *  Copyright 2020-2024 Yuntu Microelectronics Co., Ltd.
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
#define MPWM_INST   (0)
#define MPWM_CH3    (3)
#define NUM         (5)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint8_t g_lptmr_flag = 0;
uint32_t g_duty_cycle[NUM] = {0x1000, 0x2000, 0x4000, 0x6000, 0x7000};
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
    g_lptmr_flag = 1;
}
/* USER CODE END 0 */


/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */
    uint8_t i = 0;
    /* USER CODE END 1 */ 
    Board_Init();
    /* USER CODE BEGIN 2 */
    MPWM_DRV_Init(MPWM_INST, &MPWM_State0);
    MPWM_DRV_SetClkSrc(MPWM_INST, MPWM_BUS_CLOCK);
    MPWM_DRV_InitPwm(MPWM_INST, &MPWM_PWM_Config0);
    MPWM_DRV_EnableCounter(MPWM_INST, 1 << MPWM_CH3);
    INT_SYS_EnableIRQ(lpTMR0_IRQn);
    lpTMR_DRV_Init(0, &LptmrConfig, true);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        if (g_lptmr_flag)
        {
            g_lptmr_flag = 0;
            MPWM_DRV_UpdateChannelDutyCycle(MPWM_INST, MPWM_CH3, g_duty_cycle[i]);
            MPWM_DRV_SetLdok(MPWM_INST, 1 << MPWM_CH3);
            i++;
            if (i == NUM)
            {
                i = 0;
            }
        }
        /* USER CODE END WHILE */
        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

static void Board_Init(void)
{
    CLOCK_SYS_Init(g_clockManConfigsArr,CLOCK_MANAGER_CONFIG_CNT,g_clockManCallbacksArr,CLOCK_MANAGER_CALLBACK_CNT);
    if(STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX,CLOCK_MANAGER_POLICY_AGREEMENT))
    {
        /* USER CODE BEGIN ERROR_HANDLER 1 */
        SystemSoftwareReset();
        /* USER CODE END ERROR_HANDLER 1 */
    }
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0,g_pin_mux_InitConfigArr0);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
