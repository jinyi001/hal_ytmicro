/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file wku_hw_access.h
 * @version 1.4.1
 *
 * @brief WKU Hardware Access Layer.
 *
 * This header provides low-level inline register helpers for the Wakeup Unit
 * (WKU). It is used internally by the WKU driver to manipulate pin wakeup
 * flags, channel control bits, reset-pin wakeup settings, and internal module
 * wakeup enables.
 *
 * Functions are organized into these categories:
 *   - Pin Flag Management
 *   - Pin Wakeup Control
 *   - Reset Wakeup Control
 *   - Module Wakeup Control
 *
 * @note This is an internal layer. Application code should use the
 *       `WKU_DRV_*` APIs from wku_driver.h.
 */

#ifndef WKU_HW_ACCESS_H
#define WKU_HW_ACCESS_H

#include "wku_driver.h"

/*!
 * @addtogroup wku_hw_access WKU Hardware Access
 * @ingroup wku
 * @brief Low-level register access functions for the Wakeup Unit.
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Pin Flag Management
 ******************************************************************************/
/*!
 * @name Pin Flag Management
 * @brief Functions for acknowledging WKU pin wakeup flags.
 * @{
 */

/*!
 * @brief Clear the wakeup flag of one hardware channel.
 *
 * Writes the corresponding write-one-to-clear bit in the pin-flag register
 * bank selected by @a hwChannel.
 *
 * @param[in] base       Pointer to the WKU peripheral base address.
 * @param[in] hwChannel  Hardware channel index.
 */
static inline void WKU_ClearPinWakeupFlag(WKU_Type *const base,
                                          uint8_t hwChannel)
{
    /* Acknowledge the selected pin flag with a write-one-to-clear access. */
    base->PFR[(((uint32_t)hwChannel) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)hwChannel) & 0x1FUL));
}

/*! @} */ /* End of Pin Flag Management */

/*******************************************************************************
 * Pin Wakeup Control
 ******************************************************************************/
/*!
 * @name Pin Wakeup Control
 * @brief Functions for configuring individual external pin wakeup channels.
 * @{
 */

/*!
 * @brief Program the edge-detection mode for one pin wakeup channel.
 *
 * Clears the existing WUPE field and writes the requested edge-selection
 * value into the selected PCR entry.
 *
 * @param[in] base       Pointer to the WKU peripheral base address.
 * @param[in] hwChannel  Hardware channel index.
 * @param[in] edge       Edge-detection mode to program.
 */
static inline void WKU_SetWakeueEdgeEvent(WKU_Type *const base,
                                          uint8_t hwChannel,
                                          wku_edge_event_t edge)
{
    /* Clear the previous edge-selection bits before applying the new mode. */
    base->PCR[hwChannel] &= ~(WKU_PCR_WUPE_MASK);
    /* Program the requested edge-detection mode. */
    base->PCR[hwChannel] |= (uint8_t)(WKU_PCR_WUPE(edge) & 0xFFU);
}

/*!
 * @brief Enable or disable the digital filter for one pin wakeup channel.
 *
 * @param[in] base       Pointer to the WKU peripheral base address.
 * @param[in] hwChannel  Hardware channel index.
 * @param[in] enable     `true` to enable filtering, `false` to bypass it.
 */
static inline void WKU_EnableFilter(WKU_Type *const base,
                                    uint8_t hwChannel,
                                    bool enable)
{
    if (enable)
    {
        /* Enable the PCR filter bit for the selected channel. */
        base->PCR[hwChannel] |= WKU_PCR_WUPFE_MASK;
    }
    else
    {
        /* Clear the PCR filter bit for the selected channel. */
        base->PCR[hwChannel] &= ~WKU_PCR_WUPFE_MASK;
    }
}

/*!
 * @brief Enable or disable the pin interrupt path for one wakeup channel.
 *
 * @param[in] base       Pointer to the WKU peripheral base address.
 * @param[in] hwChannel  Hardware channel index.
 * @param[in] enable     `true` to enable the interrupt path, `false` to
 *                       disable it.
 */
static inline void WKU_EnablePinInterrupt(WKU_Type *const base,
                                          uint8_t hwChannel,
                                          bool enable)
{
    if (enable)
    {
        /* Set the PCR interrupt-enable bit for the selected channel. */
        base->PCR[hwChannel] |= WKU_PCR_WUPIE_MASK;
    }
    else
    {
        /* Clear the PCR interrupt-enable bit for the selected channel. */
        base->PCR[hwChannel] &= ~WKU_PCR_WUPIE_MASK;
    }
}

/*! @} */ /* End of Pin Wakeup Control */

/*******************************************************************************
 * Reset Wakeup Control
 ******************************************************************************/
/*!
 * @name Reset Wakeup Control
 * @brief Functions for configuring reset-pin wakeup behavior.
 * @{
 */

/*!
 * @brief Enable or disable reset-pin wakeup requests.
 *
 * @param[in] base    Pointer to the WKU peripheral base address.
 * @param[in] enable  `true` to enable reset-pin wakeup, `false` to disable it.
 */
static inline void WKU_EnableResetPin(WKU_Type *const base,
                                      bool enable)
{
    if (enable)
    {
        /* Enable reset-pin wakeup requests. */
        base->RPCR |= WKU_RPCR_WURPE_MASK;
    }
    else
    {
        /* Disable reset-pin wakeup requests. */
        base->RPCR &= ~WKU_RPCR_WURPE_MASK;
    }
}

/*!
 * @brief Enable or disable reset-pin digital filtering.
 *
 * @param[in] base    Pointer to the WKU peripheral base address.
 * @param[in] enable  `true` to enable reset-pin filtering, `false` to disable it.
 */
static inline void WKU_EnableResetPinFilter(WKU_Type *const base,
                                            bool enable)
{
    if (enable)
    {
        /* Enable reset-pin digital filtering. */
        base->RPCR |= WKU_RPCR_WURPFE_MASK;
    }
    else
    {
        /* Disable reset-pin digital filtering. */
        base->RPCR &= ~WKU_RPCR_WURPFE_MASK;
    }
}

#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
/*!
 * @brief Select the shared WKU filter clock source.
 *
 * Programs the filter-clock selection field used by the WKU filtering logic.
 *
 * @param[in] base  Pointer to the WKU peripheral base address.
 * @param[in] clk   Filter clock source selection.
 */
static inline void WKU_SetResetPinFilterClock(WKU_Type *const base,
                                              wku_filter_clock_source_t clk)
{
    /* Clear the previous filter clock selection. */
    base->RPCR &= ~WKU_RPCR_WURPFS_MASK;
    /* Apply the requested filter clock source. */
    base->RPCR |= WKU_RPCR_WURPFS(clk);
}
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */

/*! @} */ /* End of Reset Wakeup Control */

/*******************************************************************************
 * Module Wakeup Control
 ******************************************************************************/
/*!
 * @name Module Wakeup Control
 * @brief Functions for gating internal module wakeup sources.
 * @{
 */

/*!
 * @brief Enable or disable an internal module wakeup source.
 *
 * @param[in] base      Pointer to the WKU peripheral base address.
 * @param[in] moduleID  Internal module wakeup identifier.
 * @param[in] enable    `true` to enable the module wakeup source,
 *                      `false` to disable it.
 */
static inline void WKU_EnableModuleWakeup(WKU_Type *const base,
                                          uint8_t moduleID,
                                          bool enable)
{
    if (enable)
    {
        /* Set the module-enable bit for the selected wakeup source. */
        base->MER |= 0x01UL << moduleID;
    }
    else
    {
        /* Clear the module-enable bit for the selected wakeup source. */
        base->MER &= ~(0x01UL << moduleID);
    }
}

/*! @} */ /* End of Module Wakeup Control */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* WKU_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
