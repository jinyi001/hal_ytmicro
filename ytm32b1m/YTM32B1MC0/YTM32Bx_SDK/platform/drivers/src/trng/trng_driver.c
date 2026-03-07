/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file trng_driver.c
 * @version 1.4.0
 */

#include "trng_driver.h"
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Table of base addresses for TRNG instances. */
static TRNG_Type * const g_trngBase[TRNG_INSTANCE_COUNT] = TRNG_BASE_PTRS ;


/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_DRV_Get_Ent
 * Description   : This function read the entropy value.
 * Reading the highest offset(ENT(7)) will clear the entire entropy value
 * and start a new entropy generation.  
 *
 *END**************************************************************************/
void TRNG_DRV_Get_Ent(uint32_t instance,uint32_t pEntValue[])
{
    DEV_ASSERT(instance < TRNG_INSTANCE_COUNT);
    TRNG_Type * const base = g_trngBase[instance];

    if(pEntValue != NULL)
    {
        pEntValue[0] = TRNG_Get_ENT(base,0);
        pEntValue[1] = TRNG_Get_ENT(base,1);
        pEntValue[2] = TRNG_Get_ENT(base,2);
        pEntValue[3] = TRNG_Get_ENT(base,3);
#if !defined(FEATURE_TRNG_ENTROPY_COUNT_OPTIMIZE) || (FEATURE_TRNG_ENTROPY_COUNT_OPTIMIZE != 1)
        pEntValue[4] = TRNG_Get_ENT(base,4);
        pEntValue[5] = TRNG_Get_ENT(base,5);
        pEntValue[6] = TRNG_Get_ENT(base,6);
        pEntValue[7] = TRNG_Get_ENT(base,7);
#endif /* FEATURE_TRNG_ENTROPY_COUNT_OPTIMIZE */
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_DRV_GetDefaultConfig
 * Description   : Get default config of TRNG.
 *
 *END**************************************************************************/
static void TRNG_DRV_GetDefaultConfig(void)
{
    trngCfgSetting.lrunMax = 0x22;
    trngCfgSetting.retryCnt = 0x0F;
    trngCfgSetting.forceSystemClockEn = false;
    trngCfgSetting.clkOutputEn = false;
    trngCfgSetting.oscDiv = TRNG_OSC_NO_DEVIDER;
    trngCfgSetting.frqCntInterruptEn = false;
    trngCfgSetting.entroptValidInterruptEn = false;
    trngCfgSetting.hwErrInterruptEn = false;
    trngCfgSetting.sampleSize = 0x200;
    trngCfgSetting.entropyDelay = 0x1A0;

    trngCfgSetting.frqMin = 0x00000010;
    trngCfgSetting.frqMax = 0x003FFFFF;
    trngCfgSetting.monobitRange = 0x2F0;
    trngCfgSetting.monobitMax = 0x300;
    trngCfgSetting.pEntValue = NULL;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_DRV_Init
 * Description   : This function first resets the source triggers of all TRNG target modules
 * to their default values, then configures the TRNG with all the user defined in-out mappings. 
 *
 *END**************************************************************************/
void TRNG_DRV_Init(uint32_t instance, uint32_t entroyDelay)
{
    DEV_ASSERT(instance < TRNG_INSTANCE_COUNT);

    TRNG_Type * const base = g_trngBase[instance];

    if(entroyDelay <= 0xFFFFU)
    {
        /* Reset the TRNG registers to their default state, including disabling the TRNG */
        TRNG_HW_Init(base);
        TRNG_DRV_GetDefaultConfig();
        trngCfgSetting.entropyDelay = entroyDelay;

        TRNG_Set_CTRL(base);
        TRNG_Set_FRQMAX(base);
        TRNG_Set_FRQMIN(base);
        TRNG_Set_SCML(base);
        TRNG_Set_SDCTL(base);

        TRNG_Enable(base);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_DRV_DeInit
 * Description   : Reset to default values the source triggers corresponding to all target modules.
 *
 *END**************************************************************************/
void TRNG_DRV_DeInit(uint32_t instance)
{
    DEV_ASSERT(instance < TRNG_INSTANCE_COUNT);
    TRNG_Type * const base = g_trngBase[instance];

    /* Reset the TRNG registers to their default state, including disabling the TRNG */
    TRNG_HW_Init(base);


}

/*FUNCTION**********************************************************************
 *
 * Function Name : TRNG_DRV_IsIDLE
 * Description   : This function return the status of the entropy generation FSM.
 *
 *END**************************************************************************/
status_t TRNG_DRV_GetStatus(uint32_t instance)
{
    DEV_ASSERT(instance < TRNG_INSTANCE_COUNT);
    status_t status = STATUS_ERROR;
    TRNG_Type * const base = g_trngBase[instance];
    bool entropyValid = TRNG_GetStatusFlag(base, TRNG_ENTROPY_VALID);
    bool freqCountValid = TRNG_GetStatusFlag(base, TRNG_FREQUENCY_COUNT_VALID);
    if(TRNG_GetStatusFlag(base,TRNG_BUSY))
    {
        status = STATUS_BUSY;
    }
    else if(entropyValid && freqCountValid)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_ERROR;
    }
    return status;
}
