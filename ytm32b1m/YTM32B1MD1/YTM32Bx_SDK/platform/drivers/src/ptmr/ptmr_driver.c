/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file ptmr_driver.c
 * @version 1.4.1
 *
 * @brief pTMR Driver — implementation of the public pTMR_DRV_* API.
 *
 * This file implements the application-level pTMR driver functions declared
 * in ptmr_driver.h. Each function wraps the low-level hardware access layer
 * (ptmr_hw_access.h) and provides instance-based access to the pTMR peripheral.
 */

#include <stddef.h>
#include "ptmr_driver.h"
#include "ptmr_hw_access.h"
#include "interrupt_manager.h"
#include "clock_manager.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Table of base addresses for pTMR peripheral instances. */
static pTMR_Type * const s_ptmrBase[] = pTMR_BASE_PTRS;

/*! @brief Table of clock names for pTMR peripheral clock configuration. */
static const clock_names_t s_ptmrClkNames[pTMR_INSTANCE_COUNT] = pTMR_CLOCK_NAMES;

#if (defined(FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE) && (FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE == 1))
/*! @brief Table of IPC clock names for pTMR function clock configuration. */
static const clock_names_t s_ptmrIpcClkNames[pTMR_INSTANCE_COUNT] = pTMR_IPC_CLOCK_NAMES;
#endif /* FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE */

/*! @brief Cached functional clock frequency for each pTMR instance (Hz). */
static uint32_t s_ptmrSourceClockFrequency[pTMR_INSTANCE_COUNT] = {0};

/*! @brief Table mapping pTMR instance and channel to IRQ numbers. */
static const IRQn_Type ptmrIrqId[pTMR_INSTANCE_COUNT][pTMR_CH_COUNT] = pTMR_IRQS;

/*******************************************************************************
 * Internal Functions
 ******************************************************************************/

/*! @brief Update the cached clock frequency for the given pTMR instance. */
static void pTMR_Update_ClockFreq(uint32_t instance);

/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/

/*!
 * @brief Populate a module configuration structure with default values.
 */
void pTMR_DRV_GetDefaultConfig(ptmr_user_config_t * const config)
{
    DEV_ASSERT(config != NULL);

    config->enableRunInDebug = false;
}

/*!
 * @brief Populate a channel configuration structure with default values.
 */
void pTMR_DRV_GetDefaultChanConfig(ptmr_user_channel_config_t * const config)
{
    DEV_ASSERT(config != NULL);

    config->periodUnits = pTMR_PERIOD_UNITS_MICROSECONDS;
    config->period = 1000000U;
    config->chainChannel = false;
    config->isInterruptEnabled = true;
}

/*!
 * @brief Initialize the pTMR module.
 */
void pTMR_DRV_Init(uint32_t instance,
                   const ptmr_user_config_t *userConfig)
{
    pTMR_Type * base;

    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);
    DEV_ASSERT(userConfig != NULL);

    /* Gets current functional clock frequency of pTMR instance */
    pTMR_Update_ClockFreq(instance);
    /* When resetting the pTMR module, a delay of 4 peripheral clock cycles
        must be ensured. This peripheral clock and the core clock running the
        code could be very different, two distinct cases are identified:
         - core_clk > peripheral_clk. This requires a delay loop to be implemented,
            and the delay value based on the ratio between the two frequencies.
         - core_clk <= peripheral_clk. This requires a short delay, which is usually
            below the delay caused naturally by the read-modify-write operation.
     */
    base = s_ptmrBase[instance];
    /* Resets pTMR module */
    pTMR_Reset(base, 5);
    /* Enables functional clock of pTMR module*/
#if (defined(FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE) && (FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE == 1))
    pTMR_SetTimerClockSource(base, userConfig->useFunctionClockSource);
#endif /* FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE */
    pTMR_Enable(base, 5);
    /* Sets pTMR operation in Debug and DOZE mode*/
    pTMR_SetTimerRunInDebugCmd(base, userConfig->enableRunInDebug);
}

/*!
 * @brief De-initialize the pTMR module.
 */
void pTMR_DRV_Deinit(uint32_t instance)
{
    pTMR_Type * base;

    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);

    base = s_ptmrBase[instance];
    for (uint8_t i = 0U; i < 4U; i++)
    {
        /* Stops timer channel from counting */
        pTMR_StopTimerChannels(base, i);
        INT_SYS_DisableIRQ(ptmrIrqId[instance][i]);
    }
    /* Disables pTMR module functional clock*/
    pTMR_Disable(base);
    /* Resets pTMR module */
    pTMR_Reset(base, 5);
}

/*!
 * @brief Initialize an individual pTMR timer channel.
 */
status_t pTMR_DRV_InitChannel(uint32_t instance,
                              uint32_t channel,
                              const ptmr_user_channel_config_t * userChannelConfig)
{
    pTMR_Type * base;
    status_t reVal = STATUS_SUCCESS;

    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);
    DEV_ASSERT(userChannelConfig != NULL);
    DEV_ASSERT(channel < pTMR_CH_COUNT);

    base = s_ptmrBase[instance];

    /* Setups the timer channel chaining  */
    pTMR_SetTimerChannelChainCmd(base, channel, userChannelConfig->chainChannel);

    if (userChannelConfig->periodUnits == pTMR_PERIOD_UNITS_MICROSECONDS)
    {
        /* Setups timer channel period in microsecond unit */
        reVal = pTMR_DRV_SetTimerPeriodByUs(instance, channel, userChannelConfig->period);
    }
    else
    {
        /* Setups timer channel period in count unit */
        pTMR_DRV_SetTimerPeriodByCount(instance, channel, userChannelConfig->period);
    }

    if (reVal == STATUS_SUCCESS)
    {
        /* Setups interrupt generation for timer channel */
        if (userChannelConfig->isInterruptEnabled)
        {
            /* Enables interrupt generation */
            pTMR_EnableInterruptTimerChannels(base, channel);
            INT_SYS_EnableIRQ(ptmrIrqId[instance][channel]);
        }
        else
        {
            /* Disables interrupt generation */
            pTMR_DisableInterruptTimerChannels(base, channel);
            /* Only disable channel interrupt globally if each channel has a separate interrupt line */
#if defined(FEATURE_pTMR_HAS_NUM_IRQS_CHANS) && (FEATURE_pTMR_HAS_NUM_IRQS_CHANS == pTMR_CH_COUNT)
            INT_SYS_DisableIRQ(ptmrIrqId[instance][channel]);
#endif
        }
    }

    return reVal;
}

/*******************************************************************************
 * Timer Start & Stop
 ******************************************************************************/

/*!
 * @brief Start the specified timer channel counting.
 */
void pTMR_DRV_StartTimerChannels(uint32_t instance,
                                 uint32_t channel)
{
    pTMR_Type * base;

    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < pTMR_CH_COUNT);

    base = s_ptmrBase[instance];
    /* Starts timer channel counting */
    pTMR_StartTimerChannels(base, channel);
}

/*!
 * @brief Stop the specified timer channel counting.
 */
void pTMR_DRV_StopTimerChannels(uint32_t instance,
                                uint32_t channel)
{
    pTMR_Type * base;

    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < pTMR_CH_COUNT);

    base = s_ptmrBase[instance];
    /* Stops timer channel from counting */
    pTMR_StopTimerChannels(base, channel);
}

/*******************************************************************************
 * Timer Period
 ******************************************************************************/

/*!
 * @brief Set the timer channel period in microseconds.
 */
status_t pTMR_DRV_SetTimerPeriodByUs(uint32_t instance,
                                     uint32_t channel,
                                     uint32_t periodUs)
{
    pTMR_Type * base;
    status_t reVal = STATUS_SUCCESS;
    uint64_t count;

    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < pTMR_CH_COUNT);

    /* Gets current functional clock frequency of pTMR instance */
    pTMR_Update_ClockFreq(instance);

    base = s_ptmrBase[instance];
    /* Calculates the count value, assign it to timer channel counter register.*/
    count = ((uint64_t)periodUs) * s_ptmrSourceClockFrequency[instance];
    count = (count / 1000000U) - 1U;
    /* Checks whether the count is valid with timer channel operation mode */
    if (count > MAX_PERIOD_COUNT)
    {
        reVal = STATUS_ERROR;
    }
    if (reVal == STATUS_SUCCESS)
    {
        /* Sets the timer channel period in count unit */
        pTMR_SetTimerPeriodByCount(base, channel, (uint32_t)count);
    }
    return reVal;
}

/*!
 * @brief Set the timer channel period in raw count units.
 */
void pTMR_DRV_SetTimerPeriodByCount(uint32_t instance,
                                    uint32_t channel,
                                    uint32_t count)
{
    pTMR_Type * base;

    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < pTMR_CH_COUNT);

    base = s_ptmrBase[instance];
    /* Sets the timer channel period in count unit */
    pTMR_SetTimerPeriodByCount(base, channel, count);

}

/*!
 * @brief Get the timer channel period in microseconds.
 */
uint64_t pTMR_DRV_GetTimerPeriodByUs(uint32_t instance,
                                     uint32_t channel)
{
    const pTMR_Type * base;
    uint64_t currentPeriod;

    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < pTMR_CH_COUNT);

    /* Gets current functional clock frequency of pTMR instance */
    pTMR_Update_ClockFreq(instance);

    base = s_ptmrBase[instance];
    /* Gets current timer channel period in count.*/
    currentPeriod = pTMR_GetTimerPeriodByCount(base, channel);

    /* Converts period from count unit to microseconds unit for other modes */
    currentPeriod = ((currentPeriod + 1U) * 1000000U) / s_ptmrSourceClockFrequency[instance];

    return currentPeriod;
}

/*!
 * @brief Get the timer channel period in raw count units.
 */
uint32_t pTMR_DRV_GetTimerPeriodByCount(uint32_t instance,
                                        uint32_t channel)
{
    const pTMR_Type * base;
    uint32_t currentPeriod;

    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < pTMR_CH_COUNT);

    base = s_ptmrBase[instance];
    /* Gets current timer channel period by count.*/
    currentPeriod = pTMR_GetTimerPeriodByCount(base, channel);

    return currentPeriod;
}

/*!
 * @brief Get the current timer channel counter value in microseconds.
 */
uint64_t pTMR_DRV_GetCurrentTimerUs(uint32_t instance,
                                    uint32_t channel)
{
    const pTMR_Type * base;
    uint64_t currentTime = 0U;

    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < pTMR_CH_COUNT);

    /* Gets current functional clock frequency of pTMR instance */
    pTMR_Update_ClockFreq(instance);

    base = s_ptmrBase[instance];
    /* Gets current timer channel counting value */
    currentTime = pTMR_GetCurrentTimerCount(base, channel);

    /* Converts counting value to microseconds unit for other modes */
    currentTime = (currentTime * 1000000U) / s_ptmrSourceClockFrequency[instance];

    return currentTime;
}

/*!
 * @brief Get the current timer channel counter value in raw counts.
 */
uint32_t pTMR_DRV_GetCurrentTimerCount(uint32_t instance,
                                       uint32_t channel)
{
    const pTMR_Type * base;
    uint32_t currentTime;

    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < pTMR_CH_COUNT);

    base = s_ptmrBase[instance];
    /* Gets current timer channel counting value */
    currentTime = pTMR_GetCurrentTimerCount(base, channel);

    return currentTime;
}

/*******************************************************************************
 * Interrupt Management
 ******************************************************************************/

/*!
 * @brief Enable interrupt generation for the specified timer channel.
 */
void pTMR_DRV_EnableTimerChannelInterrupt(uint32_t instance,
                                          uint32_t channel)
{
    pTMR_Type * base;

    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < pTMR_CH_COUNT);

    base = s_ptmrBase[instance];
    /* Enable interrupt of timer channels */
    pTMR_EnableInterruptTimerChannels(base, channel);
}

/*!
 * @brief Disable interrupt generation for the specified timer channel.
 */
void pTMR_DRV_DisableTimerChannelInterrupt(uint32_t instance,
                                           uint32_t channel)
{
    pTMR_Type * base;

    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < pTMR_CH_COUNT);

    base = s_ptmrBase[instance];
    /* Disable interrupt of timer channels */
    pTMR_DisableInterruptTimerChannels(base, channel);
}

/*!
 * @brief Get the interrupt flag status of the specified timer channel.
 */
uint32_t pTMR_DRV_GetInterruptFlagTimerChannels(uint32_t instance,
                                                uint32_t channel)
{
    const pTMR_Type * base;

    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < pTMR_CH_COUNT);

    base = s_ptmrBase[instance];
    /* Gets the interrupt flag for timer channels */
    return pTMR_GetInterruptFlagTimerChannels(base, channel);
}

/*!
 * @brief Clear the interrupt flag of the specified timer channel.
 */
void pTMR_DRV_ClearInterruptFlagTimerChannels(uint32_t instance,
                                              uint32_t channel)
{
    pTMR_Type * base;

    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < pTMR_CH_COUNT);

    base = s_ptmrBase[instance];
    /* Clears the interrupt flag for timer channels */
    pTMR_ClearInterruptFlagTimerChannels(base, channel);
}

/*******************************************************************************
 * Internal Functions
 ******************************************************************************/

/*!
 * @brief Update the cached clock frequency for the given pTMR instance.
 */
static void pTMR_Update_ClockFreq(uint32_t instance)
{
    DEV_ASSERT(instance < pTMR_INSTANCE_COUNT);
    status_t clkErr;
#if (defined(FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE) && (FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE == 1))
    pTMR_Type *base;
    uint32_t function_clk_freq;
    base = s_ptmrBase[instance];
#endif /* FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE */
    /* Gets current functional clock frequency of pTMR instance */
    clkErr = CLOCK_SYS_GetFreq(s_ptmrClkNames[instance], &s_ptmrSourceClockFrequency[instance]);
    /* Checks the functional clock of pTMR module */
    DEV_ASSERT(clkErr == STATUS_SUCCESS);
    (void)clkErr;
#if (defined(FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE) && (FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE == 1))
    /* Get function clock frequency */
    clkErr = CLOCK_SYS_GetFreq(s_ptmrIpcClkNames[instance], &function_clk_freq);
    DEV_ASSERT(clkErr == STATUS_SUCCESS);
    (void)clkErr;
    /* Check if pTMR using function clock */
    if (pTMR_GetTimerClockSource(base))
    {
        /* Function clock should be less than pclk freq / 4 */
        if (function_clk_freq > (s_ptmrSourceClockFrequency[instance] / 4U))
        {
            s_ptmrSourceClockFrequency[instance] = 0;
        }
        else
        {
            s_ptmrSourceClockFrequency[instance] = function_clk_freq;
        }
    }
#endif /* FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE */
    DEV_ASSERT(s_ptmrSourceClockFrequency[instance] > 0U);
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
