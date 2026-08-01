/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file power_manager_YTM32B1Mx.h
 * @version 1.4.1
 *
 * @brief Power Manager — YTM32B1Mx platform-specific definitions and API.
 *
 * This header defines the power modes, configuration structures, and
 * platform-specific functions for the YTM32B1M series MCUs. It is
 * automatically included by power_manager.h based on the target CPU.
 */

#ifndef POWER_MANAGER_YTM32B1Mx_H
#define POWER_MANAGER_YTM32B1Mx_H

#include "device_registers.h"
#include "status.h"

/*!
 * @ingroup power_manager
 * @defgroup power_ytm32b1mx Power Manager (YTM32B1Mx)
 * @brief YTM32B1Mx-specific power mode definitions and implementation.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Power mode enumeration for YTM32B1Mx devices.
 *
 * Lists the power modes available on the YTM32B1M series. The available
 * modes depend on the specific device variant and feature macros.
 *
 * | Value                      | Description                                   |
 * |----------------------------|-----------------------------------------------|
 * | POWER_MANAGER_RUN          | Normal run mode, full performance.            |
 * | POWER_MANAGER_SLEEP        | Core in sleep, normal interrupt wakeup.       |
 * | POWER_MANAGER_DEEPSLEEP    | Most clocks gated, async interrupt wakeup.    |
 * | POWER_MANAGER_STANDBY      | LDO in low power, async interrupt wakeup.     |
 * | POWER_MANAGER_POWERDOWN    | Partial digital powered, WKU wakeup.          |
 * | POWER_MANAGER_DEEPPOWERDOWN| Deepest low-power, limited wakeup sources.    |
 */
typedef enum
{
    POWER_MANAGER_RUN,              /*!< Run mode. */
    POWER_MANAGER_SLEEP,            /*!< Sleep mode.  */
    POWER_MANAGER_DEEPSLEEP,        /*!< Deepsleep mode. */
    POWER_MANAGER_STANDBY,          /*!< Standby mode. */
#if defined(FEATURE_SOC_SUPPORT_POWERDOWN_MODE) && FEATURE_SOC_SUPPORT_POWERDOWN_MODE
    POWER_MANAGER_POWERDOWN,        /*!< Power down mode. */
#endif
#if defined(FEATURE_SOC_SUPPORT_DEEPPOWERDOWN_MODE) && FEATURE_SOC_SUPPORT_DEEPPOWERDOWN_MODE
    POWER_MANAGER_DEEPPOWERDOWN,    /*!< Deep power down mode. */
#endif
} power_manager_modes_t;

/*!
 * @brief Power mode user configuration structure for YTM32B1Mx.
 *
 * | Field           | Type                    | Description                                        |
 * |-----------------|-------------------------|----------------------------------------------------|
 * | powerMode       | power_manager_modes_t   | Target power mode to enter.                        |
 * | sleepOnExitValue| bool                    | If true, core returns to sleep after ISR completes.|
 */
typedef struct
{
    power_manager_modes_t powerMode;
    bool sleepOnExitValue;
} power_manager_user_config_t;

/*!
 * @brief Power mode status enumeration.
 *
 * Reports the current hardware power mode state.
 *
 * | Value          | Description                    |
 * |----------------|--------------------------------|
 * | STAT_RUN       | Currently in RUN mode.         |
 * | STAT_SLEEP     | Currently in SLEEP mode.       |
 * | STAT_DEEPSLEEP | Currently in DEEPSLEEP mode.   |
 * | STAT_STANDBY   | Currently in STANDBY mode.     |
 * | STAT_INVALID   | Invalid / unknown power mode.  |
 */
typedef enum
{
    STAT_RUN          = 0x01,              /*!< 0000_0001 - Current power mode is RUN*/
    STAT_SLEEP        = 0x02,              /*!< 0000_0010 - Current power mode is SLEEP*/
    STAT_DEEPSLEEP    = 0x04,              /*!< 0000_0100 - Current power mode is DEEPSLEEP*/
    STAT_STANDBY      = 0x08,              /*!< 0000_1000 - Current power mode is STANDBY*/
    STAT_INVALID      = 0xFF               /*!< 1111_1111 - Non-existing power mode*/
} power_mode_stat_t;

/*!
 * @brief System reset source enumeration for YTM32B1Mx.
 *
 * Identifies the source of a system reset event.
 *
 * | Value                       | Description                          |
 * |-----------------------------|--------------------------------------|
 * | RCU_EXTERNAL_PIN            | External pin reset.                  |
 * | RCU_CMU_LOSS_OF_CLOCK       | Clock monitor unit loss-of-clock.    |
 * | RCU_SOFTWARE                | Software-triggered reset.            |
 * | RCU_WDG                     | Watchdog timer reset.                |
 * | RCU_FORCE_POWER_ON          | Forced power-on reset.               |
 * | RCU_POWER_ON                | Normal power-on reset.               |
 * | RCU_CORE_LOCKUP             | ARM core lockup reset.               |
 * | RCU_DEEPSLEEP_ACK_TIMEOUT   | Deep sleep acknowledge timeout.      |
 */
typedef enum
{
    RCU_EXTERNAL_PIN             = 1U,       /*!< External pin reset */
#if defined(FEATURE_RCU_HAS_CMU_LOSS_OF_CLOCK) && (FEATURE_RCU_HAS_CMU_LOSS_OF_CLOCK == 1)
    RCU_CMU_LOSS_OF_CLOCK        = 2U,       /*!< Clock monitor reset */
#endif
    RCU_SOFTWARE                 = 3U,       /*!< Software reset */
    RCU_WDG                      = 4U,       /*!< Watch dog reset */
    RCU_FORCE_POWER_ON           = 5U,       /*!< Force power on reset */
    RCU_POWER_ON                 = 6U,       /*!< Power on reset */
    RCU_CORE_LOCKUP              = 7U,       /*!< Core lockup reset */
    RCU_DEEPSLEEP_ACK_TIMEOUT    = 8U,       /*!<Deepsleep acknowledge timeout reset */
    RCU_SRC_NAME_MAX
} rcu_source_names_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Platform Initialization
 ******************************************************************************/
/*!
 * @name Platform Initialization
 * @brief YTM32B1Mx-specific initialization and default configuration.
 * @{
 */

/*!
 * @brief Perform platform-specific power manager initialization.
 *
 * Called internally by POWER_SYS_Init(). Performs any YTM32B1Mx-specific
 * hardware setup required for power mode management.
 *
 * @return Execution status.
 * @retval STATUS_SUCCESS  Initialization completed successfully.
 * @retval STATUS_ERROR    Initialization failed.
 */
status_t POWER_SYS_DoInit(void);

/*!
 * @brief Perform platform-specific power manager de-initialization.
 *
 * Called internally by POWER_SYS_Deinit(). Restores platform-specific
 * hardware to its default state.
 *
 * @return Execution status.
 * @retval STATUS_SUCCESS  De-initialization completed successfully.
 * @retval STATUS_ERROR    De-initialization failed.
 */
status_t POWER_SYS_DoDeinit(void);

/*!
 * @brief Get the default power mode configuration for YTM32B1Mx.
 *
 * Fills the configuration structure with the default values:
 *   - powerMode = POWER_MANAGER_RUN
 *   - sleepOnExitValue = false
 *
 * @param[out] defaultConfig  Pointer to the configuration to fill.
 *                            Must not be NULL.
 */
static inline void POWER_SYS_DoGetDefaultConfig(power_manager_user_config_t * const defaultConfig)
{
    defaultConfig->powerMode = POWER_MANAGER_RUN;    /*!< Power manager mode  */
    defaultConfig->sleepOnExitValue = false;         /*!< Sleep on exit value */
}

/*! @} */ /* End of Platform Initialization */

/*******************************************************************************
 * Platform Mode Control
 ******************************************************************************/
/*!
 * @name Platform Mode Control
 * @brief YTM32B1Mx-specific power mode switching.
 * @{
 */

/*!
 * @brief Perform platform-specific power mode switch.
 *
 * Called internally by POWER_SYS_SetMode(). Configures the ARM SCB and
 * PCU registers and executes the WFI/STANDBY instruction to enter the
 * requested power mode.
 *
 * @param[in] configPtr  Pointer to the target power mode configuration.
 *                       Must not be NULL.
 * @return Execution status.
 * @retval STATUS_SUCCESS              Mode switch completed successfully.
 * @retval STATUS_MCU_TRANSITION_FAILED Mode switch failed.
 */
status_t POWER_SYS_DoSetMode(const power_manager_user_config_t * const configPtr);

/*! @} */ /* End of Platform Mode Control */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of power_ytm32b1mx group */

#endif /* POWER_MANAGER_YTM32B1Mx_H */

/*******************************************************************************
 * EOF
 ******************************************************************************/
