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
#define CAPINST       (1U)
#define CAPINST_CHAN0 (0U)
#define CAPINST_CHAN1 (1U)
#define CAPINST_CHAN2 (2U)
#define CAPINST_CHAN3 (3U)
#define CAPINST_CHAN6 (6U)
#define CAPINST_CHAN7 (7U)
#define CHAN_NUM      (6U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint32_t period[6] = { 0 };
uint32_t dutyCycle[6] = { 0 };
uint32_t capChannel[CHAN_NUM] = { CAPINST_CHAN0, CAPINST_CHAN1, CAPINST_CHAN2, CAPINST_CHAN3, CAPINST_CHAN6, CAPINST_CHAN7 };
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
    eTMR_DRV_Enable(1);
    OSIF_TimeDelay(1);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        for (uint8_t i = 0; i < CHAN_NUM; i++)
        {
            if (eTMR_DRV_GetInputCaptureComplete(CAPINST, capChannel[i]))
            {
                eTMR_DRV_CalculateChannelCaptureValue(CAPINST, capChannel[i]);
                period[i] = eTMR_DRV_GetInputCaptureMeasurementPeriod(CAPINST, capChannel[i]);
                dutyCycle[i] = eTMR_DRV_GetInputCapturePositivePulseCount(CAPINST, capChannel[i]) * 100 / period[i];
                eTMR_DRV_ClearInputCaptureComplete(CAPINST, capChannel[i]);
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
    UTILITY_PRINT_Init();
    eTMR_DRV_Init(0,&ETMR_CM_Config0,&ETMR_CM_Config0_State);
    eTMR_DRV_InitPwm(0,&ETMR_PWM_Config0);
    eTMR_DRV_Enable(0);
    eTMR_DRV_Init(1,&ETMR_CM_Config1,&ETMR_CM_Config1_State);
    eTMR_DRV_InitInputCapture(1,&ETMR_IC_Config0);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
