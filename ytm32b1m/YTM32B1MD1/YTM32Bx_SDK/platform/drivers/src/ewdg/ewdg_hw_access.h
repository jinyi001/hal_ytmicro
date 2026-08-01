/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*!
 * @file ewdg_hw_access.h
 * @version 1.4.1
 *
 * @brief EWDG HW Access Layer — inline register-level helper functions.
 *
 * This internal header provides static-inline helpers for direct access to the
 * EWDG peripheral registers. These helpers are used by the driver layer to
 * refresh the watchdog, program the startup window, query enable state, and
 * restore reset-like values.
 */

#ifndef EWDG_HW_ACCESS_H
#define EWDG_HW_ACCESS_H

#include "ewdg_driver.h"

/*******************************************************************************
* Definitions
******************************************************************************/


/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Refresh the EWDG counter.
 *
 * This helper writes the two-key watchdog service sequence. It must be called
 * while the counter is inside the valid refresh window defined by the compare
 * registers.
 *
 * @param[in] base  EWDG base pointer.
 */
static inline void EWDG_Refresh(EWDG_Type * const base)
{
    /* Write first byte of the service key */
    base->SERV = FEATURE_EWDG_KEY_FIRST_BYTE;
    /* Write second byte of the service key */
    base->SERV = FEATURE_EWDG_KEY_SECOND_BYTE;
}

/*!
 * @brief Read the EWDG enable bit.
 *
 * @param[in] base  EWDG base pointer.
 * @return `true` when EWDG is enabled, or `false` otherwise.
 */
static inline bool EWDG_IsEnabled(const EWDG_Type * base)
{
    return ((base->CTRL & EWDG_CTRL_EN_MASK) >> EWDG_CTRL_EN_SHIFT) != 0U;
}

/*!
 * @brief Write the EWDG control register.
 *
 * This register can be written only once after a CPU reset.
 *
 * @param[in] base   EWDG base pointer.
 * @param[in] value  Control-register value to write.
 */
static inline void EWDG_SetControl(EWDG_Type * const base, uint32_t value)
{
    base->CTRL = value;
}

/*!
 * @brief Read the EWDG control register.
 *
 * @param[in] base  EWDG base pointer.
 * @return Raw control-register value.
 */
static inline uint32_t EWDG_GetControl(const EWDG_Type * base)
{
    return base->CTRL;
}

/*!
 * @brief Write the lower refresh-window boundary.
 *
 * This register can be written only once after a CPU reset. The caller must
 * ensure that the programmed compare-high value remains greater than
 * compare-low.
 *
 * @param[in] base   EWDG base pointer.
 * @param[in] value  Compare-low value to write.
 */
static inline void EWDG_SetCompareLow(EWDG_Type * const base, uint16_t value)
{
    base->CMPL = value;
}


/*!
 * @brief Write the upper refresh-window boundary.
 *
 * This register can be written only once after a CPU reset. The caller must
 * ensure that compare-high is greater than compare-low. The maximum supported
 * compare-high value is device-dependent and validated by the driver layer.
 *
 * @param[in] base   EWDG base pointer.
 * @param[in] value  Compare-high value to write.
 */
static inline void EWDG_SetCompareHigh(EWDG_Type * const base, uint16_t value)
{
    base->CMPH = value;
}


/*!
 * @brief Program the EWDG clock prescaler.
 *
 * This register can be written only once after a CPU reset and must be
 * programmed before enabling the module.
 *
 * @param[in] base   EWDG base pointer.
 * @param[in] value  Prescaler value.
 */
static inline void EWDG_SetPrescaler(EWDG_Type * const base, uint8_t value)
{
    base->CLK_CTRL &= ~(EWDG_CLK_CTRL_PRESCALER_MASK);
    base->CLK_CTRL |= EWDG_CLK_CTRL_PRESCALER(value);
}

#if defined(EWDG_CLK_CTRL_CLKSEL_MASK)
/*!
 * @brief Program the EWDG clock source.
 *
 * This register can be written only once after a CPU reset and must be
 * programmed before enabling the module.
 *
 * @param[in] base   EWDG base pointer.
 * @param[in] value  Clock-source selector value.
 */
static inline void EWDG_SetClockSource(EWDG_Type * const base, uint8_t value)
{
    base->CLK_CTRL &= ~(EWDG_CLK_CTRL_CLKSEL_MASK);
    base->CLK_CTRL |= EWDG_CLK_CTRL_CLKSEL(value);
}
#endif

/*!
 * @brief Restore the EWDG register block to reset-like values.
 *
 * This helper clears the control and clock configuration, restores the compare
 * registers to their reset defaults, and refreshes the watchdog counter.
 *
 * @param[in] base  EWDG base pointer.
 */
static inline void  EWDG_DeInit(EWDG_Type * const base)
{
   /*Disable EWDG module, set input EWDG_IN pin directly,
    disable EWDG_IN pin, clear EWDG interrupt.*/
    base->CTRL = 0x00;
    /*Clock source select SIRC clock, clear the prescaler of clock.*/
    base->CLK_CTRL = 0x00;

    /*Default the CMPL register value*/
    base->CMPL = FEATURE_EWDG_CMPL_MIN_VALUE;
    /*Default the CMPM register value*/
    base->CMPH = FEATURE_EWDG_CMPH_MAX_VALUE + 1U;

    /* Refresh counter value */
    EWDG_Refresh(base);
}

#if defined(__cplusplus)
}
#endif

#endif /* EWDG_HW_ACCESS_H */

/*! @}*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
