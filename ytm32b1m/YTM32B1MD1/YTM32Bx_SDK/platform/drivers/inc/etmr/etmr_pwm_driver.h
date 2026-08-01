/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_pwm_driver.h
 * @version 1.4.1
 *
 * @brief eTMR PWM Driver — public API declarations and data types.
 *
 * This header declares the PWM mode API for the eTMR peripheral,
 * including independent and complementary PWM generation with
 * configurable alignment (edge/center/asymmetrical), dead-time
 * insertion, fault protection, and duty-cycle/period update at runtime.
 */

#ifndef eTMR_PWM_DRIVER_H
#define eTMR_PWM_DRIVER_H

#include "etmr_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief Maximum value for PWM duty cycle */
#define eTMR_MAX_DUTY_CYCLE      (0x8000U)
/*! @brief Shift value which converts duty to ticks */
#define eTMR_DUTY_TO_TICKS_SHIFT (15U)

/*!
 * @name PWM Type Definitions
 * @{
 */

/*!
 * @brief PWM channel output polarity.
 *
 */
typedef enum
{
    eTMR_POLARITY_NORMAL = 0x00U, /*!< The channel polarity is not inverted */
    eTMR_POLARITY_INVERT = 0x01U  /*!< The channel polarity is inverted */
} etmr_polarity_t;

/*!
 * @brief Fault mode selection.
 *
 * Determines whether the eTMR clock continues running during a fault.
 *
 */
typedef enum
{
    eTMR_FAULT_WITH_CLK = 0U,    /*!< Fault generated, but etmr clock is still existing */
    eTMR_FAULT_WITHOUT_CLK = 1U, /*!< Fault generated, and etmr clock is lost */
} etmr_fault_mode_t;

/*!
 * @brief PWM recovery opportunity after a fault event.
 *
 */
typedef enum
{
    eTMR_FAULT_PWM_RECOVERY_DISABLED = 0U,    /*!< not recovery */
    eTMR_FAULT_PWM_RECOVERY_WITH_MID = 1U,    /*!< recovery when counter matches with MID */
    eTMR_FAULT_PWM_RECOVERY_WITH_MOD = 2U,    /*!< recovery when counter matches with MOD */
    eTMR_FAULT_PWM_RECOVERY_IMMEDIATELY = 3U, /*!< recovery immediately */
} etmr_pwm_recovery_opportunity_t;

/*!
 * @brief PWM automatic recovery mode.
 *
 */
typedef enum
{
    eTMR_MANUAL_CLEAR_FAULT_FLAG_THEN_AUTO_RECOVERY = 0U, /*!< Recovery needs clearing fault flag first */
    eTMR_AUTO_RECOVERY_WITHOUT_CLEAR_FAULT_FLAG = 1U,     /*!< Recovery regardless of clearing fault flag */
    eTMR_AUTO_RECOVERY_WITH_CLEAR_FAULT_FLAG = 2U,        /*!< Recovery and clearing fault flag */
} etmr_pwm_recovery_auto_mode_t;

/*!
 * @brief Fault input polarity selection.
 *
 */
typedef enum
{
    eTMR_FAULT_SIGNAL_HIGH = 0U, /*!< Regard high level(1) input as fault signal */
    eTMR_FAULT_SIGNAL_LOW = 1U,  /*!< Regard low level(0) input as fault signal */
} etmr_fault_input_polarity_t;

/*!
 * @brief Channel safe-state output level during a fault condition.
 *
 */
typedef enum
{
    eTMR_LOW_STATE = 0x00U,  /*!< When fault is detected PWM channel is low. */
    eTMR_HIGH_STATE = 0x01U, /*!< When fault is detected PWM channel is high. */
    eTMR_TRI_STATE = 0x02U,  /*!< When fault is detected PWM channel is tristate. */
} etmr_safe_state_t;

/*!
 * @brief Per-fault-channel configuration.
 *
 */
typedef struct
{
    bool faultChannelEnabled;                       /*!< Fault channel enable or not */
    etmr_fault_input_polarity_t faultInputPolarity; /*!< Fault input polarity signal */
} etmr_fault_ch_param_t;

/*!
 * @brief Overall fault protection configuration.
 *
 */
typedef struct
{
    bool pwmFaultInterrupt;                                                   /*!< PWM fault interrupt state */
    uint8_t faultFilterSampleCounter;                                         /*!< Fault filter sample counter */
    uint8_t faultFilterSamplePeriod;                                          /*!< Fault filter sample period */
    uint8_t faultInputStrentch;                                               /*!< Fault input strentch */
    etmr_pwm_recovery_opportunity_t pwmRecoveryOpportunity;                   /*!< PWM Recovery opportunity */
    etmr_pwm_recovery_auto_mode_t pwmAutoRecoveryMode;                        /*!< PWM Recovery automatic mode */
    etmr_fault_mode_t faultMode;                                              /*!< Fault mode */
    etmr_fault_ch_param_t etmrFaultChannelParam[FEATURE_eTMR_FAULT_CHANNELS]; /*!< Fault channels configuration */
    etmr_safe_state_t safeState[FEATURE_eTMR_CHANNEL_MAX_COUNT];              /*!< Channel safe state after fault */
} etmr_fault_param_t;

/*!
 * @brief Per-channel PWM configuration.
 *
 */
typedef struct
{
    uint8_t hwChannelId;                   /*!< Physical hardware channel ID */
    etmr_polarity_t polarity;              /*!< Polarity of the complementary even channel signal */
    bool pwmSrcInvert;                     /*!< Polarity of the channel PWM, not includes output initial */
    etmr_pwm_align_mode_t align;           /*!< PWM alignment mode */
    uint8_t channelInitVal;                /*!< Channel output initial value */
    etmr_pwm_update_option_t typeOfUpdate; /*!< Type of update */
    uint32_t dutyCycle;                    /*!< PWM duty cycle */
    uint32_t offset;                       /*!< offset from 0 to the rising edge of pwm for asymmetrical mode */
    bool enableSecondChannelOutput;        /*!< Enable complementary mode on next channel */
    etmr_polarity_t secondChannelPolarity; /*!< Polarity of the complementary odd channel signal */
#if defined(FEATURE_eTMR_HAS_DOUBLE_SWITCH) && \
           (FEATURE_eTMR_HAS_DOUBLE_SWITCH == 1U)
    bool enableDoubleSwitch; /*!< Enable double switch mode */
#endif
#if defined(FEATURE_eTMR_DEADTIME_CONFIG_EACH_CHANNEL) && \
           (FEATURE_eTMR_DEADTIME_CONFIG_EACH_CHANNEL == 1U)
    uint16_t evenDeadTime; /*!< Enable/disable dead time for channel 0,2,4,6 */
    uint16_t oddDeadTime;  /*!< Enable/disable dead time for channel 1,3,5,7 */
#endif
#if defined(FEATURE_eTMR_HAS_CHANNEL_VALUE_MATCH_INTERRUPT_IN_PWM_MODE) && \
           (FEATURE_eTMR_HAS_CHANNEL_VALUE_MATCH_INTERRUPT_IN_PWM_MODE == 1U)
    bool interruptEnable; /*!< Interrupt enable or not */
#endif
} etmr_pwm_ch_param_t;

#if defined(FEATURE_eTMR_DEADTIME_CONFIG_EACH_CHANNEL) && (FEATURE_eTMR_DEADTIME_CONFIG_EACH_CHANNEL == 0U)
typedef struct
{
    uint8_t prs;           /*!< Prescaler value */
    uint16_t evenDeadTime; /*!< Enable/disable dead time for channel 0,2,4,6 */
    uint16_t oddDeadTime;  /*!< Enable/disable dead time for channel 1,3,5,7 */
} etmr_deadtime_param_t;
#endif

/*!
 * @brief Overall PWM mode configuration.
 *
 */
typedef struct
{
    uint8_t nNumPwmChannels;  /*!< Number of independent PWM channels */
    etmr_channel_mode_t mode; /*!< eTMR pwm mode */
    uint32_t uFrequencyHZ;    /*!< PWM period in Hz */
#if defined(FEATURE_eTMR_HAS_CNT_INIT_SRC) && (FEATURE_eTMR_HAS_CNT_INIT_SRC == 1U)
    bool counterInitValFromInitReg; /*!< Counter initial value source from INIT register or not */
#endif
    uint32_t cntVal;                       /*!< Counter initial value from CNT register */
    etmr_pwm_ch_param_t *pwmChannelConfig; /*!< Configuration for independent PWM channels */
#if defined(FEATURE_eTMR_DEADTIME_CONFIG_EACH_CHANNEL) && (FEATURE_eTMR_DEADTIME_CONFIG_EACH_CHANNEL == 0U)
    etmr_deadtime_param_t *deadtimeConfig; /*!< Configuration for PWM dead time */
#endif
    etmr_fault_param_t *faultConfig; /*!< Configuration for PWM fault */
} etmr_pwm_param_t;

/*! @} */

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name PWM Initialization
 * @{
 */

/*!
 * @brief Initialize counter, period, duty cycle and fault for PWM mode.
 *
 * This is the main PWM initialization function. It configures the counter
 * frequency, initializes duty cycles for all channels, sets up channel
 * modes (independent/complementary), and optionally configures fault
 * protection. Call eTMR_DRV_Init() before this function.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] param    Pointer to PWM configuration parameters.
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_InitPwm(uint32_t instance, const etmr_pwm_param_t *param);

/*!
 * @brief Stop all PWM channels and reset PWM configuration.
 *
 * Disables the counter, resets all channel modes and dead-time
 * settings, and clears MOD/MID registers.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_DeinitPwm(uint32_t instance);

/*!
 * @brief Initialize PWM channel output modes and polarity.
 *
 * Configures channel initial output values, complementary/independent
 * mode, dead-time, double-switch, polarity, and PWM source inversion
 * for each channel in the configuration.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] param    Pointer to PWM configuration parameters.
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 */
status_t eTMR_DRV_InitPwmChannel(uint32_t instance, const etmr_pwm_param_t *param);

/*!
 * @brief Initialize duty cycles for all configured PWM channels.
 *
 * Calculates and writes VAL0/VAL1 compare values for each channel
 * based on the configured duty cycle, alignment mode, and offset.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] param    Pointer to PWM configuration parameters.
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_InitPwmDutyCycleChannel(uint32_t instance, const etmr_pwm_param_t *param);

/*! @} */

/*!
 * @name Fault Configuration
 * @{
 */

/*!
 * @brief Configure fault protection settings.
 *
 * Sets fault channel enables, polarities, filter parameters, safe-state
 * output values, recovery mode, and fault interrupt enable.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] param    Pointer to fault configuration parameters.
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_InitFault(uint32_t instance, const etmr_fault_param_t *param);

/*! @} */

/*!
 * @name PWM Update
 * @{
 */

/*!
 * @brief Update duty cycle and phase offset for a single PWM channel.
 *
 * Recalculates and writes VAL0/VAL1 compare values for the specified
 * channel based on the current period and the new duty/offset values.
 *
 * @param[in] instance  The eTMR peripheral instance number.
 * @param[in] channel   The channel number.
 * @param[in] dutyCycle New duty cycle value.
 * @param[in] offset    Offset from counter initial value to the first
 *                      rising edge (asymmetrical alignment only; forced
 *                      to 0 for edge/center alignment).
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_UpdatePwmChannel(uint32_t instance, uint8_t channel, uint32_t dutyCycle, uint32_t offset);

/*!
 * @brief Update PWM period (frequency or ticks).
 *
 * Changes the MOD value and recalculates duty cycles for all channels
 * to maintain their current duty-cycle ratio. For frequency-based
 * updates the new period is converted from Hz to ticks internally.
 *
 * @param[in] instance  The eTMR peripheral instance number.
 * @param[in] unit      Period unit (Hz or ticks).
 * @param[in] newPeriod New period value (1–0xFFFF for ticks, or Hz).
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_UpdatePwmPeriod(uint32_t instance, etmr_pwm_period_unit_t unit, uint32_t newPeriod);

/*!
 * @brief Update PWM period and duty cycle simultaneously in ticks.
 *
 * A convenience function that sets a new MOD value and new VAL0/VAL1
 * for a single channel in one call, all specified in ticks.
 *
 * @param[in] instance       The eTMR peripheral instance number.
 * @param[in] channel        The channel index.
 * @param[in] periodTicks    New period in ticks.
 * @param[in] dutyCycleTicks New duty cycle in ticks.
 * @param[in] offset         Offset in ticks for asymmetrical PWM.
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_UpdatePwmPeriodAndDuty(uint32_t instance,
                                         uint8_t channel,
                                         uint32_t periodTicks,
                                         uint32_t dutyCycleTicks,
                                         uint32_t offset);

/*! @} */

/*!
 * @name Trigger & Sync
 * @{
 */

/*!
 * @brief Issue a software trigger to synchronize buffered registers.
 *
 * Generates a software trigger that loads the shadow values of
 * INIT, CHMASK, MOD, MID, CHx_VAL0 and CHx_VAL1 into active registers.
 *
 * @param[in] instance The eTMR peripheral instance number.
 */
void eTMR_DRV_SyncWithSoftTrigger(uint32_t instance);

/*! @} */

#if defined(__cplusplus)
}
#endif

#endif /* eTMR_PWM_DRIVER_H */

/*! @}*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
