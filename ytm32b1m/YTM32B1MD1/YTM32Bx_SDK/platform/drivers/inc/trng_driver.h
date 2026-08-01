/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file trng_driver.h
 * @version 1.4.1
 *
 * @brief TRNG Driver - public API for entropy generation and readout.
 *
 * This header defines the application-level interface for the true random
 * number generator (TRNG) peripheral. The driver provides a compact polling
 * workflow for starting entropy generation, checking completion, and reading
 * the generated entropy words.
 *
 * The APIs are organized into three categories:
 *   - **Initialization & De-initialization**: Reset and start the TRNG.
 *   - **Entropy Data Access**: Read the generated entropy words.
 *   - **Status Query**: Poll the generation state and completion flags.
 *
 * @note The TRNG peripheral clock must be enabled before calling these APIs.
 */

#ifndef TRNG_DRIVER_H
#define TRNG_DRIVER_H

#include "trng_hw_access.h"

/*!
 * @addtogroup trng
 * @brief True Random Number Generator driver - public API.
 * @details Provides instance-based helper APIs for initializing the TRNG,
 *          polling entropy generation status, and copying entropy words from
 *          the hardware register bank into a caller-supplied buffer.
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/
/*!
 * @name Initialization & De-initialization
 * @brief Functions for resetting the TRNG and starting a new entropy cycle.
 * @{
 */

/*!
 * @brief Initialize the TRNG driver and start entropy generation.
 *
 * This function resets the TRNG registers to their default state, loads the
 * internal default configuration, overwrites the entropy-delay field with
 * @a entroyDelay, programs the control and limit registers, and finally
 * enables the TRNG module.
 *
 * @param[in] instance     TRNG instance index (0-based). Must be less than
 *                         TRNG_INSTANCE_COUNT.
 * @param[in] entroyDelay  Entropy delay value written to the SDCTL.ENT_DLY
 *                         field. Valid range: 0x0000U to 0xFFFFU.
 *
 * @pre  The TRNG peripheral clock must already be enabled.
 * @post A new entropy-generation cycle is started when @a entroyDelay is in
 *       range.
 *
 * @note The parameter name is kept as-is to preserve the published API.
 * @warning Values above 0xFFFFU are ignored. This function returns no status,
 *          so the caller must provide a valid entropy delay value.
 */
void TRNG_DRV_Init(uint32_t instance, uint32_t entroyDelay);

/*!
 * @brief Reset the TRNG peripheral to its default state.
 *
 * This function restores the TRNG register block by calling the low-level
 * hardware initialization routine. Any in-progress entropy generation is
 * discarded.
 *
 * @param[in] instance  TRNG instance index (0-based). Must be less than
 *                      TRNG_INSTANCE_COUNT.
 *
 * @post The TRNG register block is back in its reset configuration.
 */
void TRNG_DRV_DeInit(uint32_t instance);

/*! @} */ /* End of Initialization & De-initialization */

/*******************************************************************************
 * Entropy Data Access
 ******************************************************************************/
/*!
 * @name Entropy Data Access
 * @brief Functions for copying entropy words out of the TRNG register bank.
 * @{
 */

/*!
 * @brief Read the current entropy block from the TRNG ENT registers.
 *
 * The driver reads the ENT registers in ascending order and stores the values
 * into @a pEntValue. Reading the highest entropy word clears the current
 * entropy block and immediately starts the next generation cycle.
 *
 * @param[in] instance   TRNG instance index (0-based). Must be less than
 *                       TRNG_INSTANCE_COUNT.
 * @param[out] pEntValue Pointer to the destination buffer. Pass NULL to skip
 *                       the read operation.
 *
 * @post On devices without entropy-count optimization, the driver writes
 *       eight 32-bit words to @a pEntValue[0..7]. When
 *       FEATURE_TRNG_ENTROPY_COUNT_OPTIMIZE is enabled, only
 *       @a pEntValue[0..3] are updated.
 *
 * @warning The destination buffer must provide enough storage for the device
 *          variant in use. The final ENT read also clears the current entropy
 *          block and arms the hardware for the next block.
 */
void TRNG_DRV_Get_Ent(uint32_t instance, uint32_t pEntValue[]);

/*! @} */ /* End of Entropy Data Access */

/*******************************************************************************
 * Status Query
 ******************************************************************************/
/*!
 * @name Status Query
 * @brief Functions for polling TRNG generation progress and completion.
 * @{
 */

/*!
 * @brief Get the current entropy-generation status.
 *
 * The driver reports:
 *   - STATUS_BUSY while the hardware is still generating entropy.
 *   - STATUS_SUCCESS when both the entropy-valid and frequency-count-valid
 *     status conditions are asserted.
 *   - STATUS_ERROR for all other states, including hardware failures and
 *     incomplete validation.
 *
 * @param[in] instance  TRNG instance index (0-based). Must be less than
 *                      TRNG_INSTANCE_COUNT.
 * @return Current TRNG status.
 * @retval STATUS_BUSY     Entropy generation is still in progress.
 * @retval STATUS_SUCCESS  A validated entropy block is ready to read.
 * @retval STATUS_ERROR    The block is not ready or an error condition is
 *                         present.
 *
 * @note This function does not clear any hardware status flags.
 */
status_t TRNG_DRV_GetStatus(uint32_t instance);

/*! @} */ /* End of Status Query */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of trng group */

#endif /* TRNG_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
