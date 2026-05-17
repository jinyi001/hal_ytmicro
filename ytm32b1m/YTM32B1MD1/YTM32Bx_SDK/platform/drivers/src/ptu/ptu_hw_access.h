/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*!
 * @file ptu_hw_access.h
 * @version 1.4.1
 */

#ifndef PTU_HW_ACCESS_H
#define PTU_HW_ACCESS_H

#include <stddef.h>
#include "ptu_driver.h"

/*!
 * @addtogroup ptu_hw_access
 * @{
 */

/******************************************************************************
 * Definitions
 *****************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief Resets the PTU registers to a known state.
 *
 * This function resets the PTU registers to a known state. This state is
 * defined in a reference manual and is power on reset value.
 *
 * @param[in] base Register base address for the module.
 */
void PTU_Init(PTU_Type *const base);

/*!
 * @brief Configure the PTU timer.
 *
 * This function configure the PTU's basic timer.
 *
 * @param[in] base Register base address for the module.
 * @param[in] configPtr Pointer to configuration structure, see to "ptu_timer_config_t".
 */
void PTU_ConfigTimer(PTU_Type *const base,
                     const ptu_timer_config_t *const configPtr);

/*!
 * @brief Triggers the PTU by software if enabled.
 *
 * If enabled, this function triggers the PTU by using software.
 *
 * @param[in] base Register base address for the module.
 */
static inline void PTU_SetSoftTriggerCmd(PTU_Type *const base)
{
    DEV_ASSERT(base != NULL);
    REG_BIT_SET32(&(base->CTRL), PTU_CTRL_SWTRIG_MASK);
}

/*!
 * @brief Switches on to enable the PTU module.
 *
 * This function switches on to enable the PTU module.
 *
 * @param[in] base Register base address for the module.
 */
static inline void PTU_Enable(PTU_Type *const base)
{
    DEV_ASSERT(base != NULL);
    REG_BIT_SET32(&(base->CTRL), PTU_CTRL_EN_MASK);
}

/*!
 * @brief Switches to disable the PTU module.
 *
 * This function switches to disable the PTU module.
 *
 * @param[in] base Register base address for the module.
 */
static inline void PTU_Disable(PTU_Type *const base)
{
    DEV_ASSERT(base != NULL);
    REG_BIT_CLEAR32(&(base->CTRL), PTU_CTRL_EN_MASK);
}

/*!
 * @brief Gets the PTU delay interrupt flag.
 *
 * This function gets the PTU delay interrupt flag.
 *
 * @param[in] base Register base address for the module.
 * @return[in] Flat status, true if the flag is set.
 */
static inline bool PTU_GetTimerIntFlag(PTU_Type const *const base)
{
    DEV_ASSERT(base != NULL);

    return 1U == ((base->STS & PTU_STS_IF_MASK) >> PTU_STS_IF_SHIFT);
}

/*!
 * @brief Clears the PTU delay interrupt flag.
 *
 * This function clears PTU delay interrupt flag.
 *
 * @param[in] base Register base address for the module.
 */
static inline void PTU_ClearTimerIntFlag(PTU_Type *const base)
{
    DEV_ASSERT(base != NULL);
    volatile uint32_t dummy_read;
    REG_BIT_SET32(&(base->STS), PTU_STS_IF_MASK);/* W1C */

    /* This read-after-write guarantees that the write to clear operation is completed,
     * for the case when memory write buffering is enabled. */
    dummy_read = REG_READ32(&(base->STS));
    (void) dummy_read;
}

/*!
 * @brief Loads the delay registers value for the PTU module.
 *
 * This function sets the LDOK bit and loads the delay registers value.
 * Writing one  to this bit updates the internal registers MOD, IDLY, CHnDLYm and
 * POyDLY with the values written to their buffers. The MOD, IDLY,
 * CHnDLYm and POyDLY take effect according to the load mode settings.
 *
 * After one is written to the LDOK bit, the values in the buffers of above mentioned registers
 * are not effective and cannot be written until the values in the
 * buffers are loaded into their internal registers.
 * The LDOK can be written only when the the PTU is enabled or as alone with it. It is
 * automatically cleared either when the values in the buffers are loaded into the
 * internal registers or when the PTU is disabled.
 *
 * @param[in] base Register base address for the module.
 */
static inline void PTU_SetLoadValuesCmd(PTU_Type *const base)
{
    DEV_ASSERT(base != NULL);
    REG_BIT_SET32(&(base->CTRL), PTU_CTRL_LDOK_MASK);
}

/*!
 * @brief Sets the modulus value for the PTU module.
 *
 * This function sets the modulus value for the PTU module.
 * When the counter reaches the setting value, it is automatically reset to zero.
 * When in continuous mode, the counter begins to increase
 * again.
 *
 * @param[in] base Register base address for the module.
 * @param[in] value The setting value of upper limit for PTU counter.
 */
static inline void PTU_SetTimerModulusValue(PTU_Type *const base,
                                            uint16_t value)
{
    DEV_ASSERT(base != NULL);
    REG_RMW32(&(base->MOD), PTU_MOD_MOD_MASK, PTU_MOD_MOD(value));
}

/*!
 * @brief Gets the PTU counter value of PTU timer.
 *
 * This function gets the PTU counter value of PTU timer.
 *
 * @param[in] base Register base address for the module.
 * @return The current counter value.
 */
static inline uint32_t PTU_GetTimerValue(PTU_Type const *const base)
{
    DEV_ASSERT(base != NULL);

    return ((base->CNT & PTU_CNT_CNT_MASK) >> PTU_CNT_CNT_SHIFT);
}

/*!
 * @brief Sets the interrupt delay milestone of the PTU counter.
 *
 * This function sets the interrupt delay milestone of the PTU counter.
 * If enabled, a PTU interrupt is generated when the counter is equal to the
 * setting value.
 *
 * @param[in] base Register base address for the module.
 * @param[in] value The setting value for interrupt delay milestone of PTU counter.
 */
static inline void PTU_SetValueForTimerInterrupt(PTU_Type *const base,
                                                 uint16_t value)
{
    DEV_ASSERT(base != NULL);
    REG_RMW32(&(base->INTDLY), PTU_INTDLY_INTDLY_MASK, PTU_INTDLY_INTDLY(value));
}

/*!
 * @brief Switches to enable the pre-trigger back-to-back mode.
 *
 * This function switches to enable the pre-trigger back-to-back mode.
 *
 * @param[in] base Register base address for the module.
 * @param[in] chn PTU instance index for trigger.
 * @param[in] preChnMask ADC channel group index mask for trigger.
 * @param[in] enable Switcher to assert the feature.
 */
#if FEATURE_PTU_HAS_INSTANCE_BACKTOBACK
void PTU_SetAdcPreTriggerBackToBackEnable(PTU_Type * const base,
                                          uint32_t preChnMask,
                                          bool enable);
#endif

/*!
 * @brief Switches to enable the pre-trigger output.
 *
 * This function switches to enable pre-trigger output.
 *
 * @param[in] base Register base address for the module.
 * @param[in] chn PTU instance index for trigger.
 * @param[in] preChnMask ADC channel group index mask for trigger.
 * @param[in] enable Switcher to assert the feature.
 */
void PTU_SetAdcPreTriggerOutputEnable(PTU_Type *const base,
                                      uint32_t preChnMask,
                                      bool enable);

/*!
 * @brief Switches to enable the pre-trigger.
 *
 * This function switches to enable the pre-trigger.
 *
 * @param[in] base Register base address for the module.
 * @param[in] chn PTU instance index for trigger.
 * @param[in] preChnMask ADC channel group index mask for trigger.
 * @param[in] enable Switcher to assert the feature.
 */
void PTU_SetAdcPreTriggerEnable(PTU_Type *const base,
                                uint32_t preChnMask,
                                bool enable);

/*!
 * @brief Gets the flag which indicates whether the PTU counter has reached the pre-trigger delay value.
 *
 * This function gets the flag which indicates the PTU counter has reached the
 * pre-trigger delay value.
 *
 * @param[in] base Register base address for the module.
 * @param[in] chn PTU instance index for trigger.
 * @param[in] preChnMask ADC channel group index mask for trigger.
 * @return Flag mask. Indicated bit would be 1 if the event is asserted.
 */
static inline uint32_t PTU_GetAdcPreTriggerFlags(PTU_Type const *const base,
                                                 uint32_t preChnMask)
{
    DEV_ASSERT(base != NULL);

    return preChnMask & ((base->CHSTS & PTU_CHSTS_CHF_MASK) >> PTU_CHSTS_CHF_SHIFT);
}

/*!
 * @brief Clears the flag which indicates that the PTU counter has reached the pre-trigger delay value.
 *
 * This function clears the flag which indicates that the PTU counter has reached  the
 * pre-trigger delay value.
 *
 * @param[in] base Register base address for the module.
 * @param[in] chn PTU instance index for trigger.
 * @param[in] preChnMask ADC channel group index mask for trigger.
 */
void PTU_ClearAdcPreTriggerFlags(PTU_Type *const base,
                                 uint32_t preChnMask);

/*!
 * @brief Sets the pre-trigger delay value.
 *
 * This function sets the pre-trigger delay value.
 *
 * @param[in] base Register base address for the module.
 * @param[in] chn PTU instance index for trigger.
 * @param[in] preChn ADC channel group index for trigger.
 * @param[in] value Setting value for pre-trigger's delay value.
 */
static inline void PTU_SetAdcPreTriggerDelayValue(PTU_Type *const base,
                                                  uint32_t preChn,
                                                  uint32_t value)
{
    DEV_ASSERT(base != NULL);
    DEV_ASSERT(preChn < PTU_CHDLYn_COUNT);

    base->CHDLYn[preChn] = value;
}

/*!
 * @brief Switches to enable the pulse-out trigger.
 *
 * This function switches to enable the pulse-out trigger.
 *
 * @param[in] base Register base address for the module.
 * @param[in] pulseChnMask Pulse-out channle index mask for trigger.
 * @param[in] enable Switcher to assert the feature.
 */
void PTU_SetCmpPulseOutEnable(PTU_Type *const base,
                              bool enable);

/*!
 * @brief Sets the counter delay value for the pulse-out goes high.
 *
 * This function sets the counter delay value for the pulse-out goes high.
 *
 * @param[in] base Register base address for the module.
 * @param[in] pulseChn Pulse-out channel index for trigger.
 * @param[in] value Setting value for PTU delay .
 */
static inline void PTU_SetCmpPulseOutDelayForHigh(PTU_Type *const base,
                                                  uint16_t value)
{
    DEV_ASSERT(base != NULL);

    base->PODLY.PODLY0 = value;
}

/*!
 * @brief Sets the counter delay value for the pulse-out goes low.
 *
 * This function sets the counter delay value for the pulse-out goes low.
 *
 * @param[in] base Register base address for the module.
 * @param[in] pulseChn Pulse-out channel index for trigger.
 * @param[in] value Setting value for PTU delay .
 */
static inline void PTU_SetCmpPulseOutDelayForLow(PTU_Type *const base,
                                                 uint16_t value)
{
    DEV_ASSERT(base != NULL);

    base->PODLY.PODLY1 = value;
}

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */

#endif /* PTU_HW_ACCESS_H */

/******************************************************************************
 * EOF
 *****************************************************************************/
