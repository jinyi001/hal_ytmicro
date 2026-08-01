/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
/*!
 * @file tmu_driver.h
 * @version 1.4.1
 *
 * @brief TMU Driver - Public API for Trigger Mux routing.
 *
 * This header defines the application-level interface for the Trigger Mux
 * Unit (TMU). The driver provides instance-based APIs for configuring the
 * TMU routing matrix, where each target module selects one trigger source.
 *
 * The APIs are organized into three categories:
 *   - **Initialization & De-initialization**: reset the routing matrix and
 *     apply a batch configuration.
 *   - **Routing Configuration**: update the source selection of one target
 *     module at runtime.
 *   - **Routing Query**: read back the source currently connected to one
 *     target module.
 *
 * @note Enable the TMU peripheral clock before calling this module.
 */

#ifndef TMU_DRIVER_H
#define TMU_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "status.h"
#include "device_registers.h"

/*!
 * @addtogroup tmu
 * @brief Trigger Mux Unit (TMU) Driver - Public API.
 * @details Provides instance-based services for configuring TMU routes from
 *          software. Applications can batch-program multiple routes during
 *          initialization, update individual routes later, and read back the
 *          source assigned to a target module.
 * @{
 */

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief TMU trigger source selector type.
 *
 * This type aliases the device-specific enumeration that lists every signal
 * which can drive a TMU route input. The concrete enum constants are defined
 * in the active device feature header and therefore vary by derivative.
 *
 * Typical sources include TMU input pins, timer events, ADC trigger outputs,
 * comparator outputs, and software-generated trigger events.
 *
 * Implements : tmu_trigger_source_t_Class
 */
typedef enum tmu_trigger_source_e tmu_trigger_source_t;

/*!
 * @brief TMU target module selector type.
 *
 * This type aliases the device-specific enumeration that identifies every
 * configurable TMU output or peripheral trigger input. The available targets
 * depend on the selected device and are defined in the active device feature
 * header.
 *
 * Target values are encoded so that four targets share one TMU MUX register.
 * The driver and HW access layer derive the register index from the target
 * enumeration value.
 *
 * Implements : tmu_target_module_t_Class
 */
typedef enum tmu_target_module_e tmu_target_module_t;

/*!
 * @brief TMU route descriptor.
 *
 * Each entry binds one trigger source to one target module. Arrays of this
 * structure are consumed by TMU_DRV_Init() when the application wants to
 * configure several TMU routes with one call.
 *
 * | Field         | Type                 | Description                                |
 * |---------------|----------------------|--------------------------------------------|
 * | triggerSource | tmu_trigger_source_t | Source signal to connect.                  |
 * | targetModule  | tmu_target_module_t  | Destination route slot or peripheral sink. |
 *
 * Implements : tmu_inout_mapping_config_t_Class
 */
typedef struct
{
    tmu_trigger_source_t triggerSource; /*!< TMU trigger source to be routed. */
    tmu_target_module_t targetModule;   /*!< TMU target module that receives the source. */
} tmu_inout_mapping_config_t;

/*!
 * @brief User configuration structure for batch TMU routing updates.
 *
 * Pass this structure to TMU_DRV_Init() to reset the TMU matrix and then
 * apply zero or more source-to-target mappings.
 *
 * | Field                  | Type                                  | Description                              |
 * |------------------------|---------------------------------------|------------------------------------------|
 * | numInOutMappingConfigs | uint8_t                               | Number of valid entries in the mapping array. |
 * | inOutMappingConfig     | const tmu_inout_mapping_config_t *    | Pointer to the route descriptor array.   |
 *
 * @note If @ref numInOutMappingConfigs is 0, the pointer may be NULL because
 *       the driver will not dereference it.
 *
 * Implements : tmu_user_config_t_Class
 */
typedef struct
{
    uint8_t numInOutMappingConfigs;                        /*!< Number of route descriptors to apply. */
    const tmu_inout_mapping_config_t *inOutMappingConfig; /*!< Pointer to the route descriptor array. */
} tmu_user_config_t;

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/
/*!
 * @name Initialization & De-initialization
 * @brief Functions for resetting the TMU routing matrix and applying an
 *        initial batch of routes.
 * @{
 */

/*!
 * @brief Reset the TMU routing matrix and apply user-defined routes.
 *
 * The driver first calls TMU_Init() to issue reset-value writes to every TMU
 * MUX register, then iterates through the mapping array in @a tmuUserConfig
 * and programs each source-to-target connection.
 *
 * @param[in] instance       TMU instance index (0-based).
 * @param[in] tmuUserConfig  Pointer to the user configuration structure.
 * @return Execution status.
 * @retval STATUS_SUCCESS Reset and configuration requests were issued.
 *
 * @pre The TMU peripheral clock must be enabled.
 * @pre If @a tmuUserConfig->numInOutMappingConfigs is non-zero,
 *      @a tmuUserConfig->inOutMappingConfig must point to a valid array.
 * @post All requested routes have been written to the TMU registers.
 *
 * @note This API does not perform an explicit lock-state check before
 *       applying mappings. If a target register has already been locked in
 *       hardware, the effective route depends on peripheral write behavior.
 */
status_t TMU_DRV_Init(const uint32_t instance,  const tmu_user_config_t *const tmuUserConfig);

/*!
 * @brief Reset all TMU route selections to their register reset values.
 *
 * This function forwards the reset request to TMU_Init() for the selected
 * instance.
 *
 * @param[in] instance  TMU instance index (0-based).
 * @return Execution status.
 * @retval STATUS_SUCCESS Reset writes were issued to the TMU registers.
 *
 * @pre The TMU peripheral clock must be enabled.
 * @post The driver has requested reset values for all TMU MUX registers.
 *
 * @note The function does not disable the TMU peripheral clock.
 */
status_t TMU_DRV_Deinit(const uint32_t instance);

/*! @} */ /* End of Initialization & De-initialization */

/*******************************************************************************
 * Routing Configuration
 ******************************************************************************/
/*!
 * @name Routing Configuration
 * @brief Functions for programming the trigger source of an individual target
 *        module.
 * @{
 */

/*!
 * @brief Update the trigger source connected to one target module.
 *
 * Programs the TMU selection bits associated with @a targetModule so that the
 * selected target receives @a triggerSource.
 *
 * @param[in] instance       TMU instance index (0-based).
 * @param[in] triggerSource  Trigger source enumeration value to route.
 * @param[in] targetModule   Target module enumeration value to update.
 * @return Execution status.
 * @retval STATUS_SUCCESS Route write request completed.
 *
 * @pre The TMU peripheral clock must be enabled.
 * @note This API does not read back the route or inspect the lock bit before
 *       returning. If the selected target belongs to a locked TMU MUX
 *       register, the hardware may ignore the write request.
 */
status_t TMU_DRV_SetTrigSourceForTargetModule(const uint32_t instance,
                                              const tmu_trigger_source_t triggerSource,
                                              const tmu_target_module_t targetModule);

/*! @} */ /* End of Routing Configuration */

/*******************************************************************************
 * Routing Query
 ******************************************************************************/
/*!
 * @name Routing Query
 * @brief Functions for reading back the trigger source connected to a target
 *        module.
 * @{
 */

/*!
 * @brief Read back the trigger source currently selected for one target.
 *
 * Returns the source encoded in the TMU selection field corresponding to
 * @a targetModule.
 *
 * @param[in] instance      TMU instance index (0-based).
 * @param[in] targetModule  Target module enumeration value to inspect.
 * @return Trigger source currently routed to @a targetModule.
 */
tmu_trigger_source_t TMU_DRV_GetTrigSourceForTargetModule(const uint32_t instance,
                                                          const tmu_target_module_t targetModule);

/*! @} */ /* End of Routing Query */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of tmu group */

#endif /* TMU_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
