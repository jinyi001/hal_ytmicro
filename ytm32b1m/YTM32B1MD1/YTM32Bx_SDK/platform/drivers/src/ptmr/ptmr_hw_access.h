/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file ptmr_hw_access.h
 * @version 1.4.1
 *
 * @brief pTMR Hardware Access Layer.
 *
 * This header provides low-level inline accessor functions for the pTMR
 * peripheral registers. Functions are organized into the following categories:
 *   - Module Control (enable, disable, reset)
 *   - Timer Channel Start / Stop
 *   - Timer Period Configuration (set/get period, read counter)
 *   - Interrupt Management (enable, disable, flag get/clear)
 *   - Channel Configuration (chaining, debug mode)
 *   - Clock Source Selection (conditional on FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE)
 *
 * @note This is an internal layer used by the pTMR Driver (ptmr_driver.h/c).
 *       Application code should use the pTMR_DRV_* APIs from ptmr_driver.h.
 */

#ifndef PTMR_HW_ACCESS_H
#define PTMR_HW_ACCESS_H

#include <stdbool.h>
#include "device_registers.h"
#include "ptmr_driver.h"

/*!
 * @addtogroup ptmr_hw_access pTMR Hardware Access
 * @ingroup ptmr
 * @brief Low-level register access functions for the pTMR peripheral.
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Module Control
 ******************************************************************************/
/*!
 * @name Module Control
 * @brief Functions for enabling, disabling, and resetting the pTMR module.
 * @{
 */

/*!
 * @brief Enable the pTMR module functional clock.
 *
 * Sets the EN bit in the Module Control Register (MCR) to enable the pTMR
 * functional clock. This must be called before configuring any timer channel.
 *
 * @param[in] base   Pointer to the pTMR peripheral base address.
 * @param[in] delay  Software delay loop count to ensure the required minimum
 *                   4 peripheral clock cycles elapse after enabling the module.
 *                   Set to 0 if the read-modify-write overhead is sufficient.
 *
 * @note This function does NOT un-gate the system clock gating control.
 *       The peripheral clock must be enabled via the clock_manager first.
 */
static inline void pTMR_Enable(pTMR_Type * const base, volatile uint32_t delay)
{
    volatile uint32_t tempDelay = delay;
    base->MCR |= pTMR_MCR_EN_MASK;
    /* Run this counter down to zero
        If the delay is 0, the four clock delay between setting and clearing
        the SW_RST bit is ensured by the read-modify-write operation.
    */
    while(tempDelay != 0u)
    {
        /* Since we need a four cycle delay, we assume the decrement is one cycle
            and insert three NOP instructions. The actual delay will be larger because
            of the loop overhead and the compiler optimization.
        */
        tempDelay = tempDelay - 1U; 
        __NOP();
        __NOP();
        __NOP();
    }
}

/*!
 * @brief Disable the pTMR module functional clock.
 *
 * Clears the EN bit in the MCR register to disable the pTMR functional clock.
 * All timer channels will stop counting when the module is disabled.
 *
 * @param[in] base  Pointer to the pTMR peripheral base address.
 *
 * @note This does not affect the system clock gating control.
 */
static inline void pTMR_Disable(pTMR_Type * const base)
{
    base->MCR &= ~pTMR_MCR_EN_MASK;
}

/*!
 * @brief Reset all pTMR registers to their default values.
 *
 * Clears the MCR and all channel registers (TCR, TCV, TFR, TSV) for all
 * four channels. A software delay ensures the minimum 4 peripheral clock
 * cycle requirement is met after the reset.
 *
 * @param[in] base   Pointer to the pTMR peripheral base address.
 * @param[in] delay  Software delay loop count (same as pTMR_Enable()).
 *
 * @note The Module Control Register is also cleared by this function.
 */
static inline void pTMR_Reset(pTMR_Type * const base, volatile uint32_t delay)
{
    volatile uint32_t tempDelay = delay;
    base->MCR = 0;
    for (uint32_t i = 0; i < pTMR_IRQS_CH_COUNT; i++)
    {
        base->CH[i].TCR = 0U;
        base->CH[i].TCV = 0U;
        base->CH[i].TFR = pTMR_CH_TFR_TIF_MASK;
        base->CH[i].TSV = 0U;
    }
    /* Run this counter down to zero
        If the delay is 0, the four clock delay between setting and clearing
        the SW_RST bit is ensured by the read-modify-write operation.
    */
    while(tempDelay != 0u)
    {
        /* Since we need a four cycle delay, we assume the decrement is one cycle
            and insert three NOP instructions. The actual delay will be larger because
            of the loop overhead and the compiler optimization.
        */
        tempDelay = tempDelay - 1U;
        __NOP();
        __NOP();
        __NOP();
    }
}

/*! @} */ /* End of Module Control */

/*******************************************************************************
 * Timer Channel Start / Stop
 ******************************************************************************/
/*!
 * @name Timer Channel Start / Stop
 * @brief Functions for starting and stopping individual timer channels.
 * @{
 */

/*!
 * @brief Start counting on the specified timer channel.
 *
 * Sets the TEN bit in the Timer Control Register (TCR) for the given channel.
 * Once started, the channel operates according to its configured mode and
 * control settings.
 *
 * @param[in] base     Pointer to the pTMR peripheral base address.
 * @param[in] channel  Timer channel index (0–3).
 */
static inline void pTMR_StartTimerChannels(pTMR_Type * const base,
                                           uint32_t channel)
{
    base->CH[channel].TCR |= pTMR_CH_TCR_TEN_MASK;
}

/*!
 * @brief Stop counting on the specified timer channel.
 *
 * Clears the TEN bit in the TCR register for the given channel. The channel
 * reloads its period value when subsequently restarted via
 * pTMR_StartTimerChannels().
 *
 * @param[in] base     Pointer to the pTMR peripheral base address.
 * @param[in] channel  Timer channel index (0–3).
 */
static inline void pTMR_StopTimerChannels(pTMR_Type * const base,
                                          uint32_t channel)
{
    base->CH[channel].TCR &= ~pTMR_CH_TCR_TEN_MASK;
}

/*! @} */ /* End of Timer Channel Start / Stop */

/*******************************************************************************
 * Timer Period Configuration
 ******************************************************************************/
/*!
 * @name Timer Period Configuration
 * @brief Functions for setting, reading the timer channel period, and
 *        querying the current counter value.
 * @{
 */

/*!
 * @brief Set the timer channel period in raw count units.
 *
 * Writes the start value to the Timer Start Value Register (TSV). The timer
 * channel counts down from this value to zero before generating a timeout
 * event. For a running channel, the new value takes effect after the
 * current period expires.
 *
 * @param[in] base     Pointer to the pTMR peripheral base address.
 * @param[in] channel  Timer channel index (0–3).
 * @param[in] count    Period value in raw timer counts (0 to 0xFFFFFFFF).
 *
 * @note To apply the new period immediately, disable and re-enable the channel.
 */
static inline void pTMR_SetTimerPeriodByCount(pTMR_Type * const base,
                                              uint32_t channel,
                                              uint32_t count)
{
    base->CH[channel].TSV = count;
}

/*!
 * @brief Get the timer channel period in raw count units.
 *
 * Reads the current period value from the Timer Start Value Register (TSV).
 *
 * @param[in] base     Pointer to the pTMR peripheral base address.
 * @param[in] channel  Timer channel index (0–3).
 * @return Timer channel period in raw count units.
 */
static inline uint32_t pTMR_GetTimerPeriodByCount(const pTMR_Type * base,
                                                  uint32_t channel)
{
    return (base->CH[channel].TSV);
}

/*!
 * @brief Get the current timer channel counter value.
 *
 * Reads the real-time counter value from the Timer Current Value Register
 * (TCV). The returned value ranges from the period start value down to 0.
 *
 * @param[in] base     Pointer to the pTMR peripheral base address.
 * @param[in] channel  Timer channel index (0–3).
 * @return Current counter value of the specified timer channel.
 *
 * @note Ensure the elapsed time does not exceed the timer period to avoid
 *       counter wrap-around.
 */
static inline uint32_t pTMR_GetCurrentTimerCount(const pTMR_Type * base,
                                                 uint32_t channel)
{
    return (base->CH[channel].TCV);
}

/*! @} */ /* End of Timer Period Configuration */

/*******************************************************************************
 * Interrupt Management
 ******************************************************************************/
/*!
 * @name Interrupt Management
 * @brief Functions for controlling timer channel interrupt generation and
 *        managing interrupt status flags.
 * @{
 */

/*!
 * @brief Enable interrupt generation for the specified timer channel.
 *
 * Sets the TIE bit in the TCR register. When enabled, the channel generates
 * an interrupt request upon timer timeout.
 *
 * @param[in] base     Pointer to the pTMR peripheral base address.
 * @param[in] channel  Timer channel index (0–3).
 */
static inline void pTMR_EnableInterruptTimerChannels(pTMR_Type * const base,
                                                     uint32_t channel)
{
    base->CH[channel].TCR |= pTMR_CH_TCR_TIE_MASK;
}

/*!
 * @brief Disable interrupt generation for the specified timer channel.
 *
 * Clears the TIE bit in the TCR register. The channel will no longer generate
 * interrupt requests upon timer timeout.
 *
 * @param[in] base     Pointer to the pTMR peripheral base address.
 * @param[in] channel  Timer channel index (0–3).
 */
static inline void pTMR_DisableInterruptTimerChannels(pTMR_Type * const base,
                                                      uint32_t channel)
{
    base->CH[channel].TCR &= ~pTMR_CH_TCR_TIE_MASK;
}

/*!
 * @brief Get the interrupt flag for the specified timer channel.
 *
 * Reads the TIF bit from the Timer Flag Register (TFR). The flag is set
 * when the timer channel times out (counter reaches zero).
 *
 * @param[in] base     Pointer to the pTMR peripheral base address.
 * @param[in] channel  Timer channel index (0–3).
 * @return Interrupt flag status:
 *         - Non-zero: Interrupt is pending (timer has timed out).
 *         - 0: No interrupt pending.
 */
static inline uint32_t pTMR_GetInterruptFlagTimerChannels(const pTMR_Type * base,
                                                          uint32_t channel)
{
    return (base->CH[channel].TFR) & pTMR_CH_TFR_TIF_MASK;
}

/*!
 * @brief Clear the interrupt flag for the specified timer channel.
 *
 * Clears the TIF bit in the Timer Flag Register (TFR) by writing 1 to it,
 * then clearing it. This must be called in the interrupt handler to
 * acknowledge the timer timeout event.
 *
 * @param[in] base     Pointer to the pTMR peripheral base address.
 * @param[in] channel  Timer channel index (0–3).
 */
static inline void pTMR_ClearInterruptFlagTimerChannels(pTMR_Type * const base,
                                                        uint32_t channel)
{
    /* Write 1 to clear the interrupt flag. */
    base->CH[channel].TFR |= pTMR_CH_TFR_TIF_MASK;
    base->CH[channel].TFR &= ~pTMR_CH_TFR_TIF_MASK;
}

/*! @} */ /* End of Interrupt Management */

/*******************************************************************************
 * Channel Configuration
 ******************************************************************************/
/*!
 * @name Channel Configuration
 * @brief Functions for configuring timer channel chaining and debug mode
 *        behavior.
 * @{
 */

/*!
 * @brief Configure channel chaining for the specified timer channel.
 *
 * When chaining is enabled, the timer channel decrements its counter on the
 * previous channel's timeout event instead of on each clock cycle. This
 * allows cascading multiple channels to achieve longer timing periods.
 *
 * @param[in] base              Pointer to the pTMR peripheral base address.
 * @param[in] channel           Timer channel index (0–3).
 * @param[in] isChannelChained  Channel chaining control:
 *                              - true:  Channel is chained (decrements on
 *                                       previous channel timeout).
 *                              - false: Channel runs independently (decrements
 *                                       on each clock cycle).
 *
 * @warning Channel 0 cannot be chained as it has no preceding channel.
 */
static inline void pTMR_SetTimerChannelChainCmd(pTMR_Type * const base,
                                                uint32_t channel,
                                                bool isChannelChained)
{
    base->CH[channel].TCR &= ~pTMR_CH_TCR_CHAIN_MASK;
    base->CH[channel].TCR |=  pTMR_CH_TCR_CHAIN(isChannelChained ? 1UL : 0UL);
}

/*!
 * @brief Configure pTMR behavior in debug (halt) mode.
 *
 * Controls whether the pTMR timer channels continue to count or freeze
 * when the processor enters debug mode. Freezing timers during debug
 * allows the developer to inspect the system state without timer-related
 * side effects.
 *
 * @param[in] base          Pointer to the pTMR peripheral base address.
 * @param[in] isRunInDebug  Debug mode behavior:
 *                          - true:  Timers continue to run in debug mode.
 *                          - false: Timers freeze (stop) when debug mode
 *                                   is entered.
 */
static inline void pTMR_SetTimerRunInDebugCmd(pTMR_Type * const base,
                                              bool isRunInDebug)
{
    base->MCR &= ~pTMR_MCR_FRZ_MASK;
    base->MCR |= pTMR_MCR_FRZ(isRunInDebug ? 0UL: 1UL);
}

/*! @} */ /* End of Channel Configuration */

/*******************************************************************************
 * Clock Source Selection
 ******************************************************************************/
#if (defined(FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE) && (FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE == 1))
/*!
 * @name Clock Source Selection
 * @brief Functions for selecting the pTMR clock source.
 *
 * Available only on devices with IPC clock source support
 * (FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE == 1). The pTMR can be clocked either
 * by the peripheral bus clock (PCLK) or by a function clock from the IPC
 * module.
 *
 * @warning When using the IPC function clock, its frequency must be no
 *          greater than PCLK / 4. Otherwise the pTMR behavior is undefined.
 * @{
 */

/*!
 * @brief Select the pTMR clock source.
 *
 * Configures the CLK_SEL field in the MCR register to select between the
 * peripheral bus clock (PCLK) and the IPC function clock.
 *
 * @param[in] base        Pointer to the pTMR peripheral base address.
 * @param[in] useFuncClk  Clock source selection:
 *                        - true:  Use IPC function clock.
 *                        - false: Use peripheral bus clock (PCLK).
 *
 * @pre When selecting the IPC function clock, ensure IPC clock is properly
 *      configured and its frequency does not exceed PCLK / 4.
 */
static inline void pTMR_SetTimerClockSource(pTMR_Type * const base,
                                              bool useFuncClk)
{
    base->MCR &= ~pTMR_MCR_CLK_SEL_MASK;
    base->MCR |= pTMR_MCR_CLK_SEL(useFuncClk ? 1UL: 0UL);
}

/*!
 * @brief Get the currently selected pTMR clock source.
 *
 * Reads the CLK_SEL field from the MCR register.
 *
 * @param[in] base  Pointer to the pTMR peripheral base address.
 * @return Clock source selection:
 *         - true:  IPC function clock is selected.
 *         - false: Peripheral bus clock (PCLK) is selected.
 */
 static inline bool pTMR_GetTimerClockSource(const pTMR_Type * const base)
 {
     return ((base->MCR & pTMR_MCR_CLK_SEL_MASK) >> pTMR_MCR_CLK_SEL_SHIFT) != 0U;
 }

/*! @} */ /* End of Clock Source Selection */
#endif /* FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of ptmr_hw_access group */

#endif /* PTMR_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
