/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_oc_driver.h
 * @version 1.4.1
 *
 * @brief eTMR Output Compare Driver — public API declarations.
 *
 * This header declares the output compare (OC) mode API for the eTMR
 * peripheral. In this mode the eTMR generates timed output pulses by
 * comparing the counter value against CHx_VAL0/VAL1 match values,
 * with configurable output actions (set, clear, toggle) on each match.
 */

#ifndef eTMR_OC_DRIVER_H
#define eTMR_OC_DRIVER_H

#include "etmr_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Output compare action mode.
 *
 * Specifies the channel output state change on a compare match event.
 *
 */
typedef enum
{
    eTMR_OUTPUT_CLR = 0x00U,    /*!< Clear on match */
    eTMR_OUTPUT_SET = 0x01U,    /*!< Set on match */
    eTMR_OUTPUT_TOGGLE = 0x02U, /*!< Toggle on match */
} etmr_oc_mode_t;

/*!
 * @brief Output compare per-channel configuration.
 *
 * Contains the hardware channel ID, initial output level, compare
 * values, match actions, trigger enable flags, and interrupt enable
 * for a single output compare channel.
 *
 */
typedef struct
{
    uint8_t hwChannelId;         /*!< Physical hardware channel ID */
    uint8_t channelInitVal;      /*!< Channel output initial value */
    etmr_oc_mode_t val0CmpMode;  /*!< Val0 matching output mode */
    etmr_oc_mode_t val1CmpMode;  /*!< Val1 matching output mode */
    uint16_t cmpVal0;            /*!< The compared value */
    uint16_t cmpVal1;            /*!< The compared value */
    bool enableExternalTrigger0; /*!< true: enable the generation of a trigger is used for on-chip modules
                                  *   false: disable the generation of a trigger */
    bool enableExternalTrigger1; /*!< true: enable the generation of a trigger is used for on-chip modules
                                  *   false: disable the generation of a trigger */
    bool interruptEnable;        /*!< true: enable the channel interrupt
                                  *   false: disable the channel interrupt */
} etmr_oc_ch_param_t;

/*!
 * @brief Output compare overall configuration.
 *
 * Groups the number of output channels, counter initial value,
 * maximum count, and per-channel configuration array.
 *
 */
typedef struct
{
    uint8_t nNumOutputChannels; /*!< Number of output compare channels */
#if FEATURE_eTMR_HAS_CNT_INIT_SRC
    bool counterInitValFromInitReg; /*!< Counter initial value source from INIT register or not */
#endif
    uint32_t cntVal;                         /*!< Counter initial value from CNT register */
    uint16_t maxCountValue;                  /*!< Maximum count value in ticks */
    etmr_oc_ch_param_t *outputChannelConfig; /*!< Output compare channels configuration */
} etmr_oc_param_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name OC Initialization
 * @{
 */

/*!
 * @brief Initialize the eTMR in output compare mode.
 *
 * Configures the specified channels to generate timed pulses. When the
 * counter matches CHxVAL0 or CHxVAL1, the channel output changes
 * according to the configured compare mode (set, clear, or toggle).
 * The signal period is determined by maxCountValue.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] param    Pointer to output compare configuration.
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_InitOutputCompare(uint32_t instance, const etmr_oc_param_t *param);

/*!
 * @brief De-initialize output compare mode.
 *
 * Disables compare match output control and resets the channel
 * configuration to defaults. Stops the eTMR counter.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] param    Pointer to the output compare configuration
 *                     (used to identify which channels to de-init).
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_DeinitOutputCompare(uint32_t instance, const etmr_oc_param_t *param);

/*! @} */

/*!
 * @name OC Update
 * @{
 */

/*!
 * @brief Update the compare match values and modes for a channel.
 *
 * Sets new VAL0/VAL1 compare values and their corresponding output
 * modes. Optionally triggers a software synchronization to load the
 * new values immediately.
 *
 * @param[in] instance          The eTMR peripheral instance number.
 * @param[in] channel           Output compare channel index.
 * @param[in] nextCmpMatchVal0  New VAL0 compare value.
 * @param[in] nextCmpMatchVal1  New VAL1 compare value.
 * @param[in] val0CmpMode       Output action on VAL0 match.
 * @param[in] val1CmpMode       Output action on VAL1 match.
 * @param[in] softwareTrigger   true to issue a software trigger for
 *                              immediate register synchronization.
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   VAL0 > VAL1 or other error.
 */
status_t eTMR_DRV_UpdateOutputCompareChannel(uint32_t instance,
                                             uint8_t channel,
                                             uint16_t nextCmpMatchVal0,
                                             uint16_t nextCmpMatchVal1,
                                             etmr_oc_mode_t val0CmpMode,
                                             etmr_oc_mode_t val1CmpMode,
                                             bool softwareTrigger);

/*! @} */

#if defined(__cplusplus)
}
#endif

#endif /* eTMR_OC_DRIVER_H */

/*! @}*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
