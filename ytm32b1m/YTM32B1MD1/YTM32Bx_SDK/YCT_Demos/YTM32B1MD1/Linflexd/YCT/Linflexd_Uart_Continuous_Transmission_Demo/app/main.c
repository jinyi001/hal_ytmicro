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
#define UART_TX_RX_SIZE (8)
#define INST_UART       (0)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t recvDataBuff[UART_TX_RX_SIZE] = { 30, 31, 32, 33, 34, 35, 36, 37 };
uint8_t transData = 0;
uint8_t recvData = 0;
volatile bool rxComplete = false;
volatile bool txComplete = false;
volatile uint8_t uartRxCnt = 0;
volatile uint8_t uartTxCnt = 0;
volatile bool uartError = false;
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Uart rx callback function */
void Uart_RxCallback(void *LINFLexDState, uart_event_t event, void *userData)
{
    (void)userData;
    uint32_t rxDataAddr = 0;
    linflexd_uart_state_t *state = (linflexd_uart_state_t *)LINFLexDState;

    if (event == UART_EVENT_END_TRANSFER)
    {
        rxComplete = true;
    }
    else if (event == UART_EVENT_RX_FULL)
    {
        /* Note: Only applicable to 7bit or 8bit data characters.
            Get the last receiving address. */
        rxDataAddr = ((uint32_t)&state->rxBuff[0]) - 1;
        recvDataBuff[uartRxCnt] = *(uint8_t *)rxDataAddr;
        uartRxCnt++;

        if (uartRxCnt < UART_TX_RX_SIZE)
        {
            /* Reconfigure the buff and size of the received data */
            LINFlexD_UART_DRV_SetRxBuffer(INST_UART, (uint8_t *)rxDataAddr, 1);
        }
        else
        {
            uartRxCnt = 0;
        }
    }
}

/* Uart tx callback function */
void Uart_TxCallback(void *LINFLexDState, uart_event_t event, void *userData)
{
    (void)userData;
    uint32_t txDataAddr = 0;
    linflexd_uart_state_t *state = (linflexd_uart_state_t *)LINFLexDState;

    if (event == UART_EVENT_END_TRANSFER)
    {
        txComplete = true;
    }
    else if (event == UART_EVENT_TX_EMPTY)
    {
        /* Note: Only applicable to 7bit or 8bit data characters.
            Get the last sending address. */
        txDataAddr = ((uint32_t)&state->txBuff[0]) - 1;

        if (uartTxCnt < UART_TX_RX_SIZE)
        {
            *(uint8_t *)txDataAddr = recvDataBuff[uartTxCnt];
            /* Reconfigure the buff and size of sent data */
            LINFlexD_UART_DRV_SetTxBuffer(INST_UART, (uint8_t *)txDataAddr, 1);
            uartTxCnt++;
        }
        else
        {
            uartTxCnt = 0;
        }
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
    status_t status = STATUS_SUCCESS;
    uint32_t cnt = 10;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\r\n", __DATE__, __TIME__);
    PRINTF("Linflexd Uart Continuous Transmission Demo!\r\n");
    PRINTF("Please short PTC2 and PTC3 for test.\n");

    LINFlexD_UART_DRV_InstallRxCallback(INST_UART, Uart_RxCallback, NULL);
    LINFlexD_UART_DRV_InstallTxCallback(INST_UART, Uart_TxCallback, NULL);

    while (cnt--)
    {
        /* Receive data using interrupt.
            The remaining 7 bytes of data are processed in the callback function. */
        status |= LINFlexD_UART_DRV_ReceiveData(INST_UART, &recvData, 1);
        /* Send data */
        transData = recvDataBuff[uartTxCnt];
        uartTxCnt++;
        /* The remaining 7 bytes of data are processed in the callback function. */
        status |= LINFlexD_UART_DRV_SendData(INST_UART, &transData, 1);
        OSIF_TimeDelay(100);
        PRINTF("Check if TX and RX callback function entered.\n");
        if (!txComplete || !rxComplete)
        {
            uartError = true;
            PRINTF("Error: Please confirm whether TX and RX connected!\n");
        }

        txComplete = false;
        rxComplete = false;
    }
    if (!uartError)
    {
        PRINTF("Linflexd Uart Continuous Transmission Demo Pass.\n");
    }
    else
    {
        PRINTF("Linflexd Uart Continuous Transmission Demo Fail.\n");
    }
    LINFlexD_UART_DRV_Deinit(INST_UART);
    PRINTF("Linflexd Uart Continuous Transmission Demo End.\n");
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
    LINFlexD_UART_DRV_Init(INST_UART, &linflexd_uart_config0_State, &linflexd_uart_config0);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
