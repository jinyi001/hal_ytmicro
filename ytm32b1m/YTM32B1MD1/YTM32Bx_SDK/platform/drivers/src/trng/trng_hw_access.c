/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*!
 * @file trng_hw_access.c
 * @version 1.4.1
 *
 * @brief TRNG Hardware Access Layer - low-level register helper implementations.
 *
 * This file implements the non-inline TRNG helper functions declared in
 * trng_hw_access.h. The helpers program the protected TRNG registers from the
 * shared trngCfgSetting shadow and provide direct access to entropy and status
 * registers.
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 2985 Rule 2.2 : This operation is redundant. The value of the result is
 *                        always that of the left-hand operand.
 */

#include "trng_hw_access.h"

/*! @brief Shared TRNG configuration shadow used by the low-level setters. */
trng_config_setting_t trngCfgSetting;

/*******************************************************************************
 * Module Control
 ******************************************************************************/

/*!
 * @brief Enable the TRNG module.
 */
void TRNG_Enable(TRNG_Type *const base)
{
    base->CTRL |= TRNG_CTRL_TRNG_EN(1);
}

/*!
 * @brief Restore the TRNG register block to its reset configuration.
 */
void TRNG_HW_Init(TRNG_Type *const base)
{
    DEV_ASSERT(base != NULL);

    base->CTRL = 0x01220020U;
    base->SDCTL = 0x0c8009c4U;
    base->FRQMIN = 0x00000640U;
    base->FRQMAX = 0x00006400U;
    base->FRQCNT = 0x00000000U;
    base->SCML = 0x010c0568U;
    base->SCMC = 0x00000000U;
    base->CTRL = TRNG_CTRL_UNLOCK_MASK;
}

/*!
 * @brief Disable the TRNG module.
 */
void TRNG_Disable(TRNG_Type *const base)
{
    base->CTRL &= TRNG_CTRL_TRNG_EN(0);
}

/*!
 * @brief Request a software reset of the TRNG module.
 */
void TRNG_SWReset(TRNG_Type *const base)
{
    uint32_t temp = 0;
    temp = base->CTRL;

    /* Open the protected control field before asserting software reset. */
    temp |= TRNG_CTRL_UNLOCK(1) | TRNG_CTRL_SWR(1);
    /* Close the protected control field after the reset request is issued. */
    temp &= ~TRNG_CTRL_UNLOCK(1);
    base->CTRL = temp;
}

/*******************************************************************************
 * Configuration Register Programming
 ******************************************************************************/

/*!
 * @brief Program the CTRL register from trngCfgSetting.
 */
void TRNG_Set_CTRL(TRNG_Type *const base)
{
    uint32_t temp = TRNG_CTRL_UNLOCK(1) |
                    TRNG_CTRL_RTY_CT(trngCfgSetting.retryCnt) |
                    TRNG_CTRL_LRUN_MAX(trngCfgSetting.lrunMax) |
                    TRNG_CTRL_FOR_SCLK(trngCfgSetting.forceSystemClockEn ? 1U : 0U) | /*PRQA S 2985*/
                    TRNG_CTRL_CLK_OUT(trngCfgSetting.clkOutputEn ? 1U : 0U) | /*PRQA S 2985*/
                    TRNG_CTRL_OSC_DIV(trngCfgSetting.oscDiv) |
                    TRNG_CTRL_FRQ_CT_IE(trngCfgSetting.frqCntInterruptEn ? 1U : 0U) | /*PRQA S 2985*/
                    TRNG_CTRL_ENT_VAL_IE(trngCfgSetting.entroptValidInterruptEn ? 1U : 0U) | /*PRQA S 2985*/
                    TRNG_CTRL_HW_ERR_IE(trngCfgSetting.hwErrInterruptEn ? 1U : 0U); /*PRQA S 2985*/

    base->CTRL = temp;
    /* Re-lock the protected control field after programming CTRL. */
    base->CTRL &= ~TRNG_CTRL_UNLOCK(1);
}

/*!
 * @brief Program the SDCTL register from trngCfgSetting.
 */
void TRNG_Set_SDCTL(TRNG_Type *const base)
{
    /* Open the protected control field before programming SDCTL. */
    base->CTRL |= TRNG_CTRL_UNLOCK(1);

    base->SDCTL = TRNG_SDCTL_ENT_DLY(trngCfgSetting.entropyDelay) | /*PRQA S 2985*/
                  TRNG_SDCTL_SAMP_SIZE(trngCfgSetting.sampleSize); /*PRQA S 2985*/

    /* Re-lock the protected control field after programming SDCTL. */
    base->CTRL &= ~TRNG_CTRL_UNLOCK(1);
}

/*!
 * @brief Program the FRQMIN register from trngCfgSetting.
 */
void TRNG_Set_FRQMIN(TRNG_Type *const base)
{
    /* Open the protected control field before programming FRQMIN. */
    base->CTRL |= TRNG_CTRL_UNLOCK(1);
    base->FRQMIN = TRNG_FRQMIN_FRQ_MIN(trngCfgSetting.frqMin); /*PRQA S 2985 */

    /* Re-lock the protected control field after programming FRQMIN. */
    base->CTRL &= ~TRNG_CTRL_UNLOCK(1);
}

/*!
 * @brief Program the FRQMAX register from trngCfgSetting.
 */
void TRNG_Set_FRQMAX(TRNG_Type *const base)
{
    /* Open the protected control field before programming FRQMAX. */
    base->CTRL |= TRNG_CTRL_UNLOCK(1);
    base->FRQMAX = TRNG_FRQMAX_FRQ_MAX(trngCfgSetting.frqMax); /*PRQA S 2985 */

    /* Re-lock the protected control field after programming FRQMAX. */
    base->CTRL &= ~TRNG_CTRL_UNLOCK(1);
}

/*!
 * @brief Program the SCML register from trngCfgSetting.
 */
void TRNG_Set_SCML(TRNG_Type *const base)
{
    /* Open the protected control field before programming SCML. */
    base->CTRL |= TRNG_CTRL_UNLOCK(1);
    base->SCML = TRNG_SCML_MONO_MAX(trngCfgSetting.monobitMax) | /*PRQA S 2985 */
                 TRNG_SCML_MONO_RNG(trngCfgSetting.monobitRange); /*PRQA S 2985 */

    /* Re-lock the protected control field after programming SCML. */
    base->CTRL &= ~TRNG_CTRL_UNLOCK(1);
}

/*******************************************************************************
 * Data & Status Access
 ******************************************************************************/

/*!
 * @brief Read the frequency-count result from the FRQCNT register.
 */
uint32_t TRNG_Get_FRQCNT(TRNG_Type *const base)
{
    while ((base->STAT & TRNG_STAT_FCT_VAL_MASK) == 0U)
    {
        /* Wait until the frequency-count result becomes valid. */
    }

    return ((base->FRQCNT & TRNG_FRQMAX_FRQ_MAX_MASK) >> TRNG_FRQMAX_FRQ_MAX_SHIFT);
}

/*!
 * @brief Read the monobit count from the SCMC register.
 */
uint32_t TRNG_Get_SCMC(TRNG_Type *const base)
{
    return ((base->SCMC & TRNG_SCMC_MONO_CT_MASK) >> TRNG_SCMC_MONO_CT_SHIFT);
}

/*!
 * @brief Read one entropy word from the ENT register bank.
 */
uint32_t TRNG_Get_ENT(TRNG_Type *const base, uint8_t index)
{
    return (base->ENT[index]);
}

/*!
 * @brief Get the state of one TRNG status condition.
 */
bool TRNG_GetStatusFlag(TRNG_Type *const base, trng_status_flag_t statusFlag)
{
    return (((base->STAT) >> (uint8_t)statusFlag) & 1U) != 0U;
}

/*!
 * @brief Clear one TRNG status condition or all writable status bits.
 */
status_t TRNG_ClearStatusFlag(TRNG_Type *const base, trng_status_flag_t statusFlag)
{
    if (statusFlag == TRNG_ALL_STATUS)
    {
        base->STAT |= (uint32_t)TRNG_ALL_STATUS;
    }
    else
    {
        base->STAT |= ((uint32_t)1U << (uint32_t)statusFlag);
    }

    return STATUS_SUCCESS;
}
/*******************************************************************************
 * EOF
 ******************************************************************************/
