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
#define INST_I2C (0U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t g_at24c02_data[8];
volatile bool iicError = false;
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
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\r\n", __DATE__, __TIME__);
    PRINTF("I2c Master demo!\r\n");

    for (int i = 0; i < 8; i++)
    {
        /* initialize data */
        g_at24c02_data[i] = i;
    }
    /* write data to EEPROM.
       The first data represents the first address written. */
    PRINTF("Write data to EEPROM.\n");
    for (int i = 0; i < 8; i++)
    {
        PRINTF("%d ", g_at24c02_data[i]);
    }
    PRINTF("\r\n");
    status |= I2C_DRV_MasterSendDataBlocking(INST_I2C, g_at24c02_data, 8, true, 1000);
    /* wait for EEPROM to finish writing   */
    OSIF_TimeDelay(50);

    for (int i = 0; i < 8; i++)
    {
        /* initialize data */
        g_at24c02_data[i] = 0;
    }
    /* Before reading eeprom, 
       one byte of data needs to be sent to specify the first address to be read. */
    PRINTF("Read data from EEPROM.\n");
    status |= I2C_DRV_MasterSendDataBlocking(INST_I2C, g_at24c02_data, 1, false, 1000);
    /* Read data from EEPROM */
    status |= I2C_DRV_MasterReceiveDataBlocking(INST_I2C, g_at24c02_data + 1, 7, true, 1000);
    if (status != STATUS_SUCCESS)
    {
        PRINTF("Error happens with code: %x\n", status);
        while (1)
            ;
    }
    for (int i = 0; i < 8; i++)
    {
        PRINTF("%d ", g_at24c02_data[i]);
        if (i != g_at24c02_data[i])
        {
            iicError = true;
            PRINTF("FAIL: Data not match with addr: %d\n", i);
        }
    }
    PRINTF("\r\n");
    if (!iicError)
    {
        PRINTF("I2C Master Demo Pass!\n");
    }
    else
    {
        PRINTF("I2C Master Demo Fail!\n");
    }
    I2C_DRV_MasterDeinit(0);
    PRINTF("I2C Master Demo End!\n");
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
    I2C_DRV_MasterInit(0, &I2C_MasterConfig0, &I2C_MasterConfig0_State);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
