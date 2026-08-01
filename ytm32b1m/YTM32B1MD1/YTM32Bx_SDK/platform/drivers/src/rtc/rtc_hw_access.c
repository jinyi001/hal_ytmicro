/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file rtc_hw_access.c
 * @version 1.4.1
 *
 * @brief RTC Hardware Access Layer — non-inline helper implementations.
 *
 * This file contains the RTC hardware access functions that are implemented as
 * regular functions instead of `static inline` helpers.
 */

#include "rtc_hw_access.h"

/*******************************************************************************
 * Clock Output Control
 ******************************************************************************/

/*!
 * @brief Enable the RTC time counter.
 */
status_t RTC_Enable(RTC_Type *const base)
{
    status_t statusCode = STATUS_SUCCESS;
    bool isCounterEnabled;

    isCounterEnabled = RTC_GetTimeCounterEnable(base);

    if (isCounterEnabled)
    {
        statusCode = STATUS_ERROR;
    } else
    {
        RTC_SetTimeCounterEnable(base, true);
    }

    return statusCode;
}

/*!
 * @brief Disable the RTC time counter.
 */
status_t RTC_Disable(RTC_Type *const base)
{
    if (RTC_GetTimeCounterEnable(base) == true)
    {
        RTC_SetTimeCounterEnable(base, false);
    }

    /* Report whether the hardware accepted the disable request. */
    return (status_t)(RTC_GetTimeCounterEnable(base) ? STATUS_ERROR : STATUS_SUCCESS);
}

/*******************************************************************************
 * Time Seconds Register Access
 ******************************************************************************/

/*!
 * @brief Program the RTC seconds register.
 */
status_t RTC_SetTimeSecondsRegister(RTC_Type *const base, uint32_t seconds)
{
    status_t statusCode = STATUS_SUCCESS;

    if (RTC_GetTimeCounterEnable(base) == true)
    {
        statusCode = STATUS_ERROR;
    } else
    {
        base->SEC = seconds;
    }

    return statusCode;
}

/*******************************************************************************
 * Initialization & Counter Control
 ******************************************************************************/

/*!
 * @brief Configure the RTC clock-output pin source.
 */
status_t RTC_ConfigureClockOut(RTC_Type *const base, rtc_clk_out_config_t config)
{
    switch (config)
    {
        case RTC_CLKOUT_DISABLED:
            /* Disable the RTC clock-output pin. */
            base->CTRL &= ~RTC_CTRL_CLKOUTEN_MASK;
            break;
        case RTC_CLKOUT_SRC_CLKSEL:
            /* Output the selected RTC clock source on the clock-output pin. */
            base->CTRL &= ~(RTC_CTRL_CLKOUTEN_MASK | RTC_CTRL_CLKOUTSEL_MASK);
            base->CTRL |= (RTC_CTRL_CLKOUTEN(1U) | RTC_CTRL_CLKOUTSEL(0U));
            break;
        case RTC_CLKOUT_SRC_SEC:
            /* Output the periodic seconds waveform on the clock-output pin. */
            base->CTRL &= ~(RTC_CTRL_CLKOUTEN_MASK | RTC_CTRL_CLKOUTSEL_MASK);
            base->CTRL |= (RTC_CTRL_CLKOUTEN(1U) | RTC_CTRL_CLKOUTSEL(1U));
            break;
        default:
            break;
    }

    return STATUS_SUCCESS;
}
/*******************************************************************************
 * EOF
 ******************************************************************************/
