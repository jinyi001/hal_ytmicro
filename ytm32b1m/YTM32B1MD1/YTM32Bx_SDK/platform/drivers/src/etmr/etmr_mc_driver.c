/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_mc_driver.c
 * @version 1.4.1
 *
 * @brief eTMR Timer/Counter Mode Driver — implementation.
 *
 * This file implements the timer/counter (MC) mode functions declared
 * in etmr_mc_driver.h. The eTMR is configured as a simple counter
 * with all channel outputs disabled.
 */

#include "etmr_mc_driver.h"
#include "etmr_hw_access.h"

/*!
 * @brief Initialize the eTMR counter in timer/counter mode.
 *
 * @param[in] instance  The eTMR peripheral instance number.
 * @param[in] timer     Pointer to the timer configuration structure.
 * @return Operation status.
 */
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

/*!
 * @brief Start the eTMR counter.
 *
 * @param[in] instance  The eTMR peripheral instance number.
 * @return Operation status.
 */
status_t eTMR_DRV_CounterStart(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);

    /* Enable counter */
    eTMR_DRV_Enable(instance);

    return STATUS_SUCCESS;
}

/*!
 * @brief Stop the eTMR counter.
 *
 * @param[in] instance  The eTMR peripheral instance number.
 * @return Operation status.
 */
status_t eTMR_DRV_CounterStop(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);

    /* Stop the eTMR counter */
    eTMR_DRV_Disable(instance);

    return STATUS_SUCCESS;
}

/*!
 * @brief Read the current eTMR counter value.
 *
 * @param[in] instance  The eTMR peripheral instance number.
 * @return The current counter value.
 */
uint32_t eTMR_DRV_CounterRead(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *const etmrBase = g_etmrBase[instance];

    return eTMR_GetCntVal(etmrBase);
}

/*!
 * @brief Populate a timer configuration structure with default values.
 *
 * @param[out] config  Pointer to the structure to be filled with defaults.
 */
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
