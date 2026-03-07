/*
 * Copyright 2020-2023 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file mpwm_ct_driver.h
 * @version 1.4.0
 */

#ifndef MPWM_CT_DRIVER_H
#define MPWM_CT_DRIVER_H

#include "mpwm_common.h"

/*!
 * @addtogroup mpwm_ct_driver
 * @brief mpwm common timer mode driver
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Enumeration & Struct
 ******************************************************************************/

typedef struct
{
    uint8_t hwChannelId;
    uint16_t finalValue;
    mpwm_ch_clk_prs_t clkPrs;
    mpwm_ch_reload_src_t reloadSrc;
    bool hwTrigEn;
    bool debugMode;
    mpwm_ch_count_mode_t countMode;
    bool timerOverflowIntEn;
} mpwm_ct_ch_config_t;

typedef struct
{
    uint8_t channelNum;
    mpwm_ct_ch_config_t *timerChannelConfig;
} mpwm_ct_config_t;

/*******************************************************************************
 * API Declaration
 ******************************************************************************/

/*!
 * @brief Initialize MPWM channel common timer mode
 *
 * @param[in] instance The MPWM peripheral instance
 * @param[in] config   The MPWM common timer struct configuration
 * @return status
 */
status_t MPWM_DRV_InitTimer(uint32_t instance, const mpwm_ct_config_t *config);

/*!
 * @brief Deinitialize MPWM channel common timer mode
 *
 * @param[in] instance The MPWM peripheral instance
 * @param[in] config   The MPWM common timer struct configuration
 * @return status
 */
status_t MPWM_DRV_DeinitTimer(uint32_t instance, const mpwm_ct_config_t *config);

/*!
 * @brief Start MPWM channel common timer
 *
 * @param[in] instance   The MPWM peripheral instance
 * @param[in] channelSet The set of enabled channel(s)
 * @return status
 */
status_t MPWM_DRV_StartTimer(uint32_t instance, uint16_t channelSet);

/*!
 * @brief Stop MPWM channel common timer
 *
 * @param[in] instance   The MPWM peripheral instance
 * @param[in] channelSet The set of disabled channel(s)
 * @return status
 */
status_t MPWM_DRV_StopTimer(uint32_t instance, uint16_t channelSet);

/*! @}*/

#endif /* MPWM_CT_DRIVER_H */
