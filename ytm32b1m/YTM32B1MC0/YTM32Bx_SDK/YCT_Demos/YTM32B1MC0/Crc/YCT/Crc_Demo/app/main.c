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
#define INST_CRC            (0)
/* Define CRC data size */
#define CRC_DATA_SIZE       (4U)
/* Define result CRC calculator with CRC-32 standard */
#define RESULT_CRC_32       (0x57738169U)
/* Define result CRC calculator with CCITT 32 bits standard */
#define RESULT_CRC_16_CCITT (0x4BDCU)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
const uint32_t CRC_data32[] = { 0x12345678, 0x56781234, 0x55AA55AA, 0xA5A5A5A5 };
const uint16_t CRC_data16[] = { 0x1234, 0x5678, 0x5A5A, 0xA5A5 };
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void crc32_test(void)
{
    uint32_t result = 0;
    PRINTF("CRC 32 TEST START.\n");
    CRC_DRV_Init(INST_CRC, &crc32Cfg0);
    /* Calculate CRC value for CRC_data with configuration of 32 bit wide result */
    CRC_DRV_WriteData32(INST_CRC, (const uint32_t *)CRC_data32, CRC_DATA_SIZE);
    result = CRC_DRV_GetCrcResult(INST_CRC);
    if (result != RESULT_CRC_32)
    {
        PRINTF("FAIL: CRC result not match!\n");
    }
    PRINTF("CRC TEST DONE.\n");
}

void crc16_test(void)
{
    uint16_t result = 0;
    PRINTF("CRC 16 TEST START.\n");
    CRC_DRV_Init(INST_CRC, &crc16Cfg0);
    /* Calculate CRC value for CRC_data with configuration of 16 bit wide result */
    CRC_DRV_WriteData16(INST_CRC, (const uint16_t *)CRC_data16, CRC_DATA_SIZE);
    result = CRC_DRV_GetCrcResult(INST_CRC);
    if (result != RESULT_CRC_16_CCITT)
    {
        PRINTF("FAIL: CRC result not match!\n");
    }
    PRINTF("CRC TEST DONE.\n");
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
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Hello CRC!\r\n");
    crc32_test();
    crc16_test();
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
    if(STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX,CLOCK_MANAGER_POLICY_AGREEMENT))
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
