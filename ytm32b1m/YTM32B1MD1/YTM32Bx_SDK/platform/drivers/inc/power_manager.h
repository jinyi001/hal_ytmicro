/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file power_manager.h
 * @version 1.4.1
 *
 * @brief Power Manager — Public API for system power mode management.
 *
 * This header defines the application-level interface for the Power Manager.
 * It provides APIs for initializing the power management framework, switching
 * between pre-defined power modes, and managing notification callbacks that
 * are invoked before and after power mode transitions.
 *
 * The APIs are organized into three categories:
 *   - **Initialization & De-initialization**: Set up or tear down the Power Manager.
 *   - **Power Mode Control**: Switch power modes and query the current mode.
 *   - **Status & Error Query**: Retrieve transition results and callback errors.
 *
 * @note The actual power mode definitions and hardware-specific implementation
 *       are provided by the platform-specific header (included automatically
 *       based on the target CPU).
 */

#ifndef POWER_MANAGER_H_
#define POWER_MANAGER_H_

#include "device_registers.h"
#include "status.h"

/*
 * Include the cpu API header files.
 */
#if (defined(CPU_YTM32B1LD0))
    /* YTM32B1LD0 power management API header file */
    #include "../src/power/YTM32B1LD0/power_manager_YTM32B1LD0.h"
#elif defined(YTM32B1L_SERIES)
    /* YTM32B1L series power management API header file */
    #include "../src/power/YTM32B1Lx/power_manager_YTM32B1Lx.h"
#elif defined(YTM32B1M_SERIES)
    /* YTM32B1MD0 power management API header file */
    #include "../src/power/YTM32B1Mx/power_manager_YTM32B1Mx.h"
#elif defined(YTM32B1H_SERIES)
    /* YTM32B1Hx power management API header file */
    #include "../src/power/YTM32B1Hx/power_manager_YTM32B1Hx.h"
#elif defined(YTM32Z1M_SERIES)
    /* YTM32Z1x power management API header file */
    #include "../src/power/YTM32Z1x/power_manager_YTM32Z1x.h"
#elif defined(YTM32Z1DS_SERIES)
    /* YTM32Z1x power management API header file */
    #include "../src/power/YTM32Z1DSx/power_manager_YTM32Z1DSx.h"
#else
    #error "No valid CPU defined!"
#endif

/*!
 * @defgroup power_manager Power Manager
 * @brief System Power Mode Management — Public API.
 * @details Provides APIs for initializing the power management framework,
 *          switching between pre-defined power modes with callback
 *          notification, and querying transition status. Supports both
 *          agreement-based and forced mode-change policies.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Power manager mode-change policy.
 *
 * Controls whether a power mode switch initiated by POWER_SYS_SetMode()
 * requires agreement from all registered callbacks or is forced regardless
 * of callback results.
 *
 * | Value                          | Behavior                                    |
 * |--------------------------------|---------------------------------------------|
 * | POWER_MANAGER_POLICY_AGREEMENT | Abort switch if any callback returns error.  |
 * | POWER_MANAGER_POLICY_FORCIBLE  | Force switch regardless of callback results. |
 */
typedef enum
{
    POWER_MANAGER_POLICY_AGREEMENT,      /*!< Power mode is changed if all of the callbacks return success. */
    POWER_MANAGER_POLICY_FORCIBLE        /*!< Power mode is changed regardless of the result of callbacks. */
} power_manager_policy_t;

/*!
 * @brief Callback notification type.
 *
 * Specifies the phase of a power mode transition during which a registered
 * callback is being invoked.
 *
 * | Value                        | Phase                                            |
 * |------------------------------|--------------------------------------------------|
 * | POWER_MANAGER_NOTIFY_RECOVER | Recovery after a failed mode switch attempt.      |
 * | POWER_MANAGER_NOTIFY_BEFORE  | Before the power mode change is applied.          |
 * | POWER_MANAGER_NOTIFY_AFTER   | After the power mode change has been applied.     |
 */
typedef enum
{
    POWER_MANAGER_NOTIFY_RECOVER = 0x00U,  /*!< Notify IP to recover to previous work state.      */
    POWER_MANAGER_NOTIFY_BEFORE = 0x01U,  /*!< Notify IP that the system will change the power setting.  */
    POWER_MANAGER_NOTIFY_AFTER = 0x02U   /*!< Notify IP that the system has changed to a new power setting. */
} power_manager_notify_t;

/*!
 * @brief Callback invocation type.
 *
 * Specifies when a registered callback shall be invoked during power mode
 * transitions initiated by POWER_SYS_SetMode().
 *
 * | Value                                | When invoked                    |
 * |--------------------------------------|---------------------------------|
 * | POWER_MANAGER_CALLBACK_BEFORE        | Before mode switch only.        |
 * | POWER_MANAGER_CALLBACK_AFTER         | After mode switch only.         |
 * | POWER_MANAGER_CALLBACK_BEFORE_AFTER  | Both before and after switch.   |
 */
typedef enum
{
    POWER_MANAGER_CALLBACK_BEFORE = 0x01U, /*!< Before callback. */
    POWER_MANAGER_CALLBACK_AFTER = 0x02U, /*!< After callback. */
    POWER_MANAGER_CALLBACK_BEFORE_AFTER = 0x03U  /*!< Before-After callback. */
} power_manager_callback_type_t;

/*!
 * @brief Opaque callback data type.
 *
 * Pointer to application-specific data passed during callback registration.
 * This pointer is forwarded to the callback function on every notification.
 */
typedef void power_manager_callback_data_t;

/*!
 * @brief Power mode user configuration structure.
 *
 * Defines a target power mode along with additional power options.
 * The actual structure members are platform-specific and defined in the
 * SOC-specific header (see power_manager_modes_t). Applications define
 * multiple power modes and pass them as an array to POWER_SYS_Init().
 *
 * @note Refer to the platform-specific header for the full definition
 *       of this structure (e.g., power_manager_YTM32B1Mx.h).
 */

/*!
 * @brief Callback notification structure.
 *
 * Passed to registered callback functions during power mode transition
 * notifications. Contains the target power configuration and the
 * notification type (before/after/recover).
 *
 * | Field                  | Type                           | Description                          |
 * |------------------------|--------------------------------|--------------------------------------|
 * | targetPowerConfigPtr   | power_manager_user_config_t *  | Pointer to target power config.      |
 * | targetPowerConfigIndex | uint8_t                        | Index of target config in the array. |
 * | policy                 | power_manager_policy_t         | Mode transition policy.              |
 * | notifyType             | power_manager_notify_t         | Notification phase.                  |
 */
typedef struct
{
    power_manager_user_config_t *targetPowerConfigPtr; /*!< Pointer to target power configuration */
    uint8_t targetPowerConfigIndex;    /*!< Target power configuration index. */
    power_manager_policy_t policy;     /*!< Power mode transition policy.          */
    power_manager_notify_t notifyType; /*!< Power mode notification type.          */
} power_manager_notify_struct_t;

/*!
 * @brief Callback function prototype.
 *
 * All registered callbacks must conform to this signature. The callback
 * receives a notification structure describing the transition and an
 * optional data pointer provided at registration time.
 *
 * When notified before a mode change with POWER_MANAGER_POLICY_AGREEMENT,
 * the callback may deny the transition by returning any status code other
 * than STATUS_SUCCESS.
 *
 * @param[in] notify   Pointer to the notification structure.
 * @param[in] dataPtr  Pointer to callback-specific data (may be NULL).
 * @return STATUS_SUCCESS to allow the transition, or an error code to deny it.
 */
typedef status_t (*power_manager_callback_t)(power_manager_notify_struct_t *notify,
                                             power_manager_callback_data_t *dataPtr);

/*!
 * @brief Callback configuration structure.
 *
 * Holds the configuration for a single callback registered with the
 * Power Manager. Structures of this type should be statically allocated
 * and remain valid for the lifetime of the Power Manager.
 *
 * | Field            | Type                             | Description                          |
 * |------------------|----------------------------------|--------------------------------------|
 * | callbackFunction | power_manager_callback_t         | Pointer to the callback function.    |
 * | callbackType     | power_manager_callback_type_t    | When to invoke (before/after/both).  |
 * | callbackData     | power_manager_callback_data_t *  | Application data passed to callback. |
 */
typedef struct
{
    power_manager_callback_t callbackFunction;
    power_manager_callback_type_t callbackType;
    power_manager_callback_data_t *callbackData;
} power_manager_callback_user_config_t;

/*!
 * @brief Power Manager internal state structure.
 *
 * Stores references to registered power mode configurations, callbacks,
 * and runtime state. This structure is statically allocated and initialized
 * by POWER_SYS_Init().
 *
 * @warning This structure is for internal use only. Do not modify its
 *          members directly from application code.
 */
typedef struct
{
    power_manager_user_config_t *(*configs)[];             /*!< Pointer to power configure table.*/
    uint8_t configsNumber;                                  /*!< Number of power configurations */
    power_manager_callback_user_config_t *(*staticCallbacks)[]; /*!< Pointer to callback table. */
    uint8_t staticCallbacksNumber;                          /*!< Max. number of callback configurations */
    uint8_t errorCallbackIndex;                             /*!< Index of callback returns error. */
    uint8_t currentConfig;                                  /*!< Index of current configuration.  */
} power_manager_state_t;

/*! @brief Power manager internal state instance. */
extern power_manager_state_t gPowerManagerState;

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
 * @brief Functions for setting up and tearing down the Power Manager.
 * @{
 */

/*!
 * @brief Initialize the Power Manager for operation.
 *
 * Initializes the Power Manager and its internal run-time state. An array
 * of power mode configuration structures and an optional array of callback
 * configurations must be provided. The Power Manager stores only pointers
 * to these arrays, so they must remain valid for the entire lifetime of
 * the Power Manager.
 *
 * @param[in] powerConfigsPtr  Pointer to an array of pointers to power mode
 *                              configurations. Must not be NULL.
 * @param[in] configsNumber    Number of power mode configurations in the array.
 *                              Must be at least 1.
 * @param[in] callbacksPtr     Pointer to an array of pointers to callback
 *                              configurations. Use NULL if no callbacks.
 * @param[in] callbacksNumber  Number of callback configurations. Use 0 if
 *                              no callbacks.
 * @return Execution status.
 * @retval STATUS_SUCCESS  Initialization completed successfully.
 * @retval STATUS_ERROR    Initialization failed.
 *
 * @pre  The Power Manager must not already be initialized.
 * @post The Power Manager is ready, with default mode index set to 0.
 */
status_t POWER_SYS_Init(power_manager_user_config_t *(*powerConfigsPtr)[],
                        uint8_t configsNumber,
                        power_manager_callback_user_config_t *(*callbacksPtr)[],
                        uint8_t callbacksNumber);

/*!
 * @brief De-initialize the Power Manager.
 *
 * Clears the internal state and removes all registered configurations
 * and callbacks. After calling this function, POWER_SYS_Init() must be
 * called again before using any other Power Manager API.
 *
 * @return Execution status.
 * @retval STATUS_SUCCESS  De-initialization completed successfully.
 */
status_t POWER_SYS_Deinit(void);

/*!
 * @brief Populate a configuration structure with default power mode values.
 *
 * Fills the configuration structure with the platform-specific default
 * values (typically RUN mode with sleep-on-exit disabled).
 *
 * @param[out] config  Pointer to the configuration structure to fill.
 *                     Must not be NULL.
 *
 * @note The default configuration is platform-specific. See the SOC-specific
 *       header for the actual default values.
 */
void POWER_SYS_GetDefaultConfig(power_manager_user_config_t *const config);

/*! @} */ /* End of Initialization & De-initialization */

/*******************************************************************************
 * Power Mode Control
 ******************************************************************************/
/*!
 * @name Power Mode Control
 * @brief Functions for switching and querying the active power mode.
 * @{
 */

/*!
 * @brief Switch to a specified power mode.
 *
 * Switches to one of the pre-defined power modes. The function performs
 * the following sequence:
 *   1. Notify all registered BEFORE callbacks.
 *   2. If policy is AGREEMENT and any callback returns error, abort and
 *      invoke RECOVER notifications on already-notified callbacks.
 *   3. Apply the hardware power mode switch.
 *   4. Notify all registered AFTER callbacks.
 *
 * For sleep/deep-sleep modes, AFTER callbacks are invoked upon wake-up.
 *
 * @param[in] powerModeIndex  Index into the power mode configuration array
 *                            passed to POWER_SYS_Init().
 * @param[in] policy          Mode change policy (AGREEMENT or FORCIBLE).
 * @return Execution status.
 * @retval STATUS_SUCCESS               Mode switch completed successfully.
 * @retval STATUS_MCU_NOTIFY_BEFORE_ERROR  A BEFORE callback returned error.
 * @retval STATUS_MCU_NOTIFY_AFTER_ERROR   An AFTER callback returned error.
 * @retval STATUS_MCU_TRANSITION_FAILED    Hardware mode switch failed.
 *
 * @pre  POWER_SYS_Init() must have been called.
 * @pre  powerModeIndex must be less than configsNumber.
 *
 * @note Use POWER_SYS_GetErrorCallbackIndex() or POWER_SYS_GetErrorCallback()
 *       to identify the failing callback when an error is returned.
 */
status_t POWER_SYS_SetMode(uint8_t powerModeIndex,
                           power_manager_policy_t policy);


/*!
 * @brief Get the currently active hardware power mode.
 *
 * Reads the hardware state and returns the current power mode status.
 *
 * @param[in] configPtr  Pointer to a power mode configuration structure.
 * @return The current power mode status value.
 */
power_mode_stat_t POWER_SYS_GetCurrentMode(power_manager_user_config_t *configPtr);

/*! @} */ /* End of Power Mode Control */

/*******************************************************************************
 * Status & Error Query
 ******************************************************************************/
/*!
 * @name Status & Error Query
 * @brief Functions for retrieving power mode transition results and errors.
 * @{
 */

/*!
 * @brief Get the index of the last successfully set power mode.
 *
 * Returns the index of the power mode configuration that was last set
 * via POWER_SYS_SetMode(). If any callback returned an error during
 * the transition, the return code is STATUS_ERROR.
 *
 * @param[out] powerModeIndexPtr  Pointer to receive the power mode index.
 * @return Execution status.
 * @retval STATUS_SUCCESS  All callbacks executed without error.
 * @retval STATUS_ERROR    One or more callbacks returned error.
 */
status_t POWER_SYS_GetLastMode(uint8_t * powerModeIndexPtr);

/*!
 * @brief Get the configuration of the last successfully set power mode.
 *
 * Returns a pointer to the configuration structure of the last power mode
 * set via POWER_SYS_SetMode(). If any callback returned an error, the
 * return code is STATUS_ERROR.
 *
 * @param[out] powerModePtr  Pointer to receive the configuration pointer.
 * @return Execution status.
 * @retval STATUS_SUCCESS  All callbacks executed without error.
 * @retval STATUS_ERROR    One or more callbacks returned error.
 */
status_t POWER_SYS_GetLastModeConfig(power_manager_user_config_t ** powerModePtr);

/*!
 * @brief Get the index of the last failed notification callback.
 *
 * Returns the index of the last callback that returned an error during
 * a power mode switch. If the last POWER_SYS_SetMode() call was
 * successful, a value equal to the total number of registered callbacks
 * is returned.
 *
 * @return Index of the failing callback, or callbacksNumber if no error.
 */
uint8_t POWER_SYS_GetErrorCallbackIndex(void);

/*!
 * @brief Get the configuration of the last failed notification callback.
 *
 * Returns a pointer to the configuration structure of the last callback
 * that returned an error. If the last POWER_SYS_SetMode() call was
 * successful, NULL is returned.
 *
 * @return Pointer to the failing callback configuration, or NULL.
 */
power_manager_callback_user_config_t *POWER_SYS_GetErrorCallback(void);

/*! @} */ /* End of Status & Error Query */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of power_manager group */

#endif /* POWER_MANAGER_H_ */

/*******************************************************************************
 * EOF
 ******************************************************************************/
