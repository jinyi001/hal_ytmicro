/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file wku_hw_access.h
 * @version 1.4.1
 */

#ifndef WKU_HW_ACCESS_H
#define WKU_HW_ACCESS_H

#include "wku_driver.h"

/*!
 * wku_hw_access WKU Hardware Access
 * @details This section describes the programming interface of the WKU Hardware Access.
 * @{
 */

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @name WKU Hardware Access
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Clears pin wakeup flag
 *
 * This function clears pin wakeup flag
 *
 * @param[in] base The WKU peripheral base address
 * @param[in] hwChannel The hardware channel index
 */
static inline void WKU_ClearPinWakeupFlag(WKU_Type *const base,
                                          uint8_t hwChannel)
{
    /* Clear ISR flag */
    base->PFR[(((uint32_t)hwChannel) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)hwChannel) & 0x1FUL));
}

/*!
 * @brief Udpate wakeup pin edge event
 *
 * This function Udpate wakeup pin edge event
 *
 * @param[in] base The WKU peripheral base address
 * @param[in] hwChannel The hardware channel index
 * @param[in] edge wakeup pin event
 */
static inline void WKU_SetWakeueEdgeEvent(WKU_Type *const base,
                                          uint8_t hwChannel,
                                          wku_edge_event_t edge)
{
    /* reset edge configuration */
    base->PCR[hwChannel] &= ~(WKU_PCR_WUPE_MASK);
    /* update edge configuration */
    base->PCR[hwChannel] |= (uint8_t)(WKU_PCR_WUPE(edge)& 0xFFU);
}

/*!
 * @brief Enables or disables filter
 *
 * This function enables or disables filter
 *
 * @param[in] base The WKU peripheral base address
 * @param[in] hwChannel The hardware channel index
 * @param[in] enable Enables or disables filter
 */
static inline void WKU_EnableFilter(WKU_Type *const base,
                                    uint8_t hwChannel,
                                    bool enable)
{
    /* Enables Wakeup/PinWakeup Filter Enable Register */
    if (enable)
    {
        base->PCR[hwChannel] |= WKU_PCR_WUPFE_MASK;
    }
        /* Disables Wakeup/PinWakeup Filter Enable Register */
    else
    {
        base->PCR[hwChannel] &= ~WKU_PCR_WUPFE_MASK;
    }
}

/*!
 * @brief Enables or disables interrupt
 *
 * This function enables or disables interrupt request
 *
 * @param[in] base The WKU peripheral base address
 * @param[in] hwChannel The hardware channel index
 * @param[in] enable Enables or disables interrupt
 */
static inline void WKU_EnablePinInterrupt(WKU_Type *const base,
                                          uint8_t hwChannel,
                                          bool enable)
{
    if (enable)
    {
        base->PCR[hwChannel] |= WKU_PCR_WUPIE_MASK;
    } else
    {
        base->PCR[hwChannel] &= ~WKU_PCR_WUPIE_MASK;
    }
}

/*!
 * @brief Enables or disables reset pin wakeup
 *
 * This function enables or disables reset pin wakeup request
 *
 * @param[in] base The WKU peripheral base address
 * @param[in] enable Enables or disables reset pin wakeup
 */
static inline void WKU_EnableResetPin(WKU_Type *const base,
                                      bool enable)
{
    if (enable)
    {
        base->RPCR |= WKU_RPCR_WURPE_MASK;
    } else
    {
        base->RPCR &= ~WKU_RPCR_WURPE_MASK;
    }
}

/*!
 * @brief Enables or disables reset pin wakeup pin filter
 *
 * This function enables or disables reset pin wakeup request
 *
 * @param[in] base The WKU peripheral base address
 * @param[in] enable Enables or disables reset pin wakeup filter
 */
static inline void WKU_EnableResetPinFilter(WKU_Type *const base,
                                            bool enable)
{
    if (enable)
    {
        base->RPCR |= WKU_RPCR_WURPFE_MASK;
    } else
    {
        base->RPCR &= ~WKU_RPCR_WURPFE_MASK;
    }
}

/*!
 * @brief Enables or disables reset pin wakeup pin filter
 *
 * This function enables or disables reset pin wakeup request
 *
 * @param[in] base The WKU peripheral base address
 * @param[in] clk The WKU reset pin filter clock source
 * @param[in] enable Enables or disables module wakeup
 */
static inline void WKU_EnableModuleWakeup(WKU_Type *const base,
                                          uint8_t moduleID, bool enable)
{
    if (enable)
    {
        base->MER |= 0x01UL << moduleID;
    } else
    {
        base->MER &= ~(0x01UL << moduleID);
    }
}

#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
/*!
 * @brief Enables or disables module wakeup request
 *
 * This function enables or disables module wakeup request
 *
 * @param[in] base The WKU peripheral base address
 * @param[in] moduleID The WKU module ID 
 */
static inline void WKU_SetResetPinFilterClock(WKU_Type *const base,
                                              wku_filter_clock_source_t clk)
{
    base->RPCR &= ~WKU_RPCR_WURPFS_MASK;
    base->RPCR |= WKU_RPCR_WURPFS(clk);
}
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */
/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* WKU_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
