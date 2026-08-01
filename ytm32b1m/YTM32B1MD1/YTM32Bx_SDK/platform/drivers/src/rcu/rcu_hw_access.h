/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file rcu_hw_access.h
 * @version 1.4.1
 *
 * @brief RCU Hardware Access - Internal register helpers for reset pin control and status access.
 *
 * This header provides the low-level inline helpers used by `rcu_driver.c`
 * to program the RESET_B pin filter registers and to access the latched reset
 * status register.
 */

#ifndef RCU_HW_ACCESS_H
#define RCU_HW_ACCESS_H

#include <stdbool.h>
#include "device_registers.h"

/*!
 * @defgroup rcu_hw_access RCU Hardware Access
 * @ingroup rcu
 * @brief Reset Control Unit register-level helper API.
 * @details Provides inline helpers for reset pin filter control, optional
 *          reset pin output control, and reset status register access.
 * @{
 */

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Reset Pin Filter Control
 ******************************************************************************/
/*!
 * @name Reset Pin Filter Control
 * @brief Helpers for programming the RESET_B digital filter.
 * @{
 */

/*!
 * @brief Program the RESET_B filter counter.
 *
 * Writes @a count to `RPFR[FLTVAL]` without changing the other reset pin
 * filter control bits.
 *
 * @param[in] base  Pointer to the RCU peripheral base address.
 * @param[in] count Raw filter-count value to write.
 */
static inline void RCU_SetPinFilterCount(RCU_Type *const base, uint8_t count)
{
    uint32_t tmp = base->RPFR;
    tmp &= ~RCU_RPFR_FLTVAL_MASK;
    tmp |= RCU_RPFR_FLTVAL(count);
    base->RPFR = tmp;
}

/*!
 * @brief Keep the reset pin filter active during deep sleep.
 *
 * @param[in] base Pointer to the RCU peripheral base address.
 */
static inline void RCU_EnablePinFilterInDeepsleep(RCU_Type *const base)
{
    base->RPFR |= RCU_RPFR_FLTSTEN(1U);
}

/*!
 * @brief Disable reset pin filtering during deep sleep.
 *
 * @param[in] base Pointer to the RCU peripheral base address.
 */
static inline void RCU_DisablePinFilterInDeepsleep(RCU_Type *const base)
{
    base->RPFR &= ~RCU_RPFR_FLTSTEN_MASK;
}

/*!
 * @brief Enable the RESET_B digital filter.
 *
 * @param[in] base Pointer to the RCU peripheral base address.
 */
static inline void RCU_EnablePinFilter(RCU_Type *const base)
{
    base->RPFR |= RCU_RPFR_FLTEN(1UL);
}

/*!
 * @brief Disable the RESET_B digital filter.
 *
 * @param[in] base Pointer to the RCU peripheral base address.
 */
static inline void RCU_DisablePinFilter(RCU_Type *const base)
{
    base->RPFR &= ~RCU_RPFR_FLTEN_MASK;
}

/*! @} */ /* End of Reset Pin Filter Control */

#if defined(FEATURE_RCU_HAS_PIN_OUTPUT) && (FEATURE_RCU_HAS_PIN_OUTPUT == 1)
/*******************************************************************************
 * Reset Pin Output Control
 ******************************************************************************/
/*!
 * @name Reset Pin Output Control
 * @brief Helper for configuring the optional reset pin output driver.
 * @{
 */

/*!
 * @brief Enable or disable the reset pin output driver.
 *
 * Programs the optional `RPFR[RSTPIN_OBEN]` field when the target device
 * exposes reset pin output support.
 *
 * @param[in] base   Pointer to the RCU peripheral base address.
 * @param[in] enable `true` to enable reset pin output, `false` to disable it.
 */
static inline void RCU_SetPinOutput(RCU_Type *const base, bool enable)
{
    uint32_t tmp = base->RPFR;
    tmp &= ~(RCU_RPFR_RSTPIN_OBEN_MASK);
    tmp |= RCU_RPFR_RSTPIN_OBEN(enable ? 1UL : 0UL);
    base->RPFR = tmp;
}

/*! @} */ /* End of Reset Pin Output Control */
#endif /* defined(FEATURE_RCU_HAS_PIN_OUTPUT) && (FEATURE_RCU_HAS_PIN_OUTPUT == 1) */

/*******************************************************************************
 * Reset Status Register Access
 ******************************************************************************/
/*!
 * @name Reset Status Register Access
 * @brief Helpers for reading and clearing the latched reset status register.
 * @{
 */

/*!
 * @brief Return the raw reset status register value.
 *
 * @param[in] base Pointer to the RCU peripheral base address.
 * @return Raw `RSSR` register value.
 */
static inline uint32_t RCU_GetResetReasonRawValue(RCU_Type *const base)
{
    return base->RSSR;
}

/*!
 * @brief Clear one or more reset status flags.
 *
 * Performs a write-one-to-clear update on the `RSSR` register.
 *
 * @param[in] base    Pointer to the RCU peripheral base address.
 * @param[in] bitMask Mask of reset status bits to clear.
 */
static inline void RCU_ClearRSSRFlag(RCU_Type *const base, uint32_t bitMask)
{
    base->RSSR = bitMask;
}

/*! @} */ /* End of Reset Status Register Access */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of rcu_hw_access group */

#endif /* RCU_HW_ACCESS_H */
/*******************************************************************************
 * End of file
 ******************************************************************************/
