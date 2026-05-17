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
#define CHANNEL_NUM        2
#define BOARD_RED_LED      (GPIOE)
#define BOARD_RED_LED_PIN  (8)
#define BOARD_BLUE_LED     (GPIOB)
#define BOARD_BLUE_LED_PIN (4)
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
volatile bool channel0Flag = false;
volatile bool channel1Flag = false;
volatile bool acmpError = false;

/* DAC config for self test */
acmp_dac_config_t acmpDacCfg = {
    .enable = true,
    .voltage = 128,
};
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
                channel0Flag = true;
                /* Red led light on */
                RED_LED_ON();
            }
            else
            {
                channel1Flag = true;
                /* Blue led light on */
                BLUE_LED_ON();
            }
        }
    }
    /* Clear channel flags to exit interrupt normally */
    ACMP_DRV_ClearChannelFlags(0);
}

/* Off blue LED and red LED */
void LedOff(void)
{
    RED_LED_OFF();
    BLUE_LED_OFF();
}

/* ACMP self test check */
void AcmpSelfCheck(bool channel0Result, bool channel1Result)
{
    if (channel0Result != channel0Flag)
    {
        acmpError = true;
        PRINTF("ACMP Channel 0 Fail. Unexpected.\n");
    }
    if (channel1Result != channel1Flag)
    {
        acmpError = true;
        PRINTF("ACMP Channel 1 Fail. Unexpected.\n");
    }
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
    PRINTF("Hello ACMP!\r\n");
    PRINTF("Force 4V to channel 0 and force 3V to channel 1.\n");
    /* Enable ACMP0_IRQn */
    INT_SYS_EnableIRQ(ACMP0_IRQn);
    /* Enable ACMP module */
    ACMP_DRV_Enable(0);
    /* Check channel 0/1 if set flag */
    LedOff();
    OSIF_TimeDelay(1);
    /* Disable ACMP to continue test */
    ACMP_DRV_Disable(0);
    PRINTF("Current DAC set 2.5V. ACMP channel 0/1 will set flag.\n");
    AcmpSelfCheck(true, true);

    /* Change DAC voltage and test again */
    PRINTF("Change DAC to 3.5V. Only channel 0 will set flag.\n");
    acmpDacCfg.voltage = 35U * 256 / 50;
    ACMP_DRV_ConfigDac(0, &acmpDacCfg);
    channel0Flag = false;
    channel1Flag = false;
    /* Enable ACMP module */
    ACMP_DRV_Enable(0);
    /* Check channel 0 if set flag */
    LedOff();
    OSIF_TimeDelay(1);
    /* Disable ACMP to continue test */
    ACMP_DRV_Disable(0);
    AcmpSelfCheck(true, false);

    /* Change DAC voltage and test again */
    PRINTF("Change DAC to 4.5V. No channel will set flag.\n");
    acmpDacCfg.voltage = 45U * 256 / 50;
    ACMP_DRV_ConfigDac(0, &acmpDacCfg);
    channel0Flag = false;
    channel1Flag = false;
    /* Enable ACMP module */
    ACMP_DRV_Enable(0);
    LedOff();
    OSIF_TimeDelay(1);
    /* Disable ACMP to continue test */
    ACMP_DRV_Disable(0);
    AcmpSelfCheck(false, false);

    /* Deinit ACMP */
    ACMP_DRV_Reset(0);

    if (!acmpError)
    {
        PRINTF("ACMP Continuous Demo Pass!\n");
    }
    else
    {
        PRINTF("ACMP Continuous Demo Fail!\n");
    }
    /* Deinit ACMP */
    ACMP_DRV_Reset(0);
    PRINTF("ACMP Continuous Demo End.\n");
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
    ACMP_DRV_Init(0, &ACMP_Config0);
    UTILITY_PRINT_Init();
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
