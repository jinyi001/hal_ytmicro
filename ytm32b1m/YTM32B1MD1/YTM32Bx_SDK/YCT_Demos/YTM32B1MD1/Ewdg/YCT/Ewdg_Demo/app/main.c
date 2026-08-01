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
#define EWDG0_IRQn WDG0_IRQn
#define EWDG_INST  0

#define GetEwdgOutput() (((PINS_DRV_ReadPins(GPIOE) >> 7) & 0x01))
#define SetEwdgInput(x) (PINS_DRV_WritePin(GPIOE, 13, x))

#define MAGIC_PASS_NUM (0x5AA56996U)
#define MAGIC_FAIL_NUM (0x11223344U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile bool ewdgError = false;

/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
void EWDG_Isr(void)
{
    PRINTF("EWDG interrupt happened!\n");
    /* Disable EWDG interrupt */
    EWDG_DRV_DisableInterrupt(EWDG_INST);
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
    status_t status = STATUS_SUCCESS;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */

    if (REGFILE->DR[7] == 0)
    {
        PRINTF("Build %s %s\n", __DATE__, __TIME__);
        PRINTF("EWDG Demo!\r\n");
        PRINTF("In this mode, PTE7 is connected to EWDG_OUT_b and PTE13 is connected to EWDG_IN.\n");
        PRINTF("Only when EWDG_IN high, EWDG can feed dog.\n");
        /* Install interrupt handler for EWDG */
        INT_SYS_InstallHandler(EWDG0_IRQn, &EWDG_Isr, (isr_t *)NULL);
        /* Enable EWDG interrupt */
        INT_SYS_EnableIRQ(EWDG0_IRQn);
        EWDG_DRV_Init(EWDG_INST, &ewdg_config0);
        /* EWDG_IN input high */
        for (uint8_t idx = 0; idx < 8; idx++)
        {
            OSIF_TimeDelay(1);
            EWDG_DRV_Refresh(EWDG_INST);
            if (GetEwdgOutput() == 1)
            {
                /* Feed EWDG successfully! */
            }
            else
            {
                ewdgError = true;
                PRINTF("Feed EWDG failed!\n");
            }
        }
        OSIF_TimeDelay(50);
        if (GetEwdgOutput() == 0)
        {
            PRINTF("Refreshed timeout, The EWDG_OUT_b signal is low!\n");
        }
        else
        {
            ewdgError = true;
            PRINTF("Error happens! EWDG not timeout after 50ms.\n");
        }
        EWDG_DRV_DeInit(EWDG_INST);
        /* Record current test result, REGFILE can keep value after reset */
        if (ewdgError)
        {
            REGFILE->DR[7] = MAGIC_FAIL_NUM;
        }
        else
        {
            REGFILE->DR[7] = MAGIC_PASS_NUM;
        }
        PRINTF("EWDG can't re-enable except MCU reset. Reset chip for another test.\n");
        SystemSoftwareReset();
    }
    else
    {
        PRINTF("Software reset by anther case.\n");
        if (REGFILE->DR[7] == MAGIC_FAIL_NUM)
        {
            ewdgError = true;
            PRINTF("EWDG Demo Failed.\n");
        }
        /* clear REGFILE */
        REGFILE->DR[7] = 0;
        PRINTF("If EWDG_IN low, feed dog will failed. And EWDG_OUT_b will be low immediately.\n");
        EWDG_DRV_Init(EWDG_INST, &ewdg_config0);
        /* EWDG_IN input Low */
        SetEwdgInput(0);
        /* add announce  */
        if (GetEwdgOutput() == 0)
        {
            ewdgError = true;
            PRINTF("EWDG Deinit fail\n");
        }
        EWDG_DRV_Refresh(EWDG_INST);
        if (GetEwdgOutput() == 0)
        {
            PRINTF("EWDG_IN input low, service EWDG, The EWDG_OUT_b is low!\n");
        }
        else
        {
            ewdgError = true;
            PRINTF("EWDG_IN input low fail!\n");
        }
        if (!ewdgError)
        {
            PRINTF("EWDG Demo Pass!\n");
        }
        else
        {
            PRINTF("EWDG Demo Fail!\n");
        }
        EWDG_DRV_DeInit(EWDG_INST);
        PRINTF("EWDG Demo End.\n");
    }
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
