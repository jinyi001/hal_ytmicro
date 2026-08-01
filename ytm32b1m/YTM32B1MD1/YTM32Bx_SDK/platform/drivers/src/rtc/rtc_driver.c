/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file rtc_driver.c
 * @version 1.4.1
 *
 * @brief RTC Driver — implementation of the public RTC_DRV_* API.
 *
 * This file implements the application-level RTC driver functions declared in
 * rtc_driver.h. The driver wraps the low-level hardware access helpers in
 * rtc_hw_access.h and maintains per-instance runtime state for alarm and
 * interrupt callback handling.
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 2016 Rule 16.4: This 'switch' statement 'default' clause is empty.
 *
 */

#include "rtc_hw_access.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Table of base addresses for RTC peripheral instances. */
static RTC_Type *const g_rtcBase[RTC_INSTANCE_COUNT] = RTC_BASE_PTRS;

/*! @brief Table of generic RTC IRQ numbers for each instance. */
static const IRQn_Type g_rtcIrqNumbers[] = RTC_IRQS;
#if FEATURE_RTC_HAS_SEPARATE_SECOND_IRQ
/*! @brief Table of dedicated RTC seconds IRQ numbers for each instance. */
static const IRQn_Type g_rtcSecondsIrqNb[] = RTC_SECONDS_IRQS;
#else
/*! @brief Devices without a dedicated seconds IRQ reuse the generic RTC IRQ. */
static const IRQn_Type       g_rtcSecondsIrqNb[]   = RTC_IRQS;
#endif

/* Month lengths for a non-leap year; index 0 is unused. */
static const uint8_t ULY[] = {0U, 31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};

/* Month lengths for a leap year; index 0 is unused. */
static const uint8_t LY[] = {0U, 31U, 29U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};

/* Cumulative day offsets at the start of each month for a non-leap year. */
static const uint16_t MONTH_DAYS[] = {0U, 0U, 31U, 59U, 90U, 120U, 151U, 181U, 212U, 243U, 273U, 304U, 334U};

/*!
 * @brief Per-instance runtime state used by the RTC driver.
 */
static struct
{
    bool isAlarmTimeNew;                     /*!< Tracks whether a recurring alarm has been rescheduled. */
    rtc_alarm_config_t *alarmConfig;         /*!< Stored alarm configuration pointer. */
    rtc_overflow_config_t *overflowConfig;   /*!< Stored overflow interrupt configuration pointer. */
    rtc_seconds_config_t *secondsConfig;     /*!< Stored periodic seconds interrupt configuration pointer. */
} g_rtcRuntimeConfig[RTC_INSTANCE_COUNT];


/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/

/*!
 * @brief Initialize the RTC instance and store the runtime callback configuration.
 */
status_t RTC_DRV_Init(uint32_t instance, const rtc_init_config_t *rtcUserCfg)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);
    DEV_ASSERT(rtcUserCfg != NULL);

    status_t statusCode = STATUS_SUCCESS;
    RTC_Type *basePtr = g_rtcBase[instance];

    /* Refresh the runtime configuration pointers used by the ISR paths. */
    g_rtcRuntimeConfig[instance].alarmConfig = rtcUserCfg->rtcAlarmConfig;
    g_rtcRuntimeConfig[instance].overflowConfig = rtcUserCfg->rtcOverflowConfig;
    g_rtcRuntimeConfig[instance].secondsConfig = rtcUserCfg->rtcSecondsConfig;
    g_rtcRuntimeConfig[instance].isAlarmTimeNew = false;

    /* Reinitialize the hardware only when the counter is not already running. */
    if (RTC_GetTimeCounterEnable(g_rtcBase[instance]) == false)
    {
        /* Reset the peripheral before applying the requested configuration. */
        INT_SYS_DisableIRQ(g_rtcIrqNumbers[instance]);
        RTC_SoftwareReset(basePtr);
        RTC_EnableRegisterUnlock(basePtr);
        INT_SYS_ClearPending(g_rtcIrqNumbers[instance]);
        RTC_ClearIntEnable(basePtr);
        (void) RTC_ConfigureClockOut(basePtr, rtcUserCfg->clockOutConfig);
        RTC_SetClockSource(basePtr, rtcUserCfg->clockSource);
        RTC_SetDebugMode(basePtr, rtcUserCfg->debugEnable);
        if (rtcUserCfg->compensation != 0)
        {
            RTC_SetTimeCompensation(basePtr,
                                    rtcUserCfg->compensation,
                                    rtcUserCfg->compensationInterval);
        }
    }

    /* Apply optional interrupt configuration blocks supplied by the caller. */
    if (rtcUserCfg->rtcOverflowConfig != NULL)
    {
        RTC_DRV_ConfigureOverflowInt(instance, rtcUserCfg->rtcOverflowConfig);
    } else
    {
        g_rtcRuntimeConfig[instance].overflowConfig = NULL;
    }

    if (rtcUserCfg->rtcAlarmConfig != NULL)
    {
        (void)RTC_DRV_ConfigureAlarmInt(instance, rtcUserCfg->rtcAlarmConfig);
    } else
    {
        g_rtcRuntimeConfig[instance].alarmConfig = NULL;
    }

    if (rtcUserCfg->rtcSecondsConfig != NULL)
    {
        RTC_DRV_ConfigureSecondsInt(instance, rtcUserCfg->rtcSecondsConfig);
    } else
    {
        g_rtcRuntimeConfig[instance].secondsConfig = NULL;
    }

    return statusCode;
}

/*!
 * @brief De-initialize the RTC instance with a software reset.
 */
status_t RTC_DRV_Deinit(uint32_t instance)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);

    status_t statusCode = STATUS_SUCCESS;

    /* Skip the reset if the counter is already disabled. */
    if (RTC_GetTimeCounterEnable(g_rtcBase[instance]) == false)
    {
        statusCode = STATUS_ERROR;
    } else
    {
        INT_SYS_DisableIRQ(g_rtcIrqNumbers[instance]);
        INT_SYS_DisableIRQ(g_rtcSecondsIrqNb[instance]);
        RTC_SoftwareReset(g_rtcBase[instance]);
        INT_SYS_ClearPending(g_rtcIrqNumbers[instance]);
    }

    return statusCode;
}

/*!
 * @brief Populate an RTC initialization structure with default values.
 */
void RTC_DRV_GetDefaultConfig(rtc_init_config_t *config)
{
    DEV_ASSERT(config != NULL);
    config->clockSource = (rtc_clk_source_t)0U;
    config->clockOutConfig = RTC_CLKOUT_DISABLED;
    config->debugEnable = true;
    config->compensation = 0;
    config->compensationInterval = 0U;
}

/*******************************************************************************
 * Interrupt Source Control
 ******************************************************************************/

/*!
 * @brief Enable one RTC interrupt source in hardware.
 */
void RTC_DRV_SetRtcInterrupt(uint32_t instance, rtc_interrupt_mode_t mode)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);

    RTC_Type *basePtr = g_rtcBase[instance];

    switch (mode)
    {
        case RTC_OVERFLOW_INTERRUPT:
            RTC_SetTimeOverflowIntEnable(basePtr, true);
            break;
        case RTC_ALARM_INTERRUPT:
            RTC_SetTimeAlarmIntEnable(basePtr, true);
            break;
        case RTC_SECONDS_INTERRUPT:
            RTC_SetTimeSecondsIntEnable(basePtr, true);
            break;
        default: /*PRQA S 2016*/
            break;
    }
}

/*!
 * @brief Disable one RTC interrupt source in hardware.
 */
void RTC_DRV_ClearRtcInterrupt(uint32_t instance, rtc_interrupt_mode_t mode)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);

    RTC_Type *basePtr = g_rtcBase[instance];

    switch (mode)
    {
        case RTC_OVERFLOW_INTERRUPT:
            RTC_SetTimeOverflowIntEnable(basePtr, false);
            break;
        case RTC_ALARM_INTERRUPT:
            RTC_SetTimeAlarmIntEnable(basePtr, false);
            break;
        case RTC_SECONDS_INTERRUPT:
            RTC_SetTimeSecondsIntEnable(basePtr, false);
            break;
        default: /*PRQA S 2016*/
            break;
    }
}

/*!
 * @brief Clear the pending flag for one RTC interrupt source.
 */
void RTC_DRV_ClearRtcInterruptFlag(uint32_t instance, rtc_interrupt_mode_t mode)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);

    RTC_Type *basePtr = g_rtcBase[instance];

    switch (mode)
    {
        case RTC_OVERFLOW_INTERRUPT:
            RTC_ClearTimeOverflowIntFlag(basePtr);
            break;
        case RTC_ALARM_INTERRUPT:
            RTC_ClearTimeAlarmIntFlag(basePtr);
            break;
        case RTC_SECONDS_INTERRUPT:
            RTC_ClearTimeSecondsIntFlag(basePtr);
            break;
        default: /*PRQA S 2016*/
            break;
    }
}

/*******************************************************************************
 * Counter Control
 ******************************************************************************/

/*!
 * @brief Start the RTC counter.
 */
status_t RTC_DRV_StartCounter(uint32_t instance)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);

    status_t statusCode = STATUS_SUCCESS;

    statusCode = RTC_Enable(g_rtcBase[instance]);

    return statusCode;
}

/*!
 * @brief Stop the RTC counter.
 */
status_t RTC_DRV_StopCounter(uint32_t instance)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);

    status_t statusCode = STATUS_SUCCESS;

    statusCode = RTC_Disable(g_rtcBase[instance]);

    return statusCode;
}

/*******************************************************************************
 * Calendar Time Access
 ******************************************************************************/

/*!
 * @brief Read the current RTC time and convert it to calendar format.
 */
status_t RTC_DRV_GetCurrentTimeDate(uint32_t instance, rtc_timedate_t *const currentTime)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);
    DEV_ASSERT(currentTime != NULL);

    status_t statusCode = STATUS_SUCCESS;
    uint32_t seconds;
    uint32_t tempSeconds;

    /* Double-read the seconds register to avoid sampling during rollover. */
    tempSeconds = RTC_GetTimeSecondsRegister(g_rtcBase[instance]);
    seconds = RTC_GetTimeSecondsRegister(g_rtcBase[instance]);
    if (tempSeconds != seconds)
    {
        tempSeconds = RTC_GetTimeSecondsRegister(g_rtcBase[instance]);
        if (tempSeconds != seconds)
        {
            statusCode = STATUS_ERROR;
        } else
        {
            RTC_DRV_ConvertSecondsToTimeDate(&seconds, currentTime);
        }
    } else
    {
        RTC_DRV_ConvertSecondsToTimeDate(&seconds, currentTime);
    }

    return statusCode;
}

/*!
 * @brief Program the RTC time using a calendar date/time value.
 */
status_t RTC_DRV_SetTimeDate(uint32_t instance, const rtc_timedate_t *timeDate)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);
    DEV_ASSERT(timeDate != NULL);

    status_t statusCode = STATUS_SUCCESS;
    uint32_t seconds = 0;

    if (RTC_DRV_IsTimeDateCorrectFormat(timeDate) == false)
    {
        statusCode = STATUS_ERROR;
    } else
    {
        RTC_DRV_ConvertTimeDateToSeconds(timeDate, &seconds);
        statusCode = RTC_SetTimeSecondsRegister(g_rtcBase[instance], seconds);
    }

    return statusCode;
}

/*******************************************************************************
 * Time Compensation
 ******************************************************************************/

/*!
 * @brief Program the RTC compensation value and interval.
 */
status_t RTC_DRV_ConfigureTimeCompensation(uint32_t instance, uint8_t compInterval, int8_t compensation)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);

    status_t statusCode = STATUS_SUCCESS;

    RTC_SetTimeCompensation(g_rtcBase[instance], compensation, compInterval);

    return statusCode;
}

/*!
 * @brief Read the currently active RTC compensation values.
 */
void RTC_DRV_GetTimeCompensation(uint32_t instance, uint8_t *compInterval, int8_t *compensation)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);
    DEV_ASSERT(compInterval != NULL);
    DEV_ASSERT(compensation != NULL);

    RTC_GetCurrentTimeCompensation(g_rtcBase[instance], compensation, compInterval);
}

/*******************************************************************************
 * Conversion & Validation Helpers
 ******************************************************************************/

/*!
 * @brief Convert an RTC seconds count to calendar form.
 */
void RTC_DRV_ConvertSecondsToTimeDate(const uint32_t *const seconds, rtc_timedate_t *const timeDate)
{
    DEV_ASSERT(seconds != NULL);
    DEV_ASSERT(timeDate != NULL);

    uint8_t i;
    bool yearLeap = false;
    uint32_t numberOfDays = 0U;
    uint32_t tempSeconds;
    uint16_t daysInYear;

    /* The supported epoch starts at 1970, which is not a leap year. */
    daysInYear = DAYS_IN_A_YEAR;

    timeDate->year = YEAR_RANGE_START;

    numberOfDays = (*seconds) / SECONDS_IN_A_DAY;
    tempSeconds = (*seconds) % SECONDS_IN_A_DAY;

    timeDate->hour = (uint16_t) (tempSeconds / SECONDS_IN_A_HOUR);
    tempSeconds = tempSeconds % SECONDS_IN_A_HOUR;
    timeDate->minutes = (uint16_t) (tempSeconds / SECONDS_IN_A_MIN);
    timeDate->seconds = (uint8_t) (tempSeconds % SECONDS_IN_A_MIN);

    /* Consume full years until the remaining day count fits in the current year. */
    while (numberOfDays >= daysInYear)
    {
        timeDate->year++;
        numberOfDays -= daysInYear;

        if (!RTC_DRV_IsYearLeap(timeDate->year))
        {
            daysInYear = DAYS_IN_A_YEAR;
        } else
        {
            daysInYear = DAYS_IN_A_LEAP_YEAR;
        }
    }

    numberOfDays += 1U;

    yearLeap = RTC_DRV_IsYearLeap(timeDate->year);

    /* Consume full months until the remaining day count fits in the current month. */
    for (i = 1U; i <= 12U; i++)
    {
        uint32_t daysInCurrentMonth = ((yearLeap == true) ? (uint32_t) LY[i] : (uint32_t) ULY[i]);
        if (numberOfDays <= daysInCurrentMonth)
        {
            timeDate->month = (uint16_t) i;
            break;
        } else
        {
            numberOfDays -= daysInCurrentMonth;
        }

    }

    timeDate->day = (uint16_t) numberOfDays;
}

/*!
 * @brief Convert a calendar date/time value to an RTC seconds count.
 */
void RTC_DRV_ConvertTimeDateToSeconds(const rtc_timedate_t *timeDate, uint32_t *const seconds)
{
    DEV_ASSERT(seconds != NULL);
    DEV_ASSERT(timeDate != NULL);

    uint16_t year;

    (*seconds) = (uint32_t) (DAYS_IN_A_YEAR * (uint32_t) (SECONDS_IN_A_DAY));
    (*seconds) *= ((uint32_t) timeDate->year - YEAR_RANGE_START);

    /* Add one extra day for each leap year between the epoch and the target year. */
    for (year = YEAR_RANGE_START; year < timeDate->year; year++)
    {
        if (RTC_DRV_IsYearLeap(year))
        {
            (*seconds) += SECONDS_IN_A_DAY;
        }
    }

    /* Add February 29 when the target date is after it in a leap year. */
    if ((RTC_DRV_IsYearLeap(year)) && (timeDate->month > 2U))
    {
        (*seconds) += SECONDS_IN_A_DAY;
    }

    (*seconds) += MONTH_DAYS[timeDate->month] * SECONDS_IN_A_DAY;
    (*seconds) += (uint32_t) (((uint32_t) timeDate->day - 1U) * (uint32_t) SECONDS_IN_A_DAY);
    (*seconds) += (uint32_t) (((uint32_t) timeDate->hour * SECONDS_IN_A_HOUR) + \
                             ((uint32_t) timeDate->minutes * SECONDS_IN_A_MIN) + \
                             (uint32_t) timeDate->seconds);
}

/*!
 * @brief Validate that a calendar date/time value is supported by the RTC driver.
 */
bool RTC_DRV_IsTimeDateCorrectFormat(const rtc_timedate_t *const timeDate)
{
    DEV_ASSERT(timeDate != NULL);

    bool returnCode = true;
    const uint8_t *pDays;

    pDays = RTC_DRV_IsYearLeap(timeDate->year) ? (LY) : (ULY);

    if ((timeDate->year < YEAR_RANGE_START) || (timeDate->year > YEAR_RANGE_END)
        || (timeDate->month < 1U) || (timeDate->month > 12U)
        || (timeDate->day < 1U) || (timeDate->day > 31U)
        || (timeDate->hour >= HOURS_IN_A_DAY)
        || (timeDate->minutes >= MINS_IN_A_HOUR) || (timeDate->seconds >= SECONDS_IN_A_MIN))
    {
        returnCode = false;
    }
    else if (timeDate->day > pDays[timeDate->month])
    {
        returnCode = false;
    } else
    {
        returnCode = true;
    }

    return returnCode;
}

/*!
 * @brief Check whether a year is a leap year.
 */
bool RTC_DRV_IsYearLeap(uint16_t year)
{
    bool isYearLeap = false;

    if ((year % 4U) > 0U)
    {
        isYearLeap = false;
    } else if ((year % 100U) > 0U)
    {
        isYearLeap = true;
    } else if ((year % 400U) > 0U)
    {
        isYearLeap = false;
    } else
    {
        isYearLeap = true;
    }

    return isYearLeap;
}


/*******************************************************************************
 * Internal IRQ Handlers
 ******************************************************************************/

/*!
 * @brief Handle generic RTC alarm and overflow interrupt work.
 */
void RTC_DRV_IRQHandler(uint32_t instance)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);

    uint32_t tempSeconds;
    rtc_alarm_config_t *alarmConfig = g_rtcRuntimeConfig[instance].alarmConfig;
    const rtc_overflow_config_t *const overflowConfig = g_rtcRuntimeConfig[instance].overflowConfig;

    if (RTC_GetTimeAlarmFlag(g_rtcBase[instance]) == true)
    {
        if ((alarmConfig != NULL))
        {
            if ((alarmConfig->numberOfRepeats > 0UL) || (alarmConfig->repeatForever == true))
            {
                tempSeconds = RTC_GetTimeSecondsRegister(g_rtcBase[instance]);
                tempSeconds += alarmConfig->repetitionInterval - 1UL;
                RTC_SetTimeAlarmRegister(g_rtcBase[instance], tempSeconds);

                g_rtcRuntimeConfig[instance].isAlarmTimeNew = true;
                /* Keep the repeat counter quiescent for infinite-repeat mode. */
                alarmConfig->numberOfRepeats = (alarmConfig->repeatForever == false) ? (alarmConfig->numberOfRepeats -
                                                                                        1UL) : 0UL;
            } else
            {
                RTC_SetTimeAlarmRegister(g_rtcBase[instance], 0UL);
                g_rtcRuntimeConfig[instance].isAlarmTimeNew = false;
            }
            if (alarmConfig->rtcAlarmCallback != NULL)
            {
                alarmConfig->rtcAlarmCallback(alarmConfig->callbackParams);
            }
        }
    }
    else if (overflowConfig != NULL)
    {
        if (overflowConfig->rtcOverflowCallback != NULL)
        {
            overflowConfig->rtcOverflowCallback(overflowConfig->callbackParams);
        }
    }
}

/*!
 * @brief Handle the RTC periodic seconds interrupt.
 */
void RTC_DRV_SecondsIRQHandler(uint32_t instance)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);

    const rtc_seconds_config_t *const intCfg = g_rtcRuntimeConfig[instance].secondsConfig;

    if ((intCfg != NULL) && (intCfg->rtcSecondsCallback != NULL))
    {
        intCfg->rtcSecondsCallback(intCfg->callbackParams);
    }
    RTC_ClearTimeSecondsIntFlag(g_rtcBase[instance]);
}

/*******************************************************************************
 * Interrupt Callback Configuration
 ******************************************************************************/

/*!
 * @brief Configure the RTC overflow interrupt callback state.
 */
void RTC_DRV_ConfigureOverflowInt(uint32_t instance, rtc_overflow_config_t *intConfig)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);
    DEV_ASSERT(intConfig != NULL);

    /* Update the callback pointer atomically with respect to the IRQ line. */
    INT_SYS_DisableIRQ(g_rtcIrqNumbers[instance]);
    g_rtcRuntimeConfig[instance].overflowConfig = intConfig;
    RTC_SetTimeOverflowIntEnable(g_rtcBase[instance], intConfig->overflowIntEnable);
    INT_SYS_EnableIRQ(g_rtcIrqNumbers[instance]);
}

/*!
 * @brief Configure the RTC periodic seconds interrupt.
 */
void RTC_DRV_ConfigureSecondsInt(uint32_t instance, rtc_seconds_config_t *const intConfig)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);
    DEV_ASSERT(intConfig != NULL);

    /* Update the callback pointer atomically with respect to the IRQ line. */
    INT_SYS_DisableIRQ(g_rtcSecondsIrqNb[instance]);
    g_rtcRuntimeConfig[instance].secondsConfig = intConfig;
    RTC_SetTimeSecondsIntConf(g_rtcBase[instance], intConfig->secondsIntConfig);
    RTC_SetTimeSecondsIntEnable(g_rtcBase[instance], intConfig->secondsIntEnable);
    INT_SYS_EnableIRQ(g_rtcSecondsIrqNb[instance]);
}

/*******************************************************************************
 * Alarm Management
 ******************************************************************************/

/*!
 * @brief Configure an RTC alarm and optional recurring alarm behavior.
 */
status_t RTC_DRV_ConfigureAlarmInt(uint32_t instance, rtc_alarm_config_t *const alarmConfig)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);
    DEV_ASSERT(alarmConfig != NULL);

    status_t statusCode = STATUS_SUCCESS;
    uint32_t alarmTime;
    uint32_t currentTime;

    if (RTC_DRV_IsTimeDateCorrectFormat(&(alarmConfig->alarmTime)) == true)
    {
        RTC_DRV_ConvertTimeDateToSeconds(&(alarmConfig->alarmTime), &alarmTime);
        currentTime = RTC_GetTimeSecondsRegister(g_rtcBase[instance]);

        if (alarmTime > currentTime)
        {
            INT_SYS_DisableIRQ(g_rtcIrqNumbers[instance]);
            g_rtcRuntimeConfig[instance].alarmConfig = alarmConfig;

            RTC_SetTimeAlarmRegister(g_rtcBase[instance], alarmTime);
            RTC_SetTimeAlarmIntEnable(g_rtcBase[instance], alarmConfig->alarmIntEnable);
            INT_SYS_EnableIRQ(g_rtcIrqNumbers[instance]);
        } else
        {
            statusCode = STATUS_ERROR;
        }
    } else
    {
        statusCode = STATUS_ERROR;
    }

    return statusCode;
}

/*!
 * @brief Copy the alarm configuration currently stored by the driver.
 */
void RTC_DRV_GetAlarmConfig(uint32_t instance, rtc_alarm_config_t *alarmConfig)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);
    DEV_ASSERT(alarmConfig != NULL);

    *alarmConfig = *(g_rtcRuntimeConfig[instance].alarmConfig);
}

/*!
 * @brief Check whether the RTC alarm flag is asserted.
 */
bool RTC_DRV_IsAlarmPending(uint32_t instance)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);

    return RTC_GetTimeAlarmFlag(g_rtcBase[instance]);
}

/*!
 * @brief Retrieve the next alarm time scheduled by recurring-alarm handling.
 */
status_t RTC_DRV_GetNextAlarmTime(uint32_t instance, rtc_timedate_t *const alarmTime)
{
    DEV_ASSERT(instance < RTC_INSTANCE_COUNT);
    DEV_ASSERT(alarmTime != NULL);

    status_t statusCode = STATUS_SUCCESS;
    uint32_t alarmInSec;

    if (g_rtcRuntimeConfig[instance].isAlarmTimeNew == true)
    {
        alarmInSec = RTC_GetTimeAlarmRegister(g_rtcBase[instance]);
        RTC_DRV_ConvertSecondsToTimeDate(&alarmInSec, alarmTime);
    } else
    {
        statusCode = STATUS_ERROR;
    }

    return statusCode;
}
/*******************************************************************************
 * EOF
 ******************************************************************************/
