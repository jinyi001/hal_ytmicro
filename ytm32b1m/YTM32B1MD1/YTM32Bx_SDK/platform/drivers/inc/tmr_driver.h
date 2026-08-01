/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file tmr_driver.h
 * @version 1.4.1
 *
 * @brief TMR Driver — Public API for the System Timer Module.
 *
 * This header defines the application-level interface for the TMR peripheral.
 * Each TMR instance provides one shared 32-bit up-counter and four compare
 * channels that can be used to schedule software events or optional compare
 * interrupts on supported devices.
 *
 * The APIs are organized into five categories:
 *   - **Initialization & De-initialization**: Program module-wide counter settings.
 *   - **Channel Compare Control**: Configure and enable compare channels.
 *   - **Counter Control**: Start, stop, seed, and read the shared counter.
 *   - **Time Conversion & Scheduling**: Convert microseconds to ticks and roll compare events forward.
 *   - **Status Flag Management**: Read and clear compare-channel status flags.
 *
 * @note TMR clock gating is not managed by this driver. TMR_DRV_ComputeTicksByUs()
 *       queries the functional clock through the clock_manager using
 *       `TMR_CLOCK_NAMES`.
 */

#ifndef TMR_DRIVER_H
#define TMR_DRIVER_H

#include "device_registers.h"
#include "status.h"

/*!
 * @addtogroup tmr
 * @brief System Timer Module Driver — Public API.
 * @details Provides instance-based APIs for configuring the shared counter,
 *          programming compare channels, converting microseconds to timer
 *          ticks, and servicing channel compare flags.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if defined(FEATURE_TMR_HAS_CLOCK_SELECTION) && (FEATURE_TMR_HAS_CLOCK_SELECTION == 1U)
/*!
 * @brief TMR counter clock source selection.
 *
 * Selects the counter input clock on devices that expose
 * `FEATURE_TMR_HAS_CLOCK_SELECTION`.
 *
 * | Value            | Description                        |
 * |------------------|------------------------------------|
 * | TMR_CLOCK_SYSTEM | Use the internal system clock path.|
 * | TMR_CLOCK_FXOSC  | Use the fast external oscillator.  |
 *
 * Implements : tmr_clock_source_t_Class
 */
typedef enum
{
    TMR_CLOCK_SYSTEM    = 0x0U,  /*!< Use the internal system clock path. */
    TMR_CLOCK_FXOSC     = 0x1U   /*!< Use the fast external oscillator. */
} tmr_clock_source_t;
#endif /* FEATURE_TMR_HAS_CLOCK_SELECTION */

/*!
 * @brief TMR module configuration structure.
 *
 * Holds the module-level settings exposed by the TMR public API. The current
 * driver implementation programs the prescaler, debug-stop behavior, and
 * counter start value. Some devices also expose an optional clock-source
 * field through feature guards.
 *
 * | Field           | Type               | Description                                                     | Default / Source |
 * |-----------------|--------------------|-----------------------------------------------------------------|------------------|
 * | clockSource     | tmr_clock_source_t | Counter clock source when `FEATURE_TMR_HAS_CLOCK_SELECTION=1`. | Caller-defined   |
 * | clockPrescaler  | uint8_t            | Prescaler register value; effective divisor is `value + 1`.    | 0                |
 * | stopInDebugMode | bool               | Freeze the shared counter while the core is halted in debug.   | false            |
 * | startValue      | uint32_t           | Initial value written to the shared counter register (`CNT`).  | 0                |
 *
 * @note TMR_DRV_GetDefaultConfig() fills only the fields that are common to
 *       all devices. If `clockSource` is present in this build, initialize it
 *       explicitly before calling TMR_DRV_Init().
 *
 * Implements : tmr_config_t_Class
 */
typedef struct
{
#if defined(FEATURE_TMR_HAS_CLOCK_SELECTION) && (FEATURE_TMR_HAS_CLOCK_SELECTION == 1U)
    tmr_clock_source_t clockSource;  /*!< Optional counter clock source selection. */
#endif /* FEATURE_TMR_HAS_CLOCK_SELECTION */
    uint8_t clockPrescaler;          /*!< Prescaler register value; the effective divisor is
                                          `clockPrescaler + 1`. */
    bool stopInDebugMode;            /*!< True to stop the counter in debug halt mode. */
    uint32_t startValue;             /*!< Initial value loaded into the shared counter register. */
} tmr_config_t;

/*!
 * @brief TMR compare-channel configuration structure.
 *
 * Describes the settings needed to program one compare channel.
 *
 * | Field           | Type     | Description                                                       | Default / Source |
 * |-----------------|----------|-------------------------------------------------------------------|------------------|
 * | channel         | uint8_t  | Channel index in the range `0 .. TMR_CH_COUNT - 1`.              | Caller-defined   |
 * | compareValue    | uint32_t | Absolute compare tick written to `CH[channel].CMP`.              | Caller-defined   |
 * | interruptEnable | bool     | Enable compare interrupt generation when supported by the device. | Caller-defined   |
 *
 * @note No dedicated default helper is provided for this structure; callers
 *       are expected to populate every field before use.
 *
 * Implements : tmr_channel_config_t_Class
 */
typedef struct
{
    uint8_t channel;        /*!< Target compare channel index. */
    uint32_t compareValue;  /*!< Absolute compare value for the selected channel. */
#if defined(FEATURE_TMR_HAS_INTERRUPT_ENABLE_CONFIG) && (FEATURE_TMR_HAS_INTERRUPT_ENABLE_CONFIG == 1)
    bool interruptEnable;   /*!< True to enable channel interrupt generation. */
#endif
} tmr_channel_config_t;

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
 * @brief Functions for preparing or resetting a TMR instance.
 * @{
 */

/*!
 * @brief Initialize one TMR instance with module-level counter settings.
 *
 * Programs the counter prescaler, debug-stop behavior, and initial counter
 * value stored in `CNT`. This function does not start the counter and does
 * not configure any compare channel.
 *
 * @param[in] instance  TMR instance index in the range `0 .. TMR_INSTANCE_COUNT - 1`.
 * @param[in] config    Pointer to the module configuration structure.
 *
 * @pre  `config` must point to a valid `tmr_config_t` object.
 * @post The shared counter registers are configured and ready to be started
 *       with TMR_DRV_StartTimer().
 *
 * @note Configure individual compare channels with TMR_DRV_InitChannel() or
 *       TMR_DRV_ConfigChannel() after calling this function.
 */
void TMR_DRV_Init(const uint32_t instance, const tmr_config_t *const config);

/*!
 * @brief Reset one TMR instance and all compare channels to their reset state.
 *
 * Stops the shared counter, clears the counter value, clears each channel
 * compare register, and acknowledges all channel status flags.
 *
 * @param[in] instance  TMR instance index in the range `0 .. TMR_INSTANCE_COUNT - 1`.
 *
 * @post All TMR registers are restored to their reset values.
 *
 * @note The TMR clock path itself is not disabled by this function.
 */
void TMR_DRV_Deinit(const uint32_t instance);

/*!
 * @brief Populate a TMR module configuration structure with safe defaults.
 *
 * Writes the common default settings used by the current driver:
 *   - `clockPrescaler = 0U`
 *   - `stopInDebugMode = false`
 *   - `startValue = 0U`
 *
 * @param[out] config  Pointer to the module configuration structure to fill.
 *
 * @note If `FEATURE_TMR_HAS_CLOCK_SELECTION` is enabled in the current build,
 *       initialize `config->clockSource` explicitly after calling this helper.
 */
void TMR_DRV_GetDefaultConfig(tmr_config_t *const config);

/*! @} */ /* End of Initialization & De-initialization */

/*******************************************************************************
 * Channel Compare Control
 ******************************************************************************/
/*!
 * @name Channel Compare Control
 * @brief Functions for configuring and enabling compare channels.
 * @{
 */

/*!
 * @brief Initialize one compare channel from a configuration structure.
 *
 * Writes the compare value for the selected channel, optionally enables the
 * channel interrupt on supported devices, and enables the compare channel.
 *
 * @param[in] instance  TMR instance index in the range `0 .. TMR_INSTANCE_COUNT - 1`.
 * @param[in] config    Pointer to the channel configuration structure.
 *
 * @pre `config->channel` must be in the range `0 .. TMR_CH_COUNT - 1`.
 * @post The selected compare channel is enabled.
 *
 * @note Use this function when the channel interrupt state also needs to be
 *       configured together with the compare value.
 */
void TMR_DRV_InitChannel(const uint32_t instance, const tmr_channel_config_t *const config);

/*!
 * @brief Program a compare value and enable one channel.
 *
 * Writes `compareValue` into the selected channel compare register and enables
 * the compare channel.
 *
 * @param[in] instance      TMR instance index in the range `0 .. TMR_INSTANCE_COUNT - 1`.
 * @param[in] channel       Compare channel index in the range `0 .. TMR_CH_COUNT - 1`.
 * @param[in] compareValue  Absolute compare tick written to `CH[channel].CMP`.
 *
 * @note On devices that support per-channel interrupt enable control,
 *       TMR_DRV_ConfigChannel() overwrites the channel control register.
 *       Use TMR_DRV_InitChannel() when compare interrupt enable state must be
 *       configured at the same time.
 */
void TMR_DRV_ConfigChannel(const uint32_t instance, const uint8_t channel, const uint32_t compareValue);

/*!
 * @brief Enable one compare channel without changing its compare value.
 *
 * @param[in] instance  TMR instance index in the range `0 .. TMR_INSTANCE_COUNT - 1`.
 * @param[in] channel   Compare channel index in the range `0 .. TMR_CH_COUNT - 1`.
 */
void TMR_DRV_EnableChannel(const uint32_t instance, const uint8_t channel);

/*!
 * @brief Disable one compare channel.
 *
 * @param[in] instance  TMR instance index in the range `0 .. TMR_INSTANCE_COUNT - 1`.
 * @param[in] channel   Compare channel index in the range `0 .. TMR_CH_COUNT - 1`.
 *
 * @note Disabling a channel prevents new compare events from being generated
 *       until the channel is enabled again.
 */
void TMR_DRV_DisableChannel(const uint32_t instance, const uint8_t channel);

/*! @} */ /* End of Channel Compare Control */

/*******************************************************************************
 * Counter Control
 ******************************************************************************/
/*!
 * @name Counter Control
 * @brief Functions for managing the shared TMR counter.
 * @{
 */

/*!
 * @brief Write a new start value into the shared counter register.
 *
 * @param[in] instance    TMR instance index in the range `0 .. TMR_INSTANCE_COUNT - 1`.
 * @param[in] startValue  Value written directly to the `CNT` register.
 *
 * @note This helper updates the current counter seed without re-running the
 *       full module initialization sequence.
 */
void TMR_DRV_SetStartValueCount(const uint32_t instance, const uint32_t startValue);

/*!
 * @brief Start the shared TMR counter.
 *
 * @param[in] instance  TMR instance index in the range `0 .. TMR_INSTANCE_COUNT - 1`.
 */
void TMR_DRV_StartTimer(const uint32_t instance);

/*!
 * @brief Stop the shared TMR counter.
 *
 * @param[in] instance  TMR instance index in the range `0 .. TMR_INSTANCE_COUNT - 1`.
 */
void TMR_DRV_StopTimer(const uint32_t instance);

/*!
 * @brief Read the current value of the shared TMR counter.
 *
 * @param[in] instance  TMR instance index in the range `0 .. TMR_INSTANCE_COUNT - 1`.
 * @return Snapshot of the current `CNT` register value.
 */
uint32_t TMR_DRV_GetCounterValue(const uint32_t instance);

/*! @} */ /* End of Counter Control */

/*******************************************************************************
 * Time Conversion & Scheduling
 ******************************************************************************/
/*!
 * @name Time Conversion & Scheduling
 * @brief Functions for converting time units and advancing compare events.
 * @{
 */

/*!
 * @brief Convert a microsecond interval to TMR ticks.
 *
 * Uses the current counter prescaler and the functional clock returned by
 * `clock_manager` for `TMR_CLOCK_NAMES` to convert `periodUs` into a compare
 * value increment.
 *
 * @param[in]  instance  TMR instance index in the range `0 .. TMR_INSTANCE_COUNT - 1`.
 * @param[in]  periodUs  Time interval in microseconds.
 * @param[out] ticks     Pointer that receives the converted tick count.
 * @return Execution status.
 * @retval STATUS_SUCCESS  The converted value fits in the 32-bit compare register.
 * @retval STATUS_ERROR    The converted value exceeds the compare-register range.
 *
 * @pre `ticks` must point to writable storage.
 * @note The conversion reflects the prescaler currently programmed in hardware,
 *       not a cached software copy.
 */
status_t TMR_DRV_ComputeTicksByUs(const uint32_t instance, const uint32_t periodUs, uint32_t *ticks);

/*!
 * @brief Move a compare channel forward by a relative number of ticks.
 *
 * Adds `ticks` to the current compare register value to schedule the next
 * compare event. If the current counter has already passed the newly computed
 * compare value, the function re-bases the event on the current counter value
 * instead.
 *
 * @param[in] instance  TMR instance index in the range `0 .. TMR_INSTANCE_COUNT - 1`.
 * @param[in] channel   Compare channel index in the range `0 .. TMR_CH_COUNT - 1`.
 * @param[in] ticks     Relative tick increment applied to the compare value.
 *
 * @warning Call this helper promptly after servicing the current compare event
 *          when building a periodic schedule; delaying the update can shift the
 *          next deadline forward.
 */
void TMR_DRV_IncrementTicks(const uint32_t instance, const uint8_t channel, const uint32_t ticks);

/*! @} */ /* End of Time Conversion & Scheduling */

/*******************************************************************************
 * Status Flag Management
 ******************************************************************************/
/*!
 * @name Status Flag Management
 * @brief Functions for reading and clearing channel compare status flags.
 * @{
 */

/*!
 * @brief Read the raw status register of one compare channel.
 *
 * @param[in] instance  TMR instance index in the range `0 .. TMR_INSTANCE_COUNT - 1`.
 * @param[in] channel   Compare channel index in the range `0 .. TMR_CH_COUNT - 1`.
 * @return Raw value of the channel interrupt/status register.
 *
 * @note A non-zero return value indicates that the channel compare flag is set.
 */
uint32_t TMR_DRV_GetStatusFlags(const uint32_t instance, const uint8_t channel);

/*!
 * @brief Clear the compare status flag of one channel.
 *
 * @param[in] instance  TMR instance index in the range `0 .. TMR_INSTANCE_COUNT - 1`.
 * @param[in] channel   Compare channel index in the range `0 .. TMR_CH_COUNT - 1`.
 *
 * @note The hardware uses write-one-to-clear semantics for the compare flag.
 */
void TMR_DRV_ClearStatusFlags(const uint32_t instance, const uint8_t channel);

/*! @} */ /* End of Status Flag Management */

#if defined(__cplusplus)
}
#endif
/*! @} */

#endif /* TMR_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
