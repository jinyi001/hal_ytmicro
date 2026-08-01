/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file rtc_hw_access.h
 * @version 1.4.1
 *
 * @brief RTC Hardware Access Layer.
 *
 * This header provides the low-level register access helpers used by the RTC
 * driver implementation. It exposes a mix of non-inline functions and
 * `static inline` register accessors organized into the following categories:
 *   - Counter enable/disable and RTC initialization helpers.
 *   - Seconds and alarm register access.
 *   - Compensation and control-register programming.
 *   - Status-flag and interrupt-control helpers.
 *
 * @note This is an internal layer used by rtc_driver.c. Application code
 *       should use the RTC_DRV_* APIs declared in rtc_driver.h.
 */

#ifndef RTC_HW_ACCESS_H
#define RTC_HW_ACCESS_H

#include <stdbool.h>
#include <stdint.h>
#include "device_registers.h"
#include "status.h"
#include "rtc_driver.h"

/*!
 * @addtogroup rtc_hw_access RTC Hardware Access
 * @ingroup rtc
 * @brief Low-level register access functions for the RTC peripheral.
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Initialization & Counter Control
 ******************************************************************************/
/*!
 * @name Initialization & Counter Control
 * @brief Functions for enabling, disabling, and applying top-level RTC control.
 * @{
 */

/*!
 * @brief Enable the RTC time counter.
 *
 * @param[in] base  Pointer to the RTC peripheral base address.
 * @return Execution status.
 * @retval STATUS_SUCCESS Counter enable request completed.
 * @retval STATUS_ERROR   Counter was already enabled.
 */
status_t RTC_Enable(RTC_Type *const base);

/*!
 * @brief Disable the RTC time counter.
 *
 * @param[in] base  Pointer to the RTC peripheral base address.
 * @return Execution status.
 * @retval STATUS_SUCCESS Counter disable request completed.
 * @retval STATUS_ERROR   Counter remained enabled after the disable request.
 */
status_t RTC_Disable(RTC_Type *const base);

/*!
 * @brief Configure the RTC clock-output pin source.
 *
 * @param[in] base    Pointer to the RTC peripheral base address.
 * @param[in] config  Clock-output source selection.
 * @return Execution status.
 * @retval STATUS_SUCCESS Clock-output selection was written.
 */
status_t RTC_ConfigureClockOut(RTC_Type *const base, rtc_clk_out_config_t config);

/*! @} */ /* End of Initialization & Counter Control */

/*******************************************************************************
 * Time Seconds Register Access
 ******************************************************************************/
/*!
 * @name Time Seconds Register Access
 * @brief Helpers for reading or programming the RTC seconds counter.
 * @{
 */

/*!
 * @brief Read the current RTC seconds register value.
 *
 * @param[in] base  Pointer to the RTC peripheral base address.
 * @return Current RTC seconds count.
 */
static inline uint32_t RTC_GetTimeSecondsRegister(const RTC_Type *base)
{
    uint32_t tmp = base->SEC;
    tmp = (tmp & RTC_SEC_SEC_MASK) >> RTC_SEC_SEC_SHIFT;
    return (uint32_t)(tmp);
}

/*!
 * @brief Program the RTC seconds register.
 *
 * @param[in] base     Pointer to the RTC peripheral base address.
 * @param[in] seconds  Seconds value to write into the RTC SEC register.
 * @return Execution status.
 * @retval STATUS_SUCCESS Seconds register updated successfully.
 * @retval STATUS_ERROR   The counter was running, so the write was rejected.
 */
status_t RTC_SetTimeSecondsRegister(RTC_Type *const base, uint32_t seconds);

/*! @} */ /* End of Time Seconds Register Access */

/*******************************************************************************
 * Alarm Register Access
 ******************************************************************************/
/*!
 * @name Alarm Register Access
 * @brief Helpers for reading or programming the RTC alarm register.
 * @{
 */

/*!
 * @brief Read the current RTC alarm register value.
 *
 * @param[in] base  Pointer to the RTC peripheral base address.
 * @return Alarm value in RTC seconds units.
 */
static inline uint32_t RTC_GetTimeAlarmRegister(const RTC_Type *base)
{
    uint32_t tmp = base->ALM;
    tmp = (tmp & RTC_ALM_ALM_MASK) >> RTC_ALM_ALM_SHIFT;
    return (uint32_t)(tmp);
}

/*!
 * @brief Program the RTC alarm register.
 *
 * @param[in] base     Pointer to the RTC peripheral base address.
 * @param[in] seconds  Alarm target expressed in RTC seconds units.
 *
 * @note This helper also clears the hardware update flag after writing the
 *       new alarm value.
 */
static inline void RTC_SetTimeAlarmRegister(RTC_Type *const base, uint32_t seconds)
{
    base->ALM = seconds;
    /* Clear the update flag after programming a new alarm value. */
    base->INTF = 0;
}

/*! @} */ /* End of Alarm Register Access */

/*******************************************************************************
 * Time Compensation
 ******************************************************************************/
/*!
 * @name Time Compensation
 * @brief Helpers for programming or reading RTC compensation values.
 * @{
 */

/*!
 * @brief Program the RTC compensation value and compensation interval.
 *
 * The compensation logic adjusts the effective prescaler period to trim RTC
 * drift. The supplied values are written directly into the RTC CMP register.
 *
 * @param[in] base                  Pointer to the RTC peripheral base address.
 * @param[in] compensationValue     Signed compensation value applied to the prescaler.
 * @param[in] compensationInterval  Compensation interval value written to CMPIV.
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
 * @brief Read the currently active RTC compensation value and interval.
 *
 * @param[in]  base                  Pointer to the RTC peripheral base address.
 * @param[out] compensationValue     Pointer that receives the active compensation value.
 * @param[out] compensationInterval  Pointer that receives the active compensation interval.
 */
static inline void RTC_GetCurrentTimeCompensation(const RTC_Type *base,
                                                  int8_t *compensationValue,
                                                  uint8_t *compensationInterval)
{
    uint32_t tmp;
    tmp = ((base->CMP & RTC_CMP_CMPVS_MASK) >> RTC_CMP_CMPVS_SHIFT);
    (*compensationValue) = (int8_t)tmp;
    (*compensationInterval) = (uint8_t)((base->CMP & RTC_CMP_CMPIVS_MASK) >> RTC_CMP_CMPIVS_SHIFT);
}

/*! @} */ /* End of Time Compensation */

/*******************************************************************************
 * Control Register Helpers
 ******************************************************************************/
/*!
 * @name Control Register Helpers
 * @brief Helpers for programming the RTC control and reset-related fields.
 * @{
 */

/*!
 * @brief Select the RTC clock source.
 *
 * @param[in] base        Pointer to the RTC peripheral base address.
 * @param[in] clk_select  RTC clock source value from @ref rtc_clk_source_t.
 */
static inline void RTC_SetClockSource(RTC_Type *const base, rtc_clk_source_t clk_select)
{
    uint32_t tmp = base->CTRL;
    tmp &= ~(RTC_CTRL_CLKSEL_MASK);
    tmp |= RTC_CTRL_CLKSEL(clk_select);
    base->CTRL = tmp;
}

/*!
 * @brief Configure RTC debug behavior.
 *
 * @param[in] base    Pointer to the RTC peripheral base address.
 * @param[in] enable  true to allow RTC debug operation, false to disable it.
 */
static inline void RTC_SetDebugMode(RTC_Type *const base, bool enable)
{
    uint32_t tmp = base->CTRL;
    tmp &= ~(RTC_CTRL_DBGDIS_MASK);
    tmp |= RTC_CTRL_DBGDIS(enable ? 0UL : 1UL);
    base->CTRL = tmp;
}

/*!
 * @brief Trigger an RTC software reset pulse.
 *
 * @param[in] base  Pointer to the RTC peripheral base address.
 */
static inline void RTC_SoftwareReset(RTC_Type *const base)
{
    base->CTRL |= RTC_CTRL_SWRST_MASK;
    base->CTRL &= ~RTC_CTRL_SWRST_MASK;
}

/*!
 * @brief Disable all RTC interrupt enable bits.
 *
 * @param[in] base  Pointer to the RTC peripheral base address.
 */
static inline void RTC_ClearIntEnable(RTC_Type *const base)
{
    base->INTE &= ~(RTC_INTE_ALMIE_MASK | RTC_INTE_OVFIE_MASK | RTC_INTE_SECIE_MASK);
}

/*! @} */ /* End of Control Register Helpers */

/*******************************************************************************
 * Counter State & Status Flags
 ******************************************************************************/
/*!
 * @name Counter State & Status Flags
 * @brief Helpers for reading or writing RTC enable bits and status flags.
 * @{
 */

/*!
 * @brief Write the RTC time-counter enable bit.
 *
 * @param[in] base    Pointer to the RTC peripheral base address.
 * @param[in] enable  true to enable counting, false to disable counting.
 */
static inline void RTC_SetTimeCounterEnable(RTC_Type *const base, bool enable)
{
    uint32_t tmp = base->EN;
    tmp &= ~(RTC_EN_EN_MASK);
    tmp |= RTC_EN_EN(enable ? 1UL : 0UL);
    base->EN = tmp;
}

/*!
 * @brief Read the RTC time-counter enable bit.
 *
 * @param[in] base  Pointer to the RTC peripheral base address.
 * @return true if the RTC counter is enabled, false otherwise.
 */
static inline bool RTC_GetTimeCounterEnable(const RTC_Type *base)
{
    uint32_t tmp = base->EN;
    tmp = (tmp & RTC_EN_EN_MASK) >> RTC_EN_EN_SHIFT;
    return ((tmp == 1U) ? true : false);
}

/*!
 * @brief Read the RTC alarm interrupt flag.
 *
 * @param[in] base  Pointer to the RTC peripheral base address.
 * @return true if the alarm flag is set, false otherwise.
 */
static inline bool RTC_GetTimeAlarmFlag(const RTC_Type *base)
{
    uint32_t tmp = base->INTF;
    tmp = (tmp & RTC_INTF_ALMIF_MASK) >> RTC_INTF_ALMIF_SHIFT;
    return ((tmp == 1U) ? true : false);
}

/*!
 * @brief Read the RTC overflow interrupt flag.
 *
 * @param[in] base  Pointer to the RTC peripheral base address.
 * @return true if the overflow flag is set, false otherwise.
 */
static inline bool RTC_GetTimeOverflowFlag(const RTC_Type *base)
{
    uint32_t tmp = base->INTF;
    tmp = (tmp & RTC_INTF_OVFIF_MASK) >> RTC_INTF_OVFIF_SHIFT;
    return ((tmp == 1U) ? true : false);
}

/*!
 * @brief Read the RTC register-unlock state.
 *
 * @param[in] base  Pointer to the RTC peripheral base address.
 * @return true if the register interface is locked, false if it is unlocked.
 */
static inline bool RTC_EnableRegisterLock(const RTC_Type *base)
{
    uint32_t tmp = base->CTRL;
    tmp = (tmp & RTC_CTRL_UNLOCK_MASK) >> RTC_CTRL_UNLOCK_SHIFT;
    return ((tmp == 1U) ? false : true);
}

/*!
 * @brief Unlock the RTC register interface.
 *
 * @param[in] base  Pointer to the RTC peripheral base address.
 */
static inline void RTC_EnableRegisterUnlock(RTC_Type *const base)
{
    base->CTRL |= RTC_CTRL_UNLOCK_MASK;
}

/*! @} */ /* End of Counter State & Status Flags */

/*******************************************************************************
 * Interrupt Control
 ******************************************************************************/
/*!
 * @name Interrupt Control
 * @brief Helpers for configuring RTC interrupt frequency, enable bits, and flags.
 * @{
 */

/*!
 * @brief Select the RTC periodic seconds interrupt frequency.
 *
 * @param[in] base    Pointer to the RTC peripheral base address.
 * @param[in] intCfg  Periodic interrupt frequency selection.
 */
static inline void RTC_SetTimeSecondsIntConf(RTC_Type *const base, rtc_second_int_cfg_t intCfg)
{
    uint32_t tmp = base->INTE;
    tmp &= ~(RTC_INTE_SECOCS_MASK);
    tmp |= RTC_INTE_SECOCS((uint8_t)intCfg);
    base->INTE = tmp;
}

/*!
 * @brief Enable or disable the RTC periodic seconds interrupt.
 *
 * @param[in] base    Pointer to the RTC peripheral base address.
 * @param[in] enable  true to enable the interrupt, false to disable it.
 */
static inline void RTC_SetTimeSecondsIntEnable(RTC_Type *const base, bool enable)
{
    uint32_t tmp = base->INTE;
    tmp &= ~(RTC_INTE_SECIE_MASK);
    tmp |= RTC_INTE_SECIE(enable ? 1UL : 0UL);
    base->INTE = tmp;
}

/*!
 * @brief Clear the RTC periodic seconds interrupt flag.
 *
 * @param[in] base  Pointer to the RTC peripheral base address.
 */
static inline void RTC_ClearTimeSecondsIntFlag(RTC_Type *const base)
{
    base->INTF = RTC_INTF_SECIF_MASK;
    base->INTF = 0;
}

/*!
 * @brief Clear the RTC overflow interrupt flag.
 *
 * @param[in] base  Pointer to the RTC peripheral base address.
 */
static inline void RTC_ClearTimeOverflowIntFlag(RTC_Type *const base)
{
    base->INTF &= ~(RTC_INTF_OVFIF_MASK | RTC_INTF_SECIF_MASK);
}

/*!
 * @brief Clear the RTC alarm interrupt flag.
 *
 * @param[in] base  Pointer to the RTC peripheral base address.
 */
static inline void RTC_ClearTimeAlarmIntFlag(RTC_Type *const base)
{
    base->INTF &= ~(RTC_INTF_ALMIF_MASK | RTC_INTF_SECIF_MASK);
}

/*!
 * @brief Enable or disable the RTC alarm interrupt source.
 *
 * @param[in] base    Pointer to the RTC peripheral base address.
 * @param[in] enable  true to enable the interrupt, false to disable it.
 */
static inline void RTC_SetTimeAlarmIntEnable(RTC_Type *const base, bool enable)
{
    uint32_t tmp = base->INTE;
    tmp &= ~(RTC_INTE_ALMIE_MASK);
    tmp |= RTC_INTE_ALMIE(enable ? 1UL : 0UL);
    base->INTE = tmp;
}

/*!
 * @brief Enable or disable the RTC overflow interrupt source.
 *
 * @param[in] base    Pointer to the RTC peripheral base address.
 * @param[in] enable  true to enable the interrupt, false to disable it.
 */
static inline void RTC_SetTimeOverflowIntEnable(RTC_Type *const base, bool enable)
{
    uint32_t tmp = base->INTE;
    tmp &= ~(RTC_INTE_OVFIE_MASK);
    tmp |= RTC_INTE_OVFIE(enable ? 1UL : 0UL);
    base->INTE = tmp;
}

/*! @} */ /* End of Interrupt Control */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of rtc_hw_access group */

#endif /* RTC_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
