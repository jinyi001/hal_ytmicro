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
#define WKU_INST (0U)

#define STANDBY_MODE   (0U)
#define POWERDOWN_MODE (1U)

// #define LOW_POWER_MODE  STANDBY_MODE
#define LOW_POWER_MODE POWERDOWN_MODE

#define WKU_HW_CHANNEL (6U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint32_t wkuSts = 0;
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void GPIOC_IRQHandler(void)
{
    PINS_DRV_ClearPinIntFlagCmd(GPIOC, 1);
}

void WKU_IRQHandler(void)
{
    /* Wakeup from power down mode, MCU will enter WKU ISR */
    wkuSts = WKU->PFR[0];
    /* Clear status */
    WKU->PFR[0] = WKU->PFR[0];
    /* Add DSB to avoid re-entry irq */
    __DSB();
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
    /* Disable Isolation to relieve all pins, otherwise can't print or toggle GPIO */
    WKU_DRV_SetPinIsolation(false);
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Power Mode Demo!\r\n");
    PRINTF("If not Power Down mode, PTC1 gpio interrupt wakeup MCU\n");
    PRINTF("In Power Down, WKU wake up MCU.\n");
    OSIF_TimeDelay(5000);

    /* Wakeup from power down mode, MCU will enter WKU ISR */
    if (wkuSts != 0U)
    {
        PRINTF("WKU->PFR = 0x%08X\n", wkuSts);
        PRINTF("PTC1 is WKU channel 6, expected status is 0x%08X\n", 1 << WKU_HW_CHANNEL);
    }

    WKU_DRV_InitReset(WKU_INST, &wkup_resetConfig0);
    WKU_DRV_InitPinWakeup(WKU_INST, WKP_PIN_CH_NUM0, wkup_pinsConfig0);
    PRINTF("Wakeup count: %d\n", REGFILE->DR[0]);

    /* if POR or Reset pin */
    if ((RCU_RSSR_POR_LVD_MASK | RCU_RSSR_PIN_MASK) & RCU->RSSR)
    {
        PRINTF("POR or PIN reset.\n");
        RCU->RSSR = RCU_RSSR_POR_LVD_MASK | RCU_RSSR_PIN_MASK;
    }
    else
    {
        REGFILE->DR[0]++;
        PRINTF("Powerdown wakeup count: %d\n", REGFILE->DR[0]);
    }

    PRINTF("Wait to enter low power mode.\n");
    OSIF_TimeDelay(1000);
    PRINTF("Start to enter low power mode.\n");
    PRINTF("Press SW3(PTC1) to wake up MCU.\n");
    /* Enable Isolation, all pins will keep current value until disable Isolation. */
    WKU_DRV_SetPinIsolation(true);
    // Enter into low power mode
    POWER_SYS_SetMode(LOW_POWER_MODE, POWER_MANAGER_POLICY_AGREEMENT);
    PRINTF("Device has been waken up from standby or deepsleep mode!\n");
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
    if (STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX, CLOCK_MANAGER_POLICY_AGREEMENT))
    {
        /* USER CODE BEGIN ERROR_HANDLER 1 */
        SystemSoftwareReset();
        /* USER CODE END ERROR_HANDLER 1 */
    }
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    INT_SYS_ConfigInit();
    UTILITY_PRINT_Init();
    POWER_SYS_Init(&powerConfigsArr, POWER_MANAGER_CONFIG_CNT, NULL, POWER_MANAGER_CALLBACK_CNT);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
