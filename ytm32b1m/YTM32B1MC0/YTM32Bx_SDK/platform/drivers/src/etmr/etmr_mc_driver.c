/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_mc_driver.c
 * @version 1.4.0
 */

#include "etmr_mc_driver.h"
#include "etmr_hw_access.h"

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_InitCounter
 * Description   : Initializes the eTMR counter.
 *
 * Implements    : eTMR_DRV_InitCounter_Activity
 *END**************************************************************************/
status_t eTMR_DRV_InitCounter(uint32_t instance, const etmr_timer_param_t *timer)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(timer != NULL);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    etmr_state_t *state = etmrStatePtr[instance];
    status_t retStatus = STATUS_SUCCESS;
    uint8_t channel;

    if (NULL != state)
    {
        /* Disable etmr counter */
        eTMR_Disable(etmrBase);
        /* Clear the overflow flag */
        eTMR_DRV_ClearTofFlag(instance);
#if FEATURE_eTMR_HAS_INIT_REG
        /* Set counter just count from INIT register */
        eTMR_SetCounterInitValSrc(etmrBase, 0x1U);
        /* Set counter initial and maximum values */
        eTMR_SetInitVal(etmrBase, timer->initialValue);
#endif
        /* Set modulus */
        eTMR_SetMod(etmrBase, timer->finalValue);
        /* Disable the quadrature decoder mode */
        eTMR_DisableQuadDecoder(etmrBase);
        /* Use eTMR as counter, disable all the channels */
        for (channel = 0U; channel < g_etmrChannelNum[instance]; channel++)
        {
            eTMR_SetChnMode(etmrBase, channel, eTMR_CHANNEL_DISABLE);
        }
    }
    else
    {
        retStatus = STATUS_ERROR;
    }

    return retStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_CounterStart
 * Description   : Starts the eTMR counter.
 *
 * Implements    : eTMR_DRV_CounterStart_Activity
 *END**************************************************************************/
status_t eTMR_DRV_CounterStart(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);

    /* Enable counter */
    eTMR_DRV_Enable(instance);

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_CounterStop
 * Description   : Stops the eTMR counter.
 *
 * Implements    : eTMR_DRV_CounterStop_Activity
 *END**************************************************************************/
status_t eTMR_DRV_CounterStop(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);

    /* Stop the eTMR counter */
    eTMR_DRV_Disable(instance);

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_CounterRead
 * Description   : Reads back the current value of the eTMR counter.
 *
 * Implements    : eTMR_DRV_CounterRead_Activity
 *END**************************************************************************/
uint32_t eTMR_DRV_CounterRead(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *const etmrBase = g_etmrBase[instance];

    return eTMR_GetCntVal(etmrBase);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_MC_DRV_SetDefaultConfig
 * Description   : This function will get the default configuration values
 * in the structure which is used as a common use-case.
 * Return        : None
 * Implements    : eTMR_MC_DRV_GetDefaultConfig_Activity
 *END**************************************************************************/
void eTMR_MC_DRV_SetDefaultConfig(etmr_timer_param_t *const config)
{
    DEV_ASSERT(config != NULL);

    config->initialValue = 0U;
    config->finalValue = 65535U;
}

/*! @}*/
/*******************************************************************************
* EOF
******************************************************************************/
