/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file wku_driver.h
 * @version 1.4.1
 */

#ifndef WKU_DRIVER_H
#define WKU_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "status.h"
#include "device_registers.h"


/*!
 * @addtogroup wku_driver
 * @brief This section describes the programming interface of the WKU driver.
 * @{
 */

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*!
 * @brief Edge event.
 * Implements : wku_edge_event_t_Class
 */
typedef enum
{
    WKU_EDGE_NONE = 0U,   /*!< None event */
    WKU_EDGE_RISING = 1U,   /*!< Rising edge event */
    WKU_EDGE_FALLING = 2U,   /*!< Falling edge event */
    WKU_EDGE_BOTH = 3U    /*!< Both rising and falling edge event */
} wku_edge_event_t;

/*!
 * @brief Filter clock source.
 * Implements : wku_filter_clock_source_t_Class
 */
typedef enum
{
    WKU_FILTER_CLK_SIRC = 0U,   /*!< SRIC clock source */
    WKU_FILTER_CLK_SXOSC = 1U,   /*!< SXOSC clock source */
} wku_filter_clock_source_t;


/*!
 * @brief WKU pin wakeup configuration structure.
 * Implements : wku_pin_wakeup_cfg_t_Class
 */
typedef struct
{
    uint8_t hwChannel;                      /*!< Hardware channel index */
    bool wakeupEn;                          /*!< WKU/pin wakeup enable */
    wku_edge_event_t edgeEvent;             /*!< WKU/pin wakeup edge event */
    bool filterEn;                          /*!< WKU/pin wakeup filter enable */
#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
    wku_filter_clock_source_t filterClkSrc; /*!< WKU/pin wakeup filter clock source */
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */
} wku_pin_wakeup_cfg_t;


/*!
 * @brief Reset configuration structure.
 * Implements : wku_reset_cfg_t_Class
 */
typedef struct
{
    bool wakeupEn;                          /*!< Reset wakeup request enable */
    bool filterEn;                          /*!< Reset pin wakeup filter enable */
#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
    wku_filter_clock_source_t filterClkSrc; /*!< Reset pin wakeup filter clock source */
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */
} wku_reset_cfg_t;

/* WKU wakeup module list */
typedef enum
{
    WKU_RTC_WAKEUP = 0x00U, /*!< WKU RTC module wakeup */
    WKU_RTC_SECONDS_WAKEUP = 0x01U,   /*!< WKU RTC seconds interrupt wakeup */
    WKU_lpTMR0_WAKEUP = 0x02U,   /*!< WKU lpTMR0 module wakeup */
    WKU_ACMP0_WAKEUP = 0x03U,   /*!< WKU ACMP0 module wakeup */
    WKU_ACMP1_WAKEUP = 0x04U,   /*!< WKU ACMP1 module wakeup, only valid when MCU support ACMP1 */
} wku_wakeup_module_t;


/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @name WKU DRIVER API
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif


/*!
 * @brief Initializes the pin wakeup WKU module
 *
 * This function initializes pin wakeup WKU driver based on user configuration input.
 * The channelCnt takes values between 1 and the maximum channel count supported by the hardware
 *
 * @param[in] instance The WKU instance number
 * @param[in] channelCnt The number of channels
 * @param[in] pPinWakeupConfig Pointer to structure of pin wakeup configuration for each the channel
 * @return Execution status (success/error)
 */
status_t WKU_DRV_InitPinWakeup(uint32_t instance,
                               uint8_t channelCnt,
                               const wku_pin_wakeup_cfg_t *pPinWakeupConfig);

/*!
 * @brief De-initializes the pin wakeup WKU module
 *
 * This function de-initializes the pin wakeup WKU module.
 * Reset pin wakeup configuration, disable IRQ and Wake-up, clear filter enable,
 * pull-up enable, edge events enable.
 *
 * @param[in] instance The WKU instance number
 * @return Execution status (success/error)
 */
status_t WKU_DRV_DeinitPinWakeup(uint32_t instance);

/*!
 * @brief Gets pin wakeup default configuration
 *
 * This function gets pin wakeup default configuration.
 * Note that: The user need provides an array have 32 elements of pin wakeup configuration
 *
 * @param[out] pPinWakeupConfig Pointer to structure of pin wakeup configuration for each the channel
 */
void WKU_DRV_GetPinWakeupDefaultConfig(wku_pin_wakeup_cfg_t *const pPinWakeupConfig);

/*!
 * @brief Configures for pin wakeup
 *
 * This function sets the pin wakeup configuration based on pin wakeup configuration input
 *
 * @param[in] instance The WKU instance number
 * @param[in] pPinWakeupConfig Pointer to structure of pin wakeup configuration
 */
void WKU_DRV_SetPinWakeupConfig(uint32_t instance,
                                const wku_pin_wakeup_cfg_t *pPinWakeupConfig);

/*!
 * @brief Clears pin wakeup Configuration
 *
 * This function clears pin wakeup configuration, disable IRQ and Wake-up,
 * clear filter enable, pull-up enable, edge events enable
 *
 * @param[in] instance The WKU instance number
 * @param[in] hwChannel The WKU hardware channel
 */
void WKU_DRV_ClearPinWakeupConfig(uint32_t instance,
                                  uint8_t hwChannel);

/*!
 * @brief Sets pin wakeup normal mode
 *
 * This function sets pin wakeup normal mode(enable) for the WKU,
 * enable pin wakeup and wake-up
 *
 * @param[in] instance The WKU instance number
 * @param[in] hwChannel The WKU hardware channel
 */
void WKU_DRV_SetPinWakeupNormalMode(uint32_t instance,
                                    uint8_t hwChannel);

/*!
 * @brief Sets pin wakeup sleep mode
 *
 * This function sets sleep mode for the WKU or disable pin wakeup
 * and wake-up
 *
 * @param[in] instance The WKU instance number
 * @param[in] hwChannel The WKU hardware channel
 */
void WKU_DRV_SetPinWakeupSleepMode(uint32_t instance,
                                   uint8_t hwChannel);

/*!
 * @brief Sets edge events
 *
 * This function sets edge events for each channel of the WKU
 *
 * @param[in] instance The WKU instance number
 * @param[in] hwChannel The WKU hardware channel
 * @param[in] edge The edge event for pin wakeup
 */
void WKU_DRV_SetPinWakeupEdgeEvent(uint32_t instance,
                                   uint8_t hwChannel,
                                   wku_edge_event_t edge);

/*!
 * @brief Clears pin wakeup flag
 *
 * This function clears pin wakeup flag for channel mask
 *
 * @param[in] instance The WKU instance number
 * @param[in] hwChannel The WKU hardware channel
 */
void WKU_DRV_ClearPinWakeupFlag(uint32_t instance,
                                uint8_t hwChannel);

/*!
 * @brief Initializes the reset WKU module
 *
 * This function sets reset configuration of the WKU based on reset configuration input
 *
 * @param[in] instance The WKU instance number
 * @param[in] pResetConfig Pointer to structure of reset configuration
 */
void WKU_DRV_InitReset(uint32_t instance,
                       const wku_reset_cfg_t *pResetConfig);

/*!
 * @brief De-initializes the Reset WKU module
 *
 * This function de-initializes the Reset WKU module. Reset configuration,
 * disable Wake-up, edge events enable and lock enable.
 *
 * @param[in] instance The WKU instance number
 */
void WKU_DRV_DeinitReset(uint32_t instance);

/*!
 * @brief Gets Reset default configuration
 *
 * This function gets Reset default configuration.
 *
 * @param[out] pResetConfig Pointer to structure of NMI configuration
 */
void WKU_DRV_GetResetDefaultConfig(wku_reset_cfg_t *pResetConfig);

/*!
 * @brief Enable module wakeup request
 *
 * This function enables module wakeup request
 *
 * @param[in] instance The WKU instance number
 * @param[in] moduleID The wakeup module number
 */
void WKU_DRV_EnableModuleWakeup(uint32_t instance, uint8_t moduleID);

/*!
 * @brief Disable module wakeup request
 *
 * This function disables module wakeup request
 *
 * @param[in] instance The WKU instance number
 * @param[in] moduleID The wakeup module number
 */
void WKU_DRV_DisableModuleWakeup(uint32_t instance, uint8_t moduleID);

#if FEATURE_WKU_SUPPORT_PIN_ISOLATION
/*!
 * @brief Enable disable PIN isolation under power down mode
 *
 * PIN isolation will keep the pin state when the power down mode is entered and
 * after the power down mode is exited. Application should disable pin isolation after
 * MCU reinitialization pin configuration.
 *
 * @note Not all MCU support pin isolation features
 *
 * @param[in] enable Enable/disable pin isolation
 */

static inline void WKU_DRV_SetPinIsolation(bool enable)
{
#if defined(CPU_YTM32B1ME1)
    PCU->CTRL0 = (PCU->CTRL0 & ~PCU_CTRL0_PKE_MASK)| PCU_CTRL0_PKE(enable ? 1U : 0U);
#else
    PCU->CTRL = (PCU->CTRL & ~PCU_CTRL_PKE_MASK)| PCU_CTRL_PKE(enable ? 1U : 0U);
#endif /* CPU_YTM32B1ME1 */
}
#endif /* FEATURE_WKU_SUPPORT_PIN_ISOLATION */

/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* WKU_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
