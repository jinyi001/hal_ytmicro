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
#define FLASH_INST            (0U)
#define TEST_FLASH_ADDR       (0x42000U)
#define TEST_FLASH_LEN        (0x2000U)
#define CUS_NVR_TEST          (1U) /* 1: test custom NVR, 0: not test custom NVR */
#define CUS_NVR_TEST_ADDR     (0x10000400U)
#define CUS_NVR_TEST_LEN      (0x200U) /* 512 bytes */
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
        flashError = true;
        PRINTF("operation error!\r\n");
    }
    else
    {
        flashError = true;
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
    uint32_t data[TEST_FLASH_LEN / 4];
    /* USER CODE END 1 */ 
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Flash Async Demo!\r\n");

    /* PFlash0 is in Block0; PFlash1 is in Block1 */
    PRINTF("Asynchronous mode can only be used for data where operations and programs are on different blocks!\r\n");
    PRINTF("So can only operate the PFlash1, if the program only exists on the PFlash0.\r\n");

    PRINTF("Checking PFlash1 from 0x%x to 0x%x!\r\n", TEST_FLASH_ADDR, TEST_FLASH_ADDR + TEST_FLASH_LEN);
    status |= FLASH_DRV_EraseSector(FLASH_INST, TEST_FLASH_ADDR, TEST_FLASH_LEN);

    /* Write 8K from 0x42000 to 0x44000 */
    for (i = 0; i < TEST_FLASH_LEN / 4; i++)
    {
        data[i] = TEST_FLASH_ADDR + 4 * i;
    }

    while (FLASH_DRV_GetBusyStatus(FLASH_INST))
        ;
    status |= FLASH_DRV_Program(FLASH_INST, TEST_FLASH_ADDR, TEST_FLASH_LEN, data);
    /* Wait for the program to complete, because check data would access program address */
    while (FLASH_DRV_GetBusyStatus(FLASH_INST))
        ;

    for (addr = TEST_FLASH_ADDR; addr < TEST_FLASH_ADDR + TEST_FLASH_LEN; addr += 4)
    {
        if (*(uint32_t *)addr != addr)
        {
            flashError = true;
            PRINTF("Address 0x%x program failed!\r\n", addr);
            PRINTF("Expected 0x%x, actual 0x%x.\r\n", addr, *(uint32_t *)addr);
        }
    }

#if CUS_NVR_TEST
    PRINTF("Checking CUS NVR from 0x%x to 0x%x!\r\n",
           CUS_NVR_TEST_ADDR,
           CUS_NVR_TEST_ADDR + CUS_NVR_TEST_LEN);

    /* Unlock NVR for customer access */
    EFM->CUS_KEY = 0x4dff32U;

    /* CUS_NVR is in block 1, So it could be accessed asynchronously when app runs in block 0 */
    /* Erase the CUS NVR sector (single sector, 1KB) */
    status |= FLASH_DRV_EraseNVR(FLASH_INST, CUS_NVR_TEST_ADDR);
    /* Wait for erase to complete */
    while (FLASH_DRV_GetBusyStatus(FLASH_INST))
        ;

    /* Verify erase: check all bytes are 0xFF */
    status |= FLASH_DRV_ReadNVR(FLASH_INST, CUS_NVR_TEST_ADDR, CUS_NVR_TEST_LEN, data);
    for (i = 0; i < CUS_NVR_TEST_LEN / 4; i++)
    {
        if (data[i] != 0xFFFFFFFFU)
        {
            flashError = true;
            PRINTF("CUS NVR erase failed at offset 0x%x! Expected 0xFFFFFFFF, actual 0x%x.\r\n",
                   CUS_NVR_TEST_ADDR + 4 * i, data[i]);
        }
    }

    /* Prepare data: fill with expected values */
    for (i = 0; i < CUS_NVR_TEST_LEN / 4; i++)
    {
        data[i] = CUS_NVR_TEST_ADDR + 4 * i;
    }

    status |= FLASH_DRV_ProgramNVR(FLASH_INST, CUS_NVR_TEST_ADDR, CUS_NVR_TEST_LEN, data);
    /* Wait for program to complete before verifying */
    while (FLASH_DRV_GetBusyStatus(FLASH_INST))
        ;

    /* Verify CUS NVR using FLASH_DRV_ReadNVR */
    status |= FLASH_DRV_ReadNVR(FLASH_INST, CUS_NVR_TEST_ADDR, CUS_NVR_TEST_LEN, data);
    for (i = 0; i < CUS_NVR_TEST_LEN / 4; i++)
    {
        if (data[i] != CUS_NVR_TEST_ADDR + 4 * i)
        {
            flashError = true;
            PRINTF("CUS NVR Address 0x%x program failed!\r\n", CUS_NVR_TEST_ADDR + 4 * i);
            PRINTF("Expected 0x%x, actual 0x%x.\r\n", CUS_NVR_TEST_ADDR + 4 * i, data[i]);
        }
    }

    /* Lock NVR */
    EFM->CUS_KEY = 0x0U;

    PRINTF("CUS NVR check finished.\n");
#endif /* CUS_NVR_TEST */

    lpTMR_DRV_StopCounter(0);
    PRINTF("Flash check finished.\n");
    if (!flashError)
    {
        PRINTF("Flash Async Demo Pass!\n");
    }
    else
    {
        PRINTF("Flash Async Demo Failed!\n");
    }
    FLASH_DRV_Deinit(FLASH_INST);
    PRINTF("Flash Async Demo End.\n");
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
    CLOCK_SYS_Init(g_clockManConfigsArr,CLOCK_MANAGER_CONFIG_CNT,g_clockManCallbacksArr,CLOCK_MANAGER_CALLBACK_CNT);
    if(STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX,CLOCK_MANAGER_POLICY_AGREEMENT))
    {
        /* USER CODE BEGIN ERROR_HANDLER 1 */
        SystemSoftwareReset();
        /* USER CODE END ERROR_HANDLER 1 */
    }
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0,g_pin_mux_InitConfigArr0);
    UTILITY_PRINT_Init();
    FLASH_DRV_Init(0,&flash_config0,&flash_config0_State);
    lpTMR_DRV_Init(0,&LPTMR_Config,false);
    INT_SYS_ConfigInit();
    lpTMR_DRV_StartCounter(0);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
