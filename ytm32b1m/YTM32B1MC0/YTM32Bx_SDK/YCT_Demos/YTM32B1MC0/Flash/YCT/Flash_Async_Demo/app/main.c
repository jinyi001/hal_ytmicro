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
#define FLASH_INST (0U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
void flash_callback(flash_event_t event);
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void flash_callback(flash_event_t event)
{
    /* Note: Debug output inserted into interrupt routine for demo clarity. Might introduce delay. */
    PRINTF("Flash callback, ");
    if (event == FLASH_EVENT_COMPLETE)
    {
        PRINTF("operation complete\r\n");
    }
    else if (event == FLASH_EVENT_ERROR)
    {
        PRINTF("operation error!\r\n");
    }
    else
    {
        PRINTF("access error!\r\n");
    }
}

void lpTMR0_IRQHandler(void)
{
    /* Note: Debug output inserted into interrupt routine for demo clarity. Might introduce delay. */
    PRINTF("LpTMR ISR enter!\n");
    /* Clear compare flag */
    lpTMR_DRV_ClearCompareFlag(0);
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
    uint32_t i;
    uint32_t addr;
    uint32_t data[10240 / 4];
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Flash Async Demo!\r\n");

    /* PFlash0 and DFlash0 is in Block0; PFlash1 and DFlash1 is in Block1*/
    PRINTF("Asynchronous mode can only be used for data where operations and programs are on different blocks!\r\n");
    PRINTF("So can only operate the PFlash1 and DFlash1, if the program only exists on the block0.\r\n");

    PRINTF("Checking PFlash1 from 0x20000 to 0x22800!\r\n");
    status |= FLASH_DRV_EraseSector(FLASH_INST, 0x20000, 10 * 1024);

    /* Write 10K from 0x20000 to 0x22800 */
    for (i = 0; i < 0x2800 / 4; i++)
    {
        data[i] = 0x20000 + 4 * i;
    }

    while (FLASH_DRV_GetBusyStatus(FLASH_INST))
        ;
    status |= FLASH_DRV_Program(FLASH_INST, 0x20000, 10 * 1024, data);
    /* Wait for the program to complete, because check data would access program address */
    while (FLASH_DRV_GetBusyStatus(FLASH_INST))
        ;

    for (addr = 0x20000; addr < 0x22800; addr += 4)
    {
        if (*(uint32_t *)addr != addr)
        {
            PRINTF("Address %x program failed!\r\n", addr);
        }
    }

    PRINTF("Checking DFlash1 from 0x10011000 to 0x10013800!\r\n");
    status |= FLASH_DRV_EraseSector(FLASH_INST, 0x10011000, 10 * 1024);
    /* Write 10K from 0x10011000 to 0x10013800 */
    for (i = 0; i < 0x2800 / 4; i++)
    {
        data[i] = 0x10011000 + 4 * i;
    }

    while (FLASH_DRV_GetBusyStatus(FLASH_INST))
        ;
    status |= FLASH_DRV_Program(FLASH_INST, 0x10011000, 10 * 1024, data);
    /* Wait for the program to complete, because check data would access program address */
    while (FLASH_DRV_GetBusyStatus(FLASH_INST))
        ;

    for (addr = 0x10011000; addr < 0x10013800; addr += 4)
    {
        if (*(uint32_t *)addr != addr)
        {
            PRINTF("Address %x program failed!\r\n", addr);
        }
    }
    lpTMR_DRV_StopCounter(0);
    PRINTF("Flash check finished.\n");
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
    FLASH_DRV_Init(FLASH_INST, &flash_config0, &flash_config0_State);
    lpTMR_DRV_Init(0, &LPTMR_Config, true);
    INT_SYS_ConfigInit();
    lpTMR_DRV_StartCounter(0);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
