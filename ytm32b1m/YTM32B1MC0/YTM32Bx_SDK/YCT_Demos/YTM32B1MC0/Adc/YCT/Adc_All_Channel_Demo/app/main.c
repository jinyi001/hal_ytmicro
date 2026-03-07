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
#define PRINTF_UART (3) /* UART instance for PRINTF */
#define SEQ_LEN     (8) /* channel number */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static uint32_t data;

/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
void ADC0_IRQHandler(void)
{
    ADC_DRV_ClearEoseqFlagCmd(0);

    for (int i = 0; i < SEQ_LEN; i++)
    {
        data = ADC0->FIFO;
        /* Note: Debug output inserted into interrupt routine for demo clarity. Might introduce delay. */
        PRINTF("ADC0 channel %d value is %d\r\n", data >> 16, data & 0xFFFF);
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
    uint32_t i = 0;
    status_t status = STATUS_SUCCESS;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    INT_SYS_EnableIRQ(ADC0_IRQn);
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Hello ADC ALL CHANNELS!\r\n");

    for (i = 0; i < 2; i++)
    {
        ADC_DRV_ConfigConverter(0, &adc_config0);
        ADC_DRV_Start(0);
        OSIF_TimeDelay(500);
        PRINTF("=======\r\n");

        ADC_DRV_ConfigConverter(0, &adc_config1);
        ADC_DRV_Start(0);
        OSIF_TimeDelay(500);
        PRINTF("=======\r\n");

        ADC_DRV_ConfigConverter(0, &adc_config2);
        ADC_DRV_Start(0);
        OSIF_TimeDelay(500);
        PRINTF("=======\r\n");

        ADC_DRV_ConfigConverter(0, &adc_config3);
        ADC_DRV_Start(0);
        OSIF_TimeDelay(500);
        PRINTF("=======\r\n");
    }
    PRINTF("DEMO END\n\n");
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
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
