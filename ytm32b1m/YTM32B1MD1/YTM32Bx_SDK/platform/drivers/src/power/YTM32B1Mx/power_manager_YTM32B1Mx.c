/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file power_manager_YTM32B1Mx.c
 * @version 1.4.1
 *
 * @brief Power Manager — YTM32B1Mx platform-specific implementation.
 *
 * This file implements the platform-specific POWER_SYS_Do* functions
 * for the YTM32B1M series MCUs. It handles ARM SCB, PCU, CMU, and
 * SysTick register manipulation for power mode transitions.
 */

#include "power_manager.h"
#include "clock_manager.h"

/*! @brief Power manager internal state instance. */
power_manager_state_t gPowerManagerState;

/*******************************************************************************
 * INTERNAL FUNCTIONS
 ******************************************************************************/
static void POWER_SYS_SwitchToLowPowerMode(const power_manager_user_config_t * const configPtr);

/*******************************************************************************
 * Platform Initialization
 ******************************************************************************/

/*!
 * @brief Perform platform-specific power manager initialization.
 *
 * @code{.c}
 *
 *  power_manager_user_config_t sleepConfig = {
 *     .powerMode = POWER_MANAGER_SLEEP,
 *     .sleepOnExitValue = false,
 *  };
 *
 *  power_manager_user_config_t deepsleepConfig = {
 *     .powerMode = POWER_MANAGER_DEEPSLEEP,
 *     .sleepOnExitValue = false,
 *  };
 *
 *  power_manager_user_config_t const * powerConfigsArr[] = {
 *     &sleepConfig,
 *     &deepsleepConfig
 *  };
 *
 *  power_manager_callback_user_config_t callbackCfg0 = {
 *     .callbackFunction                     = &callback0,
 *     .callbackType                         = POWER_MANAGER_CALLBACK_BEFORE_AFTER,
 *     .callbackData                         = (void *)0,
 *  };
 *
 *  power_manager_callback_user_config_t const * callbacksConfigsArr[] = {
 *     &callbackCfg0
 *  };
 *
 *  status_t callback0(power_manager_notify_struct_t * notify,
 *                                      power_manager_callback_data_t * dataPtr)
 *  {
 *   status_t ret = STATUS_SUCCESS;
 *   ...
 *   return ret;
 *  }
 *
 *  int main(void)
 *  {
 *   status_t ret = STATUS_SUCCESS;
 *
 *   POWER_SYS_Init(&powerConfigsArr, 2U, &powerStaticCallbacksConfigsArr, 1U);
 *
 *   ret = POWER_SYS_SetMode(MODE_DEEPSLEEP,POWER_MANAGER_POLICY_AGREEMENT);
 *
 *   if (ret != STATUS_SUCCESS)
 *   {
 *     return -1;
 *   }
 *   return 0;
 *  }
 *
 * @endcode
 */
status_t POWER_SYS_DoInit(void)
{
    return STATUS_SUCCESS;
}

/*!
 * @brief Perform platform-specific power manager de-initialization.
 */
status_t POWER_SYS_DoDeinit(void)
{
    /* Biasing disabled, core logic can run in full performance */
    status_t status = STATUS_SUCCESS;

    return status;
}

/*******************************************************************************
 * Platform Mode Control
 ******************************************************************************/

/*!
 * @brief Perform platform-specific power mode switch.
 */
status_t POWER_SYS_DoSetMode(const power_manager_user_config_t * const configPtr)
{
    status_t returnCode = STATUS_SUCCESS; /* Function return */

    POWER_SYS_SwitchToLowPowerMode(configPtr);

    return returnCode;
}

/*!
 * @brief Get the currently active hardware power mode.
 */
power_mode_stat_t POWER_SYS_GetCurrentMode(power_manager_user_config_t *configPtr)
{
    power_mode_stat_t retVal;

    switch (configPtr->powerMode)
    {
        /* Run mode */
        case POWER_MANAGER_RUN:
            retVal = STAT_RUN;
            break;
        /* Sleep mode */
        case POWER_MANAGER_SLEEP:
            retVal = STAT_SLEEP;
            break;
        case POWER_MANAGER_DEEPSLEEP:
            retVal = STAT_DEEPSLEEP;
            break;
        /* This should never happen - core has to be in some run mode to execute code */
        default:
            retVal = STAT_INVALID;
            break;
    }

    return retVal;
}

/*******************************************************************************
 * Internal: Low-Power Mode Switch
 ******************************************************************************/

/*!
 * @brief Switch to a low-power mode by configuring SCB, PCU, and executing WFI.
 *
 * Handles CMU disable/restore, SysTick disable/restore, cache management,
 * flash idle wait (errata workaround), and FXOSC wait-for-valid after wakeup.
 */
static void POWER_SYS_SwitchToLowPowerMode(const power_manager_user_config_t * const configPtr)
{
    uint32_t systickCSR;
#if (defined(FEATURE_SOC_HAS_SEPARATE_CMU_AND_CLKOUT) && (FEATURE_SOC_HAS_SEPARATE_CMU_AND_CLKOUT==1))
    uint32_t cmuCtrl = CMU0->CTRL;
#else
    uint32_t cmuCtrl = SCU->CMU_CTRL;
#endif

#if defined(FEATURE_SOC_SUPPORT_CACHE) && (FEATURE_SOC_SUPPORT_CACHE == 1U)
    uint32_t cacheCACR = CACHE->CACR;
#endif /* FEATURE_SOC_SUPPORT_CACHE */

    /* Configure the hardware layer */
    switch (configPtr->powerMode)
    {
        case POWER_MANAGER_RUN:
            /* No operation needed */
            SCB->SCR &= ~(SCB_SCR_SLEEPONEXIT_Msk);
            break;
        case POWER_MANAGER_SLEEP:
            SCB->SCR &= ~(SCB_SCR_SLEEPDEEP_Msk);
            break;
#if defined(CPU_YTM32B1MC0) || defined(CPU_YTM32B1MD2)
        case POWER_MANAGER_DEEPSLEEP:
            SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
            PCU->CSR &= ~(PCU_CSR_PMCLPEN_MASK);
            break;
        case POWER_MANAGER_STANDBY:
            SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
            PCU->CSR |= (PCU_CSR_PMCLPEN_MASK);
            break;
#elif defined(CPU_YTM32B1ME1)
        case POWER_MANAGER_DEEPSLEEP:
            SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
            PCU->CTRL0 &= ~(PCU_CTRL0_PWR_MODE_MASK);
            break;
        case POWER_MANAGER_STANDBY:
            SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
            PCU->CTRL0 &= ~(PCU_CTRL0_PWR_MODE_MASK);
            PCU->CTRL0 |= (PCU_CTRL0_PWR_MODE(0x1));
            break;
        case POWER_MANAGER_POWERDOWN:
            SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
            PCU->CTRL0 &= ~(PCU_CTRL0_PWR_MODE_MASK);
            PCU->CTRL0 |= (PCU_CTRL0_PWR_MODE(0x2));
            break;
        case POWER_MANAGER_DEEPPOWERDOWN:
            SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
            PCU->CTRL0 &= ~(PCU_CTRL0_PWR_MODE_MASK);
            PCU->CTRL0 |= (PCU_CTRL0_PWR_MODE(0x3));
            break;
#else
        case POWER_MANAGER_DEEPSLEEP:
            SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
            PCU->CTRL &= ~(PCU_CTRL_RPMEN_MASK | PCU_CTRL_STANDBYEN_MASK);
            break;
        case POWER_MANAGER_STANDBY:
            SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
            PCU->CTRL |= PCU_CTRL_RPMEN_MASK | PCU_CTRL_STANDBYEN_MASK;
            break;
        case POWER_MANAGER_POWERDOWN:
            SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
            PCU->CTRL &= ~(PCU_CTRL_RPMEN_MASK | PCU_CTRL_STANDBYEN_MASK);
            PCU->CTRL |= PCU_CTRL_RPMEN_MASK;
            break;
#endif
        default:
            SCB->SCR &= ~(SCB_SCR_SLEEPDEEP_Msk);
            break;
    }

    if ((configPtr->powerMode == POWER_MANAGER_SLEEP) ||
        (configPtr->powerMode == POWER_MANAGER_DEEPSLEEP) ||
        #if defined(CPU_YTM32B1MD1) || defined(CPU_YTM32B1ME0) || defined(CPU_YTM32B1ME1)
        (configPtr->powerMode == POWER_MANAGER_POWERDOWN) ||
        #endif
        #if defined(CPU_YTM32B1ME1)
        (configPtr->powerMode == POWER_MANAGER_DEEPPOWERDOWN) ||
        #endif
        (configPtr->powerMode == POWER_MANAGER_STANDBY))
    {
        /* Disable CMU before entering low power mode */
    #if defined(CPU_YTM32B1MD2)
        CMU0->CTRL = 0;
        CMU0->STS = CMU0->STS;
    #else
        SCU->CMU_CTRL = 0;
        SCU->CMUSTS = SCU->CMUSTS;
    #endif
        /* Disable systick before entering low power mode */
        systickCSR = SysTick->CTRL;
        SysTick->CTRL = 0x00;
        /* Clear systick pending, to prevent immediate wake-up after sleep. */
        SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;
    
        if (configPtr->sleepOnExitValue)
        {
            SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
        }
        else
        {
            SCB->SCR &= ~(SCB_SCR_SLEEPONEXIT_Msk);
        }

#if defined(FEATURE_SOC_SUPPORT_CACHE) && (FEATURE_SOC_SUPPORT_CACHE == 1U)
        /* ERRATA_E022001, disable cache before entering low power mode to avoid reset caused by lpack. */
        CACHE->CACR = 0;
        __DSB();
        __ISB();
#endif /* FEATURE_SOC_SUPPORT_CACHE */

        /* ERRATA_E220008, this error only occurs with YTM32B1MD1.
           But for robustness, it's better to wait for flash idle before entering low power mode. */
        while(0U == (EFM->STS & EFM_STS_IDLE_MASK))
        {
            // wait for flash idle
        }
        STANDBY();
    
        /* Clear SLEEPDEEP bit to avoid RTOS tickless idle getting into deepsleep mode */
        SCB->SCR &= ~(SCB_SCR_SLEEPDEEP_Msk);
    
        /* Restore systick */
        SysTick->CTRL = systickCSR;
    
        if ((SCU->FXOSC_CTRL & SCU_FXOSC_CTRL_FXOSC_EN_MASK) != 0U)
        {
            (void)CLOCK_SYS_WaitFXOSCValid();
        }

#if defined(FEATURE_SOC_SUPPORT_CACHE) && (FEATURE_SOC_SUPPORT_CACHE == 1U)
        /* Restore cache CACR */
        if ((cacheCACR & CACHE_CACR_EN_MASK) != 0)
        {
            /* Invalidate way0 and way1 */
            CACHE->CACR |= CACHE_CACR_IW0_MASK | CACHE_CACR_IW1_MASK | CACHE_CACR_CMDREQ_MASK;
            while ((CACHE->CACR & CACHE_CACR_CMDREQ_MASK) == CACHE_CACR_CMDREQ_MASK)
            {
                /* Wait until cache command request done */
            }
            __DSB();
            __ISB();
            CACHE->CACR = cacheCACR;
        }
#endif /* FEATURE_SOC_SUPPORT_CACHE */

        /* Restore CMU control register */
    #if defined(CPU_YTM32B1MD2)
        CMU0->CTRL = cmuCtrl;
    #else
        SCU->CMU_CTRL = cmuCtrl;
    #endif
    }
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
