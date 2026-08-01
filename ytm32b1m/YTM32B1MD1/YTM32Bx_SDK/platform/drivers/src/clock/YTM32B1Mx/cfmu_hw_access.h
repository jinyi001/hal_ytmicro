/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file cfmu_hw_access.h
 * @version 1.4.1
 *
 * @brief CFMU Hardware Access — Clock Frequency Measurement Unit register interface.
 *
 * This header provides low-level inline functions for controlling the CFMU
 * peripheral, which is used for clock output routing and clock frequency
 * measurement on devices with separate CMU and CLKOUT hardware
 * (FEATURE_SOC_HAS_SEPARATE_CMU_AND_CLKOUT == 1).
 *
 * Functions are organized into:
 *   - **Clock Output Enable** — Enable/disable the clock measurement output.
 *   - **Clock Output Source** — Select the clock source to be measured.
 *   - **Clock Output Divider** — Set the output frequency divider.
 */

#ifndef CFMU_HW_ACCESS_H
#define CFMU_HW_ACCESS_H

#include "device_registers.h"
#include <stdbool.h>
#include <stddef.h>



/*!
 * @ingroup cfmu_hw_access
 * @defgroup cfmu_hw_access
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*******************************************************************************
 * Clock Output Control
 ******************************************************************************/
/*!
 * @name Clock Output Control
 * @brief Functions for enabling, selecting, and dividing the CFMU clock output.
 * @{
 */

/*!
 * @brief Enable or disable the CFMU clock output.
 *
 * Controls whether the selected clock source is output through the CFMU
 * measurement path.
 *
 * @param[in] base    Base address for current CFMU instance.
 * @param[in] enable  true to enable clock output, false to disable.
 */
static inline void CFMU_SetClkOutEnable(CFMU_Type *base, const bool enable)
{
#ifdef CFMU_CTRL_CLK_OUTEN_SHIFT
    uint32_t regValue = base->CTRL;
    regValue &= ~CFMU_CTRL_CLK_OUTEN_MASK;
    base->CTRL = regValue | ((uint32_t)(enable ? 1U : 0U) << CFMU_CTRL_CLK_OUTEN_SHIFT);
#else
    (void) base;
    (void) enable;
#endif
}

/*!
 * @brief Select the clock source to be measured / output.
 *
 * @param[in] base  Base address for current CFMU instance.
 * @param[in] src   Clock output source selector value.
 */
static inline void CFMU_SetClkOutSrc(CFMU_Type* const base, const uint8_t src)
{
    uint32_t regValue = base->CTRL;
    regValue &= ~(CFMU_CTRL_CLK_SEL_MASK);
    regValue |= CFMU_CTRL_CLK_SEL(src);
    base->CTRL = regValue;
}

/*!
 * @brief Set the CFMU clock output divider.
 *
 * Output frequency = input frequency / (clkDiv + 1).
 *
 * @param[in] base    Base address for current CFMU instance.
 * @param[in] clkDiv  Divider value. Range [0, 255]. 0 = divide by 1.
 */
static inline void CFMU_SetClkOutDiv(CFMU_Type *base, const uint8_t clkDiv)
{
    uint32_t regValue = base->CTRL;
    regValue &= (uint32_t)(~CFMU_CTRL_CLK_DIV_MASK);
    regValue |= (uint32_t)CFMU_CTRL_CLK_DIV(clkDiv);
    base->CTRL = regValue;
}

/*! @} */ /* End of Clock Output Control */

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

/*! @}*/

#endif /* CFMU_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
