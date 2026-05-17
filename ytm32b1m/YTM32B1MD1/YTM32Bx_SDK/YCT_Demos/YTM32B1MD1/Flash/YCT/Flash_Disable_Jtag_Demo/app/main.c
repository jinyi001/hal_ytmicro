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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define EFM_CUS_NVR_ADDR        (0x10000400U)
#define EFM_DEBUGGER_TAG_OFFSET (0x08U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint32_t g_sw2_pressed = 0;
volatile uint32_t g_sw3_pressed = 0;
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void GPIOB_IRQHandler(void)
{
    if (PINS_DRV_GetPortIntFlag(GPIOB) & 0x4)
    {
        PINS_DRV_ClearPinIntFlagCmd(GPIOB, 2);
        g_sw2_pressed = 1;
    }
    if (PINS_DRV_GetPortIntFlag(GPIOB) & 0x8)
    {
        /* Rev.B, SW3 is on PTB3 */
        PINS_DRV_ClearPinIntFlagCmd(GPIOB, 3);
        g_sw3_pressed = 1;
    }
}

void GPIOC_IRQHandler(void)
{
    if (PINS_DRV_GetPortIntFlag(GPIOC) & 0x2)
    {
        /* Rev.B, SW3 is on PTC1 */
        PINS_DRV_ClearPinIntFlagCmd(GPIOC, 1);
        g_sw3_pressed = 1;
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
    status_t status;
    uint32_t cus_nvr[FEATURE_EFM_NVR_ARRAY_SECTOR_SIZE >> 2] = { 0 };
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Flash Disable Jtag Demo!\r\n");
    NVIC_EnableIRQ(GPIOB_IRQn);
    NVIC_EnableIRQ(GPIOC_IRQn);
    /* Set the CUS_KEY register to unlock the CUS_NVR.
       The default value is 0x4DFF32. It can be changed to any other value. */
    EFM->CUS_KEY = 0x4DFF32;
    status = FLASH_DRV_ReadNVR(0, EFM_CUS_NVR_ADDR, FEATURE_EFM_NVR_ARRAY_SECTOR_SIZE, cus_nvr);
    if (status != STATUS_SUCCESS)
    {
        PRINTF("Read CUS_NVR Failed!\r\n");
    }

    if (EFM->CTRL_OVRD & EFM_CTRL_OVRD_DBG_DISABLE_MASK)
    {
        PRINTF("SWD is disabled, please press SW3 to enable it!\r\n");
    }
    else
    {
        PRINTF("SWD is enabled, please press SW2 to disable it!\r\n");
    }
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */
        /* USER CODE BEGIN 3 */

        if (g_sw2_pressed)
        {
            g_sw2_pressed = 0;
            PRINTF("=> SW2 pressed!\r\n");
            PRINTF("Start to disable Jtag/SWD!\r\n");

            cus_nvr[EFM_DEBUGGER_TAG_OFFSET >> 2] = 0x5A5A5A5A;
            cus_nvr[(EFM_DEBUGGER_TAG_OFFSET >> 2) + 1] = 0x5A5A5A5A;
            status = FLASH_DRV_EraseNVR(0, EFM_CUS_NVR_ADDR);
            status |= FLASH_DRV_ProgramNVR(0, EFM_CUS_NVR_ADDR, FEATURE_EFM_NVR_ARRAY_SECTOR_SIZE, cus_nvr);
            if (status == STATUS_SUCCESS)
            {
                PRINTF("Disable Jtag/SWD successfully!\r\n");
                PRINTF("Will reset the device to take effect!\r\n");
                PRINTF("-------------------------------------------\r\n");
                SystemSoftwareReset();
            }
        }

        if (g_sw3_pressed)
        {
            g_sw3_pressed = 0;
            PRINTF("=> SW3 pressed!\r\n");
            PRINTF("Start to enable Jtag/SWD!\r\n");

            cus_nvr[EFM_DEBUGGER_TAG_OFFSET >> 2] = 0xFFFFFFFF;
            cus_nvr[(EFM_DEBUGGER_TAG_OFFSET >> 2) + 1] = 0xFFFFFFFF;
            status = FLASH_DRV_EraseNVR(0, EFM_CUS_NVR_ADDR);
            status |= FLASH_DRV_ProgramNVR(0, EFM_CUS_NVR_ADDR, FEATURE_EFM_NVR_ARRAY_SECTOR_SIZE, cus_nvr);
            if (status == STATUS_SUCCESS)
            {
                PRINTF("Enable Jtag/SWD successfully!\r\n");
                PRINTF("Will reset the device to take effect!\r\n");
                PRINTF("-------------------------------------------\r\n");
                SystemSoftwareReset();
            }
        }
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
    FLASH_DRV_Init(0, &flash_config0, &flash_config0_State);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
