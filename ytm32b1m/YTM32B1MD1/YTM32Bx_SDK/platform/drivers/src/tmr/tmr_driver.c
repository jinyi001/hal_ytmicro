/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
/*!
 * @file tmr_driver.c
 * @version 1.4.1
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
/*! @brief The maximum value of compare register */
#define TMR_COMPARE_MAX (0xFFFFFFFFU)

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*! @brief Table of base addresses for TMR instances. */
static TMR_Type *const s_tmrBase[TMR_INSTANCE_COUNT] = TMR_BASE_PTRS;
/*! @brief TMR functional clock variable which will be updated in some driver functions */
static uint32_t s_tmrClockSrcFreq[TMR_INSTANCE_COUNT] = {0};
/*******************************************************************************
 * Code
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : TMR_DRV_Init
 * Description   : Initializes the TMR module.
 * This function initializes TMR module base on the members of the tmr_config_t structure
 * with the desired values. Including clock source for module, prescaler, allow counter to
 * be stopped in debug mode and start-value for common counter register.
 *
 * Implements    : TMR_DRV_Init_Activity
 *END**************************************************************************/
void TMR_DRV_Init(const uint32_t instance,
                  const tmr_config_t *const config)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    TMR_Type *const base = s_tmrBase[instance];

    /* Configure clock source selection, prescaler, runs in stop mode */
    base->PRS = TMR_PRS_PRS(config->clockPrescaler); /*PRQA S 2985*/
    base->CTRL = TMR_CTRL_DBGDIS(config->stopInDebugMode ? 1UL : 0UL); /*PRQA S 2985*/
    /* Set start-value for counter register */
    base->CNT = config->startValue;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TMR_DRV_Deinit
 * Description   : De-Initializes the TMR module.
 * This function resets all control registers and registers of each channel to default values
 * (Reference Manual Resets).
 * This function should only be called if user wants to stop all channels (not only one channel).
 * System clock is always enabled for TMR module, and doesn't have any option to disable clock.
 *
 * Implements    : TMR_DRV_Deinit_Activity
 *END**************************************************************************/
void TMR_DRV_Deinit(const uint32_t instance)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    uint8_t i;
    TMR_Type *const base = s_tmrBase[instance];

    /* Disable counter and reset counter registers */
    base->CTRL = 0x0U;
    base->CNT = 0x0U;
    /* Reset all channels to default */
    for (i = 0; i < TMR_CH_COUNT; i++)
    {
        base->CH[i].CTRL = 0x0U;
        base->CH[i].INT = 0x1U;
        base->CH[i].CMP = 0x0U;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TMR_DRV_GetDefaultConfig
 * Description   : Gets the default configuration structure of TMR with default settings.
 * This function initializes the hardware configuration structure to default values
 * (Reference Manual Resets).
 * This function should be called before configuring the hardware feature by TMR_DRV_Init()
 * function, otherwise all members be written by user.
 * This function insures that all members are written with safe values, but the user still can
 * modify the desired members.
 *
 * Implements    : TMR_DRV_GetDefaultConfig_Activity
 *END**************************************************************************/
void TMR_DRV_GetDefaultConfig(tmr_config_t *const config)
{
    DEV_ASSERT(config != NULL);
    /* Divide TMR clock by 1 */
    config->clockPrescaler = 0U;
    /* Counter continues to run in debug mode */
    config->stopInDebugMode = false;
    /* Value start for common counter register */
    config->startValue = 0U;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TMR_DRV_InitChannel
 * Description   : Initializes the TMR channel module with a structure.
 * This function initializes TMR channel module base on the members of the tmr_channel_config_t
 * structure for each channel with the desired values. Including channel selected and compare-value
 * for that channel. This function is useful when using PEx tool.
 *
 * Implements    : TMR_DRV_InitChannel_Activity
 *END**************************************************************************/
void TMR_DRV_InitChannel(const uint32_t instance,
                         const tmr_channel_config_t *const config)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);
    DEV_ASSERT(config->channel < TMR_CH_COUNT);

    TMR_Type *const base = s_tmrBase[instance];

    /* Compare value for channel selected */
    base->CH[config->channel].CMP = config->compareValue;
    /* Enable channel */
#if (defined(FEATURE_TMR_HAS_INTERRUPT_ENABLE_CONFIG) && (FEATURE_TMR_HAS_INTERRUPT_ENABLE_CONFIG == 1))
    if (config->interruptEnable)
    {
        base->CH[config->channel].CTRL |= TMR_CH_CTRL_CHIE_MASK;
    }
#endif
    base->CH[config->channel].CTRL |= TMR_CH_CTRL_CHEN_MASK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TMR_DRV_ConfigChannel
 * Description   : Configures the TMR channel module with parameters.
 * This function initializes the desired settings for each channel.
 * This function is the same TMR_DRV_InitChannel() function about feature. But it is required
 * for user to have more options when configure the channel.
 *
 * Implements    : TMR_DRV_ConfigChannel_Activity
 *END**************************************************************************/
void TMR_DRV_ConfigChannel(const uint32_t instance,
                           const uint8_t channel,
                           const uint32_t compareValue)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < TMR_CH_COUNT);

    TMR_Type *const base = s_tmrBase[instance];
    /* Compare value for channel selected */
    base->CH[channel].CMP = compareValue;
    /* Enable channel */
    base->CH[channel].CTRL = TMR_CH_CTRL_CHEN_MASK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TMR_DRV_EnableChannel
 * Description   : Enables the channel selected.
 * This function enables channel selected. The feature in this function is contained
 * in TMR_DRV_InitChannel() also, so after calling that function then no need to call this
 * function for the first time. It is called when a channel is disable momentarily and
 *  user wants to enable channel again.
 *
 * Implements    : TMR_DRV_EnableChannel_Activity
 *END**************************************************************************/
void TMR_DRV_EnableChannel(const uint32_t instance,
                           const uint8_t channel)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < TMR_CH_COUNT);

    TMR_Type *const base = s_tmrBase[instance];
    /* Enable channel */
    base->CH[channel].CTRL |= TMR_CH_CTRL_CHEN_MASK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TMR_DRV_DisableChannel
 * Description   : Disables the channel selected.
 * This function disables channel selected. There is no channel interrupt request is generated
 * after calling this function.
 *
 * Implements    : TMR_DRV_DisableChannel_Activity
 *END**************************************************************************/
void TMR_DRV_DisableChannel(const uint32_t instance,
                            const uint8_t channel)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < TMR_CH_COUNT);

    TMR_Type *const base = s_tmrBase[instance];
    /* Disable channel */
    base->CH[channel].CTRL &= ~TMR_CH_CTRL_CHEN_MASK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TMR_DRV_SetStartValueCount
 * Description   : Sets start-value for Counter register.
 * This function sets start-value for common Counter register. There is only one counter
 * for all channels and the feature in this function is contained in TMR_DRV_Init() also,
 * after calling that function then no need to call this function for the first time.
 * It is called when user wants to set a new start-value to run again instead of calling
 * TMR_DRV_Init(), the action calls TMR_DRV_Init() will reduce performance of module.
 *
 * Implements    : TMR_DRV_SetStartValueCount_Activity
 *END**************************************************************************/
void TMR_DRV_SetStartValueCount(const uint32_t instance,
                                const uint32_t startValue)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);

    TMR_Type *const base = s_tmrBase[instance];
    /* Set start-value for counter register */
    base->CNT = startValue;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TMR_DRV_StartTimer
 * Description   : Starts timer counter.
 * This function enables common Timer Counter and starts running.
 *
 * Implements    : TMR_DRV_StartTimer_Activity
 *END**************************************************************************/
void TMR_DRV_StartTimer(const uint32_t instance)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);

    TMR_Type *const base = s_tmrBase[instance];
    /* Timer counter is started */
    base->CTRL |= TMR_CTRL_TEN_MASK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TMR_DRV_StopTimer
 * Description   : Stops timer counter.
 * This function disables common Timer Counter and stop counting.
 *
 * Implements    : TMR_DRV_StopTimer_Activity
 *END**************************************************************************/
void TMR_DRV_StopTimer(const uint32_t instance)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);

    TMR_Type *const base = s_tmrBase[instance];
    /* Timer counter is stopped */
    base->CTRL &= ~TMR_CTRL_TEN_MASK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TMR_DRV_ComputeTicksByUs
 * Description   : Computes the number of ticks from microseconds.
 * This function computes the number of ticks from microseconds.
 * The number of ticks depends on the frequency and counter prescaler of the TMR source clock.
 * User has to configure the frequency and counter prescaler suitable by themself before calling
 * this function to have valid the number of ticks.
 *
 * Implements    : TMR_DRV_ComputeTicksByUs_Activity
 *END**************************************************************************/
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
    /* Gets current clock prescaler */
    clkPrescaler = ((base->PRS & TMR_PRS_PRS_MASK) >> TMR_PRS_PRS_SHIFT) + 1U;
    /* Gets current functional clock frequency of TMR */
    clkErr = CLOCK_SYS_GetFreq(s_tmrClkNames[clkSelect], &s_tmrClockSrcFreq[instance]);
    /* Checks the functional clock of TMR */
    (void) clkErr;
    DEV_ASSERT(clkErr == STATUS_SUCCESS);
    DEV_ASSERT(s_tmrClockSrcFreq[instance] > 0U);

    /* The formula to convert the microsecond value to the number of tick */
    /* ticks = ((periodUs * ClockSrcFreq) / clkPrescaler) / 1000000 */
    tempTicks = (((uint64_t) periodUs * s_tmrClockSrcFreq[instance]) / clkPrescaler) / 1000000U;

    if (tempTicks > TMR_COMPARE_MAX)
    {
        /* The number of ticks is out of range of compare register */
        retStatus = STATUS_ERROR;
    } else
    {
        /* The number of ticks is in of range of compare register */
        *ticks = (uint32_t) tempTicks;
    }

    return retStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TMR_DRV_IncrementTicks
 * Description   : Increases the number of ticks in compare register.
 * This function will compute the compare-value suitable and set that compare-value for compare
 * register to create a periodic event. To make sure about a periodic event, user should call
 * this function immediately after the event occurs.
 *
 * Implements    : TMR_DRV_IncrementTicks_Activity
 *END**************************************************************************/
void TMR_DRV_IncrementTicks(const uint32_t instance,
                            const uint8_t channel,
                            const uint32_t ticks)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < TMR_CH_COUNT);

    TMR_Type *const base = s_tmrBase[instance];
    base->CH[channel].CMP += ticks;
    uint32_t cnt = base->CNT;
    if (cnt > base->CH[channel].CMP)
    {
        /* Timer already timeout, need to use CNT value instead */
        base->CH[channel].CMP = base->CNT + ticks;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TMR_DRV_GetStatusFlags
 * Description   : Gets status of timer interrupt flag.
 * This function returns the status of each channel selected. When common Counter Timer
 * is enabled and value in Counter Timer reaches to compare-value in Channel Compare register
 * then a channel interrupt request is generated.
 *
 * Implements    : TMR_DRV_GetStatusFlags_Activity
 *END**************************************************************************/
uint32_t TMR_DRV_GetStatusFlags(const uint32_t instance,
                                const uint8_t channel)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < TMR_CH_COUNT);

    const TMR_Type *const base = s_tmrBase[instance];
    /* Return status of channel */
    return base->CH[channel].INT;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TMR_DRV_ClearStatusFlags
 * Description   : Clears channel interrupt flag.
 * This function will clear the flag of channel selected by writing a 1 to bit flag
 * which user wants to clear. All efforts write 0 to bit flag has no effect.
 *
 * Implements    : TMR_DRV_ClearStatusFlags_Activity
 *END**************************************************************************/
void TMR_DRV_ClearStatusFlags(const uint32_t instance,
                              const uint8_t channel)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < TMR_CH_COUNT);

    TMR_Type *const base = s_tmrBase[instance];
    /* Clear interrupt flag, write 1 to clear */
    base->CH[channel].INT = TMR_CH_INT_CHIF_MASK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TMR_DRV_GetCounterValue
 * Description   : Returns current counter module.
 * This function will return the counter value at the moment it is called.
 *
 * Implements    : TMR_DRV_GetCounterValue_Activity
 *END**************************************************************************/
uint32_t TMR_DRV_GetCounterValue(const uint32_t instance)
{
    DEV_ASSERT(instance < TMR_INSTANCE_COUNT);

    const TMR_Type *const base = s_tmrBase[instance];
    /* Return current counter */
    return base->CNT;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
