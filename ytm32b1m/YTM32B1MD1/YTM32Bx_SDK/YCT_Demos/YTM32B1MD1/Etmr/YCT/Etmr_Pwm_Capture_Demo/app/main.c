/* USER CODE BEGIN Header */
/* you can remove the copyright */

/*
 *  Copyright 2020-2024 Yuntu Microelectronics Co., Ltd.
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
#define PWM_INST     (0U)
#define CAPTURE_INST (3U)
#define CHAN_NUM     (4U)

#define CAPINST_CHAN0 (0U)
#define CAPINST_CHAN1 (1U)
#define CAPINST_CHAN2 (2U)
#define CAPINST_CHAN3 (3U)

#define MAX_DUTY       (100.0)
#define MAX_DUTY_CODE  (0x8000U)
#define HALF_DUTY_CODE (0x4000U)
#define STEP_DUTY_CODE (0x1000U)

#define PWM_FREQ_0 (5000U)
#define PWM_FREQ_1 (10000U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint32_t period[CHAN_NUM] = { 0 };
uint32_t dutyCycle[CHAN_NUM] = { 0 };
uint32_t capChannel[CHAN_NUM] = { CAPINST_CHAN0, CAPINST_CHAN1, CAPINST_CHAN2, CAPINST_CHAN3 };
volatile bool etmrError = false;
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
static void eTMR_MaskChannel(uint32_t instance, uint8_t maskChannel, uint16_t maskValue);
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void checkFreqDuty(uint32_t freq, uint32_t expFreq, uint32_t duty, uint32_t expDuty)
{
    if ((freq < (expFreq - 10)) || (freq > (expFreq + 10)))
    {
        PRINTF("Error, expected frequency is %d but actual %d\n", expFreq, freq);
        etmrError = true;
    }
    if ((duty < (expDuty - 5)) || (duty > (expDuty + 5)))
    {
        PRINTF("Error, expected duty is %d but actual %d\n", expDuty, duty);
        etmrError = true;
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
    uint8_t i = 0;
    uint32_t busClk = 0;
    uint32_t etmrClk = 0;
    /* USER CODE END 1 */
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Hello eTMR!\r\n");
    PRINTF("Demo shows eTMR0 output PWM and eTMR3 capture it.\n");
    PRINTF("Please short \n");
    PRINTF("\tPTB12--PTB8\n");
    PRINTF("\tPTB13--PTB9\n");
    PRINTF("\tPTB14--PTC6\n");
    PRINTF("\tPTB15--PTC7\n");

    /* Get Core clock and eTMR clock */
    CLOCK_DRV_GetFreq(FAST_BUS_CLK, &busClk);
    etmrClk = busClk / ETMR_CM_Config2.etmrPrescaler;

    /* Enable PWM and Input Capture */
    eTMR_DRV_Enable(PWM_INST);
    eTMR_DRV_Enable(CAPTURE_INST);
    /* Print PWM frequency and duty */
    for (i = 0; i < CHAN_NUM; i++)
    {
        PRINTF("PWM channel%d freq is %d Hz and duty is %.1f%%\n",
               capChannel[i],
               PWM_FREQ_0,
               ETMR_PWM_Config0.pwmChannelConfig[i].dutyCycle * MAX_DUTY / MAX_DUTY_CODE);
    }
    OSIF_TimeDelay(10);
    /* Get PWM period and duty */
    for (i = 0; i < CHAN_NUM; i++)
    {
        if (eTMR_DRV_GetInputCaptureComplete(CAPTURE_INST, capChannel[i]))
        {
            period[i] = eTMR_DRV_GetInputCaptureMeasurementPeriod(CAPTURE_INST, capChannel[i]);
            dutyCycle[i] = (uint32_t)(eTMR_DRV_GetInputCapturePositivePulseCount(CAPTURE_INST, capChannel[i]) *
                                      MAX_DUTY / period[i]);
            eTMR_DRV_ClearInputCaptureComplete(CAPTURE_INST, capChannel[i]);
        }
    }

    /* Print Capture result */
    for (i = 0; i < CHAN_NUM; i++)
    {
        PRINTF("Capture Channel%d freq = %d Hz and duty is %d%% \n", capChannel[i], etmrClk / period[i], dutyCycle[i]);
        checkFreqDuty(etmrClk / period[i],
                      PWM_FREQ_0,
                      dutyCycle[i],
                      (uint32_t)(ETMR_PWM_Config0.pwmChannelConfig[i].dutyCycle * MAX_DUTY / MAX_DUTY_CODE));
    }

    /* Update PWM duty */
    PRINTF("Update PWM duty.\n");
    for (i = 0; i < CHAN_NUM; i++)
    {
        eTMR_DRV_UpdatePwmChannel(PWM_INST, capChannel[i], HALF_DUTY_CODE + i * STEP_DUTY_CODE, 0);
    }
    eTMR_DRV_SetLdok(PWM_INST);
    /* Print PWM frequency and duty */
    for (i = 0; i < CHAN_NUM; i++)
    {
        PRINTF("PWM channel%d freq is %d Hz and duty is %.1f%%\n",
               capChannel[i],
               PWM_FREQ_0,
               (HALF_DUTY_CODE + i * STEP_DUTY_CODE) * MAX_DUTY / MAX_DUTY_CODE);
        eTMR_DRV_ClearInputCaptureComplete(CAPTURE_INST, capChannel[i]);
    }
    OSIF_TimeDelay(10);

    /* Get PWM period and duty */
    for (i = 0; i < CHAN_NUM; i++)
    {
        if (eTMR_DRV_GetInputCaptureComplete(CAPTURE_INST, capChannel[i]))
        {
            period[i] = eTMR_DRV_GetInputCaptureMeasurementPeriod(CAPTURE_INST, capChannel[i]);
            dutyCycle[i] = (uint32_t)(eTMR_DRV_GetInputCapturePositivePulseCount(CAPTURE_INST, capChannel[i]) *
                                      MAX_DUTY / period[i]);
            eTMR_DRV_ClearInputCaptureComplete(CAPTURE_INST, capChannel[i]);
        }
    }

    /* Print Capture result */
    for (i = 0; i < CHAN_NUM; i++)
    {
        PRINTF("Capture Channel%d freq = %d Hz and duty is %d%% \n", capChannel[i], etmrClk / period[i], dutyCycle[i]);
        checkFreqDuty(etmrClk / period[i],
                      PWM_FREQ_0,
                      dutyCycle[i],
                      (uint32_t)((HALF_DUTY_CODE + i * STEP_DUTY_CODE) * MAX_DUTY / MAX_DUTY_CODE));
    }

    /* Update PWM Period */
    PRINTF("Update PWM frequency.\n");
    eTMR_DRV_UpdatePwmPeriod(PWM_INST, eTMR_PWM_PERIOD_IN_HZ, PWM_FREQ_1);
    eTMR_DRV_SetLdok(PWM_INST);
    /* Print PWM frequency and duty */
    for (i = 0; i < CHAN_NUM; i++)
    {
        PRINTF("PWM channel%d freq is %d Hz and duty is %.1f%%\n",
               capChannel[i],
               PWM_FREQ_1,
               (HALF_DUTY_CODE + i * STEP_DUTY_CODE) * MAX_DUTY / MAX_DUTY_CODE);
        eTMR_DRV_ClearInputCaptureComplete(CAPTURE_INST, capChannel[i]);
    }
    OSIF_TimeDelay(10);

    /* Get PWM period and duty */
    for (i = 0; i < CHAN_NUM; i++)
    {
        if (eTMR_DRV_GetInputCaptureComplete(CAPTURE_INST, capChannel[i]))
        {
            period[i] = eTMR_DRV_GetInputCaptureMeasurementPeriod(CAPTURE_INST, capChannel[i]);
            dutyCycle[i] = (uint32_t)(eTMR_DRV_GetInputCapturePositivePulseCount(CAPTURE_INST, capChannel[i]) *
                                      MAX_DUTY / period[i]);
            eTMR_DRV_ClearInputCaptureComplete(CAPTURE_INST, capChannel[i]);
        }
    }

    /* Print Capture result */
    for (i = 0; i < CHAN_NUM; i++)
    {
        PRINTF("Capture Channel%d freq = %d Hz and duty is %d%% \n", capChannel[i], etmrClk / period[i], dutyCycle[i]);
        checkFreqDuty(etmrClk / period[i],
                      PWM_FREQ_1,
                      dutyCycle[i],
                      (uint32_t)((HALF_DUTY_CODE + i * STEP_DUTY_CODE) * MAX_DUTY / MAX_DUTY_CODE));
    }

    PRINTF("Channel 0, 1 output 0%% duty PWM.\n");
    PRINTF("Channel 2, 3 output 100%% duty PWM.\n");
    eTMR_MaskChannel(PWM_INST, (0 << 0 | 0 << 1 | 1 << 2 | 1 << 3), (0 << 0*2 | 0 << 1*2 | 1 << 2*2 | 1 << 3*2));
    eTMR_DRV_UpdatePwmChannel(PWM_INST, 0, 0, 0);
    eTMR_DRV_UpdatePwmChannel(PWM_INST, 1, 0, 0);
    eTMR_DRV_SetLdok(PWM_INST);
    OSIF_TimeDelay(10);

    /* Update PWM duty */
    PRINTF("Channel 0, 1 output 100%% duty PWM.\n");
    PRINTF("Channel 2, 3 output 0%% duty PWM.\n");
    eTMR_MaskChannel(PWM_INST, (1 << 0 | 1 << 1 | 0 << 2 | 0 << 3), (1 << 0*2 | 1 << 1*2 | 0 << 2*2 | 0 << 3*2));
    eTMR_DRV_UpdatePwmChannel(PWM_INST, 2, 0, 0);
    eTMR_DRV_UpdatePwmChannel(PWM_INST, 3, 0, 0);
    eTMR_DRV_SetLdok(PWM_INST);
    OSIF_TimeDelay(10);

    /* Self test check */
    if (!etmrError)
    {
        PRINTF("eTMR PWM output and Input Capture Demo Pass.\n");
    }
    else
    {
        PRINTF("eTMR PWM output and Input Capture Demo Fail.\n");
    }
    /* Deinit eTMR */
    eTMR_DRV_Deinit(PWM_INST);
    eTMR_DRV_Deinit(CAPTURE_INST);
    PRINTF("eTMR PWM output and Input Capture Demo End.\n");
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
    eTMR_DRV_Init(PWM_INST, &ETMR_CM_Config0, &ETMR_CM_Config0_State);
    eTMR_DRV_InitPwm(PWM_INST, &ETMR_PWM_Config0);
    eTMR_DRV_Init(CAPTURE_INST, &ETMR_CM_Config2, &ETMR_CM_Config2_State);
    eTMR_DRV_InitInputCapture(CAPTURE_INST, &ETMR_IC_Config1);
}

/* USER CODE BEGIN 4 */
/*!
 * @brief Mask specified channel to 100% duty cycle
 * @param[in] instance     The eTMR peripheral instance number.
 * @param[in] maskChannel  The channel set, for example: if channel 2,3,4, then 1 << 2 | 1 << 3 | 1 << 4
 * @param[in] maskValue    The mask value set, for example: (1 << 2*2) | (1 << 3*2) | (1 << 4*2)
 */
static void eTMR_MaskChannel(uint32_t instance, uint8_t maskChannel, uint16_t maskValue)
{
    eTMR_Type *base = g_etmrBase[instance];

    uint32_t tmp = ((maskChannel) & 0xFFFFU) | ((maskValue << 16) & 0xFFFF0000U);

    base->CHMASK = tmp;
}
/* USER CODE END 4 */
