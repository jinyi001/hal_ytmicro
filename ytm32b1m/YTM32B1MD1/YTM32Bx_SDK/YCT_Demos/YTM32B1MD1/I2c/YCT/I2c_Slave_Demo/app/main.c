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
#define PRINTF_UART (2)
#define I2C_INST    (0)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define BUFF_SIZE 8
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t slaveTxBuffer[BUFF_SIZE];
uint8_t slaveRxBuffer[BUFF_SIZE];
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
    /* I2c slave receives data */
    I2C_DRV_SlaveReceiveData(I2C_INST, slaveRxBuffer, BUFF_SIZE);
    while (STATUS_SUCCESS != I2C_DRV_SlaveGetTransferStatus(I2C_INST, NULL))
        ;

    /* I2C slave send data */
    for (uint8_t i = 0; i < BUFF_SIZE; i++)
    {
        slaveTxBuffer[i] = i;
    }
    I2C_DRV_SlaveSendData(I2C_INST, slaveTxBuffer, BUFF_SIZE);
    while (STATUS_SUCCESS != I2C_DRV_SlaveGetTransferStatus(I2C_INST, NULL))
        ;

    for (uint8_t i = 0; i < BUFF_SIZE; i++)
    {
        PRINTF("The value of RxBuffer member %d is %x\n", i, slaveRxBuffer[i]);
    }

    I2C_DRV_SlaveDeinit(I2C_INST);
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
    I2C_DRV_SlaveInit(0, &I2C_SlaveConfig0, &I2C_SlaveConfig0_State);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
