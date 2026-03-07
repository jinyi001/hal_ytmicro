/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_oc_driver.h
 * @version 1.4.0
 */

#ifndef eTMR_OC_DRIVER_H
#define eTMR_OC_DRIVER_H

#include "etmr_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief eTMR Mode configuration for output compare mode
 *
 * Implements : etmr_oc_mode_t_Class
 */
typedef enum
{
    eTMR_OUTPUT_CLR = 0x00U,    /*!< Clear on match */
    eTMR_OUTPUT_SET = 0x01U,    /*!< Set on match */
    eTMR_OUTPUT_TOGGLE = 0x02U, /*!< Toggle on match */
} etmr_oc_mode_t;

/*!
 * @brief eTMR driver PWM parameters each channel in the output compare mode
 *
 * Implements : etmr_oc_ch_param_t_Class
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
 * @brief eTMR driver PWM parameters which is configured for the list of channels
 *
 * Implements : etmr_oc_param_t_Class
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
 * @brief Configures the eTMR to generate timed pulses (Output compare mode).
 *
 * When the eTMR counter matches the value of CHxVAL0 or CHxVAL1, the channel output is 
 * changed based on what is specified in the mode argument. The signal period can be modified 
 * using param->maxCountValue. 
 *
 * @param [in] instance The eTMR peripheral instance number.
 * @param [in] param configuration of the output compare channels
 * @return success
 *        - STATUS_SUCCESS : Completed successfully.
 *        - STATUS_ERROR : Error occurred.
 */
status_t eTMR_DRV_InitOutputCompare(uint32_t instance, const etmr_oc_param_t *param);

/*!
 * @brief  Disables compare match output control and clears eTMR timer configuration
 *
 * @param [in] instance The eTMR peripheral instance number.
 * @param [in] param Configuration of the output compare channel
 * @return success
 *        - STATUS_SUCCESS : Completed successfully.
 *        - STATUS_ERROR : Error occurred.
 */
status_t eTMR_DRV_DeinitOutputCompare(uint32_t instance, const etmr_oc_param_t *param);

/*!
 * @brief Sets the next compare match value based on the current counter value
 *
 * @param [in] instance The eTMR peripheral instance number.
 * @param [in] channel  output compare channel
 * @param [in] nextCmpMatchVal0 the new value 0
 * @param [in] nextCmpMatchVal1 the new value 1
 * @param [in] val0CmpMode the mode of output compare
 * @param [in] val1CmpMode the mode of output compare
 * @param [in] softwareTrigger This parameter will be true if software trigger sync is enabled
 * @return success
 *        - STATUS_SUCCESS : Completed successfully.
 *        - STATUS_ERROR : Error occurred.
 */
status_t eTMR_DRV_UpdateOutputCompareChannel(uint32_t instance,
                                             uint8_t channel,
                                             uint16_t nextCmpMatchVal0,
                                             uint16_t nextCmpMatchVal1,
                                             etmr_oc_mode_t val0CmpMode,
                                             etmr_oc_mode_t val1CmpMode,
                                             bool softwareTrigger);

#if defined(__cplusplus)
}
#endif

#endif /* eTMR_OC_DRIVER_H */

/*! @}*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
