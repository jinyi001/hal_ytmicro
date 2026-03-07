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
#include "string.h"
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
uint8_t g_uartRxBuffer[UART_RX_LENGTH] = { 0 };
volatile bool g_idleErrorFlag = false;
volatile bool g_Flag = false;
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
    uart_state_t *stat;

    (void)userData;
    stat = (uart_state_t *)uartState;
    if (UART_EVENT_END_TRANSFER == event)
    {
        /* Receive complete */
        g_Flag = true;
    }
    else if (UART_EVENT_ERROR == event)
    {
        /* error happened */
        if (stat->receiveStatus == STATUS_UART_IDLE_ERROR)
        {
            g_idleErrorFlag = true;
        }
    }
}

void uart_TX_Callback(void *uartState, uart_event_t event, void *userData)
{
    (void)userData;
    (void)uartState;
    if (UART_EVENT_END_TRANSFER == event)
    {
        g_sendCompleteFlag = true;
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
    uint8_t uartTxLength = UART_RX_LENGTH - 1;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("UART Idle Interrupt Demo!\r\n");
    PRINTF("Connnect PTB0 with PTB1 to test UART0 in loopback mode!\r\n");
    /* Receive data using interrupt */
    UART_DRV_ReceiveData(INST_UART, g_uartRxBuffer, UART_RX_LENGTH);
    /* Send data using interrupt */
    UART_DRV_SendData(INST_UART, g_uartTxBuffer, uartTxLength);
    PRINTF("Please check if TX and RX callback function entered.\n");
    OSIF_TimeDelay(200);

    while (1)
    {
        while (!g_sendCompleteFlag)
            ;
        g_sendCompleteFlag = false;
        /* all data sent*/
        PRINTF("Data sent.\n");

        /* UART wants to receive UART_RX_LENGTH data, but only receives uartTxLength data, so it will enter the idle error interrupt. */
        while (!g_idleErrorFlag)
            ;
        g_idleErrorFlag = false;
        PRINTF("Data Received: ");
        for (uint32_t i = 0; i < (UART_RX_LENGTH - uart_config1_State.rxSize); i++)
        {
            PRINTF("%d ", g_uartRxBuffer[i]);
        }
        PRINTF("\n");
        memset(g_uartRxBuffer, 0x00, UART_RX_LENGTH);
        if (uartTxLength == 1)
        {
            break;
        }
        /* Abort receiving data */
        UART_DRV_AbortReceivingData(INST_UART);
        /* Receive data using interrupt */
        UART_DRV_ReceiveData(INST_UART, g_uartRxBuffer, UART_RX_LENGTH);
        uartTxLength--;
        /* Send data using interrupt */
        UART_DRV_SendData(INST_UART, g_uartTxBuffer, uartTxLength);
    }

    PRINTF("UART Idle Interrupt Demo End!\r\n");
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
