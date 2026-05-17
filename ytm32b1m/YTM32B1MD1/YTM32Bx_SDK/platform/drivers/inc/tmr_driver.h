/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file tmr_driver.h
 * @version 1.4.1
 */

#ifndef TMR_DRIVER_H
#define TMR_DRIVER_H

#include "device_registers.h"
#include "status.h"

/*!
 * @defgroup tmr_drv System Timer Driver
 * @brief System Timer Driver(TMR)
 * @ingroup tmr
 * @{
 */

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if defined(FEATURE_TMR_HAS_CLOCK_SELECTION) && (FEATURE_TMR_HAS_CLOCK_SELECTION == 1U)
/*! @brief TMR clock source selection
 *
 * Implements : tmr_clock_source_t_Class
 */
typedef enum
{
    TMR_CLOCK_SYSTEM    = 0x0U,  /*!< Internal system clock */
    TMR_CLOCK_FXOSC     = 0x1U   /*!< Fast external crystal oscillator */
} tmr_clock_source_t;
#endif /* FEATURE_TMR_HAS_CLOCK_SELECTION */

/*! @brief TMR configuration structure
 *
 * Implements : tmr_config_t_Class
 */
typedef struct
{
#if defined(FEATURE_TMR_HAS_CLOCK_SELECTION) && (FEATURE_TMR_HAS_CLOCK_SELECTION == 1U)
    tmr_clock_source_t clockSource;  /*!< TMR clock source selection */
#endif /* FEATURE_TMR_HAS_CLOCK_SELECTION */
    uint8_t clockPrescaler;          /*!< Clock divide value for the prescaler */
    bool stopInDebugMode;            /*!< Allows the timer counter to be stopped in debug mode */
    uint32_t startValue;             /*!< Start-value for counter register */
} tmr_config_t;

/*! @brief TMR configuration structure for channel
 *
 * Implements : tmr_channel_config_t_Class
 */
typedef struct
{
    uint8_t channel;        /*!< The channel is selected */
    uint32_t compareValue;  /*!< Compare Value for channel */
#if defined(FEATURE_TMR_HAS_INTERRUPT_ENABLE_CONFIG) && (FEATURE_TMR_HAS_INTERRUPT_ENABLE_CONFIG == 1)  
    bool interruptEnable;  /*!< Enable/Disable Interrupt */
#endif
} tmr_channel_config_t;

/*******************************************************************************
 * API
 *******************************************************************************/
/*!
 * @name TMR Driver API
 * @{
 */
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initializes the TMR module.
 *
 * This function initializes TMR module base on the members of the tmr_config_t structure
 * with the desired values. Including clock source for module, prescaler, allow counter to
 * be stopped in debug mode and start-value for common counter register.
 *
 * This is an example demonstrating how to define a TMR configuration structure:
 * @code
 * tmr_config_t tmrInit =
 * {
 *      .clockSource = TMR_CLOCK_SYSTEM,
 *      .clockPrescaler = 0U,
 *      .stopInDebugMode = false,
 *      .startValue = 0U
 * };
 * @endcode
 *
 * @param[in] instance The TMR peripheral instance number.
 * @param[in] config Pointer to TMR configuration structure.
 */
void TMR_DRV_Init(const uint32_t instance, const tmr_config_t *const config);

/*!
 * @brief De-Initializes the TMR module
 *
 * This function resets all control registers and registers of each channel to default values
 * (Reference Manual Resets).
 * This function should only be called if user wants to stop all channels (not only one channel).
 * System clock is always enabled for TMR module, and doesn't have any option to disable clock.
 *
 * @param[in] instance The TMR peripheral instance number.
 */
void TMR_DRV_Deinit(const uint32_t instance);

/*!
 * @brief Gets the default configuration structure of TMR with default settings.
 *
 * This function initializes the hardware configuration structure to default values
 * (Reference Manual Resets).
 * This function should be called before configuring the hardware feature by TMR_DRV_Init() function,
 * otherwise all members be written by user.
 * This function insures that all members are written with safe values, but the user still can
 * modify the desired members.
 *
 * @param[out] config Pointer to TMR configuration structure.
 */
void TMR_DRV_GetDefaultConfig(tmr_config_t *const config);

/*!
 * @brief Initializes the TMR channel module with a structure.
 *
 * This function initializes TMR channel module base on the members of the tmr_channel_config_t
 * structure for each channel with the desired values. Including channel selected and compare-value
 * for that channel. This function is useful when using PEx tool.
 *
 * This is an example demonstrating how to define a TMR channel configuration structure:
 * @code
 * tmr_config_t tmrTestInit =
 * {
 *      .channel = 0U,
 *      .compareValue = 0U
 * };
 * @endcode
 *
 * @param[in] instance The TMR peripheral instance number.
 * @param[in] config Pointer to TMR channel configuration structure.
 */
void TMR_DRV_InitChannel(const uint32_t instance, const tmr_channel_config_t *const config);

/*!
 * @brief Configures the TMR channel module with parameters.
 *
 * This function initializes the desired settings for each channel.
 * This function is the same TMR_DRV_InitChannel() function about feature. But it is required
 * for user to have more options when configure the channel.
 *
 * @param[in] instance The TMR peripheral instance number.
 * @param[in] channel The channel number.
 * @param[in] compareValue The compare-value for channel selected.
 */
void TMR_DRV_ConfigChannel(const uint32_t instance, const uint8_t channel, const uint32_t compareValue);

/*!
 * @brief Enables the channel selected.
 *
 * This function enables channel selected. The feature in this function is contained
 * in TMR_DRV_InitChannel() also, so after calling that function then no need to call this
 * function for the first time. It is called when a channel is disable momentarily and
 *  user wants to enable channel again.
 *
 * @param[in] instance The TMR peripheral instance number.
 * @param[in] channel The channel number.
 */
void TMR_DRV_EnableChannel(const uint32_t instance, const uint8_t channel);

/*!
 * @brief Disables the channel selected.
 *
 * This function disables channel selected. There is no channel interrupt request is generated
 * after calling this function.
 *
 * @param[in] instance The TMR peripheral instance number.
 * @param[in] channel The channel number.
 */
void TMR_DRV_DisableChannel(const uint32_t instance, const uint8_t channel);

/*!
 * @brief Sets start-value for Counter register.
 *
 * This function sets start-value for common Counter register. There is only one counter
 * for all channels and the feature in this function is contained in TMR_DRV_Init() also,
 * after calling that function then no need to call this function for the first time.
 * It is called when user wants to set a new start-value to run again instead of calling
 * TMR_DRV_Init(), the action calls TMR_DRV_Init() will reduce performance of module.
 *
 * @param[in] instance The TMR peripheral instance number.
 * @param[in] startValue The start-value for counter register.
 */
void TMR_DRV_SetStartValueCount(const uint32_t instance, const uint32_t startValue);

/*!
 * @brief Starts timer counter.
 *
 * This function enables common Timer Counter and starts running.
 *
 * @param[in] instance The TMR peripheral instance number.
 */
void TMR_DRV_StartTimer(const uint32_t instance);

/*!
 * @brief Stops timer counter.
 *
 * This function disables common Timer Counter and stop counting.
 *
 * @param[in] instance The TMR peripheral instance number.
 */
void TMR_DRV_StopTimer(const uint32_t instance);

/*!
 * @brief Computes the number of ticks from microseconds.
 *
 * This function computes the number of ticks from microseconds.
 * The number of ticks depends on the frequency and counter prescaler of the TMR source clock.
 * User has to configure the frequency and counter prescaler by themself before calling this
 * function to have valid the number of ticks.
 *
 * @param[in] instance The TMR peripheral instance number.
 * @param[in] periodUs Time in microseconds.
 * @param[out] ticks The number of ticks after computing.
 * @return Operation status
 *         - STATUS_SUCCESS: Input microseconds is in of range of compare register
 *         - STATUS_ERROR: Input microseconds is out of range of compare register
 */
status_t TMR_DRV_ComputeTicksByUs(const uint32_t instance, const uint32_t periodUs, uint32_t *ticks);

/*!
 * @brief Increases the number of ticks in compare register.
 *
 * This function will compute the compare-value suitable and set that compare-value for compare
 * register to create a periodic event. To make sure about a periodic event, user should call
 * this function immediately after the event occurs.
 *
 * @param[in] instance The TMR peripheral instance number.
 * @param[in] channel The channel number.
 * @param[in] ticks The number of ticks.
 */
void TMR_DRV_IncrementTicks(const uint32_t instance, const uint8_t channel, const uint32_t ticks);

/*!
 * @brief Gets status of timer interrupt flag.
 *
 * This function returns the status of each channel selected. When common Counter Timer
 * is enabled and value in Counter Timer reaches to compare-value in Channel Compare register
 * then a channel interrupt request is generated.
 *
 * @param[in] instance The TMR peripheral instance number.
 * @param[in] channel The channel will be read flag.
 * @return The status of timer interrupt flag.
 */
uint32_t TMR_DRV_GetStatusFlags(const uint32_t instance, const uint8_t channel);

/*!
 * @brief Clears channel interrupt flag.
 *
 * This function will clear the flag of channel selected by writing a 1 to bit flag
 * which user wants to clear. All efforts write 0 to bit flag has no effect.
 *
 * @param[in] instance The TMR peripheral instance number.
 * @param[in] channel The channel will be cleared flag.
 */
void TMR_DRV_ClearStatusFlags(const uint32_t instance, const uint8_t channel);

/*!
 * @brief Gets current counter module.
 *
 * This function will get the counter value at the moment this function is called.
 *
 * @param[in] instance The TMR peripheral instance number.
 */
uint32_t TMR_DRV_GetCounterValue(const uint32_t instance);


/*! @} */
#if defined(__cplusplus)
}
#endif
/*! @} */

#endif /* TMR_DRIVER_H */
/*******************************************************************************
* EOF
*******************************************************************************/
