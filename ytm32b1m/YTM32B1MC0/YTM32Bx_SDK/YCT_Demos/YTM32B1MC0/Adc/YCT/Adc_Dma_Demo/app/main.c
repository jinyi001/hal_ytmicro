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
#include "printf.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PRINTF_UART (3) /* UART instance for PRINTF */
#define ADC_INST    (0) /* ADC instance */
#define SEQ_LEN     (8) /* channel number */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint16_t g_adc_result[8];
dma_loop_transfer_config_t adcLoopTransferConfig = { .triggerLoopIterationCount = 10, /*! Total ADC trigger count */
                                                     .dstOffsetEnable = true,
                                                     .triggerLoopOffset = -(SEQ_LEN * 2) };
const dma_transfer_config_t adcTransferConfig = { .srcAddr = (uint32_t)&ADC0->FIFO,
                                                  .destAddr = (uint32_t)&g_adc_result[0U],
                                                  .destOffset = 0x02U,
                                                  .srcOffset = 0x00U,
                                                  .srcTransferSize = DMA_TRANSFER_SIZE_2B,
                                                  .destTransferSize = DMA_TRANSFER_SIZE_2B,
                                                  .srcModulo = DMA_MODULO_OFF,
                                                  .destModulo = DMA_MODULO_OFF,
                                                  .transferLoopByteCount = (SEQ_LEN * 2),
                                                  .loopTransferConfig = &adcLoopTransferConfig,
                                                  .destLastAddrAdjust = -(SEQ_LEN * 2),
                                                  .interruptEnable = false };
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
    uint8_t i = 10;
    status_t status = STATUS_SUCCESS;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Hello ADC DMA!\r\n");

    /* Initiate DMA Loop Transfer */
    DMA_DRV_ConfigLoopTransfer(0U, &adcTransferConfig);
    DMA_DRV_StartChannel(0U);

    while (i--)
    {
        ADC_DRV_Start(ADC_INST);
        OSIF_TimeDelay(500);
        PRINTF("=======\r\n");
        for (uint8_t j = 0; j < SEQ_LEN; j++)
        {
            PRINTF("ADC CH %d=%d\r\n", j, g_adc_result[j]);
        }
    }
    PRINTF("====ADC DMA DEMO END====");
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
    if(STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX,CLOCK_MANAGER_POLICY_AGREEMENT))
    {
        /* USER CODE BEGIN ERROR_HANDLER 1 */
        SystemSoftwareReset();
        /* USER CODE END ERROR_HANDLER 1 */
    }
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    UTILITY_PRINT_Init();
    ADC_DRV_ConfigConverter(0, &adc_config0);
    DMA_DRV_Init(&dmaState, &dmaController_InitConfig, dmaChnState, dmaChnConfigArray, NUM_OF_CONFIGURED_DMA_CHANNEL);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
