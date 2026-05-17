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
#include "floats.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define PRINTF_UART     (2) /* UART instance for PRINTF */
#define SEQ_LEN         (8) /* channel number */
#define ADC_CONFIG_CNTS (5)
#define VREFH           (float32_t)(5.0F)
#define ADRESULT_DIV    (float32_t)(4096.0F)
#define CONSTANT_K      (float32_t)(530.0F)
#define ADC0_TSC        (uint32_t *)(0x40040054U)
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint16_t adcData[SEQ_LEN];
volatile bool adcDone = false;
volatile bool adcError = false;
uint16_t Calibrated_C = 0U;
float32_t CoreTemp = 0.0F;
adc_converter_config_t const *adcConfig[ADC_CONFIG_CNTS] = {
    &adc_config0, &adc_config1, &adc_config2, &adc_config3, &adc_config4,
};

/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
void ADC0_IRQHandler(void)
{
    /* End of seq check */
    ADC_DRV_ClearEoseqFlagCmd(0);
    for (uint8_t i = 0; i < SEQ_LEN; i++)
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
    uint32_t i = 0;
    uint32_t j = 0;
    uint8_t channel = 0;
    float32_t voltage;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Hello All ADC Channels!\r\n");
    PRINTF("Force Channel 0 to 4V and Force Channel 1 to 3V.\n");
    /* enable temperature sensor by ADC0_CTRL[TSEN] write 1*/
    ADC0->CTRL |= ADC_CTRL_TSEN_MASK;
    /* Read coefficient calibrated C */
    Calibrated_C = (*(ADC0_TSC));

    for (i = 0; i < ADC_CONFIG_CNTS; i++)
    {
        /* Config ADC by i */
        ADC_DRV_ConfigConverter(0, adcConfig[i]);
        /* Clear adcDone */
        adcDone = false;
        ADC_DRV_Start(0);
        while (!adcDone)
            ;
        for (j = 0; j < adcConfig[i]->sequenceConfig.totalChannels; j++)
        {
            channel = adcConfig[i]->sequenceConfig.channels[j];
            voltage = adcData[j] * VREFH / ADRESULT_DIV;
            /* Check several channel voltage */
            if (channel == 0)
            {
                PRINTF("Channel 0 force 4V, actual %.3fV.\n", voltage);
                if (voltage < 3.8F || voltage > 4.2F)
                {
                    adcError = true;
                }
            }
            else if (channel == 1)
            {
                PRINTF("Channel 1 force 3V, actual %.3fV.\n", voltage);
                if (voltage < 2.8F || voltage > 3.2F)
                {
                    adcError = true;
                }
            }
            else if (channel == 32)
            {
                CoreTemp = (float32_t)Calibrated_C - (voltage * CONSTANT_K);
                PRINTF("Core temperature is %.1f.\n", CoreTemp);
                if (CoreTemp < 10 || CoreTemp > 45)
                {
                    adcError = true;
                }
            }
            else if (channel == 33)
            {
                PRINTF("PMC VREF expected 1.2V, actual %.3fV.\n", voltage);
                if (voltage < 1.1 || voltage > 1.3)
                {
                    adcError = true;
                }
            }
            else if (channel == 34)
            {
                PRINTF("VREFH expected 5.0V, actual %.3fV.\n", voltage);
                if (voltage < 4.9)
                {
                    adcError = true;
                }
            }
            else if (channel == 35)
            {
                PRINTF("VREFL expected 0V, actual %.3fV.\n", voltage);
                if (voltage > 0.1)
                {
                    adcError = true;
                }
            }
            else
            {
                PRINTF("Channel %d voltage is %.3fV.\n", channel, voltage);
            }
        }
        PRINTF("==========================\r\n");
    }

    if (!adcError)
    {
        PRINTF("ADC All Channel Demo Pass!\r\n");
    }
    else
    {
        PRINTF("ADC All Channel Demo Fail!\r\n");
    }
    /* Disable ADC0 */
    ADC_DRV_Disable(0);
    PRINTF("ADC All Channel Demo End!\r\n");
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
    UTILITY_PRINT_Init();
    INT_SYS_EnableIRQ(ADC0_IRQn);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
