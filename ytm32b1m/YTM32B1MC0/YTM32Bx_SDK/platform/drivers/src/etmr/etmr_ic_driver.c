/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_ic_driver.c
 * @version 1.4.0
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 2985 Rule 2.2: This operation is redundant. The value of the result is
 *                       always that of the left-hand operand.
 *
 */

#include "etmr_ic_driver.h"
#include "etmr_hw_access.h"
#include "interrupt_manager.h"


#if defined(CPU_YTM32B1LE0) || defined(CPU_YTM32B1LE1) || defined(CPU_YTM32Z1LS0)

void eTMR0_CH_0_7_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 0; index < g_etmrChannelNum[0]; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(0, index))
        {
            eTMR_DRV_InputCaptureHandler(0, index);
        }
    }
}

#if eTMR_INSTANCE_COUNT > 1U
void eTMR1_IRQHandler(void)
{
    for (uint8_t index = 0; index < g_etmrChannelNum[1]; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(1, index))
        {
            eTMR_DRV_InputCaptureHandler(1, index);
        }
    }
}
#endif

#if eTMR_INSTANCE_COUNT > 2U
void eTMR2_IRQHandler(void)
{
    for (uint8_t index = 0; index < g_etmrChannelNum[2]; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(2, index))
        {
            eTMR_DRV_InputCaptureHandler(2, index);
        }
    }
}
#endif

#else // CPU_YTM32B1MC0, CPU_YTM32B1MD1, CPU_YTM32B1ME0, CPU_YTM32B1HA0

void eTMR0_Ch0_Ch1_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 0; index < 2U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(0U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(0U, index);
#else
            eTMR_DRV_InputCaptureHandler(0U, index);
#endif
        }
    }
}

void eTMR0_Ch2_Ch3_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 2U; index < 4U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(0U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(0U, index);
#else
            eTMR_DRV_InputCaptureHandler(0U, index);
#endif
        }
    }
}

void eTMR0_Ch4_Ch5_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 4U; index < 6U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(0U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(0U, index);
#else
            eTMR_DRV_InputCaptureHandler(0U, index);
#endif
        }
    }
}

void eTMR0_Ch6_Ch7_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 6U; index < 8U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(0U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(0U, index);
#else
            eTMR_DRV_InputCaptureHandler(0U, index);
#endif
        }
    }
}

void eTMR1_Ch0_Ch1_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 0; index < 2U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(1U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(1U, index);
#else
            eTMR_DRV_InputCaptureHandler(1U, index);
#endif
        }
    }
}

void eTMR1_Ch2_Ch3_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 2U; index < 4U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(1U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(1U, index);
#else
            eTMR_DRV_InputCaptureHandler(1U, index);
#endif
        }
    }
}

void eTMR1_Ch4_Ch5_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 4U; index < 6U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(1U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(1U, index);
#else
            eTMR_DRV_InputCaptureHandler(1U, index);
#endif
        }
    }
}

void eTMR1_Ch6_Ch7_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 6U; index < 8U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(1U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(1U, index);
#else
            eTMR_DRV_InputCaptureHandler(1U, index);
#endif
        }
    }
}

#if eTMR_INSTANCE_COUNT > 2U
void eTMR2_Ch0_Ch1_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 0U; index < 2U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(2U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(2U, index);
#else
            eTMR_DRV_InputCaptureHandler(2U, index);
#endif
        }
    }
}

void eTMR2_Ch2_Ch3_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 2U; index < 4U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(2U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(2U, index);
#else
            eTMR_DRV_InputCaptureHandler(2U, index);
#endif
        }
    }
}

void eTMR2_Ch4_Ch5_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 4U; index < 6U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(2U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(2U, index);
#else
            eTMR_DRV_InputCaptureHandler(2U, index);
#endif
        }
    }
}

void eTMR2_Ch6_Ch7_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 6U; index < 8U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(2U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(2U, index);
#else
            eTMR_DRV_InputCaptureHandler(2U, index);
#endif
        }
    }
}
#endif

#if eTMR_INSTANCE_COUNT > 3U
void eTMR3_Ch0_Ch1_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 0U; index < 2U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(3U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(3U, index);
#else
            eTMR_DRV_InputCaptureHandler(3U, index);
#endif
        }
    }
}

void eTMR3_Ch2_Ch3_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 2U; index < 4U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(3U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(3U, index);
#else
            eTMR_DRV_InputCaptureHandler(3U, index);
#endif
        }
    }
}

void eTMR3_Ch4_Ch5_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 4U; index < 6U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(3U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(3U, index);
#else
            eTMR_DRV_InputCaptureHandler(3U, index);
#endif
        }
    }
}

void eTMR3_Ch6_Ch7_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 6U; index < 8U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(3U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(3U, index);
#else
            eTMR_DRV_InputCaptureHandler(3U, index);
#endif
        }
    }
}
#endif

#if eTMR_INSTANCE_COUNT > 4U
void eTMR4_Ch0_Ch1_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 0U; index < 2U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(4U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(4U, index);
#else
            eTMR_DRV_InputCaptureHandler(4U, index);
#endif
        }
    }
}

void eTMR4_Ch2_Ch3_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 2U; index < 4U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(4U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(4U, index);
#else
            eTMR_DRV_InputCaptureHandler(4U, index);
#endif
        }
    }
}

void eTMR4_Ch4_Ch5_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 4U; index < 6U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(4U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(4U, index);
#else
            eTMR_DRV_InputCaptureHandler(4U, index);
#endif
        }
    }
}

void eTMR4_Ch6_Ch7_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 6U; index < 8U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(4U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(4U, index);
#else
            eTMR_DRV_InputCaptureHandler(4U, index);
#endif
        }
    }
}
#endif

#if eTMR_INSTANCE_COUNT > 5U
void eTMR5_Ch0_Ch1_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 0U; index < 2U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(5U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(5U, index);
#else
            eTMR_DRV_InputCaptureHandler(5U, index);
#endif
        }
    }
}

void eTMR5_Ch2_Ch3_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 2U; index < 4U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(5U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(5U, index);
#else
            eTMR_DRV_InputCaptureHandler(5U, index);
#endif
        }
    }
}

void eTMR5_Ch4_Ch5_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 4U; index < 6U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(5U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(5U, index);
#else
            eTMR_DRV_InputCaptureHandler(5U, index);
#endif
        }
    }
}

void eTMR5_Ch6_Ch7_IRQHandler(void)
{
    /* Traverse all channel to process capture event */
    for (uint8_t index = 6U; index < 8U; index++)
    {
        /* If capture event occurs, process it */
        if (eTMR_DRV_GetChnFlag(5U, index))
        {
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
            eTMR_DRV_InputCaptureHardwareHandler(5U, index);
#else
            eTMR_DRV_InputCaptureHandler(5U, index);
#endif
        }
    }
}
#endif

#endif /* CPU_<device> */

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_InitInputCapture
 * Description   : This function initialize the channel in the Input Capture mode
 *
 * Implements    : eTMR_DRV_InitInputCapture_Activity
 *END**************************************************************************/
status_t eTMR_DRV_InitInputCapture(uint32_t instance, const etmr_ic_param_t *param)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(param != NULL);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    etmr_state_t *state = etmrStatePtr[instance];
    uint8_t index;
    uint8_t hwChannel;
    status_t status = STATUS_SUCCESS;

    if (NULL != state)
    {
        eTMR_SetCntVal(etmrBase, 0U);
#if FEATURE_eTMR_HAS_CNT_INIT_SRC
        eTMR_SetCounterInitValSrc(etmrBase, (uint8_t)COUNTER_VAL_FROM_INIT);
#endif
#if FEATURE_eTMR_HAS_INIT_REG
        /* Set initial value for counter */
        eTMR_SetInitVal(etmrBase, 0U);
#endif
        /* Set maximum value for counter */
        eTMR_SetMod(etmrBase, param->countValue);
        /* Get current MOD value */
        state->etmrModValue = param->countValue;

#if FEATURE_eTMR_HAS_COMBINATION_CAPTURE
        for (index = 0U; index < (param->numChannelPairs); index++)
        {
            if (param->inputChPairConfig[index].combEn)
            {
                uint8_t hwChannelPair = param->inputChPairConfig[index].hwChannelPairId;
                eTMR_DRV_SetChnCombMode(instance, hwChannelPair, param->inputChPairConfig[index].combEn);
                eTMR_DRV_SetChnCombSrc(instance, hwChannelPair, (uint8_t)param->inputChPairConfig[index].combSrc);
            }
        }
#endif

        /* Initialize channel configuration */
        for (index = 0U; index < (param->numChannels); index++)
        {
            hwChannel = param->inputChConfig[index].hwChannelId;

            /* Set input capture mode for channel */
            eTMR_SetChnMode(etmrBase, hwChannel, eTMR_INPUT_CAPTURE_MODE);
            state->etmrChnMode[hwChannel] = eTMR_STATE_INPUT_CAPTURE;
            /* Set edge detection */
            eTMR_SetChnCapEdge(etmrBase, hwChannel, (uint8_t)param->inputChConfig[index].edge);
            state->captureEdge[hwChannel] = param->inputChConfig[index].edge;
            /* Store measurement type */
            state->measurementType[hwChannel] = param->inputChConfig[index].measurementType;
#if FEATURE_eTMR_HAS_CAPTURE_HOLD
            /* Set capture hold feature */
            eTMR_SetChnCapHoldEnable(etmrBase, hwChannel, param->inputChConfig[index].capHoldEn);
#endif
            /* Set filter sample counter/period for input capture channel */
            eTMR_SetChnCapFilterSampleCounter(etmrBase, hwChannel, param->inputChConfig[index].filterSampleCounter);
            eTMR_SetChnCapFilterSamplePeriod(etmrBase, hwChannel, param->inputChConfig[index].filterSamplePeriod);
            /* Set DMA */
            if (param->inputChConfig[index].dmaEnable)
            {
                eTMR_SetChnDmaEnable(etmrBase, hwChannel, true);
            }

            /* Set channel interrupt */
            if (param->inputChConfig[index].interruptEnable)
            {
                eTMR_DRV_EnableChnInt(instance, hwChannel);
                INT_SYS_EnableIRQ(g_etmrIrqId[instance][hwChannel]);
            }

            /* Clear state elements */
            for (int i = 0; i < 3; i++)
            {
                state->icOvfCnt[hwChannel][i] = 0;
                state->icCntVal[hwChannel][i] = 0;
                state->icCapPol[hwChannel][i] = 0;
            }
            state->icCnt[hwChannel] = 0;
            state->measurementComplete[hwChannel] = false;
            state->measurementPeriod[hwChannel] = 0;
            state->measurementPosPulseCnt[hwChannel] = 0;
            state->measurementNegPulseCnt[hwChannel] = 0;
            state->measurementCapCnt[hwChannel] = 0;

            /* Save in state structure user define handlers */
            state->enableNotification[hwChannel] = param->inputChConfig[index].enableNotification;
            state->channelsCallbacksParams[hwChannel] = param->inputChConfig[index].channelsCallbacksParams;
            state->channelsCallbacks[hwChannel] = param->inputChConfig[index].channelsCallbacks;
        }
    }
    else /* state is NULL */
    {
        status = STATUS_ERROR;
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_DeinitInputCapture
 * Description   : This function de-initialize input capture mode and configure
 *                 channel as none mode
 *
 * Implements    : eTMR_DRV_DeinitInputCapture_Activity
 *END**************************************************************************/
status_t eTMR_DRV_DeinitInputCapture(uint32_t instance, const etmr_ic_param_t *param)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(param != NULL);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    etmr_state_t *state = etmrStatePtr[instance];
    DEV_ASSERT(state != NULL);
    uint8_t index;
    uint8_t hwChannel;

    /* Deinit channel configuration */
#if FEATURE_eTMR_HAS_COMBINATION_CAPTURE
    for (index = 0U; index < (param->numChannelPairs); index++)
    {
        uint8_t hwChannelPair = param->inputChPairConfig[index].hwChannelPairId;
        eTMR_DRV_SetChnCombMode(instance, hwChannelPair, false);
        eTMR_DRV_SetChnCombSrc(instance, hwChannelPair, (uint8_t)eTMR_COMB_SRC_EVEN);
    }
#endif

    for (index = 0U; index < param->numChannels; index++)
    {
        hwChannel = param->inputChConfig[index].hwChannelId;

        /* Set channel mode as none */
        eTMR_SetChnMode(etmrBase, hwChannel, eTMR_CHANNEL_DISABLE);
        /* Set channel capture edge as none */
        eTMR_SetChnCapEdge(etmrBase, hwChannel, (uint8_t)eTMR_NOT_SEL_EDGE);
        /* Disable channel interrupt */
        eTMR_DRV_DisableChnInt(instance, hwChannel);
        INT_SYS_DisableIRQ(g_etmrIrqId[instance][hwChannel]);
        /* Clear channel event flags */
        eTMR_DRV_ClearChnFlag(instance, hwChannel);
#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
        eTMR_DRV_ClearChnPosPulseRdyFlag(instance, hwChannel);
        eTMR_DRV_ClearChnNegPulseRdyFlag(instance, hwChannel);
#endif
#if FEATURE_eTMR_HAS_CAPTURE_HOLD
        eTMR_SetChnCapHoldEnable(etmrBase, hwChannel, false);
#endif
        /* Disable filter */
        eTMR_SetChnCapFilterSampleCounter(etmrBase, hwChannel, 0);
        eTMR_SetChnCapFilterSamplePeriod(etmrBase, hwChannel, 0);
        /* Disable DMA */
        eTMR_SetChnDmaEnable(etmrBase, hwChannel, false);
        /* Save etmr channel mode in state structure */
        state->etmrChnMode[hwChannel] = eTMR_STATE_NO_MODE;
        /* Clear capture edge */
        state->captureEdge[hwChannel] = eTMR_NOT_SEL_EDGE;
        /* Store measurement type */
        state->measurementType[hwChannel] = eTMR_POS_PULSE_MEASUREMENT;

        /* Clear state elements */
        for (int i = 0; i < 3; i++)
        {
            state->icOvfCnt[i][hwChannel] = 0;
            state->icCntVal[hwChannel][i] = 0;
            state->icCapPol[hwChannel][i] = 0;
        }
        state->icCnt[hwChannel] = 0;
        state->measurementComplete[hwChannel] = false;
        state->measurementPeriod[hwChannel] = 0;
        state->measurementPosPulseCnt[hwChannel] = 0;
        state->measurementNegPulseCnt[hwChannel] = 0;
        state->measurementCapCnt[hwChannel] = 0;

        /* Clear Callbacks function from the state structure */
        state->enableNotification[index] = false;
        state->channelsCallbacksParams[index] = NULL;
        state->channelsCallbacks[index] = NULL;
    }

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_GetInputCaptureComplete
 * Description   : This function is used to get complete status.
 *
 * Implements    : eTMR_DRV_GetInputCaptureComplete_Activity
 *END**************************************************************************/
bool eTMR_DRV_GetInputCaptureComplete(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    etmr_state_t *state = etmrStatePtr[instance];
    bool value = false;

    if (state != NULL)
    {
        value = state->measurementComplete[channel];
    }

    return value;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_ClearInputCaptureComplete
 * Description   : This function is used to clear complete status.
 *
 * Implements    : eTMR_DRV_ClearInputCaptureComplete_Activity
 *END**************************************************************************/
status_t eTMR_DRV_ClearInputCaptureComplete(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    etmr_state_t *state = etmrStatePtr[instance];

    if (state != NULL)
    {
        state->measurementComplete[channel] = false;
    }

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_GetInputCaptureMeasurementPeriod
 * Description   : This function is used to get the period of pwm. The
 *                 calculation is available in the eTMR_DRV_InputCaptureHandler
 *                 function.
 *
 * Implements    : eTMR_DRV_GetInputCaptureMeasurementPeriod_Activity
 *END**************************************************************************/
uint32_t eTMR_DRV_GetInputCaptureMeasurementPeriod(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    etmr_state_t *state = etmrStatePtr[instance];
    uint32_t value = 0U;

    if (state != NULL)
    {
        value = state->measurementPeriod[channel];
    }

    return value;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_GetInputCapturePositivePulseCount
 * Description   : This function is used to get positive pulse count value.
 *
 * Implements    : eTMR_DRV_GetInputCapturePositivePulseCount_Activity
 *END**************************************************************************/
uint32_t eTMR_DRV_GetInputCapturePositivePulseCount(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    const etmr_state_t *state = etmrStatePtr[instance];
    uint32_t value = 0;

    if (state != NULL)
    {
        value = state->measurementPosPulseCnt[channel];
    }

    return value;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_GetInputCaptureNegativePulseCount
 * Description   : This function is used to get negative pulse count value.
 *
 * Implements    : eTMR_DRV_GetInputCaptureNegativePulseCount_Activity
 *END**************************************************************************/
uint32_t eTMR_DRV_GetInputCaptureNegativePulseCount(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    const etmr_state_t *state = etmrStatePtr[instance];
    uint32_t value = 0;

    if (state != NULL)
    {
        value = state->measurementNegPulseCnt[channel];
    }

    return value;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_GetInputCaptureValue
 * Description   : This function is used to get capture count value.
 *
 * Implements    : eTMR_DRV_GetInputCaptureValue_Activity
 *END**************************************************************************/
uint32_t eTMR_DRV_GetInputCaptureValue(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    return eTMR_GetChnCapVal(etmrBase, channel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_InputCaptureHandler
 * Description   : This function is used to get capture value, polarity and the
 *                 count of overflow on each edge.
 *
 * Implements    : eTMR_DRV_InputCaptureHandler_Activity
 *END**************************************************************************/
void eTMR_DRV_InputCaptureHandler(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    etmr_state_t *state = etmrStatePtr[instance];
    eTMR_Type *etmrBase = g_etmrBase[instance];

    etmrBase->STS = ((uint32_t)0x1U << channel) << eTMR_STS_CH0F_SHIFT; /*PRQA S 2985*/

    if (state->measurementComplete[channel] == false)
    {
        /* Get capture value */
        state->icCntVal[channel][state->icCnt[channel]] = etmrBase->CH[channel].CVAL;
        /* Get io status */
        state->icCapPol[channel][state->icCnt[channel]] = (uint8_t)((etmrBase->IOSTS >> channel) & eTMR_IOSTS_CH0IO_MASK);

        /* This branch is used to capture signal period and duty of the pwm signal */
        if (state->measurementType[channel] == eTMR_PERIOD_MEASUREMENT)
        {
            /* Use dual edges capture method to measure period */
            if (state->captureEdge[channel] == eTMR_DUAL_EDGES)
            {
                /* Capture three edges, when capture count is equal to 3, capture complete */
                if (state->icCnt[channel] < 2U)
                {
                    state->icCnt[channel]++;
                }
                else
                {
                    state->measurementComplete[channel] = true;
                    state->icCnt[channel] = 0U;
                }
            }
            else /* Use positive edge or negative edge capture method to measure period */
            {
                /* Capture two edges, when capture count is equal to 2, capture complete */
                if (state->icCnt[channel] < 1U)
                {
                    state->icCnt[channel]++;
                }
                else
                {
                    state->measurementComplete[channel] = true;
                    state->icCnt[channel] = 0U;
                }
            }
        }
        else /* Measure one positive pulse or one negative pulse */
        {
            /* When capturing one positive pulse or one negative pulse, it is necessary to use dual edges capture */
            if (state->captureEdge[channel] == eTMR_DUAL_EDGES)
            {
                /* When capture count is equal to 2, capture complete */
                if (state->icCnt[channel] < 1U)
                {
                    state->icCnt[channel]++;
                }
                else
                {
                    state->measurementComplete[channel] = true;
                    state->icCnt[channel] = 0U;
                }
            }
        }

        /* If the callback is defined to use it */
        if ((state->channelsCallbacks[channel] != NULL) && (state->enableNotification[channel] == true))
        {
            if (state->measurementComplete[channel] == true)
            {
                state->channelsCallbacks[channel](IC_EVENT_MEASUREMENT_COMPLETE, state->channelsCallbacksParams[channel]);
            }
        }
    }
}

#if defined(FEATURE_eTMR_HAS_HARDWARE_CAPTURE) && (FEATURE_eTMR_HAS_HARDWARE_CAPTURE == 1U)
/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_InputCaptureHardwareHandler
 * Description   : This function is used to get positive pulse width, negative
 *                 pulse width and period. If use hardware process, there is no
 *                 no need to use eTMR_DRV_CalculateChannelCaptureValue function.
 *                 Furthermore, overflow interrupt software process is no longer
 *                 required.
 *
 *END**************************************************************************/
void eTMR_DRV_InputCaptureHardwareHandler(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    etmr_state_t *state = etmrStatePtr[instance];
    eTMR_Type *etmrBase = g_etmrBase[instance];
    bool chnPosPulseRdyFlag = false;
    bool chnNegPulseRdyFlag = false;
    uint32_t posTmp0 = 0U;
    uint32_t posTmp1 = 0U;
    uint32_t negTmp0 = 0U;
    uint32_t negTmp1 = 0U;

    etmrBase->STS = ((uint32_t)0x1U << channel) << eTMR_STS_CH0F_SHIFT; /*PRQA S 2985*/

    if (state->measurementComplete[channel] == false)
    {
        if (state->measurementType[channel] == eTMR_PERIOD_MEASUREMENT)
        {
            chnPosPulseRdyFlag = eTMR_DRV_GetChnPosPulseRdyFlag(instance, channel);
            chnNegPulseRdyFlag = eTMR_DRV_GetChnNegPulseRdyFlag(instance, channel);
            if (chnPosPulseRdyFlag && chnNegPulseRdyFlag)
            {
                posTmp0 = etmrBase->CH[channel].PPCNTV;
                posTmp1 = etmrBase->CH[channel].PPCNTN * state->etmrModValue;
                negTmp0 = etmrBase->CH[channel].NPCNTV;
                negTmp1 = etmrBase->CH[channel].NPCNTN * state->etmrModValue;
                state->measurementPosPulseCnt[channel] = posTmp0 + posTmp1;
                state->measurementNegPulseCnt[channel] = negTmp0 + negTmp1;
                state->measurementPeriod[channel] = state->measurementPosPulseCnt[channel] + state->measurementNegPulseCnt[channel];
                eTMR_DRV_ClearChnPosPulseRdyFlag(instance, channel);
                eTMR_DRV_ClearChnNegPulseRdyFlag(instance, channel);
                state->measurementComplete[channel] = true;
            }
        }
        else if (state->measurementType[channel] == eTMR_POS_PULSE_MEASUREMENT)
        {
            if (eTMR_DRV_GetChnPosPulseRdyFlag(instance, channel))
            {
                posTmp0 = etmrBase->CH[channel].PPCNTV;
                posTmp1 = etmrBase->CH[channel].PPCNTN * state->etmrModValue;
                state->measurementPosPulseCnt[channel] = posTmp0 + posTmp1;
                eTMR_DRV_ClearChnPosPulseRdyFlag(instance, channel);
                state->measurementComplete[channel] = true;
            }
        }
        else // eTMR_NEG_PULSE_MEASUREMENT
        {
            if (eTMR_DRV_GetChnNegPulseRdyFlag(instance, channel))
            {
                negTmp0 = etmrBase->CH[channel].NPCNTV;
                negTmp1 = etmrBase->CH[channel].NPCNTN * state->etmrModValue;
                state->measurementNegPulseCnt[channel] = negTmp0 + negTmp1;
                eTMR_DRV_ClearChnNegPulseRdyFlag(instance, channel);
                state->measurementComplete[channel] = true;
            }
        }
                
        /* If the callback is defined to use it */
        if ((state->channelsCallbacks[channel] != NULL) && (state->enableNotification[channel] == true))
        {
            if (state->measurementComplete[channel] == true)
            {
                state->channelsCallbacks[channel](IC_EVENT_MEASUREMENT_COMPLETE, state->channelsCallbacksParams[channel]);
            }
        }
    }
}
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_CalculateChannelCaptureValue
 * Description   : This function is used to calculate period, positive pulse or
                   negative pulse count for each channel. When measurementType is 
                   eTMR_PERIOD_MEASUREMENT and captureEdge is eTMR_DUAL_EDGES,
                   period, positive pulse width and negative pulse width can be 
                   measured. When measurementType is eTMR_PERIOD_MEASUREMENT and
                   captureEdge is eTMR_POS_EDGE or eTMR_NEG_EDGE, period can be
                   measured. When measurementType is eTMR_POS_PULSE_MEASUREMENT
                   or eTMR_NEG_PULSE_MEASUREMENT, positive pulse width or negative
                   pulse width can be measured. It is necessary to use dual edges
                   capture method to measure positive pulse width or negative pulse
                   width.
 *END**************************************************************************/
void eTMR_DRV_CalculateChannelCaptureValue(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    etmr_state_t *state = etmrStatePtr[instance];

    if (state->measurementType[channel] == eTMR_PERIOD_MEASUREMENT)
    {
        if (state->captureEdge[channel] == eTMR_DUAL_EDGES)
        {
            /* Process the first pulse */
            /* When first capture value is greater than the second capture value, overflow count is 1 at least, so if overflow n times,
             * it needs to add (n-1)*mod. */
            if (state->icCntVal[channel][0] > state->icCntVal[channel][1])
            {
                state->measurementCapCnt[channel] = state->icCntVal[channel][1] + 
                                                    (state->etmrModValue - state->icCntVal[channel][0]);
            }
            else /* When first capture value is less than the second capture value, if overflow n times, need to add n*mod */
            {
                state->measurementCapCnt[channel] = state->icCntVal[channel][1] - state->icCntVal[channel][0];
            }

            /* When the first capture polarity is high, it means that the edge is positive edge, so pulse is positive pulse */
            if (state->icCapPol[channel][0] == 1U)
            {
                state->measurementPosPulseCnt[channel] = state->measurementCapCnt[channel];
            }
            else /* if first capture polarity is low, it means that the edge is negative edge, so pulse is negative pulse */
            {
                state->measurementNegPulseCnt[channel] = state->measurementCapCnt[channel];
            }

            /* Process the second pulse */
            if (state->icCntVal[channel][1] > state->icCntVal[channel][2])
            {
                state->measurementCapCnt[channel] = state->icCntVal[channel][2] + 
                                                    (state->etmrModValue - state->icCntVal[channel][1]);
            }
            else
            {
                state->measurementCapCnt[channel] = state->icCntVal[channel][2] - state->icCntVal[channel][1];
            }

            if (state->icCapPol[channel][1] == 1U)
            {
                state->measurementPosPulseCnt[channel] = state->measurementCapCnt[channel];
            }
            else
            {
                state->measurementNegPulseCnt[channel] = state->measurementCapCnt[channel];
            }

            /* Calculate the period of signal according to positive pulse and negative pulse */
            state->measurementPeriod[channel] = state->measurementPosPulseCnt[channel] + 
                                                state->measurementNegPulseCnt[channel];
        }
        else /* eTMR_NEG_EDGE eTMR_POS_EDGE */
        {
            /* Use positive or negative edge to measure period, just need two edge capture value */
            if (state->icCntVal[channel][0] > state->icCntVal[channel][1])
            {
                state->measurementPeriod[channel] = state->icCntVal[channel][1] + 
                                                    (state->etmrModValue - state->icCntVal[channel][0]);
            }
            else
            {
                state->measurementPeriod[channel] = state->icCntVal[channel][1] - state->icCntVal[channel][0];
            }
        }
    }
    else /* Capture one positive pulse or one negative pulse */
    {
        if (state->captureEdge[channel] == eTMR_DUAL_EDGES)
        {
            /* When first capture value is greater than the second capture value, overflow count is 1 at least, so if overflow n times,
             * it needs to add (n-1)*mod. */
            if (state->icCntVal[channel][0] > state->icCntVal[channel][1])
            {
                state->measurementCapCnt[channel] = state->icCntVal[channel][1] + 
                                                    (state->etmrModValue - state->icCntVal[channel][0]);
            }
            /* When first capture value is less than the second capture value, overflow count is 0 at least, so if overflow n times,
               it needs to add n*mod */
            else
            {
                state->measurementCapCnt[channel] = state->icCntVal[channel][1] - state->icCntVal[channel][0];
            }

            /* According to the first capture polarity, determine positive pulse or negative pulse */
            if (state->icCapPol[channel][0] == 1U)
            {
                state->measurementPosPulseCnt[channel] = state->measurementCapCnt[channel];
            }
            else
            {
                state->measurementNegPulseCnt[channel] = state->measurementCapCnt[channel];
            }
        }
    }
}

/*! @}*/
/*******************************************************************************
* EOF
******************************************************************************/
