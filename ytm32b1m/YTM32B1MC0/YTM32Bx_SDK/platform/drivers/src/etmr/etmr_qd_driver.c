/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_qd_driver.c
 * @version 1.4.0
 */

#include "etmr_qd_driver.h"
#include "etmr_hw_access.h"


/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_QuadDecodeStart
 * Description   : Configures the parameters needed and activates quadrature
 *                 decode mode.
 *
 * Implements    : eTMR_DRV_QuadDecodeStart_Activity
 *END**************************************************************************/
status_t eTMR_DRV_QuadDecodeStart(uint32_t instance,
                                  const etmr_qd_config_t *config)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);
    eTMR_Type * etmrBase = g_etmrBase[instance];
    status_t retStatus = STATUS_SUCCESS;

    if (config != NULL)
    {
        /* Disable Quadrature Decoder */
        eTMR_DisableQuadDecoder(etmrBase);

        /* Set quad decoder mode */
        eTMR_SetQuadDecoderMode(etmrBase, config->mode);

        /* Set input filter sample count */
        eTMR_SetQuadDecoderFilterSampleCount(etmrBase, config->qdInputFilterSampleCount);
        /* Set input filter sample period */
        eTMR_SetQuadDecoderFilterSamplePeriod(etmrBase, config->qdInputFilterSamplePeriod);

        /* Set polarity for Phase A */
        eTMR_SetQuadDecoderPhaseAPol(etmrBase, (uint8_t)config->phaseAPol);
        /* Set polarity for Phase B */
        eTMR_SetQuadDecoderPhaseBPol(etmrBase, (uint8_t)config->phaseBPol);
        /* Configure quad decoder clock prescaler */
        eTMR_SetQuadDecoderClkPrs(etmrBase, config->qdClkPrs);

#if FEATURE_eTMR_HAS_INIT_REG
        /* Configure counter (initial value and maximum value) */
        eTMR_SetQuadDecoderCounterInitVal(etmrBase, config->initVal);
#endif
#if FEATURE_eTMR_HAS_QUAD_DECODER_MOD
        eTMR_SetQuadDecoderMod(etmrBase, config->modVal);
#endif

        /* Enable Quadrature Decoder */
        eTMR_EnableQuadDecoder(etmrBase);
    }

    return retStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_QuadDecodeStop
 * Description   : De-activates quadrature decoder mode.
 *
 * Implements    : eTMR_DRV_QuadDecodeStop_Activity
 *END**************************************************************************/
status_t eTMR_DRV_QuadDecodeStop(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type * etmrBase = g_etmrBase[instance];

    /* Disable Quadrature decoder */
    eTMR_DisableQuadDecoder(etmrBase);

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_DRV_GetQuadDecodeState
 * Description   : Return the current quadrature decoder state
 *                 (counter value, overflow flag and overflow direction)
 *
 * Implements    : eTMR_DRV_GetQaudDecodeState_Activity
 *END**************************************************************************/
etmr_qd_state_t eTMR_DRV_GetQuadDecodeState(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type * const etmrBase = g_etmrBase[instance];
    etmr_qd_state_t state;

    state.qdCounterDirection = eTMR_DRV_GetQuadDecodeDir(instance);
    state.qdOverflowFlag = eTMR_DRV_GetQuadDecodeTofFlag(instance);
    state.qdCounterValue = eTMR_GetQuadDecodeCounterValue(etmrBase);
    state.qdOverflowDirection = false;

    if (state.qdOverflowFlag)
    {
        if (state.qdCounterDirection == false)
        {
            state.qdOverflowDirection = true;  // means increase to QD MOD, then overflow
        }
        else
        {
            state.qdOverflowDirection = false; // means decrease to QD INIT, then overflow
        }
    }

    return state;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : eTMR_QD_DRV_GetDefaultConfig
 * Description   : This function will get the default configuration values
 *                 in the structure which is used as a common use-case.
 *
 * Implements    : eTMR_QD_DRV_GetDefaultConfig_Activity
 *END**************************************************************************/
void eTMR_DRV_GetQuadDecodeDefaultConfig(etmr_qd_config_t * const config)
{
    DEV_ASSERT(config != NULL);

    config->mode                           = eTMR_QUAD_PHASE_B_FOLLOW_A;
#if FEATURE_eTMR_HAS_INIT_REG
    config->initVal                        = 0U;
#endif
#if FEATURE_eTMR_HAS_QUAD_DECODER_MOD
    config->modVal                         = 65535U;
#endif
    config->qdInputFilterSampleCount       = 0U;
    config->qdInputFilterSamplePeriod      = 0U;
    config->phaseAPol                      = eTMR_QUAD_PHASE_NORMAL;
    config->phaseBPol                      = eTMR_QUAD_PHASE_NORMAL;
}

/*! @}*/
/*******************************************************************************
* EOF
******************************************************************************/
