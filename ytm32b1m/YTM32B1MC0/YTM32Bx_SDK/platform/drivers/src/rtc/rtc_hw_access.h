/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file rtc_hw_access.h
 * @version 1.4.0
 */

#ifndef RTC_HW_ACCESS_H
#define RTC_HW_ACCESS_H

#include <stdbool.h>
#include "device_registers.h"
#include "status.h"
#include "rtc_driver.h"
#include <stdint.h>

/*!
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name Configuration
 * @{
 */

/*****************************************************************************
 * Methods for RTC Control
 ****************************************************************************/

/*!
 * @brief Enable RTC instance counter
 *
 * @param[in] base RTC base pointer
 * @return    STATUS_SUCCESS if the operation was successful, STATUS_ERROR
 *            if the counter is enabled or if the time invalid flag is set.
 */
status_t RTC_Enable(RTC_Type *const base);

/*!
 * @brief Disable RTC instance counter
 *
 * @param[in] base RTC base pointer
 * @return    STATUS_SUCCESS if the operation was successful, STATUS_ERROR
 *            if the counter was not disabled.
 */
status_t RTC_Disable(RTC_Type *const base);

/*!
 * @brief This function configures the Clock Out pin source
 *
 * @param[in] base RTC base pointer
 * @param[in] config Source for the Clock Out pin
 * @return    Returns the status of the operation, STATUS_SUCCESS
 *            if the configuration was successful, STATUS_ERROR if the Control
 *            Register is locked.
 */
status_t RTC_ConfigureClockOut(RTC_Type *const base, rtc_clk_out_config_t config);

/*****************************************************************************
 * Methods for RTC Time Seconds register
 ****************************************************************************/

/*!
 * @brief Get Time Seconds Register Value
 *
 * @param[in] base RTC base pointer
 * @return    Number of seconds passed
 */
static inline uint32_t RTC_GetTimeSecondsRegister(const RTC_Type* base)
{
    uint32_t tmp = base->SEC;
    tmp = (tmp & RTC_SEC_SEC_MASK) >> RTC_SEC_SEC_SHIFT;
    return (uint32_t) (tmp);
}

/*!
 * @brief Set Time Seconds Register
 *
 * This function will help you set the time at a specified value.
 * The write will fail if the Time Counter is enabled and will return
 * STATUS_ERROR, otherwise the return will be STATUS_SUCCESS
 *
 * @param[in] base    RTC base pointer
 * @param[in] seconds number of seconds passed
 * @return    STATUS_SUCCESS if the write is succeeded or STATUS_ERROR if
 *            the counter is enabled.
 */
status_t RTC_SetTimeSecondsRegister(RTC_Type *const base, uint32_t seconds);

/*****************************************************************************
 * Methods for RTC Time Alarm register
 ****************************************************************************/

/*!
 * @brief Get Time Alarm Register
 *
 * @param[in] base RTC base pointer
 * @return    Value in seconds of the Time Alarm Register
 */
static inline uint32_t RTC_GetTimeAlarmRegister(const RTC_Type * base)
{
    uint32_t tmp = base->ALM;
    tmp = (tmp & RTC_ALM_ALM_MASK) >> RTC_ALM_ALM_SHIFT;
    return (uint32_t) (tmp);
}

/*!
 * @brief Set Time Alarm Register
 *
 * @param[in] base    RTC base pointer
 * @param[in] seconds Number of seconds at which the alarm is triggered. The ALM
 *                    value is correct only if the value is greater than current
 *                    time (Time seconds register)
 */
static inline void RTC_SetTimeAlarmRegister(RTC_Type *const base, uint32_t seconds)
{
    base->ALM = seconds;
    /* update flag bit */
    base->INTF = 0;
}

/*****************************************************************************
 * Methods for RTC Time Compensation register
 ****************************************************************************/

/*!
 * @brief Set Time Compensation
 *
 * Configure the frequency of the Time Seconds counter together with
 * Compensation Interval register.
 *
 * The Time Prescaler register overflows at every 32768 - (compValue)
 * cycles. For example if the compValue is -128 TPR overflows at
 * 32768 - (-128) = 32896 cycles
 *
 * Else if compValue is 127 TPR overflows at 32641 cycles
 *
 * The compensation interval in seconds from 1 to 256 is used to
 * control how frequently the CMP should adjust the number of 32.768 kHz
 * cycles in each second. The value written should be one less than the
 * number of seconds. For example, write zero to configure for a
 * compensation interval of one second. This register is double buffered
 * and writes do not take affect until the end of the current compensation
 * interval.
 *
 * @param[in] base RTC base pointer
 * @param[in] compensationValue - the value which is subtracted from the counter
 *                                valid range -128, +127
 * @param[in] compensationInterval Compensation interval at which the compensation value
 *                                is added to the prescaler register
 * @return    None
 */
static inline void RTC_SetTimeCompensation(RTC_Type *const base,
                                           int8_t compensationValue,
                                           uint8_t compensationInterval)
{
    uint32_t tmp = base->CMP;
    tmp &= ~(RTC_CMP_CMPV_MASK | RTC_CMP_CMPIV_MASK);
    tmp |= RTC_CMP_CMPV(compensationValue);
    tmp |= RTC_CMP_CMPIV(compensationInterval);
    base->CMP = tmp;
}

/*!
 * @brief Get TimeCompensation Value and Interval
 *
 * Returns current value used by the compensation logic for the present
 * second interval. Updated once a second if the CIC equals 0 with the
 * contents of the CMP field. If the CIC does not equal zero then it
 * is loaded with zero.
 *
 * @param[in] base RTC base pointer
 * @param[out] compensationValue -  Current value which is subtracted from the counter
 *                                  valid range -128, +127
 * @param[out] compensationInterval Current Compensation interval at which the compensation value
 *                                  is added to the prescaler register
 * @return    Current value used by the compensation logic for the present second interval
 */
static inline void RTC_GetCurrentTimeCompensation(const RTC_Type * base,
                                                  int8_t *compensationValue,
                                                  uint8_t *compensationInterval)
{
    uint32_t tmp;
    tmp = ((base->CMP & RTC_CMP_CMPVS_MASK) >> RTC_CMP_CMPVS_SHIFT);
    (*compensationValue) = (int8_t)tmp;
    (*compensationInterval) = (uint8_t) ((base->CMP & RTC_CMP_CMPIVS_MASK) >> RTC_CMP_CMPIVS_SHIFT);
}

/*****************************************************************************
 * Methods for RTC Control register
 ****************************************************************************/

/*!
 * @brief Select clock source for RTC prescaler
 *
 * When set, the RTC prescaler increments using the LPO 1kHz
 * clock and not the RTC 32kHz crystal clock. The LPO increments
 * the prescaler from bit TPR[5] (TPR[4:0] are ignored),
 * supporting close to 1 second increment of the seconds register.
 *
 * @param[in] base RTC base pointer
 * @param[in] clk_select clock source
 */
static inline void RTC_SetClockSource(RTC_Type *const base, rtc_clk_source_t clk_select)
{
    uint32_t tmp = base->CTRL;
    tmp &= ~(RTC_CTRL_CLKSEL_MASK);
    tmp |= RTC_CTRL_CLKSEL(clk_select);
    base->CTRL = tmp;
}

/*!
 * @brief Set Non-Supervisor access mode
 *
 * @param[in] base RTC base pointer
 * @param[in] enable supervisor access
 *          - if true Non-supervisor mode write accesses are supported.
 *          - if false Non-supervisor mode write accesses are not supported and
 *            generate a bus error.
 */
static inline void RTC_SetDebugMode(RTC_Type *const base, bool enable)
{
    uint32_t tmp = base->CTRL;
    tmp &= ~(RTC_CTRL_DBGDIS_MASK);
    tmp |= RTC_CTRL_DBGDIS(enable ? 0UL : 1UL);
    base->CTRL = tmp;
}

/*!
 * @brief Trigger a software reset
 *
 * @param[in] base RTC base pointer
 */
static inline void RTC_SoftwareReset(RTC_Type *const base)
{
    base->CTRL |= RTC_CTRL_SWRST_MASK;
    base->CTRL &= ~RTC_CTRL_SWRST_MASK;

}

static inline void RTC_ClearIntEnable(RTC_Type *const base)
{
    base->INTE &= ~(RTC_INTE_ALMIE_MASK | RTC_INTE_OVFIE_MASK | RTC_INTE_SECIE_MASK);
}

/*****************************************************************************
 * Methods for RTC Status register
 ****************************************************************************/

/*!
 * @brief Enable or disable the Time counter
 *
 * When time counter is disabled the SEC register and TPR register are
 * writable, but do not increment.
 * When time counter is enabled the SEC register and TPR register are
 * not writable, but increment.
 *
 * @param[in] base RTC base pointer
 * @param[in] enable :
 *            - true to enable the counter
 *            - false to disable the counter
 */
static inline void RTC_SetTimeCounterEnable(RTC_Type *const base, bool enable)
{
    uint32_t tmp = base->EN;
    tmp &= ~(RTC_EN_EN_MASK);
    tmp |= RTC_EN_EN(enable ? 1UL : 0UL);
    base->EN = tmp;
}

/*!
 * @brief Get the Time Counter Enable value
 *
 * @param[in] base RTC base pointer
 * @return    State of the counter enable bit
 *      -     true if the counter is enabled
 *      -     false if the counter is disabled
 */
static inline bool RTC_GetTimeCounterEnable(const RTC_Type * base)
{
    uint32_t tmp = base->EN;
    tmp = (tmp & RTC_EN_EN_MASK) >> RTC_EN_EN_SHIFT;
    return ((tmp == 1U) ? true : false);
}

/*!
 * @brief Get the Time alarm flag
 *
 * The alarm flag is cleared after a write in Time Alarm Register
 *
 * @param[in] base RTC base pointer
 * @return :  State of the alarm flag
 *      -     true if an alarm occurred
 *      -     false if an alarm was not occurred
 */
static inline bool RTC_GetTimeAlarmFlag(const RTC_Type * base)
{
    uint32_t tmp = base->INTF;
    tmp = (tmp & RTC_INTF_ALMIF_MASK) >> RTC_INTF_ALMIF_SHIFT;
    return ((tmp == 1U) ? true : false);
}

/*!
 * @brief Get Time Overflow Flag
 *
 * The OVFIF is set when Time Seconds Register overflows. Disable the
 * counter and write SEC to clear this bit
 *
 * @param[in] base RTC base pointer
 * @return    State of the Time overflow flag
 *      -     true if an overflow has occurred
 *      -     false if an overflow has not occurred
 */
static inline bool RTC_GetTimeOverflowFlag(const RTC_Type * base)
{
    uint32_t tmp = base->INTF;
    tmp = (tmp & RTC_INTF_OVFIF_MASK) >> RTC_INTF_OVFIF_SHIFT;
    return ((tmp == 1U) ? true : false);
}

/*!
 * @brief Get the Enable Register Lock state
 *
 * @param[in] base RTC base pointer
 * @return    State of the Enable register lock
 *      -     true if register is locked
 *      -     false if the register is unlocked
 */
static inline bool RTC_EnableRegisterLock(const RTC_Type * base)
{
    uint32_t tmp = base->CTRL;
    tmp = (tmp & RTC_CTRL_UNLOCK_MASK) >> RTC_CTRL_UNLOCK_SHIFT;
    return ((tmp == 1U) ? false : true);
}


/*!
 * @brief Unlock the Enable Register
 *
 * This method locks the Control Register.
 *
 * @param[in] base RTC base pointer
 */
static inline void RTC_EnableRegisterUnlock(RTC_Type *const base)
{
    base->CTRL |= RTC_CTRL_UNLOCK_MASK;
}


/*****************************************************************************
 * Methods for RTC Interrupt Enable register
 ****************************************************************************/

/*!
 * @brief Configure Time Seconds interrupt
 *
 * @param[in] base RTC base pointer
 * @param[in] intCfg Select at which frequency the interrupt
 *                   will occur.
 */
static inline void RTC_SetTimeSecondsIntConf(RTC_Type *const base, rtc_second_int_cfg_t intCfg)
{
    uint32_t tmp = base->INTE;
    tmp &= ~(RTC_INTE_SECOCS_MASK);
    tmp |= RTC_INTE_SECOCS((uint8_t) intCfg);
    base->INTE = tmp;
}

/*!
 * @brief Enable TimeSeconds interrupt
 *
 * @param[in] base RTC base pointer
 * @param[in] enable Write:
 *      -     true to enable the interrupt
 *      -     false to disable it
 */
static inline void RTC_SetTimeSecondsIntEnable(RTC_Type *const base, bool enable)
{
    uint32_t tmp = base->INTE;
    tmp &= ~(RTC_INTE_SECIE_MASK);
    tmp |= RTC_INTE_SECIE(enable ? 1UL : 0UL);
    base->INTE = tmp;
}

/*!
 * @brief Clear TimeSeconds interrupt flag
 *
 * @param[in] base RTC base pointer
 */
static inline void RTC_ClearTimeSecondsIntFlag(RTC_Type *const base)
{
    base->INTF = RTC_INTF_SECIF_MASK;
    base->INTF = 0;
}

static inline void RTC_ClearTimeOverflowIntFlag(RTC_Type *const base)
{
    base->INTF &= ~(RTC_INTF_OVFIF_MASK | RTC_INTF_SECIF_MASK);
}

static inline void RTC_ClearTimeAlarmIntFlag(RTC_Type *const base)
{
    base->INTF &= ~(RTC_INTF_ALMIF_MASK | RTC_INTF_SECIF_MASK);
}

/*!
 * @brief Enable TimeAlarm interrupt
 *
 * @param[in] base RTC base pointer
 * @param[in] enable Write
 *      -     true to enable the interrupt
 *      -     false to disable it
 */
static inline void RTC_SetTimeAlarmIntEnable(RTC_Type *const base, bool enable)
{
    uint32_t tmp = base->INTE;
    tmp &= ~(RTC_INTE_ALMIE_MASK);
    tmp |= RTC_INTE_ALMIE(enable ? 1UL : 0UL);
    base->INTE = tmp;
}

/*!
 * @brief Enable TimeOverflow interrupt
 *
 * @param[in] base RTC base pointer
 * @param[in] enable Write
 *          - true to enable the interrupt
 *          - false to disable it
 */
static inline void RTC_SetTimeOverflowIntEnable(RTC_Type *const base, bool enable)
{
    uint32_t tmp = base->INTE;
    tmp &= ~(RTC_INTE_OVFIE_MASK);
    tmp |= RTC_INTE_OVFIE(enable ? 1UL : 0UL);
    base->INTE = tmp;
}


/*! @}*/

#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* RTC_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
