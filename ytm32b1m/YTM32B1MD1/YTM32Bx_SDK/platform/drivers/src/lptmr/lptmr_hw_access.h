/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file lptmr_hw_access.h
 * @version 1.4.1
 *
 * @brief lpTMR Hardware Access Layer.
 *
 * This header provides the low-level register access helpers used by the
 * public lpTMR driver. It exposes the reset-state initializer plus inline
 * accessors for flags, modes, prescaler settings, compare values, and counter
 * reads.
 *
 * @note This is an internal layer. Application code should use the
 *       `lpTMR_DRV_*` APIs from `lptmr_driver.h`.
 */

#ifndef LPTMR_HW_ACCESS_H
#define LPTMR_HW_ACCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device_registers.h"
#include "lptmr_driver.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @addtogroup lptmr_hw_access lpTMR Hardware Access
 * @ingroup lptmr
 * @brief Low-level register access helpers for the lpTMR peripheral.
 * @{
 */

/*******************************************************************************
 * Initialization
 ******************************************************************************/
/*!
 * @name Initialization
 * @brief Functions for restoring the peripheral to a known default state.
 * @{
 */

/*!
 * @brief Initialize the lpTMR registers to their reset-state values.
 *
 * Disables the module and writes the documented reset values for the control,
 * status, interrupt, prescaler, and compare registers.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 *
 * @pre `base` must not be `NULL`.
 */
void lpTMR_Init(lpTMR_Type* const base);

/*! @} */ /* End of Initialization */

/*******************************************************************************
 * DMA & Interrupt Control
 ******************************************************************************/
/*!
 * @name DMA & Interrupt Control
 * @brief Functions for enabling and querying DMA requests and compare interrupts.
 * @{
 */

#if defined(lpTMR_DIE_DMAEN_MASK)
/*!
 * @brief Read the DMA request enable state.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 * @return `true` if compare-match DMA requests are enabled, `false` otherwise.
 */
static inline bool lpTMR_GetDmaRequest(const lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->DIE;

    tmp = (tmp & lpTMR_DIE_DMAEN_MASK) >> lpTMR_DIE_DMAEN_SHIFT;

    return ((tmp == 1u) ? true : false);
}

/*!
 * @brief Enable or disable compare-match DMA requests.
 *
 * @param[in] base    Pointer to the lpTMR peripheral base address.
 * @param[in] enable  `true` to enable DMA requests, `false` to disable them.
 *
 * @note When enabled, the DMA request is generated on compare match.
 */
static inline void lpTMR_SetDmaRequest(lpTMR_Type* const base,
                                       bool enable)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->DIE;
    tmp &= ~(lpTMR_DIE_DMAEN_MASK);
    tmp |= lpTMR_DIE_DMAEN(enable ? (uint32_t)1u : (uint32_t)0u);
    base->DIE = tmp;
}
#endif /* lpTMR_DIE_DMAEN_MASK */

/*!
 * @brief Read the compare interrupt enable state.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 * @return `true` if compare interrupts are enabled, `false` otherwise.
 */
static inline bool lpTMR_GetInterruptEnable(const lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->DIE;
    tmp = (tmp & lpTMR_DIE_IE_MASK) >> lpTMR_DIE_IE_SHIFT;

    return ((tmp == 1u) ? true : false);
}

/*!
 * @brief Enable or disable compare interrupts.
 *
 * @param[in] base    Pointer to the lpTMR peripheral base address.
 * @param[in] enable  `true` to enable compare interrupts, `false` to disable them.
 */
static inline void lpTMR_SetInterrupt(lpTMR_Type* const base,
                                      bool enable)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->DIE;
    tmp &= ~(lpTMR_DIE_IE_MASK);
    tmp |= lpTMR_DIE_IE(enable ? (uint32_t)1u : (uint32_t)0u);
    base->DIE = tmp;
}

/*! @} */ /* End of DMA & Interrupt Control */

/*******************************************************************************
 * Compare Flag Handling
 ******************************************************************************/
/*!
 * @name Compare Flag Handling
 * @brief Functions for reading and clearing the compare-match flag.
 * @{
 */

/*!
 * @brief Read the compare-match flag state.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 * @return `true` if a compare match is pending, `false` otherwise.
 */
static inline bool lpTMR_GetCompareFlag(const lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->STS;
    tmp = (tmp & lpTMR_STS_CCF_MASK) >> lpTMR_STS_CCF_SHIFT;

    return ((tmp == 1u) ? true : false);
}

/*!
 * @brief Clear the compare-match flag.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 *
 * @note On affected cores, the read-after-write sequence preserves the
 *       required memory-ordering behavior for the erratum workaround.
 */
static inline void lpTMR_ClearCompareFlag(lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->STS;
    tmp |= (lpTMR_STS_CCF_MASK);
    base->STS = tmp;
#ifdef ERRATA_ARM_838869
    /* Perform a read-after-write to satisfy the serialization requirement. */
    (void)base->CSR;
#endif
}

/*! @} */ /* End of Compare Flag Handling */

/*******************************************************************************
 * Pulse Counter Input Configuration
 ******************************************************************************/
/*!
 * @name Pulse Counter Input Configuration
 * @brief Functions for selecting the pulse source and active edge.
 * @{
 */

/*!
 * @brief Read the selected pulse input source.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 * @return Active pulse input selection.
 */
static inline lptmr_pinselect_t lpTMR_GetPinSelect(const lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->CTRL;
    tmp = (tmp & lpTMR_CTRL_PINSEL_MASK) >> lpTMR_CTRL_PINSEL_SHIFT;
    return (lptmr_pinselect_t)(tmp);
}

/*!
 * @brief Program the pulse input source.
 *
 * @param[in] base    Pointer to the lpTMR peripheral base address.
 * @param[in] pinsel  Pulse input source selection.
 *
 * @note The lpTMR must be disabled before changing the input route.
 */
static inline void lpTMR_SetPinSelect(lpTMR_Type* const base,
                                      const lptmr_pinselect_t pinsel)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->CTRL;
    tmp &= ~(lpTMR_CTRL_PINSEL_MASK);
    tmp |= lpTMR_CTRL_PINSEL(pinsel);
    base->CTRL = tmp;
}

/*!
 * @brief Read the active pulse-counting edge.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 * @return Active pulse polarity selection.
 */
static inline lptmr_pinpolarity_t lpTMR_GetPinPolarity(const lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->STS;
    tmp = (tmp & lpTMR_CTRL_PINPOL_MASK) >> lpTMR_CTRL_PINPOL_SHIFT;

    return (lptmr_pinpolarity_t)((tmp == 0u) ? lpTMR_PINPOLARITY_RISING : lpTMR_PINPOLARITY_FALLING);
}

/*!
 * @brief Program the active pulse-counting edge.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 * @param[in] pol   Edge selection used for pulse counting.
 *
 * @note The lpTMR must be disabled before changing the pulse polarity.
 */
static inline void lpTMR_SetPinPolarity(lpTMR_Type* const base,
                                        const lptmr_pinpolarity_t pol)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->CTRL;
    tmp &= ~(lpTMR_CTRL_PINPOL_MASK);
    tmp |= lpTMR_CTRL_PINPOL(pol);
    base->CTRL = tmp;
}

/*! @} */ /* End of Pulse Counter Input Configuration */

/*******************************************************************************
 * Mode & Run Control
 ******************************************************************************/
/*!
 * @name Mode & Run Control
 * @brief Functions for selecting lpTMR operating mode and enable state.
 * @{
 */

/*!
 * @brief Read the free-running mode enable state.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 * @return `true` if free-running mode is enabled, `false` otherwise.
 */
static inline bool lpTMR_GetFreeRunning(const lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->CTRL;
    tmp = (tmp & lpTMR_CTRL_TMODE_MASK) >> lpTMR_CTRL_TMODE_SHIFT;

    return ((tmp == 1u) ? true : false);
}

/*!
 * @brief Enable or disable free-running mode.
 *
 * @param[in] base    Pointer to the lpTMR peripheral base address.
 * @param[in] enable  `true` to reset on overflow, `false` to reset on compare match.
 *
 * @note The lpTMR must be disabled before changing free-running mode.
 */
static inline void lpTMR_SetFreeRunning(lpTMR_Type* const base,
                                        const bool enable)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->CTRL;
    tmp &= ~(lpTMR_CTRL_TMODE_MASK);
    tmp |= lpTMR_CTRL_TMODE(enable ? (uint32_t)1u : (uint32_t)0u);
    base->CTRL = tmp;
}

/*!
 * @brief Read the current lpTMR operating mode.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 * @return Current work mode selection.
 */
static inline lptmr_workmode_t lpTMR_GetWorkMode(const lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->CTRL;
    tmp = (tmp & lpTMR_CTRL_MODE_MASK) >> lpTMR_CTRL_MODE_SHIFT;

    return (lptmr_workmode_t)((tmp == 0u) ? lpTMR_WORKMODE_TIMER : lpTMR_WORKMODE_PULSECOUNTER);
}

/*!
 * @brief Program the lpTMR operating mode.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 * @param[in] mode  New work mode selection.
 *
 * @note The lpTMR must be disabled before changing work mode.
 */
static inline void lpTMR_SetWorkMode(lpTMR_Type* const base,
                                     const lptmr_workmode_t mode)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->CTRL;
    tmp &= ~(lpTMR_CTRL_MODE_MASK);
    tmp |= lpTMR_CTRL_MODE(mode);
    base->CTRL = tmp;
}

/*!
 * @brief Read the module enable state.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 * @return `true` if the counter is enabled, `false` otherwise.
 */
static inline bool lpTMR_GetEnable(const lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->CTRL;
    tmp = (tmp & lpTMR_CTRL_EN_MASK) >> lpTMR_CTRL_EN_SHIFT;

    return ((tmp == 1u) ? true : false);
}

/*!
 * @brief Enable the lpTMR counter.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 */
static inline void lpTMR_Enable(lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->CTRL;
    tmp &= ~(lpTMR_CTRL_EN_MASK);
    tmp |= lpTMR_CTRL_EN(1u);
    base->CTRL = tmp;
}

/*!
 * @brief Disable the lpTMR counter.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 */
static inline void lpTMR_Disable(lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->CTRL;
    tmp &= ~(lpTMR_CTRL_EN_MASK);
    tmp |= lpTMR_CTRL_EN(0u);
    base->CTRL = tmp;
}

/*! @} */ /* End of Mode & Run Control */

/*******************************************************************************
 * Prescaler / Clock Configuration
 ******************************************************************************/
/*!
 * @name Prescaler / Clock Configuration
 * @brief Functions for selecting the timer prescaler, glitch filter, bypass,
 *        and optional clock source.
 * @{
 */

/*!
 * @brief Read the active prescaler or glitch-filter selection.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 * @return Current prescaler or glitch-filter setting.
 */
static inline lptmr_prescaler_t lpTMR_GetPrescaler(const lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->PRS;
    tmp = (tmp & lpTMR_PRS_PRES_MASK) >> lpTMR_PRS_PRES_SHIFT;
    return (lptmr_prescaler_t)(tmp);
}

/*!
 * @brief Program the prescaler or glitch-filter selection.
 *
 * @param[in] base   Pointer to the lpTMR peripheral base address.
 * @param[in] presc  New prescaler or glitch-filter setting.
 *
 * @note The lpTMR must be disabled before changing this field.
 */
static inline void lpTMR_SetPrescaler(lpTMR_Type* const base,
                                      const lptmr_prescaler_t presc)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->PRS;
    tmp &= ~(lpTMR_PRS_PRES_MASK);
    tmp |= lpTMR_PRS_PRES(presc);
    base->PRS = tmp;
}

#ifdef FEATURE_lpTMR_HAS_CLOCK_SELECTION
/*!
 * @brief Program the lpTMR counter clock source.
 *
 * @param[in] base    Pointer to the lpTMR peripheral base address.
 * @param[in] source  New clock source selection.
 *
 * @note The lpTMR must be disabled before changing the clock source.
 */
static inline void lpTMR_SetClockSource(lpTMR_Type* const base,
                                        const lptmr_clock_source_t source)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->PRS;
    tmp &= ~(lpTMR_PRS_CLKSEL_MASK);
    tmp |= lpTMR_PRS_CLKSEL(source);
    base->PRS = tmp;
}
#endif /* FEATURE_lpTMR_HAS_CLOCK_SELECTION */

/*!
 * @brief Read the prescaler or glitch-filter bypass state.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 * @return `true` if bypass is enabled, `false` otherwise.
 */
static inline bool lpTMR_GetBypass(const lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->PRS;
    tmp = (tmp & lpTMR_PRS_BYPASS_MASK) >> lpTMR_PRS_BYPASS_SHIFT;

    return ((tmp == 1u) ? true : false);
}

/*!
 * @brief Enable or disable prescaler or glitch-filter bypass.
 *
 * @param[in] base    Pointer to the lpTMR peripheral base address.
 * @param[in] enable  `true` to bypass the prescaler/filter, `false` to use it.
 *
 * @note The lpTMR must be disabled before changing bypass state.
 */
static inline void lpTMR_SetBypass(lpTMR_Type* const base,
                                   const bool enable)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->PRS;
    tmp &= ~(lpTMR_PRS_BYPASS_MASK);
    tmp |= lpTMR_PRS_BYPASS(enable ? (uint32_t)1u : (uint32_t)0u);
    base->PRS = tmp;
}

/*! @} */ /* End of Prescaler / Clock Configuration */

/*******************************************************************************
 * Compare & Counter Access
 ******************************************************************************/
/*!
 * @name Compare & Counter Access
 * @brief Functions for reading and programming compare and counter registers.
 * @{
 */

/*!
 * @brief Read the compare register value.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 * @return Current compare value in raw ticks.
 */
static inline uint16_t lpTMR_GetCompareValue(const lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->CMP;
    tmp = (tmp & lpTMR_CMP_CMP_MASK) >> lpTMR_CMP_CMP_SHIFT;
    return (uint16_t)(tmp);
}

/*!
 * @brief Program the compare register value.
 *
 * @param[in] base     Pointer to the lpTMR peripheral base address.
 * @param[in] compval  New compare value in raw ticks.
 *
 * @note A compare value of zero causes the compare event to assert and remain
 *       asserted until the timer is disabled.
 */
static inline void lpTMR_SetCompareValue(lpTMR_Type* const base,
                                         const uint16_t compval)
{
    DEV_ASSERT(base != NULL);

    uint32_t tmp = base->CMP;
    tmp &= ~(lpTMR_CMP_CMP_MASK);
    tmp |= lpTMR_CMP_CMP(compval);
    base->CMP = tmp;
}

/*!
 * @brief Latch and read the current counter value.
 *
 * @param[in] base  Pointer to the lpTMR peripheral base address.
 * @return Current counter value in raw ticks.
 */
static inline uint16_t lpTMR_GetCounterValue(lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    /* Latch the counter value before reading the count register. */
    base->LCNT = (0u);
    uint16_t cnr = (uint16_t)base->CNT;
    return cnr;
}

/*! @} */ /* End of Compare & Counter Access */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* LPTMR_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
