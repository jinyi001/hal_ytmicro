/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file rtc_driver.h
 * @version 1.4.1
 *
 * @brief RTC Driver — Public API for calendar timekeeping, alarms, and RTC interrupts.
 *
 * This header defines the application-level interface for the Real Time Clock
 * peripheral. The driver provides services for:
 *   - Initializing the RTC instance and applying runtime configuration.
 *   - Starting and stopping the seconds counter.
 *   - Reading or updating calendar time in @ref rtc_timedate_t format.
 *   - Configuring repeating alarms, overflow callbacks, and periodic second interrupts.
 *   - Converting between calendar time and RTC seconds counts.
 *
 * @note The RTC clock source and board-level pin routing must be configured by
 *       the application before calling this API.
 */

#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

#include <stddef.h>
#include "status.h"
#include "interrupt_manager.h"

/*!
 * @addtogroup rtc
 * @brief Real Time Clock peripheral driver — public API.
 * @details Provides instance-based APIs for RTC initialization, timekeeping,
 *          alarm scheduling, clock-output selection, time compensation, and
 *          interrupt callback registration.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SECONDS_IN_A_DAY     (86400UL)
#define SECONDS_IN_A_HOUR    (3600U)
#define SECONDS_IN_A_MIN     (60U)
#define MINS_IN_A_HOUR       (60U)
#define HOURS_IN_A_DAY       (24U)
#define DAYS_IN_A_YEAR       (365U)
#define DAYS_IN_A_LEAP_YEAR  (366U)
#define YEAR_RANGE_START     (1970U)
#define YEAR_RANGE_END       (2107U)

/*!
 * @brief RTC interrupt sources managed by the driver helper functions.
 *
 * Selects which hardware interrupt enable bit or status flag is affected by
 * RTC_DRV_SetRtcInterrupt(), RTC_DRV_ClearRtcInterrupt(), and
 * RTC_DRV_ClearRtcInterruptFlag().
 *
 * | Value                  | Description                          |
 * |------------------------|--------------------------------------|
 * | RTC_OVERFLOW_INTERRUPT | Time-counter overflow interrupt.     |
 * | RTC_ALARM_INTERRUPT    | Alarm compare interrupt.             |
 * | RTC_SECONDS_INTERRUPT  | Periodic seconds/square-wave event.  |
 */
typedef enum
{
    RTC_OVERFLOW_INTERRUPT = 0U,
    RTC_ALARM_INTERRUPT = 1U,
    RTC_SECONDS_INTERRUPT = 2U,
} rtc_interrupt_mode_t;

/*!
 * @brief Periodic RTC seconds interrupt divider configuration.
 *
 * Defines the square-wave / seconds interrupt rate produced by the RTC when
 * the periodic seconds interrupt is enabled.
 *
 * | Value         | Interrupt Rate |
 * |---------------|----------------|
 * | RTC_INT_1HZ   | 1 Hz           |
 * | RTC_INT_2HZ   | 2 Hz           |
 * | RTC_INT_4HZ   | 4 Hz           |
 * | RTC_INT_8HZ   | 8 Hz           |
 * | RTC_INT_16HZ  | 16 Hz          |
 * | RTC_INT_32HZ  | 32 Hz          |
 * | RTC_INT_64HZ  | 64 Hz          |
 * | RTC_INT_128HZ | 128 Hz         |
 */
typedef enum
{
    RTC_INT_1HZ = 0x00U,      /*!< Generate a 1 Hz periodic event. */
    RTC_INT_2HZ = 0x01U,      /*!< Generate a 2 Hz periodic event. */
    RTC_INT_4HZ = 0x02U,      /*!< Generate a 4 Hz periodic event. */
    RTC_INT_8HZ = 0x03U,      /*!< Generate an 8 Hz periodic event. */
    RTC_INT_16HZ = 0x04U,     /*!< Generate a 16 Hz periodic event. */
    RTC_INT_32HZ = 0x05U,     /*!< Generate a 32 Hz periodic event. */
    RTC_INT_64HZ = 0x06U,     /*!< Generate a 64 Hz periodic event. */
    RTC_INT_128HZ = 0x07U     /*!< Generate a 128 Hz periodic event. */
} rtc_second_int_cfg_t;

/*!
 * @brief RTC clock-output pin source selection.
 *
 * Controls whether the RTC clock-output pin is disabled, driven by the
 * configured seconds interrupt waveform, or driven by the selected RTC clock.
 *
 * | Value                 | Description                                              |
 * |-----------------------|----------------------------------------------------------|
 * | RTC_CLKOUT_DISABLED   | Disable the RTC clock-output pin.                        |
 * | RTC_CLKOUT_SRC_SEC    | Output the periodic seconds interrupt waveform.          |
 * | RTC_CLKOUT_SRC_CLKSEL | Output the RTC clock selected by @ref rtc_clk_source_t.  |
 */
typedef enum
{
    RTC_CLKOUT_DISABLED = 0x00U,  /*!< Disable the RTC clock-output pin. */
    RTC_CLKOUT_SRC_SEC = 0x01U,   /*!< Output the configured periodic seconds waveform. */
    RTC_CLKOUT_SRC_CLKSEL = 0x02U /*!< Output the RTC clock source selected in CTRL.CLKSEL. */
} rtc_clk_out_config_t;

/*!
 * @brief RTC date and time value in Gregorian calendar form.
 *
 * Used for current-time snapshots, alarm targets, and conversion helpers.
 * Valid values are checked by RTC_DRV_IsTimeDateCorrectFormat().
 *
 * | Field   | Type     | Description                                 |
 * |---------|----------|---------------------------------------------|
 * | year    | uint16_t | Calendar year in the range 1970 to 2107.    |
 * | month   | uint16_t | Calendar month in the range 1 to 12.        |
 * | day     | uint16_t | Day of month, validated against month/year. |
 * | hour    | uint16_t | Hour in the range 0 to 23.                  |
 * | minutes | uint16_t | Minute in the range 0 to 59.                |
 * | seconds | uint8_t  | Second in the range 0 to 59.                |
 */
typedef struct
{
    uint16_t year;      /*!< Calendar year. */
    uint16_t month;     /*!< Calendar month. */
    uint16_t day;       /*!< Day of month. */
    uint16_t hour;      /*!< Hour of day. */
    uint16_t minutes;   /*!< Minute of hour. */
    uint8_t seconds;    /*!< Second of minute. */
} rtc_timedate_t;

/*!
 * @brief Alarm configuration passed to RTC_DRV_ConfigureAlarmInt().
 *
 * Describes the target alarm time, optional repetition behavior, and callback
 * information that should be used when the alarm interrupt fires.
 *
 * | Field              | Type                         | Description                                             |
 * |--------------------|------------------------------|---------------------------------------------------------|
 * | alarmTime          | rtc_timedate_t               | Absolute alarm time in calendar form.                   |
 * | repetitionInterval | uint32_t                     | Interval, in seconds, between repeated alarms.          |
 * | numberOfRepeats    | uint32_t                     | Remaining repeat count when repeatForever is false.     |
 * | repeatForever      | bool                         | Keep rescheduling the alarm indefinitely.               |
 * | alarmIntEnable     | bool                         | Enable the hardware alarm interrupt source.             |
 * | rtcAlarmCallback   | void (*)(void *)             | User callback invoked from the alarm ISR path.          |
 * | callbackParams     | void *                       | User context passed back to rtcAlarmCallback.           |
 */
typedef struct
{
    rtc_timedate_t alarmTime;                 /*!< Absolute alarm target in calendar form. */
    uint32_t repetitionInterval;              /*!< Repeat interval, in seconds, for recurring alarms. */
    uint32_t numberOfRepeats;                 /*!< Remaining repeat count for finite recurring alarms. */
    bool repeatForever;                       /*!< Continue rescheduling the alarm indefinitely. */
    bool alarmIntEnable;                      /*!< Enable the alarm interrupt source in hardware. */
    void (*rtcAlarmCallback)(void *callbackParam); /*!< Callback invoked when the alarm interrupt fires. */
    void *callbackParams;                     /*!< User context passed to rtcAlarmCallback. */
} rtc_alarm_config_t;

/*!
 * @brief Overflow interrupt callback configuration.
 *
 * Used by RTC_DRV_ConfigureOverflowInt() to register a callback for RTC
 * overflow events.
 *
 * | Field               | Type             | Description                                   |
 * |---------------------|------------------|-----------------------------------------------|
 * | overflowIntEnable   | bool             | Enable the overflow interrupt source.         |
 * | rtcOverflowCallback | void (*)(void *) | Callback invoked for overflow interrupt work. |
 * | callbackParams      | void *           | User context passed to rtcOverflowCallback.   |
 */
typedef struct
{
    bool overflowIntEnable;                        /*!< Enable the RTC overflow interrupt source. */
    void (*rtcOverflowCallback)(void *callbackParam); /*!< Callback invoked when an overflow occurs. */
    void *callbackParams;                          /*!< User context passed to rtcOverflowCallback. */
} rtc_overflow_config_t;

/*!
 * @brief Periodic seconds interrupt configuration.
 *
 * Used by RTC_DRV_ConfigureSecondsInt() to select the event rate, enable
 * state, and callback used by the periodic seconds interrupt path.
 *
 * | Field              | Type                   | Description                                   |
 * |--------------------|------------------------|-----------------------------------------------|
 * | secondsIntConfig   | rtc_second_int_cfg_t   | Periodic interrupt frequency selection.       |
 * | secondsIntEnable   | bool                   | Enable the seconds interrupt source.          |
 * | rtcSecondsCallback | void (*)(void *)       | Callback invoked from the seconds ISR path.   |
 * | callbackParams     | void *                 | User context passed to rtcSecondsCallback.    |
 */
typedef struct
{
    rtc_second_int_cfg_t secondsIntConfig;         /*!< Periodic seconds interrupt frequency. */
    bool secondsIntEnable;                         /*!< Enable the periodic seconds interrupt source. */
    void (*rtcSecondsCallback)(void *callbackParam); /*!< Callback invoked when the seconds interrupt fires. */
    void *callbackParams;                          /*!< User context passed to rtcSecondsCallback. */
} rtc_seconds_config_t;

/*!
 * @brief RTC initialization parameters.
 *
 * Collects the RTC core configuration together with optional interrupt
 * configuration structures. The pointed-to interrupt configuration objects are
 * stored by the driver and reused at runtime.
 *
 * | Field                | Type                    | Description                                                  |
 * |----------------------|-------------------------|--------------------------------------------------------------|
 * | compensationInterval | uint8_t                 | Compensation interval written to the RTC CMP register.       |
 * | compensation         | int8_t                  | Compensation value applied to the prescaler.                 |
 * | clockSource          | rtc_clk_source_t        | RTC clock source selection from the device features header.  |
 * | clockOutConfig       | rtc_clk_out_config_t    | RTC clock-output pin source selection.                       |
 * | debugEnable          | bool                    | Allow RTC operation control while debugging.                 |
 * | rtcAlarmConfig       | rtc_alarm_config_t *    | Optional alarm configuration stored by the driver.           |
 * | rtcOverflowConfig    | rtc_overflow_config_t * | Optional overflow interrupt configuration.                   |
 * | rtcSecondsConfig     | rtc_seconds_config_t *  | Optional periodic seconds interrupt configuration.           |
 */
typedef struct
{
    uint8_t compensationInterval;              /*!< Compensation interval programmed into the RTC. */
    int8_t compensation;                       /*!< Compensation value applied to the RTC prescaler. */
    rtc_clk_source_t clockSource;              /*!< RTC clock source selection. */
    rtc_clk_out_config_t clockOutConfig;       /*!< RTC clock-output pin source selection. */
    bool debugEnable;                          /*!< Keep RTC control accessible while the debugger is active. */
    rtc_alarm_config_t *rtcAlarmConfig;        /*!< Optional alarm configuration retained by the driver. */
    rtc_overflow_config_t *rtcOverflowConfig;  /*!< Optional overflow interrupt configuration retained by the driver. */
    rtc_seconds_config_t *rtcSecondsConfig;    /*!< Optional seconds interrupt configuration retained by the driver. */
} rtc_init_config_t;

/*!
 * @brief RTC register-lock state snapshot.
 *
 * Mirrors the software-visible lock state of the RTC lock-protected register
 * groups.
 *
 * | Field                        | Type | Description                                   |
 * |------------------------------|------|-----------------------------------------------|
 * | lockRegisterLock             | bool | Lock state of the lock-control register.      |
 * | statusRegisterLock           | bool | Lock state of the status / enable register.   |
 * | controlRegisterLock          | bool | Lock state of the control register.           |
 * | timeCompensationRegisterLock | bool | Lock state of the compensation register.      |
 */
typedef struct
{
    bool lockRegisterLock;              /*!< Lock state of the lock-control register. */
    bool statusRegisterLock;            /*!< Lock state of the status / enable register. */
    bool controlRegisterLock;           /*!< Lock state of the control register. */
    bool timeCompensationRegisterLock;  /*!< Lock state of the compensation register. */
} rtc_register_lock_config_t;

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
 * @brief Functions for preparing or resetting an RTC instance.
 * @{
 */

/*!
 * @brief Initialize the RTC instance and apply the supplied runtime configuration.
 *
 * Stores the optional callback configuration pointers in the driver runtime
 * state, resets and reconfigures the RTC hardware when the counter is not
 * running, and applies any optional overflow/alarm/seconds interrupt setup.
 *
 * @param[in] instance    RTC instance index (0-based).
 * @param[in] rtcUserCfg  Pointer to the initialization structure. Must not be NULL.
 * @return Execution status.
 * @retval STATUS_SUCCESS Initialization completed.
 *
 * @pre The RTC module clock source and board-level routing must already be configured.
 * @note If the RTC counter is already enabled, the hardware reset/configuration
 *       sequence is skipped and only the runtime callback pointers plus optional
 *       interrupt configuration are refreshed.
 */
status_t RTC_DRV_Init(uint32_t instance, const rtc_init_config_t *rtcUserCfg);

/*!
 * @brief De-initialize the RTC instance with a software reset.
 *
 * Disables the RTC-related NVIC lines for the selected instance, performs a
 * software reset, and clears the pending generic RTC interrupt when the
 * counter is running.
 *
 * @param[in] instance  RTC instance index (0-based).
 * @return Execution status.
 * @retval STATUS_SUCCESS De-initialization completed.
 * @retval STATUS_ERROR   The counter was already disabled, so no reset was performed.
 */
status_t RTC_DRV_Deinit(uint32_t instance);

/*!
 * @brief Populate an initialization structure with driver defaults.
 *
 * Initializes the core RTC settings used by RTC_DRV_Init():
 *   - clockSource: first clock source enum value
 *   - clockOutConfig: RTC_CLKOUT_DISABLED
 *   - debugEnable: true
 *   - compensation: 0
 *   - compensationInterval: 0
 *
 * @param[out] config  Pointer to the structure to initialize. Must not be NULL.
 *
 * @note The optional pointer fields (`rtcAlarmConfig`, `rtcOverflowConfig`,
 *       and `rtcSecondsConfig`) are not modified by this function and should
 *       be initialized by the caller.
 */
void RTC_DRV_GetDefaultConfig(rtc_init_config_t *config);

/*! @} */ /* End of Initialization & De-initialization */

/*******************************************************************************
 * Interrupt Source Control
 ******************************************************************************/
/*!
 * @name Interrupt Source Control
 * @brief Helpers for enabling, disabling, and clearing RTC interrupt sources.
 * @{
 */

/*!
 * @brief Enable one RTC interrupt source in hardware.
 *
 * @param[in] instance  RTC instance index (0-based).
 * @param[in] mode      Interrupt source to enable.
 */
void RTC_DRV_SetRtcInterrupt(uint32_t instance, rtc_interrupt_mode_t mode);

/*!
 * @brief Disable one RTC interrupt source in hardware.
 *
 * @param[in] instance  RTC instance index (0-based).
 * @param[in] mode      Interrupt source to disable.
 */
void RTC_DRV_ClearRtcInterrupt(uint32_t instance, rtc_interrupt_mode_t mode);

/*!
 * @brief Clear the pending flag for one RTC interrupt source.
 *
 * @param[in] instance  RTC instance index (0-based).
 * @param[in] mode      Interrupt source whose flag should be cleared.
 */
void RTC_DRV_ClearRtcInterruptFlag(uint32_t instance, rtc_interrupt_mode_t mode);

/*! @} */ /* End of Interrupt Source Control */

/*******************************************************************************
 * Counter Control
 ******************************************************************************/
/*!
 * @name Counter Control
 * @brief Functions for starting and stopping the RTC seconds counter.
 * @{
 */

/*!
 * @brief Start the RTC counter.
 *
 * @param[in] instance  RTC instance index (0-based).
 * @return Execution status.
 * @retval STATUS_SUCCESS Counter enabled successfully.
 * @retval STATUS_ERROR   Counter was already enabled.
 *
 * @pre Configure the initial time with RTC_DRV_SetTimeDate() before the first start.
 */
status_t RTC_DRV_StartCounter(uint32_t instance);

/*!
 * @brief Stop the RTC counter.
 *
 * @param[in] instance  RTC instance index (0-based).
 * @return Execution status.
 * @retval STATUS_SUCCESS Counter disabled successfully.
 * @retval STATUS_ERROR   Counter remained enabled after the stop request.
 */
status_t RTC_DRV_StopCounter(uint32_t instance);

/*! @} */ /* End of Counter Control */

/*******************************************************************************
 * Calendar Time Access
 ******************************************************************************/
/*!
 * @name Calendar Time Access
 * @brief Functions for reading or updating RTC time in calendar format.
 * @{
 */

/*!
 * @brief Read the current RTC time and convert it to calendar format.
 *
 * Reads the seconds register twice to avoid capturing a value while the
 * hardware counter is rolling over, then converts the stable seconds value to
 * @ref rtc_timedate_t.
 *
 * @param[in]  instance     RTC instance index (0-based).
 * @param[out] currentTime  Pointer to the structure that receives the current time.
 * @return Execution status.
 * @retval STATUS_SUCCESS Current time was read successfully.
 * @retval STATUS_ERROR   A stable seconds snapshot could not be obtained.
 */
status_t RTC_DRV_GetCurrentTimeDate(uint32_t instance, rtc_timedate_t *const currentTime);

/*!
 * @brief Program the RTC time using a calendar date/time structure.
 *
 * Validates @a timeDate, converts it to the RTC seconds representation, and
 * writes the result to the seconds register.
 *
 * @param[in] instance   RTC instance index (0-based).
 * @param[in] timeDate   Pointer to the desired calendar time. Must not be NULL.
 * @return Execution status.
 * @retval STATUS_SUCCESS Time value written successfully.
 * @retval STATUS_ERROR   The input time was invalid or the counter was still running.
 *
 * @pre Stop the RTC counter with RTC_DRV_StopCounter() before calling this API.
 */
status_t RTC_DRV_SetTimeDate(uint32_t instance, const rtc_timedate_t *timeDate);

/*! @} */ /* End of Calendar Time Access */

/*******************************************************************************
 * Register Protection
 ******************************************************************************/
/*!
 * @name Register Protection
 * @brief Functions related to RTC register lock handling.
 * @{
 */

/*!
 * @brief Lock the RTC register interface for the selected instance.
 *
 * @param[in] instance  RTC instance index (0-based).
 * @return Execution status reported by the implementation.
 */
status_t RTC_DRV_LockRegisters(uint32_t instance);

/*! @} */ /* End of Register Protection */

/*******************************************************************************
 * Time Compensation
 ******************************************************************************/
/*!
 * @name Time Compensation
 * @brief Functions for configuring or reading RTC compensation coefficients.
 * @{
 */

/*!
 * @brief Program the RTC compensation interval and compensation value.
 *
 * @param[in] instance      RTC instance index (0-based).
 * @param[in] compInterval  Compensation interval value written to the RTC CMP register.
 * @param[in] compensation  Compensation value written to the RTC CMP register.
 * @return Execution status.
 * @retval STATUS_SUCCESS Compensation parameters were written.
 *
 * @note This function forwards the supplied values directly to hardware and
 *       does not validate their range.
 */
status_t RTC_DRV_ConfigureTimeCompensation(uint32_t instance, uint8_t compInterval, int8_t compensation);

/*!
 * @brief Read the currently active RTC compensation values.
 *
 * @param[in]  instance       RTC instance index (0-based).
 * @param[out] compInterval   Pointer that receives the active compensation interval.
 * @param[out] compensation   Pointer that receives the active compensation value.
 */
void RTC_DRV_GetTimeCompensation(uint32_t instance, uint8_t *compInterval, int8_t *compensation);

/*! @} */ /* End of Time Compensation */

/*******************************************************************************
 * Interrupt Callback Configuration
 ******************************************************************************/
/*!
 * @name Interrupt Callback Configuration
 * @brief Functions for storing runtime callback configuration and programming
 *        the related interrupt sources.
 * @{
 */

/*!
 * @brief Configure the RTC overflow interrupt callback state.
 *
 * Stores @a intConfig in the driver runtime state, updates the hardware
 * overflow interrupt enable bit, and re-enables the generic RTC NVIC line.
 *
 * @param[in] instance   RTC instance index (0-based).
 * @param[in] intConfig  Pointer to the overflow interrupt configuration.
 */
void RTC_DRV_ConfigureOverflowInt(uint32_t instance, rtc_overflow_config_t *intConfig);

/*!
 * @brief Configure the RTC periodic seconds interrupt.
 *
 * Stores @a intConfig in the driver runtime state, selects the periodic event
 * frequency, and updates the seconds interrupt enable bit.
 *
 * @param[in] instance   RTC instance index (0-based).
 * @param[in] intConfig  Pointer to the seconds interrupt configuration.
 */
void RTC_DRV_ConfigureSecondsInt(uint32_t instance, rtc_seconds_config_t *const intConfig);

/*! @} */ /* End of Interrupt Callback Configuration */

/*******************************************************************************
 * Alarm Management
 ******************************************************************************/
/*!
 * @name Alarm Management
 * @brief Functions for configuring, querying, and tracking RTC alarms.
 * @{
 */

/*!
 * @brief Configure an RTC alarm and optional recurring alarm behavior.
 *
 * Validates the requested alarm time, converts it to seconds, verifies that it
 * is later than the current RTC time, then stores the configuration and
 * programs the hardware alarm register.
 *
 * @param[in] instance     RTC instance index (0-based).
 * @param[in] alarmConfig  Pointer to the alarm configuration.
 * @return Execution status.
 * @retval STATUS_SUCCESS Alarm configuration was accepted.
 * @retval STATUS_ERROR   Alarm time was invalid or not later than the current RTC time.
 */
status_t RTC_DRV_ConfigureAlarmInt(uint32_t instance, rtc_alarm_config_t *const alarmConfig);

/*!
 * @brief Copy the alarm configuration currently stored by the driver.
 *
 * @param[in]  instance     RTC instance index (0-based).
 * @param[out] alarmConfig  Pointer that receives the stored alarm configuration.
 *
 * @pre An alarm configuration pointer must already have been supplied through
 *      RTC_DRV_Init() or RTC_DRV_ConfigureAlarmInt().
 */
void RTC_DRV_GetAlarmConfig(uint32_t instance, rtc_alarm_config_t *alarmConfig);

/*!
 * @brief Check whether the RTC alarm flag is asserted.
 *
 * @param[in] instance  RTC instance index (0-based).
 * @return true if the alarm flag is set, false otherwise.
 */
bool RTC_DRV_IsAlarmPending(uint32_t instance);

/*!
 * @brief Retrieve the next alarm time after a recurring alarm reschedule.
 *
 * @param[in]  instance   RTC instance index (0-based).
 * @param[out] alarmTime  Pointer that receives the next alarm time in calendar form.
 * @return Execution status.
 * @retval STATUS_SUCCESS A next alarm time was available and converted successfully.
 * @retval STATUS_ERROR   No rescheduled alarm is pending.
 */
status_t RTC_DRV_GetNextAlarmTime(uint32_t instance, rtc_timedate_t *const alarmTime);

/*! @} */ /* End of Alarm Management */

/*******************************************************************************
 * Conversion & Validation Helpers
 ******************************************************************************/
/*!
 * @name Conversion & Validation Helpers
 * @brief Helper functions for converting RTC seconds values and validating
 *        calendar dates.
 * @{
 */

/*!
 * @brief Convert an RTC seconds counter value to calendar form.
 *
 * @param[in]  seconds   Pointer to the RTC seconds value to convert.
 * @param[out] timeDate  Pointer that receives the converted calendar time.
 */
void RTC_DRV_ConvertSecondsToTimeDate(const uint32_t *const seconds, rtc_timedate_t *const timeDate);

/*!
 * @brief Convert a calendar date/time value to RTC seconds format.
 *
 * @param[in]  timeDate  Pointer to the calendar time to convert.
 * @param[out] seconds   Pointer that receives the converted seconds count.
 */
void RTC_DRV_ConvertTimeDateToSeconds(const rtc_timedate_t *timeDate, uint32_t *const seconds);

/*!
 * @brief Check whether a year is a leap year in the Gregorian calendar.
 *
 * @param[in] year  Calendar year to evaluate.
 * @return true if @a year is a leap year, false otherwise.
 */
bool RTC_DRV_IsYearLeap(uint16_t year);

/*!
 * @brief Validate that a calendar date/time value is representable by the RTC driver.
 *
 * @param[in] timeDate  Pointer to the calendar time to validate.
 * @return true if the value is within the supported ranges, false otherwise.
 */
bool RTC_DRV_IsTimeDateCorrectFormat(const rtc_timedate_t *const timeDate);

/*! @} */ /* End of Conversion & Validation Helpers */

/*******************************************************************************
 * Internal IRQ Handlers
 ******************************************************************************/
/*!
 * @name Internal IRQ Handlers
 * @brief Driver-side ISR entry points used by the vector wrapper in rtc_irq.c.
 * @{
 */

/*!
 * @brief Handle generic RTC interrupt work for one instance.
 *
 * Processes alarm and overflow interrupt conditions and dispatches any user
 * callbacks stored in the runtime configuration.
 *
 * @param[in] instance  RTC instance index (0-based).
 */
void RTC_DRV_IRQHandler(uint32_t instance);

/*!
 * @brief Handle the periodic seconds interrupt for one instance.
 *
 * Dispatches the registered periodic seconds callback, if present, and clears
 * the corresponding seconds interrupt flag.
 *
 * @param[in] instance  RTC instance index (0-based).
 */
void RTC_DRV_SecondsIRQHandler(uint32_t instance);

/*! @} */ /* End of Internal IRQ Handlers */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of rtc group */

#endif /* RTC_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
