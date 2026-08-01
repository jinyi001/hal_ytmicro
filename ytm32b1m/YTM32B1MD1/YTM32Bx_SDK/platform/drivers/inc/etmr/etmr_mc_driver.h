/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_mc_driver.h
 * @version 1.4.1
 *
 * @brief eTMR Timer/Counter Mode Driver — public API declarations.
 *
 * This header declares the timer/counter (MC) mode API for the eTMR
 * peripheral. In this mode the eTMR operates as a simple free-running
 * counter without channel output, suitable for periodic timing and
 * event counting applications.
 *
 * @note All channel outputs are disabled in timer/counter mode.
 */

#ifndef eTMR_MC_DRIVER_H
#define eTMR_MC_DRIVER_H

#include "etmr_common.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Timer/counter mode configuration structure.
 *
 * Specifies the initial and final (modulus) counter values for
 * the eTMR timer/counter mode.
 *
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
 * @name Counter Initialization
 * @{
 */

/*!
 * @brief Initialize the eTMR counter in timer/counter mode.
 *
 * Configures the counter with the specified initial and final values,
 * disables all channel outputs, and disables the quadrature decoder.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] timer    Pointer to the timer configuration structure.
 * @return Operation status.
 * @retval STATUS_SUCCESS Initialized successfully.
 * @retval STATUS_ERROR   Driver state is not initialized.
 */
status_t eTMR_DRV_InitCounter(uint32_t instance, const etmr_timer_param_t *timer);

/*!
 * @brief Populate a timer configuration structure with default values.
 *
 * Sets initialValue = 0 and finalValue = 65535, providing a full
 * 16-bit counting range.
 *
 * @param[in] config Pointer to the structure to be filled with defaults.
 */
void eTMR_MC_DRV_SetDefaultConfig(etmr_timer_param_t *const config);

/*! @} */

/*!
 * @name Counter Control
 * @{
 */

/*!
 * @brief Start the eTMR counter.
 *
 * Enables the eTMR counter clock so that counting begins from the
 * previously configured initial value.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return Operation status.
 * @retval STATUS_SUCCESS Started successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_CounterStart(uint32_t instance);

/*!
 * @brief Stop the eTMR counter.
 *
 * Disables the eTMR counter clock, halting the count.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return Operation status.
 * @retval STATUS_SUCCESS Stopped successfully.
 */
status_t eTMR_DRV_CounterStop(uint32_t instance);

/*!
 * @brief Read the current eTMR counter value.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return The current counter value.
 */
uint32_t eTMR_DRV_CounterRead(uint32_t instance);

/*! @} */

#if defined(__cplusplus)
}
#endif

#endif /* eTMR_MC_DRIVER_H */

/*! @}*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
