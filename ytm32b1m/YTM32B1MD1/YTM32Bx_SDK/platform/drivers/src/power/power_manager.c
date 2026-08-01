/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file power_manager.c
 * @version 1.4.1
 *
 * @brief Power Manager — Implementation of the common POWER_SYS_* API.
 *
 * This file implements the platform-independent Power Manager functions
 * declared in power_manager.h. Each public function wraps the low-level
 * platform-specific Do* functions and manages the callback notification
 * mechanism for power mode transitions.
 */

#include <stddef.h>
#include "power_manager.h"

/*******************************************************************************
 * INTERNAL FUNCTIONS
 ******************************************************************************/

static status_t POWER_SYS_CallbacksManagement(power_manager_notify_struct_t * notifyStruct,
                                              uint8_t * currentStaticCallback,
                                              power_manager_policy_t policy);

/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/

/*!
 * @brief Initialize the Power Manager for operation.
 */
status_t POWER_SYS_Init(power_manager_user_config_t * (*powerConfigsPtr)[],
                        uint8_t configsNumber,
                        power_manager_callback_user_config_t * (*callbacksPtr)[],
                        uint8_t callbacksNumber)
{
    DEV_ASSERT(powerConfigsPtr != NULL);                  /* Reference to the power configurations is valid. */
    DEV_ASSERT(configsNumber != 0U);                      /* Power configuration index is valid. */
    DEV_ASSERT(gPowerManagerState.configs == NULL);       /* Driver is not initialized, reference to configuration is not valid. */
    DEV_ASSERT(gPowerManagerState.configsNumber == 0U);   /* Driver is not initialized, number of configurations is zero. */

    /* Store references to user-defined power mode configurations */
    gPowerManagerState.configs = (power_manager_user_config_t * (*)[])powerConfigsPtr;
    gPowerManagerState.configsNumber = configsNumber;
    gPowerManagerState.currentConfig = 0U;

    /* Store references to user-defined callback configurations and increment call-back handle counter */
    if (callbacksPtr != NULL)
    {
        gPowerManagerState.staticCallbacks = (power_manager_callback_user_config_t * (*)[])callbacksPtr;
        gPowerManagerState.staticCallbacksNumber = callbacksNumber;
        /* Default value of handle of last call-back that returned error */
        gPowerManagerState.errorCallbackIndex = callbacksNumber;
    }
    else
    {
        gPowerManagerState.staticCallbacks = NULL;
        gPowerManagerState.staticCallbacksNumber = 0U;
        gPowerManagerState.errorCallbackIndex = 0U;
    }

    return POWER_SYS_DoInit();
}

/*!
 * @brief De-initialize the Power Manager.
 */
status_t POWER_SYS_Deinit(void)
{
    gPowerManagerState.configs = NULL;
    gPowerManagerState.configsNumber = 0U;
    gPowerManagerState.staticCallbacks = NULL;
    gPowerManagerState.staticCallbacksNumber = 0U;

    return POWER_SYS_DoDeinit();
}

/*******************************************************************************
 * Status & Error Query
 ******************************************************************************/

/*!
 * @brief Get the index of the last successfully set power mode.
 */
status_t POWER_SYS_GetLastMode(uint8_t * powerModeIndexPtr)
{
    status_t returnCode; /* Function return */

    /* Pass index of user-defined configuration structure of currently running power mode */
    *powerModeIndexPtr = gPowerManagerState.currentConfig;

    /* Return whether all call-backs executed without error */
    if (gPowerManagerState.errorCallbackIndex == gPowerManagerState.staticCallbacksNumber)
    {
        returnCode = STATUS_SUCCESS;
    }
    else
    {
        returnCode = STATUS_ERROR;
    }

    return returnCode;
}

/*!
 * @brief Get the configuration of the last successfully set power mode.
 */
status_t POWER_SYS_GetLastModeConfig(power_manager_user_config_t ** powerModePtr)
{
    status_t returnCode; /* Function return */
    /* Pass reference to user-defined configuration structure of currently running power mode */
    *powerModePtr = (*gPowerManagerState.configs)[gPowerManagerState.currentConfig];

    /* Return whether all call-backs executed without error */
    if (gPowerManagerState.errorCallbackIndex == gPowerManagerState.staticCallbacksNumber)
    {
        returnCode = STATUS_SUCCESS;
    }
    else
    {
        returnCode = STATUS_ERROR;
    }

    return returnCode;
}

/*!
 * @brief Get the index of the last failed notification callback.
 */
uint8_t POWER_SYS_GetErrorCallbackIndex(void)
{
    return gPowerManagerState.errorCallbackIndex;
}

/*!
 * @brief Get the configuration of the last failed notification callback.
 */
power_manager_callback_user_config_t * POWER_SYS_GetErrorCallback(void)
{
    power_manager_callback_user_config_t *config;
    /* If all callbacks return success. */
    if (gPowerManagerState.errorCallbackIndex >= gPowerManagerState.staticCallbacksNumber)
    {
        config = NULL;
    }
    else
    {
        config = (*gPowerManagerState.staticCallbacks)[gPowerManagerState.errorCallbackIndex];
    }
    return config;
}

/*******************************************************************************
 * Internal: Callback Management
 ******************************************************************************/

/*!
 * @brief Internal callback management for power mode transitions.
 *
 * Iterates through registered callbacks and invokes them according to
 * the notification type and callback type filter. Handles error tracking
 * and policy-based abort logic.
 */
static status_t POWER_SYS_CallbacksManagement(power_manager_notify_struct_t * notifyStruct,
                                              uint8_t * currentStaticCallback,
                                              power_manager_policy_t policy)
{
    uint8_t callbacksNumber = 0U;                                   /* The total number of callbacks */
    status_t returnCode = STATUS_SUCCESS;                      /* Function return */
    status_t errorCode = STATUS_SUCCESS;                       /* Error code to be returned(error case) */
    status_t callbackStatusCode;                               /* Status code returned by callback function */
    power_manager_callback_type_t callbackTypeFilter = POWER_MANAGER_CALLBACK_BEFORE;          /* Callback types to be excluded */

    switch (notifyStruct->notifyType)
    {
        /* notify before */
        case POWER_MANAGER_NOTIFY_BEFORE:
            callbacksNumber = gPowerManagerState.staticCallbacksNumber;
            callbackTypeFilter = POWER_MANAGER_CALLBACK_AFTER;
            errorCode = STATUS_MCU_NOTIFY_BEFORE_ERROR;
            break;
        /* notify after */
        case POWER_MANAGER_NOTIFY_AFTER:
            callbacksNumber = gPowerManagerState.staticCallbacksNumber;
            callbackTypeFilter = POWER_MANAGER_CALLBACK_BEFORE;
            errorCode = STATUS_MCU_NOTIFY_AFTER_ERROR;
            break;
        /* notify recover */
        case POWER_MANAGER_NOTIFY_RECOVER:
            callbacksNumber = gPowerManagerState.staticCallbacksNumber;
            callbackTypeFilter = POWER_MANAGER_CALLBACK_AFTER;
            errorCode = STATUS_MCU_NOTIFY_BEFORE_ERROR;
            break;
        default:
            /* invalid value for notifyType */
            DEV_ASSERT(false);
            break;
    }

    /* From all statically registered call-backs... */
    for ((*currentStaticCallback) = 0U; (*currentStaticCallback) < callbacksNumber; (*currentStaticCallback)++)
    {
        /* Pointer to callback configuration */
        const power_manager_callback_user_config_t * const callbackConfig = ((*gPowerManagerState.staticCallbacks)[*currentStaticCallback]);

        /* Check pointer to static callback configuration */
        if ((callbackConfig != NULL) && (callbackTypeFilter != callbackConfig->callbackType))
        {
            /* In case that call-back returned error code mark it, store the call-back handle and eventually cancel the mode switch */

            callbackStatusCode = callbackConfig->callbackFunction(notifyStruct, callbackConfig->callbackData);
            if (STATUS_SUCCESS != callbackStatusCode)
            {
                returnCode = errorCode;
                gPowerManagerState.errorCallbackIndex = *currentStaticCallback;
                /* If not forcing power mode switch, call all already notified call-backs to revert their state as the mode change is canceled */
                if (policy != POWER_MANAGER_POLICY_FORCIBLE)
                {
                    break;
                }
            }
        }
    }

    return returnCode;
}

/*******************************************************************************
 * Power Mode Control
 ******************************************************************************/

/*!
 * @brief Switch to a specified power mode.
 */
status_t POWER_SYS_SetMode(uint8_t powerModeIndex,
                           power_manager_policy_t policy)
{
    power_manager_user_config_t * configPtr; /* Local pointer to the requested user-defined power mode configuration */
    status_t returnCode; /* Function return */
    status_t errorCode;
    bool successfulSwitch;                                 /* Power mode switch is successful or not */
    uint8_t currentStaticCallback = 0U;                    /* Index to array of statically registered call-backs */
    power_manager_notify_struct_t notifyStruct;            /* Callback notification structure */

    /* Driver is already initialized. */
    DEV_ASSERT(gPowerManagerState.configs != NULL);
    DEV_ASSERT(gPowerManagerState.configsNumber != 0U);

    /* Power mode index is valid. */
    DEV_ASSERT(powerModeIndex < gPowerManagerState.configsNumber);

    /* Initialization of local pointer to the requested user-defined power mode configuration */
    configPtr = (*gPowerManagerState.configs)[powerModeIndex];

    /* Reference to the requested user-defined power mode configuration is valid. */
    DEV_ASSERT(configPtr != NULL);

    /* Default value of handle of last call-back that returned error */
    gPowerManagerState.errorCallbackIndex = gPowerManagerState.staticCallbacksNumber;

    /* Set the transaction policy in the notification structure */
    notifyStruct.policy = policy;

    /* Set the target power mode configuration in the notification structure */
    notifyStruct.targetPowerConfigIndex = powerModeIndex;
    notifyStruct.targetPowerConfigPtr = configPtr;

    /* Notify those which asked to be called before the power mode change */
    notifyStruct.notifyType = POWER_MANAGER_NOTIFY_BEFORE;
    returnCode = POWER_SYS_CallbacksManagement(&notifyStruct, &currentStaticCallback, policy);

    /* Power mode switch */
    /* In case that any call-back returned error code and  policy doesn't force the mode switch go to after switch call-backs */
    if ((policy == POWER_MANAGER_POLICY_FORCIBLE) || (returnCode == STATUS_SUCCESS))
    {
        returnCode = POWER_SYS_DoSetMode(configPtr);
        successfulSwitch = (STATUS_SUCCESS == returnCode);
    }
    else
    {
        /* Unsuccessful switch */
        successfulSwitch = false;
    }

    if (successfulSwitch)
    {
        /* End of successful switch */

        /* Update current configuration index */
        gPowerManagerState.currentConfig = powerModeIndex;

        /* Notify those which asked to be called after the power mode change */
        notifyStruct.notifyType = POWER_MANAGER_NOTIFY_AFTER;
        returnCode = POWER_SYS_CallbacksManagement(&notifyStruct, &currentStaticCallback, POWER_MANAGER_POLICY_FORCIBLE);
    }
    else
    {
        /* End of unsuccessful switch */

        /* Notify those which have been called before the power mode change */
        notifyStruct.notifyType = POWER_MANAGER_NOTIFY_RECOVER;
        errorCode = POWER_SYS_CallbacksManagement(&notifyStruct, &currentStaticCallback, POWER_MANAGER_POLICY_FORCIBLE);
        (void)(errorCode);
    }

    return returnCode;
}

/*******************************************************************************
 * Default Configuration
 ******************************************************************************/

/*!
 * @brief Populate a configuration structure with default power mode values.
 */
void POWER_SYS_GetDefaultConfig(power_manager_user_config_t * const config)
{
    POWER_SYS_DoGetDefaultConfig(config);
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
