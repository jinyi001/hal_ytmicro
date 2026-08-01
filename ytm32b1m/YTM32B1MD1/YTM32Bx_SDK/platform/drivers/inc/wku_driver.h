/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file wku_driver.h
 * @version 1.4.1
 *
 * @brief WKU Driver - Public API for Wakeup Unit configuration.
 *
 * This header defines the application-facing interface for the Wakeup Unit
 * (WKU). It provides helper APIs for configuring external pin wakeup sources,
 * reset-pin wakeup behavior, internal module wakeup requests, and optional
 * pin-isolation support during power-down transitions.
 *
 * The APIs are organized into the following categories:
 *   - Pin Wakeup Initialization & De-initialization
 *   - Pin Wakeup Channel Control
 *   - Reset Wakeup Control
 *   - Internal Module Wakeup Control
 *   - Power-Down Pin Isolation
 *
 * @note Configure the relevant clocking and low-power flow before relying on
 *       WKU as a wakeup source. See @ref clock_manager and @ref power_manager.
 */

#ifndef WKU_DRIVER_H
#define WKU_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "status.h"
#include "device_registers.h"

/*!
 * @addtogroup wku
 * @brief Wakeup Unit peripheral driver - public API.
 * @details Provides instance-based APIs for external pin wakeup setup,
 *          reset-pin wakeup control, internal module wakeup requests, and
 *          optional pin-isolation handling for power-down scenarios.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Edge-detection mode for a WKU pin channel.
 *
 * Selects which transition on the monitored pin is treated as a wakeup event.
 *
 * | Value            | Description                               |
 * |------------------|-------------------------------------------|
 * | WKU_EDGE_NONE    | Disable edge detection for the channel.   |
 * | WKU_EDGE_RISING  | Detect low-to-high transitions only.      |
 * | WKU_EDGE_FALLING | Detect high-to-low transitions only.      |
 * | WKU_EDGE_BOTH    | Detect both rising and falling edges.     |
 */
typedef enum
{
    WKU_EDGE_NONE = 0U,       /*!< Disable edge detection. */
    WKU_EDGE_RISING = 1U,     /*!< Detect rising edges only. */
    WKU_EDGE_FALLING = 2U,    /*!< Detect falling edges only. */
    WKU_EDGE_BOTH = 3U        /*!< Detect both rising and falling edges. */
} wku_edge_event_t;

/*!
 * @brief Digital-filter clock source selection.
 *
 * Selects the clock used by the WKU filter logic when
 * `FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL` is enabled.
 *
 * | Value               | Description                         |
 * |---------------------|-------------------------------------|
 * | WKU_FILTER_CLK_SIRC | Use the SIRC clock as filter input. |
 * | WKU_FILTER_CLK_SXOSC | Use the SXOSC clock as filter input. |
 */
typedef enum
{
    WKU_FILTER_CLK_SIRC = 0U,    /*!< Select the SIRC clock source. */
    WKU_FILTER_CLK_SXOSC = 1U    /*!< Select the SXOSC clock source. */
} wku_filter_clock_source_t;

/*!
 * @brief External pin-wakeup channel configuration.
 *
 * Describes the configuration applied to one WKU external wakeup channel.
 *
 * | Field        | Type                       | Description |
 * |--------------|----------------------------|-------------|
 * | hwChannel    | uint8_t                    | Hardware channel index programmed in the WKU PCR array. |
 * | wakeupEn     | bool                       | Requested wakeup-enable state tracked by the channel configuration. |
 * | edgeEvent    | wku_edge_event_t           | Edge-detection mode programmed for the channel. |
 * | filterEn     | bool                       | Enables or bypasses the digital filter for the channel. |
 * | filterClkSrc | wku_filter_clock_source_t  | Filter clock source when filter-clock selection is supported. |
 */
typedef struct
{
    uint8_t hwChannel;                      /*!< Hardware channel index. */
    bool wakeupEn;                          /*!< Requested wakeup-enable state for the channel. */
    wku_edge_event_t edgeEvent;             /*!< Edge-detection mode for the channel. */
    bool filterEn;                          /*!< Enable or bypass the digital filter. */
#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
    wku_filter_clock_source_t filterClkSrc; /*!< Filter clock source used when filtering is enabled. */
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */
} wku_pin_wakeup_cfg_t;

/*!
 * @brief Reset-pin wakeup configuration.
 *
 * Describes the WKU settings applied to the reset pin when it is used as a
 * wakeup source from low-power mode.
 *
 * | Field        | Type                       | Description |
 * |--------------|----------------------------|-------------|
 * | wakeupEn     | bool                       | Enable or disable reset-pin wakeup requests. |
 * | filterEn     | bool                       | Enable or bypass the reset-pin digital filter. |
 * | filterClkSrc | wku_filter_clock_source_t  | Filter clock source when filter-clock selection is supported. |
 */
typedef struct
{
    bool wakeupEn;                          /*!< Enable or disable reset-pin wakeup requests. */
    bool filterEn;                          /*!< Enable or bypass reset-pin filtering. */
#if FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL
    wku_filter_clock_source_t filterClkSrc; /*!< Filter clock source for the reset-pin path. */
#endif /* FEATURE_WKU_SUPPORT_FILTER_CLOCK_SEL */
} wku_reset_cfg_t;

/*!
 * @brief Internal low-power module wakeup identifiers.
 *
 * Selects which internal module wakeup request is enabled through the WKU
 * module-enable register.
 *
 * | Value                    | Description |
 * |--------------------------|-------------|
 * | WKU_RTC_WAKEUP           | RTC alarm/module wakeup source. |
 * | WKU_RTC_SECONDS_WAKEUP   | RTC seconds interrupt wakeup source. |
 * | WKU_lpTMR0_WAKEUP        | lpTMR0 wakeup source. |
 * | WKU_ACMP0_WAKEUP         | ACMP0 wakeup source. |
 * | WKU_ACMP1_WAKEUP         | ACMP1 wakeup source when supported by the device. |
 *
 * @note Actual module availability is device-dependent.
 */
typedef enum
{
    WKU_RTC_WAKEUP = 0x00U,          /*!< RTC module wakeup source. */
    WKU_RTC_SECONDS_WAKEUP = 0x01U,  /*!< RTC seconds interrupt wakeup source. */
    WKU_lpTMR0_WAKEUP = 0x02U,       /*!< lpTMR0 wakeup source. */
    WKU_ACMP0_WAKEUP = 0x03U,        /*!< ACMP0 wakeup source. */
    WKU_ACMP1_WAKEUP = 0x04U         /*!< ACMP1 wakeup source on supported devices. */
} wku_wakeup_module_t;

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Pin Wakeup Initialization & De-initialization
 ******************************************************************************/
/*!
 * @name Pin Wakeup Initialization & De-initialization
 * @brief Functions for preparing and resetting external pin wakeup channels.
 * @{
 */

/*!
 * @brief Initialize a set of external pin wakeup channels.
 *
 * Applies each entry from @a pPinWakeupConfig to the selected WKU instance.
 * The driver programs the channel filter, optional filter clock, interrupt
 * enable, and edge-detection mode for each configured entry.
 *
 * @param[in] instance          WKU instance index. Must be less than
 *                              `WKU_INSTANCE_COUNT`.
 * @param[in] channelCnt        Number of configuration entries to apply.
 *                              Valid range: 1 to `FEATURE_WKU_MAX_CHANNEL_COUNT`.
 * @param[in] pPinWakeupConfig  Pointer to the configuration table.
 *                              Must not be NULL.
 * @return Execution status.
 * @retval STATUS_SUCCESS Initialization completed successfully.
 *
 * @pre The WKU peripheral must be accessible for the selected instance.
 * @post The first @a channelCnt entries have been applied to hardware.
 */
status_t WKU_DRV_InitPinWakeup(uint32_t instance,
                               uint8_t channelCnt,
                               const wku_pin_wakeup_cfg_t *pPinWakeupConfig);

/*!
 * @brief Clear all external pin wakeup channel settings.
 *
 * Iterates across every supported WKU channel and clears the previously
 * programmed filter, interrupt, flag, and edge-detection settings.
 *
 * @param[in] instance WKU instance index. Must be less than
 *                     `WKU_INSTANCE_COUNT`.
 * @return Execution status.
 * @retval STATUS_SUCCESS De-initialization completed successfully.
 *
 * @post All supported pin wakeup channels are returned to their cleared state.
 */
status_t WKU_DRV_DeinitPinWakeup(uint32_t instance);

/*!
 * @brief Populate the default pin wakeup configuration table.
 *
 * Initializes one configuration entry per hardware channel. Each entry is
 * assigned its channel index, both-edge detection, filter disabled, and the
 * default filter clock when supported by the device.
 *
 * @param[out] pPinWakeupConfig Pointer to an array containing
 *                              `FEATURE_WKU_MAX_CHANNEL_COUNT` elements.
 *                              Must not be NULL.
 *
 * @post All array entries for the supported hardware channels are initialized.
 * @note The `wakeupEn` field is not written by this helper and should be set
 *       explicitly by the caller before applying the configuration.
 */
void WKU_DRV_GetPinWakeupDefaultConfig(wku_pin_wakeup_cfg_t *const pPinWakeupConfig);

/*! @} */ /* End of Pin Wakeup Initialization & De-initialization */

/*******************************************************************************
 * Pin Wakeup Channel Control
 ******************************************************************************/
/*!
 * @name Pin Wakeup Channel Control
 * @brief Functions for configuring, clearing, and servicing individual pin
 *        wakeup channels.
 * @{
 */

/*!
 * @brief Apply the configuration for one external pin wakeup channel.
 *
 * Clears any pending flag on the target channel, programs the digital filter,
 * optionally selects the filter clock, enables the pin interrupt path, and
 * updates the edge-detection mode.
 *
 * @param[in] instance          WKU instance index. Must be less than
 *                              `WKU_INSTANCE_COUNT`.
 * @param[in] pPinWakeupConfig  Pointer to the channel configuration structure.
 *                              Must not be NULL.
 *
 * @note If filtering is disabled, the filter clock selection is not used.
 */
void WKU_DRV_SetPinWakeupConfig(uint32_t instance,
                                const wku_pin_wakeup_cfg_t *pPinWakeupConfig);

/*!
 * @brief Clear the configuration of one external pin wakeup channel.
 *
 * Clears the pending flag, disables the digital filter and interrupt path,
 * restores the default filter clock when supported, and removes edge
 * detection for the selected channel.
 *
 * @param[in] instance   WKU instance index. Must be less than
 *                       `WKU_INSTANCE_COUNT`.
 * @param[in] hwChannel  Hardware channel index. Must be less than
 *                       `FEATURE_WKU_MAX_CHANNEL_COUNT`.
 */
void WKU_DRV_ClearPinWakeupConfig(uint32_t instance,
                                  uint8_t hwChannel);

/*!
 * @brief Put a pin wakeup channel into the driver's normal operating mode.
 *
 * @param[in] instance   WKU instance index.
 * @param[in] hwChannel  Hardware channel index.
 */
void WKU_DRV_SetPinWakeupNormalMode(uint32_t instance,
                                    uint8_t hwChannel);

/*!
 * @brief Put a pin wakeup channel into the driver's sleep-mode policy.
 *
 * @param[in] instance   WKU instance index.
 * @param[in] hwChannel  Hardware channel index.
 */
void WKU_DRV_SetPinWakeupSleepMode(uint32_t instance,
                                   uint8_t hwChannel);

/*!
 * @brief Update the edge-detection mode of one pin wakeup channel.
 *
 * @param[in] instance   WKU instance index. Must be less than
 *                       `WKU_INSTANCE_COUNT`.
 * @param[in] hwChannel  Hardware channel index. Must be less than
 *                       `FEATURE_WKU_MAX_CHANNEL_COUNT`.
 * @param[in] edge       Edge-detection mode to program.
 */
void WKU_DRV_SetPinWakeupEdgeEvent(uint32_t instance,
                                   uint8_t hwChannel,
                                   wku_edge_event_t edge);

/*!
 * @brief Clear the wakeup flag of one pin channel.
 *
 * @param[in] instance   WKU instance index. Must be less than
 *                       `WKU_INSTANCE_COUNT`.
 * @param[in] hwChannel  Hardware channel index. Must be less than
 *                       `FEATURE_WKU_MAX_CHANNEL_COUNT`.
 *
 * @post Any pending wakeup flag for the selected channel is acknowledged.
 */
void WKU_DRV_ClearPinWakeupFlag(uint32_t instance,
                                uint8_t hwChannel);

/*! @} */ /* End of Pin Wakeup Channel Control */

/*******************************************************************************
 * Reset Wakeup Control
 ******************************************************************************/
/*!
 * @name Reset Wakeup Control
 * @brief Functions for configuring reset-pin wakeup behavior.
 * @{
 */

/*!
 * @brief Initialize reset-pin wakeup handling.
 *
 * Programs the reset-pin wakeup enable state, the reset-pin digital filter,
 * and the filter clock selection when supported by the device.
 *
 * @param[in] instance      WKU instance index. Must be less than
 *                          `WKU_INSTANCE_COUNT`.
 * @param[in] pResetConfig  Pointer to the reset-pin configuration structure.
 *                          Must not be NULL.
 */
void WKU_DRV_InitReset(uint32_t instance,
                       const wku_reset_cfg_t *pResetConfig);

/*!
 * @brief Restore the reset-pin wakeup path to its cleared state.
 *
 * Disables reset-pin wakeup requests, disables the reset-pin filter, and
 * restores the default filter clock when supported by the device.
 *
 * @param[in] instance WKU instance index. Must be less than
 *                     `WKU_INSTANCE_COUNT`.
 */
void WKU_DRV_DeinitReset(uint32_t instance);

/*!
 * @brief Populate the default reset-pin wakeup configuration.
 *
 * Fills the configuration structure with wakeup enabled, filter enabled, and
 * the SIRC filter clock when filter-clock selection is supported.
 *
 * @param[out] pResetConfig Pointer to the reset-pin configuration structure.
 *                          Must not be NULL.
 */
void WKU_DRV_GetResetDefaultConfig(wku_reset_cfg_t *pResetConfig);

/*! @} */ /* End of Reset Wakeup Control */

/*******************************************************************************
 * Internal Module Wakeup Control
 ******************************************************************************/
/*!
 * @name Internal Module Wakeup Control
 * @brief Functions for gating internal module wakeup requests.
 * @{
 */

/*!
 * @brief Enable an internal module wakeup request.
 *
 * @param[in] instance  WKU instance index. Must be less than
 *                      `WKU_INSTANCE_COUNT`.
 * @param[in] moduleID  Internal wakeup source ID. See @ref wku_wakeup_module_t.
 */
void WKU_DRV_EnableModuleWakeup(uint32_t instance, uint8_t moduleID);

/*!
 * @brief Disable an internal module wakeup request.
 *
 * @param[in] instance  WKU instance index. Must be less than
 *                      `WKU_INSTANCE_COUNT`.
 * @param[in] moduleID  Internal wakeup source ID. See @ref wku_wakeup_module_t.
 */
void WKU_DRV_DisableModuleWakeup(uint32_t instance, uint8_t moduleID);

/*! @} */ /* End of Internal Module Wakeup Control */

#if FEATURE_WKU_SUPPORT_PIN_ISOLATION
/*******************************************************************************
 * Power-Down Pin Isolation
 ******************************************************************************/
/*!
 * @name Power-Down Pin Isolation
 * @brief Optional helper for preserving pin state across power-down cycles.
 * @{
 */

/*!
 * @brief Enable or disable pin isolation during power-down mode.
 *
 * Pin isolation preserves the current pad state while the device is in
 * power-down mode and until software reconfigures the pads after wakeup.
 *
 * @param[in] enable `true` to enable pin isolation, `false` to disable it.
 *
 * @note Availability is controlled by `FEATURE_WKU_SUPPORT_PIN_ISOLATION`.
 * @warning Disable pin isolation after the MCU reinitializes its pin
 *          configuration following wakeup.
 */
static inline void WKU_DRV_SetPinIsolation(bool enable)
{
#if defined(CPU_YTM32B1ME1)
    PCU->CTRL0 = (PCU->CTRL0 & ~PCU_CTRL0_PKE_MASK) | PCU_CTRL0_PKE(enable ? 1U : 0U);
#else
    PCU->CTRL = (PCU->CTRL & ~PCU_CTRL_PKE_MASK) | PCU_CTRL_PKE(enable ? 1U : 0U);
#endif /* CPU_YTM32B1ME1 */
}

/*! @} */ /* End of Power-Down Pin Isolation */
#endif /* FEATURE_WKU_SUPPORT_PIN_ISOLATION */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* WKU_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
