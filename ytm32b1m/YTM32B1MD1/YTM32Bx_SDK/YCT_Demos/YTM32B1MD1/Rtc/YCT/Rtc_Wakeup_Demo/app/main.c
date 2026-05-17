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
#define RTC_INST       (0U)
#define RUN_MODE       (0U)
#define SLEEP_MODE     (1U)
#define DEEPSLEEP_MODE (2U)
#define STANDBY_MODE   (3U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void second_callback(void *param)
{
    rtc_timedate_t time;
    (void)param;
    RTC_DRV_GetCurrentTimeDate(RTC_INST, &time);
    /* Note: Debug output inserted into interrupt routine for demo clarity. Might introduce delay. */
    PRINTF("%d/%d/%d %d:%d:%d\n", time.year, time.month, time.day, time.hour, time.minutes, time.seconds);
}

void alarm_callback(void *param)
{
    (void)param;
    PRINTF("RTC alarm interrupt!\n");
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
    const rtc_timedate_t currentTime = { .year = 2023, .month = 10, .day = 1, .hour = 13, .minutes = 37, .seconds = 20 };
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("rtc wakeup!\r\n");
    RTC_DRV_Deinit(RTC_INST);
    RTC_DRV_Init(RTC_INST, &rtc_config0);
    RTC_DRV_SetTimeDate(RTC_INST, &currentTime);

    RTC_DRV_StartCounter(RTC_INST);
    for (uint8_t i = 0; i < 10; i++)
    {
        PRINTF("Start to enter STANDBY mode.\n");
        POWER_SYS_SetMode(STANDBY_MODE, POWER_MANAGER_POLICY_AGREEMENT);
        PRINTF("wakeup from standby mode\n");
    }
    OSIF_TimeDelay(1000);
    PRINTF("RTC Wakeup Demo Pass!\n");
    RTC_DRV_Deinit(RTC_INST);
    PRINTF("RTC Wakeup Demo End.\n");
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
    POWER_SYS_Init(&powerConfigsArr, POWER_MANAGER_CONFIG_CNT, NULL, POWER_MANAGER_CALLBACK_CNT);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
