/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file wdg_hw_access.h
 * @version 1.4.1
 *
 * @brief WDG HW Access Layer — inline register-level helper functions.
 *
 * This internal header provides static-inline helpers for direct access to the
 * watchdog registers. These helpers are used by the driver layer to trigger
 * refresh sequences, configure window mode, clear interrupt flags, update
 * debug and deep-sleep behavior, and query watchdog state.
 */

#ifndef WDG_HW_ACCESS_H
#define WDG_HW_ACCESS_H

#include <stddef.h>
#include "wdg_driver.h"
#include "interrupt_manager.h"

/*!
 * @brief Watchdog Timer hardware access helper layer.
 *
 * This hardware access layer provides low-level access to all watchdog
 * hardware features.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* The dummy read is used in order to make sure that any write to the
 * WDG registers will be started only after the write of the unlock value was
 * completed.
 */
#define WDG_UNLOCK(base)  do\
{ \
    (base)->SVCR = FEATURE_WDG_UNLOCK_VALUE_1; \
    (base)->SVCR = FEATURE_WDG_UNLOCK_VALUE_2; \
} while(0)

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name WDG Common Configurations
 * @{
 */

/*!
 * @brief Restore the watchdog to reset-like values.
 *
 * @param[in] base  WDG base pointer.
 */
void WDG_Deinit(WDG_Type *const base);

/*!
 * @brief Enable or disable the WDG timeout interrupt.
 *
 * @param[in] base    WDG base pointer.
 * @param[in] enable  `true` enables the timeout interrupt.
 */
void WDG_SetInt(WDG_Type *const base,
                bool enable);

/*!
 * @brief Configure the watchdog from the user configuration.
 *
 * @param[in] base           WDG base pointer.
 * @param[in] wdgUserConfig  Pointer to user configuration structure.
 * @return Execution status.
 */
status_t WDG_Config(WDG_Type *const base,
                    const wdg_user_config_t *wdgUserConfig);

/*!
 * @brief Read back the current watchdog configuration.
 *
 * @param[in] base    WDG base pointer.
 * @param[out] config WDG configuration pointer.
 */
void WDG_GetConfig(const WDG_Type *base, wdg_user_config_t *const config);

/*!
 * @brief Check whether the watchdog is enabled.
 *
 * @param[in] base  WDG base pointer.
 * @return `true` if the watchdog is enabled, `false` otherwise.
 */
static inline bool WDG_IsEnabled(const WDG_Type *base)
{
    return (((base->CR & WDG_CR_EN_MASK) >> WDG_CR_EN_SHIFT) != 0U);
}

/*!
 * @brief Refresh the watchdog counter.
 *
 * @param[in] base  WDG base pointer.
 */
static inline void WDG_Trigger(WDG_Type *const base)
{
    SDK_ENTER_CRITICAL();
    base->SVCR = FEATURE_WDG_TRIGGER_VALUE_1;
    base->SVCR = FEATURE_WDG_TRIGGER_VALUE_2;
    SDK_EXIT_CRITICAL();
}

/*!
 * @brief Enable or disable watchdog window mode.
 *
 * @param[in] base    WDG base pointer.
 * @param[in] enable  `true` enables window mode.
 */
static inline void WDG_SetWindowMode(WDG_Type *const base,
                                     bool enable)
{
    SDK_ENTER_CRITICAL();
    WDG_UNLOCK(base);
    base->CR = (base->CR & ~WDG_CR_WIN_MASK) | WDG_CR_WIN(enable ? 1UL : 0UL);
    SDK_EXIT_CRITICAL();
}

/*!
 * @brief Set the watchdog window threshold value.
 *
 * @param[in] base    WDG base pointer.
 * @param[in] window  Window threshold value.
 */
static inline void WDG_SetWindowValue(WDG_Type *const base,
                                      uint32_t window)
{
    SDK_ENTER_CRITICAL();
    WDG_UNLOCK(base);
    base->WVR = window;
    SDK_EXIT_CRITICAL();
}

/*!
 * @brief Clear the watchdog interrupt flag.
 *
 * @param[in] base  WDG base pointer.
 */
static inline void WDG_ClearIntFlag(WDG_Type *const base)
{
    SDK_ENTER_CRITICAL();
    WDG_UNLOCK(base);
    base->INTF = WDG_INTF_IF_MASK;
    SDK_EXIT_CRITICAL();
}

/*!
 * @brief Check whether watchdog configuration updates are allowed.
 *
 * @param[in] base  WDG base pointer.
 * @return `true` if updates are allowed, `false` otherwise.
 */
static inline bool WDG_IsUpdateEnabled(const WDG_Type *base)
{
    return ((base->LR & WDG_LR_HL_MASK) >> WDG_LR_HL_SHIFT) == 0U;
}

/*!
 * @brief Enable or disable watchdog operation during debug halt.
 *
 * @param[in] base    WDG base pointer.
 * @param[in] enable  `true` keeps the watchdog active in debug mode.
 */
static inline void WDG_SetDebug(WDG_Type *const base,
                                bool enable)
{
    SDK_ENTER_CRITICAL();
    WDG_UNLOCK(base);
    /*LDRA_NOANALYSIS*/
    /* The comment LDRA_NOANALYSIS only use to run code coverage */
    base->CR = (base->CR & ~WDG_CR_DBGDIS_MASK) | WDG_CR_DBGDIS(enable ? 0UL : 1UL);
    /*LDRA_ANALYSIS*/
    /* The comment LDRA_ANALYSIS only use to run code coverage */
    SDK_EXIT_CRITICAL();
}

/*!
 * @brief Enable or disable watchdog operation in deep-sleep mode.
 *
 * @param[in] base    WDG base pointer.
 * @param[in] enable  `true` keeps the watchdog active in deep-sleep mode.
 */
static inline void WDG_SetDeepSleep(WDG_Type *const base,
                                    bool enable)
{
    SDK_ENTER_CRITICAL();
    WDG_UNLOCK(base);
    /*LDRA_NOANALYSIS*/
    /* The comment LDRA_NOANALYSIS only use to run code coverage */
    base->CR = (base->CR & ~WDG_CR_DSDIS_MASK) | WDG_CR_DSDIS(enable ? 0UL : 1UL);
    /*LDRA_ANALYSIS*/
    /* The comment LDRA_ANALYSIS only use to run code coverage */
    SDK_EXIT_CRITICAL();
}

/*!
 * @brief Check whether the watchdog configuration registers are unlocked.
 *
 * @param[in] base  WDG base pointer.
 * @return `true` if the module is unlocked, `false` otherwise.
 */
static inline bool WDG_IsUnlocked(const WDG_Type *base)
{
    return ((((base->LR & WDG_LR_SL_MASK) >> WDG_LR_SL_SHIFT) == 0U)
           && (((base->LR & WDG_LR_HL_MASK) >> WDG_LR_HL_SHIFT) == 0U));
}

/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* WDG_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
