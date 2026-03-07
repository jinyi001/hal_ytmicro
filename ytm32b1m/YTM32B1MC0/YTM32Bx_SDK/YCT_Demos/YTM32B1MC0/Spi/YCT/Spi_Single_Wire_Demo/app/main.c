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
#define SPI_TRANS_LENGTH (8)
#define SPI_MASTER_INST  (2)
#define SPI_SLAVE_INST   (0)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t g_spi_tx_buff_master[SPI_TRANS_LENGTH] = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7 };
uint8_t g_spi_rx_buff_master[SPI_TRANS_LENGTH];

uint8_t g_spi_tx_buff_slave[SPI_TRANS_LENGTH] = { 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1 };
uint8_t g_spi_rx_buff_slave[SPI_TRANS_LENGTH];
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
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("SPI single wire transfer Demo\n");
    /* Configure to Single Wire Mode */
    SPI2->CTRL &= ~SPI_CTRL_PINCFG_MASK;
    SPI2->CTRL |= SPI_CTRL_PINCFG(0x02U) | SPI_CTRL_DHZEN_MASK;
    SPI0->CTRL &= ~SPI_CTRL_PINCFG_MASK;
    SPI0->CTRL |= SPI_CTRL_PINCFG(0x02U) | SPI_CTRL_DHZEN_MASK;

    /* Start data transfer, master send, slave receive */
    /* For slave, sendBuffer is NULL; For master, receiveBuffer is NULL */
    SPI_DRV_SlaveTransfer(SPI_SLAVE_INST, NULL, g_spi_rx_buff_slave, SPI_TRANS_LENGTH);
    SPI_DRV_MasterTransfer(SPI_MASTER_INST, g_spi_tx_buff_master, NULL, SPI_TRANS_LENGTH);

    /* Wait data transfer finished */
    while (STATUS_SUCCESS != SPI_DRV_MasterGetTransferStatus(SPI_MASTER_INST, NULL))
        ;

    PRINTF("Slave receive:");

    for (uint8_t idx = 0; idx < SPI_TRANS_LENGTH; idx++)
    {
        PRINTF("%x ", g_spi_rx_buff_slave[idx]);
    }

    /* Start data transfer, slave send, master receive */
    /* For slave, receiveBuffer is NULL; For master, sendBuffer is NULL */
    SPI_DRV_SlaveTransfer(SPI_SLAVE_INST, g_spi_tx_buff_slave, NULL, SPI_TRANS_LENGTH);
    SPI_DRV_MasterTransfer(SPI_MASTER_INST, NULL, g_spi_rx_buff_master, SPI_TRANS_LENGTH);

    /* Wait data transfer finished */
    while (STATUS_SUCCESS != SPI_DRV_MasterGetTransferStatus(SPI_MASTER_INST, NULL))
        ;

    PRINTF("\nMaster receive:");

    for (uint8_t idx = 0; idx < SPI_TRANS_LENGTH; idx++)
    {
        PRINTF("%x ", g_spi_rx_buff_master[idx]);
    }

    PRINTF("\nSPI single wire transfer complete\n");
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
    SPI_DRV_SlaveInit(0, &spi_SlaveConfig0_State, &spi_SlaveConfig0);
    SPI_DRV_MasterInit(2, &spi_MasterConfig0_State, &spi_MasterConfig0);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
