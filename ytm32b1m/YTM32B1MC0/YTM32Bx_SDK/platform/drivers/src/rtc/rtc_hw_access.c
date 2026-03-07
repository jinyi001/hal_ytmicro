/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file rtc_hw_access.c
 * @version 1.4.0
 */

#include "rtc_hw_access.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : RTC_Enable
 * Description   : This function enables the RTC counter
 * Return          STATUS_SUCCESS if the operation was successful, STATUS_ERROR
 *                 if the counter is enabled.
 *END**************************************************************************/
status_t RTC_Enable(RTC_Type *const base)
{
    status_t statusCode = STATUS_SUCCESS;
    bool isCounterEnabled;

    /* Get the flags signaling if the counter is enabled */
    isCounterEnabled = RTC_GetTimeCounterEnable(base);

    /* Check if the RTC counter is enabled */
    if (isCounterEnabled)
    {
        statusCode = STATUS_ERROR;
    } else
    {
        /* Enable oscillator and seconds counter */
        RTC_SetTimeCounterEnable(base, true);
    }
    /* Return the exit code */
    return statusCode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : RTC_Disable
 * Description   : This function disables the RTC counter
 * Return          STATUS_SUCCESS if the operation was successful, STATUS_ERROR
 *                 if the counter was not disabled.
 *END**************************************************************************/
status_t RTC_Disable(RTC_Type *const base)
{
    /* Check if counter is enabled and disable it */
    if (RTC_GetTimeCounterEnable(base) == true)
    {
        RTC_SetTimeCounterEnable(base, false);
    }

    /* Read TCE bit to check if the counter is really disabled and return the
     * corresponding result.
     *  -   Error if the timer is still enabled (The register can be locked)
     *  -   Success if the timer is disabled
     */
    return (status_t) (RTC_GetTimeCounterEnable(base) ? STATUS_ERROR : STATUS_SUCCESS);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : RTC_SetTimeSecondsRegister
 * Description   : This function will help you set the time at a specified value.
 *                 write will fail if the Time Counter is enabled and will return
 *                 STATUS_ERROR, otherwise the return will be STATUS_SUCCESS
 * Return        : STATUS_SUCCESS if write is succeeded or STATUS_ERROR if
 *                 the counter is enabled.
 *END**************************************************************************/
status_t RTC_SetTimeSecondsRegister(RTC_Type *const base, uint32_t seconds)
{
    status_t statusCode = STATUS_SUCCESS;

    if (RTC_GetTimeCounterEnable(base) == true)
    {
        statusCode = STATUS_ERROR;
    } else
    {
        base->SEC = seconds;
        statusCode = STATUS_SUCCESS;
    }
    /* Return the exit code */
    return statusCode;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : RTC_ConfigureClockOut
 * Description   : This method will allow you to configure the RTC Clock out pin.
 *                 It will return STATUS_SUCCESS if the configuration was successful
 *                 STATUS_ERROR if the Control Register is locked.
 * Return        : Status of the operation
 *END**************************************************************************/
status_t RTC_ConfigureClockOut(RTC_Type *const base, rtc_clk_out_config_t config)
{
    /*
     * Check if the Control Register is already locked,
     * if true, clock out configuration cannot be modified.
     */
    switch (config)
    {
        case RTC_CLKOUT_DISABLED:
            /* Disable the clock out pin */
            base->CTRL &= ~RTC_CTRL_CLKOUTEN_MASK;
            break;
        case RTC_CLKOUT_SRC_CLKSEL:
            /* Select clock out source as the 32 KHz clock and enable the pin */
            base->CTRL &= ~(RTC_CTRL_CLKOUTEN_MASK | RTC_CTRL_CLKOUTSEL_MASK);
            base->CTRL |= (RTC_CTRL_CLKOUTEN(1U) | RTC_CTRL_CLKOUTSEL(0U));
            break;
        case RTC_CLKOUT_SRC_SEC:
            /* Select clock out source as Time Seconds Interrupt and enable the pin */
            base->CTRL &= ~(RTC_CTRL_CLKOUTEN_MASK | RTC_CTRL_CLKOUTSEL_MASK);
            base->CTRL |= (RTC_CTRL_CLKOUTEN(1U) | RTC_CTRL_CLKOUTSEL(1U));
            break;
        default:
            /* This statement should not be reached */
            break;
    }
    /* Return the exit code */
    return STATUS_SUCCESS;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
