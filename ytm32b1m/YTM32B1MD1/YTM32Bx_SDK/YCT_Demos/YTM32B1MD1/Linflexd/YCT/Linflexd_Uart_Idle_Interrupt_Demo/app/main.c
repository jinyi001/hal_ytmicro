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
#define UART_RX_SIZE (256)
#define UART_TX_SIZE (8)
#define INST_UART    (0)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t sendData[UART_TX_SIZE] = { 30, 31, 32, 33, 34, 35, 36, 37 };
uint8_t recvData[UART_RX_SIZE];
uint32_t rxRemainNum = 0;
volatile bool rxComplete = false;
volatile bool txComplete = false;
volatile bool timeoutFlag = false;
linflexd_uart_idle_timeout_config_t LINFlexDUartIdleTimeoutConfig = {
    .enableIdleTimeout = true,
    .idleTimeoutCount = 100,
};
/* self test flag */
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
    (void)LINFLexDState;
    (void)userData;
    if (event == UART_EVENT_END_TRANSFER)
    {
        rxComplete = true;
    }
    else if (event == UART_EVENT_TIMEOUT)
    {
        LINFlexD_UART_DRV_GetReceiveStatus(INST_UART, &rxRemainNum);
        timeoutFlag = true;
    }
}

/* Uart tx callback function */
void Uart_TxCallback(void *LINFLexDState, uart_event_t event, void *userData)
{
    (void)LINFLexDState;
    (void)userData;
    if (event == UART_EVENT_END_TRANSFER)
    {
        txComplete = true;
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
    PRINTF("Linflexd Uart Idle Interrupt Demo!\r\n");
    PRINTF("Please short PTC2 and PTC3 for test.\n");

    LINFlexD_UART_DRV_ConfigureTimeoutCounter(INST_UART, &LINFlexDUartIdleTimeoutConfig);
    LINFlexD_UART_DRV_InstallRxCallback(INST_UART, Uart_RxCallback, NULL);
    LINFlexD_UART_DRV_InstallTxCallback(INST_UART, Uart_TxCallback, NULL);

    while (cnt--)
    {
        /* UART receive 256bytes, but only 8 bytes received, UART will timeout and stop receive */
        status |= LINFlexD_UART_DRV_ReceiveData(INST_UART, recvData, UART_RX_SIZE);
        status |= LINFlexD_UART_DRV_SendData(INST_UART, sendData, UART_TX_SIZE);
        /* Wait for reception to complete */
        while ((!rxComplete) && (!timeoutFlag))
            ;

        /* If a timeout event occurs, indicating the expected number of data bytes was not received, 
            the UART enters an idle state, terminating the current communication. */
        if (timeoutFlag)
        {
            PRINTF("Number of remaining received bytes is %d\r\n", rxRemainNum);
            /* Expected remain number is 248 */
            if (rxRemainNum != (UART_RX_SIZE - UART_TX_SIZE))
            {
                uartError = true;
                PRINTF("Error: Check UART TX and RX if connected.\n");
            }
            PRINTF("UART receive:\n");
            for (uint8_t i = 0; i < UART_TX_SIZE; i++)
            {
                PRINTF("%x, ", recvData[i]);
                if (sendData[i] != recvData[i])
                {
                    uartError = true;
                    PRINTF("UART receive mismatch. Expected %x, actual %x\n", sendData[i], recvData[i]);
                }
                sendData[i]++;
            }
            PRINTF("\n");
        }

        if (rxComplete)
        {
            uartError = true;
            PRINTF("Error: TX size need less than RX size.\n");
            LINFlexD_UART_DRV_GetReceiveStatus(INST_UART, &rxRemainNum);
        }

        txComplete = false;
        rxComplete = false;
        timeoutFlag = false;
    }

    if (!uartError)
    {
        PRINTF("LINFlexD UART IDLE Interrupt Demo Pass.\n");
    }
    else
    {
        PRINTF("LINFlexD UART IDLE Interrupt Demo Fail.\n");
    }
    LINFlexD_UART_DRV_Deinit(INST_UART);
    PRINTF("Linflexd UART IDLE Interrupt Demo End.\n");
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
    LINFlexD_UART_DRV_Init(0, &linflexd_uart_config0_State, &linflexd_uart_config0);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
