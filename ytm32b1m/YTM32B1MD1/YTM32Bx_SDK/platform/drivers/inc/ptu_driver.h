/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file ptu_driver.h
 * @version 1.4.1
 */

#ifndef PTU_DRIVER_H
#define PTU_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "clock_manager.h"


/*!
 * @addtogroup ptu_driver
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*!
 * @brief Defines the type of value load mode for the PTU module.
 *
 * Some timing related registers, such as the MOD, IDLY, CHnDLYm, INTx and POyDLY,
 * buffer the setting values. Only the load operation is triggered.
 * The setting value is loaded from a buffer and takes effect. There are
 * four loading modes to fit different applications.
 * Implements : ptu_load_value_mode_t_Class
 */
typedef enum
{
    PTU_LOAD_VAL_IMMEDIATELY = 0U,
    /*!<  Loaded immediately after load operation.  */
    PTU_LOAD_VAL_AT_MODULO_COUNTER = 1U,
    /*!< Loaded when counter hits the modulo after load operation.  */
    PTU_LOAD_VAL_AT_NEXT_TRIGGER = 2U,
    /*!< Loaded when detecting an input trigger after load operation.  */
    PTU_LOAD_VAL_AT_MODULO_COUNTER_OR_NEXT_TRIGGER = 3U
    /*!< Loaded when counter hits the modulo or detecting an input trigger after load operation.  */
} ptu_load_value_mode_t;

/*!
 * @brief Defines the type of prescaler divider for the PTU counter clock.
 * Implements : ptu_clk_prescaler_div_t_Class
 */
typedef enum
{
    PTU_CLK_PREDIV_BY_1 = 0U, /*!< Counting divided by 1 x prescaler multiplication factor (selected by MULT).  */
    PTU_CLK_PREDIV_BY_2 = 1U, /*!< Counting divided by 2 x prescaler multiplication factor (selected by MULT).  */
    PTU_CLK_PREDIV_BY_4 = 2U, /*!< Counting divided by 4 x prescaler multiplication factor (selected by MULT).  */
    PTU_CLK_PREDIV_BY_8 = 3U, /*!< Counting divided by 8 x prescaler multiplication factor (selected by MULT).  */
    PTU_CLK_PREDIV_BY_16 = 4U, /*!< Counting divided by 16 x prescaler multiplication factor (selected by MULT).  */
    PTU_CLK_PREDIV_BY_32 = 5U, /*!< Counting divided by 32 x prescaler multiplication factor (selected by MULT).  */
    PTU_CLK_PREDIV_BY_64 = 6U, /*!< Counting divided by 64 x prescaler multiplication factor (selected by MULT).  */
    PTU_CLK_PREDIV_BY_128 = 7U  /*!< Counting divided by 128 x prescaler multiplication factor (selected by MULT).  */
} ptu_clk_prescaler_div_t;

/*!
 * @brief Defines the type of trigger source mode for the PTU.
 *
 * Selects the trigger input source for the PTU. The trigger input source can
 * be internal or the software trigger.
 * Implements : ptu_trigger_src_t_Class
 */
typedef enum
{
    PTU_EXTERNAL_TRIGGER = 0U,    /*!< Select software trigger.  */
    PTU_SOFTWARE_TRIGGER = 1U,    /*!< Source trigger comes from TMU.  */
} ptu_trigger_src_t;

/*!
 * @brief Defines the type of the multiplication source mode for PTU.
 *
 * Selects the multiplication factor of the prescaler divider for the PTU counter clock.
 * Implements : ptu_clk_prescaler_mult_factor_t_Class
 */
typedef enum
{
    PTU_CLK_PREMULT_FACT_AS_1 = 0U, /*!< Multiplication factor is 1.  */
    PTU_CLK_PREMULT_FACT_AS_10 = 1U, /*!< Multiplication factor is 10.  */
    PTU_CLK_PREMULT_FACT_AS_20 = 2U, /*!< Multiplication factor is 20.  */
    PTU_CLK_PREMULT_FACT_AS_40 = 3U  /*!< Multiplication factor is 40.  */
} ptu_clk_prescaler_mult_factor_t;

/*!
 * @brief Defines the type of structure for basic timer in PTU.
 *
 * 
 * Implements : ptu_timer_config_t_Class
 */
typedef struct
{
    ptu_load_value_mode_t loadValueMode;            /*!< Select the load mode. */
    ptu_clk_prescaler_div_t clkPreDiv;                /*!< Select the prescaler divider. */
    ptu_clk_prescaler_mult_factor_t clkPreMultFactor;         /*!< Select multiplication factor for prescaler. */
    ptu_trigger_src_t triggerInput;             /*!< Select the trigger input source. */
    bool continuousModeEnable;     /*!< Enable the continuous mode. */
    bool dmaEnable;                /*!< Enable the dma for timer. */
    bool intEnable;                /*!< Enable the interrupt for timer. \note: interrupt is generated only if DMA is disabled. */
} ptu_timer_config_t;

/*!
 * @brief Defines the type of structure for configuring ADC's pre_trigger.
 * 
 * Implements : ptu_adc_pretrigger_config_t_Class
 */
typedef struct
{
    uint32_t adcPreTriggerIdx;       /*!< Setting pre_trigger's index. */
    bool preTriggerEnable;           /*!< Enable the pre_trigger. */
    bool preTriggerOutputEnable;     /*!< Enable the pre_trigger output. */
#if FEATURE_PTU_HAS_INTERCHANNEL_BACKTOBACK
    bool preTriggerBackToBackEnable; /*!< Enable the back to back mode for ADC pre_trigger.
                                          If enabled, the pretrigger will be activated automatically when the ADC COCO flag corresponding to the previous pretrigger in the chain, is set.
                                          The previous pretrigger for pretriggers with index 0, depend on features instanceBackToBackEnable and interchannelBackToBackEnable.  */
#endif
} ptu_adc_pretrigger_config_t;

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initializes the PTU counter and triggers input.
 *
 * This function initializes the PTU counter, input triggers and general pre-trigger settings.
 * It resets PTU registers and enables the PTU clock. Therefore, it should be
 * called before any other operation. After it is initialized, the PTU can
 * act as a triggered timer, which enables other features in PTU module.
 *
 * @param[in] instance PTU instance ID.
 * @param[in] userConfigPtr Pointer to the user configuration structure. See the "ptu_timer_config_t".
 */
void PTU_DRV_Init(const uint32_t instance,
                  const ptu_timer_config_t *const userConfigPtr);

/*!
 * @brief De-initializes the PTU module.
 *
 * This function de-initializes the PTU module.
 * Calling this function shuts down the PTU module and reduces the power consumption.
 *
 * @param[in] instance PTU instance ID.
 */
void PTU_DRV_Deinit(const uint32_t instance);

/*!
 * @brief Gets the default configuration structure of PTU with default settings.
 *
 * This function initializes the hardware configuration structure to default values
 * (Reference Manual Resets).
 * This function should be called before configuring the hardware feature by PTU_DRV_Init() function,
 * otherwise all members be written by user.
 * This function ensures that all members are written with safe values, but the user still can
 * modify the desired members.
 *
 * @param[out] config Pointer to PTU configuration structure.
 */
void PTU_DRV_GetDefaultConfig(ptu_timer_config_t *const config);

/*!
 * @brief Enables the PTU module.
 *
 * This function enables the PTU module, counter is on.
 *
 * @param[in] instance PTU instance ID.
 */
void PTU_DRV_Enable(const uint32_t instance);

/*!
 * @brief Disables the PTU module.
 *
 * This function disables the PTU module, counter is off also.
 *
 * @param[in] instance PTU instance ID.
 */
void PTU_DRV_Disable(const uint32_t instance);

/*!
 * @brief Triggers the PTU with a software trigger.
 *
 * This function triggers the PTU with a software trigger.
 * When the PTU is set to use the software trigger as input, calling this function
 * triggers the PTU.
 *
 * @param[in] instance PTU instance ID.
 */
void PTU_DRV_SoftTriggerCmd(const uint32_t instance);

/*!
 * @brief Gets the current value of the PTU counter.
 *
 * This function gets the current counter value.
 *
 * @param[in] instance PTU instance ID.
 * @return Current PTU counter value.
 */
uint32_t PTU_DRV_GetTimerValue(const uint32_t instance);

/*!
 * @brief Gets the PTU interrupt flag.
 *
 * This function gets the PTU interrupt flag. It is asserted if the PTU interrupt occurs.
 *
 * @param[in] instance PTU instance ID.
 * @return Assertion of indicated event.
 */
bool PTU_DRV_GetTimerIntFlag(const uint32_t instance);

/*!
 * @brief Clears the interrupt flag.
 *
 * This function clears the interrupt flag.
 *
 * @param[in] instance PTU instance ID.
 */
void PTU_DRV_ClearTimerIntFlag(const uint32_t instance);

/*!
 * @brief Executes the command of loading values.
 *
 * This function executes the command of loading values.
 *
 * @param[in] instance PTU instance ID.
 */
void PTU_DRV_LoadValuesCmd(const uint32_t instance);

/*!
 * @brief Sets the value of timer modulus.
 *
 * This function sets the value of timer modulus.
 *
 * @param[in] instance PTU instance ID.
 * @param[in] value Setting value.
 */
void PTU_DRV_SetTimerModulusValue(const uint32_t instance, const uint16_t value);

/*!
 * @brief Sets the value for the timer interrupt.
 *
 * This function sets the value for the timer interrupt.
 *
 * @param[in] instance PTU instance ID.
 * @param[in] value Setting value.
 */
void PTU_DRV_SetValueForTimerInterrupt(const uint32_t instance, const uint16_t value);

/*!
 * @brief Configures the ADC pre_trigger in the PTU module.
 *
 * This function configures the ADC pre_trigger in the PTU module.
 * Important note: any pre-trigger which is enabled and has the trigger output enabled (preTriggerOutputEnable
 * and preTriggerEnable both true) must have the corresponding delay value set to a non-zero
 * value by calling ::PTU_DRV_SetAdcPreTriggerDelayValue function.
 *
 * @param[in] instance PTU instance ID.
 * @param[in] configPtr Pointer to the user configuration structure. See ::ptu_adc_pretrigger_config_t.
 */
void PTU_DRV_ConfigAdcPreTrigger(const uint32_t instance,
                                 const ptu_adc_pretrigger_config_t *configPtr);

/*!
 * @brief Gets the ADC pre_trigger flag in the PTU module.
 *
 * This function gets the ADC pre_trigger flags in the PTU module.
 *
 * @param[in] instance PTU instance ID.
 * @param[in] preChnMask ADC pre_trigger channels mask.
 * @return Assertion of indicated flag.
 */
uint32_t PTU_DRV_GetAdcPreTriggerFlags(const uint32_t instance, const uint32_t preChnMask);

/*!
 * @brief Clears the ADC pre_trigger flag in the PTU module.
 *
 * This function clears the ADC pre_trigger flags in the PTU module.
 *
 * @param[in] instance PTU instance ID.
 * @param[in] preChnMask ADC pre_trigger channels mask.
 */
void PTU_DRV_ClearAdcPreTriggerFlags(const uint32_t instance, const uint32_t preChnMask);

/*!
 * @brief Gets the ADC pre_trigger flag in the PTU module.
 *
 * This function gets the ADC pre_trigger flags in the PTU module.
 *
 * @param[in] instance PTU instance ID.
 * @param[in] preChnMask ADC pre_trigger channels mask.
 * @return Assertion of indicated flag.
 */
uint32_t PTU_DRV_GetAdcPreTriggerSeqErrFlags(uint32_t instance, uint32_t preChnMask);

/*!
 * @brief Sets the ADC pre_trigger delay value in the PTU module.
 *
 * This function sets the ADC pre_trigger delay value in the PTU module.
 *
 *
 * @param instance PTU instance ID.
 * @param preChn ADC pre_channel.
 * @param value Setting value.
 */
void PTU_DRV_SetAdcPreTriggerDelayValue(const uint32_t instance, const uint32_t preChn, const uint32_t value);

/*!
 * @brief Switches on/off the CMP pulse out in the PTU module.
 *
 * This function switches the CMP pulse on/off in the PTU module.
 *
 * @param[in] instance PTU instance ID.
 * @param[in] enable Switcher to assert the feature.
 */
void PTU_DRV_SetCmpPulseOutEnable(const uint32_t instance, const bool enable);

/*!
 * @brief Sets the CMP pulse out delay value for high in the PTU module.
 *
 * This function sets the CMP pulse out delay value for high in the PTU module.
 *
 * @param[in] instance PTU instance ID.
 * @param[in] value Setting value.
 */
void PTU_DRV_SetCmpPulseOutDelayForHigh(const uint32_t instance, const uint16_t value);

/*!
 * @brief Sets the CMP pulse out delay value for low in the PTU module.
 *
 * This function sets the CMP pulse out delay value for low in the PTU module.
 *
 * @param[in] instance PTU instance ID.
 * @param[in] value Setting value.
 */
void PTU_DRV_SetCmpPulseOutDelayForLow(const uint32_t instance, const uint16_t value);

#if defined(__cplusplus)
}
#endif

/*!
 *@}
 */

#endif /* PTU_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
