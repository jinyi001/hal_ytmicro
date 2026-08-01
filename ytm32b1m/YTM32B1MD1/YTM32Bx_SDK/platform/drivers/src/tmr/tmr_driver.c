/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
/*!
 * @file tmr_driver.c
 * @version 1.4.1
 *
 * @brief TMR Driver — implementation of the public TMR_DRV_* API.
 *
 * This file implements the application-level TMR driver declared in
 * tmr_driver.h. The driver resolves each instance base address and programs
 * the TMR registers directly because this module does not use a standalone
 * hardware-access layer.
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 2985 Rule 2.2 : This operation is redundant. The value of the result is always
 *                        that of the left-hand operand.
 */

#include <stddef.h>
#include "tmr_driver.h"
#include "clock_manager.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief Maximum value accepted by the 32-bit compare register. */
#define TMR_COMPARE_MAX (0xFFFFFFFFU)

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*! @brief Table of base addresses for TMR peripheral instances. */
static TMR_Type *const s_tmrBase[TMR_INSTANCE_COUNT] = TMR_BASE_PTRS;
/*! @brief Cached functional clock frequency for each TMR instance (Hz). */
static uint32_t s_tmrClockSrcFreq[TMR_INSTANCE_COUNT] = {0};

/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/

/*!
 * @brief Initialize one TMR instance with module-level counter settings.
 */
void TMR_DRV_Init(const uint32_t instance,
                  const tmr_config_t *const config)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    TMR_Type *const base = s_tmrBase[instance];

    /* Program the counter prescaler. */
    base->PRS = TMR_PRS_PRS(config->clockPrescaler); /*PRQA S 2985*/
    /* Configure whether the counter stops during debug halt. */
    base->CTRL = TMR_CTRL_DBGDIS(config->stopInDebugMode ? 1UL : 0UL); /*PRQA S 2985*/
    /* Seed the shared counter start value. */
    base->CNT = config->startValue;
}

/*!
 * @brief Reset one TMR instance and all compare channels to their reset state.
 */
void TMR_DRV_Deinit(const uint32_t instance)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    uint8_t i;
    TMR_Type *const base = s_tmrBase[instance];

    /* Stop the shared counter and clear its current value. */
    base->CTRL = 0x0U;
    base->CNT = 0x0U;

    /* Restore every compare channel to its reset configuration. */
    for (i = 0; i < TMR_CH_COUNT; i++)
    {
        base->CH[i].CTRL = 0x0U;
        base->CH[i].INT = 0x1U;
        base->CH[i].CMP = 0x0U;
    }
}

/*!
 * @brief Populate a TMR module configuration structure with safe defaults.
 */
void TMR_DRV_GetDefaultConfig(tmr_config_t *const config)
{
    DEV_ASSERT(config != NULL);

    /* Divide the module clock by 1. */
    config->clockPrescaler = 0U;
    /* Let the counter continue running while debugging. */
    config->stopInDebugMode = false;
    /* Start counting from zero. */
    config->startValue = 0U;
}

/*******************************************************************************
 * Channel Compare Control
 ******************************************************************************/

/*!
 * @brief Initialize one compare channel from a configuration structure.
 */
void TMR_DRV_InitChannel(const uint32_t instance,
                         const tmr_channel_config_t *const config)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);
    DEV_ASSERT(config->channel < TMR_CH_COUNT);

    TMR_Type *const base = s_tmrBase[instance];

    /* Program the compare value for the selected channel. */
    base->CH[config->channel].CMP = config->compareValue;

    /* Enable channel interrupt generation when requested by the caller. */
#if (defined(FEATURE_TMR_HAS_INTERRUPT_ENABLE_CONFIG) && (FEATURE_TMR_HAS_INTERRUPT_ENABLE_CONFIG == 1))
    if (config->interruptEnable)
    {
        base->CH[config->channel].CTRL |= TMR_CH_CTRL_CHIE_MASK;
    }
#endif
    /* Enable the compare channel. */
    base->CH[config->channel].CTRL |= TMR_CH_CTRL_CHEN_MASK;
}

/*!
 * @brief Program a compare value and enable one channel.
 */
void TMR_DRV_ConfigChannel(const uint32_t instance,
                           const uint8_t channel,
                           const uint32_t compareValue)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < TMR_CH_COUNT);

    TMR_Type *const base = s_tmrBase[instance];

    /* Program the new compare value. */
    base->CH[channel].CMP = compareValue;
    /* Enable the selected compare channel. */
    base->CH[channel].CTRL = TMR_CH_CTRL_CHEN_MASK;
}

/*!
 * @brief Enable one compare channel without changing its compare value.
 */
void TMR_DRV_EnableChannel(const uint32_t instance,
                           const uint8_t channel)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < TMR_CH_COUNT);

    TMR_Type *const base = s_tmrBase[instance];

    /* Enable the selected compare channel. */
    base->CH[channel].CTRL |= TMR_CH_CTRL_CHEN_MASK;
}

/*!
 * @brief Disable one compare channel.
 */
void TMR_DRV_DisableChannel(const uint32_t instance,
                            const uint8_t channel)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < TMR_CH_COUNT);

    TMR_Type *const base = s_tmrBase[instance];

    /* Disable the selected compare channel. */
    base->CH[channel].CTRL &= ~TMR_CH_CTRL_CHEN_MASK;
}

/*******************************************************************************
 * Counter Control
 ******************************************************************************/

/*!
 * @brief Write a new start value into the shared counter register.
 */
void TMR_DRV_SetStartValueCount(const uint32_t instance,
                                const uint32_t startValue)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);

    TMR_Type *const base = s_tmrBase[instance];

    /* Seed the shared counter register with a new start value. */
    base->CNT = startValue;
}

/*!
 * @brief Start the shared TMR counter.
 */
void TMR_DRV_StartTimer(const uint32_t instance)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);

    TMR_Type *const base = s_tmrBase[instance];

    /* Enable counting on the shared counter. */
    base->CTRL |= TMR_CTRL_TEN_MASK;
}

/*!
 * @brief Stop the shared TMR counter.
 */
void TMR_DRV_StopTimer(const uint32_t instance)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);

    TMR_Type *const base = s_tmrBase[instance];

    /* Disable counting on the shared counter. */
    base->CTRL &= ~TMR_CTRL_TEN_MASK;
}

/*!
 * @brief Read the current value of the shared TMR counter.
 */
uint32_t TMR_DRV_GetCounterValue(const uint32_t instance)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);

    const TMR_Type *const base = s_tmrBase[instance];

    /* Return a snapshot of the current counter value. */
    return base->CNT;
}

/*******************************************************************************
 * Time Conversion & Scheduling
 ******************************************************************************/

/*!
 * @brief Convert a microsecond interval to TMR ticks.
 */
status_t TMR_DRV_ComputeTicksByUs(const uint32_t instance,
                                  const uint32_t periodUs,
                                  uint32_t *ticks)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    const TMR_Type *const base = s_tmrBase[instance];
    status_t retStatus = STATUS_SUCCESS;
    status_t clkErr;
    uint32_t clkPrescaler;
    uint64_t tempTicks;
    uint32_t clkSelect;

    const clock_names_t s_tmrClkNames[] = TMR_CLOCK_NAMES;
    clkSelect = 0x0U;

    /* Read the active prescaler from hardware and convert it to a divisor. */
    clkPrescaler = ((base->PRS & TMR_PRS_PRS_MASK) >> TMR_PRS_PRS_SHIFT) + 1U;
    /* Query the functional clock used by the TMR instance. */
    clkErr = CLOCK_SYS_GetFreq(s_tmrClkNames[clkSelect], &s_tmrClockSrcFreq[instance]);
    /* Validate the returned clock information before using it. */
    (void) clkErr;
    DEV_ASSERT(clkErr == STATUS_SUCCESS);
    DEV_ASSERT(s_tmrClockSrcFreq[instance] > 0U);

    /* Convert microseconds to timer ticks. */
    tempTicks = (((uint64_t) periodUs * s_tmrClockSrcFreq[instance]) / clkPrescaler) / 1000000U;

    if (tempTicks > TMR_COMPARE_MAX)
    {
        /* The converted value does not fit in the compare register. */
        retStatus = STATUS_ERROR;
    } else
    {
        /* Return the converted compare increment to the caller. */
        *ticks = (uint32_t) tempTicks;
    }

    return retStatus;
}

/*!
 * @brief Move a compare channel forward by a relative number of ticks.
 */
void TMR_DRV_IncrementTicks(const uint32_t instance,
                            const uint8_t channel,
                            const uint32_t ticks)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < TMR_CH_COUNT);

    TMR_Type *const base = s_tmrBase[instance];

    /* Schedule the next deadline relative to the existing compare value. */
    base->CH[channel].CMP += ticks;
    uint32_t cnt = base->CNT;
    if (cnt > base->CH[channel].CMP)
    {
        /* If the deadline is already in the past, re-base it on the current counter. */
        base->CH[channel].CMP = base->CNT + ticks;
    }
}

/*******************************************************************************
 * Status Flag Management
 ******************************************************************************/

/*!
 * @brief Read the raw status register of one compare channel.
 */
uint32_t TMR_DRV_GetStatusFlags(const uint32_t instance,
                                const uint8_t channel)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < TMR_CH_COUNT);

    const TMR_Type *const base = s_tmrBase[instance];

    /* Return the raw status value for the selected compare channel. */
    return base->CH[channel].INT;
}

/*!
 * @brief Clear the compare status flag of one channel.
 */
void TMR_DRV_ClearStatusFlags(const uint32_t instance,
                              const uint8_t channel)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < TMR_CH_COUNT);

    TMR_Type *const base = s_tmrBase[instance];

    /* Clear the latched compare flag with write-one-to-clear semantics. */
    base->CH[channel].INT = TMR_CH_INT_CHIF_MASK;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
