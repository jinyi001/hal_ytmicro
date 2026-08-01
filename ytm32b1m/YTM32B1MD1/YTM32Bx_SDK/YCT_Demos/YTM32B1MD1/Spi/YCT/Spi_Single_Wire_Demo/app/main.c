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
#define SPI_MASTER_INST  (1)
#define SPI_SLAVE_INST   (2)
#define SPI_TRANS_LENGTH (8)

#define TIMEOUT (10)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t spiTxBuff[SPI_TRANS_LENGTH] = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7 };
uint8_t spiRxBuff[SPI_TRANS_LENGTH];

volatile bool spiError = false;
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
    uint8_t timer = 0;
    uint32_t cnt = 10;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("SPI Single Wire Demo!\r\n");
    PRINTF("Demo shows SPI1 as master, and SPI2 as slave. You can connect \n");
    PRINTF("\tPTD3--PTC14\n");
    PRINTF("\tPTE9--PTE15\n");
    PRINTF("\tPTD2--PTE16\n");
    PRINTF("\tPTE3--PTA8\n");
    PRINTF("to enjoy demo.\n");

    /* Configure to Single Wire Mode */
    SPI1->CTRL &= ~SPI_CTRL_PINCFG_MASK;
    SPI1->CTRL |= SPI_CTRL_PINCFG(0x02U) | SPI_CTRL_DHZEN_MASK;
    SPI2->CTRL &= ~SPI_CTRL_PINCFG_MASK;
    SPI2->CTRL |= SPI_CTRL_PINCFG(0x01U) | SPI_CTRL_DHZEN_MASK;

    while (cnt--)
    {
        /* 
            The third parameter of function SPI_DRV_SlaveTransfer is NULL, it will disable write received data to RX FIFO;
            The second parameter of function SPI_DRV_MasterTransfer is NULL, it will start a new transfer ,and SPI data out 
            will be tristates.
        */
        SPI_DRV_SlaveTransfer(SPI_SLAVE_INST, spiTxBuff, NULL, SPI_TRANS_LENGTH);
        SPI_DRV_MasterTransfer(SPI_MASTER_INST, NULL, spiRxBuff, SPI_TRANS_LENGTH);

        /* Wait data transfer finished */
        while ((STATUS_SUCCESS != SPI_DRV_MasterGetTransferStatus(SPI_MASTER_INST, NULL)) ||
               STATUS_SUCCESS != SPI_DRV_SlaveGetTransferStatus(SPI_SLAVE_INST, NULL))
        {
            OSIF_TimeDelay(1);
            if (timer++ > TIMEOUT)
            {
                spiError = true;
                PRINTF("Error: Please check pin connected\n");
            }
        }
        timer = 0;

        PRINTF("SPI Master receive: ");
        for (uint8_t idx = 0; idx < SPI_TRANS_LENGTH; idx++)
        {
            PRINTF("%x ", spiRxBuff[idx]);
            if (spiTxBuff[idx] != spiRxBuff[idx])
            {
                spiError = true;
                PRINTF("SPI master receive Error. Expected %x, actual %x\n", spiTxBuff[idx], spiRxBuff[idx]);
            }
            spiTxBuff[idx]++;
        }
        PRINTF("\n");
    }
    if (!spiError)
    {
        PRINTF("\nSPI Single Wire Demo Pass\n");
    }
    else
    {
        PRINTF("\nSPI Single Wire Demo Fail\n");
    }
    SPI_DRV_MasterDeinit(SPI_MASTER_INST);
    SPI_DRV_SlaveDeinit(SPI_SLAVE_INST);
    PRINTF("SPI Single Wire Demo End.\n");
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
    SPI_DRV_MasterInit(SPI_MASTER_INST, &spi_MasterConfig0_State, &spi_MasterConfig0);
    SPI_DRV_SlaveInit(SPI_SLAVE_INST, &spi_SlaveConfig0_State, &spi_SlaveConfig0);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
