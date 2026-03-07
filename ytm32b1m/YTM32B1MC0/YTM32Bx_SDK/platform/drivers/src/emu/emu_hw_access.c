/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file emu_hw_access.c
 * @version 1.4.0
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 2985 Rule 2.2 : This operation is redundant. The value of the result is 
 *                        always that of the left-hand operand.
 *
 */

#include "emu_hw_access.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_Init
 * Description   : This function restores the EMU module to reset value.
 *
 *END**************************************************************************/
void EMU_Init(EMU_Type *const base)
{
    uint8_t idx = 0U;
    base->EIGCR = 0x00000000U;
    base->EICHCR = 0x00000000U;
#if (EMU_EICHD_COUNT > 1U)
    for (idx = 0; idx < EMU_EICHD_COUNT; idx++)
#endif
    {
        base->EICHD[idx].CHKBIT = 0x00000000U;
#if defined(FEATURE_EMU_SUPPORT_64BIT_INJECTION) && (FEATURE_EMU_SUPPORT_64BIT_INJECTION == 1U)
        base->EICHD[idx].DATAL = 0x00000000U;
        base->EICHD[idx].DATAH = 0x00000000U;
#else
        base->EICHD[idx].DATA = 0x00000000U;
#endif /* FEATURE_EMU_SUPPORT_64BIT_INJECTION */
        base->EICHD[idx].ADDR = 0x00000000U;
        
        base->ERINFO[idx].CNT = 0x00000000U;
    }

    base->ERIE = 0x00000000U;
    base->ERIF = 0xFFFFFFFFU;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_ClearErrReportCnt
 * Description   : This function clear report count of EMU module.
 *
 *END**************************************************************************/
void EMU_ClearErrReportCnt(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);
    base->ERINFO[channel].CNT = 0x00U & EMU_ERINFO_CNT_CNT_MASK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_Disable
 * Description   : This function disable the EMU module.
 *
 *END**************************************************************************/
void EMU_Disable(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    base->EICHCR &= (~((uint32_t)1U << channel)); //enable channel
    base->EIGCR = 0x00; //enable Gen
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_Enable
 * Description   : This function enable the EMU module.
 *
 *END**************************************************************************/
void EMU_Enable(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    base->EICHCR |= ((uint32_t)1U << channel);
    #if ((defined FEATURE_EMU_GLOBAL_ENABLE_KEY) && ( FEATURE_EMU_GLOBAL_ENABLE_KEY == 1))
    base->EIGCR = 0x89abcdefU;
    #else
    base->EIGCR = 0x01U;
    #endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_GetErrReportCnt
 * Description   : This function get the error count of EMU module.
 *
 *END**************************************************************************/
uint32_t EMU_GetErrReportCnt(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);
    return base->ERINFO[channel].CNT;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_SetErrInjectAddr
 * Description   : This function set the inject error address of EMU module.
 *
 *END**************************************************************************/
void EMU_SetErrInjectAddr(EMU_Type *const base, uint8_t channel, volatile uint32_t addr)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
    base->EICHD[channel].ADDR = addr;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_SetErrInjectChkbit
 * Description   : This function set the inject error check bit of EMU module.
 *
 *END**************************************************************************/
void EMU_SetErrInjectChkbit(EMU_Type *const base, uint8_t channel, emu_chkbit_type chkbit)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
    base->EICHD[channel].CHKBIT = (uint32_t)EMU_EICHD_CHKBIT_CHKBIT_MASK & ((uint32_t)1U << chkbit);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_SetErrInjectChkMulti
 * Description   : This function set the multiple injection error check bits of EMU module.
 *
 *END**************************************************************************/
void EMU_SetErrInjectChkMulti(EMU_Type *const base, uint8_t channel, emu_chkbit_type chkMulti)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    base->EICHD[channel].CHKBIT = (uint32_t)EMU_EICHD_CHKBIT_CHKBIT_MASK & chkMulti;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_ClearChkbit
 * Description   : This function clear the inject error check bit of EMU module.
 *
 *END**************************************************************************/
void EMU_ClearChkbit(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    base->EICHD[channel].CHKBIT = 0x00;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_SetErrInjectData
 * Description   : This function set the inject error data bit of EMU module.
 *
 *END**************************************************************************/
void EMU_SetErrInjectData(EMU_Type *const base, uint8_t channel, emu_databit_type databit)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

#if defined(FEATURE_EMU_SUPPORT_64BIT_INJECTION) && (FEATURE_EMU_SUPPORT_64BIT_INJECTION == 1U)
    if(databit < 32U)
    {
        base->EICHD[channel].DATAL = ((uint32_t)0x1U << databit);
    }
    else
    {
        base->EICHD[channel].DATAH = ((uint32_t)0x1U << (databit-32U));
    }
#else
    base->EICHD[channel].DATA = ((uint32_t)0x1U << databit);
#endif /* FEATURE_EMU_SUPPORT_64BIT_INJECTION */
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_SetErrInjectDataMulti
 * Description   : This function set the inject multi error data bits of EMU module.
 *
 *END**************************************************************************/
void EMU_SetErrInjectDataMulti(EMU_Type *const base, uint8_t channel, emu_databit_type dataMulti)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

#if defined(FEATURE_EMU_SUPPORT_64BIT_INJECTION) && (FEATURE_EMU_SUPPORT_64BIT_INJECTION == 1U)
    base->EICHD[channel].DATAL = (uint32_t)(dataMulti);
    base->EICHD[channel].DATAH = (uint32_t)((dataMulti >> 32U));
#else
    base->EICHD[channel].DATA = (uint32_t)(dataMulti);
#endif /* FEATURE_EMU_SUPPORT_64BIT_INJECTION */
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_ClearData
 * Description   : This function clear the inject error data bit of EMU module.
 *
 *END**************************************************************************/
void EMU_ClearData(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
#if defined(FEATURE_EMU_SUPPORT_64BIT_INJECTION) && (FEATURE_EMU_SUPPORT_64BIT_INJECTION == 1U)
    base->EICHD[channel].DATAL = 0x00;
    base->EICHD[channel].DATAH = 0x00;
#else
    base->EICHD[channel].DATA = 0x00;
#endif /* FEATURE_EMU_SUPPORT_64BIT_INJECTION */
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_GetErrReportAddr
 * Description   : This function get the report error address of EMU module.
 *
 *END**************************************************************************/
uint32_t EMU_GetErrReportAddr(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    return base->ERINFO[channel].ADDR;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_EnableInterruptSignalBit
 * Description   : This function enable the interrupt signal bit of EMU module.
 *
 *END**************************************************************************/
void EMU_EnableInterruptSignalBit(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);

    base->ERIE |= EMU_ERIE_SCIE((uint32_t)1U << channel); /*PRQA S 2985*/
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_DisableInterruptSignalBit
 * Description   : This function disable the interrupt signal bit of EMU module.
 *
 *END**************************************************************************/
void EMU_DisableInterruptSignalBit(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);

    base->ERIE &= ~EMU_ERIE_SCIE((uint32_t)1U << channel); /*PRQA S 2985*/
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_ClearInterruptFlagSignalBit
 * Description   : This function clear the interrupt signal bit flag of EMU module.
 *
 *END**************************************************************************/
void EMU_ClearInterruptFlagSignalBit(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);

    base->ERIF = EMU_ERIF_SCIF((uint32_t)1U << channel); /*PRQA S 2985*/
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_EnableInterruptDoubleBit
 * Description   : This function enable the interrupt double bit of EMU module.
 *
 *END**************************************************************************/
void EMU_EnableInterruptDoubleBit(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);

    base->ERIE |= EMU_ERIE_NCIE((uint32_t)1U << channel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_DisableInterruptDoubleBit
 * Description   : This function disable the interrupt double bit of EMU module.
 *
 *END**************************************************************************/
void EMU_DisableInterruptDoubleBit(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);

    base->ERIE &= ~EMU_ERIE_NCIE((uint32_t)1U << channel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_ClearInterruptFlagDoubleBit
 * Description   : This function clear the interrupt double bit of EMU module.
 *
 *END**************************************************************************/
void EMU_ClearInterruptFlagDoubleBit(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);

    base->ERIF = EMU_ERIF_NCIF((uint32_t)1U << channel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_GetSyndrome
 * Description   : This function get the syndrome of EMU module.
 *
 *END**************************************************************************/
uint32_t EMU_GetSyndrome(EMU_Type *const base, uint8_t channel)
{
    return (base->ERINFO[channel].SYND & EMU_ERINFO_SYND_SYNDROME_MASK) >> EMU_ERINFO_SYND_SYNDROME_SHIFT;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_GetChannelSBInterruptFlag
 * Description   : This function gets single bit error correct check interrupt flag.
 *
 *END**************************************************************************/
uint8_t EMU_GetChannelSBInterruptFlag(EMU_Type *const base, uint8_t channel)
{
    return (uint8_t)((((base->ERIF & EMU_ERIF_SCIF_MASK) >> EMU_ERIF_SCIF_SHIFT) >> channel));
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
