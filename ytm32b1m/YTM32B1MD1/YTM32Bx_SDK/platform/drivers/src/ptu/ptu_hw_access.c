/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file ptu_hw_access.c
 * @version 1.4.1
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 2985 Rule 2.2 : This operation is redundant. The value of the result is 
 *                        always that of the left-hand operand.
 */

#include "ptu_hw_access.h"

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_Init
 * Description   : Reset PTU's registers to a known state. This state is
 * defined in Reference Manual, which is power on reset value.
 *
 *END*************************************************************************/
void PTU_Init(PTU_Type *const base)
{
    DEV_ASSERT(base != NULL);
    uint32_t preChn;

    base->CTRL = 0U;
    PTU_Enable(base);
    base->MOD = 0xFFFFU;
    base->INTDLY = 0xFFFFU;
    /* For ADC trigger. */
    base->CHCTRL = 0U;
    base->CHSTS = 0U;
    for (preChn = 0U; preChn < PTU_CHDLYn_COUNT; preChn++)
    {
        PTU_SetAdcPreTriggerDelayValue(base, preChn, 0U);
    }

    /* For Pulse out trigger. */
    base->POEN = 0U;
    base->PODLY.PODLY0 = 0U;
    base->PODLY.PODLY1 = 0U;

    /* Load the setting value. */
    PTU_SetLoadValuesCmd(base);
    PTU_Disable(base);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_ConfigTimer
 * Description   : Configure the PTU timer.
 *
 *END*************************************************************************/
void PTU_ConfigTimer(PTU_Type *const base,
                     const ptu_timer_config_t *const configPtr)
{
    DEV_ASSERT(base != NULL);
    DEV_ASSERT(configPtr != NULL);

    uint32_t temp;

    temp = base->CTRL;
    temp &= ~((uint32_t) PTU_CTRL_LDMD_MASK
              | (uint32_t) PTU_CTRL_TRIGSEL_MASK
              | (uint32_t) PTU_CTRL_CONT_MASK
              | (uint32_t) PTU_CTRL_DMAEN_MASK);

    temp |= PTU_CTRL_LDMD((uint32_t) (configPtr->loadValueMode));
    temp |= PTU_CTRL_TRIGSEL((uint32_t) (configPtr->triggerInput));
    if (configPtr->continuousModeEnable)
    {
        temp |= PTU_CTRL_CONT_MASK;
    }
    if (configPtr->dmaEnable)
    {
        temp |= PTU_CTRL_DMAEN_MASK;
    }
    base->CTRL = temp;

    temp = base->PRS;
    temp &= ~((uint32_t) PTU_PRS_DIV_MASK
              | (uint32_t) PTU_PRS_MULT_MASK);
    temp |= PTU_PRS_DIV((uint32_t) (configPtr->clkPreDiv));
    temp |= PTU_PRS_MULT((uint32_t) (configPtr->clkPreMultFactor)); /*PRQA S 2985*/
    base->PRS = temp;

    temp = base->INTE;
    temp &= ~((uint32_t) PTU_INTE_IE_MASK);
    if (configPtr->intEnable)
    {
        temp |= PTU_INTE_IE_MASK;
    }
    base->INTE = temp;

}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_SetAdcPreTriggerOutputEnable
 * Description   : Switch to enable pre-trigger's output.
 *
 *END*************************************************************************/
void PTU_SetAdcPreTriggerOutputEnable(PTU_Type *const base,
                                      uint32_t preChnMask,
                                      bool enable)
{
    DEV_ASSERT(base != NULL);

    uint32_t cr = base->CHCTRL;
    if (enable)
    {
        cr |= PTU_CHCTRL_CHDLYEN(preChnMask); /*PRQA S 2985*/
    } else
    {
        cr &= ~PTU_CHCTRL_CHDLYEN(preChnMask); /*PRQA S 2985*/
    }

    base->CHCTRL = cr;
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_SetAdcPreTriggerEnable
 * Description   : Switch to enable pre-trigger's.
 *
 *END*************************************************************************/
void PTU_SetAdcPreTriggerEnable(PTU_Type *const base,
                                uint32_t preChnMask,
                                bool enable)
{
    DEV_ASSERT(base != NULL);

    uint32_t cr = base->CHCTRL;
    if (enable)
    {
        cr |= PTU_CHCTRL_CHEN(preChnMask); /*PRQA S 2985*/
    } else
    {
        cr &= ~PTU_CHCTRL_CHEN(preChnMask); /*PRQA S 2985*/
    }

    base->CHCTRL = cr;
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_ClearAdcPreTriggerFlags
 * Description   : Clear the flag that the PTU counter reaches to the
 * pre-trigger's delay value.
 *
 *END*************************************************************************/
void PTU_ClearAdcPreTriggerFlags(PTU_Type *const base,
                                 uint32_t preChnMask)
{
    DEV_ASSERT(base != NULL);

    /* Write 0 to clear. */
    uint32_t s = base->CHSTS;
    s |= PTU_CHSTS_CHF(preChnMask); /* Update the change. W1C */
    base->CHSTS = s;
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_SetCmpPulseOutEnable
 * Description   : Switch to enable the pulse-out trigger.
 *
 *END*************************************************************************/
void PTU_SetCmpPulseOutEnable(PTU_Type *const base, bool enable)
{
    DEV_ASSERT(base != NULL);
    uint32_t poen = base->POEN;
    if (enable)
    {
        poen |= PTU_POEN_POEN(1);
    } else
    {
        poen &= ~PTU_POEN_POEN(1);
    }

    base->POEN = poen;
}

/******************************************************************************
 * EOF
 *****************************************************************************/
