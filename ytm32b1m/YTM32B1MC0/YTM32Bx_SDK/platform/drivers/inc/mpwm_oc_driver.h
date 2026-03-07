/*
 * Copyright 2020-2023 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file mpwm_oc_driver.h
 * @version 1.4.0
 */

#ifndef MPWM_OC_DRIVER_H
#define MPWM_OC_DRIVER_H

#include "mpwm_common.h"

/*!
 * @addtogroup mpwm_oc_driver
 * @brief mpwm output compare mode driver
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Enumeration & Struct
 ******************************************************************************/

typedef enum
{
    MPWM_OUTPUT_DISABLE,  /*!< Compare output is disabled */
    MPWM_OUTPUT_CLR,  /*!< Compare output is 0 */
    MPWM_OUTPUT_SET,  /*!< Compare output is 1 */
    MPWM_OUTPUT_TOGGLE,  /*!< Compare output toggle */
} mpwm_oc_mode_t;

typedef struct
{
    uint8_t hwChannelId;
    mpwm_ch_clk_prs_t clkPrs;
    uint16_t maxCountValue;
    uint16_t compareValue;
    mpwm_oc_mode_t compareOutput;
    mpwm_ch_output_init_t outputInit;
    bool debugMode;
    mpwm_ch_reload_src_t reloadSrc;
    mpwm_ch_output_trig_config_t outputTrigConfig;
    bool intEn;
    bool timerOverflowIntEn;
} mpwm_oc_ch_config_t;

typedef struct
{
    uint8_t channelNum;
    mpwm_oc_ch_config_t *outputCompareChannelConfig;
} mpwm_oc_config_t;

/*******************************************************************************
 * API Declaration
 ******************************************************************************/

/*!
 * @brief Initialize MPWM output compare mode
 *
 * @param[in] instance The MPWM peripheral instance
 * @param[in] config   The MPWM output compare configuration parameters
 * @return status
 */
status_t MPWM_DRV_InitOutputCompare(uint32_t instance,
                                    const mpwm_oc_config_t *config);

/*!
 * @brief Deinitialize MPWM output compare mode
 *
 * @param[in] instance The MPWM peripheral instance
 * @param[in] config   The MPWM output compare configuration parameters
 * @return status
 */
status_t MPWM_DRV_DeinitOutputCompare(uint32_t instance,
                                      const mpwm_oc_config_t *config);

/*!
 * @brief Update output compare configuration parameters for channel
 *
 * @param[in] instance The MPWM peripheral instance
 * @param[in] channel  The MPWM channel
 * @param[in] mode     The output mode - set, clear or toggle
 * @param[in] newCmpVal The new compare value
 * @param[in] newPerVal The new period value
 * @return status
 */
status_t MPWM_DRV_UpdateOutputCompareChannel(uint32_t instance,
                                             uint8_t channel,
                                             mpwm_oc_mode_t mode,
                                             uint16_t newCmpVal,
                                             uint16_t newPerVal);

/*! @}*/

#endif /* MPWM_OC_DRIVER_H */
