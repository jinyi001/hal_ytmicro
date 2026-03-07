/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*!
 * @file ewdg_hw_access.h
 * @version 1.4.0
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
 * @brief Refresh EWDG
 *
 * This method needs to be called within the window period
 * specified by the Compare Low and Compare High registers
 *
 * @param[in] base EWDG base pointer
 */
static inline void EWDG_Refresh(EWDG_Type * const base)
{
    /* Write first byte of the service key */
    base->SERV = FEATURE_EWDG_KEY_FIRST_BYTE;
    /* Write second byte of the service key */
    base->SERV = FEATURE_EWDG_KEY_SECOND_BYTE;
}

/*!
 * @brief Get the EWDG enable bit.
 *
 * @param[in] base EWDG base pointer
 * @return The state of the device enable bit:
 *      -   false - EWDG disabled
 *      -   true  - EWDG enabled
 */
static inline bool EWDG_IsEnabled(const EWDG_Type * base)
{
    return ((base->CTRL & EWDG_CTRL_EN_MASK) >> EWDG_CTRL_EN_SHIFT) != 0U;
}

/*!
 * @brief Set the Control Value.
 *
 * This register can be only written once after a CPU reset.
 *
 * @param[in] base  EWDG base pointer
 * @param[in] value Value to write into Control register
 */
static inline void EWDG_SetControl(EWDG_Type * const base, uint32_t value)
{
    base->CTRL = value;
}

/*!
 * @brief Get the Control register Value.
 *
 * @param[in] base EWDG base pointer
 * @return Value stored in Control register
 */
static inline uint32_t EWDG_GetControl(const EWDG_Type * base)
{
    return base->CTRL;
}

/*!
 * @brief Set the Compare Low Value.
 *
 * This register can be only written once after a CPU reset.
 * The user must make sure that the Compare High is greater than Compare Low value
 *
 * @param[in] base  EWDG base pointer
 * @param[in] value Value to write into Compare Low register
 */
static inline void EWDG_SetCompareLow(EWDG_Type * const base, uint16_t value)
{
    base->CMPL = value;
}


/*!
 * @brief Set the Compare High Value.
 *
 * This register can be only written once after a CPU reset.
 * The user must make sure that the Compare High is greater than Compare Low value
 * Note: The maximum Compare High value is 0xFE
 *
 * @param[in] base  EWDG base pointer
 * @param[in] value Value to write into Compare High register
 */
static inline void EWDG_SetCompareHigh(EWDG_Type * const base, uint16_t value)
{
    base->CMPH = value;
}


/*!
 * @brief Set the Clock Prescaler Value.
 *
 * This register can be only written once after a CPU reset and
 * it must be written before enabling the EWDG
 *
 * @param[in] base  EWDG base pointer
 * @param[in] value Prescaler Value
 */
static inline void EWDG_SetPrescaler(EWDG_Type * const base, uint8_t value)
{
    base->CLK_CTRL &= ~(EWDG_CLK_CTRL_PRESCALER_MASK);
    base->CLK_CTRL |= EWDG_CLK_CTRL_PRESCALER(value);
}

#if defined(EWDG_CLK_CTRL_CLKSEL_MASK)
/*!
 * @brief Set the Clock Source
 *
 * This register can be only written once after a CPU reset and
 * it must be written before enabling the EWDG
 *
 * @param[in] base  EWDG base pointer
 * @param[in] value Clock Source
 */

static inline void EWDG_SetClockSource(EWDG_Type * const base, uint8_t value)
{
    base->CLK_CTRL &= ~(EWDG_CLK_CTRL_CLKSEL_MASK);
    base->CLK_CTRL |= EWDG_CLK_CTRL_CLKSEL(value);
}
#endif

/*!
 * @brief DeInit the EWDG.
 *
 * This function is the deinit ewdg module, 
 * and the value of each register is the same as the value of the chip reset.
 *
 * @param[in] base  EWDG base pointer
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
