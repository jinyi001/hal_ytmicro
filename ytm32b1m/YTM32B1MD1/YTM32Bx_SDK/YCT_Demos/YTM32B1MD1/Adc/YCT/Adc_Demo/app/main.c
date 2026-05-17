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
#define PRINTF_UART (2) /* UART instance for PRINTF */
#define ADC_INST    (0) /* ADC instance */
#define SEQ_LEN     (8) /* channel number */
#define VREFH       (5000U)
#define MAX_CODE    (4096U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint16_t adcData[SEQ_LEN];
volatile bool adcDone = false;
volatile bool adcError = false;

/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
void ADC0_IRQHandler(void)
{
    ADC_DRV_ClearEoseqFlagCmd(0);

    for (int i = 0; i < SEQ_LEN; i++)
    {
        adcData[i] = ADC_DRV_ReadFIFO(0);
    }
    adcDone = true;
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
    uint32_t i = 10;
    uint32_t j = 0;
    uint8_t channel = 0;
    uint32_t voltage;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Hello ADC!\r\n");
    PRINTF("Force Channel 0 to 4V and Force Channel 1 to 3V.\n");

    while (i--)
    {
        /* Clear adcDone */
        adcDone = false;
        ADC_DRV_Start(ADC_INST);
        while (!adcDone)
            ;
        for (j = 0; j < adc_config0.sequenceConfig.totalChannels; j++)
        {
            channel = adc_config0.sequenceConfig.channels[j];
            voltage = adcData[j] * VREFH / MAX_CODE;
            /* Check several channel voltage */
            if (channel == 0)
            {
                PRINTF("Channel 0 force 4000mV, actual %dmV.\n", voltage);
                if (voltage < 3800 || voltage > 4200)
                {
                    adcError = true;
                }
            }
            else if (channel == 1)
            {
                PRINTF("Channel 1 force 3000mV, actual %dmV.\n", voltage);
                if (voltage < 2800 || voltage > 3200)
                {
                    adcError = true;
                }
            }
            else if (channel == 33)
            {
                PRINTF("PMC VREF expected 1200mV, actual %dmV.\n", voltage);
                if (voltage < 1100 || voltage > 1300)
                {
                    adcError = true;
                }
            }
            else if (channel == 34)
            {
                PRINTF("VREFH expected 5000mV, actual %dmV.\n", voltage);
                if (voltage < 4900)
                {
                    adcError = true;
                }
            }
            else if (channel == 35)
            {
                PRINTF("VREFL expected 0V, actual %dmV.\n", voltage);
                if (voltage > 100)
                {
                    adcError = true;
                }
            }
            else
            {
                PRINTF("Channel %d voltage is %dmV.\n", channel, voltage);
            }
        }
        PRINTF("==========================\r\n");
    }
    if (adcError)
    {
        PRINTF("ADC Demo Fail!\r\n");
    }
    else
    {
        PRINTF("ADC Demo Pass!\r\n");
    }
    /* Disable ADC0 */
    ADC_DRV_Disable(0);
    PRINTF("ADC Demo End!\r\n");
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
    if (STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX, CLOCK_MANAGER_POLICY_AGREEMENT))
    {
        /* USER CODE BEGIN ERROR_HANDLER 1 */
        SystemSoftwareReset();
        /* USER CODE END ERROR_HANDLER 1 */
    }
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    ADC_DRV_ConfigConverter(0, &adc_config0);
    UTILITY_PRINT_Init();
    INT_SYS_EnableIRQ(ADC0_IRQn);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
