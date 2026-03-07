/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_ic_driver.h
 * @version 1.4.0
 */

#ifndef eTMR_IC_DRIVER_H
#define eTMR_IC_DRIVER_H

#include "etmr_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if FEATURE_eTMR_HAS_COMBINATION_CAPTURE
/*!
 * @brief eTMR input capture combination source
 *
 * Implements : etmr_comb_src_t_Class
 */
typedef enum
{
    eTMR_COMB_SRC_EVEN, /*!< Combination source as even channel */
    eTMR_COMB_SRC_ODD,  /*!< Combination source as odd channel */
} etmr_comb_src_t;

/*!
 * @brief eTMR input capture combination parameters
 *
 * Implements : etmr_ic_ch_pair_param_t_Class
 */
typedef struct
{
    uint8_t hwChannelPairId; /*!< Physical hardware channel pair ID */
    bool combEn;             /*!< Combination enable or not */
    etmr_comb_src_t combSrc; /*!< Combination source */
} etmr_ic_ch_pair_param_t;
#endif

/*!
 * @brief eTMR driver Input capture parameters for each channel
 *
 * Implements : etmr_ic_ch_param_t_Class
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
 * @brief eTMR driver input capture parameters
 *
 * Implements : etmr_ic_param_t_Class
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

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

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

/*!
 * @brief This function initialize the channel in the Input Capture mode
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] param Configuration of the input capture channel.
 * @return status
 *        - STATUS_SUCCESS : Completed successfully.
 *        - STATUS_ERROR : Error occurred.
 */
status_t eTMR_DRV_InitInputCapture(uint32_t instance, const etmr_ic_param_t *param);

/*!
 * @brief This function de-initialize input capture mode
 *
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] param Configuration of the input capture mode.
 * @return status
 *        - STATUS_SUCCESS : Completed successfully.
 *        - STATUS_ERROR : Error occurred.
 */
status_t eTMR_DRV_DeinitInputCapture(uint32_t instance, const etmr_ic_param_t *param);

/*!
 * @brief This function is used to get the complete state.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel
 *
 * @return state   Input capture complete state.
 */
bool eTMR_DRV_GetInputCaptureComplete(uint32_t instance, uint8_t channel);

/*!
 * @brief This function is used to clear the complete state.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel
 *
 * @return state
 */
status_t eTMR_DRV_ClearInputCaptureComplete(uint32_t instance, uint8_t channel);

/*!
 * @brief This function is used to get the input capture measurement period.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel
 *
 * @return value   The input capture measurement period.
 */
uint32_t eTMR_DRV_GetInputCaptureMeasurementPeriod(uint32_t instance, uint8_t channel);

/*!
 * @brief This function is used to get the positive pulse count.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel
 *
 * @return value   The positive pulse count value.
 */
uint32_t eTMR_DRV_GetInputCapturePositivePulseCount(uint32_t instance, uint8_t channel);

/*!
 * @brief This function is used to get the negative pulse count.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel
 *
 * @return value   The negative pulse count value.
 */
uint32_t eTMR_DRV_GetInputCaptureNegativePulseCount(uint32_t instance, uint8_t channel);

/*!
 * @brief This function is used to get the CVAL value.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel
 *
 * @return value   The CVAL value.
 */
uint32_t eTMR_DRV_GetInputCaptureValue(uint32_t instance, uint8_t channel);

/*!
 * @brief Input capture handler
 *
 * This function is used to calculate measurement for input capture.
 *
 * @param[in] instance  The input capture instance number.
 * @param[in] channel   The channel number.
 */
void eTMR_DRV_InputCaptureHandler(uint32_t instance, uint8_t channel);

#if FEATURE_eTMR_HAS_HARDWARE_CAPTURE
/*!
 * @brief    Input capture handler through hardware process way
 * @details  Positive pulse and Negative pulse both obtained by hardware
 *           and do not need to use eTMR_DRV_CalculateChannelCaptureValue function.
 * 
 * @param[in] instance  The input capture instance number
 * @param[in] channel   The channel number
 */
void eTMR_DRV_InputCaptureHardwareHandler(uint32_t instance, uint8_t channel);
#endif

/*!
 * @brief This function is used to calculate period, positive pulse or negative pulse count for each channel.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel
 */
void eTMR_DRV_CalculateChannelCaptureValue(uint32_t instance, uint8_t channel);

#if defined(__cplusplus)
}
#endif

#endif /* eTMR_IC_DRIVER_H */

/*! @}*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
