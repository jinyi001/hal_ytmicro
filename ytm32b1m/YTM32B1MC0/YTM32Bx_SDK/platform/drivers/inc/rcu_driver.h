/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file rcu_driver.h
 * @version 1.4.0
 */

#ifndef RCU_DIIVER_H
#define RCU_DIIVER_H

#include <stdbool.h>
#include "device_registers.h"
#include "status.h"

/*!
 * rcu_drive
 * @{
 */

/*******************************************************************************
 * API
 ******************************************************************************/


/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*!
 * @brief Defines the reset reason of the RCU module.
 *
 */
typedef enum
{
    RESET_REASON_HVD,                     /*> Reset caused by high voltage detected */
    RESET_REASON_DEEPSLEEPACK,            /*> Reset caused by eepsleep/powerdown mode entry timeout */
#if defined(NUMBER_OF_CORES) && (NUMBER_OF_CORES > 1U)
    RESET_REASON_WDG_CORE0,                     /*> Reset caused by clock monitor watchdog */
    RESET_REASON_WDG_CORE1,                     /*> Reset caused by clock monitor watchdog */
#else
    RESET_REASON_WDG,                     /*> Reset caused by clock monitor watchdog */
#endif /* FEATURE_SOC_HAS_MULTI_CORE */
    RESET_REASON_CMU,                     /*> Reset caused by reset caused by clock monitor if CMURE is set */
    RESET_REASON_CORELOCK,                /*> Reset caused by ARM core lockup */
    RESET_REASON_DEBUG,                   /*> Reset caused by debug */
    RESET_REASON_SOFTWARE_RESET,          /*> Reset caused by SYSRESETREQ of ARM Core */
    RESET_REASON_PIN,                     /*> Reset caused by RESETB pin */
    RESET_REASON_POR_LVD,                 /*> Reset caused by POR or lowvoltage detected */
#if defined(FEATURE_PCTRL_HAS_DIGITAL_FILTER)
#if defined(FMU_INSTANCE_COUNT)&&(FMU_INSTANCE_COUNT > 0U)
    RESET_REASON_FMU_FAIL_REACT,           /*> Reset caused by FMU Fail React */
    RESET_REASON_FMU,                      /*> Reset caused by FMU */
#endif
#endif
    RESET_REASON_MUTI_REASON               /*> Reset caused by multiple reason */
} rcu_reset_reason_t;

/*!
 * @brief Defines reset pin config structure.
 *
 */
typedef struct
{
#if defined(FEATURE_RCU_HAS_PIN_OUTPUT) && (FEATURE_RCU_HAS_PIN_OUTPUT == 1)
    bool pinOutputEnable;/*> Enable or disable reset pin output */
#endif
    bool pinFilterEnable;/*> Enable or disable reset pin fliter */
    bool pinFilterEnableInDeepsleep;/*> Enable or disable reset pin fliter in deepsleep mode */
    uint8_t filterCount; /*> fliter counter value */
} rcu_config_t;


#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initializes the RCU module.
 *
 * This function Initializes the RCU module.
 *
 * @param[in] rcuUserCfg Pointer to RCU configuration structure.
 */
void RCU_DRV_Init(const rcu_config_t *rcuUserCfg);

/*!
 * @brief Get the reset reason
 *
 * This function get the reset reason.
 *
 */
rcu_reset_reason_t RCU_DRV_GetResetReason(void);

/*!
 * @brief Get the reset reason
 *
 * This function get the reset reason raw value from register.
 *
 */
uint32_t RCU_DRV_GetResetReasonRawValue(void);

/*!
 * @brief Clear the reset reason flags
 *
 * This function clear the reset reason flags.
 *
 */
void RCU_DRV_ClearResetReasonFlag(rcu_reset_reason_t resetReason);

/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* RCU_DRIVE_H*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
