/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*!
 * @file tmu_hw_access.h
 * @version 1.4.1
 *
 * @brief TMU Hardware Access Layer.
 *
 * This header provides low-level helpers for resetting TMU MUX registers,
 * programming source selections, reading back the active route, and managing
 * the per-register lock bit.
 *
 * Functions are organized into four categories:
 *   - Initialization
 *   - Routing Configuration
 *   - Routing Query
 *   - Lock Control
 *
 * @note This is an internal layer used by the TMU driver. Application code
 *       should prefer the TMU_DRV_* APIs unless direct register access or
 *       lock control is required.
 */

#ifndef TMU_HW_ACCESS_H
#define TMU_HW_ACCESS_H

#include "tmu_driver.h"

/*!
 * @addtogroup tmu_hw_access TMU Hardware Access
 * @ingroup tmu
 * @brief Low-level register access functions for the TMU peripheral.
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Initialization
 ******************************************************************************/
/*!
 * @name Initialization
 * @brief Functions for restoring the TMU routing matrix to a known reset
 *        state.
 * @{
 */

/*!
 * @brief Write reset values to all TMU MUX registers.
 *
 * Clears every TMU MUX register by writing zero to the full register. This
 * removes all configured source selections from software's point of view and
 * issues a reset-style state to the routing matrix.
 *
 * @param[in] base  Pointer to the TMU peripheral base address.
 * @return Execution status.
 * @retval STATUS_SUCCESS Reset writes were issued.
 *
 * @note The function does not poll lock state before writing.
 */
status_t TMU_Init(TMU_Type *const base);

/*! @} */ /* End of Initialization */

/*******************************************************************************
 * Routing Configuration
 ******************************************************************************/
/*!
 * @name Routing Configuration
 * @brief Functions for programming TMU source-selection fields.
 * @{
 */

/*!
 * @brief Program the trigger source of one TMU target module.
 *
 * The target module encoding determines both the TMU MUX register index and
 * the SEL field within that register. Only the selected SEL field is updated;
 * the other route selections and the lock bit in the same register are
 * preserved.
 *
 * @param[in] base           Pointer to the TMU peripheral base address.
 * @param[in] triggerSource  Trigger source to route. Must fit within
 *                           TMU_MUX_SEL0_WIDTH bits.
 * @param[in] targetModule   Target module whose source selection is updated.
 *
 * @note The function does not inspect the lock bit before issuing the write.
 */
void TMU_SetTrigSourceForTargetModule(TMU_Type *const base,
                                      const tmu_trigger_source_t triggerSource,
                                      const tmu_target_module_t targetModule);

/*! @} */ /* End of Routing Configuration */

/*******************************************************************************
 * Routing Query
 ******************************************************************************/
/*!
 * @name Routing Query
 * @brief Functions for reading the current TMU route selection.
 * @{
 */

/*!
 * @brief Read the trigger source currently selected for one target module.
 *
 * Decodes the TMU MUX register index and SEL field associated with
 * @a targetModule, then returns the source value stored in that field.
 *
 * @param[in] base         Pointer to the TMU peripheral base address.
 * @param[in] targetModule Target module whose route selection is read.
 * @return Trigger source encoded for the selected target module.
 */
tmu_trigger_source_t TMU_GetTrigSourceForTargetModule(const TMU_Type *const base,
                                                      const tmu_target_module_t targetModule);

/*! @} */ /* End of Routing Query */

/*******************************************************************************
 * Lock Control
 ******************************************************************************/
/*!
 * @name Lock Control
 * @brief Functions for setting and reading the per-register TMU lock bit.
 * @{
 */

/*!
 * @brief Set the lock bit of the TMU MUX register that owns a target module.
 *
 * The TMU lock bit is stored once per MUX register, not once per target. As a
 * result, locking one target also locks the other three target selections that
 * share the same register.
 *
 * @param[in] base          Pointer to the TMU peripheral base address.
 * @param[in] targetModule  Any target module located in the register to lock.
 *
 * @warning After the lock bit is set, later route updates for all four targets
 *          in the same TMU MUX register may be ignored until hardware reset
 *          or device-specific lock-clear behavior occurs.
 */
void TMU_SetLockForTargetModule(TMU_Type *const base,
                                const tmu_target_module_t targetModule);

/*!
 * @brief Read the lock bit of the TMU MUX register that owns a target module.
 *
 * Because the lock state is tracked per MUX register, the returned value
 * applies to all four target modules stored in that register.
 *
 * @param[in] base         Pointer to the TMU peripheral base address.
 * @param[in] targetModule Any target module in the register to inspect.
 * @return `true` if the shared lock bit is set, `false` otherwise.
 */
bool TMU_GetLockForTargetModule(const TMU_Type *const base,
                                const tmu_target_module_t targetModule);

/*! @} */ /* End of Lock Control */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of tmu_hw_access group */

#endif /* TMU_HW_ACCESS_H */
/*******************************************************************************
* EOF
*******************************************************************************/
