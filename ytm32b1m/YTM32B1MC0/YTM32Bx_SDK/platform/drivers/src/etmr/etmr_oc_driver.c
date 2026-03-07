/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_oc_driver.c
 * @version 1.4.0
 */

#include "etmr_oc_driver.h"
#include "etmr_hw_access.h"
#include "interrupt_manager.h"


/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_InitOutputCompare
 * Description   : Configures the eTMR to generate timed pulses
 * When the eTMR counter matches the value of compareVal argument (this is
 * written into CHn_VAL, CHn_VAL1 register), the channel output is changed based on what is specified
 * in the compareMode argument.
 *
 * Implements    : eTMR_DRV_InitOutputCompare_Activity
 *END**************************************************************************/
status_t eTMR_DRV_InitOutputCompare(uint32_t instance, const etmr_oc_param_t *param)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(param != NULL);
    DEV_ASSERT(param->maxCountValue > 0U);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    uint8_t index;
    uint8_t hwChannel;
    etmr_state_t *state = etmrStatePtr[instance];
    status_t retStatus = STATUS_SUCCESS;

    if (NULL != state)
    {
        /* Disable eTMR counter */
        eTMR_Disable(etmrBase);
        /* Clear the overflow flag */
        eTMR_DRV_ClearTofFlag(instance);

#if FEATURE_eTMR_HAS_CNT_INIT_SRC
        /* Configure counter initial value */
        if (param->counterInitValFromInitReg)
        {
            /* Set first count from CNT */
            eTMR_SetCounterInitValSrc(etmrBase, (uint8_t)COUNTER_VAL_FROM_INIT);
            /* Configure INIT register, count from 0 */
            eTMR_SetInitVal(etmrBase, 0U);
        }
        else
#endif
        {
            /* Configure CNT register */
            eTMR_SetCntVal(etmrBase, param->cntVal);
#if FEATURE_eTMR_HAS_INIT_REG
            /* Configure INIT register, count from 0 */
            eTMR_SetInitVal(etmrBase, 0U);
#endif
        }

        for (index = 0U; index < param->nNumOutputChannels; index++)
        {
            hwChannel = param->outputChannelConfig[index].hwChannelId;
            eTMR_SetChnOutInitVal(etmrBase, hwChannel, (uint32_t)(param->outputChannelConfig[index].channelInitVal));
            eTMR_InitChnOutput(etmrBase, hwChannel);
        }

        /* Set MOD value */
        eTMR_SetMod(etmrBase, param->maxCountValue);
        /* Disable qd counter */
        eTMR_DisableQuadDecoder(etmrBase);

        /* Use eTMR as counter, disable all the channels */
        for (index = 0U; index < param->nNumOutputChannels; index++)
        {
            DEV_ASSERT(param->maxCountValue >= param->outputChannelConfig[index].cmpVal1);
            hwChannel = param->outputChannelConfig[index].hwChannelId;

            /* Set Channel Output Compare mode */
            eTMR_SetChnMode(etmrBase, hwChannel, eTMR_COMPARE_MODE);

            /* Write initial count value for all channels */
            eTMR_SetChnVal0(etmrBase, hwChannel, param->outputChannelConfig[index].cmpVal0);
            eTMR_SetChnVal1(etmrBase, hwChannel, param->outputChannelConfig[index].cmpVal1);

            /* Set val0 and val1 compare mode */
            eTMR_SelChnVal0MatchingOutputVal(etmrBase, hwChannel, (uint8_t)param->outputChannelConfig[index].val0CmpMode);
            eTMR_SelChnVal1MatchingOutputVal(etmrBase, hwChannel, (uint8_t)param->outputChannelConfig[index].val1CmpMode);

            /* Enable/Disable the generation a trigger on chip module */
            eTMR_IsChnVal0MatchTrigEnable(etmrBase,
                                          hwChannel,
                                          param->outputChannelConfig[index].enableExternalTrigger0);
            eTMR_IsChnVal1MatchTrigEnable(etmrBase,
                                          hwChannel,
                                          param->outputChannelConfig[index].enableExternalTrigger1);

            if (param->outputChannelConfig[index].interruptEnable)
            {
                eTMR_DRV_EnableChnInt(instance, hwChannel);
                INT_SYS_EnableIRQ(g_etmrIrqId[instance][hwChannel]);
            }

            state->etmrChnMode[hwChannel] = eTMR_STATE_OUTPUT_COMPARE;
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
 * Function Name : eTMR_DRV_DeinitOutputCompare
 * Description   : Disables compare match output control and clears compare configuration
 *
 * Implements    : eTMR_DRV_DeinitOutputCompare_Activity
 *END**************************************************************************/
status_t eTMR_DRV_DeinitOutputCompare(uint32_t instance, const etmr_oc_param_t *param)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(param != NULL);
    DEV_ASSERT(etmrStatePtr[instance] != NULL);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    uint8_t index;
    uint8_t hwChannel;

    eTMR_ClearLdok(etmrBase);
    /* Stop the eTMR counter */
    eTMR_Disable(etmrBase);
    /* Clear the overflow flag */
    eTMR_DRV_ClearTofFlag(instance);
#if FEATURE_eTMR_HAS_INIT_REG
    /* Configure INIT register */
    eTMR_SetInitVal(etmrBase, 0);
#endif
#if FEATURE_eTMR_HAS_CNT_INIT_SRC
    /* Restore initial value source */
    eTMR_SetCounterInitValSrc(etmrBase, (uint8_t)COUNTER_VAL_FROM_CNT);
#endif

    for (index = 0U; index < param->nNumOutputChannels; index++)
    {
        hwChannel = param->outputChannelConfig[index].hwChannelId;

        /* Disable Channel Output mode */
        eTMR_SetChnMode(etmrBase, hwChannel, eTMR_CHANNEL_DISABLE);

        /* Write initial count value for all channels to 0 */
        eTMR_SetChnVal0(etmrBase, hwChannel, 0U);
        eTMR_SetChnVal1(etmrBase, hwChannel, 0U);

        /* Set val0 and val1 compare mode to eTMR_OUTPUT_CLR */
        eTMR_SelChnVal0MatchingOutputVal(etmrBase, hwChannel, (uint8_t)eTMR_OUTPUT_CLR);
        eTMR_SelChnVal1MatchingOutputVal(etmrBase, hwChannel, (uint8_t)eTMR_OUTPUT_CLR);

        /* Disable val0 and val1 output trigger */
        eTMR_IsChnVal0MatchTrigEnable(etmrBase, hwChannel, false);
        eTMR_IsChnVal1MatchTrigEnable(etmrBase, hwChannel, false);

        eTMR_DRV_DisableChnInt(instance, hwChannel);
        INT_SYS_DisableIRQ(g_etmrIrqId[instance][hwChannel]);
    }

    /* Clear out the registers */
    eTMR_SetMod(etmrBase, 0U);

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_UpdateOutputCompareChannel
 * Description   : Sets the next compare match value on the given channel starting
 *                 from the current counter value.
 *
 * Implements    : eTMR_DRV_UpdateOutputCompareChannel_Activity
 *END**************************************************************************/
status_t eTMR_DRV_UpdateOutputCompareChannel(uint32_t instance,
                                             uint8_t channel,
                                             uint16_t nextCmpMatchVal0,
                                             uint16_t nextCmpMatchVal1,
                                             etmr_oc_mode_t val0CmpMode,
                                             etmr_oc_mode_t val1CmpMode,
                                             bool softwareTrigger)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    status_t status = STATUS_SUCCESS;
    eTMR_Type *const etmrBase = g_etmrBase[instance];

    if (nextCmpMatchVal0 <= nextCmpMatchVal1)
    {
        /* Set CHn_VAL0 and CHn_VAL1 */
        eTMR_SetChnVal0(etmrBase, channel, nextCmpMatchVal0);
        eTMR_SetChnVal1(etmrBase, channel, nextCmpMatchVal1);

        /* Set Value0/1 compare mode */
        eTMR_SelChnVal0MatchingOutputVal(etmrBase, channel, (uint8_t)val0CmpMode);
        eTMR_SelChnVal1MatchingOutputVal(etmrBase, channel, (uint8_t)val1CmpMode);

        /* Set software trigger */
        if (softwareTrigger)
        {
            eTMR_SetLdok(etmrBase);
            eTMR_GenSoftwareTrigger(etmrBase, softwareTrigger);
        }
    }
    else
    {
        status = STATUS_ERROR;
    }

    return status;
}

/*! @}*/
/*******************************************************************************
* EOF
******************************************************************************/
