/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file scu_hw_access.h
 * @version 1.4.1
 *
 * @brief SCU Hardware Access — System Clock Unit register interface.
 *
 * This header provides low-level inline functions for direct register access
 * to the SCU (System Clock Unit) peripheral. It controls system clock source
 * selection, clock dividers, oscillators (FXOSC/SXOSC), PLL, IRC (FIRC/SIRC),
 * clock output routing, and CMU (Clock Monitor Unit) channels.
 *
 * Functions are organized into the following groups:
 *   - **System Clock Dividers** — Core, fast-bus, and slow-bus divider control.
 *   - **System Clock Source** — System clock MUX selection and readback.
 *   - **Clock Output** — CLKOUT source and divider (non-CFMU devices).
 *   - **FXOSC Control** — Fast external oscillator enable, bypass, gain, ALC.
 *   - **SXOSC Control** — Slow external oscillator enable, bypass, low-power modes.
 *   - **PLL Control** — PLL enable, reference clock, feedback/reference dividers.
 *   - **SIRC Control** — Slow IRC low-power mode retention settings.
 *   - **FIRC Control** — Fast IRC enable and low-power mode settings.
 *   - **Clock Status** — Oscillator/PLL valid flags and divider stability.
 *   - **CMU Configuration** — Clock monitor channel enable, reset, thresholds.
 */

#ifndef SCU_HW_ACCESS_H
#define SCU_HW_ACCESS_H

#include "device_registers.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*******************************************************************************
 * System Clock Dividers
 ******************************************************************************/
/*!
 * @name System Clock Dividers
 * @brief Get/set core clock, fast-bus, and slow-bus clock dividers.
 * @{
 */

/*!
 * @brief Set the system (core) clock divider.
 *
 * @param[in] base       Base address for current SCU instance.
 * @param[in] sysClkDiv  Divider value (0 = /1, 1 = /2, ... 15 = /16).
 */
static inline void SCU_SetSysClkDiv(SCU_Type* const base, uint8_t sysClkDiv)
{
    uint32_t regValue = base->DIV;
    regValue &= ~(SCU_DIV_CPDIVS_MASK);
    regValue |= SCU_DIV_CPDIVS(sysClkDiv);
    base->DIV = regValue;
}

/*!
 * @brief Get the current system (core) clock divider.
 *
 * @param[in] base  Base address for current SCU instance.
 * @return Current system clock divider value.
 */
static inline uint8_t SCU_GetSysClkDiv(SCU_Type* const base)
{
    return (uint8_t)((base->DIVSTS & SCU_DIVSTS_CPDIVST_MASK) >> SCU_DIVSTS_CPDIVST_SHIFT);
}

/*!
 * @brief Set the fast bus clock divider.
 *
 * @param[in] base          Base address for current SCU instance.
 * @param[in] fastBusClkDiv Fast bus clock divider value.
 */
static inline void SCU_SetFastBusClkDiv(SCU_Type* const base, uint16_t fastBusClkDiv)
{
    uint32_t regValue = base->DIV;
    regValue &= ~(SCU_DIV_FBDIVS_MASK);
    regValue |= SCU_DIV_FBDIVS(fastBusClkDiv);
    base->DIV  = regValue;
}

/*!
 * @brief Get the current fast bus clock divider.
 *
 * @param[in] base  Base address for current SCU instance.
 * @return Current fast bus clock divider value.
 */
static inline uint8_t SCU_GetFastBusClkDiv(SCU_Type* const base)
{
    return (uint8_t)((base->DIVSTS & SCU_DIVSTS_FBDIVST_MASK) >> SCU_DIVSTS_FBDIVST_SHIFT);
}

/*!
 * @brief Set the slow bus clock divider.
 *
 * @param[in] base          Base address for current SCU instance.
 * @param[in] slowBusClkDiv Slow bus clock divider value.
 */
static inline void SCU_SetSlowBusClkDiv(SCU_Type* const base, uint16_t slowBusClkDiv)
{
    uint32_t regValue = base->DIV;
    regValue &= ~(SCU_DIV_SBDIVS_MASK);
    regValue |= SCU_DIV_SBDIVS(slowBusClkDiv);
    base->DIV  = regValue;
}

/*!
 * @brief Get the current slow bus clock divider.
 *
 * @param[in] base  Base address for current SCU instance.
 * @return Current slow bus clock divider value.
 */
static inline uint8_t SCU_GetSlowBusClkDiv(SCU_Type* const base)
{
    return (uint8_t)((base->DIVSTS & SCU_DIVSTS_SBDIVST_MASK) >> SCU_DIVSTS_SBDIVST_SHIFT);
}

/*! @} */ /* End of System Clock Dividers */

/*******************************************************************************
 * System Clock Source
 ******************************************************************************/
/*!
 * @name System Clock Source
 * @brief Select and query the system clock source.
 * @{
 */

/*!
 * @brief Set the system clock source.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] clkSrc  Clock source selector: 0=FIRC, 1=PLL, 2=FXOSC, 3=SIRC.
 */
static inline void SCU_SetSysClkSrc(SCU_Type* const base, uint8_t clkSrc)
{
    uint32_t regValue = base->CLKS;
    regValue &= ~(SCU_CLKS_CLKSRCSEL_MASK);
    regValue |= SCU_CLKS_CLKSRCSEL(clkSrc);
    base->CLKS = regValue;
}

/*!
 * @brief Get the current system clock source.
 *
 * @param[in] base  Base address for current SCU instance.
 * @return Current clock source: 0=FIRC, 1=PLL, 2=FXOSC, 3=SIRC.
 */
static inline uint8_t SCU_GetSysClkSrc(SCU_Type* const base)
{
    return (uint8_t)((base->STS & SCU_STS_CLKST_MASK) >> SCU_STS_CLKST_SHIFT);
}

/*! @} */ /* End of System Clock Source */

/*******************************************************************************
 * Clock Output (non-CFMU devices)
 ******************************************************************************/
/*!
 * @name Clock Output
 * @brief Configure clock output source and divider (devices without separate CFMU).
 * @{
 */

#if defined(FEATURE_SOC_HAS_SEPARATE_CMU_AND_CLKOUT) && (FEATURE_SOC_HAS_SEPARATE_CMU_AND_CLKOUT == 0)
/*!
 * @brief Select clock output source.
 *
 * @param[in] base  Base address for current SCU instance.
 * @param[in] src   Clock output source: 0=Disable, 1=FIRC, 2=SIRC, 3=FXOSC, 4=SXOSC, 5=PLL.
 */
static inline void SCU_SetClkOutSrc(SCU_Type* const base, const uint8_t src)
{
    uint32_t regValue = base->CLKO;
    regValue &= ~(SCU_CLKO_CLKOSEL_MASK);
    regValue |= SCU_CLKO_CLKOSEL(src);
    base->CLKO = regValue;
}

/*!
 * @brief Set the clock output divider.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] clkDiv  Clock output divider value.
 */
static inline void SCU_SetClkOutDiv(SCU_Type *base, const uint8_t clkDiv)
{
    uint32_t regValue = base->CLKO;
    regValue &= (uint32_t)(~SCU_CLKO_CLKODIV_MASK);
    regValue |= (uint32_t)SCU_CLKO_CLKODIV(clkDiv);
    base->CLKO = regValue;
}
#endif /* FEATURE_SOC_HAS_SEPARATE_CMU_AND_CLKOUT */

/*! @} */ /* End of Clock Output */

/*******************************************************************************
 * FXOSC Control
 ******************************************************************************/
/*!
 * @name FXOSC Control
 * @brief Fast External Oscillator enable, bypass, gain, and ALC configuration.
 * @{
 */

/*!
 * @brief Enable or disable the fast external oscillator (FXOSC).
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] enable  true to enable FXOSC, false to disable.
 */
static inline void SCU_EnableFxosc(SCU_Type* const base, const bool enable)
{
    uint32_t regValue = base->FXOSC_CTRL;
    regValue &= (uint32_t)(~(SCU_FXOSC_CTRL_FXOSC_EN_MASK));
    regValue |= (uint32_t)SCU_FXOSC_CTRL_FXOSC_EN(enable ? 1U : 0U);
    base->FXOSC_CTRL = regValue;
}

/*!
 * @brief Get the FXOSC enable flag.
 *
 * @param[in] base  Base address for current SCU instance.
 * @return 1 if FXOSC is enabled, 0 if disabled.
 */
static inline uint8_t SCU_GetFxoscEnableFlag(SCU_Type* const base)
{
    return (uint8_t)(((base->FXOSC_CTRL) & SCU_FXOSC_CTRL_FXOSC_EN_MASK) >> SCU_FXOSC_CTRL_FXOSC_EN_SHIFT);
}

/*!
 * @brief Set FXOSC bypass mode.
 *
 * In bypass mode, an external clock signal is used directly instead of
 * the crystal oscillator feedback loop.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] enable  true for external clock input (bypass), false for crystal mode.
 */
static inline void SCU_SetFxoscBypassMode(SCU_Type* const base, const bool enable)
{
    uint32_t regValue = base->FXOSC_CTRL;
    regValue &= (uint32_t)(~(SCU_FXOSC_CTRL_FXOSC_MODE_MASK));
    regValue |= (uint32_t)SCU_FXOSC_CTRL_FXOSC_MODE(enable ? 0U : 1U);
    base->FXOSC_CTRL = regValue;
}

#if defined(SCU_FXOSC_CTRL_COMPEN_MASK)
/*!
 * @brief Enable or disable FXOSC compensation mode.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] enable  true to enable compensation, false to disable.
 */
static inline void SCU_SetFxoscComp(SCU_Type* const base, const bool enable)
{
    uint32_t regValue = base->FXOSC_CTRL;
    regValue &= (uint32_t)(~(SCU_FXOSC_CTRL_COMPEN_MASK));
    regValue |= (uint32_t)SCU_FXOSC_CTRL_COMPEN(enable ? 0U : 1U);
    base->FXOSC_CTRL = regValue;
}
#endif /* SCU_FXOSC_CTRL_COMPEN_MASK */

#if defined(SCU_FXOSC_CTRL_HYSTDIS_MASK)
/*!
 * @brief Enable or disable FXOSC hysteresis.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] enable  true to enable hysteresis, false to disable.
 */
static inline void SCU_SetFxoscHysteresis(SCU_Type* const base, const bool enable)
{
    uint32_t regValue = base->FXOSC_CTRL;
    regValue &= (uint32_t)(~(SCU_FXOSC_CTRL_HYSTDIS_MASK));
    regValue |= (uint32_t)SCU_FXOSC_CTRL_HYSTDIS(enable ? 0U : 1U);
    base->FXOSC_CTRL = regValue;
}
#endif /* SCU_FXOSC_CTRL_HYSTDIS_MASK */

#if defined(SCU_FXOSC_CTRL_GMSEL_MASK)
/*!
 * @brief Set FXOSC oscillator gain.
 *
 * @param[in] base  Base address for current SCU instance.
 * @param[in] gain  Gain selection value.
 */
static inline void SCU_SetFxoscGain(SCU_Type* const base, const uint8_t gain)
{
    uint32_t regValue = base->FXOSC_CTRL;
    regValue &= (uint32_t)(~(SCU_FXOSC_CTRL_GMSEL_MASK));
    regValue |= (uint32_t)SCU_FXOSC_CTRL_GMSEL(gain);
    base->FXOSC_CTRL = regValue;
}

/*!
 * @brief Enable or disable FXOSC Automatic Level Control (ALC).
 *
 * When ALC is enabled (autoGainDisable = false), the oscillator automatically
 * adjusts its gain. Set autoGainDisable = true to use a fixed gain.
 *
 * @param[in] base             Base address for current SCU instance.
 * @param[in] autoGainDisable  true to disable ALC (fixed gain), false to enable ALC.
 */
static inline void SCU_SetFxoscALC(SCU_Type* const base, const bool autoGainDisable)
{
    uint32_t regValue = base->FXOSC_CTRL;
    regValue &= (uint32_t)(~(SCU_FXOSC_CTRL_ALCDIS_MASK));
    regValue |= (uint32_t)SCU_FXOSC_CTRL_ALCDIS(autoGainDisable ? 1U : 0U);
    base->FXOSC_CTRL = regValue;
}
#endif /* SCU_FXOSC_CTRL_GMSEL_MASK */

/*! @} */ /* End of FXOSC Control */

/*******************************************************************************
 * SXOSC Control
 ******************************************************************************/
/*!
 * @name SXOSC Control
 * @brief Slow External Oscillator enable, bypass, gain, and low-power mode settings.
 * @{
 */

#if defined(FEATURE_SCU_SUPPORT_SXOSC) && (FEATURE_SCU_SUPPORT_SXOSC == 1)
/*!
 * @brief Enable or disable the slow external oscillator (SXOSC).
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] enable  true to enable SXOSC, false to disable.
 */
static inline void SCU_EnableSxosc(SCU_Type* const base, const bool enable)
{
    uint32_t regValue = base->SXOSC_CTRL;
    regValue &= (uint32_t)(~(SCU_SXOSC_CTRL_SXOSC_EN_MASK));
    regValue |= (uint32_t)SCU_SXOSC_CTRL_SXOSC_EN(enable ? 1U : 0U);
    base->SXOSC_CTRL = regValue;
}

/*!
 * @brief Enable or disable SXOSC during deep-sleep mode.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] enable  true to keep SXOSC running in deep-sleep, false to disable.
 */
static inline void SCU_SxoscDeepSleepEnable(SCU_Type* const base, const bool enable)
{
    uint32_t regValue = base->SXOSC_CTRL;
    regValue &= (uint32_t)(~(SCU_SXOSC_CTRL_SXOSC_DS_EN_MASK));
    regValue |= (uint32_t)SCU_SXOSC_CTRL_SXOSC_DS_EN(enable ? 1U : 0U);
    base->SXOSC_CTRL = regValue;
}

/*!
 * @brief Enable or disable SXOSC during standby mode.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] enable  true to keep SXOSC running in standby, false to disable.
 */
static inline void SCU_SxoscStandbyEnable(SCU_Type* const base, const bool enable)
{
    uint32_t regValue = base->SXOSC_CTRL;
    regValue &= (uint32_t)(~(SCU_SXOSC_CTRL_SXOSC_STB_EN_MASK));
    regValue |= (uint32_t)SCU_SXOSC_CTRL_SXOSC_STB_EN(enable ? 1U : 0U);
    base->SXOSC_CTRL = regValue;
}

#if (defined(FEATURE_SOC_SUPPORT_POWERDOWN_MODE) && (FEATURE_SOC_SUPPORT_POWERDOWN_MODE == 1U))
/*!
 * @brief Enable or disable SXOSC during power-down mode.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] enable  true to keep SXOSC running in power-down, false to disable.
 */
static inline void SCU_SxoscPowerDownEnable(SCU_Type* const base, const bool enable)
{
    uint32_t regValue = base->SXOSC_CTRL;
    regValue &= (uint32_t)(~(SCU_SXOSC_CTRL_SXOSC_PD_EN_MASK));
    regValue |= (uint32_t)SCU_SXOSC_CTRL_SXOSC_PD_EN(enable ? 1U : 0U);
    base->SXOSC_CTRL = regValue;
}
#endif

/*!
 * @brief Get the SXOSC enable flag.
 *
 * @param[in] base  Base address for current SCU instance.
 * @return 1 if SXOSC is enabled, 0 if disabled.
 */
static inline uint8_t SCU_GetSxoscEnableFlag(SCU_Type* const base)
{
    return (uint8_t)(((base->SXOSC_CTRL) & SCU_SXOSC_CTRL_SXOSC_EN_MASK) >> SCU_SXOSC_CTRL_SXOSC_EN_SHIFT);
}

/*!
 * @brief Set SXOSC bypass mode.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] enable  true for external clock input (bypass), false for crystal mode.
 */
static inline void SCU_SetSxoscBypassMode(SCU_Type* const base, const bool enable)
{
    uint32_t regValue = base->SXOSC_CTRL;
    regValue &= (uint32_t)(~(SCU_SXOSC_CTRL_SXOSC_MODE_MASK));
    regValue |= (uint32_t)SCU_SXOSC_CTRL_SXOSC_MODE(enable ? 0U : 1U);
    base->SXOSC_CTRL = regValue;
}

#if defined(SCU_SXOSC_CTRL_GMSEL_MASK)
/*!
 * @brief Set SXOSC oscillator gain.
 *
 * @param[in] base  Base address for current SCU instance.
 * @param[in] gain  Gain selection value.
 */
static inline void SCU_SetSxoscGain(SCU_Type* const base, const uint8_t gain)
{
    uint32_t regValue = base->SXOSC_CTRL;
    regValue &= (uint32_t)(~(SCU_SXOSC_CTRL_GMSEL_MASK));
    regValue |= (uint32_t)SCU_SXOSC_CTRL_GMSEL(gain);
    base->SXOSC_CTRL = regValue;
}
#endif /* SCU_SXOSC_CTRL_GMSEL_MASK */

#endif /* FEATURE_SCU_SUPPORT_SXOSC */

/*! @} */ /* End of SXOSC Control */

/*******************************************************************************
 * PLL Control
 ******************************************************************************/
/*!
 * @name PLL Control
 * @brief Phase-Locked Loop enable, reference clock, and divider configuration.
 * @{
 */

#if defined(FEATURE_SCU_SUPPORT_PLL) && (FEATURE_SCU_SUPPORT_PLL == 1U)
/*!
 * @brief Set the PLL reference clock source.
 *
 * @param[in] base       Base address for current SCU instance.
 * @param[in] pllRefClk  Reference clock selector: 0=FXOSC, 1=FIRC.
 */
static inline void SCU_SetPllRefClk(SCU_Type* const base, const uint8_t pllRefClk)
{
    uint32_t regValue = base->PLL_CTRL;
    regValue &= (uint32_t)(~(SCU_PLL_CTRL_REFCLKSRCSEL_MASK));
    regValue |= (uint32_t)SCU_PLL_CTRL_REFCLKSRCSEL(pllRefClk);
    base->PLL_CTRL = regValue;
}

/*!
 * @brief Get the current PLL reference clock source.
 *
 * @param[in] base  Base address for current SCU instance.
 * @return Reference clock selector: 0=FXOSC, 1=FIRC.
 */
static inline uint8_t SCU_GetPllRefClk(SCU_Type* const base)
{
    return (uint8_t)((base->PLL_CTRL & SCU_PLL_CTRL_REFCLKSRCSEL_MASK) >> SCU_PLL_CTRL_REFCLKSRCSEL_SHIFT);
}

/*!
 * @brief Set the PLL feedback divider (Ndiv).
 *
 * PLL output: Fout = Fref × Ndiv / (2 × Npre).
 *
 * @param[in] base           Base address for current SCU instance.
 * @param[in] pllFeedBackDiv Feedback divider value (Ndiv). Written as Ndiv-1.
 */
static inline void SCU_SetPllFeedBackDiv(SCU_Type* const base, const uint8_t pllFeedBackDiv)
{
    uint32_t regValue = base->PLL_CTRL;
    regValue &= (~(SCU_PLL_CTRL_FBDIV_MASK));
    regValue |= SCU_PLL_CTRL_FBDIV((uint32_t)pllFeedBackDiv - 1U);
    base->PLL_CTRL = regValue;
}

/*!
 * @brief Get the current PLL feedback divider (Ndiv).
 *
 * @param[in] base  Base address for current SCU instance.
 * @return Feedback divider value (register value + 1).
 */
static inline uint8_t SCU_GetPllFeedBackDiv(SCU_Type* const base)
{
    return (uint8_t)(((base->PLL_CTRL & SCU_PLL_CTRL_FBDIV_MASK) >> SCU_PLL_CTRL_FBDIV_SHIFT) + 1U);
}

/*!
 * @brief Set the PLL reference clock divider (Npre).
 *
 * @param[in] base          Base address for current SCU instance.
 * @param[in] pllRefClkDiv  Reference divider value (Npre). Written as Npre-1.
 */
static inline void SCU_SetPllRefClkDiv(SCU_Type* const base, const uint8_t pllRefClkDiv)
{
    uint32_t regValue = base->PLL_CTRL;
    regValue &= (~(SCU_PLL_CTRL_REFDIV_MASK));
    regValue |= SCU_PLL_CTRL_REFDIV((uint32_t)pllRefClkDiv - 1U);
    base->PLL_CTRL = regValue;
}

/*!
 * @brief Get the current PLL reference clock divider (Npre).
 *
 * @param[in] base  Base address for current SCU instance.
 * @return Reference divider value (register value + 1).
 */
static inline uint8_t SCU_GetPllRefClkDiv(SCU_Type* const base)
{
    return (uint8_t)(((base->PLL_CTRL & SCU_PLL_CTRL_REFDIV_MASK) >> SCU_PLL_CTRL_REFDIV_SHIFT) + 1U);
}

/*!
 * @brief Enable or disable the PLL.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] enable  true to enable PLL, false to disable.
 */
static inline void SCU_EnablePll(SCU_Type* const base, const bool enable)
{
    uint32_t regValue = base->PLL_CTRL;
    regValue &= (uint32_t)(~(SCU_PLL_CTRL_PLL_EN_MASK));
    regValue |= (uint32_t)SCU_PLL_CTRL_PLL_EN(enable ? 1U : 0U);
    base->PLL_CTRL = regValue;
}

/*!
 * @brief Get the PLL lock status.
 *
 * @param[in] base  Base address for current SCU instance.
 * @return true if PLL is locked, false otherwise.
 */
static inline bool SCU_GetPllLock(SCU_Type* const base)
{
    return ((base->STS & SCU_STS_PLL_LOCK_MASK) >> SCU_STS_PLL_LOCK_SHIFT) != 0U;
}
#endif /* FEATURE_SCU_SUPPORT_PLL */

/*! @} */ /* End of PLL Control */

/*******************************************************************************
 * SIRC Control
 ******************************************************************************/
/*!
 * @name SIRC Control
 * @brief Slow Internal RC oscillator low-power mode retention settings.
 * @{
 */

/*!
 * @brief Keep SIRC running during deep-sleep mode.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] enable  true to force SIRC on in deep-sleep, false to allow shutdown.
 */
static inline void SCU_SircDeepSleepEnable(SCU_Type *base, const bool enable)
{
    uint32_t regValue = base->SIRC_CTRL;
    regValue &= (uint32_t)(~SCU_SIRC_CTRL_SIRC_DS_EN_MASK);
    regValue |= (uint32_t)SCU_SIRC_CTRL_SIRC_DS_EN(enable ? 1U : 0U);
    base->SIRC_CTRL = regValue;
}

/*!
 * @brief Keep SIRC running during standby mode.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] enable  true to force SIRC on in standby, false to allow shutdown.
 */
static inline void SCU_SircStandbyEnable(SCU_Type *base, const bool enable)
{
    uint32_t regValue = base->SIRC_CTRL;
    regValue &= (uint32_t)(~SCU_SIRC_CTRL_SIRC_STB_EN_MASK);
    regValue |= (uint32_t)SCU_SIRC_CTRL_SIRC_STB_EN(enable ? 1U : 0U);
    base->SIRC_CTRL = regValue;
}

#if defined(FEATURE_SOC_SUPPORT_POWERDOWN_MODE) && (FEATURE_SOC_SUPPORT_POWERDOWN_MODE == 1)
/*!
 * @brief Keep SIRC running during power-down mode.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] enable  true to force SIRC on in power-down, false to allow shutdown.
 */
static inline void SCU_SircPowerDownEnable(SCU_Type *base, const bool enable)
{
    uint32_t regValue = base->SIRC_CTRL;
    regValue &= (uint32_t)(~SCU_SIRC_CTRL_SIRC_PD_EN_MASK);
    regValue |= (uint32_t)SCU_SIRC_CTRL_SIRC_PD_EN(enable ? 1U : 0U);
    base->SIRC_CTRL = regValue;
}

#endif /* FEATURE_SOC_SUPPORT_POWERDOWN_MODE */

/*! @} */ /* End of SIRC Control */

/*******************************************************************************
 * FIRC Control
 ******************************************************************************/
/*!
 * @name FIRC Control
 * @brief Fast Internal RC oscillator enable and low-power mode settings.
 * @{
 */

/*!
 * @brief Enable or disable the Fast Internal RC oscillator (FIRC).
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] enable  true to enable FIRC, false to disable.
 */
static inline void SCU_EnableFirc(SCU_Type *base, const bool enable)
{
    uint32_t regValue = base->FIRC_CTRL;
    regValue &= (uint32_t)(~SCU_FIRC_CTRL_FIRC_EN_MASK);
    regValue |= (uint32_t)SCU_FIRC_CTRL_FIRC_EN(enable ? 1U : 0U);
    base->FIRC_CTRL = regValue;
}

/*!
 * @brief Keep FIRC running during deep-sleep mode.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] enable  true to force FIRC on in deep-sleep, false to allow shutdown.
 */
static inline void SCU_FircDeepSleepEnable(SCU_Type *base, const bool enable)
{
    uint32_t regValue = base->FIRC_CTRL;
    regValue &= (uint32_t)(~SCU_FIRC_CTRL_FIRC_DS_EN_MASK);
    regValue |= (uint32_t)SCU_FIRC_CTRL_FIRC_DS_EN(enable ? 1U : 0U);
    base->FIRC_CTRL = regValue;
}

/*! @} */ /* End of FIRC Control */

/*******************************************************************************
 * Clock Status
 ******************************************************************************/
/*!
 * @name Clock Status
 * @brief Query oscillator/PLL validity and divider stability.
 * @{
 */

/*!
 * @brief Get the FIRC valid status.
 *
 * @param[in] base  Base address for current SCU instance.
 * @return true if FIRC is valid, false otherwise.
 */
static inline bool SCU_GetFIRCValid(SCU_Type* const base)
{
    return ((base->STS & SCU_STS_FIRC_VALID_MASK) >> SCU_STS_FIRC_VALID_SHIFT) != 0U;
}

/*!
 * @brief Get the FIRC enable flag.
 *
 * @param[in] base  Base address for current SCU instance.
 * @return 1 if FIRCEN bit is set, 0 otherwise.
 */
static inline uint8_t SCU_GetFircEnableFlag(SCU_Type* const base)
{
    return (uint8_t)(((base->FIRC_CTRL) & SCU_FIRC_CTRL_FIRC_EN_MASK) >> SCU_FIRC_CTRL_FIRC_EN_SHIFT);
}

/*!
 * @brief Get the FXOSC valid status.
 *
 * @param[in] base  Base address for current SCU instance.
 * @return true if FXOSC is valid, false otherwise.
 */
static inline bool SCU_GetFXOSCValid(SCU_Type* const base)
{
    return ((base->STS & SCU_STS_FXOSC_VALID_MASK) >> SCU_STS_FXOSC_VALID_SHIFT) != 0U;
}

#if defined(FEATURE_SCU_SUPPORT_SXOSC) && (FEATURE_SCU_SUPPORT_SXOSC == 1)
/*!
 * @brief Get the SXOSC valid status.
 *
 * @param[in] base  Base address for current SCU instance.
 * @return true if SXOSC is valid, false otherwise.
 */
static inline bool SCU_GetSXOSCValid(SCU_Type* const base)
{
    return ((base->STS & SCU_STS_SXOSC_VALID_MASK) >> SCU_STS_SXOSC_VALID_SHIFT) != 0U;
}
#endif /* FEATURE_SCU_SUPPORT_SXOSC */

/*!
 * @brief Check if system clock dividers are stable.
 *
 * Compares the DIV and DIVSTS registers to determine if the divider
 * change has taken effect.
 *
 * @param[in] base  Base address for current SCU instance.
 * @return true if dividers are stable, false if still transitioning.
 */
static inline bool SCU_GetDividerStable(SCU_Type* const base)
{
    uint32_t regValue = base->DIVSTS;
    return (regValue == base->DIV);
}

/*! @} */ /* End of Clock Status */

/*******************************************************************************
 * CMU Configuration (SCU-integrated CMU)
 ******************************************************************************/
/*!
 * @name CMU Configuration
 * @brief Clock Monitor Unit channel control on devices with SCU-integrated CMU.
 * @{
 */

#if defined(FEATURE_SOC_HAS_SEPARATE_CMU_AND_CLKOUT) && (FEATURE_SOC_HAS_SEPARATE_CMU_AND_CLKOUT == 0)
/*!
 * @brief Enable or disable a CMU monitoring channel.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] ch      CMU channel index.
 * @param[in] enable  true to enable, false to disable.
 */
static inline void SCU_SetCmuChEnable(SCU_Type *base, const uint8_t ch, const bool enable)
{
    uint32_t regValue = base->CMU_CTRL;
    regValue &= (~((uint32_t)0x01U << (SCU_CMU_CTRL_CMU0_EN_SHIFT + ch)));
    base->CMU_CTRL = regValue | ((uint32_t)(enable ? 1U : 0U) << (SCU_CMU_CTRL_CMU0_EN_SHIFT + ch));
}

/*!
 * @brief Enable or disable system reset on CMU channel clock error.
 *
 * @param[in] base    Base address for current SCU instance.
 * @param[in] ch      CMU channel index.
 * @param[in] enable  true to enable reset on error, false to disable.
 */
static inline void SCU_SetCmuChResetEnable(SCU_Type *base, const uint8_t ch, const bool enable)
{
    uint32_t regValue = base->CMU_CTRL;
    regValue &= (~((uint32_t)0x01U << (SCU_CMU_CTRL_CMU0_RE_SHIFT + ch)));
    base->CMU_CTRL = regValue | ((uint32_t)(enable ? 1U : 0U) << (SCU_CMU_CTRL_CMU0_RE_SHIFT + ch));
}

#if defined(FEATURE_SCU_SUPPORT_CMU_REFCLK_SEL) && (FEATURE_SCU_SUPPORT_CMU_REFCLK_SEL)
/*!
 * @brief Select the reference clock for a CMU channel.
 *
 * @param[in] base      Base address for current SCU instance.
 * @param[in] ch        CMU channel index.
 * @param[in] refClock  Reference clock: 0=SIRC, 1=FXOSC.
 */
static inline void SCU_SetCmuChRefClock(SCU_Type *base, const uint8_t ch, const uint8_t refClock)
{
    uint32_t regValue = base->CMU_CTRL;
    regValue &= (~((uint32_t)0x01U << (SCU_CMU_CTRL_CMU0_REFS_SHIFT + ch)));
    base->CMU_CTRL = regValue | ((uint32_t)refClock << (SCU_CMU_CTRL_CMU0_REFS_SHIFT + ch));
}
#endif /* FEATURE_SCU_SUPPORT_CMU_REFCLK_SEL */

/*!
 * @brief Set the high compare threshold for a CMU channel.
 *
 * @param[in] base   Base address for current SCU instance.
 * @param[in] ch     CMU channel index.
 * @param[in] value  High threshold compare value.
 */
static inline void SCU_SetCmuChCompHigh(SCU_Type *base, const uint8_t ch, const uint16_t value)
{
    base->CMUCMP[ch].HIGH = value;
}

/*!
 * @brief Set the low compare threshold for a CMU channel.
 *
 * @param[in] base   Base address for current SCU instance.
 * @param[in] ch     CMU channel index.
 * @param[in] value  Low threshold compare value.
 */
static inline void SCU_SetCmuChCompLow(SCU_Type *base, const uint8_t ch, const uint16_t value)
{
    base->CMUCMP[ch].LOW = value;
}
#endif /* FEATURE_SOC_HAS_SEPARATE_CMU_AND_CLKOUT */

/*! @} */ /* End of CMU Configuration */

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* SCU_HW_ACCESS_H */

/*! @}*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
