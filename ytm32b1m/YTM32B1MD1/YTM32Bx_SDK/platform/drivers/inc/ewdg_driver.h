/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file ewdg_driver.h
 * @version 1.4.1
 *
 * @brief EWDG Driver — Public API for the External Watchdog Monitor.
 *
 * This header defines the application-level interface for the EWDG peripheral.
 * The driver provides instance-based APIs for one-shot watchdog configuration,
 * periodic refresh servicing, input-pin assert-mode query, and interrupt
 * masking.
 *
 * The APIs are organized into three categories:
 *   - **Initialization & De-initialization**: Program or restore the watchdog configuration.
 *   - **Watchdog Service**: Refresh the watchdog counter inside the programmed window.
 *   - **Status & Interrupt Control**: Read input-pin behavior and disable interrupt generation.
 *
 * @note Configure clocks and external EWDG pins before calling EWDG_DRV_Init().
 * @note The primary watchdog configuration registers are intended to be
 *       programmed once per reset, so the application should choose the window
 *       and pin behavior before enabling the module.
 */

#ifndef EWDG_DRIVER_H
#define EWDG_DRIVER_H

#include "device_registers.h"
#include "status.h"

/*******************************************************************************
 * Doxygen Group
 ******************************************************************************/

/*!
 * @addtogroup ewdg
 * @brief External Watchdog Monitor Driver — Public API.
 * @details Provides instance-based APIs for configuring the external watchdog,
 *          refreshing its service sequence, querying the `EWDG_in` assert
 *          mode, and disabling the watchdog interrupt after initialization.
 * @{
 */

/*******************************************************************************
* Definitions
******************************************************************************/
#if (defined(EWDG_CLK_CTRL_CLKSEL_MASK))
/*!
 * @brief EWDG counter clock source selection.
 *
 * Selects the clock path that drives the EWDG prescaler and timeout counter
 * on devices that expose the `CLKSEL` field.
 *
 * | Value            | Description                                                          |
 * |------------------|----------------------------------------------------------------------|
 * | EWDG_SIRC_CLOCK  | Use the slow internal RC clock.                                      |
 * | EWDG_LPO_CLOCK   | Use the low-power oscillator when `FEATURE_EWDG_SUPPORT_LPO_CLOCK=1`.|
 * | EWDG_SXOSC_CLOCK | Use the SXOSC clock path on devices without LPO-clock support.       |
 *
 * Implements : ewdg_clock_source_t_Class
 */
typedef enum
{
    EWDG_SIRC_CLOCK = 0x00U,    /*!< Use the slow internal RC clock. */
#if defined(FEATURE_EWDG_SUPPORT_LPO_CLOCK) && (FEATURE_EWDG_SUPPORT_LPO_CLOCK == 1)
    EWDG_LPO_CLOCK = 0x01U,     /*!< Use the low-power oscillator. */
#else
    EWDG_SXOSC_CLOCK = 0x01U,   /*!< Use the SXOSC clock path. */
#endif
} ewdg_clock_source_t;
#endif

/*!
 * @brief EWDG input-pin assert behavior.
 *
 * Selects whether the external `EWDG_in` pin participates in the watchdog
 * safety path and, when enabled, which logic level is treated as an assert
 * request.
 *
 * | Value                        | Description                                      |
 * |------------------------------|--------------------------------------------------|
 * | EWDG_IN_ASSERT_DISABLED      | Ignore the `EWDG_in` pin.                        |
 * | EWDG_IN_ASSERT_ON_LOGIC_ZERO | Assert the safety output when `EWDG_in` is low.  |
 * | EWDG_IN_ASSERT_ON_LOGIC_ONE  | Assert the safety output when `EWDG_in` is high. |
 *
 * Implements : ewdg_in_assert_logic_t_Class
 */
typedef enum
{
    EWDG_IN_ASSERT_DISABLED = 0x00U,            /*!< Ignore the external input pin. */
    EWDG_IN_ASSERT_ON_LOGIC_ZERO = 0x01U,       /*!< Assert when `EWDG_in` is logic 0. */
    EWDG_IN_ASSERT_ON_LOGIC_ONE = 0x02U         /*!< Assert when `EWDG_in` is logic 1. */
} ewdg_in_assert_logic_t;

/*!
 * @brief EWDG initialization configuration.
 *
 * Holds the complete startup configuration consumed by EWDG_DRV_Init().
 * The structure defines the watchdog clock source (when supported), external
 * input-pin behavior, interrupt enable, prescaler, and the valid refresh
 * window.
 *
 * | Field           | Type                    | Description                                                   | Default From EWDG_DRV_GetDefaultConfig() |
 * |-----------------|-------------------------|---------------------------------------------------------------|------------------------------------------|
 * | clockSource     | ewdg_clock_source_t     | Watchdog counter clock source when the field is available.    | EWDG_SIRC_CLOCK                          |
 * | assertLogic     | ewdg_in_assert_logic_t  | External input-pin assert behavior.                           | EWDG_IN_ASSERT_ON_LOGIC_ZERO             |
 * | interruptEnable | bool                    | Enable watchdog interrupt generation.                         | false                                    |
 * | prescaler       | uint8_t                 | Prescaler field written into `CLK_CTRL`.                      | 255U                                     |
 * | compareLow      | uint16_t                | Earliest counter value at which a refresh is accepted.        | FEATURE_EWDG_CMPL_MIN_VALUE              |
 * | compareHigh     | uint16_t                | Latest counter value at which a refresh is accepted.          | FEATURE_EWDG_CMPH_MAX_VALUE              |
 *
 * @warning `compareHigh` must be greater than `compareLow` and must not exceed
 *          `FEATURE_EWDG_CMPH_MAX_VALUE`.
 *
 * Implements : ewdg_init_config_t_Class
 */
typedef struct
{
#if (defined(EWDG_CLK_CTRL_CLKSEL_MASK))
    ewdg_clock_source_t clockSource;        /*!< Watchdog counter clock source. */
#endif
    ewdg_in_assert_logic_t assertLogic;     /*!< External input-pin assert behavior. */
    bool interruptEnable;                   /*!< Enable watchdog interrupt generation. */
    uint8_t prescaler;                      /*!< Prescaler value written to `CLK_CTRL`. */
    uint16_t compareLow;                    /*!< Lower refresh-window boundary. */
    uint16_t compareHigh;                   /*!< Upper refresh-window boundary. */
} ewdg_init_config_t;

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
 * @brief Functions for preparing or restoring an EWDG instance.
 * @{
 */

/*!
 * @brief Initialize one EWDG instance with the requested watchdog settings.
 *
 * Validates the requested service window, checks that the module is not
 * already enabled, programs the timing registers, and then writes the control
 * register image that enables the watchdog.
 *
 * @param[in] instance  EWDG instance index. Must be less than
 *                      `EWDG_INSTANCE_COUNT`.
 * @param[in] config    Pointer to the startup configuration. Must not be NULL.
 *
 * @return Execution status.
 * @retval STATUS_SUCCESS  The watchdog was configured and enabled successfully.
 * @retval STATUS_ERROR    The instance was already enabled, or the refresh
 *                         window was invalid.
 *
 * @pre  The EWDG peripheral clock and related pins must already be configured.
 * @post The EWDG instance is enabled and begins monitoring the refresh window.
 *
 * @warning This driver expects the EWDG startup registers to be programmed only
 *          once after reset. Re-initializing an active instance returns
 *          `STATUS_ERROR`.
 */
status_t EWDG_DRV_Init(uint32_t instance, const ewdg_init_config_t *config);

/*!
 * @brief Populate an EWDG configuration structure with default values.
 *
 * Writes a permissive service window together with the default prescaler,
 * disabled interrupt generation, and logic-low input assertion.
 *
 * @param[out] config  Pointer to the configuration structure to initialize.
 *
 * @post `config` contains a valid startup configuration for EWDG_DRV_Init().
 */
void EWDG_DRV_GetDefaultConfig(ewdg_init_config_t *config);

/*!
 * @brief Restore one EWDG instance to its reset-like register state.
 *
 * Calls the low-level restore helper while global interrupts are masked so the
 * reset sequence is not interrupted.
 *
 * @param[in] instance  EWDG instance index.
 *
 * @note This function does not configure clocks or pins.
 */
void EWDG_DRV_DeInit(uint32_t instance);

/*! @} */ /* End of Initialization & De-initialization */

/*******************************************************************************
 * Watchdog Service
 ******************************************************************************/
/*!
 * @name Watchdog Service
 * @brief Functions for servicing the active watchdog window.
 * @{
 */

/*!
 * @brief Refresh the EWDG counter inside the programmed service window.
 *
 * Masks global interrupts, writes the required two-byte service sequence, and
 * then restores the global interrupt state.
 *
 * @param[in] instance  EWDG instance index.
 *
 * @warning Call this function only when the EWDG counter is between
 *          `compareLow` and `compareHigh`. Refreshing outside the valid window
 *          may assert the EWDG safety output.
 */
void EWDG_DRV_Refresh(uint32_t instance);

/*! @} */ /* End of Watchdog Service */

/*******************************************************************************
 * Status & Interrupt Control
 ******************************************************************************/
/*!
 * @name Status & Interrupt Control
 * @brief Functions for reading EWDG input behavior and masking interrupts.
 * @{
 */

/*!
 * @brief Read the configured `EWDG_in` assert behavior.
 *
 * Decodes the input-enable and polarity bits from the control register and
 * converts them into the public `ewdg_in_assert_logic_t` enumeration.
 *
 * @param[in] instance  EWDG instance index.
 * @return Configured input-pin assert behavior.
 */
ewdg_in_assert_logic_t EWDG_DRV_GetInputPinAssertLogic(uint32_t instance);

/*!
 * @brief Disable EWDG interrupt generation for one instance.
 *
 * Clears the interrupt-enable bit in the control register image without
 * otherwise changing the watchdog configuration.
 *
 * @param[in] instance  EWDG instance index.
 */
void EWDG_DRV_DisableInterrupt(uint32_t instance);

/*! @} */ /* End of Status & Interrupt Control */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of ewdg group */

#endif /* EWDG_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
