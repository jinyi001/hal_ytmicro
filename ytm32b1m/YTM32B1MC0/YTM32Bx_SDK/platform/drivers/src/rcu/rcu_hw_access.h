/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file rcu_hw_access.h
 * @version 1.4.0
 */

#ifndef RCU_HW_ACCESS_H
#define RCU_HW_ACCESS_H

#include <stdbool.h>
#include "device_registers.h"

/*!
 * rcu_hw_access
 * @{
 */

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

static inline void RCU_SetPinFilterCount(RCU_Type *const base, uint8_t count)
{
    uint32_t tmp = base->RPFR;
    tmp &= ~RCU_RPFR_FLTVAL_MASK;
    tmp |= RCU_RPFR_FLTVAL(count);
    base->RPFR = tmp;
}

static inline void RCU_EnablePinFilterInDeepsleep(RCU_Type *const base)
{
    base->RPFR |= RCU_RPFR_FLTSTEN(1U);
}

static inline void RCU_DisablePinFilterInDeepsleep(RCU_Type *const base)
{
    base->RPFR &= ~RCU_RPFR_FLTSTEN_MASK;
}

static inline void RCU_EnablePinFilter(RCU_Type *const base)
{
    base->RPFR |= RCU_RPFR_FLTEN(1UL);
}

static inline void RCU_DisablePinFilter(RCU_Type *const base)
{
    base->RPFR &= ~RCU_RPFR_FLTEN_MASK;
}

#if defined(FEATURE_RCU_HAS_PIN_OUTPUT) && (FEATURE_RCU_HAS_PIN_OUTPUT == 1)
static inline void RCU_SetPinOutput(RCU_Type * const base,  bool enable)
{
    uint32_t tmp = base->RPFR;
    tmp &= ~(RCU_RPFR_RSTPIN_OBEN_MASK);
    tmp |= RCU_RPFR_RSTPIN_OBEN(enable ? 1UL : 0UL);
    base->RPFR = tmp;
}
#endif

static inline uint32_t RCU_GetResetReasonRawValue(RCU_Type *const base)
{
    return base->RSSR;
}

static inline void RCU_ClearRSSRFlag(RCU_Type *const base, uint32_t bitMask)
{
    base->RSSR |= bitMask;
}

/*! @} */

#if defined(__cplusplus)
}
#endif

#endif /* RCU_HW_ACCESS_H*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
