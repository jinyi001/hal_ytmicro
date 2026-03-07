/*
 * Copyright 2020-2023 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file mpwm_ic_driver.h
 * @version 1.4.0
 */

#ifndef MPWM_IC_DRIVER_H
#define MPWM_IC_DRIVER_H

#include "mpwm_common.h"

/*!
 * @addtogroup mpwm_ic_driver
 * @brief mpwm input capture mode driver
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Enumeration & Struct
 ******************************************************************************/

/*!
 * @brief MPWM input capture edge mode as rising edge or falling edge
 *
 * Implements : mpwm_ic_edge_t_Class
 */
typedef enum
{
    MPWM_CAP_DISABLE,
    MPWM_CAP_RISING_EDGE,
    MPWM_CAP_FALLING_EDGE,
    MPWM_CAP_BOTH_EDGES,
} mpwm_ic_edge_t;

/*!
 * @brief MPWM input capture combination source
 *
 * Implements : mpwm_comb_src_t_Class
 */
typedef enum
{
    MPWM_COMB_SRC_EVEN, /*!< Combination source as even channel */
    MPWM_COMB_SRC_ODD,  /*!< Combination source as odd channel */
} mpwm_comb_src_t;

/*!
 * @brief MPWM input capture channel configuration
 *
 * Implements : mpwm_ic_ch_config_t_Class
 */
typedef struct
{
    uint8_t hwChannelId;
    mpwm_ch_clk_prs_t clkPrs;
    bool cntAutoClr;
    bool overwrite;
    mpwm_ic_edge_t edge;
    bool dmaEn;
    mpwm_ch_count_mode_t countMode;
    uint16_t maxCountValue;
    bool debugMode;
    bool intEn;
	bool timerOverflowIntEn;
} mpwm_ic_ch_config_t;

/*!
 * @brief MPWM input capture channel pair configuration
 *
 * Implements : mpwm_ic_ch_pair_config_t_Class
 */
typedef struct
{
    uint8_t hwChannelPairId; /*!< Hardware channel pair ID */
    bool combEn;             /*!< Combination enable or not */
    mpwm_comb_src_t combSrc; /*!< Combination source */
} mpwm_ic_ch_pair_config_t;

/*!
 * @brief MPWM input capture configuration
 *
 * Implements : mpwm_ic_config_t_Class
 */
typedef struct
{
    uint8_t channelNum;
    uint8_t channelPairNum;
    mpwm_ic_ch_config_t *inputCaptureChannelConfig;
    mpwm_ic_ch_pair_config_t *inputCaptureChannelPairConfig;
} mpwm_ic_config_t;

/*******************************************************************************
 * API Declaration
 ******************************************************************************/

/*!
 * @brief Initialize MPWM input capture mode
 *
 * @param[in] instance The MPWM peripheral instance
 * @param[in] config   The MPWM input capture configuration parameters
 * @return status
 */
status_t MPWM_DRV_InitInputCapture(uint32_t instance,
                                   const mpwm_ic_config_t *config);

/*!
 * @brief Deinitialize MPWM input capture mode
 *
 * @param[in] instance The MPWM peripheral instance
 * @param[in] config   The MPWM input capture configuration parameters
 * @return status
 */
status_t MPWM_DRV_DeinitInputCapture(uint32_t instance,
                                     const mpwm_ic_config_t *config);

/*! @}*/

#endif /* MPWM_IC_DRIVER_H */
