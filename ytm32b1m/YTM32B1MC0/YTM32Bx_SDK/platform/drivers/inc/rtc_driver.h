/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file rtc_driver.h
 * @version 1.4.0
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 4604 Rule 21.2: The object/function '%1s' is being declared with the same 
 *                        name as an ordinary identifier defined in '<%2s>'.
 *
 */

#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

#include <stddef.h>
#include "status.h"
#include "interrupt_manager.h"

/*!
 * @addtogroup rtc_driver RTC Driver
 * @ingroup rtc
 * @brief Real Time Clock Peripheral Driver
 * @details This section describes the programming interface of the RTC driver.
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

typedef enum
{
    RTC_OVERFLOW_INTERRUPT = 0U,
    RTC_ALARM_INTERRUPT = 1U,
    RTC_SECONDS_INTERRUPT = 2U,
} rtc_interrupt_mode_t;

/*!
 * @brief RTC Seconds interrupt configuration
 * Implements : rtc_second_int_cfg_t_Class
 */
typedef enum
{
    RTC_INT_1HZ = 0x00U,     /*!< RTC seconds interrupt occurs at 1 Hz      */
    RTC_INT_2HZ = 0x01U,     /*!< RTC seconds interrupt occurs at 2 Hz      */
    RTC_INT_4HZ = 0x02U,     /*!< RTC seconds interrupt occurs at 4 Hz      */
    RTC_INT_8HZ = 0x03U,     /*!< RTC seconds interrupt occurs at 8 Hz      */
    RTC_INT_16HZ = 0x04U,     /*!< RTC seconds interrupt occurs at 16 Hz     */
    RTC_INT_32HZ = 0x05U,     /*!< RTC seconds interrupt occurs at 32 Hz     */
    RTC_INT_64HZ = 0x06U,     /*!< RTC seconds interrupt occurs at 64 Hz     */
    RTC_INT_128HZ = 0x07U      /*!< RTC seconds interrupt occurs at 128 Hz    */
} rtc_second_int_cfg_t;

/*!
 * @brief RTC CLKOUT pin configuration
 * Implements : rtc_clk_out_config_t_Class
 */
typedef enum
{
    RTC_CLKOUT_DISABLED = 0x00U,  /*!< Clock out pin is disabled                                    */
    RTC_CLKOUT_SRC_SEC = 0x01U,  /*!< Output on RTC_CLKOUT as configured on Time seconds interrupt */
    RTC_CLKOUT_SRC_CLKSEL = 0x02U   /*!< Output on RTC_CLKOUT of the RTC clock select              */
} rtc_clk_out_config_t;


/*!
 * @brief RTC Time Date structure
 * Implements : rtc_timedate_t_Class
 */
typedef struct
{
    uint16_t year;      /*!< Year       */
    uint16_t month;     /*!< Month      */
    uint16_t day;       /*!< Day        */
    uint16_t hour;      /*!< Hour       */
    uint16_t minutes;   /*!< Minutes    */
    uint8_t seconds;    /*!< Seconds    */
} rtc_timedate_t;

/*!
 * @brief RTC alarm configuration
 * Implements : rtc_alarm_config_t_Class
 */
typedef struct
{
    rtc_timedate_t alarmTime;                                /*!< Alarm time                                       */
    uint32_t repetitionInterval;                       /*!< Interval of repetition in sec                    */
    uint32_t numberOfRepeats;                          /*!< Number of alarm repeats                          */
    bool repeatForever;                            /*!< Repeat forever if set, discard number of repeats */
    bool alarmIntEnable;                           /*!< Enable alarm interrupt                           */
    void (*rtcAlarmCallback)(void *callbackParam);  /*!< Pointer to the user callback method.             */
    void *callbackParams;                         /*!< Pointer to the callback parameters.              */
} rtc_alarm_config_t;

/*!
 * @brief RTC interrupt configuration. It is used to configure interrupt other
 *        than Time Alarm and Time Seconds interrupt
 * Implements : rtc_interrupt_config_t_Class
 */
typedef struct
{
    bool overflowIntEnable;                       /*!< Enable Time Overflow Interrupt       */
    void (*rtcOverflowCallback)(void *callbackParam);    /*!< Pointer to the user callback method. */
    void *callbackParams;                         /*!< Pointer to the callback parameters.  */
} rtc_overflow_config_t;

/*!
 * @brief RTC Seconds Interrupt Configuration
 * Implements : rtc_seconds_int_config_t_Class
 */
typedef struct
{
    rtc_second_int_cfg_t secondsIntConfig;                             /*!< Seconds Interrupt frequency          */
    bool secondsIntEnable;                             /*!< Seconds Interrupt enable             */
    void (*rtcSecondsCallback)(void *callbackParam); /*!< Pointer to the user callback method. */
    void *callbackParams;                      /*!< Pointer to the callback parameters.  */
} rtc_seconds_config_t;

/*!
 * @brief RTC Initialization structure
 * Implements : rtc_init_config_t_Class
 */
typedef struct
{
    uint8_t compensationInterval;       /*!< Compensation Interval                                                             */
    int8_t compensation;               /*!< Compensation Value                                                                */
    rtc_clk_source_t clockSource;                /*!< RTC Clock Select                                                                  */
    rtc_clk_out_config_t clockOutConfig;             /*!< RTC Clock Out Source                                                              */
    bool debugEnable;                /*!< Enable changing the Time Counter Enable bit even if the Status register is locked */
    rtc_alarm_config_t *rtcAlarmConfig;
    rtc_overflow_config_t *rtcOverflowConfig;
    rtc_seconds_config_t *rtcSecondsConfig;
} rtc_init_config_t;

/*!
 * @brief RTC Register Lock Configuration
 * Implements : rtc_register_lock_config_t_Class
 */
typedef struct
{
    bool lockRegisterLock;              /*!< Lock state of the Lock Register              */
    bool statusRegisterLock;            /*!< Lock state of the Status Register            */
    bool controlRegisterLock;           /*!< Lock state of the Control Register           */
    bool timeCompensationRegisterLock;  /*!< Lock state of the Time Compensation Register */
} rtc_register_lock_config_t;

/*******************************************************************************
 * Code
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief This function initializes the RTC instance with the settings
 *        provided by the user via the rtcUserCfg parameter. The user must ensure
 *        that clock is enabled for the RTC instance used. If the Control register
 *        is locked then this method returns STATUS_ERROR.
 *        In order to clear the CR Lock the user must perform a power-on reset.
 *
 * @param[in] instance The number of the RTC instance used
 * @param[in] rtcUserCfg Pointer to the user's configuration structure
 * @return    STATUS_SUCCESS if the operation was successful, STATUS_ERROR
 *            if Control is locked.
 */
status_t RTC_DRV_Init(uint32_t instance, const rtc_init_config_t *rtcUserCfg);

/*!
 * @brief This function de-initializes the RTC instance.
 *        If the Control register is locked then this method returns
 *        STATUS_ERROR.
 *
 * @param[in] instance The number of the RTC instance used
 * @return    STATUS_SUCCESS if the operation was successful or
 *            STATUS_ERROR if Control register is locked.
 */
status_t RTC_DRV_Deinit(uint32_t instance);

/*!
 * @brief This function will set the default configuration
 *        values into the structure passed as a parameter.
 * @param[out] config Pointer to the structure in which the
 * 				      configuration will be saved.
 */
void RTC_DRV_GetDefaultConfig(rtc_init_config_t *config);

/*!
 * @brief This function will set the RTC interrupt
 * @param[in] instance The number of the RTC instance
 * @param[in] mode The mode of RTC interrupt
 */
void RTC_DRV_SetRtcInterrupt(uint32_t instance, rtc_interrupt_mode_t mode);

/*!
 * @brief This function will clear the RTC interrupt
 * @param[in] instance The number of the RTC instance
 * @param[in] mode The mode of RTC interrupt
 */
void RTC_DRV_ClearRtcInterrupt(uint32_t instance, rtc_interrupt_mode_t mode);

/*!
 * @brief This function will clear the RTC interrupt flag
 * @param[in] instance The number of the RTC instance
 * @param[in] mode The mode of RTC interrupt
 */
void RTC_DRV_ClearRtcInterruptFlag(uint32_t instance, rtc_interrupt_mode_t mode);

/*!
 * @brief Start RTC instance counter. Before calling this function the user
 * should use RTC_DRV_SetTimeDate to configure the start time
 *
 * @param[in] instance The number of the RTC instance used
 * @return    STATUS_SUCCESS if the operation was successful, STATUS_ERROR
 *            if the counter cannot be enabled or is already enabled.
 */
status_t RTC_DRV_StartCounter(uint32_t instance);

/*!
 * @brief Disable RTC instance counter
 *
 * @param[in] instance The number of the RTC instance used
 * @return    STATUS_SUCCESS if the operation was successful, STATUS_ERROR
 *            if the counter could not be stopped.
 */
status_t RTC_DRV_StopCounter(uint32_t instance);

/*!
 * @brief Get current time and date from RTC instance
 * @param[in]  instance The number of the RTC instance used
 * @param[out] currentTime Pointer to the variable in which to store the result
 * @return     STATUS_SUCCESS if the operation was successful, STATUS_ERROR
 *             if there was a problem.
 */
status_t RTC_DRV_GetCurrentTimeDate(uint32_t instance, rtc_timedate_t *const currentTime);

/*!
 * @brief Set time and date for RTC instance. The user must stop the counter
 * before using this function. Otherwise it will return an error
 *
 * @param[in] instance The number of the RTC instance used
 * @param[in] time Pointer to the variable in which the time is stored
 * @return    STATUS_SUCCESS if the operation was successful, STATUS_ERROR
 *            if the time provided was invalid or if the counter was not
 *            stopped.
 */
status_t RTC_DRV_SetTimeDate(uint32_t instance, const rtc_timedate_t *time); /* PRQA S 4604 */

/*!
 * @brief This method configures register lock for the corresponding
 *        RTC instance.
 *
 * @param[in] instance The number of the RTC instance used
 * @return    STATUS_SUCCESS if the operation was successful,
 *            STATUS_ERROR if the Lock Register is locked.
 */
status_t RTC_DRV_LockRegisters(uint32_t instance);

/*!
 * @brief This method configures time compensation. Data is passed by
 *         the compInterval and compensation parameters.
 *         For more details regarding coefficient calculation see the
 *         Reference Manual.
 *
 * @param[in] instance The number of the RTC instance used
 * @param[in] compInterval Compensation interval
 * @param[in] compensation Compensation value
 * @return    STATUS_SUCCESS if the operation was successful,
 *            STATUS_ERROR if the TC Register is locked.
 */
status_t RTC_DRV_ConfigureTimeCompensation(uint32_t instance, uint8_t compInterval, int8_t compensation);

/*!
 * @brief This retrieves the time compensation coefficients and saves
 *        them on the variables referenced by the parameters.
 *
 * @param[in]  instance The number of the RTC instance used
 * @param[out] compInterval Pointer to the variable in which to save the compensation
 *             interval
 * @param[out] compensation Pointer to the variable in which to save the compensation
 *             value
 */
void RTC_DRV_GetTimeCompensation(uint32_t instance, uint8_t *compInterval, int8_t *compensation);

/*!
 * @brief This method configures fault interrupts such as:
 *         - Time Overflow Interrupt
 *         - Time Invalid Interrupt
 *        with the user provided configuration struct intConfig.
 *
 * @param[in] instance The number of the RTC instance used
 * @param[in] intConfig Pointer to the structure which holds the configuration
 */
void RTC_DRV_ConfigureOverflowInt(uint32_t instance, rtc_overflow_config_t *intConfig);

/*!
 * @brief This method configures the Time Seconds Interrupt with the
 *        configuration from the intConfig parameter.
 *
 * @param[in] instance The number of the RTC instance used
 * @param[in] intConfig Pointer to the structure which holds the configuration
 */
void RTC_DRV_ConfigureSecondsInt(uint32_t instance, rtc_seconds_config_t *const intConfig);

/*!
 * @brief This method configures the alarm with the
 *        configuration from the alarmConfig parameter.
 *
 * @param[in] instance The number of the RTC instance used
 * @param[in] alarmConfig Pointer to the structure which holds the alarm configuration
 * @return    STATUS_SUCCESS if the configuration is successful or
 *            STATUS_ERROR if the alarm time is invalid.
 */
status_t RTC_DRV_ConfigureAlarmInt(uint32_t instance, rtc_alarm_config_t *const alarmConfig);

/*!
 * @brief Get alarm configuration for RTC instance
 *
 * @param[in]  instance The number of the RTC instance used
 * @param[out] alarmConfig Pointer to the structure in which to store the alarm
 *             configuration
 */
void RTC_DRV_GetAlarmConfig(uint32_t instance, rtc_alarm_config_t *alarmConfig);

/*!
 * @brief Check if alarm is pending
 *
 * @param[in] instance The number of the RTC instance used
 * @return    True if the alarm has occurred, false if not
 */
bool RTC_DRV_IsAlarmPending(uint32_t instance);

/*!
 * @brief Convert seconds to rtc_timedate_t structure
 *
 * @param[in]  seconds Pointer to the seconds
 * @param[out] timeDate Pointer to the structure in which to store the result
 */
void RTC_DRV_ConvertSecondsToTimeDate(const uint32_t *const seconds, rtc_timedate_t *const timeDate);

/*!
 * @brief Convert seconds to rtc_timedate_t structure
 *
 * @param[in]  timeDate Pointer to the source struct
 * @param[out] seconds Pointer to the variable in which to store the result
 */
void RTC_DRV_ConvertTimeDateToSeconds(const rtc_timedate_t *timeDate, uint32_t *const seconds);

/*!
 * @brief Check if the current year is leap
 *
 * @param[in] year Year to check
 * @return    True if the year is leap, false if not
 */
bool RTC_DRV_IsYearLeap(uint16_t year);

/*!
 * @brief Check if the date time struct is configured properly
 *
 * @param[in] timeDate Structure to check to check
 * @return    True if the time date is in the correct format, false if not
 */
bool RTC_DRV_IsTimeDateCorrectFormat(const rtc_timedate_t *const timeDate);

/*!
 * @brief Gets the next alarm time
 *
 * @param[in]  instance The number of the RTC instance used
 * @param[out] alarmTime Pointer to the variable in which to store the data
 * @return     STATUS_SUCCESS if the next alarm time is valid,
 *             STATUS_ERROR if there is no new alarm or alarm configuration
 *             specified.
 */
status_t RTC_DRV_GetNextAlarmTime(uint32_t instance, rtc_timedate_t *const alarmTime);

/*!
 * @brief This method is the API's Interrupt handler for generic and alarm IRQ.
 * It will handle the alarm repetition and calls the user callbacks if they
 * are not NULL.
 *
 * @param[in] instance RTC instance used
 */
void RTC_DRV_IRQHandler(uint32_t instance);

/*!
 * @brief This method is the API's Interrupt handler for RTC Second
 * interrupt. This ISR will call the user callback if defined.
 *
 * @param[in] instance RTC instance used
 */
void RTC_DRV_SecondsIRQHandler(uint32_t instance);

#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* RTC_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
