/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_qd_driver.h
 * @version 1.4.0
 */

#ifndef eTMR_QD_DRIVER_H
#define eTMR_QD_DRIVER_H

#include "etmr_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*!
 * @brief eTMR quadrature phase polarities, normal or inverted polarity
 *
 * Implements : etmr_qd_phase_polarity_t_Class
 */
typedef enum
{
    eTMR_QUAD_PHASE_NORMAL = 0x00U, /*!< Phase input signal is not inverted before identifying
                                      *   the rising and falling edges of this signal */
    eTMR_QUAD_PHASE_INVERT = 0x01U, /*!< Phase input signal is inverted before identifying
                                      *   the rising and falling edges of this signal */
} etmr_qd_phase_polarity_t;

/*!
 * @brief eTMR quadrature configure structure
 *
 * Implements : etmr_quad_decode_config_t_Class
 */
typedef struct
{
    etmr_qd_mode_t mode;                /*!< eTMR_QUAD_PHASE_ENCODE or eTMR_QUAD_COUNT_AND_DIR */
    etmr_qd_clock_prs_t qdClkPrs;       /*!< eTMR quadrature decoder clock prescaler */
#if FEATURE_eTMR_HAS_INIT_REG
    uint32_t initVal;                   /*!< Counter init value*/
#endif
#if FEATURE_eTMR_HAS_QUAD_DECODER_MOD
    uint32_t modVal;                    /*!< Counter mod value*/
#endif
    uint8_t qdInputFilterSampleCount;   /*!< Quad decoder phase A and B input filter sample count */
    uint8_t qdInputFilterSamplePeriod;  /*!< Quad decoder phase A and B input filter sample period */
    etmr_qd_phase_polarity_t phaseAPol; /*!< Configuration for the input phase a */
    etmr_qd_phase_polarity_t phaseBPol; /*!< Configuration for the input phase b */
} etmr_qd_config_t;

/*!
 * @brief eTMR quadrature state(counter value and flags)
 *
 * Implements : etmr_qd_state_t_Class
 */
typedef struct
{
    uint32_t qdCounterValue;  /*!< Quad decoder counter value */
    bool qdOverflowFlag;      /*!< True if overflow occurred, False if overflow doesn't occurred */
    bool qdOverflowDirection; /*!< False if overflow occurred at minimum value,
                               *   True if overflow occurred at maximum value */
    bool qdCounterDirection;  /*!< False eTMR counter is decreasing, True eTMR counter is increasing */
} etmr_qd_state_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Configures the quadrature mode and starts measurement
 *
 * @param [in] instance Instance number of the eTMR module.
 * @param [in] config   Configuration structure(quadrature decode mode, polarity for both phases,
 *                      initial and maximum value for the counter, filter configuration).
 * @return success
 *        - STATUS_SUCCESS : Completed successfully.
 *        - STATUS_ERROR : Error occurred.
 */
status_t eTMR_DRV_QuadDecodeStart(uint32_t instance, const etmr_qd_config_t *config);

/*!
 * @brief De-activates the quadrature decode mode.
 *
 * @param [in] instance Instance number of the eTMR module.
 * @return success
 *        - STATUS_SUCCESS : Completed successfully.
 *        - STATUS_ERROR : Error occurred.
 */
status_t eTMR_DRV_QuadDecodeStop(uint32_t instance);

/*!
 * @brief Return the current quadrature decoder state (counter value, overflow flag and
 * overflow direction)
 *
 * @param [in] instance Instance number of the eTMR module.
 * @return The current state of quadrature decoder
 */
etmr_qd_state_t eTMR_DRV_GetQuadDecodeState(uint32_t instance);

/*!
 * @brief This function will get the default configuration values
 *        in the structure which is used as a common use-case.
 * @param[out] config Pointer to the structure in which the
 *                    configuration will be saved.
 */
void eTMR_DRV_GetQuadDecodeDefaultConfig(etmr_qd_config_t *const config);

#if defined(__cplusplus)
}
#endif

#endif /* eTMR_QD_DRIVER_H */

/*! @}*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
