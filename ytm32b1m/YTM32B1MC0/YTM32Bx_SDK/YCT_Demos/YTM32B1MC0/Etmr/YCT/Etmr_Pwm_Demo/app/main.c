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
#define ETMR_INST           0
#define ETMR_CHANNEL_INDEX0 2
#define ETMR_CHANNEL_INDEX1 3
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
    /* PWM is generated when eTMR enable, the duty cycle is configured in the initial function */
    eTMR_DRV_Enable(ETMR_INST);
    OSIF_TimeDelay(10);

    /* The following code shows how to switch the different duty cycle */
    /* channel 2 duty cycle is 50% */
    eTMR_DRV_UpdatePwmChannel(ETMR_INST, ETMR_CHANNEL_INDEX0, 0x4000U, 0);
    /* channel 3 duty cycle is 75% */
    eTMR_DRV_UpdatePwmChannel(ETMR_INST, ETMR_CHANNEL_INDEX1, 0x6000U, 0);
    eTMR_DRV_SetLdok(ETMR_INST);
    OSIF_TimeDelay(10);

    /* Here use chmask register to accomplish 100% duty cycle. */
    g_etmrBase[ETMR_INST]->CHMASK = (1 << eTMR_CHMASK_CH2M_SHIFT) | (1 << eTMR_CHMASK_CH3M_SHIFT) |
                                    (1 << eTMR_CHMASK_CH2MV_SHIFT) | (1 << eTMR_CHMASK_CH3MV_SHIFT);
    eTMR_DRV_SetLdok(ETMR_INST);
    OSIF_TimeDelay(10);

    /* channel 2 duty cycle is 0, first cancel chmask enable */
    g_etmrBase[ETMR_INST]->CHMASK = (0 << eTMR_CHMASK_CH2M_SHIFT) | (0 << eTMR_CHMASK_CH3M_SHIFT);
    eTMR_DRV_UpdatePwmChannel(ETMR_INST, ETMR_CHANNEL_INDEX0, 0, 0);
    /* channel 3 duty cycle is 0 */
    eTMR_DRV_UpdatePwmChannel(ETMR_INST, ETMR_CHANNEL_INDEX1, 0, 0);
    eTMR_DRV_SetLdok(ETMR_INST);
    OSIF_TimeDelay(10);

    /* channel 2 duty cycle is 0x6000 */
    eTMR_DRV_UpdatePwmChannel(ETMR_INST, ETMR_CHANNEL_INDEX0, 0x6000U, 0);
    /* channel 3 duty cycle is 0x4000 */
    eTMR_DRV_UpdatePwmChannel(ETMR_INST, ETMR_CHANNEL_INDEX1, 0x4000U, 0);
    eTMR_DRV_SetLdok(ETMR_INST);
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
    eTMR_DRV_Init(0, &ETMR_CM_Config0, &ETMR_CM_Config0_State);
    eTMR_DRV_InitPwm(0, &ETMR_PWM_Config0);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
