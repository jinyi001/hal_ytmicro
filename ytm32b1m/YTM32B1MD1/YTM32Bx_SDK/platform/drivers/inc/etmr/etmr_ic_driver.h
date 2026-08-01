/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_ic_driver.h
 * @version 1.4.1
 *
 * @brief eTMR Input Capture Driver — public API declarations and data types.
 *
 * This header declares the input capture (IC) mode API for the eTMR
 * peripheral, including edge detection, pulse-width measurement,
 * period measurement, and optional combination capture mode.
 */

#ifndef eTMR_IC_DRIVER_H
#define eTMR_IC_DRIVER_H

#include "etmr_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @name IC Type Definitions
 * @{
 */

#if FEATURE_eTMR_HAS_COMBINATION_CAPTURE
/*!
 * @brief Input capture combination source selection.
 *
 * Determines which channel of a pair provides the combination
 * capture trigger.
 *
 */
typedef enum
{
    eTMR_COMB_SRC_EVEN, /*!< Combination source as even channel */
    eTMR_COMB_SRC_ODD,  /*!< Combination source as odd channel */
} etmr_comb_src_t;

/*!
 * @brief Input capture combination pair parameters.
 *
 * Configures a channel pair for combination capture mode, where
 * two adjacent channels share a single input with separate edge
 * detection.
 *
 */
typedef struct
{
    uint8_t hwChannelPairId; /*!< Physical hardware channel pair ID */
    bool combEn;             /*!< Combination enable or not */
    etmr_comb_src_t combSrc; /*!< Combination source */
} etmr_ic_ch_pair_param_t;
#endif

/*!
 * @brief Per-channel input capture configuration.
 *
 * Specifies the capture edge, measurement type, input filter settings,
 * interrupt/DMA enables, and user callback for a single IC channel.
 *
 */
typedef struct
{
    uint8_t hwChannelId;                        /*!< Physical hardware channel ID*/
    etmr_ic_capture_edge_t edge;                /*!< Capture edge */
    etmr_ic_measurement_type_t measurementType; /*!< Measurement type */
#if FEATURE_eTMR_HAS_CAPTURE_HOLD
    bool capHoldEn; /*!< Capture hold enable or not */
#endif
    uint8_t filterSampleCounter; /*!< Filter Sample Counter */
    uint8_t filterSamplePeriod;  /*!< Filter Sample Period */
    bool interruptEnable; /*!< Interrupt enable or not */
    bool dmaEnable;                  /*!< DMA enable or not */
    bool enableNotification;         /*!< If enable notification */
    ic_callback_t channelsCallbacks; /*!< The callback function for channels events */
    void *channelsCallbacksParams;   /*!< The parameters of callback functions for channels events */
} etmr_ic_ch_param_t;

/*!
 * @brief Overall input capture configuration.
 *
 * Groups the number of channels, counter period, per-channel configs,
 * and optional combination-capture pair configs.
 *
 */
typedef struct
{
    uint8_t numChannels;               /*!< Number of input capture channel used */
    uint32_t countValue;               /*!< Maximum counter value. Minimum value is 1 for this mode */
    etmr_ic_ch_param_t *inputChConfig; /*!< Input capture channels configuration */
#if FEATURE_eTMR_HAS_COMBINATION_CAPTURE
    uint8_t numChannelPairs;                    /*!< Number of input capture channel pair used */
    etmr_ic_ch_pair_param_t *inputChPairConfig; /*!< Input capture channels pair configuration */
#endif
} etmr_ic_param_t;

/*! @} */

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name IRQ Handlers
 * @{
 */

#if defined(CPU_YTM32B1LE0) || defined(CPU_YTM32B1LE1) || defined(CPU_YTM32Z1LS0)

void eTMR0_CH_0_7_IRQHandler(void);
#if eTMR_INSTANCE_COUNT > 1U
void eTMR1_IRQHandler(void);
#endif
#if eTMR_INSTANCE_COUNT > 2U
void eTMR2_IRQHandler(void);
#endif

#else // CPU_YTM32B1MC0, CPU_YTM32B1MD1, CPU_YTM32B1ME0, CPU_YTM32B1HA0

void eTMR0_Ch0_Ch1_IRQHandler(void);
void eTMR0_Ch2_Ch3_IRQHandler(void);
void eTMR0_Ch4_Ch5_IRQHandler(void);
void eTMR0_Ch6_Ch7_IRQHandler(void);
void eTMR1_Ch0_Ch1_IRQHandler(void);
void eTMR1_Ch2_Ch3_IRQHandler(void);
void eTMR1_Ch4_Ch5_IRQHandler(void);
void eTMR1_Ch6_Ch7_IRQHandler(void);
#if eTMR_INSTANCE_COUNT > 2U
void eTMR2_Ch0_Ch1_IRQHandler(void);
void eTMR2_Ch2_Ch3_IRQHandler(void);
void eTMR2_Ch4_Ch5_IRQHandler(void);
void eTMR2_Ch6_Ch7_IRQHandler(void);
#endif
#if eTMR_INSTANCE_COUNT > 3U
void eTMR3_Ch0_Ch1_IRQHandler(void);
void eTMR3_Ch2_Ch3_IRQHandler(void);
void eTMR3_Ch4_Ch5_IRQHandler(void);
void eTMR3_Ch6_Ch7_IRQHandler(void);
#endif
#if eTMR_INSTANCE_COUNT > 4U
void eTMR4_Ch0_Ch1_IRQHandler(void);
void eTMR4_Ch2_Ch3_IRQHandler(void);
void eTMR4_Ch4_Ch5_IRQHandler(void);
void eTMR4_Ch6_Ch7_IRQHandler(void);
#endif
#if eTMR_INSTANCE_COUNT > 5U
void eTMR5_Ch0_Ch1_IRQHandler(void);
void eTMR5_Ch2_Ch3_IRQHandler(void);
void eTMR5_Ch4_Ch5_IRQHandler(void);
void eTMR5_Ch6_Ch7_IRQHandler(void);
#endif

#endif /* CPU_<device> */

/*! @} */

/*!
 * @name IC Initialization
 * @{
 */

/*!
 * @brief Initialize the eTMR channels in input capture mode.
 *
 * Configures edge detection, input filters, interrupts/DMA, and
 * optional combination capture for each specified channel.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] param    Pointer to the input capture configuration.
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_InitInputCapture(uint32_t instance, const etmr_ic_param_t *param);

/*!
 * @brief De-initialize input capture mode.
 *
 * Disables capture channels, clears input filters, and disables
 * channel interrupts.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] param    Pointer to the input capture configuration
 *                     (used to identify which channels to de-init).
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_DeinitInputCapture(uint32_t instance, const etmr_ic_param_t *param);

/*! @} */

/*!
 * @name Measurement Query
 * @{
 */

/*!
 * @brief Check whether an input capture measurement is complete.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel index.
 * @return true if capture is complete, false otherwise.
 */
bool eTMR_DRV_GetInputCaptureComplete(uint32_t instance, uint8_t channel);

/*!
 * @brief Clear the input capture complete flag.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel index.
 * @return Operation status.
 */
status_t eTMR_DRV_ClearInputCaptureComplete(uint32_t instance, uint8_t channel);

/*!
 * @brief Get the measured input signal period.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel index.
 * @return The measured period in timer ticks.
 */
uint32_t eTMR_DRV_GetInputCaptureMeasurementPeriod(uint32_t instance, uint8_t channel);

/*!
 * @brief Get the positive pulse width count.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel index.
 * @return The positive pulse count value in timer ticks.
 */
uint32_t eTMR_DRV_GetInputCapturePositivePulseCount(uint32_t instance, uint8_t channel);

/*!
 * @brief Get the negative pulse width count.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel index.
 * @return The negative pulse count value in timer ticks.
 */
uint32_t eTMR_DRV_GetInputCaptureNegativePulseCount(uint32_t instance, uint8_t channel);

/*!
 * @brief Get the raw capture value (CVAL register).
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel index.
 * @return The captured counter value.
 */
uint32_t eTMR_DRV_GetInputCaptureValue(uint32_t instance, uint8_t channel);

/*! @} */

/*!
 * @name Capture Handler
 * @{
 */

/*!
 * @brief Input capture interrupt handler (software capture).
 *
 * Called from the channel ISR to process edge events and calculate
 * period/pulse-width measurements using software logic.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The channel that triggered the interrupt.
 */
void eTMR_DRV_InputCaptureHandler(uint32_t instance, uint8_t channel);

#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
/*!
 * @brief Input capture interrupt handler (hardware capture).
 *
 * Similar to eTMR_DRV_InputCaptureHandler but uses hardware-measured
 * positive/negative pulse-width registers, eliminating the need for
 * the eTMR_DRV_CalculateChannelCaptureValue function.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The channel that triggered the interrupt.
 */
void eTMR_DRV_InputCaptureHardwareHandler(uint32_t instance, uint8_t channel);
#endif

/*!
 * @brief Calculate period and pulse-width measurements for a channel.
 *
 * Post-processes captured edge timestamps to derive the period,
 * positive pulse count, and negative pulse count values.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The channel to calculate measurements for.
 */
void eTMR_DRV_CalculateChannelCaptureValue(uint32_t instance, uint8_t channel);

/*! @} */

#if defined(__cplusplus)
}
#endif

#endif /* eTMR_IC_DRIVER_H */

/*! @}*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
