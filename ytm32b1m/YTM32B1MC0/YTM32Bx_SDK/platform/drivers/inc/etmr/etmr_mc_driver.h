/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_mc_driver.h
 * @version 1.4.0
 */

#ifndef eTMR_MC_DRIVER_H
#define eTMR_MC_DRIVER_H

#include "etmr_common.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief The configuration structure in timer mode
 *
 * Implements : etmr_timer_param_t_Class
 */
typedef struct
{
    uint16_t initialValue;                  /*!< Initial counter value */
    uint16_t finalValue;                    /*!< Final counter value */
} etmr_timer_param_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initialize the eTMR counter.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] timer Timer configuration structure.
 * @return operation status
 *        - STATUS_SUCCESS : Initialized successfully.
 */
status_t eTMR_DRV_InitCounter(uint32_t instance, const etmr_timer_param_t *timer);

/*!
 * @brief Starts the eTMR counter.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return operation status
 *        - STATUS_SUCCESS : Completed successfully.
 *        - STATUS_ERROR : Error occurred.
 */
status_t eTMR_DRV_CounterStart(uint32_t instance);

/*!
 * @brief Stops the eTMR counter.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return operation status
 *        - STATUS_SUCCESS : Completed successfully.
 */
status_t eTMR_DRV_CounterStop(uint32_t instance);

/*!
 * @brief Reads back the current value of the eTMR counter.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return The current counter value
 */
uint32_t eTMR_DRV_CounterRead(uint32_t instance);

/*!
 * @brief This function will set the default configuration values
 *        in the structure which is used as a common use-case.
 * @param[in] config Pointer to the structure in which the
 *             configuration will be saved.
 */
void eTMR_MC_DRV_SetDefaultConfig(etmr_timer_param_t *const config);

#if defined(__cplusplus)
}
#endif

#endif /* eTMR_MC_DRIVER_H */

/*! @}*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
