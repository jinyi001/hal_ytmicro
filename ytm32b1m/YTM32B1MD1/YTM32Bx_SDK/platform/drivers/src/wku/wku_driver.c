/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file wku_driver.c
 * @version 1.4.1
 */

#include <stddef.h>
#include "wku_hw_access.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Table of base addresses for WKU instances. */
static WKU_Type *const s_wkuBase[] = WKU_BASE_PTRS;

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Local functions
 ******************************************************************************/
/*******************************************************************************
 * Code
 ******************************************************************************/



/*FUNCTION**********************************************************************
 *
 * Function Name : WKU_DRV_InitPinWakeup
 * Description   : This function initializes pin wakeup WKU driver based on user configuration input.
 * The channelCnt takes values between 1 and the maximum channel count supported by the hardware.
 *
 * Implements    : WKU_DRV_InitPinWakeup_Activity
 *END**************************************************************************/
status_t WKU_DRV_InitPinWakeup(uint32_t instance,
                               uint8_t channelCnt,
                               const wku_pin_wakeup_cfg_t *pPinWakeupConfig)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    DEV_ASSERT(pPinWakeupConfig != NULL);
    DEV_ASSERT(channelCnt > 0U);
    DEV_ASSERT(channelCnt <= FEATURE_WKU_MAX_CHANNEL_COUNT);
    uint8_t i;

    /* Configure for PinWakeup */
    for (i = 0U; i < channelCnt; i++)
    {
        WKU_DRV_SetPinWakeupConfig(instance, &pPinWakeupConfig[i]);
    }

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WKU_DRV_DeinitPinWakeup
 * Description   : This function de-initializes the pin wakeup WKU module.
 * Reset pin wakeup configuration, disable IRQ and Wake-up, clear filter enable,
 * pull-up enable, edge events enable.
 *
 * Implements    : WKU_DRV_DeinitPinWakeup_Activity
 *END**************************************************************************/
status_t WKU_DRV_DeinitPinWakeup(uint32_t instance)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    uint8_t i;

    /* Configure for PinWakeup */
    for (i = 0U; i < FEATURE_WKU_MAX_CHANNEL_COUNT; i++)
    {
        WKU_DRV_ClearPinWakeupConfig(instance, i);
    }


    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WKU_DRV_GetPinWakeupDefaultConfig
 * Description   : This function gets pin wakeup default configuration.
 * Note that: The user need provides an array have 32 elements of pin wakeup configuration.
 *
 * Implements    : WKU_DRV_GetPinWakeupDefaultConfig_Activity
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : WKU_DRV_SetPinWakeupConfig
 * Description   : This function sets the pin wakeup configuration based on
 * pin wakeup configuration input.
 *
 * Implements    : WKU_DRV_SetPinWakeupConfig_Activity
 *END**************************************************************************/
void WKU_DRV_SetPinWakeupConfig(uint32_t instance,
                                const wku_pin_wakeup_cfg_t *pPinWakeupConfig)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    DEV_ASSERT(pPinWakeupConfig != NULL);

    WKU_Type *base = s_wkuBase[instance];
    uint8_t hwChannel = pPinWakeupConfig->hwChannel;

    /* Clear previous flags */
    WKU_ClearPinWakeupFlag(base, hwChannel);

    /* Set Wakeup/PinWakeup Filter Enable Register */
    WKU_EnableFilter(base, hwChannel, pPinWakeupConfig->filterEn);

#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
    /* Set Wakeup/PinWakeup Filter Clock sorurce */
    if (pPinWakeupConfig->filterEn)
    {
        WKU_SetResetPinFilterClock(base,pPinWakeupConfig->filterClkSrc);
    }
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */

    /* Set Wakeup/PinWakeup Pin Interrupt Enable Register */
    WKU_EnablePinInterrupt(base, hwChannel, true);

    /* Set edge events enable registers */
    WKU_DRV_SetPinWakeupEdgeEvent(instance, hwChannel, pPinWakeupConfig->edgeEvent);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WKU_DRV_ClearPinWakeupConfig
 * Description   : This function clears pin wakeup configuration, disable IRQ and Wake-up,
 * clear filter enable, pull-up enable, edge events enable.
 *
 * Implements    : WKU_DRV_ClearPinWakeupConfig_Activity
 *END**************************************************************************/
void WKU_DRV_ClearPinWakeupConfig(uint32_t instance,
                                  uint8_t hwChannel)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    DEV_ASSERT(hwChannel < FEATURE_WKU_MAX_CHANNEL_COUNT);

    WKU_Type *base = s_wkuBase[instance];

    /* Clear previous flags */
    WKU_ClearPinWakeupFlag(base, hwChannel);

    /* Disable Wakeup/PinWakeup Filter Enable Register */
    WKU_EnableFilter(base, hwChannel, false);

#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
    /* Set Wakeup/PinWakeup Filter Clock sorurce to SIRC */
    WKU_SetResetPinFilterClock(base,WKU_FILTER_CLK_SIRC);
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */

    /* Disable Wakeup/PinWakeup Pull-up Enable Register */
    WKU_EnablePinInterrupt(base, hwChannel, false);

    /* Clear edge events enable registers */
    WKU_DRV_SetPinWakeupEdgeEvent(instance, hwChannel, WKU_EDGE_NONE);
}


/*FUNCTION**********************************************************************
 *
 * Function Name : WKU_DRV_SetPinWakeupEdgeEvent
 * Description   : This function sets edge events for each channel of the WKU.
 *
 * Implements    : WKU_DRV_SetPinWakeupEdgeEvent_Activity
 *END**************************************************************************/
void WKU_DRV_SetPinWakeupEdgeEvent(uint32_t instance,
                                   uint8_t hwChannel,
                                   wku_edge_event_t edge)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    DEV_ASSERT(hwChannel < FEATURE_WKU_MAX_CHANNEL_COUNT);

    WKU_Type *base = s_wkuBase[instance];
    WKU_SetWakeueEdgeEvent(base, hwChannel, edge);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WKU_DRV_ClearPinWakeupFlag
 * Description   : This function clears pin wakeup flag for channel mask.
 *
 * Implements    : WKU_DRV_ClearPinWakeupFlag_Activity
 *END**************************************************************************/
void WKU_DRV_ClearPinWakeupFlag(uint32_t instance,
                                uint8_t hwChannel)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    DEV_ASSERT(hwChannel < FEATURE_WKU_MAX_CHANNEL_COUNT);

    WKU_Type *base = s_wkuBase[instance];

    /* Clear pin wakeup flags */
    WKU_ClearPinWakeupFlag(base, hwChannel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WKU_DRV_InitReset
 * Description   : This function sets reset configuration of the WKU based on
 * reset configuration input.
 *
 * Implements    : WKU_DRV_InitReset_Activity
 *END**************************************************************************/
void WKU_DRV_InitReset(uint32_t instance,
                       const wku_reset_cfg_t *pResetConfig)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    DEV_ASSERT(pResetConfig != NULL);

    WKU_Type *base = s_wkuBase[instance];

    /* Configure reset wake-up request */
    WKU_EnableResetPin(base, pResetConfig->wakeupEn);
    /* Configure reset wake-up filter enable */
    WKU_EnableResetPinFilter(base, pResetConfig->filterEn);
#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
    /* Configure reset wake-up filter clock */
    WKU_SetResetPinFilterClock(base, pResetConfig->filterClkSrc);
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WKU_DRV_DeinitReset
 * Description   : This function de-initializes the Reset WKU module. Reset configuration,
 * disable Wake-up, edge events enable and lock enable.
 *
 * Implements    : WKU_DRV_DeinitReset_Activity
 *END**************************************************************************/
void WKU_DRV_DeinitReset(uint32_t instance)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);

    WKU_Type *base = s_wkuBase[instance];

    /* Configure reset wake-up request */
    WKU_EnableResetPin(base, false);
    /* Configure reset wake-up filter enable */
    WKU_EnableResetPinFilter(base, false);
#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
    /* Configure reset wake-up filter clock */
    WKU_SetResetPinFilterClock(base, WKU_FILTER_CLK_SIRC);
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WKU_DRV_GetResetDefaultConfig
 * Description   : This function gets Reset default configuration.
 *
 * Implements    : WKU_DRV_GetResetDefaultConfig_Activity
 *END**************************************************************************/
void WKU_DRV_GetResetDefaultConfig(wku_reset_cfg_t *pResetConfig)
{
    DEV_ASSERT(pResetConfig != NULL);

    pResetConfig->wakeupEn = true;
    pResetConfig->filterEn = true;
#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
    pResetConfig->filterClkSrc = WKU_FILTER_CLK_SIRC;
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WKU_DRV_EnableModuleWakeup
 * Description   : This function enables module wakeup request.
 *
 * Implements    : WKU_DRV_EnableModuleWakeup_Activity
 *END**************************************************************************/
void WKU_DRV_EnableModuleWakeup(uint32_t instance, uint8_t moduleID)
{
    DEV_ASSERT(instance < WKU_INSTANCE_COUNT);
    DEV_ASSERT(moduleID < FEATURE_WKU_MAX_CHANNEL_COUNT);
    WKU_Type *base = s_wkuBase[instance];
    WKU_EnableModuleWakeup(base, moduleID, true);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : WKU_DRV_DisableModuleWakeup
 * Description   : This function disables module wakeup request.
 *
 * Implements    : WKU_DRV_DisableModuleWakeup_Activity
 *END**************************************************************************/
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
