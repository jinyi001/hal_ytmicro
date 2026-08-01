/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file trng_driver.c
 * @version 1.4.1
 *
 * @brief TRNG Driver - implementation of the public TRNG_DRV_* API.
 *
 * This file implements the polling-oriented TRNG driver declared in
 * trng_driver.h. The driver resolves the peripheral base address from the
 * instance index and delegates low-level register programming to the hardware
 * access layer in trng_hw_access.h.
 */

#include "trng_driver.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Table of base addresses for TRNG instances. */
static TRNG_Type * const g_trngBase[TRNG_INSTANCE_COUNT] = TRNG_BASE_PTRS;

/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/

/*!
 * @brief Load the internal TRNG configuration shadow with default values.
 */
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

/*!
 * @brief Initialize the TRNG driver and start entropy generation.
 */
void TRNG_DRV_Init(uint32_t instance, uint32_t entroyDelay)
{
    DEV_ASSERT(instance < TRNG_INSTANCE_COUNT);

    TRNG_Type * const base = g_trngBase[instance];

    if (entroyDelay <= 0xFFFFU)
    {
        /* Reset the TRNG block before applying the default configuration. */
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

/*!
 * @brief Reset the TRNG peripheral to its default state.
 */
void TRNG_DRV_DeInit(uint32_t instance)
{
    DEV_ASSERT(instance < TRNG_INSTANCE_COUNT);
    TRNG_Type * const base = g_trngBase[instance];

    /* Restore the TRNG register block to its reset configuration. */
    TRNG_HW_Init(base);
}

/*******************************************************************************
 * Entropy Data Access
 ******************************************************************************/

/*!
 * @brief Read the current entropy block from the TRNG ENT registers.
 */
void TRNG_DRV_Get_Ent(uint32_t instance, uint32_t pEntValue[])
{
    DEV_ASSERT(instance < TRNG_INSTANCE_COUNT);
    TRNG_Type * const base = g_trngBase[instance];

    if (pEntValue != NULL)
    {
        pEntValue[0] = TRNG_Get_ENT(base, 0);
        pEntValue[1] = TRNG_Get_ENT(base, 1);
        pEntValue[2] = TRNG_Get_ENT(base, 2);
        pEntValue[3] = TRNG_Get_ENT(base, 3);
#if !defined(FEATURE_TRNG_ENTROPY_COUNT_OPTIMIZE) || (FEATURE_TRNG_ENTROPY_COUNT_OPTIMIZE != 1)
        pEntValue[4] = TRNG_Get_ENT(base, 4);
        pEntValue[5] = TRNG_Get_ENT(base, 5);
        pEntValue[6] = TRNG_Get_ENT(base, 6);
        pEntValue[7] = TRNG_Get_ENT(base, 7);
#endif /* FEATURE_TRNG_ENTROPY_COUNT_OPTIMIZE */
    }
}

/*******************************************************************************
 * Status Query
 ******************************************************************************/

/*!
 * @brief Get the current entropy-generation status.
 */
status_t TRNG_DRV_GetStatus(uint32_t instance)
{
    DEV_ASSERT(instance < TRNG_INSTANCE_COUNT);
    status_t status = STATUS_ERROR;
    TRNG_Type * const base = g_trngBase[instance];
    bool entropyValid = TRNG_GetStatusFlag(base, TRNG_ENTROPY_VALID);
    bool freqCountValid = TRNG_GetStatusFlag(base, TRNG_FREQUENCY_COUNT_VALID);

    if (TRNG_GetStatusFlag(base, TRNG_BUSY))
    {
        status = STATUS_BUSY;
    }
    else if (entropyValid && freqCountValid)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_ERROR;
    }

    return status;
}
/*******************************************************************************
 * EOF
 ******************************************************************************/
