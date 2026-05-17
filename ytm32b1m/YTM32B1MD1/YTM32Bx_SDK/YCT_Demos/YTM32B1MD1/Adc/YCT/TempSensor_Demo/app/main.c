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
#include "floats.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADC_INST     (0U)
#define VREFH        (float32_t)(5.0F)
#define ADRESULT_DIV (float32_t)(4096.0F)
#define CONSTANT_K   (float32_t)(530.0F)
#define ADC0_TSC     (uint32_t *)(0x40040054U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint16_t adcData = 0U;
float32_t sampleData = 0.0F;
uint16_t coefficient = 0U;
float32_t coreTemp = 0.0F;
volatile bool adcDone = false;
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
static void ADC0_SetTempSensConfig(void);
static void ADC0_Get_Coefficient(void);
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void ADC0_IRQHandler(void)
{
    ADC_DRV_ClearEoseqFlagCmd(ADC_INST);

    /* T = C - K * V; V = (D/4096) * Vrefh */
    adcData = ADC_DRV_ReadFIFO(ADC_INST);

    sampleData = (((float32_t)adcData * VREFH) / ADRESULT_DIV);

    coreTemp = (float32_t)coefficient - (sampleData * CONSTANT_K);

    adcDone = true;
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
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Hello Temp Sensor!\r\n");

    ADC0_SetTempSensConfig();

    ADC0_Get_Coefficient();

    ADC_DRV_Start(ADC_INST);
    while (!adcDone)
        ;
    PRINTF("Core Temperature is %.2f\r\n", coreTemp);
    if ((coreTemp < 10) || (coreTemp > 45))
    {
        PRINTF("Temp Sensor Demo Fail! Current Temperature over range.\r\n");
    }
    else
    {
        PRINTF("Temp Sensor Demo Pass! Current Temperature in range.\r\n");
    }
    /* Disable ADC0 */
    ADC_DRV_Disable(0);
    PRINTF("Temp Sensor Demo End.\r\n");
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
    CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX, CLOCK_MANAGER_POLICY_AGREEMENT);
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    INT_SYS_EnableIRQ(ADC0_IRQn);
    ADC_DRV_ConfigConverter(0, &adc_config0);
    UTILITY_PRINT_Init();
}

/* USER CODE BEGIN 4 */
static void ADC0_SetTempSensConfig(void)
{
    /* enable temperature sensor by ADC0_CTRL[TSEN] write 1*/
    ADC0->CTRL |= ADC_CTRL_TSEN_MASK;
}

static void ADC0_Get_Coefficient(void)
{
    /* Read coefficient calibrated C */
    coefficient = (*(ADC0_TSC));
}

/* USER CODE END 4 */
