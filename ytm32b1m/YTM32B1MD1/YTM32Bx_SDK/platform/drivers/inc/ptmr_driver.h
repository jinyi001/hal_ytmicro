/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file ptmr_driver.h
 * @version 1.4.1
 *
 * @brief pTMR Driver — Public API for Low Power Periodic Interrupt Timer.
 *
 * This header defines the application-level interface for the pTMR peripheral.
 * It wraps the low-level hardware access layer and provides a convenient API
 * for configuring timer channels and generating periodic interrupts.
 *
 * The APIs are organized into four categories:
 *   - **Initialization & De-initialization**: Set up or tear down the pTMR module and channels.
 *   - **Timer Start & Stop**: Control timer channel counting.
 *   - **Timer Period**: Configure and query timer period in count or microsecond units.
 *   - **Interrupt Management**: Enable, disable, query, and clear channel interrupts.
 *
 * @note The pTMR peripheral clock must be enabled (via clock_manager) before
 *       calling any function in this module.
 */

#ifndef PTMR_DRIVER_H
#define PTMR_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "status.h"
#include "device_registers.h"

/*!
 * @addtogroup ptmr
 * @brief Low Power Periodic Interrupt Timer Peripheral Driver — Public API.
 * @details Provides instance-based APIs for pTMR module initialization, timer
 *          channel configuration, period management, and interrupt control.
 *          Supports up to 4 independent timer channels with optional channel
 *          chaining for extended timing periods.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Maximum timer period in raw count units (32-bit counter). */
#define MAX_PERIOD_COUNT                    (0xFFFFFFFFU)

/*!
 * @brief Timer period unit selection.
 *
 * Determines how the period value in ptmr_user_channel_config_t is interpreted:
 *
 * | Value                          | Unit         |
 * |--------------------------------|--------------|
 * | pTMR_PERIOD_UNITS_COUNTS       | Raw counts   |
 * | pTMR_PERIOD_UNITS_MICROSECONDS | Microseconds |
 *
 * Implements : ptmr_period_units_t_Class
 */
typedef enum
{
    pTMR_PERIOD_UNITS_COUNTS = 0x00U, /*!< Period value unit is count */
    pTMR_PERIOD_UNITS_MICROSECONDS = 0x01U  /*!< Period value unit is microsecond */
} ptmr_period_units_t;

/*!
 * @brief pTMR module configuration structure.
 *
 * Holds module-level settings that apply to all timer channels in the pTMR
 * instance. Pass this structure to pTMR_DRV_Init().
 *
 * | Field                  | Type  | Description                                        | Default |
 * |------------------------|-------|----------------------------------------------------|---------|
 * | enableRunInDebug       | bool  | Continue running when processor enters debug mode.  | false   |
 * | useFunctionClockSource | bool  | Use IPC function clock instead of PCLK (conditional).| N/A    |
 *
 * Implements : ptmr_user_config_t_Class
 */
typedef struct
{
    bool enableRunInDebug; /*!< True: Timer channels continue to run in debug mode
                                False: Timer channels stop in debug mode            */
#if (defined(FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE) && (FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE == 1))
    bool useFunctionClockSource;          /*!< pTMR use function clock from IPC module */
#endif /* FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE */
} ptmr_user_config_t;

/*!
 * @brief pTMR timer channel configuration structure.
 *
 * Holds per-channel settings for individual timer channel initialization.
 * Pass this structure to pTMR_DRV_InitChannel().
 *
 * | Field              | Type                 | Description                                  | Default             |
 * |--------------------|----------------------|----------------------------------------------|---------------------|
 * | periodUnits        | ptmr_period_units_t  | Unit of the period field (counts or µs).     | MICROSECONDS        |
 * | period             | uint32_t             | Timer period in the selected unit.           | 1000000 (1 second)  |
 * | chainChannel       | bool                 | Chain this channel to the previous channel.  | false               |
 * | isInterruptEnabled | bool                 | Enable interrupt on timer timeout.           | true                |
 *
 * Implements : ptmr_user_channel_config_t_Class
 */
typedef struct
{
    ptmr_period_units_t periodUnits;     /*!< Timer period value units                                      */
    uint32_t period;                     /*!< Period of timer channel                                       */
    bool chainChannel;                   /*!< Channel chaining enable                                       */
    bool isInterruptEnabled;             /*!< Timer channel interrupt generation enable                     */
} ptmr_user_channel_config_t;

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
 * @brief Functions for setting up and tearing down the pTMR module and
 *        individual timer channels.
 * @{
 */

/*!
 * @brief Populate a module configuration structure with default values.
 *
 * Fills @a config with safe default settings:
 *   - enableRunInDebug: false (timers freeze in debug mode)
 *
 * @param[out] config  Pointer to the module configuration structure. Must not
 *                     be NULL.
 *
 * @note Call this function to obtain a baseline configuration, then modify
 *       individual fields as needed before passing to pTMR_DRV_Init().
 */
void pTMR_DRV_GetDefaultConfig(ptmr_user_config_t *const config);

/*!
 * @brief Populate a channel configuration structure with default values.
 *
 * Fills @a config with safe default settings:
 *   - periodUnits: pTMR_PERIOD_UNITS_MICROSECONDS
 *   - period: 1000000 µs (1 second)
 *   - chainChannel: false
 *   - isInterruptEnabled: true
 *
 * @param[out] config  Pointer to the channel configuration structure. Must not
 *                     be NULL.
 */
void pTMR_DRV_GetDefaultChanConfig(ptmr_user_channel_config_t *const config);

/*!
 * @brief Initialize the pTMR module.
 *
 * Resets all pTMR registers, enables the module functional clock, and
 * configures module-level settings (debug mode behavior, clock source).
 * This function must be called before using any other pTMR driver API.
 *
 * @param[in] instance    pTMR instance index (0-based). Must be less than
 *                        pTMR_INSTANCE_COUNT.
 * @param[in] userConfig  Pointer to the module configuration structure.
 *                        Must not be NULL.
 *
 * @pre  The pTMR peripheral clock must be enabled via the clock_manager.
 * @post The pTMR module is enabled and ready for channel initialization.
 */
void pTMR_DRV_Init(uint32_t instance,
                   const ptmr_user_config_t *userConfig);

/*!
 * @brief De-initialize the pTMR module.
 *
 * Stops all timer channels, disables their interrupts, then disables the
 * pTMR module functional clock and resets all registers. To use the pTMR
 * module again, pTMR_DRV_Init() must be called.
 *
 * @param[in] instance  pTMR instance index (0-based).
 *
 * @note This function does NOT disable the system clock gating. The caller
 *       is responsible for clock management after de-initialization.
 */
void pTMR_DRV_Deinit(uint32_t instance);

/*!
 * @brief Initialize an individual pTMR timer channel.
 *
 * Configures the specified channel with the given settings: channel chaining,
 * timer period, and interrupt generation. The channel does NOT start counting
 * after this call — use pTMR_DRV_StartTimerChannels() to begin counting.
 *
 * @param[in] instance          pTMR instance index (0-based).
 * @param[in] channel           Timer channel index (0–3).
 * @param[in] userChannelConfig Pointer to the channel configuration structure.
 *                              Must not be NULL.
 * @return Operation status.
 * @retval STATUS_SUCCESS  Channel initialized successfully.
 * @retval STATUS_ERROR    The requested period exceeds the valid range for
 *                         the current clock frequency.
 *
 * @pre  pTMR_DRV_Init() must have been called for this instance.
 * @post The channel is configured and ready to start counting.
 *
 * @note To reconfigure the period at runtime, use
 *       pTMR_DRV_SetTimerPeriodByUs() or pTMR_DRV_SetTimerPeriodByCount().
 */
status_t pTMR_DRV_InitChannel(uint32_t instance,
                              uint32_t channel,
                              const ptmr_user_channel_config_t *userChannelConfig);

/*! @} */ /* End of Initialization & De-initialization */

/*******************************************************************************
 * Timer Start & Stop
 ******************************************************************************/
/*!
 * @name Timer Start & Stop
 * @brief Functions for starting and stopping timer channel counting.
 * @{
 */

/*!
 * @brief Start the specified timer channel counting.
 *
 * Enables counting for the given channel. The channel begins counting down
 * from its configured period value according to its mode and control
 * settings.
 *
 * @param[in] instance  pTMR instance index (0-based).
 * @param[in] channel   Timer channel index (0–3).
 *
 * @pre pTMR_DRV_InitChannel() must have been called for this channel.
 */
void pTMR_DRV_StartTimerChannels(uint32_t instance,
                                 uint32_t channel);

/*!
 * @brief Stop the specified timer channel counting.
 *
 * Disables counting for the given channel. The channel reloads its period
 * value when subsequently restarted via pTMR_DRV_StartTimerChannels().
 *
 * @param[in] instance  pTMR instance index (0-based).
 * @param[in] channel   Timer channel index (0–3).
 */
void pTMR_DRV_StopTimerChannels(uint32_t instance,
                                uint32_t channel);

/*! @} */ /* End of Timer Start & Stop */

/*******************************************************************************
 * Timer Period
 ******************************************************************************/
/*!
 * @name Timer Period
 * @brief Functions for configuring and querying the timer channel period
 *        and current counter value.
 *
 * Period can be expressed in two units:
 * - **Microseconds**: the driver converts to raw counts using the current
 *   clock frequency. Suitable for time-based applications.
 * - **Raw counts**: direct register value. Suitable for precise hardware
 *   control.
 * @{
 */

/*!
 * @brief Set the timer channel period in microseconds.
 *
 * Converts the given period in microseconds to raw timer counts using the
 * current pTMR functional clock frequency, then writes the value to the
 * hardware. The valid range depends on the clock frequency.
 *
 * @param[in] instance  pTMR instance index (0-based).
 * @param[in] channel   Timer channel index (0–3).
 * @param[in] periodUs  Desired period in microseconds.
 * @return Operation status.
 * @retval STATUS_SUCCESS  Period is valid and has been applied.
 * @retval STATUS_ERROR    Period exceeds the maximum representable count
 *                         for the current clock frequency.
 *
 * @note For a running channel, the new period takes effect after the
 *       current period expires. To apply immediately, stop and restart
 *       the channel.
 */
status_t pTMR_DRV_SetTimerPeriodByUs(uint32_t instance,
                                     uint32_t channel,
                                     uint32_t periodUs);

/*!
 * @brief Set the timer channel period in raw count units.
 *
 * Writes the period value directly to the Timer Start Value Register (TSV).
 * For a running channel, the new value takes effect after the current
 * period expires.
 *
 * @param[in] instance  pTMR instance index (0-based).
 * @param[in] channel   Timer channel index (0–3).
 * @param[in] count     Period value in raw timer counts.
 *
 * @note To apply the new period immediately, disable and re-enable the channel.
 */
void pTMR_DRV_SetTimerPeriodByCount(uint32_t instance,
                                    uint32_t channel,
                                    uint32_t count);

/*!
 * @brief Get the timer channel period in microseconds.
 *
 * Reads the current period from hardware and converts it to microseconds
 * using the current pTMR functional clock frequency.
 *
 * @param[in] instance  pTMR instance index (0-based).
 * @param[in] channel   Timer channel index (0–3).
 * @return Timer channel period in microseconds.
 */
uint64_t pTMR_DRV_GetTimerPeriodByUs(uint32_t instance,
                                     uint32_t channel);

/*!
 * @brief Get the timer channel period in raw count units.
 *
 * Reads the current period value from the Timer Start Value Register (TSV).
 *
 * @param[in] instance  pTMR instance index (0-based).
 * @param[in] channel   Timer channel index (0–3).
 * @return Timer channel period in raw count units.
 */
uint32_t pTMR_DRV_GetTimerPeriodByCount(uint32_t instance,
                                        uint32_t channel);

/*!
 * @brief Get the current timer channel counter value in microseconds.
 *
 * Reads the real-time counter and converts it to microseconds. Useful for
 * measuring elapsed time — call this function at two points and compute
 * the difference.
 *
 * @param[in] instance  pTMR instance index (0-based).
 * @param[in] channel   Timer channel index (0–3).
 * @return Current counter value in microseconds.
 *
 * @warning Ensure the measurement interval does not exceed the timer period
 *          to avoid counter wrap-around.
 */
uint64_t pTMR_DRV_GetCurrentTimerUs(uint32_t instance,
                                    uint32_t channel);

/*!
 * @brief Get the current timer channel counter value in raw counts.
 *
 * Reads the real-time counter value from the Timer Current Value Register
 * (TCV). The value ranges from the period start value down to 0.
 *
 * @param[in] instance  pTMR instance index (0-based).
 * @param[in] channel   Timer channel index (0–3).
 * @return Current counter value in raw counts.
 */
uint32_t pTMR_DRV_GetCurrentTimerCount(uint32_t instance,
                                       uint32_t channel);

/*! @} */ /* End of Timer Period */

/*******************************************************************************
 * Interrupt Management
 ******************************************************************************/
/*!
 * @name Interrupt Management
 * @brief Functions for enabling, disabling, querying, and clearing timer
 *        channel interrupt flags.
 * @{
 */

/*!
 * @brief Enable interrupt generation for the specified timer channel.
 *
 * Enables the timer channel to generate an interrupt request when its
 * counter reaches zero (timeout event).
 *
 * @param[in] instance  pTMR instance index (0-based).
 * @param[in] channel   Timer channel index (0–3).
 */
void pTMR_DRV_EnableTimerChannelInterrupt(uint32_t instance,
                                          uint32_t channel);

/*!
 * @brief Disable interrupt generation for the specified timer channel.
 *
 * Prevents the timer channel from generating interrupt requests on timeout.
 *
 * @param[in] instance  pTMR instance index (0-based).
 * @param[in] channel   Timer channel index (0–3).
 */
void pTMR_DRV_DisableTimerChannelInterrupt(uint32_t instance,
                                           uint32_t channel);

/*!
 * @brief Get the interrupt flag status of the specified timer channel.
 *
 * Reads the Timer Interrupt Flag (TIF) for the given channel. The flag is
 * set when the timer counter reaches zero.
 *
 * @param[in] instance  pTMR instance index (0-based).
 * @param[in] channel   Timer channel index (0–3).
 * @return Interrupt flag status:
 *         - Non-zero: Timer has timed out (interrupt pending).
 *         - 0: No timeout event has occurred.
 */
uint32_t pTMR_DRV_GetInterruptFlagTimerChannels(uint32_t instance,
                                                uint32_t channel);

/*!
 * @brief Clear the interrupt flag of the specified timer channel.
 *
 * Clears the Timer Interrupt Flag (TIF) for the given channel. This must
 * be called in the interrupt handler to acknowledge the timeout event.
 *
 * @param[in] instance  pTMR instance index (0-based).
 * @param[in] channel   Timer channel index (0–3).
 */
void pTMR_DRV_ClearInterruptFlagTimerChannels(uint32_t instance,
                                              uint32_t channel);

/*! @} */ /* End of Interrupt Management */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of ptmr group */

#endif /* PTMR_DRIVER_H*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
