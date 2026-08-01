/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file wku_driver.c
 * @version 1.4.1
 *
 * @brief WKU Driver - implementation of the public WKU_DRV_* API.
 *
 * This file implements the application-level Wakeup Unit driver declared in
 * wku_driver.h. Each function resolves the WKU instance base address and then
 * delegates the register-level work to the internal helpers in
 * wku_hw_access.h.
 */

#include <stddef.h>
#include "wku_hw_access.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Table of base addresses for WKU peripheral instances. */
static WKU_Type *const s_wkuBase[] = WKU_BASE_PTRS;

/*******************************************************************************
 * Pin Wakeup Initialization & De-initialization
 ******************************************************************************/

/*!
 * @brief Initialize a set of external pin wakeup channels.
 */
status_t WKU_DRV_InitPinWakeup(uint32_t instance,
                               uint8_t channelCnt,
                               const wku_pin_wakeup_cfg_t *pPinWakeupConfig)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    DEV_ASSERT(pPinWakeupConfig != NULL);
    DEV_ASSERT(channelCnt > 0U);
    DEV_ASSERT(channelCnt <= FEATURE_WKU_MAX_CHANNEL_COUNT);
    uint8_t i;

    /* Apply each caller-provided channel configuration entry. */
    for (i = 0U; i < channelCnt; i++)
    {
        WKU_DRV_SetPinWakeupConfig(instance, &pPinWakeupConfig[i]);
    }

    return STATUS_SUCCESS;
}

/*!
 * @brief Clear all external pin wakeup channel settings.
 */
status_t WKU_DRV_DeinitPinWakeup(uint32_t instance)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    uint8_t i;

    /* Clear every supported hardware channel. */
    for (i = 0U; i < FEATURE_WKU_MAX_CHANNEL_COUNT; i++)
    {
        WKU_DRV_ClearPinWakeupConfig(instance, i);
    }

    return STATUS_SUCCESS;
}

/*!
 * @brief Populate the default pin wakeup configuration table.
 */
void WKU_DRV_GetPinWakeupDefaultConfig(wku_pin_wakeup_cfg_t *const pPinWakeupConfig)
{
    DEV_ASSERT(pPinWakeupConfig != NULL);

    uint8_t i;

    for (i = 0U; i < FEATURE_WKU_MAX_CHANNEL_COUNT; i++)
    {
        pPinWakeupConfig[i].hwChannel = i;
        pPinWakeupConfig[i].edgeEvent = WKU_EDGE_BOTH;
        pPinWakeupConfig[i].filterEn = false;
#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
        pPinWakeupConfig[i].filterClkSrc = WKU_FILTER_CLK_SIRC;
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */
    }
}

/*******************************************************************************
 * Pin Wakeup Channel Control
 ******************************************************************************/

/*!
 * @brief Apply the configuration for one external pin wakeup channel.
 */
void WKU_DRV_SetPinWakeupConfig(uint32_t instance,
                                const wku_pin_wakeup_cfg_t *pPinWakeupConfig)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    DEV_ASSERT(pPinWakeupConfig != NULL);

    WKU_Type *base = s_wkuBase[instance];
    uint8_t hwChannel = pPinWakeupConfig->hwChannel;

    /* Clear any previously latched wakeup flag on the target channel. */
    WKU_ClearPinWakeupFlag(base, hwChannel);

    /* Apply the per-channel filter enable state. */
    WKU_EnableFilter(base, hwChannel, pPinWakeupConfig->filterEn);

#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
    /* Update the shared filter clock only when filtering is requested. */
    if (pPinWakeupConfig->filterEn)
    {
        WKU_SetResetPinFilterClock(base, pPinWakeupConfig->filterClkSrc);
    }
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */

    /* Enable the WKU interrupt path for the selected pin channel. */
    WKU_EnablePinInterrupt(base, hwChannel, true);

    /* Program the requested edge-detection mode. */
    WKU_DRV_SetPinWakeupEdgeEvent(instance, hwChannel, pPinWakeupConfig->edgeEvent);
}

/*!
 * @brief Clear the configuration of one external pin wakeup channel.
 */
void WKU_DRV_ClearPinWakeupConfig(uint32_t instance,
                                  uint8_t hwChannel)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    DEV_ASSERT(hwChannel < FEATURE_WKU_MAX_CHANNEL_COUNT);

    WKU_Type *base = s_wkuBase[instance];

    /* Clear any pending flag before disabling the channel. */
    WKU_ClearPinWakeupFlag(base, hwChannel);

    /* Disable channel filtering. */
    WKU_EnableFilter(base, hwChannel, false);

#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
    /* Restore the default shared filter clock selection. */
    WKU_SetResetPinFilterClock(base, WKU_FILTER_CLK_SIRC);
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */

    /* Disable the channel interrupt path. */
    WKU_EnablePinInterrupt(base, hwChannel, false);

    /* Remove edge detection for the selected channel. */
    WKU_DRV_SetPinWakeupEdgeEvent(instance, hwChannel, WKU_EDGE_NONE);
}

/*!
 * @brief Update the edge-detection mode of one pin wakeup channel.
 */
void WKU_DRV_SetPinWakeupEdgeEvent(uint32_t instance,
                                   uint8_t hwChannel,
                                   wku_edge_event_t edge)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    DEV_ASSERT(hwChannel < FEATURE_WKU_MAX_CHANNEL_COUNT);

    WKU_Type *base = s_wkuBase[instance];
    WKU_SetWakeueEdgeEvent(base, hwChannel, edge);
}

/*!
 * @brief Clear the wakeup flag of one pin channel.
 */
void WKU_DRV_ClearPinWakeupFlag(uint32_t instance,
                                uint8_t hwChannel)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    DEV_ASSERT(hwChannel < FEATURE_WKU_MAX_CHANNEL_COUNT);

    WKU_Type *base = s_wkuBase[instance];

    /* Acknowledge the selected wakeup flag. */
    WKU_ClearPinWakeupFlag(base, hwChannel);
}

/*******************************************************************************
 * Reset Wakeup Control
 ******************************************************************************/

/*!
 * @brief Initialize reset-pin wakeup handling.
 */
void WKU_DRV_InitReset(uint32_t instance,
                       const wku_reset_cfg_t *pResetConfig)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    DEV_ASSERT(pResetConfig != NULL);

    WKU_Type *base = s_wkuBase[instance];

    /* Program the reset-pin wakeup enable state. */
    WKU_EnableResetPin(base, pResetConfig->wakeupEn);
    /* Apply the reset-pin filter enable state. */
    WKU_EnableResetPinFilter(base, pResetConfig->filterEn);
#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
    /* Select the requested shared filter clock source. */
    WKU_SetResetPinFilterClock(base, pResetConfig->filterClkSrc);
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */
}

/*!
 * @brief Restore the reset-pin wakeup path to its cleared state.
 */
void WKU_DRV_DeinitReset(uint32_t instance)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);

    WKU_Type *base = s_wkuBase[instance];

    /* Disable reset-pin wakeup requests. */
    WKU_EnableResetPin(base, false);
    /* Disable reset-pin filtering. */
    WKU_EnableResetPinFilter(base, false);
#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
    /* Restore the default shared filter clock source. */
    WKU_SetResetPinFilterClock(base, WKU_FILTER_CLK_SIRC);
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */
}

/*!
 * @brief Populate the default reset-pin wakeup configuration.
 */
void WKU_DRV_GetResetDefaultConfig(wku_reset_cfg_t *pResetConfig)
{
    DEV_ASSERT(pResetConfig != NULL);

    pResetConfig->wakeupEn = true;
    pResetConfig->filterEn = true;
#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
    pResetConfig->filterClkSrc = WKU_FILTER_CLK_SIRC;
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */
}

/*******************************************************************************
 * Internal Module Wakeup Control
 ******************************************************************************/

/*!
 * @brief Enable an internal module wakeup request.
 */
void WKU_DRV_EnableModuleWakeup(uint32_t instance, uint8_t moduleID)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    DEV_ASSERT(moduleID < FEATURE_WKU_MAX_CHANNEL_COUNT);
    WKU_Type *base = s_wkuBase[instance];
    WKU_EnableModuleWakeup(base, moduleID, true);
}

/*!
 * @brief Disable an internal module wakeup request.
 */
void WKU_DRV_DisableModuleWakeup(uint32_t instance, uint8_t moduleID)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    DEV_ASSERT(moduleID < FEATURE_WKU_MAX_CHANNEL_COUNT);
    WKU_Type *base = s_wkuBase[instance];
    WKU_EnableModuleWakeup(base, moduleID, false);
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
