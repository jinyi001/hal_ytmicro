/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_common.c
 * @version 1.4.1
 *
 * @brief eTMR Common Driver — implementation.
 *
 * This file implements the shared driver functions declared in
 * etmr_common.h, including initialization, counter control,
 * synchronization, interrupt management, and status flag access.
 */

#include "etmr_common.h"
#include "etmr_hw_access.h"
#include "interrupt_manager.h"
#include "clock_manager.h"


/*! @brief Table of base addresses for eTMR instances. */
eTMR_Type *const g_etmrBase[eTMR_INSTANCE_COUNT] = eTMR_BASE_PTRS;
const uint8_t g_etmrChannelNum[eTMR_INSTANCE_COUNT] = FEATURE_eTMR_CHANNEL_COUNTS;

/*! @brief Interrupt vectors for the eTMR peripheral. */
IRQn_Type g_etmrIrqId[eTMR_INSTANCE_COUNT][FEATURE_eTMR_CHANNEL_INTERRUPT_COUNTS] = eTMR_CHN_IRQS;
static IRQn_Type g_etmrFaultIrqId[eTMR_INSTANCE_COUNT] = eTMR_FAULT_IRQS;
static IRQn_Type g_etmrOverflowIrqId[eTMR_INSTANCE_COUNT] = eTMR_OVF_IRQS;

/*! @brief IPC clock id for etmr peripheral */
static clock_names_t g_etmrIpcClkId[eTMR_INSTANCE_COUNT] = eTMR_IPC_CLK;

/*! @brief Pointer to runtime state structure. */
etmr_state_t *etmrStatePtr[eTMR_INSTANCE_COUNT];

/*!
 * @brief Reset the eTMR registers for one instance.
 *
 * @param[in] instance  eTMR instance index (0-based).
 */
void eTMR_DRV_Reset(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    CLOCK_DRV_ResetModule(g_etmrIpcClkId[instance]);
}

/*!
 * @brief Disable the eTMR counter.
 *
 * @param[in] instance  eTMR instance index (0-based).
 */
void eTMR_DRV_Disable(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);

    eTMR_Type *etmrBase = g_etmrBase[instance];

    eTMR_Disable(etmrBase);
}

/*!
 * @brief Enable the eTMR counter.
 *
 * @param[in] instance  eTMR instance index (0-based).
 */
void eTMR_DRV_Enable(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);

    eTMR_Type *etmrBase = g_etmrBase[instance];

#if defined(FEATURE_eTMR_ERRATA_E503005)
    etmrBase->STS = etmrBase->STS;
#endif
    eTMR_Enable(etmrBase);
}

/*!
 * @brief Enable or disable the global time base feature.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @param[in] enable    `true` to enable the global time base, `false` to disable it.
 */
void eTMR_DRV_SetGlobalTimeBase(uint32_t instance, bool enable)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    eTMR_SetGlobalEnable(etmrBase, enable);
}

/*!
 * @brief Generate or clear the global synchronization signal.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @param[in] enable    `true` to generate the signal, `false` to clear it.
 */
void eTMR_DRV_GenGlobalSignal(uint32_t instance, bool enable)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    eTMR_GenGlobalSignal(etmrBase, enable);
}

/*!
 * @brief Initialize the eTMR driver for one instance.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @param[in] info      Pointer to the user configuration structure.
 * @param[out] state    Pointer to the runtime state structure.
 * @return Execution status.
 */
status_t eTMR_DRV_Init(uint32_t instance, const etmr_user_config_t *info, etmr_state_t *state)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(info != NULL);
    DEV_ASSERT((info->etmrClockSource == eTMR_CLOCK_SOURCE_INTERNALCLK) ||
               (info->etmrClockSource == eTMR_CLOCK_SOURCE_EXTERNALCLK) ||
               (info->etmrClockSource == eTMR_CLOCK_SOURCE_EXT_IPC_CLK));
    status_t status = STATUS_SUCCESS;
    uint8_t index;
    if (instance < eTMR_INSTANCE_COUNT)
    {
        eTMR_Type *etmrBase = g_etmrBase[instance];
        /* Check if this instance is already initialized */
        if (etmrStatePtr[instance] != NULL)
        {
            status = STATUS_ERROR;
        }
        else // instance is not initialized
        {
            /* Configure state structure. */
            state->etmrClockSource = info->etmrClockSource;
            state->etmrPeriod = 0U;
            state->etmrModValue = 0U;
            for (index = 0U; index < g_etmrChannelNum[instance]; index++)
            {
                state->typeOfUpdate[index] = eTMR_PWM_UPDATE_IN_DUTY_CYCLE;
                state->alignMode[index] = eTMR_PWM_RIGHT_EDGE_ALIGN;
                state->dutyCycle[index] = 0;
                state->offset[index] = 0;
            }

            /* The reset operation */
            eTMR_DRV_Reset(instance);
            /* Set etmr clock source */
#if defined(FEATURE_eTMR_HAS_IPC_CLOCK_SOURCE) && (FEATURE_eTMR_HAS_IPC_CLOCK_SOURCE == 1)
            if (info->etmrClockSource == eTMR_CLOCK_SOURCE_EXT_IPC_CLK)
            {
                eTMR_DRV_ConfigIpcClock(instance);
                eTMR_SetClkSrc(etmrBase, eTMR_CLOCK_SOURCE_EXTERNALCLK);
            }
            else
#endif
            {
                eTMR_SetClkSrc(etmrBase, info->etmrClockSource);
            }

            /* Set etmr clock prescaler */
            eTMR_SetClkPrs(etmrBase, info->etmrPrescaler - 1U);
            /* Set etmr debug mode */
            eTMR_SetDebugModeDisable(etmrBase, !info->debugMode);
            /* Configure etmrStatePtr */
            etmrStatePtr[instance] = state;
            /* Get clock name used to configure the eTMR module */
            state->etmrSourceClockFrequency = eTMR_DRV_GetFrequency(instance);
        }

        if (STATUS_SUCCESS == status)
        {
            /* Check if enable interrupt in counter mode */
            if (info->isTofIntEnabled)
            {
                /* Clear the overflow flag */
                eTMR_DRV_ClearTofFlag(instance);
                /* Enable timer overflow interrupt */
                eTMR_SetTofIntEnable(etmrBase, true);
                INT_SYS_EnableIRQ(g_etmrOverflowIrqId[instance]);
            }
            else
            {
                eTMR_SetTofIntEnable(etmrBase, false);
                INT_SYS_DisableIRQ(g_etmrOverflowIrqId[instance]);
            }

            /* Configure sync for between registers and buffers */
            if (info->syncMethod != NULL)
            {
                status = eTMR_DRV_SetSync(instance, info->syncMethod);
                if (status == STATUS_SUCCESS)
                {
                    /* Configure the output trigger */
                    if (info->outputTrigConfig != NULL)
                    {
                        status = eTMR_DRV_SetOutputTrigger(instance, info->outputTrigConfig);
                    }
                }
            }
        }
    }

    return status;
}

/*!
 * @brief De-initialize the eTMR driver for one instance.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return Execution status.
 */
status_t eTMR_DRV_Deinit(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);

    /* Reset all eTMR register */
    eTMR_DRV_Reset(instance);

    /* Disable IRQn */
    INT_SYS_DisableIRQ(g_etmrOverflowIrqId[instance]);
    INT_SYS_DisableIRQ(g_etmrFaultIrqId[instance]);
    for (uint8_t i = 0U; i < g_etmrChannelNum[i]; i++)
    {
        INT_SYS_DisableIRQ(g_etmrIrqId[instance][i]);
    }

    if (etmrStatePtr[instance] != NULL)
    {
        /* Clear etmr state */
        etmrStatePtr[instance] = NULL;
    }

    return STATUS_SUCCESS;
}

/*!
 * @brief Populate an eTMR user configuration structure with default values.
 *
 * @param[out] config  Pointer to the configuration structure to populate.
 */
void eTMR_DRV_GetDefaultConfig(etmr_user_config_t *const config)
{
    DEV_ASSERT(config != NULL);

    config->etmrClockSource = eTMR_CLOCK_SOURCE_INTERNALCLK;
    config->etmrPrescaler = 1;
    config->debugMode = true;
    config->syncMethod->regSyncFreq = 1;                // 1 cycles
    config->syncMethod->regSyncSel = REG_SYNC_WITH_MOD; // sync when counter match with MOD
    config->syncMethod->cntInitSyncSel = CNT_SYNC_WITH_REG;
    config->syncMethod->maskOutputSyncSel = CHMASK_SYNC_WITH_REG;
    config->syncMethod->regSyncTrigSrc = DISABLE_TRIGGER;
    config->syncMethod->cntInitSyncTrigSrc = DISABLE_TRIGGER;
    config->syncMethod->maskOutputSyncTrigSrc = DISABLE_TRIGGER;
    config->syncMethod->hwTrigFromTmuEnable = false;
    config->syncMethod->hwTrigFromCimEnable = false;
    config->syncMethod->hwTrigFromPadEnable = false;
    config->outputTrigConfig = NULL;
    config->isTofIntEnabled = false;
}

/*!
 * @brief Configure channel output masking and its synchronization behavior.
 *
 * @param[in] instance         eTMR instance index (0-based).
 * @param[in] maskEn           Per-channel mask enable bitmap.
 * @param[in] maskVal          Per-channel output mask value bitmap.
 * @param[in] softwareTrigger  `true` to load via software trigger, `false` to sync with register loading.
 * @return Execution status.
 */
status_t eTMR_DRV_SetChnOutMask(uint32_t instance, uint8_t maskEn, uint16_t maskVal, bool softwareTrigger)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    etmr_sync_trig_src_t trigSrc;
    etmr_mask_sync_sel_t opportunity;
    eTMR_ClearLdok(etmrBase);

    if (softwareTrigger)
    {
        /* keep mask loading trigger source */
        trigSrc = (etmr_sync_trig_src_t)eTMR_GetMaskLoadTrigSrc(etmrBase);
        /* keep mask loading opportunity */
        opportunity = (etmr_mask_sync_sel_t)eTMR_GetMaskLoadOpportunity(etmrBase);
        /* select CHMASK loading trigger source */
        eTMR_SetMaskLoadTrigSrc(etmrBase, SW_TRIGGER);
        /* select CHMASK loading opportunity */
        eTMR_SetMaskLoadOpportunity(etmrBase, CHMASK_SYNC_WITH_TRIG);
    }
    else // sync with register loading
    {
        /* select CHMASK loading trigger source */
        eTMR_SetMaskLoadTrigSrc(etmrBase, DISABLE_TRIGGER);
        /* select CHMASK loading opportunity */
        eTMR_SetMaskLoadOpportunity(etmrBase, CHMASK_SYNC_WITH_REG);
    }
    /* set channel Mask enable and value */
    eTMR_SetChnOutMask(etmrBase, maskVal, maskEn);
    /* set ldok */
    eTMR_SetLdok(etmrBase);

    if (softwareTrigger)
    {
        /* generate one bus clock cycle pulse trigger */
        eTMR_GenSoftwareTrigger(etmrBase, true);
        /* restore CHMASK loading trigger source */
        eTMR_SetMaskLoadTrigSrc(etmrBase, trigSrc);
        /* restore CHMASK loading opportunity */
        eTMR_SetMaskLoadOpportunity(etmrBase, opportunity);
    }

    return STATUS_SUCCESS;
}

#if FEATURE_eTMR_HAS_INIT_REG
/*!
 * @brief Configure the initial counter value after enabling eTMR.
 *
 * @param[in] instance         eTMR instance index (0-based).
 * @param[in] counterValue     Initial counter value to load.
 * @param[in] softwareTrigger  `true` to apply via software trigger.
 * @return Execution status.
 */
status_t eTMR_DRV_SetCounterInit(uint32_t instance, uint32_t counterValue, bool softwareTrigger)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];

    eTMR_ClearLdok(etmrBase);
    /* set counter initial value */
    eTMR_SetInitVal(etmrBase, counterValue);
    /* set ldok */
    eTMR_SetLdok(etmrBase);
    if (softwareTrigger)
    {
        /* generate software trigger */
        eTMR_GenSoftwareTrigger(etmrBase, softwareTrigger);
    }

    return STATUS_SUCCESS;
}
#endif

/*!
 * @brief Configure the counter MOD value.
 *
 * @param[in] instance         eTMR instance index (0-based).
 * @param[in] counterValue     MOD value to program.
 * @param[in] softwareTrigger  `true` to apply via software trigger.
 * @return Execution status.
 */
status_t eTMR_DRV_SetCounterMod(uint32_t instance, uint32_t counterValue, bool softwareTrigger)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];

    eTMR_ClearLdok(etmrBase);
    /* set MOD value */
    eTMR_SetMod(etmrBase, counterValue);
    /* set ldok */
    eTMR_SetLdok(etmrBase);
    if (softwareTrigger)
    {
        /* generate software trigger */
        eTMR_GenSoftwareTrigger(etmrBase, softwareTrigger);
    }

    return STATUS_SUCCESS;
}

#if defined(FEATURE_eTMR_HAS_MID) && (FEATURE_eTMR_HAS_MID == 1)
/*!
 * @brief Configure the counter MID value.
 *
 * @param[in] instance      eTMR instance index (0-based).
 * @param[in] counterValue  MID value to program.
 * @param[in] softwareTrigger  `true` to apply via software trigger.
 * @return Execution status.
 */
status_t eTMR_DRV_SetCounterMid(uint32_t instance, uint32_t counterValue, bool softwareTrigger)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];

    eTMR_ClearLdok(etmrBase);
    /* set MID value */
    eTMR_SetMid(etmrBase, counterValue);
    /* set ldok */
    eTMR_SetLdok(etmrBase);
    if (softwareTrigger)
    {
        /* generate software trigger */
        eTMR_GenSoftwareTrigger(etmrBase, softwareTrigger);
    }

    return STATUS_SUCCESS;
}

/*!
 * @brief Get the current counter MID value.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return Current MID register value.
 */
uint32_t eTMR_DRV_GetMid(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];

    /* get MID value */
    return eTMR_GetMid(etmrBase);
}
#endif

/*!
 * @brief Set LDOK to start shadow-to-working register synchronization.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return Execution status.
 */
status_t eTMR_DRV_SetLdok(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];

    /* Open Ldok to start transfer from shadow register to working register */
    eTMR_SetLdok(etmrBase);

    return STATUS_SUCCESS;
}

/*!
 * @brief Clear the LDOK state.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return Execution status.
 */
status_t eTMR_DRV_ClearLdok(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];

    /* Open Ldok to start transfer from shadow register to working register */
    eTMR_ClearLdok(etmrBase);

    return STATUS_SUCCESS;
}

/*!
 * @brief Configure synchronization for buffered eTMR registers.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @param[in] param     Pointer to the synchronization configuration structure.
 * @return Execution status.
 */
status_t eTMR_DRV_SetSync(uint32_t instance, const etmr_pwm_sync_t *param)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(param != NULL);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    status_t retStatus = STATUS_SUCCESS;

    /* set register loading frequency, it represents that registers reload(sync) once 
     * per n time(s) of reload(sync) event. (n = param->regSyncFreq) */
    DEV_ASSERT(param->regSyncFreq > 0U);
    if (param->regSyncFreq == 0U)
    {
        retStatus = STATUS_ETMR_LOAD_FREQ_ERROR;
    }
    eTMR_SetRegLoadFreq(etmrBase, param->regSyncFreq - 1U);
    /* set hardware trigger to register loading enable or not */
    eTMR_SetHwTrig0Enable(etmrBase, param->hwTrigFromTmuEnable);
    /* set hardware trigger to counter loading enable or not */
    eTMR_SetHwTrig1Enable(etmrBase, param->hwTrigFromCimEnable);
    /* set hardware trigger to channel mask loading enable or not */
    eTMR_SetHwTrig2Enable(etmrBase, param->hwTrigFromPadEnable);

    /* enable CHxVAL0 & CHxVAL1 register loading */
    for (uint8_t i = 0U; i < g_etmrChannelNum[instance]; i++)
    {
        eTMR_SetChnLdEnable(etmrBase, i, true);
    }
    /* set register loading opportunity */
    eTMR_SetRegLoadOpportunity(etmrBase, param->regSyncSel);
    /* set counter initial loading opportunity */
    eTMR_SetCounterLoadOpportunity(etmrBase, param->cntInitSyncSel);
    /* set mask output loading opportunity */
    eTMR_SetMaskLoadOpportunity(etmrBase, param->maskOutputSyncSel);

    /* set register loading trigger source */
    eTMR_SetRegLoadTrigSrc(etmrBase, param->regSyncTrigSrc);
    /* set counter initial value loading trigger source */
    eTMR_SetCounterLoadTrigSrc(etmrBase, param->cntInitSyncTrigSrc);
    /* set mask output loading trigger source */
    eTMR_SetMaskLoadTrigSrc(etmrBase, param->maskOutputSyncTrigSrc);

    return retStatus;
}

/*!
 * @brief Set the channel safe-state bitmap used on fault events.
 *
 * @param[in] instance   eTMR instance index (0-based).
 * @param[in] safeState  Safe-state bitmap for channel outputs.
 * @return Execution status.
 */
status_t eTMR_DRV_SetSafeState(uint32_t instance, uint32_t safeState)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    status_t retStatus = STATUS_SUCCESS;

    eTMR_SetChnOutFaultVal(etmrBase, (uint16_t)safeState);

    return retStatus;
}

/*!
 * @brief Configure the eTMR output trigger behavior.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @param[in] param     Pointer to the output trigger configuration structure.
 * @return Execution status.
 */
status_t eTMR_DRV_SetOutputTrigger(uint32_t instance, const etmr_trig_config_t *param)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(param != NULL);

    uint8_t index;
    eTMR_Type *etmrBase = g_etmrBase[instance];
    status_t retStatus = STATUS_SUCCESS;

    if (param != NULL)
    {
        if (param->trigSrc == TRIGGER_FROM_PWM)
        {
            /* select output trigger source */
            eTMR_SetOutTrigSrc(etmrBase, TRIGGER_FROM_PWM);

            /* select pwm output source of channel n */
            eTMR_SetPulseOutSrc(etmrBase, (etmr_channel_index_t)param->pwmOutputChannel);
        }
        else // TRIGGER_FROM_MATCHING_EVENT
        {
#if defined(FEATURE_eTMR_HAS_OUTPUT_TRIGGER_WIDTH) && (FEATURE_eTMR_HAS_OUTPUT_TRIGGER_WIDTH == 1)
            /* set output trigger width */
            eTMR_SetOutputTrigWidth(etmrBase, param->outputTrigWidth);
#endif

            /* set output trigger frequency, it represents that generate(output) one trigger 
             * per n trigger(s). (n = param->outputTrigFreq) */
            DEV_ASSERT(param->outputTrigFreq > 0U);
            if (param->outputTrigFreq == 0U)
            {
                retStatus = STATUS_ETMR_TRIG_OUTPUT_FREQ_ERROR;
            }
            eTMR_SetOutputTrigFreq(etmrBase, param->outputTrigFreq - 1U);

            /* select output trigger source */
            eTMR_SetOutTrigSrc(etmrBase, TRIGGER_FROM_MATCHING_EVENT);

            /* enable mod matching event trigger */
            if (param->modMatchTrigEnable)
            {
                eTMR_SetModMatchTrigEnable(etmrBase, param->modMatchTrigEnable);
            }

#if defined(FEATURE_eTMR_HAS_MID) && (FEATURE_eTMR_HAS_MID == 1)
            /* enable mid matching event trigger */
            if (param->midMatchTrigEnable)
            {
                eTMR_SetMidMatchTrigEnable(etmrBase, param->midMatchTrigEnable);
            }
#endif

#if defined(FEATURE_eTMR_HAS_INIT_REG) && (FEATURE_eTMR_HAS_INIT_REG == 1)
            /* enable init matching event trigger */
            if (param->initMatchTrigEnable)
            {
                eTMR_SetInitMatchTrigEnable(etmrBase, param->initMatchTrigEnable);
            }
#endif

            if (param->channelTrigParamConfig != NULL)
            {
                for (index = 0; index < param->numOfChannels; index++)
                {
                    eTMR_IsChnVal0MatchTrigEnable(etmrBase,
                                                  param->channelTrigParamConfig[index].channelId,
                                                  param->channelTrigParamConfig[index].channelVal0MatchTrigEn);
                    eTMR_IsChnVal1MatchTrigEnable(etmrBase,
                                                  param->channelTrigParamConfig[index].channelId,
                                                  param->channelTrigParamConfig[index].channelVal1MatchTrigEn);
                }
            }
        }
    }

    return retStatus;
}

/*!
 * @brief Enable a set of eTMR interrupts.
 *
 * @param[in] instance       eTMR instance index (0-based).
 * @param[in] interruptMask  Bitmask of interrupts to enable (etmr_interrupt_option_t).
 * @return Execution status.
 */
status_t eTMR_DRV_EnableInterrupts(uint32_t instance, uint32_t interruptMask)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    uint32_t chnlInts = (interruptMask & 0x000000FFU);
    uint8_t channel = 0U;

    /* Enable the timer overflow interrupt */
    if ((interruptMask & (uint32_t)eTMR_TIME_OVER_FLOW_INT_ENABLE) != 0x0U)
    {
        eTMR_SetTofIntEnable(etmrBase, true);
        INT_SYS_EnableIRQ(g_etmrOverflowIrqId[instance]);
    }

    /* Enable the fault interrupt */
    if ((interruptMask & (uint32_t)eTMR_FAULT_INT_ENABLE) != 0x0U)
    {
        eTMR_SetFaultIntEnable(etmrBase, true);
        INT_SYS_EnableIRQ(g_etmrFaultIrqId[instance]);
    }

    /* Enable the channel interrupts */
    while (chnlInts != 0U)
    {
        if ((chnlInts & 0x1U) != 0x0U)
        {
            eTMR_DRV_EnableChnInt(instance, channel);
            INT_SYS_EnableIRQ(g_etmrIrqId[instance][channel]);
        }
        channel++;
        chnlInts = chnlInts >> 1U;
    }

    return STATUS_SUCCESS;
}

/*!
 * @brief Disable a set of eTMR interrupts.
 *
 * @param[in] instance       eTMR instance index (0-based).
 * @param[in] interruptMask  Bitmask of interrupts to disable.
 */
void eTMR_DRV_DisableInterrupts(uint32_t instance, uint32_t interruptMask)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    uint32_t chnlInts = (interruptMask & 0x000000FFU);
    uint8_t channel = 0U;

    /* Disable the timer overflow interrupt */
    if ((interruptMask & (uint32_t)eTMR_TIME_OVER_FLOW_INT_ENABLE) != 0x0U)
    {
        eTMR_SetTofIntEnable(etmrBase, false);
        INT_SYS_DisableIRQ(g_etmrOverflowIrqId[instance]);
    }

    /* Disable the fault interrupt */
    if ((interruptMask & (uint32_t)eTMR_FAULT_INT_ENABLE) != 0x0U)
    {
        eTMR_SetFaultIntEnable(etmrBase, false);
        INT_SYS_DisableIRQ(g_etmrFaultIrqId[instance]);
    }

    /* Disable the channel interrupts */
    while (chnlInts != 0U)
    {
        if ((chnlInts & 0x1U) != 0x0U)
        {
            eTMR_DRV_DisableChnInt(instance, channel);
            INT_SYS_DisableIRQ(g_etmrIrqId[instance][channel]);
        }
        channel++;
        chnlInts = chnlInts >> 1U;
    }
}

/*!
 * @brief Get the currently enabled eTMR interrupts.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return Bitmask of enabled interrupts.
 */
uint32_t eTMR_DRV_GetEnabledInterrupts(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    uint32_t enabledInterrupts = 0U;
    uint8_t channel = g_etmrChannelNum[instance];

    /* Check if timer overflow interrupt is enabled */
    if (eTMR_DRV_IsTofIntEnabled(instance) == true)
    {
        enabledInterrupts |= (uint32_t)eTMR_TIME_OVER_FLOW_INT_ENABLE;
    }

    /* Check if fault interrupt is enabled */
    if (eTMR_DRV_IsFaultIntEnabled(instance) == true)
    {
        enabledInterrupts |= (uint32_t)eTMR_FAULT_INT_ENABLE;
    }

    /* Check if the channel interrupts are enabled */
    while (channel > 0U)
    {
        channel--;
        if (eTMR_DRV_IsChnIntEnabled(instance, channel) == true)
        {
            enabledInterrupts |= (1UL << (uint32_t)channel);
        }
    }

    return enabledInterrupts;
}

/*!
 * @brief Get the aggregated eTMR status flags.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return Bitmask of active status flags.
 */
uint32_t eTMR_DRV_GetStatusFlags(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    uint8_t channel;
    uint8_t fltChannel;
    uint32_t statusFlags = 0U;

    /* Check the timer overflow flag */
    if (eTMR_DRV_GetTofFlag(instance) == true)
    {
        statusFlags |= (uint32_t)eTMR_TIME_OVER_FLOW_FLAG;
    }

    /* Check reload flag */
    if (eTMR_DRV_GetReloadFlag(instance) == true)
    {
        statusFlags |= (uint32_t)eTMR_RELOAD_FLAG;
    }

    /* Check fault flag */
    for (fltChannel = 0U; fltChannel < (g_etmrChannelNum[instance] >> 1U); fltChannel++)
    {
        if (eTMR_DRV_GetFaultFlag(instance, fltChannel) == true)
        {
            statusFlags |= ((uint32_t)eTMR_FAULT0_FLAG << fltChannel);
        }
    }

    /* Lower 8 bits contain the channel status flags */
    for (channel = 0U; channel < g_etmrChannelNum[instance]; channel++)
    {
        if (eTMR_DRV_GetChnFlag(instance, channel) == true)
        {
            statusFlags |= (1UL << (uint32_t)channel);
        }
    }

    return statusFlags;
}

/*!
 * @brief Get the frequency of the clock source feeding the eTMR counter.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return Counter source clock frequency in Hz.
 */
uint32_t eTMR_DRV_GetFrequency(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(etmrStatePtr[instance] != NULL);
    eTMR_Type const *etmrBase = g_etmrBase[instance];
    status_t returnCode = STATUS_SUCCESS;
    uint8_t clkPrs;
    uint32_t frequency = 0U;
    const etmr_state_t *state = etmrStatePtr[instance];
    clkPrs = (uint8_t)(eTMR_GetClkPrs(etmrBase) + 1U);

    switch (state->etmrClockSource)
    {
        case eTMR_CLOCK_SOURCE_INTERNALCLK:
            /* Get the clock frequency value */
            returnCode = CLOCK_SYS_GetFreq(eTMR_INTERNAL_CLOCK, &frequency);
            break;
        case eTMR_CLOCK_SOURCE_EXTERNALCLK:
            frequency = FEATURE_eTMR_EXTERNAL_CLOCK_FREQ;
            break;
#if defined(FEATURE_eTMR_HAS_IPC_CLOCK_SOURCE) && (FEATURE_eTMR_HAS_IPC_CLOCK_SOURCE == 1)
        case eTMR_CLOCK_SOURCE_EXT_IPC_CLK:
            returnCode = CLOCK_SYS_GetFreq(g_etmrIpcClkId[instance], &frequency);
            break;
#endif
        default:
            /* Nothing to do */
            break;
    }

    /* Checks the functional clock of eTMR module */
    (void)returnCode;
    DEV_ASSERT(returnCode == STATUS_SUCCESS);

    return (uint32_t)(frequency / clkPrs);
}

/*!
 * @brief Convert a target frequency in Hz to timer period ticks.
 *
 * @param[in] instance    eTMR instance index (0-based).
 * @param[in] freqencyHz  Target frequency in Hz.
 * @return Period value in timer ticks.
 */
uint32_t eTMR_DRV_ConvertFreqToPeriodTicks(uint32_t instance, uint32_t freqencyHz)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(freqencyHz != 0U);
    uint32_t ueTMRhz;
    const etmr_state_t *state = etmrStatePtr[instance];
    ueTMRhz = state->etmrSourceClockFrequency;

    return (ueTMRhz / freqencyHz);
}

/*! @}*/
/*******************************************************************************
* EOF
******************************************************************************/
