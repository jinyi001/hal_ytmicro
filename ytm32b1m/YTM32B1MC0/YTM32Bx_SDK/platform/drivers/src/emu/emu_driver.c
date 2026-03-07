/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file emu_driver.c
 * @version 1.4.0
 */

#include "emu_driver.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Table of base addresses for EMU instances. */
static EMU_Type *g_emuBase[EMU_INSTANCE_COUNT] = EMU_BASE_PTRS;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name :     EMU_DRV_SetConfig
 * Description   :     This function configures the EMU with all the user
 *  defined in-out mappings.
 * 
 *END**************************************************************************/
static void EMU_DRV_SetConfig(uint32_t instance, const emu_config_t *configPtr)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(configPtr != NULL);


    uint8_t channel = (uint8_t)configPtr->channel;
    uint32_t injectErrAddr = configPtr->injectErrAddr;
    emu_databit_type dataBit = (uint32_t)configPtr->injectErrSetInfo.dataBit;
    emu_chkbit_type chkBit = (uint32_t)configPtr->injectErrSetInfo.chkBit;
    emu_inject_bits_type injectBitsType = configPtr->injectErrSetInfo.injectBitsType;
    bool interruptEnable = configPtr->interruptEnable;

    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    EMU_Type *const base = g_emuBase[instance];

   
    /* Set error inject address.*/
    EMU_SetErrInjectAddr(base, channel, injectErrAddr);
    if (injectBitsType == EMU_INJECT_BITS_TYPE_CHK)
    {
        /* Set error inject check bit.*/
        if(chkBit != NO_INJECTION_ERROR)
        {
            EMU_SetErrInjectChkbit(base, channel, chkBit);
        }
    } else if (injectBitsType == EMU_INJECT_BITS_TYPE_DATA)
    {
        /* Set error inject data.*/
        if(dataBit != NO_INJECTION_ERROR)
        {
            EMU_SetErrInjectData(base, channel, dataBit);
        }
    } else if (injectBitsType == EMU_INJECT_BITS_TYPE_MIXTURE)
    {
        /* Set error inject check bit.*/
        if(chkBit != NO_INJECTION_ERROR)
        {
            EMU_SetErrInjectChkbit(base, channel, chkBit);
        }
        /* Set error inject data bit.*/
        if(dataBit != NO_INJECTION_ERROR)
        {
            EMU_SetErrInjectData(base, channel, dataBit);
        }
    }else
    {
        /* Do nothing */
    }

    if (true == interruptEnable)
    {
        /* Enable interrupt signal bit.*/
        EMU_EnableInterruptSignalBit(base, channel);
        /* Enable interrupt double bit.*/
        EMU_EnableInterruptDoubleBit(base, channel);
    } else
    {
        /* Disable interrupt signal bit.*/
        EMU_DisableInterruptSignalBit(base, channel);
        /* Disable interrupt double bit.*/
        EMU_DisableInterruptDoubleBit(base, channel);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name :     EMU_DRV_InjectError
 * Description   :     This function inject error .
 * 
 *END**************************************************************************/
void EMU_DRV_InjectError(uint32_t instance, uint32_t channel,uint32_t injectErrAddr,emu_databit_type dataMulti,emu_chkbit_type chkMulti)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    EMU_Type *const base = g_emuBase[instance];

    EMU_Disable(base, (uint8_t)channel);
    
    /* Set error injection address.*/
    EMU_SetErrInjectAddr(base, (uint8_t)channel, injectErrAddr);
   
    /* Set multiple error injection check bits.*/
    if(chkMulti != NO_INJECTION_ERROR)
    {
        EMU_SetErrInjectChkMulti(base, (uint8_t)channel, chkMulti);
    }

    /* Set multiple error injection data bits.*/
    if(dataMulti != NO_INJECTION_ERROR)
    {
        EMU_SetErrInjectDataMulti(base, (uint8_t)channel, dataMulti);
    }

    EMU_Enable(base, (uint8_t)channel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name :     EMU_DRV_Init
 * Description   :     This function first resets the source triggers of all EMU target modules
 * to their default values, then configures the EMU with all the user defined in-out mappings.
 * This example shows how to set up the emu_user_config_t parameters and how to call the
 * EMU_DRV_Init() function with the required parameters:
 *   emu_config_t             emuConfig;
 * 
 *   EMU_DRV_Init(instance, &trngConfig0);
 *END**************************************************************************/
void EMU_DRV_Init(uint32_t instance, const emu_config_t *configPtr, uint32_t startFlag)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(configPtr != NULL);

    EMU_Type *const base = g_emuBase[instance];
    uint8_t injectErrChannel = (uint8_t)configPtr->channel;

    /* Reset all the register of EMU module. */
    EMU_Init(base);
    
    EMU_DRV_SetConfig(instance, configPtr);

    if (startFlag == 1U)
    {
        EMU_Enable(base, injectErrChannel);
    }
    else
    {
        EMU_Disable(base, injectErrChannel);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_DRV_Deinit
 * Description   : Reset to default values the source triggers corresponding to all target modules.
 *
 *END**************************************************************************/
void EMU_DRV_DeInit(uint32_t instance, uint32_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    EMU_Type *const base = g_emuBase[instance];
    /* Disable the EMU module. */
    EMU_Disable(base, (uint8_t)channel);
    /* Reset all the register of EMU module. */
    EMU_Init(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_DRV_GetErrReportChkbit
 * Description   : Get error report of check bit
 *
 *END**************************************************************************/
uint8_t EMU_DRV_GetErrReportChkbit(uint32_t instance, uint8_t channel)
{
    uint8_t chkbit = 0;
    volatile uint32_t value;
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);
    EMU_Type *const base = g_emuBase[instance];

    value = EMU_GetSyndrome(base, channel);
    while (value > 0U)
    {
        value = value >> 1U;
        if (value == 0U)
        {
            break;
        } else
        {
            chkbit++;
        }
    }
    return chkbit;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_DRV_ClearInterruptFlagSignalBit
 * Description   : Clear interrupt flag of signal bit.
 *
 *END**************************************************************************/
void EMU_DRV_ClearInterruptFlagSignalBit(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
    EMU_Type *const base = g_emuBase[instance];

    EMU_ClearInterruptFlagSignalBit(base, channel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_DRV_EnableInterruptDoubleBit
 * Description   : Enale interrupt  of double bit.
 *
 *END**************************************************************************/
void EMU_DRV_EnableInterruptDoubleBit(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
    EMU_Type *const base = g_emuBase[instance];

    EMU_EnableInterruptDoubleBit(base, channel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_DRV_DisableInterruptDoubleBit
 * Description   : Disable interrupt  of double bit.
 *
 *END**************************************************************************/
void EMU_DRV_DisableInterruptDoubleBit(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
    EMU_Type *const base = g_emuBase[instance];

    EMU_DisableInterruptDoubleBit(base, channel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_DRV_ClearInterruptFlagDoubleBit
 * Description   : Clear interrupt flag of double bit.
 *
 *END**************************************************************************/
void EMU_DRV_ClearInterruptFlagDoubleBit(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
    EMU_Type *const base = g_emuBase[instance];

    EMU_ClearInterruptFlagDoubleBit(base, channel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_DRV_GetErrReportCnt
 * Description   : Get error report count.
 *
 *END**************************************************************************/
uint32_t EMU_DRV_GetErrReportCnt(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
    EMU_Type *const base = g_emuBase[instance];

    return EMU_GetErrReportCnt(base, channel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_DRV_ClearErrReportCnt
 * Description   : Clear error report count.
 *
 *END**************************************************************************/
void EMU_DRV_ClearErrReportCnt(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);
    EMU_Type *const base = g_emuBase[instance];
    EMU_ClearErrReportCnt(base, channel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_DRV_GetErrReportAddr
 * Description   : Get error report address.
 *
 *END**************************************************************************/
uint32_t EMU_DRV_GetErrReportAddr(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    EMU_Type *const base = g_emuBase[instance];
    return EMU_GetErrReportAddr(base, channel);

}

/*FUNCTION**********************************************************************
 *
 * Function Name : EMU_GetChannelSBInterruptFlag
 * Description   : This function gets single bit error correct check interrupt flag.
 *
 *END**************************************************************************/
uint8_t EMU_DRV_GetChannelSBInterruptFlag(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    EMU_Type *const base = g_emuBase[instance];
    return EMU_GetChannelSBInterruptFlag(base, channel);
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
