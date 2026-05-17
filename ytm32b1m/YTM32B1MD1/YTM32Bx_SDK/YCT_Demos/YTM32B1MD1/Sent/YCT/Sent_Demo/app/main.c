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
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
sent_fast_msg_t fastMessage[SENT_CH_COUNT];
sent_slow_msg_t slowMessage[SENT_CH_COUNT];
sent_event_t activeEvents[SENT_CH_COUNT];

uint32_t fastMsgCnt[SENT_CH_COUNT] = { 0, 0 };
uint32_t slowMsgCnt[SENT_CH_COUNT] = { 0, 0 };
uint32_t errorMsgCnt[SENT_CH_COUNT] = { 0, 0 };
uint32_t errorMsgSts[SENT_CH_COUNT] = { 0, 0 };
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Callback for SENT handler */
void sentNotification(uint32_t instance, uint32_t channel, sent_callback_type_t type, void *param)
{
    (void)param;
    switch (type)
    {
        case SENT_CALLBACK_SLOW_DMA_RX_COMPLETE:
            break;

        case SENT_CALLBACK_FAST_DMA_RX_COMPLETE:
            break;

        case SENT_CALLBACK_SLOW_RX_COMPLETE:
            SENT_DRV_GetSlowMsg(instance, channel, &slowMessage[channel]);
            slowMsgCnt[channel]++;
            break;

        case SENT_CALLBACK_FAST_RX_COMPLETE:
            SENT_DRV_GetFastMsg(instance, channel, &fastMessage[channel]);
            fastMsgCnt[channel]++;
            break;

        case SENT_CALLBACK_RX_ERROR:
            SENT_DRV_GetEvents(instance, channel, &activeEvents[channel]);
            errorMsgCnt[channel]++;
            break;

        default:
            break;
    }
}

/* Printf current slow message and short message */
void sent_print_msg(uint8_t ch)
{
    PRINTF("********** CH %d **********\n", ch);
    if (SENT_SLOW_TYPE_SHORT == slowMessage[ch].type)
    {
        PRINTF("Short message cnt %d data = %x, TS = %x, crc = %x, id = %x\n",
               slowMsgCnt[ch],
               slowMessage[ch].data,
               slowMessage[ch].timeStamp,
               slowMessage[ch].crc,
               slowMessage[ch].id);
    }
    else if (SENT_SLOW_TYPE_ENHANCED_4_BIT == slowMessage[ch].type)
    {
        PRINTF("Enhance message with 4-bit cnt %d data = %x, TS = %x, crc = %x, id = %x\n",
               slowMsgCnt[ch],
               slowMessage[ch].data,
               slowMessage[ch].timeStamp,
               slowMessage[ch].crc,
               slowMessage[ch].id);
    }
    else if (SENT_SLOW_TYPE_ENHANCED_8_BIT == slowMessage[ch].type)
    {
        PRINTF("Enhance message with 8-bit cnt %d data = %x, TS = %x, crc = %x, id = %x\n",
               slowMsgCnt[ch],
               slowMessage[ch].data,
               slowMessage[ch].timeStamp,
               slowMessage[ch].crc,
               slowMessage[ch].id);
    }

    PRINTF("Fast message cnt %d data = %x, TS = %x, crc = %x\n",
           fastMsgCnt[ch],
           fastMessage[ch].data,
           fastMessage[ch].timeStamp,
           fastMessage[ch].crc);
    PRINTF("Error message cnt %d, status = %x\n", errorMsgCnt[ch], errorMsgSts[ch]);
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
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("SENT demo!\n");
    PRINTF("Wait SENT receive several messages\n");
    OSIF_TimeDelay(1000);
    /* Printf message of channel 0 */
    sent_print_msg(0);
    /* Printf message of channel 1 */
    sent_print_msg(1);
    SENT_DRV_Deinit(0);
    PRINTF("SENT demo End!\n");
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
    DMA_DRV_Init(&dmaState, &dmaController_InitConfig, dmaChnState, dmaChnConfigArray, NUM_OF_CONFIGURED_DMA_CHANNEL);
    UTILITY_PRINT_Init();
    SENT_DRV_Init(0, &sentUserConfig, &sentState);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
