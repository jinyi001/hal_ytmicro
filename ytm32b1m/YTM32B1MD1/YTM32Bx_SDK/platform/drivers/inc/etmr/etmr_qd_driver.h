/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_qd_driver.h
 * @version 1.4.1
 *
 * @brief eTMR Quadrature Decoder Driver — public API declarations.
 *
 * This header declares the quadrature decoder (QD) mode API for the
 * eTMR peripheral. In this mode the eTMR decodes two-phase (A/B)
 * encoder signals to provide position counting and direction detection,
 * supporting both phase-encode and count-and-direction modes.
 */

#ifndef eTMR_QD_DRIVER_H
#define eTMR_QD_DRIVER_H

#include "etmr_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Quadrature phase polarity selection.
 *
 * Determines whether the phase input signal is inverted before
 * edge identification.
 *
 */
typedef enum
{
    eTMR_QUAD_PHASE_NORMAL = 0x00U, /*!< Phase input signal is not inverted before identifying
                                      *   the rising and falling edges of this signal */
    eTMR_QUAD_PHASE_INVERT = 0x01U, /*!< Phase input signal is inverted before identifying
                                      *   the rising and falling edges of this signal */
} etmr_qd_phase_polarity_t;

/*!
 * @brief Quadrature decoder configuration structure.
 *
 * Contains all parameters needed to configure and start the
 * quadrature decoder, including mode, clock prescaler, counter
 * range, input filtering, and phase polarities.
 *
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
 * @brief Quadrature decoder state structure.
 *
 * Reports the current counter value, overflow status, and direction
 * of the quadrature decoder.
 *
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
 * @name QD Initialization
 * @{
 */

/*!
 * @brief Configure and start quadrature decode mode.
 *
 * Sets up the quadrature decoder with the specified mode, phase
 * polarities, input filters, clock prescaler, and counter range,
 * then enables the decoder.
 *
 * @param[in] instance Instance number of the eTMR module.
 * @param[in] config   Pointer to the quadrature decoder configuration.
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_QuadDecodeStart(uint32_t instance, const etmr_qd_config_t *config);

/*!
 * @brief De-activate quadrature decode mode.
 *
 * Disables the quadrature decoder without modifying other eTMR settings.
 *
 * @param[in] instance Instance number of the eTMR module.
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_QuadDecodeStop(uint32_t instance);

/*!
 * @brief Populate a quadrature decoder configuration with default values.
 *
 * Default: phase-B-follow-A mode, no input filters, normal polarity
 * for both phases, counter range 0–65535.
 *
 * @param[out] config Pointer to the structure to be filled with defaults.
 */
void eTMR_DRV_GetQuadDecodeDefaultConfig(etmr_qd_config_t *const config);

/*! @} */

/*!
 * @name QD State Query
 * @{
 */

/*!
 * @brief Get the current quadrature decoder state.
 *
 * Returns the counter value, overflow flag, overflow direction,
 * and current counting direction.
 *
 * @param[in] instance Instance number of the eTMR module.
 * @return The current quadrature decoder state.
 */
etmr_qd_state_t eTMR_DRV_GetQuadDecodeState(uint32_t instance);

/*! @} */

#if defined(__cplusplus)
}
#endif

#endif /* eTMR_QD_DRIVER_H */

/*! @}*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
