/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*!
 * @file trng_hw_access.c
 * @version 1.4.0
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 2985 Rule 2.2 : This operation is redundant. The value of the result is 
 *                        always that of the left-hand operand.
 */

#include "trng_hw_access.h"

trng_config_setting_t trngCfgSetting;

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_Enable
 * Description   : This function enable the TRNG module.
 *
 *END**************************************************************************/
void TRNG_Enable(TRNG_Type *const base)
{
    base->CTRL |= TRNG_CTRL_TRNG_EN(1);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_Init
 * Description   : This function restores the TRNG module to reset value.
 *
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_Disable
 * Description   : This function disable the TRNG module.
 *
 *END**************************************************************************/
void TRNG_Disable(TRNG_Type *const base)
{
    base->CTRL &= TRNG_CTRL_TRNG_EN(0);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_SWReset
 * Description   : This function reset the TRNG module by software.
 *
 *END**************************************************************************/
void TRNG_SWReset(TRNG_Type *const base)
{
    uint32_t temp = 0;
    temp = base->CTRL;
    /*Enable unlock, the control bits are able to be modified.*/
    temp |= TRNG_CTRL_UNLOCK(1) | TRNG_CTRL_SWR(1) ;
    /*Disable unlock, write to the control bits has bo effect.*/
    temp &= ~TRNG_CTRL_UNLOCK(1);
    base->CTRL = temp; 
}
/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_Set_CTRL
 * Description   : This function set the CTRL register of TRNG module.
 *
 *END**************************************************************************/
void TRNG_Set_CTRL(TRNG_Type *const base)
{
    uint32_t temp = TRNG_CTRL_UNLOCK(1)|
                    TRNG_CTRL_RTY_CT(trngCfgSetting.retryCnt)|
                    TRNG_CTRL_LRUN_MAX(trngCfgSetting.lrunMax)|
                    TRNG_CTRL_FOR_SCLK(trngCfgSetting.forceSystemClockEn ? 1U : 0U)| /*PRQA S 2985*/
                    TRNG_CTRL_CLK_OUT(trngCfgSetting.clkOutputEn ? 1U : 0U)| /*PRQA S 2985*/
                    TRNG_CTRL_OSC_DIV(trngCfgSetting.oscDiv)|
                    TRNG_CTRL_FRQ_CT_IE(trngCfgSetting.frqCntInterruptEn ? 1U : 0U)| /*PRQA S 2985*/
                    TRNG_CTRL_ENT_VAL_IE(trngCfgSetting.entroptValidInterruptEn ? 1U : 0U)| /*PRQA S 2985*/
                    TRNG_CTRL_HW_ERR_IE(trngCfgSetting.hwErrInterruptEn ? 1U : 0U); /*PRQA S 2985*/

    base->CTRL = temp;
    /*Disable unlock, write to the control bits has bo effect.*/
    base->CTRL &= ~TRNG_CTRL_UNLOCK(1);              
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_Set_SDCTL
 * Description   : This function set the SDCTL register of TRNG module.
 *
 *END**************************************************************************/
void TRNG_Set_SDCTL(TRNG_Type * const base)
{
    /*Enable unlock, the control bits are able to be modified.*/
    base->CTRL |= TRNG_CTRL_UNLOCK(1)  ;

    base->SDCTL = TRNG_SDCTL_ENT_DLY(trngCfgSetting.entropyDelay)| /*PRQA S 2985*/
                    TRNG_SDCTL_SAMP_SIZE(trngCfgSetting.sampleSize); /*PRQA S 2985*/

    /*Disable unlock, write to the control bits has bo effect.*/
    base->CTRL &= ~TRNG_CTRL_UNLOCK(1)  ;  
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_Set_FRQMIN
 * Description   : This function set the FRQMIN register of TRNG module.
 *
 *END**************************************************************************/
void TRNG_Set_FRQMIN(TRNG_Type *const base)
{
    /*Enable unlock, the control bits are able to be modified.*/
    base->CTRL |= TRNG_CTRL_UNLOCK(1);
    base->FRQMIN = TRNG_FRQMIN_FRQ_MIN(trngCfgSetting.frqMin); /*PRQA S 2985 */

    /*Disable unlock, write to the control bits has bo effect.*/
    base->CTRL &= ~TRNG_CTRL_UNLOCK(1);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_Set_FRQMAX
 * Description   : This function set the FRQMAX register of TRNG module.
 *
 *END**************************************************************************/
void TRNG_Set_FRQMAX(TRNG_Type *const base)
{
    /*Enable unlock, the control bits are able to be modified.*/
    base->CTRL |= TRNG_CTRL_UNLOCK(1);
    base->FRQMAX = TRNG_FRQMAX_FRQ_MAX(trngCfgSetting.frqMax); /*PRQA S 2985 */

    /*Disable unlock, write to the control bits has bo effect.*/
    base->CTRL &= ~TRNG_CTRL_UNLOCK(1); 
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_Get_FRQCNT
 * Description   : This function get the FRQCNT register value.
 *
 *END**************************************************************************/
uint32_t TRNG_Get_FRQCNT(TRNG_Type *const base)
{
    while(((base->STAT >> TRNG_STAT_FCT_VAL_SHIFT) & TRNG_STAT_FCT_VAL_MASK) == 0U)
    {
        /* Wait for count for trng_osc_clk is ready to read */
    }
    return (base->FRQCNT>>TRNG_FRQMAX_FRQ_MAX_SHIFT) & TRNG_FRQMAX_FRQ_MAX_MASK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_Set_SCML
 * Description   : This function set the SCML register of TRNG module.
 *
 *END**************************************************************************/
void TRNG_Set_SCML(TRNG_Type *const base)
{
    /* Enable unlock, the control bits are able to be modified. */
    base->CTRL |= TRNG_CTRL_UNLOCK(1)  ;
    base->SCML = TRNG_SCML_MONO_MAX(trngCfgSetting.monobitMax)| /*PRQA S 2985 */
                    TRNG_SCML_MONO_RNG(trngCfgSetting.monobitRange); /*PRQA S 2985 */

    /*Disable unlock, write to the control bits has bo effect.*/
    base->CTRL &= ~TRNG_CTRL_UNLOCK(1)  ;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_Get_SCMC
 * Description   : This function get the SCMC register value of TRNG module.
 *
 *END**************************************************************************/
uint32_t TRNG_Get_SCMC(TRNG_Type *const base)
{
    return (base->SCMC >>TRNG_SCMC_MONO_CT_SHIFT) & TRNG_SCMC_MONO_CT_MASK;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_Get_ENT
 * Description   : This function get the ENT register value of TRNG module.
 *
 *END**************************************************************************/
uint32_t TRNG_Get_ENT(TRNG_Type *const base , uint8_t index)
{
    return (base->ENT[index]);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_GetStatusFlag
 * Description   : This function returns the state of one of the Trng status flags 
 * as requested by the user.
 *
 *END**************************************************************************/
bool TRNG_GetStatusFlag(TRNG_Type *const base, trng_status_flag_t statusFlag)
{
    return (((base->STAT) >> (uint8_t)statusFlag) & 1U) != 0U;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_ClearStatusFlag
 * Description   : This function clears the state of one of the TRNG status flags 
 * as requested by the user.
 *
 *END**************************************************************************/
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

