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
#define INST_LIN   0
#define INST_LPTMR 0

#define TIMER_COMPARE_VAL         (uint16_t)(20000U)
#define TIMER_TICKS_1US           (uint16_t)(40U)
#define TIMEOUT                   500
#define FRAME_SLAVE_RECEIVE_DATA  (1U)
#define FRAME_MASTER_RECEIVE_DATA (2U)
#define FRAME_GO_TO_SLEEP         (3U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint16_t timerOverflowInterruptCount = 0U;
volatile bool txCompFlag = false;
volatile bool rxCompFlag = false;
volatile bool wakeupSignalFlag = false;
volatile bool sleepSignalFlag = false;

/* Define for DATA buffer transmit */
uint8_t txBuff[8] = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18 };
/* Define for DATA buffer receiver */
uint8_t rxBuff[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const char *power_mode_str[] = { "RUN", "SLEEP", "DEEPSLEEP", "STANDBY" };
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*!
 * @brief LPTMR Interrupt Service Routine
 * The ISR will call LIN timeout service every 500us
 */
void LPTMR_ISR(void)
{
    /* Timer Interrupt Handler */
    /* Provide timeout service for LIN,because LIN has frame header and response timeout mechanism */
    LIN_DRV_TimeoutService(INST_LIN);
    /* Increment overflow count */
    timerOverflowInterruptCount++;
    /* Clear compare flag */
    lpTMR_DRV_ClearCompareFlag(INST_LPTMR);
}

/*!
 * @brief Callback function to get time interval in nano seconds
 *
 *  counterValue : New counter value of lptmr
 *  timerOverflowInterruptCount : The number of times lptmr overflowed
 *  TIMER_COMPARE_VAL : The compare value of lptmr
 *  previousCountValue : The previous counter value of lptmr
 *  TIMER_TICKS_1US : The tick value of LPTMR counting 1 microsecond
 *
 *  Time calculation formula : 
 *
 *      Time(ns) = (timerOverflowInterruptCount * TIMER_COMPARE_VAL + counterValue - previousCountValue) * 1000U / TIMER_TICKS_1US
 *      
 * @param[out] ns - number of nanoseconds passed since the last call of the function
 * @return dummy value
 */
uint32_t linTimerGetTimeIntervalCallback(uint32_t *ns)
{
    static uint32_t previousCountValue = 0UL;
    uint32_t counterValue;

    counterValue = lpTMR_DRV_GetCounterValueByCount(INST_LPTMR);
    *ns = ((uint32_t)(counterValue + timerOverflowInterruptCount * TIMER_COMPARE_VAL - previousCountValue)) * 1000U /
          TIMER_TICKS_1US;
    timerOverflowInterruptCount = 0UL;
    previousCountValue = counterValue;

    return 0UL;
}

static void CallbackHandler(uint32_t instance, lin_state_t *lin_State)
{
    (void)instance;

    switch (lin_State->currentEventId)
    {
        case LIN_PID_OK:
            /* Set frame header timeout */
            LIN_DRV_SetTimeoutCounter(INST_LIN, TIMEOUT);
            /* If PID is FRAME_MASTER_RECEIVE_DATA, master node will receive data */
            if (FRAME_MASTER_RECEIVE_DATA == lin_State->currentId)
            {
                LIN_DRV_SendFrameData(INST_LIN, txBuff, sizeof(txBuff));
                /* Call to Send Frame DATA Function */
            }
            /* If PID is FRAME_SLAVE_RECEIVE_DATA, salve node will receive data from master node */
            else if (FRAME_SLAVE_RECEIVE_DATA == lin_State->currentId)
            {
                LIN_DRV_ReceiveFrameData(INST_LIN, rxBuff, sizeof(rxBuff));
            }
            /* If PID is FRAME_GO_TO_SLEEP, salve node will go to sleep mode */
            else if (FRAME_GO_TO_SLEEP == lin_State->currentId)
            {
                sleepSignalFlag = true;
                LIN_DRV_ReceiveFrameData(INST_LIN, rxBuff, sizeof(rxBuff));
            }
            else
            {
                /* Go to idle mode */
                LIN_DRV_GotoIdleState(INST_LIN);
            }
            break;
        case LIN_PID_ERROR:
            /* Go to idle mode */
            LIN_DRV_GotoIdleState(INST_LIN);
            break;
        case LIN_TX_COMPLETED:
            txCompFlag = true;
            /* Go to idle mode */
            LIN_DRV_GotoIdleState(INST_LIN);
            break;
        case LIN_RX_COMPLETED:
            rxCompFlag = true;
            /* Go to idle mode */
            LIN_DRV_GotoIdleState(INST_LIN);
            break;
        case LIN_CHECKSUM_ERROR:
        case LIN_READBACK_ERROR:
        case LIN_FRAME_ERROR:
        case LIN_RECV_BREAK_FIELD_OK:
            /* Set response timeout */
            LIN_DRV_SetTimeoutCounter(INST_LIN, TIMEOUT);
            break;
        case LIN_WAKEUP_SIGNAL:
            /* Set wakeup signal flag */
            wakeupSignalFlag = true;
            break;
        case LIN_SYNC_ERROR:
        case LIN_BAUDRATE_ADJUSTED:
        case LIN_NO_EVENT:
        case LIN_SYNC_OK:
        default:
            /* do nothing */
            break;
    }
}

/*!
 * @brief Main LIN slave task
 * This function will initialize the LIN interface and set the scheme
 * to the normal table.
 * After the LIN network is initialized, the function will perform feedback 
 * operations based on the received data
 */
void lin_slave_baremetal_task(void)
{
    /* Install callback function */
    LIN_DRV_InstallCallback(INST_LIN, (lin_callback_t)CallbackHandler);

    for (;;)
    {
        if (txCompFlag)
        {
            txCompFlag = false;
            PRINTF("lin send data success!\n");
            PRINTF("Slave send %x, %x, %x, %x, %x, %x, %x, %x\n",
                   txBuff[0],
                   txBuff[1],
                   txBuff[2],
                   txBuff[3],
                   txBuff[4],
                   txBuff[5],
                   txBuff[6],
                   txBuff[7]);
        }
        if (rxCompFlag && !sleepSignalFlag)
        {
            rxCompFlag = false;
            PRINTF("lin receive data success!\n");
            PRINTF("Slave receive %x, %x, %x, %x, %x, %x, %x, %x\n",
                   rxBuff[0],
                   rxBuff[1],
                   rxBuff[2],
                   rxBuff[3],
                   rxBuff[4],
                   rxBuff[5],
                   rxBuff[6],
                   rxBuff[7]);
        }
        if (rxCompFlag && sleepSignalFlag)
        {
            rxCompFlag = false;
            sleepSignalFlag = false;
            PRINTF("lin enter sleep mode!\n");
            /* Go to sleep mode */
            LIN_DRV_GoToSleepMode(INST_LIN);

            if (rxBuff[0] < POWER_MANAGER_CONFIG_CNT)
            {
                /* Chip go to sleep */
                PRINTF("Wait 1s to enter low power mode: %s\n", power_mode_str[rxBuff[0]]);
                OSIF_TimeDelay(1000);
                PRINTF("Start to enter low power mode: %s\n", power_mode_str[rxBuff[0]]);
                if (rxBuff[0] == 0x01)
                {
                    /* Prevent PTMR from waking up MCU */
                    INT_SYS_DisableIRQ(lpTMR0_IRQn);
                }
                POWER_SYS_SetMode(rxBuff[0], POWER_MANAGER_POLICY_AGREEMENT);
                PRINTF("Wake up from  %s\n", power_mode_str[rxBuff[0]]);
                if (rxBuff[0] == 0x01)
                {
                    /* enable ptmr for measureing the wakeup signal */
                    INT_SYS_EnableIRQ(lpTMR0_IRQn);
                }
                while (!wakeupSignalFlag)
                    ;
                wakeupSignalFlag = false;
                PRINTF("lin wakeup success!\n");
                break;
            }
        }
        if (wakeupSignalFlag)
        {
            wakeupSignalFlag = false;
            PRINTF("lin wakeup success!\n");
            break;
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
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("LIN slave low power Demo.\n");
    lin_slave_baremetal_task();
    PRINTF("Lin slave low power demo finished.\n");
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
    LIN_DRV_Init(INST_LIN, &lin_config0, &lin_config0_State);
    lpTMR_DRV_Init(INST_LPTMR, &LPTMR_Config, false);
    INT_SYS_InstallHandler(lpTMR0_IRQn, LPTMR_ISR, (isr_t *)NULL);
    INT_SYS_EnableIRQ(lpTMR0_IRQn);
    lpTMR_DRV_StartCounter(INST_LPTMR);
    POWER_SYS_Init(&powerConfigsArr, POWER_MANAGER_CONFIG_CNT, NULL, POWER_MANAGER_CALLBACK_CNT);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
