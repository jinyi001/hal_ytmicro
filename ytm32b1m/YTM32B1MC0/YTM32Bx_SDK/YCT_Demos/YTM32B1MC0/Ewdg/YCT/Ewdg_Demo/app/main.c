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
//Ewdg and Wdg share the same inerrupt vector number.
#define EWDG0_IRQn WDG0_IRQn
#define EWDG_INST  0
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
void EWDG_Watchdog_ISR(void)
{
    EWDG_DRV_DeInit(0);
    /*Interruption of printing will take a certain amount of time. 
    This is only for demo demonstration. 
    In actual applications, you need to delete printing.
    */
    PRINTF("Ewdg interrupt!\n");
}

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
    uint32_t flag = 0;
    /* USER CODE END 1 */ 
    Board_Init();
    /* USER CODE BEGIN 2 */

    /* Install interrupt handler for EWDG and Watchdog */
    INT_SYS_InstallHandler(EWDG0_IRQn, &EWDG_Watchdog_ISR, (isr_t *)0);
    /* Enable the interrupt */
    INT_SYS_EnableIRQ(EWDG0_IRQn);

    EWDG_DRV_Init(EWDG_INST, &ewdg_config0);
    /* EWDG_IN input high */
    /* In this mode, PTE8 is connected to EWDG_OUT_b and PTB4 is connected to EWDG_IN. */
    for (uint8_t idx = 0; idx < 8; idx++)
    {
        OSIF_TimeDelay(25);
        EWDG_DRV_Refresh(EWDG_INST);
        /* If the level of ewdg_in input is high, ewdg_out_b outputs a high level. */
        if (((PINS_DRV_ReadPins(GPIOE) >> 8) & 0x01) == 1)
        {
            PRINTF("FEED DOG SUCCESSFUL!\n");
        }
        else
        {
            PRINTF("FEED DOG FAILED!\n");
            flag = 1;
        }
    }

    OSIF_TimeDelay(50);
    /* Waiting timeout,  ewdg_out_b outputs a low level. */
    if ((!flag) && (((PINS_DRV_ReadPins(GPIOE) >> 8) & 0x01) == 0))
    {
        PRINTF("Refreshed timeout,The EWDG_OUT_b signal assert!\n");
    }

    EWDG_DRV_DeInit(EWDG_INST);
    EWDG_DRV_Init(EWDG_INST,&ewdg_config0);
    /* EWDG_IN input Low */
    PINS_DRV_WritePin(GPIOB, 4, 0);
    if (((PINS_DRV_ReadPins(GPIOE) >> 8) & 0x01) == 1)
    {
        PRINTF(" deinit fail\n");
    }
    EWDG_DRV_Refresh(EWDG_INST);
    /* If the level of ewdg_in input is low, ewdg_out_b outputs a low level. */
    if (((PINS_DRV_ReadPins(GPIOE) >> 8) & 0x01) == 0)
    {
        PRINTF("EWDG_IN input low, service EWDG, The EWDG_OUT_b signal assert!\n");
    } else {
        PRINTF("EWDG_IN input low fail!\n");
    }
    PRINTF("Demo end!\n");
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
    CLOCK_SYS_Init(g_clockManConfigsArr,CLOCK_MANAGER_CONFIG_CNT,g_clockManCallbacksArr,CLOCK_MANAGER_CALLBACK_CNT);
    if(STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX,CLOCK_MANAGER_POLICY_AGREEMENT))
    {
        /* USER CODE BEGIN ERROR_HANDLER 1 */
        SystemSoftwareReset();
        /* USER CODE END ERROR_HANDLER 1 */
    }
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0,g_pin_mux_InitConfigArr0);
    UTILITY_PRINT_Init();
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
