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
#define FLASH_INST        (0U)
#define TEST_FLASH_ADDR_0 (0x32000U)
#define TEST_FLASH_LEN_0  (4 * 1024U)
#define TEST_FLASH_ADDR_1 (0x64000U)
#define TEST_FLASH_LEN_1  (6 * 1024U)
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile bool flashError = false;
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
    PRINTF("Flash Demo!\r\n");

    PRINTF("Checking program flash block 0 from 0x%x to 0x%x!\r\n",
           TEST_FLASH_ADDR_0,
           TEST_FLASH_ADDR_0 + TEST_FLASH_LEN_0);
    /* Erase 4K from 0x32000 to 0x32FFF */
    status |= FLASH_DRV_EraseSector(FLASH_INST, TEST_FLASH_ADDR_0, TEST_FLASH_LEN_0);
    /* Write 4K from 0x32000 to 0x32FFF */
    for (addr = TEST_FLASH_ADDR_0; addr < (TEST_FLASH_ADDR_0 + TEST_FLASH_LEN_0); addr += 2 * 4)
    {
        data[0] = addr;
        data[1] = addr + 4;
        /* Write minimum 2 words */
        status |= FLASH_DRV_Program(FLASH_INST, addr, 2 * 4, data);
    }
    for (addr = TEST_FLASH_ADDR_0; addr < (TEST_FLASH_ADDR_0 + TEST_FLASH_LEN_0); addr += 4)
    {
        if (*(uint32_t *)addr != addr)
        {
            flashError = true;
            PRINTF("Address %x program failed!\r\n", addr);
            PRINTF("Expected 0x%x, actual 0x%x.\r\n", addr, *(uint32_t *)addr);
        }
    }
    PRINTF("Flash block 0 check finished.\n");

    PRINTF("Checking program flash block 1 from 0x%x to 0x%x!\r\n",
           TEST_FLASH_ADDR_1,
           TEST_FLASH_ADDR_1 + TEST_FLASH_LEN_1);
    /* When accessing block 1, if code running in block 0, no need to disable global interrupt */
    status |= FLASH_DRV_EraseSector(FLASH_INST, TEST_FLASH_ADDR_1, TEST_FLASH_LEN_1);
    /* Write 6K from 0x64000 to 0x657FF */
    for (addr = TEST_FLASH_ADDR_1; addr < (TEST_FLASH_ADDR_1 + TEST_FLASH_LEN_1); addr += 4 * 4)
    {
        data[0] = addr;
        data[1] = addr + 4;
        data[2] = addr + 8;
        data[3] = addr + 12;
        status |= FLASH_DRV_Program(FLASH_INST, addr, 4 * 4, data);
    }
    for (addr = TEST_FLASH_ADDR_1; addr < (TEST_FLASH_ADDR_1 + TEST_FLASH_LEN_1); addr += 4)
    {
        if (*(uint32_t *)addr != addr)
        {
            flashError = true;
            PRINTF("Address %x program failed!\r\n", addr);
            PRINTF("Expected 0x%x, actual 0x%x.\r\n", addr, *(uint32_t *)addr);
        }
    }
    PRINTF("Flash block 1 check finished.\n");
    if (!flashError)
    {
        PRINTF("Flash Demo Pass!\n");
    }
    else
    {
        PRINTF("Flash Demo Failed!\n");
    }
    FLASH_DRV_Deinit(FLASH_INST);
    PRINTF("Flash Demo End.\n");
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
    UTILITY_PRINT_Init();
    FLASH_DRV_Init(0, &flash_config0, &flash_config0_State);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
