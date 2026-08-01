/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file rcu_driver.h
 * @version 1.4.1
 *
 * @brief RCU Driver - Public API for reset pin filtering and reset-source reporting.
 *
 * This header declares the application-facing interface for the Reset Control
 * Unit (RCU). The driver programs the RESET_B pin filter path, exposes the
 * latched reset status register, and provides helpers for decoding or
 * clearing reset reason flags.
 *
 * The APIs are organized into three categories:
 *   - Initialization
 *   - Reset Reason Query
 *   - Reset Reason Flag Management
 *
 * @note The driver operates on the singleton RCU peripheral defined by the
 *       selected device header.
 */

#ifndef RCU_DRIVER_H
#define RCU_DRIVER_H

#include <stdbool.h>
#include "device_registers.h"
#include "status.h"

/*!
 * @addtogroup rcu
 * @brief Reset Control Unit driver - public API.
 * @details Provides a single-instance driver for configuring the external
 *          reset pin filter and for reading or clearing latched reset-source
 *          flags.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Reset reasons decoded from the RCU reset status register.
 *
 * `RCU_DRV_GetResetReason()` maps the raw `RSSR` register to one public
 * enumeration value for the common single-source cases. When multiple reset
 * flags are latched at the same time, the driver reports
 * `RESET_REASON_MUTI_REASON`.
 *
 * @note Some values are only returned on devices that implement the matching
 *       reset status flag in the device header.
 */
typedef enum
{
    RESET_REASON_HVD,                     /*!< Reset caused by high-voltage detection, when supported. */
    RESET_REASON_DEEPSLEEPACK,            /*!< Reset caused by low-power entry acknowledge timeout. */
#if defined(NUMBER_OF_CORES) && (NUMBER_OF_CORES > 1U)
    RESET_REASON_WDG_CORE0,               /*!< Reset caused by the core 0 watchdog. */
    RESET_REASON_WDG_CORE1,               /*!< Reset caused by the core 1 watchdog. */
#else
    RESET_REASON_WDG,                     /*!< Reset caused by the watchdog timer. */
#endif /* defined(NUMBER_OF_CORES) && (NUMBER_OF_CORES > 1U) */
    RESET_REASON_CMU,                     /*!< Reset caused by the clock monitor unit. */
    RESET_REASON_CORELOCK,                /*!< Reset caused by a core lockup event. */
    RESET_REASON_DEBUG,                   /*!< Reset caused by a debug reset request, when supported. */
    RESET_REASON_SOFTWARE_RESET,          /*!< Reset caused by a software system reset request. */
    RESET_REASON_PIN,                     /*!< Reset caused by the external RESET_B pin. */
    RESET_REASON_POR_LVD,                 /*!< Reset caused by power-on, brown-out, or low-voltage detection. */
#if defined(FEATURE_PCTRL_HAS_DIGITAL_FILTER)
#if defined(FMU_INSTANCE_COUNT) && (FMU_INSTANCE_COUNT > 0U)
    RESET_REASON_FMU_FAIL_REACT,          /*!< Reset caused by the FMU fail-reaction path. */
    RESET_REASON_FMU,                     /*!< Reset caused by the FMU functional reset flag. */
#endif /* defined(FMU_INSTANCE_COUNT) && (FMU_INSTANCE_COUNT > 0U) */
#endif /* defined(FEATURE_PCTRL_HAS_DIGITAL_FILTER) */
    RESET_REASON_MUTI_REASON              /*!< Multiple reset-source flags were latched simultaneously. */
} rcu_reset_reason_t;

/*!
 * @brief Reset pin filter configuration.
 *
 * Describes how `RCU_DRV_Init()` programs the RESET_B pin filter path.
 *
 * | Field                        | Type      | Description |
 * |------------------------------|-----------|-------------|
 * | pinOutputEnable              | bool      | Enable the optional reset pin output driver when the device supports it. |
 * | pinFilterEnable              | bool      | Enable the digital filter on the external reset pin. |
 * | pinFilterEnableInDeepsleep   | bool      | Keep the reset pin filter active while the device is in deep sleep. |
 * | filterCount                  | uint8_t   | Raw filter-count value written to `RPFR[FLTVAL]`. |
 */
typedef struct
{
#if defined(FEATURE_RCU_HAS_PIN_OUTPUT) && (FEATURE_RCU_HAS_PIN_OUTPUT == 1)
    bool pinOutputEnable;                 /*!< Enable the reset pin output driver, when supported. */
#endif
    bool pinFilterEnable;                 /*!< Enable the digital filter on the external reset pin. */
    bool pinFilterEnableInDeepsleep;      /*!< Keep the filter active while the device is in deep sleep. */
    uint8_t filterCount;                  /*!< Raw filter-count value programmed into `RPFR[FLTVAL]`. */
} rcu_config_t;

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Initialization
 ******************************************************************************/
/*!
 * @name Initialization
 * @brief Functions for programming the RESET_B pin filter path.
 * @{
 */

/*!
 * @brief Apply the caller-provided reset pin configuration.
 *
 * Programs the reset pin filter enable bit, deep-sleep filter enable bit,
 * filter counter value, and optional reset pin output enable bit from
 * @a rcuUserCfg.
 *
 * @param[in] rcuUserCfg Pointer to the configuration structure. Must not be
 *                       NULL.
 *
 * @pre `rcuUserCfg` must point to a valid configuration structure.
 * @post The RESET_B pin filter registers reflect the requested settings.
 * @note This function does not clear any latched reset status flags.
 */
void RCU_DRV_Init(const rcu_config_t *rcuUserCfg);

/*! @} */ /* End of Initialization */

/*******************************************************************************
 * Reset Reason Query
 ******************************************************************************/
/*!
 * @name Reset Reason Query
 * @brief Functions for reading the latched reset status.
 * @{
 */

/*!
 * @brief Return the decoded reason for the most recent reset event.
 *
 * Reads the raw `RSSR` register and maps single asserted flags to the public
 * `rcu_reset_reason_t` enumeration.
 *
 * @return Decoded reset reason.
 *
 * @note If multiple reset-source bits are set, the function returns
 *       `RESET_REASON_MUTI_REASON`.
 * @note If the raw reset status register reads as zero, the driver reports
 *       `RESET_REASON_POR_LVD`.
 */
rcu_reset_reason_t RCU_DRV_GetResetReason(void);

/*!
 * @brief Return the raw reset status register value.
 *
 * Provides direct access to the latched `RSSR` bits for applications that
 * need to inspect multiple reset sources at once.
 *
 * @return Raw `RSSR` register value.
 */
uint32_t RCU_DRV_GetResetReasonRawValue(void);

/*! @} */ /* End of Reset Reason Query */

/*******************************************************************************
 * Reset Reason Flag Management
 ******************************************************************************/
/*!
 * @name Reset Reason Flag Management
 * @brief Functions for clearing latched reset-source flags.
 * @{
 */

/*!
 * @brief Clear one or more reset reason flags.
 *
 * Clears the reset flag associated with @a resetReason. Passing
 * `RESET_REASON_MUTI_REASON` clears all flags that are currently latched in
 * `RSSR`.
 *
 * @param[in] resetReason Reset reason selector to clear.
 *
 * @note This helper performs write-one-to-clear updates on the `RSSR`
 *       register.
 */
void RCU_DRV_ClearResetReasonFlag(rcu_reset_reason_t resetReason);

/*! @} */ /* End of Reset Reason Flag Management */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of rcu group */

#endif /* RCU_DRIVER_H */
/*******************************************************************************
 * End of file
 ******************************************************************************/
