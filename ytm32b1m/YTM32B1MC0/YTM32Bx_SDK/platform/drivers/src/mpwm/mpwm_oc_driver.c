/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file mpwm_oc_driver.c
 * @version 1.4.0
 */

#include "mpwm_oc_driver.h"
#include "mpwm_hw_access.h"

/*******************************************************************************
 * API Definition
 ******************************************************************************/

/*!
 * @brief Initialize MPWM output compare mode
 */
status_t MPWM_DRV_InitOutputCompare(uint32_t instance,
                                    const mpwm_oc_config_t *config)
{
    DEV_ASSERT(instance < MPWM_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    MPWM_Type *base = g_mpwmBase[instance];
    status_t status = STATUS_SUCCESS;
    mpwm_state_t *state = g_mpwmState[instance];
    uint16_t outputInitSet = 0;
    uint8_t index;
    uint8_t channel;
    mpwm_ch_reload_src_t rldSrc;
    bool dbgMode;
    mpwm_ch_clk_prs_t prs;
    uint16_t maxCntVal;
    uint16_t cmpVal;
    mpwm_oc_mode_t cmpOut;
    bool perOutputTrigEn;
    bool cmpOutputTrigEn;
    bool intEn;
    bool tofIntEn;

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->outputCompareChannelConfig[index].hwChannelId;
        intEn = config->outputCompareChannelConfig[index].intEn;
        tofIntEn = config->outputCompareChannelConfig[index].timerOverflowIntEn;

        outputInitSet |= (uint16_t)config->outputCompareChannelConfig[index].outputInit << channel;

        if (intEn)
        {
            /* Set interrupt enable */
            MPWM_SetChannelIntEnState(base, channel, intEn);
            /* Enable channel interrupt IRQ */
            INT_SYS_EnableIRQ(g_mpwmChnIrqId[instance][channel]);
        }

        if (tofIntEn)
        {
            /* Set timer overflow interrupt enable */
            MPWM_SetChannelTofIntEnState(base, channel, tofIntEn);
            /* Enable overflow interrupt IRQ */
            INT_SYS_EnableIRQ(g_mpwmOvfIrqId[instance][channel]);
        }
    }

    /* Set output initial value together */
    MPWM_SetOutputInit(base, outputInitSet);

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->outputCompareChannelConfig[index].hwChannelId;
        prs = config->outputCompareChannelConfig[index].clkPrs;
        maxCntVal = config->outputCompareChannelConfig[index].maxCountValue;
        cmpVal = config->outputCompareChannelConfig[index].compareValue;
        cmpOut = config->outputCompareChannelConfig[index].compareOutput;
        dbgMode = config->outputCompareChannelConfig[index].debugMode;
        rldSrc = config->outputCompareChannelConfig[index].reloadSrc;
        perOutputTrigEn =
            config->outputCompareChannelConfig[index].outputTrigConfig.perTrigOutputEn;
        cmpOutputTrigEn =
            config->outputCompareChannelConfig[index].outputTrigConfig.cmpTrigOutputEn;

        /* Set channel counter clock prescaler */
        MPWM_SetClkPrs(base, channel, (uint8_t)prs);

        /* Get channel counter frequency */
        state->mpwmChFreq[channel] = (state->mpwmClkSrcFreq / ((uint32_t)1U << (uint8_t)prs));

        /* Set reload source */
        MPWM_SetReloadSrc(base, channel, (uint8_t)rldSrc);

        /* Set debug mode disable or not */
        MPWM_SetDebugMode(base, channel, dbgMode);

        /* Set period output trigger enable or not */
        MPWM_SetPerOutputTrigEnState(base, channel, perOutputTrigEn);

        /* Set compare output trigger enable or not */
        MPWM_SetCmpOutputTrigEnState(base, channel, cmpOutputTrigEn);

        /* Set channel maximum counter value */
        MPWM_SetChannelPeriod(base, channel, maxCntVal);

        state->mpwmChPeriodTicks[channel] = maxCntVal;

        /* Set channel compare value */
        MPWM_SetChannelCompare(base, channel, cmpVal);

        /* Set channel output compare mode */
        MPWM_SetCmpMode(base, channel, (uint8_t)cmpOut);

    }

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->outputCompareChannelConfig[index].hwChannelId;

        state->mpwmChMode[channel] = MPWM_OUTPUT_COMPARE_MODE;

        /* Set channel as output compare mode */
        MPWM_SetChannelMode(base, channel, (uint8_t)MPWM_OUTPUT_COMPARE_MODE);
    }

    return status;
}

/*!
 * @brief Deinitialize MPWM output compare mode
 */
status_t MPWM_DRV_DeinitOutputCompare(uint32_t instance,
                                      const mpwm_oc_config_t *config)
{
    DEV_ASSERT(instance < MPWM_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    MPWM_Type *base = g_mpwmBase[instance];
    uint8_t index;
    uint8_t channel;
    uint16_t channelSet = 0;
    uint16_t outputInitSet = 0;

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->outputCompareChannelConfig[index].hwChannelId;

        /* Set compare output is 0 */
        MPWM_SetCmpMode(base, channel, (uint8_t)MPWM_OUTPUT_CLR);

        channelSet |= ((uint16_t)0x1U << channel);

        outputInitSet &= ~((uint16_t)0x1U << channel);
    }

    MPWM_SetOutputInit(base, outputInitSet);

    MPWM_DisableCounter(base, channelSet);

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->outputCompareChannelConfig[index].hwChannelId;

        MPWM_SetClkPrs(base, channel, (uint8_t)MPWM_CLK_DIV_BY_1);

        /* Set reload source */
        MPWM_SetReloadSrc(base, channel, (uint8_t)MPWM_RELOAD_ON_PERIOD);

        /* Set hardware trigger enable or not */
        MPWM_SetHwTrigEnState(base, channel, false);

        /* Set debug mode disable or not */
        MPWM_SetDebugMode(base, channel, false);

        /* Set period output trigger enable or not */
        MPWM_SetPerOutputTrigEnState(base, channel, false);

        /* Set compare output trigger enable or not */
        MPWM_SetCmpOutputTrigEnState(base, channel, false);

        /* Clear channel period ticks */
        MPWM_SetChannelPeriod(base, channel, 0);

        /* Clear channel compare ticks */
        MPWM_SetChannelCompare(base, channel, 0);

        /* Disable channel interrupt */
        MPWM_SetChannelIntEnState(base, channel, false);

        /* Disable channel timer overflow interrupt */
        MPWM_SetChannelTofIntEnState(base, channel, false);

        /* Disable channel IRQ */
        INT_SYS_DisableIRQ(g_mpwmChnIrqId[instance][channel]);
		INT_SYS_DisableIRQ(g_mpwmOvfIrqId[instance][channel]);
    }

    return STATUS_SUCCESS;
}

/*!
 * @brief Update output compare configuration parameters for channel
 */
status_t MPWM_DRV_UpdateOutputCompareChannel(uint32_t instance,
                                             uint8_t channel,
                                             mpwm_oc_mode_t mode,
                                             uint16_t newCmpVal,
                                             uint16_t newPerVal)
{
    DEV_ASSERT(instance < MPWM_INSTANCE_COUNT);
    DEV_ASSERT(channel < MPWM_CH_COUNT);

    MPWM_Type *base = g_mpwmBase[instance];

    MPWM_SetCmpMode(base, channel, (uint8_t)mode);

    MPWM_ClearLdok(base, channel);

    MPWM_SetChannelPeriod(base, channel, newPerVal);

    MPWM_SetChannelCompare(base, channel, newCmpVal);

    return STATUS_SUCCESS;
}

/*******************************************************************************
* EOF
*******************************************************************************/
