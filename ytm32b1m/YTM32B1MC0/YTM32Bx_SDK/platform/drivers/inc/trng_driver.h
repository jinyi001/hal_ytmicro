/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file trng_driver.h
 * @version 1.4.0
 */

#ifndef TRNG_DRIVER_H
#define TRNG_DRIVER_H

#include "trng_hw_access.h"

/*!
 * @addtogroup trng_driver
 * @ingroup trng
 * @details This section describes the programming interface of the Trng Peripheral Driver.
 * @{
 */

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
/*!
 * @name Trng driver APIs
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initialize a TRNG instance for operation.
 *
 * This function first resets the source triggers of all TRNG target modules to their default values,
 * then configures the TRNG with all the user defined in-out mappings.
 * This example shows how to set up the trng_config_t parameters and how to call the
 * TRNG_DRV_Init() function with the required parameters:
 *  @code
 *   TRNG_DRV_Init(instance, entroyDelay);
 *   @endcode
 *
 * @param[in] instance          The TRNG instance number.
 * @param[in] entroyDelay       The length of each entropy sample taken.
 */
void TRNG_DRV_Init(uint32_t instance, uint32_t entroyDelay);


/*!
 * @brief Reset to default values the source triggers corresponding to all target 
 *
 * @param[in] instance          The TRNG instance number.
 */
void TRNG_DRV_DeInit(uint32_t instance);

/*!
 * @brief Read the entropy value..
 *        Reading the highest offset(ENT(7)) will clear the entire entropy value
 *        and start a new entropy generation.
 * @param[in] instance          The TRNG instance number.
 * @param[in] pEntValue         Store the entropy pointer.
 */
void TRNG_DRV_Get_Ent(uint32_t instance,uint32_t pEntValue[]);
/*!
 * @brief Get the status of the entropy generation FSM.
 *
 * @param[in] instance          The TRNG instance number.
 */
status_t TRNG_DRV_GetStatus(uint32_t instance);


#if defined(__cplusplus)
}
#endif
/*! @}*/ /* End of Trng driver APIs*/
/*! @}*/ /* End of addtogroup trng_driver */

#endif  /* TRNG_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/

