/*
 * Copyright 2020-2023 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file mpwm_ct_driver.c
 * @version 1.4.0
 */

#include "mpwm_ct_driver.h"
#include "mpwm_hw_access.h"

/*******************************************************************************
 * API Definition
 ******************************************************************************/

/*!
 * @brief Initialize MPWM channel common timer mode
 */
status_t MPWM_DRV_InitTimer(uint32_t instance, const mpwm_ct_config_t *config)
{
    DEV_ASSERT(instance < MPWM_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    MPWM_Type *base = g_mpwmBase[instance];
    mpwm_state_t *state = g_mpwmState[instance];
    uint8_t channel;
    mpwm_ch_reload_src_t rldSrc;
    bool hwTrigEn;
    bool dbgMode;
    mpwm_ch_clk_prs_t prs;
    uint8_t cntMode;
    bool tofIntEn;
    uint16_t finalVal;
    uint8_t index;

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->timerChannelConfig[index].hwChannelId;
        rldSrc = config->timerChannelConfig[index].reloadSrc;
        hwTrigEn = config->timerChannelConfig[index].hwTrigEn;
        dbgMode = config->timerChannelConfig[index].debugMode;
        prs = config->timerChannelConfig[index].clkPrs;
        cntMode = (uint8_t)(config->timerChannelConfig[index].countMode);
        tofIntEn = config->timerChannelConfig[index].timerOverflowIntEn;
        finalVal = config->timerChannelConfig[index].finalValue;

        /* Set reload source */
        MPWM_SetReloadSrc(base, channel, (uint8_t)rldSrc);

        /* Set hardware trigger enable or not */
        MPWM_SetHwTrigEnState(base, channel, hwTrigEn);

        /* Set debug mode disable or not */
        MPWM_SetDebugMode(base, channel, dbgMode);

        /* Set channel counter clock prescaler */
        MPWM_SetClkPrs(base, channel, (uint8_t)prs);

        state->mpwmChFreq[channel] = (state->mpwmClkSrcFreq / ((uint32_t)1U << (uint8_t)prs));

        /* Set channel counter continuous mode or one shot mode */
        MPWM_SetCountMode(base, channel, cntMode);

        if (tofIntEn)
        {
            /* Set channel timer overflow interrupt enable or not */
            MPWM_SetChannelTofIntEnState(base, channel, tofIntEn);

            /* Enable channel overflow interrupt IRQ */
            INT_SYS_EnableIRQ(g_mpwmOvfIrqId[instance][channel]);
        }

        /* Set channel counter max value */
        MPWM_SetChannelPeriod(base, channel, finalVal);

        state->mpwmChPeriodTicks[channel] = finalVal;

        /* Set channel as common timer mode */
        MPWM_SetChannelMode(base, channel, (uint8_t)MPWM_COMMON_TIMER_MODE);
    }

    return STATUS_SUCCESS;
}

/*!
 * @brief Deinitialize MPWM channel common timer mode
 */
status_t MPWM_DRV_DeinitTimer(uint32_t instance, const mpwm_ct_config_t *config)
{
    DEV_ASSERT(instance < MPWM_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    MPWM_Type *base = g_mpwmBase[instance];
    uint16_t channelSet = 0;
    uint8_t channel;
    uint8_t index;

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->timerChannelConfig[index].hwChannelId;

        /* Get channels that need to disable */
        channelSet |= ((uint16_t)0x1U << channel);
    }

    /* Stop channel counter */
    (void)MPWM_DRV_StopTimer(instance, channelSet);

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->timerChannelConfig[index].hwChannelId;

        /* Not disable debug mode */
        MPWM_SetDebugMode(base, channel, false);

        /* Set reload source */
        MPWM_SetReloadSrc(base, channel, (uint8_t)MPWM_RELOAD_ON_PERIOD);

        /* Set hardware trigger enable or not */
        MPWM_SetHwTrigEnState(base, channel, false);

        /* Set channel no counter clock prescaler */
        MPWM_SetClkPrs(base, channel, (uint8_t)MPWM_CLK_DIV_BY_1);

        /* Set channel counter continuous mode */
        MPWM_SetCountMode(base, channel, (uint8_t)MPWM_CONTINUOUS_MODE);

        /* Set channel timer overflow interrupt disable */
        MPWM_SetChannelTofIntEnState(base, channel, false);

        /* Clear channel period register */
        MPWM_SetChannelPeriod(base, channel, 0);
    }

    return STATUS_SUCCESS;
}

/*!
 * @brief Start MPWM channel counter for common timer
 */
status_t MPWM_DRV_StartTimer(uint32_t instance, uint16_t channelSet)
{
    DEV_ASSERT(instance < MPWM_INSTANCE_COUNT);

    MPWM_Type *base = g_mpwmBase[instance];
    MPWM_EnableCounter(base, channelSet);

    return STATUS_SUCCESS;
}

/*!
 * @brief Stop MPWM channel counter for common timer
 */
status_t MPWM_DRV_StopTimer(uint32_t instance, uint16_t channelSet)
{
    DEV_ASSERT(instance < MPWM_INSTANCE_COUNT);

    MPWM_Type *base = g_mpwmBase[instance];
    MPWM_DisableCounter(base, channelSet);

    return STATUS_SUCCESS;
}

/*******************************************************************************
* EOF
*******************************************************************************/
