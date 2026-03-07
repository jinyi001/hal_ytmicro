/*
 * Copyright 2020-2023 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file mpwm_pc_driver.h
 * @version 1.4.0
 */

#ifndef MPWM_PC_DRIVER_H
#define MPWM_PC_DRIVER_H

#include "mpwm_common.h"

/*!
 * @addtogroup mpwm_pc_driver
 * @brief mpwm pulse counter mode driver
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
    MPWM_PULSE_DISABLE,
    MPWM_PULSE_RISING_EDGE,
    MPWM_PULSE_FALLING_EDGE,
    MPWM_PULSE_BOTH_EDGES,
} mpwm_pc_edge_t;

typedef struct
{
    uint8_t hwChannelId;
    mpwm_ch_clk_prs_t clkPrs;
    mpwm_pc_edge_t edge;
    uint16_t maxPulseNum;
    uint16_t cmpPulseNum;
    mpwm_ch_reload_src_t reloadSrc;
    bool dmaEn;
    mpwm_ch_count_mode_t countMode;
    bool debugMode;
    bool timerOverflowIntEn;
    bool intEn;
} mpwm_pc_ch_config_t;

typedef struct
{
    uint8_t channelNum;
    mpwm_pc_ch_config_t *pulseCounterChannelConfig;
} mpwm_pc_config_t;

/*******************************************************************************
 * API Declaration
 ******************************************************************************/

/*!
 * @brief Initialize MPWM pulse counter mode
 *
 * @param[in] instance The MPWM peripheral instance
 * @param[in] config   The MPWM pulse counter mode configuration parameters
 * @return status
 */
status_t MPWM_DRV_InitPulseCounter(uint32_t instance,
                                   const mpwm_pc_config_t *config);

/*!
 * @brief Deinitialize MPWM pulse counter mode
 *
 * @param[in] instance The MPWM peripheral instance
 * @param[in] config   The MPWM pulse counter mode configuration parameters
 * @return status
 */
status_t MPWM_DRV_DeinitPulseCounter(uint32_t instance,
                                     const mpwm_pc_config_t *config);

/*! @}*/

#endif /* MPWM_PC_DRIVER_H */
