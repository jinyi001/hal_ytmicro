/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
 
/*!
 * @file tmu_hw_access.c
 * @version 1.4.1
 *
 * @brief TMU Hardware Access Layer - register-level routing helpers.
 *
 * This file implements the low-level TMU services declared in
 * tmu_hw_access.h. The functions operate directly on TMU MUX registers to
 * clear routes, program source selections, read back active routes, and
 * manage the shared lock bit.
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 2985 Rule 2.2: This operation is redundant. The value of the result is 
 *                       always that of the left-hand operand.
 */

#include <stddef.h>
#include "tmu_hw_access.h"

/*******************************************************************************
 * Initialization
 ******************************************************************************/

/*!
 * @brief Write reset values to all TMU MUX registers.
 */
status_t TMU_Init(TMU_Type *const base)
{
    DEV_ASSERT(base != NULL);

    status_t status = STATUS_SUCCESS;

    /* Clear every MUX register to its reset value. */
    for (uint8_t i = 0; i < TMU_MUX_COUNT; i++)
    {
        base->MUX[i] = 0;
    }

    return status;
}

/*******************************************************************************
 * Routing Configuration
 ******************************************************************************/

/*!
 * @brief Program the trigger source of one TMU target module.
 */
void TMU_SetTrigSourceForTargetModule(TMU_Type *const base,
                                      const tmu_trigger_source_t triggerSource,
                                      const tmu_target_module_t targetModule)
{
    DEV_ASSERT(base != NULL);
    DEV_ASSERT(triggerSource < (1 << TMU_MUX_SEL0_WIDTH));
    DEV_ASSERT(targetModule < (TMU_MUX_COUNT * 4));

    uint8_t muxCnt = 0U;
    uint8_t selCnt = 0U;
    uint32_t tmpReg;

    /* Derive the shared MUX register index and SEL field index. */
    muxCnt = (uint8_t)targetModule >> 2U;
    selCnt = (uint8_t)targetModule & 0x3U;

    /* Read the full register so the other route selections are preserved. */
    tmpReg = base->MUX[muxCnt];
    switch (selCnt)
    {
        case (0x0U):
            tmpReg &= ~TMU_MUX_SEL0_MASK;
            tmpReg |= TMU_MUX_SEL0(triggerSource); /*PRQA S 2985*/
            break;
        case (0x1U):
            tmpReg &= ~TMU_MUX_SEL1_MASK;
            tmpReg |= TMU_MUX_SEL1(triggerSource);
            break;
        case (0x2U):
            tmpReg &= ~TMU_MUX_SEL2_MASK;
            tmpReg |= TMU_MUX_SEL2(triggerSource);
            break;
        case (0x3U):
            tmpReg &= ~TMU_MUX_SEL3_MASK;
            tmpReg |= TMU_MUX_SEL3(triggerSource);
            break;
        default:
            /* No update for unexpected selector values. */
            break;
    }

    /* Commit the updated source selection back to the register. */
    base->MUX[muxCnt] = tmpReg;
}

/*******************************************************************************
 * Routing Query
 ******************************************************************************/

/*!
 * @brief Read the trigger source currently selected for one target module.
 */
tmu_trigger_source_t TMU_GetTrigSourceForTargetModule(const TMU_Type *const base,
                                                      const tmu_target_module_t targetModule)
{
    DEV_ASSERT(base != NULL);
    DEV_ASSERT(targetModule < (TMU_MUX_COUNT * 4));

    uint8_t muxCnt = 0U;
    uint8_t selCnt = 0U;
    uint8_t trigSource;

    /* Derive the shared MUX register index and SEL field index. */
    muxCnt = (uint8_t)targetModule / 4U;
    selCnt = (uint8_t)targetModule % 4U;

    if (selCnt == 0U)
    {
        trigSource = (uint8_t)((base->MUX[muxCnt] >> TMU_MUX_SEL0_SHIFT) & TMU_MUX_SEL0_MASK);
    } else if (selCnt == 1U)
    {
        trigSource = (uint8_t)((base->MUX[muxCnt] >> TMU_MUX_SEL1_SHIFT) & TMU_MUX_SEL1_MASK);
    } else if (selCnt == 2U)
    {
        trigSource = (uint8_t)((base->MUX[muxCnt] >> TMU_MUX_SEL2_SHIFT) & TMU_MUX_SEL2_MASK);
    } else
    {
        trigSource = (uint8_t)((base->MUX[muxCnt] >> TMU_MUX_SEL3_SHIFT) & TMU_MUX_SEL3_MASK);
    }

    /* Return the encoded route value as the public trigger-source type. */
    return (tmu_trigger_source_t) (trigSource);
}

/*******************************************************************************
 * Lock Control
 ******************************************************************************/

/*!
 * @brief Set the lock bit of the TMU MUX register that owns a target module.
 */
void TMU_SetLockForTargetModule(TMU_Type *const base,
                                const tmu_target_module_t targetModule)
{
    DEV_ASSERT(base != NULL);
    DEV_ASSERT(targetModule < (TMU_MUX_COUNT * 4));

    uint8_t muxCnt = 0;

    muxCnt = (uint8_t)targetModule / 4U;

    /* Set the shared lock bit for the owning MUX register. */
    base->MUX[muxCnt] |= (((uint32_t) 1U) << TMU_MUX_LOCK_SHIFT);
}

/*!
 * @brief Read the lock bit of the TMU MUX register that owns a target module.
 */
bool TMU_GetLockForTargetModule(const TMU_Type *const base,
                                const tmu_target_module_t targetModule)
{
    DEV_ASSERT(base != NULL);
    DEV_ASSERT(targetModule < (TMU_MUX_COUNT * 4));

    uint8_t muxCnt;
    uint32_t lockVal;
    bool isLock;

    muxCnt = (uint8_t)targetModule / 4U;

    /* Read the shared lock bit of the owning MUX register. */
    lockVal = ((base->MUX[muxCnt] & TMU_MUX_LOCK_MASK) >> TMU_MUX_LOCK_SHIFT);

    isLock = (lockVal == 0U) ? false : true;

    return isLock;
}

/*******************************************************************************
* EOF
*******************************************************************************/
