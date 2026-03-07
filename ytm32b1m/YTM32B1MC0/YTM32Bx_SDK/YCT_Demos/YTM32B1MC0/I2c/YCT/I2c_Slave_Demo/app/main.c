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
#define INST_I2C  (0U)
#define BUFF_SIZE (8U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* Slave TX and RX buffers definition */
uint8_t slaveTxBuffer[BUFF_SIZE] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
uint8_t slaveRxBuffer[BUFF_SIZE];
volatile bool g_i2cCompleteFlag = false;
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void I2C_SlaveCallback(i2c_slave_event_t slaveEvent, void *userData)
{
    /* Get the instance number from userData */
    uint32_t instance = INST_I2C;
    (void)userData;
    /* Depending on the event received, set the buffers or abort the transfer */
    switch (slaveEvent)
    {
        case I2C_SLAVE_EVENT_RX_REQ:
            /*
             * If the bus master requests data, then set the destination RX buffer
             * and accepted transfer size
             */
            I2C_DRV_SlaveSetRxBuffer(instance, slaveRxBuffer, BUFF_SIZE);
            break;
        case I2C_SLAVE_EVENT_TX_REQ:
            /*
             * If the bus master sends data, then set the source TX buffer
             * and accepted transfer size
             */
            I2C_DRV_SlaveSetTxBuffer(instance, slaveTxBuffer, BUFF_SIZE);
            break;
        case I2C_SLAVE_EVENT_TX_EMPTY:
            /*
             * If the TX buffer is empty. Because the example does not handle 
             * this case there is no action taken.
             */
        case I2C_SLAVE_EVENT_RX_FULL:
            /*
             * If the RX buffer is full, check the slave receive buffer is correct
             */
            break;
        case I2C_SLAVE_EVENT_STOP:
            /*
             * This case is used when a stop condition is on the bus. Because
             * the example does not handle this case there is no action taken.
             */
            g_i2cCompleteFlag = true;
            break;
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
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("I2C Slave DEMO!\r\n");
    /* All data and response will be processed under I2C interrupt. */
    while (!g_i2cCompleteFlag)
        ;
    PRINTF("I2C Slave DEMO End!\r\n");
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
    if(STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX,CLOCK_MANAGER_POLICY_AGREEMENT))
    {
        /* USER CODE BEGIN ERROR_HANDLER 1 */
        SystemSoftwareReset();
        /* USER CODE END ERROR_HANDLER 1 */
    }
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    UTILITY_PRINT_Init();
    I2C_DRV_SlaveInit(INST_I2C, &I2C_SlaveConfig0, &I2C_SlaveConfig0_State);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
