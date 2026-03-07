/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file mpwm_pc_driver.c
 * @version 1.4.0
 */

#include "mpwm_pc_driver.h"
#include "mpwm_hw_access.h"

/*******************************************************************************
 * API Definition
 ******************************************************************************/

/*!
 * @brief Initialize MPWM pulse counter mode
 */
status_t MPWM_DRV_InitPulseCounter(uint32_t instance,
                                   const mpwm_pc_config_t *config)
{
    DEV_ASSERT(instance < MPWM_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    MPWM_Type *base = g_mpwmBase[instance];
    status_t status = STATUS_SUCCESS;
    mpwm_state_t *state = g_mpwmState[instance];
    uint8_t index;
    uint8_t channel;
    mpwm_ch_clk_prs_t prs;
    uint16_t maxPulseNum;
    uint16_t cmpPulseNum;
    mpwm_ch_reload_src_t rldSrc;
    bool dbgMode;
    mpwm_pc_edge_t edge;
    bool dmaEn;
    mpwm_ch_count_mode_t cntMode;
    bool intEn;
    bool tofIntEn;

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->pulseCounterChannelConfig[index].hwChannelId;
        prs = config->pulseCounterChannelConfig[index].clkPrs;
        maxPulseNum = config->pulseCounterChannelConfig[index].maxPulseNum;
        cmpPulseNum = config->pulseCounterChannelConfig[index].cmpPulseNum;
        rldSrc = config->pulseCounterChannelConfig[index].reloadSrc;
        dbgMode = config->pulseCounterChannelConfig[index].debugMode;
        edge = config->pulseCounterChannelConfig[index].edge;
        dmaEn = config->pulseCounterChannelConfig[index].dmaEn;
        cntMode = config->pulseCounterChannelConfig[index].countMode;

        /* Set channel counter clock prescaler */
        MPWM_SetClkPrs(base, channel, (uint8_t)prs);

        /* Get channel counter frequency */
        state->mpwmChFreq[channel] = (state->mpwmClkSrcFreq / (uint8_t)prs);

        /* Set reload source */
        MPWM_SetReloadSrc(base, channel, (uint8_t)rldSrc);

        /* Set debug mode disable or not */
        MPWM_SetDebugMode(base, channel, dbgMode);

        /* Set capture edge */
        MPWM_SetCapEdge(base, channel, (uint8_t)edge);

        /* Set dma enable or not */
        MPWM_SetDmaEnState(base, channel, dmaEn);

        /* Set channel counter continuous mode or one shot mode */
        MPWM_SetCountMode(base, channel, (uint8_t)cntMode);

        /* Set channel maximum counter value */
        MPWM_SetChannelPeriod(base, channel, maxPulseNum);

        /* Set channel compare */
        MPWM_SetChannelCompare(base, channel, cmpPulseNum);

        /* Get maximum counter value */
        state->mpwmChPeriodTicks[channel] = maxPulseNum;
    }

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->pulseCounterChannelConfig[index].hwChannelId;

        /* Set channel as pulse counter mode */
        MPWM_SetChannelMode(base, channel, (uint8_t)MPWM_PULSE_COUNTER_MODE);

        state->mpwmChMode[channel] = MPWM_PULSE_COUNTER_MODE;
    }

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->pulseCounterChannelConfig[index].hwChannelId;
        intEn = config->pulseCounterChannelConfig[index].intEn;
        tofIntEn = config->pulseCounterChannelConfig[index].timerOverflowIntEn;

        if (intEn)
        {
            /* Set interrupt enable */
            MPWM_SetChannelIntEnState(base, channel, intEn);
            INT_SYS_EnableIRQ(g_mpwmChnIrqId[instance][channel]);
        }

        if (tofIntEn)
        {
            /* Set timer overflow interrupt enable */
            MPWM_SetChannelTofIntEnState(base, channel, tofIntEn);
            INT_SYS_EnableIRQ(g_mpwmOvfIrqId[instance][channel]);
        }
    }

    return status;
}

/*!
 * @brief Deinitialize MPWM pulse counter mode
 */
status_t MPWM_DRV_DeinitPulseCounter(uint32_t instance,
                                     const mpwm_pc_config_t *config)
{
    DEV_ASSERT(instance < MPWM_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    MPWM_Type *base = g_mpwmBase[instance];
    uint8_t index;
    uint8_t channel;
    uint16_t channelSet = 0;

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->pulseCounterChannelConfig[index].hwChannelId;

        channelSet |= ((uint16_t)0x1U << channel);
    }

    MPWM_DisableCounter(base, channelSet);

    for (index = 0; index < config->channelNum; index++)
    {
        channel = config->pulseCounterChannelConfig[index].hwChannelId;

        MPWM_SetClkPrs(base, channel, (uint8_t)MPWM_CLK_DIV_BY_1);

        MPWM_SetChannelMode(base, channel, (uint8_t)MPWM_PULSE_COUNTER_MODE);

        /* Set debug mode disable or not */
        MPWM_SetDebugMode(base, channel, false);

        /* Set reload source */
        MPWM_SetReloadSrc(base, channel, (uint8_t)MPWM_RELOAD_DISABLE);

        /* Set capture edge */
        MPWM_SetCapEdge(base, channel, (uint8_t)MPWM_PULSE_DISABLE);

        /* Set dma enable or not */
        MPWM_SetDmaEnState(base, channel, false);

        /* Clear channel period ticks */
        MPWM_SetChannelPeriod(base, channel, 0);

        /* Set channel compare */
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

/*******************************************************************************
* EOF
*******************************************************************************/
