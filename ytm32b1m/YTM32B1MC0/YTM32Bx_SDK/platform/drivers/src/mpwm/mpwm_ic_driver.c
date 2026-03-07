/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file mpwm_ic_driver.c
 * @version 1.4.0
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 2985 Rule 2.2: This operation is redundant. The value of the result is
 *                       always that of the left-hand operand.
 */

#include "mpwm_ic_driver.h"
#include "mpwm_hw_access.h"

/*******************************************************************************
 * API Definition
 ******************************************************************************/

/*!
 * @brief Initialize MPWM input capture mode
 */
status_t MPWM_DRV_InitInputCapture(uint32_t instance,
                                   const mpwm_ic_config_t *config)
{
    DEV_ASSERT(instance < MPWM_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    MPWM_Type *base = g_mpwmBase[instance];
    status_t status = STATUS_SUCCESS;
    mpwm_state_t *state = g_mpwmState[instance];
    uint8_t index;
    uint8_t channel;
#if FEATURE_MPWM_HAS_COMBINATION_CAPTURE
    uint8_t channelPair;
    uint32_t channelPairCombEnSet = 0;
    uint32_t channelPairCombSrcSet = 0;
#endif
    bool dbgMode;
    bool autoClrEn;
    bool overwriteEn;
    mpwm_ic_edge_t edge;
    bool dmaEn;
    mpwm_ch_count_mode_t cntMode;
    mpwm_ch_clk_prs_t prs;
    uint16_t maxCntVal;

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->inputCaptureChannelConfig[index].hwChannelId;
        prs = config->inputCaptureChannelConfig[index].clkPrs;
        maxCntVal = config->inputCaptureChannelConfig[index].maxCountValue;
        dbgMode = config->inputCaptureChannelConfig[index].debugMode;
        autoClrEn = config->inputCaptureChannelConfig[index].cntAutoClr;
        overwriteEn = config->inputCaptureChannelConfig[index].overwrite;
        edge = config->inputCaptureChannelConfig[index].edge;
        dmaEn = config->inputCaptureChannelConfig[index].dmaEn;
        cntMode = config->inputCaptureChannelConfig[index].countMode;

        /* Set channel counter clock prescaler */
        MPWM_SetClkPrs(base, channel, (uint8_t)prs);

        /* Get channel counter frequency */
        state->mpwmChFreq[channel] = (state->mpwmClkSrcFreq / ((uint32_t)1U << (uint8_t)prs));

        /* Set debug mode disable or not */
        MPWM_SetDebugMode(base, channel, dbgMode);

        /* Set auto clear enable or not */
        MPWM_SetAutoClearEnState(base, channel, autoClrEn);

        /* Set overwrite enable or not */
        MPWM_SetOverWriteEnState(base, channel, overwriteEn);

        /* Set capture edge */
        MPWM_SetCapEdge(base, channel, (uint8_t)edge);

        /* Set dma enable or not */
        MPWM_SetDmaEnState(base, channel, dmaEn);

        /* Set channel counter continuous mode or one shot mode */
        MPWM_SetCountMode(base, channel, (uint8_t)cntMode);

        /* Set channel maximum counter value */
        MPWM_SetChannelPeriod(base, channel, maxCntVal);

        /* Get maximum counter value */
        state->mpwmChPeriodTicks[channel] = maxCntVal;
    }

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->inputCaptureChannelConfig[index].hwChannelId;

        /* Set channel as input capture mode */
        MPWM_SetChannelMode(base, channel, (uint8_t)MPWM_INPUT_CAPTURE_MODE);

        /* Enable channel interrupt */
        if (config->inputCaptureChannelConfig[index].intEn)
        {
            MPWM_SetChannelIntEnState(base, channel, true);
            /* Enable channel IRQ */
            INT_SYS_EnableIRQ(g_mpwmChnIrqId[instance][channel]);
        }

        /* Enable channel overflow interrupt */
        if (config->inputCaptureChannelConfig[index].timerOverflowIntEn)
        {
            MPWM_SetChannelTofIntEnState(base, channel, true);
            /* Enable channel overflow IRQ */
            INT_SYS_EnableIRQ(g_mpwmOvfIrqId[instance][channel]);
        }

        state->mpwmChMode[channel] = MPWM_INPUT_CAPTURE_MODE;
    }

#if FEATURE_MPWM_HAS_COMBINATION_CAPTURE
    for (index = 0; index < config->channelPairNum; index++)
    {
        channelPair = config->inputCaptureChannelPairConfig[index].hwChannelPairId;
        if (config->inputCaptureChannelPairConfig[index].combEn)
        {
            channelPairCombEnSet |= ((uint32_t)0x1U << channelPair);
            channelPairCombSrcSet |= (((uint32_t)config->inputCaptureChannelPairConfig[index].combSrc << 16U) << channelPair); /*PRQA S 2985*/
        }
    }

    /* Set combination source */
    MPWM_SetChPairCombSrc(base, channelPairCombSrcSet);

    /* Set combination enable */
    MPWM_SetChPairCombEnState(base, channelPairCombEnSet);
#endif

    return status;
}

/*!
 * @brief Deinitialize MPWM input capture mode
 */
status_t MPWM_DRV_DeinitInputCapture(uint32_t instance,
                                     const mpwm_ic_config_t *config)
{
    DEV_ASSERT(instance < MPWM_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    MPWM_Type *base = g_mpwmBase[instance];
    uint8_t index;
    uint8_t channel;
    uint16_t channelSet = 0;

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->inputCaptureChannelConfig[index].hwChannelId;

        channelSet |= ((uint16_t)0x1U << channel);
    }

    MPWM_DisableCounter(base, channelSet);

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->inputCaptureChannelConfig[index].hwChannelId;

        MPWM_SetClkPrs(base, channel, (uint8_t)MPWM_CLK_DIV_BY_1);

        MPWM_SetChannelMode(base, channel, (uint8_t)MPWM_INPUT_CAPTURE_MODE);

        /* Set debug mode disable or not */
        MPWM_SetDebugMode(base, channel, false);

        /* Set auto clear enable or not */
        MPWM_SetAutoClearEnState(base, channel, false);

        /* Set overwrite enable or not */
        MPWM_SetOverWriteEnState(base, channel, false);

        /* Set capture edge */
        MPWM_SetCapEdge(base, channel, (uint8_t)MPWM_CAP_DISABLE);

        /* Set dma enable or not */
        MPWM_SetDmaEnState(base, channel, false);

        /* Set channel counter continuous mode or one shot mode */
        MPWM_SetCountMode(base, channel, (uint8_t)MPWM_CONTINUOUS_MODE);

        /* Clear channel period ticks */
        MPWM_SetChannelPeriod(base, channel, 0);

        /* Disable channel interrupt */
        MPWM_SetChannelIntEnState(base, channel, false);

        /* Disable channel IRQ */
        INT_SYS_DisableIRQ(g_mpwmChnIrqId[instance][channel]);
    }

    return STATUS_SUCCESS;
}

/*******************************************************************************
* EOF
*******************************************************************************/
