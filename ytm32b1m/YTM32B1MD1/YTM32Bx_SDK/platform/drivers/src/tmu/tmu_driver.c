/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file tmu_driver.c
 * @version 1.4.1
 *
 * @brief TMU Driver - implementation of the public TMU_DRV_* API.
 *
 * This file implements the instance-based Trigger Mux routing functions
 * declared in tmu_driver.h. Each driver API resolves the TMU peripheral base
 * address and delegates register access to the TMU hardware access layer.
 */

#include <stddef.h>
#include "tmu_driver.h"
#include "tmu_hw_access.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Table of base addresses for TMU instances. */
static TMU_Type *const s_tmuBase[TMU_INSTANCE_COUNT] = TMU_BASE_PTRS;

/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/

/*!
 * @brief Reset the TMU routing matrix and apply user-defined routes.
 */
status_t TMU_DRV_Init(const uint32_t instance,
                      const tmu_user_config_t *const tmuUserConfig)
{
    DEV_ASSERT(instance < TMU_INSTANCE_COUNT);
    DEV_ASSERT(tmuUserConfig != NULL);

    status_t status;
    TMU_Type *base = s_tmuBase[instance];
    uint8_t count;

    /* Reset the TMU routing matrix before applying user mappings. */
    status = TMU_Init(base);

    if (status == STATUS_SUCCESS)
    {
        /* Program each requested source-to-target route. */
        for (count = 0U; count < tmuUserConfig->numInOutMappingConfigs; count++)
        {
            TMU_SetTrigSourceForTargetModule(base, tmuUserConfig->inOutMappingConfig[count].triggerSource,
                                             tmuUserConfig->inOutMappingConfig[count].targetModule);
        }
    }

    return status;
}

/*!
 * @brief Reset all TMU route selections to their register reset values.
 */
status_t TMU_DRV_Deinit(const uint32_t instance)
{
    DEV_ASSERT(instance < TMU_INSTANCE_COUNT);

    TMU_Type *base = s_tmuBase[instance];
    status_t status;

    /* Restore reset values to the TMU MUX registers. */
    status = TMU_Init(base);

    return status;
}

/*******************************************************************************
 * Routing Configuration
 ******************************************************************************/

/*!
 * @brief Update the trigger source connected to one target module.
 */
status_t TMU_DRV_SetTrigSourceForTargetModule(const uint32_t instance,
                                              const tmu_trigger_source_t triggerSource,
                                              const tmu_target_module_t targetModule)
{
    DEV_ASSERT(instance < TMU_INSTANCE_COUNT);

    TMU_Type *base = s_tmuBase[instance];
    status_t status;

    /* Forward the route update to the HW access layer. */
    TMU_SetTrigSourceForTargetModule(base, triggerSource, targetModule);

    status = STATUS_SUCCESS;

    return status;
}

/*******************************************************************************
 * Routing Query
 ******************************************************************************/

/*!
 * @brief Read back the trigger source currently selected for one target.
 */
tmu_trigger_source_t TMU_DRV_GetTrigSourceForTargetModule(const uint32_t instance,
                                                          const tmu_target_module_t targetModule)
{
    DEV_ASSERT(instance < TMU_INSTANCE_COUNT);

    const TMU_Type *base = s_tmuBase[instance];

    return TMU_GetTrigSourceForTargetModule(base, targetModule);
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
