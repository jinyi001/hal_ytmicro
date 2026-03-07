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
#define INST_UART      (0)
#define UART_RX_LENGTH (10)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t g_uartTxBuffer[] = { 30, 31, 32, 33, 34, 35, 36, 37, 38, 39 };
uint16_t g_uartRxBuffer[UART_RX_LENGTH] = { 0 };
uint16_t g_uartRxData = 0;
uint16_t g_uartTxData = 0;
uint8_t g_uartRxCnt = 0;
uint8_t g_uartTxCnt = 0;
volatile bool g_receiveCompleteFlag = false;
volatile bool g_sendCompleteFlag = false;
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void uart_RX_Callback(void *uartState, uart_event_t event, void *userData)
{
    (void)userData;
    (void)uartState;
    uart_state_t *state = (uart_state_t *)uartState;
    uint32_t rxDataAddr = 0;
    switch (event)
    {
        case UART_EVENT_RX_FULL:
            /* Note: Only applicable to 7bit or 8bit data characters.
               Get the last receiving address. */
            rxDataAddr = ((uint32_t)&state->rxBuff[0]) - 1;
            g_uartRxBuffer[g_uartRxCnt] = *(uint8_t *)rxDataAddr;
            g_uartRxCnt++;

            if (g_uartRxCnt < UART_RX_LENGTH)
            {
                /* Reconfigure the buff and size of the received data */
                UART_DRV_SetRxBuffer(INST_UART, (uint8_t *)rxDataAddr, 1);
            }
            else
            {
                g_uartRxCnt = 0;
            }

            break;
        case UART_EVENT_END_TRANSFER:
            g_receiveCompleteFlag = true;
            break;
        default:
            break;
    }
}

void uart_TX_Callback(void *uartState, uart_event_t event, void *userData)
{
    (void)userData;
    (void)uartState;
    uart_state_t *state = (uart_state_t *)uartState;
    uint32_t txDataAddr = 0;

    switch (event)
    {
        case UART_EVENT_TX_EMPTY:
            /* Note: Only applicable to 7bit or 8bit data characters.
                Get the last sending address. */
            txDataAddr = ((uint32_t)&state->txBuff[0]) - 1;

            if (g_uartTxCnt < UART_RX_LENGTH)
            {
                *(uint16_t *)txDataAddr = g_uartTxBuffer[g_uartTxCnt];
                /* Reconfigure the buff and size of sent data */
                UART_DRV_SetTxBuffer(INST_UART, (uint8_t *)txDataAddr, 1);
                g_uartTxCnt++;
            }
            else
            {
                g_uartTxCnt = 0;
            }
            break;
        case UART_EVENT_END_TRANSFER:
            g_sendCompleteFlag = true;
            break;
        default:
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
    PRINTF("UART Continuous Transmission Demo!\r\n");
    PRINTF("Connnect PTB0 with PTB1 to test UART0 in loopback mode!\r\n");

    /* Receive data using interrupt */
    UART_DRV_ReceiveData(INST_UART, (uint8_t *)&g_uartRxData, 1);
    /* Send data using interrupt */
    g_uartTxData = g_uartTxBuffer[g_uartTxCnt];
    g_uartTxCnt++;
    UART_DRV_SendData(INST_UART, (uint8_t *)&g_uartTxData, 1);
    PRINTF("Please check if TX and RX callback function entered.\n");
    OSIF_TimeDelay(200);
    if (g_sendCompleteFlag && g_receiveCompleteFlag)
    {
        PRINTF("Succes : Uart continuous transmission demo end!\n");
    }
    else
    {
        PRINTF("Error : Please confirm whether PTB0 and PTB1 are connected!\n");
    }
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
    UART_DRV_Init(INST_UART, &uart_config1_State, &uart_config1);
    UART_DRV_InstallRxCallback(INST_UART, uart_RX_Callback, NULL);
    UART_DRV_InstallTxCallback(INST_UART, uart_TX_Callback, NULL);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
