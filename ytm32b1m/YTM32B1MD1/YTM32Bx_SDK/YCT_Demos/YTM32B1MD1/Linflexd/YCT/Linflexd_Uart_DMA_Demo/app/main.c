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
#define TIMEOUT         (10)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t recvData[UART_TX_RX_SIZE];
uint8_t sendData[UART_TX_RX_SIZE] = { 30, 31, 32, 33, 34, 35, 36, 37 };
volatile bool rxComplete = false;
volatile bool txComplete = false;
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
}

/* Uart tx callback function */
void Uart_TxCallback(void *LINFLexDState, uart_event_t event, void *userData)
{
    (void)LINFLexDState;
    (void)userData;
    if (event == UART_EVENT_TX_EMPTY)
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
    uint8_t timer = 0;
    uint32_t cnt = 10;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\r\n", __DATE__, __TIME__);
    PRINTF("Linflexd Uart DMA Demo!\r\n");
    PRINTF("Please short PTC2 and PTC3 for test.\n");

    LINFlexD_UART_DRV_InstallRxCallback(INST_UART, Uart_RxCallback, NULL);
    LINFlexD_UART_DRV_InstallTxCallback(INST_UART, Uart_TxCallback, NULL);

    while (cnt--)
    {
        /* Receive data */
        status |= LINFlexD_UART_DRV_ReceiveData(INST_UART, recvData, UART_TX_RX_SIZE);
        /* Send data */
        status |= LINFlexD_UART_DRV_SendData(INST_UART, sendData, UART_TX_RX_SIZE);

        /* Wait for transmit to complete */
        while (!rxComplete || !txComplete)
        {
            OSIF_TimeDelay(1);
            if (timer++ > TIMEOUT)
            {
                uartError = true;
                PRINTF("Error: Please check TX and RX if connected\n");
            }
        }

        PRINTF("UART send: ");
        for (uint32_t i = 0; i < UART_TX_RX_SIZE; i++)
        {
            PRINTF("%x, ", sendData[i]);
        }
        PRINTF("\n");

        PRINTF("UART recv: ");
        for (uint32_t i = 0; i < UART_TX_RX_SIZE; i++)
        {
            PRINTF("%x, ", recvData[i]);
            if (sendData[i] != recvData[i])
            {
                uartError = true;
                PRINTF("UART receive Error. Expected %x, actual %x\n", sendData[i], recvData[i]);
            }
        }
        PRINTF("\n");

        timer = 0;
        txComplete = false;
        rxComplete = false;
        sendData[0]++;
    }
    if (!uartError)
    {
        PRINTF("LINFlexD UART DMA Demo Pass\n");
    }
    else
    {
        PRINTF("LINFlexD UART DMA Demo Fail\n");
    }
    LINFlexD_UART_DRV_Deinit(INST_UART);
    PRINTF("Linflexd UART DMA Demo End.\n");
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
    DMA_DRV_Init(&dmaState, &dmaController_InitConfig, dmaChnState, dmaChnConfigArray, NUM_OF_CONFIGURED_DMA_CHANNEL);
    LINFlexD_UART_DRV_Init(0, &linflexd_uart_config0_State, &linflexd_uart_config0);
    UTILITY_PRINT_Init();
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
