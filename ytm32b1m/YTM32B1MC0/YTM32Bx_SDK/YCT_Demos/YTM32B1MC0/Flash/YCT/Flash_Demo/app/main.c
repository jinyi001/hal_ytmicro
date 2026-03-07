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
    uint32_t addr;
    uint32_t data[4];
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Flash demo!\r\n");

    PRINTF("Checking program flash 0!\r\n");
    /* Erase 4K from 0x18000 to 0x18FFF */
    status |= FLASH_DRV_EraseSector(FLASH_INST, 0x18000, 4 * 1024);
    /* Write 4K from 0x18000 to 0x18FFF */
    for (addr = 0x18000; addr < 0x18FFF; addr += 2 * 4)
    {
        data[0] = addr;
        data[1] = addr + 4;
        /* Write minimum 2 words */
        status |= FLASH_DRV_Program(FLASH_INST, addr, 2 * 4, data);
    }
    for (addr = 0x18000; addr < 0x18FFF; addr += 4)
    {
        if (*(uint32_t *)addr != addr)
        {
            PRINTF("Address %x program failed!\r\n", addr);
        }
    }
    PRINTF("Checking program flash 1!\r\n");
    /* When accessing block 1, if code running in block 0, no need to disable global interrupt */
    status |= FLASH_DRV_EraseSector(FLASH_INST, 0x20000, 6 * 1024);
    /* Write 6K from 0x20000 to 0x21800 */
    for (addr = 0x20000; addr < 0x21800; addr += 4 * 4)
    {
        data[0] = addr;
        data[1] = addr + 4;
        data[2] = addr + 8;
        data[3] = addr + 12;
        status |= FLASH_DRV_Program(FLASH_INST, addr, 4 * 4, data);
    }
    for (addr = 0x20000; addr < 0x21800; addr += 4)
    {
        if (*(uint32_t *)addr != addr)
        {
            PRINTF("Address %x program failed!\r\n", addr);
        }
    }

    PRINTF("Checking data flash 0!\r\n");
    status |= FLASH_DRV_EraseSector(FLASH_INST, 0x10001000, 0x400);
    /* Write 1K from 0x10001000 to 0x10001400 */
    for (addr = 0x10001000; addr < 0x10001400; addr += 4 * 4)
    {
        data[0] = addr;
        data[1] = addr + 4;
        data[2] = addr + 8;
        data[3] = addr + 12;
        status |= FLASH_DRV_Program(FLASH_INST, addr, 4 * 4, data);
    }
    for (addr = 0x10001000; addr < 0x10001400; addr += 4)
    {
        if (*(uint32_t *)addr != addr)
        {
            PRINTF("Address %x program failed!\r\n", addr);
        }
    }

    PRINTF("Checking data flash 1!\r\n");
    status |= FLASH_DRV_EraseSector(FLASH_INST, 0x10011000, 0x400);
    /* Write 1K from 0x10011000 to 0x10011400 */
    for (addr = 0x10011000; addr < 0x10011400; addr += 4 * 4)
    {
        data[0] = addr;
        data[1] = addr + 4;
        data[2] = addr + 8;
        data[3] = addr + 12;
        status |= FLASH_DRV_Program(FLASH_INST, addr, 4 * 4, data);
    }
    for (addr = 0x10011000; addr < 0x10011400; addr += 4)
    {
        if (*(uint32_t *)addr != addr)
        {
            PRINTF("Address %x program failed!\r\n", addr);
        }
    }
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
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
