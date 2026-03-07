/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file rcu_driver.c
 * @version 1.4.0
 */

#include <stddef.h>
#include "rcu_driver.h"
#include "rcu_hw_access.h"

/*FUNCTION**********************************************************************
 *
 * Function Name : RCU_DRV_Init
 * Description   : Initialize RCU mode with user config
 *
 *END**************************************************************************/
void RCU_DRV_Init(const rcu_config_t *rcuUserCfg)
{
    DEV_ASSERT(rcuUserCfg != NULL);

    /* Enable/disable reset pin filter */
    if (rcuUserCfg->pinFilterEnable)
    {
        RCU_EnablePinFilter(RCU);
    } else
    {
        RCU_DisablePinFilter(RCU);
    }

    /* Enable/disable reset pin filter in deepsleep mode */
    if (rcuUserCfg->pinFilterEnableInDeepsleep)
    {
        RCU_EnablePinFilterInDeepsleep(RCU);
    } else
    {
        RCU_DisablePinFilterInDeepsleep(RCU);
    }

    /* Set filter count value */
    RCU_SetPinFilterCount(RCU, rcuUserCfg->filterCount);

#if defined(FEATURE_RCU_HAS_PIN_OUTPUT) && (FEATURE_RCU_HAS_PIN_OUTPUT == 1)
    RCU_SetPinOutput(RCU, rcuUserCfg->pinOutputEnable);
#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : RCU_DRV_GetResetReason
 * Description   : Get mcu reset reason
 *
 *END**************************************************************************/
rcu_reset_reason_t RCU_DRV_GetResetReason(void)
{
    uint32_t resetReasonRawValue;
    rcu_reset_reason_t resetReason;
    /* Get reset raw reason for register */
    resetReasonRawValue = RCU_GetResetReasonRawValue(RCU);
    /* Analyse reset reason */
    if (0U != resetReasonRawValue)
    {
        switch (resetReasonRawValue)
        {
#ifdef RCU_RSSR_HVD_MASK
        case RCU_RSSR_HVD_MASK:
            resetReason = RESET_REASON_HVD;
            break;
#endif
        case RCU_RSSR_LPACK_MASK:
            resetReason = RESET_REASON_DEEPSLEEPACK;
            break;
#if defined(NUMBER_OF_CORES) && (NUMBER_OF_CORES > 1U)
        case RCU_RSSR_WDG0_MASK:
            resetReason = RESET_REASON_WDG_CORE0;
            break;
        case RCU_RSSR_WDG1_MASK:
            resetReason = RESET_REASON_WDG_CORE1;
            break;
#else
        case RCU_RSSR_WDG_MASK:
            resetReason = RESET_REASON_WDG;
            break;
#endif
        case RCU_RSSR_CMU_MASK:
            resetReason = RESET_REASON_CMU;
            break;
        case RCU_RSSR_LOCKUP_MASK:
            resetReason = RESET_REASON_CORELOCK;
            break;
#if defined(RCU_RSSR_DBG_MASK)
        case RCU_RSSR_DBG_MASK:
            resetReason = RESET_REASON_DEBUG;
            break;
#endif /* RCU_RSSR_DBG_MASK */
        case RCU_RSSR_SW_MASK:
            resetReason = RESET_REASON_SOFTWARE_RESET;
            break;
        case RCU_RSSR_PIN_MASK:
            resetReason = RESET_REASON_PIN;
            break;
#if defined(RCU_RSSR_POR_BOR_MASK)
        case RCU_RSSR_POR_BOR_MASK:
#else
        case RCU_RSSR_POR_LVD_MASK:
#endif /* YTM32B1L_SERIES */
            resetReason = RESET_REASON_POR_LVD;
            break;
#ifdef RCU_RSSR_FMU_FAIL_REACT_MASK
        case RCU_RSSR_FMU_FAIL_REACT_MASK:
            resetReason = RESET_REASON_FMU_FAIL_REACT;
            break;
#endif
#ifdef RCU_RSSR_FMU_FUNC_MASK
        case RCU_RSSR_FMU_FUNC_MASK:
            resetReason = RESET_REASON_FMU;
            break;
#endif
        default:
            resetReason = RESET_REASON_MUTI_REASON;
            break;
        }
    }
    else
    {
        resetReason = RESET_REASON_POR_LVD;
    }

    return resetReason;
}

uint32_t RCU_DRV_GetResetReasonRawValue(void)
{
    return RCU_GetResetReasonRawValue(RCU);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : RCU_DRV_ClearResetReasonFlag
 * Description   : Clear mcu reset reason flag
 *
 *END**************************************************************************/
void RCU_DRV_ClearResetReasonFlag(rcu_reset_reason_t resetReason)
{
    switch (resetReason)
    {
#ifdef RCU_RSSR_HVD_MASK
    case RESET_REASON_HVD:
        RCU_ClearRSSRFlag(RCU, RCU_RSSR_HVD_MASK);
        break;
#endif /* RCU_RSSR_HVD_MASK */
    case RESET_REASON_DEEPSLEEPACK:
        RCU_ClearRSSRFlag(RCU, RCU_RSSR_LPACK_MASK);
        break;
#if defined(NUMBER_OF_CORES) && (NUMBER_OF_CORES > 1U)
    case RESET_REASON_WDG_CORE0:
        RCU_ClearRSSRFlag(RCU, RCU_RSSR_WDG0_MASK);
        break;
    case RESET_REASON_WDG_CORE1:
        RCU_ClearRSSRFlag(RCU, RCU_RSSR_WDG1_MASK);
        break;
#else
    case RESET_REASON_WDG:
        RCU_ClearRSSRFlag(RCU, RCU_RSSR_WDG_MASK);
        break;
#endif /* FEATURE_SOC_HAS_MULTI_CORE */
    case RESET_REASON_CMU:
        RCU_ClearRSSRFlag(RCU, RCU_RSSR_CMU_MASK);
        break;
    case RESET_REASON_CORELOCK:
        RCU_ClearRSSRFlag(RCU, RCU_RSSR_LOCKUP_MASK);
        break;
#if defined(RCU_RSSR_DBG_MASK)
    case RESET_REASON_DEBUG:
        RCU_ClearRSSRFlag(RCU, RCU_RSSR_DBG_MASK);
        break;
#endif /* RCU_RSSR_DBG_MASK */
    case RESET_REASON_SOFTWARE_RESET:
        RCU_ClearRSSRFlag(RCU, RCU_RSSR_SW_MASK);
        break;
    case RESET_REASON_PIN:
        RCU_ClearRSSRFlag(RCU, RCU_RSSR_PIN_MASK);
        break;
#if defined(RCU_RSSR_POR_BOR_MASK)
    case RESET_REASON_POR_LVD:
        RCU_ClearRSSRFlag(RCU, RCU_RSSR_POR_BOR_MASK);
        break;
#else
    case RESET_REASON_POR_LVD:
        RCU_ClearRSSRFlag(RCU, RCU_RSSR_POR_LVD_MASK);
        break;
#endif /* YTM32B1L_SERIES */
#ifdef RCU_RSSR_FMU_FAIL_REACT_MASK
    case RESET_REASON_FMU_FAIL_REACT:
        RCU_ClearRSSRFlag(RCU, RCU_RSSR_FMU_FAIL_REACT_MASK);
        break;
#endif /* RCU_RSSR_FMU_FAIL_REACT_MASK */
#ifdef RCU_RSSR_FMU_FUNC_MASK
    case RESET_REASON_FMU:
        RCU_ClearRSSRFlag(RCU, RCU_RSSR_FMU_FUNC_MASK);
        break;
#endif /* RCU_RSSR_FMU_FUNC_MASK */
    case RESET_REASON_MUTI_REASON:
        RCU_ClearRSSRFlag(RCU, RCU->RSSR);
        break;
    default:
        /* default */
        break;
    }
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
