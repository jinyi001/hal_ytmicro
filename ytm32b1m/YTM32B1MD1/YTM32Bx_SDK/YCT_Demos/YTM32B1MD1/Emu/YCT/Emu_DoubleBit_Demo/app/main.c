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
#define MAGIC_MARKER (0x5AA56996)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
void Emu_HardFaultISR(void);
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Emu_HardFaultISR(void)
{
    /* Interruption of printing will take a certain amount of time. 
    This is only for demo demonstration. 
    In actual applications, you need to delete printing.
    */
    PRINTF("Enter the hardfault program.\n");
    PRINTF("Ready for software reset to clear ecc fault...\n");
    /* Set one reg for error record which will store after reset */
    REGFILE->DR[7] = MAGIC_MARKER;
    /* Software reset to clear ECC error. */
    SystemSoftwareReset();
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */
    uint32_t reportErrAddr = 0;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    if (REGFILE->DR[7] == MAGIC_MARKER)
    {
        /* Clear reg at first */
        REGFILE->DR[7] = 0;
        PRINTF("System reset by software reset!\n");
        /* log error addr */
        reportErrAddr = EMU_DRV_GetErrReportAddr(0, EMU_Config0.channel);
        PRINTF("injectErrAddr:%x\n", EMU_Config0.injectErrAddr);
        PRINTF("reportErrAddr:%x\n", reportErrAddr);

        if (EMU_Config0.injectErrAddr != reportErrAddr)
        {
            PRINTF("EMU double bit inject Demo Fail!\n");
        }
        else
        {
            PRINTF("EMU double bit inject Demo Pass!\n");
        }
        PRINTF("EMU double bit inject Demo End!\n");
    }
    else
    {
        PRINTF("Build %s %s\n", __DATE__, __TIME__);
        PRINTF("Hello EMU double bit.\n");

        INT_SYS_InstallHandler(HardFault_IRQn, Emu_HardFaultISR, NULL);

        /* Initialize */
        EMU_DRV_Init(0, &EMU_Config0, 1);
        /* Read memory,it will trigger ECC error.
        It can cause hardfault. */
        *(volatile uint32_t *)(EMU_Config0.injectErrAddr);
        /* If 1s no EMU error happens, Failed. */
        OSIF_TimeDelay(1000);
        PRINTF("EMU double bit inject Demo Fail.\n");
        PRINTF("EMU double bit inject Demo End!\n");
    }

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
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
