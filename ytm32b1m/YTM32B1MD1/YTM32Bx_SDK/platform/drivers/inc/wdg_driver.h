/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file wdg_driver.h
 * @version 1.4.1
 *
 * @brief WDG Driver - Public API for watchdog supervision.
 *
 * This header defines the application-level interface for the WDG peripheral.
 * It wraps the internal hardware access layer and provides instance-based APIs
 * for watchdog configuration, counter refresh, runtime control, and timeout
 * interrupt handling.
 *
 * The APIs are organized into four categories:
 *   - **Initialization & De-initialization**: Start or reset a WDG instance.
 *   - **Configuration**: Read or update runtime watchdog parameters.
 *   - **Watchdog Operation**: Service the watchdog and inspect the counter.
 *   - **Interrupt Management**: Control and acknowledge timeout interrupts.
 *
 * @note Enable the WDG peripheral clock before calling any function in this
 *       module.
 */

#ifndef WDG_DRIVER_H
#define WDG_DRIVER_H

#include "status.h"
#include "interrupt_manager.h"

/*!
 * @addtogroup wdg
 * @brief Watchdog Timer Driver - Public API.
 * @details Provides instance-based APIs for watchdog startup, runtime
 *          configuration, counter servicing, and interrupt control. The driver
 *          supports normal refresh mode, optional window mode, debug and
 *          deep-sleep run control, and interrupt-before-reset behavior.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Runtime mode selector used by WDG_DRV_SetMode().
 *
 * Selects which non-run operating state is updated when enabling or disabling
 * watchdog activity after initialization.
 *
 * | Value              | Description                                            |
 * |--------------------|--------------------------------------------------------|
 * | WDG_DEBUG_MODE     | Control whether the WDG runs while the core is halted. |
 * | WDG_DEEPSLEEP_MODE | Control whether the WDG runs in deep-sleep mode.       |
 */
typedef enum
{
    WDG_DEBUG_MODE = 0x00U,     /*!< Update debug-halt behavior. */
    WDG_DEEPSLEEP_MODE = 0x01U, /*!< Update deep-sleep behavior. */
} wdg_set_mode_t;

/*!
 * @brief WDG run-state control flags.
 *
 * Controls whether the watchdog continues counting while the device is in
 * deep-sleep mode or halted by a debugger.
 *
 * | Field     | Type | Description                                           |
 * |-----------|------|-------------------------------------------------------|
 * | deepsleep | bool | true keeps the WDG active in deep-sleep mode.         |
 * | debug     | bool | true keeps the WDG active while the core is halted.   |
 */
typedef struct
{
    bool deepsleep;  /*!< Controls watchdog activity in deep-sleep mode. */
    bool debug;      /*!< Controls watchdog activity during debug halt. */
} wdg_op_mode_t;

/*!
 * @brief WDG user configuration structure.
 *
 * Holds the parameters written to the watchdog registers during
 * WDG_DRV_Init(). Use WDG_DRV_GetDefaultConfig() to populate the software
 * defaults, then override the fields required by the application.
 *
 * | Field               | Type               | Description                                                   | Default / Source              |
 * |---------------------|--------------------|---------------------------------------------------------------|-------------------------------|
 * | clockSource         | wdg_clock_source_t | Watchdog clock source on devices that support source select.  | Caller supplied when present  |
 * | opMode              | wdg_op_mode_t      | Debug-halt and deep-sleep run-state behavior.                 | Both fields false             |
 * | updateEnable        | bool               | Keeps configuration writable after initialization when true.  | true                          |
 * | intEnable           | bool               | Enables interrupt-before-reset on the first timeout.          | false                         |
 * | winEnable           | bool               | Enables window refresh mode.                                  | false                         |
 * | windowValue         | uint32_t           | Lower edge of the valid refresh window when enabled.          | FEATURE_WDG_WVR_RESET_VALUE   |
 * | timeoutValue        | uint32_t           | Timeout reload value written to TOVR.                         | FEATURE_WDG_TOVR_RESET_VALUE  |
 * | apbErrorResetEnable | bool               | Enables reset generation on APB access error.                 | false                         |
 *
 * @note On devices that expose @c clockSource, WDG_DRV_GetDefaultConfig()
 *       does not initialize that field. Set it explicitly before
 *       WDG_DRV_Init() if the application depends on a specific source.
 */
typedef struct
{
#if (defined(WDG_CR_CLKSRC_MASK))
    wdg_clock_source_t clockSource;           /*!< Watchdog clock source on devices with source select. */
#endif
    wdg_op_mode_t opMode;                     /*!< Debug-halt and deep-sleep run-state configuration. */
    bool updateEnable;                        /*!< Keeps watchdog configuration writable after init when true. */
    bool intEnable;                           /*!< Enables the first-timeout interrupt-before-reset path. */
    bool winEnable;                           /*!< Enables refresh-window checking when true. */
    uint32_t windowValue;                     /*!< Window threshold used when window mode is enabled. */
    uint32_t timeoutValue;                    /*!< Timeout reload value written to the watchdog. */
    bool apbErrorResetEnable;                 /*!< Enables reset generation on APB access error. */
} wdg_user_config_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/
/*!
 * @name Initialization & De-initialization
 * @brief Functions for setting up and tearing down a WDG driver instance.
 * @{
 */

/*!
 * @brief Initialize a WDG instance with user-provided settings.
 *
 * Validates the timeout and optional window parameters, programs the watchdog
 * registers, and enables the instance IRQ after the hardware accepts the
 * configuration.
 *
 * @param[in] instance       WDG instance index (0-based). Must be less than
 *                           WDG_INSTANCE_COUNT.
 * @param[in] userConfigPtr  Pointer to the configuration structure. Must not
 *                           be NULL.
 * @return Execution status.
 * @retval STATUS_SUCCESS  The watchdog was configured and started successfully.
 * @retval STATUS_ERROR    The request was rejected because the timeout or
 *                         window parameters are invalid, or because the
 *                         hardware is already active or locked.
 *
 * @pre  The WDG peripheral clock must be enabled before this call.
 * @post On success, the watchdog is running and must be serviced before the
 *       timeout expires.
 *
 * @note Use WDG_DRV_GetDefaultConfig() as the starting point for the
 *       configuration structure.
 */
status_t WDG_DRV_Init(uint32_t instance,
                      const wdg_user_config_t *userConfigPtr);

/*!
 * @brief Attempt to stop the watchdog and restore reset values.
 *
 * Disables the instance IRQ, resets the watchdog registers through the
 * hardware access layer, and leaves peripheral clock management to the caller.
 *
 * @param[in] instance  WDG instance index (0-based).
 * @return Execution status.
 * @retval STATUS_SUCCESS  The watchdog was reset to its default state.
 * @retval STATUS_ERROR    Configuration updates are locked, so the watchdog
 *                         cannot be stopped until the next reset.
 *
 * @warning If the watchdog was initialized with @c updateEnable set to false,
 *          this function cannot disable it.
 */
status_t WDG_DRV_Deinit(uint32_t instance);

/*!
 * @brief Populate a configuration structure with the driver's software defaults.
 *
 * Fills @a config with the baseline watchdog settings used by this driver:
 *   - opMode.debug / opMode.deepsleep: false
 *   - updateEnable: true
 *   - intEnable: false
 *   - winEnable: false
 *   - timeoutValue: FEATURE_WDG_TOVR_RESET_VALUE
 *   - windowValue: FEATURE_WDG_WVR_RESET_VALUE
 *   - apbErrorResetEnable: false
 *
 * @param[out] config  Pointer to the configuration structure to populate.
 *                     Must not be NULL.
 *
 * @note On devices that include the @c clockSource field, this helper leaves
 *       that field unchanged.
 */
void WDG_DRV_GetDefaultConfig(wdg_user_config_t *const config);

/*! @} */ /* End of Initialization & De-initialization */

/*******************************************************************************
 * Configuration
 ******************************************************************************/
/*!
 * @name Configuration
 * @brief Functions for querying or modifying WDG runtime parameters.
 * @{
 */

/*!
 * @brief Read the current WDG configuration from hardware.
 *
 * Decodes the watchdog control, timeout, window, and lock registers into the
 * public configuration structure.
 *
 * @param[in]  instance  WDG instance index (0-based).
 * @param[out] config    Pointer to the structure that receives the current
 *                       configuration. Must not be NULL.
 */
void WDG_DRV_GetConfig(uint32_t instance, wdg_user_config_t *const config);

/*!
 * @brief Enable or disable window mode and optionally update the window value.
 *
 * Updates the watchdog window-enable bit. When @a enable is true, the
 * function also writes @a windowValue to the WVR register.
 *
 * @param[in] instance     WDG instance index (0-based).
 * @param[in] enable       true enables window mode; false disables it.
 * @param[in] windowValue  Window threshold written when @a enable is true.
 * @return Execution status.
 * @retval STATUS_SUCCESS  The requested change was written successfully.
 * @retval STATUS_ERROR    Configuration updates are locked.
 *
 * @note This function does not validate @a windowValue against the current
 *       timeout setting. The caller must ensure the programmed window is valid.
 * @warning When window mode is enabled, refreshing the watchdog before the
 *          counter reaches the programmed window causes an immediate reset.
 */
status_t WDG_DRV_SetWindow(uint32_t instance,
                           bool enable,
                           uint32_t windowValue);

/*!
 * @brief Update watchdog behavior in debug halt or deep-sleep mode.
 *
 * Selects one runtime mode bit and programs whether the watchdog keeps
 * counting or freezes while the device is in that state.
 *
 * @param[in] instance  WDG instance index (0-based).
 * @param[in] enable    true keeps the watchdog active in the selected mode;
 *                      false freezes it.
 * @param[in] setMode   Runtime mode selector to update.
 * @return Execution status.
 * @retval STATUS_SUCCESS  The mode setting was updated successfully.
 * @retval STATUS_ERROR    Configuration updates are locked.
 */
status_t WDG_DRV_SetMode(uint32_t instance,
                         bool enable,
                         wdg_set_mode_t setMode);

/*!
 * @brief Write a new timeout reload value to the watchdog.
 *
 * Programs the TOVR register through the protected write path.
 *
 * @param[in] instance  WDG instance index (0-based).
 * @param[in] timeout   Timeout reload value written to TOVR.
 * @return Execution status.
 * @retval STATUS_SUCCESS  The timeout register was updated successfully.
 * @retval STATUS_ERROR    Configuration updates are locked.
 *
 * @note The caller is responsible for providing a value that is greater than
 *       FEATURE_WDG_MINIMUM_TIMEOUT_VALUE and consistent with the current
 *       window setting.
 */
status_t WDG_DRV_SetTimeout(uint32_t instance, uint32_t timeout);

/*! @} */ /* End of Configuration */

/*******************************************************************************
 * Watchdog Operation
 ******************************************************************************/
/*!
 * @name Watchdog Operation
 * @brief Functions for refreshing (feeding) the WDG counter and reading
 *        its current value.
 * @{
 */

/*!
 * @brief Refresh the watchdog counter.
 *
 * Issues the hardware trigger sequence that reloads the watchdog counter and
 * prevents timeout as long as it is called at the correct time.
 *
 * @param[in] instance  WDG instance index (0-based).
 *
 * @warning In window mode, servicing the watchdog too early causes an
 *          immediate reset.
 */
void WDG_DRV_Trigger(uint32_t instance);

/*!
 * @brief Read the current watchdog counter value.
 *
 * Returns a snapshot of the CNTCVR register.
 *
 * @param[in] instance  WDG instance index (0-based).
 * @return Current watchdog counter value.
 */
uint32_t WDG_DRV_GetCounter(uint32_t instance);

/*! @} */ /* End of Watchdog Operation */

/*******************************************************************************
 * Interrupt Management
 ******************************************************************************/
/*!
 * @name Interrupt Management
 * @brief Functions for managing WDG timeout interrupt generation and
 *        clearing interrupt flags.
 * @{
 */

/*!
 * @brief Enable or disable interrupt-before-reset mode.
 *
 * Updates the watchdog IBR control bit. When enabled, the first timeout
 * generates an interrupt and the next timeout can reset the device if the
 * watchdog is not serviced.
 *
 * @param[in] instance  WDG instance index (0-based).
 * @param[in] enable    true enables interrupt-before-reset mode; false
 *                      disables it.
 * @return Execution status.
 * @retval STATUS_SUCCESS  The interrupt mode was updated successfully.
 * @retval STATUS_ERROR    Configuration updates are locked.
 */
status_t WDG_DRV_SetInt(uint32_t instance, bool enable);

/*!
 * @brief Clear the pending watchdog interrupt flag.
 *
 * Acknowledges a timeout interrupt that has already been signaled by the
 * hardware.
 *
 * @param[in] instance  WDG instance index (0-based).
 */
void WDG_DRV_ClearIntFlag(uint32_t instance);

/*! @} */ /* End of Interrupt Management */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* WDG_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
