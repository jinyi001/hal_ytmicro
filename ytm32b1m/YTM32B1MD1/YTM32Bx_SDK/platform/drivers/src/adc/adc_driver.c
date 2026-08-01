/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file adc_driver.c
 * @version 1.4.1
 *
 * @brief ADC Driver - implementation of the public ADC_DRV_* API.
 *
 * This file implements the application-level ADC driver functions declared in
 * adc_driver.h. Each function resolves the ADC instance base address and then
 * coordinates the lower-level register helpers from adc_hw_access.h.
 */

#include <stddef.h>
#include "adc_driver.h"
#include "adc_hw_access.h"
#include "clock_manager.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Table of base addresses for ADC instances. */
static ADC_Type *const s_adcBase[ADC_INSTANCE_COUNT] = ADC_BASE_PTRS;

/*******************************************************************************
 * Converter Configuration
 ******************************************************************************/

/*!
 * @brief Fill a converter configuration structure with default values.
 */
void ADC_DRV_InitConverterStruct(adc_converter_config_t *const config)
{
    DEV_ASSERT(config != NULL);


    config->clockDivider = ADC_DEFAULT_CLOCK_DIVIDER;
    config->startTime = ADC_DEFAULT_START_TIME;
    config->sampleTime = ADC_DEFAULT_SAMPLE_TIME;

    config->overrunMode = false;
    config->autoOffEnable = false;
    config->waitEnable = false;
    config->triggerSource = 0x00;
#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
    config->trigger = ADC_TRIG_INJECT_SW_NORMAL_SW;
#else
    config->trigger = ADC_TRIGGER_SOFTWARE;
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */
    config->align = ADC_ALIGN_RIGHT;
    config->resolution = ADC_RESOLUTION_12BIT;
    config->dmaWaterMark = 0x01;
    config->dmaEnable = false;

    config->sequenceConfig.channels[0] = ADC_INPUTCHAN_EXT0;
    config->sequenceConfig.totalChannels = 1;
    config->sequenceConfig.sequenceMode = ADC_CONV_LOOP;
    config->sequenceConfig.sequenceIntEnable = false;
    config->sequenceConfig.convIntEnable = false;
    config->sequenceConfig.ovrunIntEnable = false;
    config->sequenceConfig.sampIntEnable = false;
    config->sequenceConfig.readyIntEnable = false;

    config->compIntEnable = false;
#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
    config->adcCompareCnt = 0;
#else
    config->compareConfig.compareEnable = false;
    config->compareConfig.compareAllChannelEnable = false;
    config->compareConfig.compHigh = 0xFFF;
    config->compareConfig.compLow = 0x000;
#endif /* FEATURE_ADC_WDG_CHANNEL_COUNT */
}

/*!
 * @brief Apply a top-level converter configuration to an ADC instance.
 */
status_t ADC_DRV_ConfigConverter(const uint32_t instance,
                                 const adc_converter_config_t *const config)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    ADC_Type *const base = s_adcBase[instance];
    clock_names_t adc_clocks[ADC_INSTANCE_COUNT] = ADC_CLOCKS;
    uint32_t adc_freq = 0u;
    status_t clk_status = CLOCK_SYS_GetFreq(adc_clocks[instance], &adc_freq);
    DEV_ASSERT(clk_status == STATUS_SUCCESS);
    (void) clk_status;
    status_t status = STATUS_SUCCESS;

    adc_freq = adc_freq / (uint32_t) (1UL + ((uint32_t) (config->clockDivider)));
    if((adc_freq >= ADC_CLOCK_FREQ_MIN_RUNTIME) && (adc_freq <= ADC_CLOCK_FREQ_MAX_RUNTIME))
    {
        /* Program converter timing, trigger, DMA, and resolution settings. */
        ADC_SetClockDivide(base, config->clockDivider);
        ADC_SetStartTime(base, config->startTime);
        ADC_SetSampleTime(base, config->sampleTime);
        ADC_SetOverrunModeFlag(base, config->overrunMode);
        ADC_SetautoOffEnableFlag(base, config->autoOffEnable);
        ADC_SetResolution(base, config->resolution);
        ADC_SetTriggerSource(base, config->triggerSource);
        ADC_SetTriggerMode(base, config->trigger);
        ADC_SetWaitEnableFlag(base, config->waitEnable);
        ADC_SetDMAEnableFlag(base, config->dmaEnable);
        ADC_SetDMAWatermark(base, config->dmaWaterMark);
        ADC_DRV_ConfigSequence(instance, &config->sequenceConfig);
#if FEATURE_ADC_SUPPORT_LOW_POWER_KEEP
        ADC_SetLowPowerEnableFlag(base, false);
#endif
#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
        ADC_DRV_ConfigInject(instance, &config->injectConfig);
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */
        ADC_SetAwdIntEnableFlag(base, config->compIntEnable);
#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
        ADC_DRV_ConfigHwCompareGroup(instance, config->compareConfig, config->adcCompareCnt);
#else
        ADC_DRV_ConfigHwCompare(instance, &config->compareConfig);
#endif /* FEATURE_ADC_WDG_CHANNEL_COUNT */
    }else{
        /* Confirm ADC clock in range */
        status = STATUS_InvalidArgument;
    }
    return status;
}

/*!
 * @brief Read the active converter configuration from hardware.
 */
void ADC_DRV_GetConverterConfig(const uint32_t instance,
                                adc_converter_config_t *const config)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    ADC_Type *const base = s_adcBase[instance];
    config->clockDivider = ADC_GetClockDivide(base);
    config->sampleTime = ADC_GetSampleTime(base);
    config->resolution = ADC_GetResolution(base);
    config->trigger = ADC_GetTriggerMode(base);
    config->waitEnable = ADC_GetWaitEnableFlag(base);
    config->dmaEnable = ADC_GetDMAEnableFlag(base);
    config->dmaWaterMark = ADC_GetDMAWatermark(base);
    ADC_DRV_GetSequenceConfig(instance, &config->sequenceConfig);
#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
    ADC_DRV_GetInjectConfig(instance, &config->injectConfig);
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */
#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
#else
    ADC_DRV_GetHwCompareConfig(instance, &config->compareConfig);
#endif /* FEATURE_ADC_WDG_CHANNEL_COUNT */
}

/*******************************************************************************
 * Conversion Control & Status
 ******************************************************************************/

/*!
 * @brief Reset the ADC instance to its runtime default state.
 */
void ADC_DRV_Reset(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_DRV_Disable(instance);
}

/*!
 * @brief Enable the ADC hardware.
 */
void ADC_DRV_Enable(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const baseAddr = s_adcBase[instance];

    ADC_Enable(baseAddr);
}

/*!
 * @brief Disable the ADC hardware and abort any active conversion.
 */
void ADC_DRV_Disable(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const baseAddr = s_adcBase[instance];

    if (ADC_GetEnableFlag(baseAddr))
    {
        ADC_Stop(baseAddr);
        ADC_Disable(baseAddr);
    }
}

/*!
 * @brief Start or arm ADC conversion processing.
 */
void ADC_DRV_Start(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const baseAddr = s_adcBase[instance];

    ADC_Enable(baseAddr);

    ADC_Start(baseAddr);
}

/*!
 * @brief Stop the current ADC conversion activity.
 */
void ADC_DRV_Stop(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const baseAddr = s_adcBase[instance];

    ADC_Stop(baseAddr);
}

/*******************************************************************************
 * Hardware Compare Configuration
 ******************************************************************************/

/*!
 * @brief Fill a watchdog configuration structure with default values.
 */
void ADC_DRV_InitHwCompareStruct(adc_compare_config_t *const config)
{
    DEV_ASSERT(config != NULL);

    config->compareEnable = false;
    config->compareAllChannelEnable = false;
    config->compHigh = 0U;
    config->compLow = 0U;
}

/*!
 * @brief Apply a single hardware-compare configuration.
 */
void ADC_DRV_ConfigHwCompare(const uint32_t instance,
                             const adc_compare_config_t *const config)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    uint8_t wdgChannel = 0;
    ADC_Type *const base = s_adcBase[instance];
    ADC_SetHwCompareEnableFlag(base, config->compareEnable, wdgChannel);
    ADC_SetHwCompareAllEnableFlag(base, config->compareAllChannelEnable);
    ADC_SetHwCompareRangeEnableFlag(base, config->compChalSel, wdgChannel);
    ADC_SetHwCompareCompHighValue(base, config->compHigh, wdgChannel);
    ADC_SetHwCompareCompLowValue(base, config->compLow, wdgChannel);
    ADC_SetAwdEffectiveMode(base, (config->effectiveMode == ADC_AWG_EFFECTIVE_INSIDE) ? true : false, wdgChannel);
}

#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
/*!
 * @brief Apply a watchdog configuration array.
 */
void ADC_DRV_ConfigHwCompareGroup(const uint32_t instance,
                                  adc_compare_config_t *const config, 
                                  uint8_t count)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);
    if (count != 0u)
    {
        DEV_ASSERT(config != NULL);

        ADC_Type *const base = s_adcBase[instance];
        for(uint8_t i = 0; i < count; i ++)
        {
            /* Program each watchdog channel entry from the caller array. */
            ADC_SetHwCompareEnableFlag(base, config[i].compareEnable, config[i].comWdchIndex);
            ADC_SetHwCompareRangeEnableFlag(base, config[i].compChalSel, config[i].comWdchIndex);
            ADC_SetHwCompareCompHighValue(base, config[i].compHigh, config[i].comWdchIndex);
            ADC_SetHwCompareCompLowValue(base, config[i].compLow, config[i].comWdchIndex);
            ADC_SetAwdEffectiveMode(base, (config[i].effectiveMode == ADC_AWG_EFFECTIVE_INSIDE) ? true : false, config[i].comWdchIndex);
            /* Only watchdog channel 0 supports the all-channel compare mode. */
            if(config[i].comWdchIndex == 0u)
            {
                ADC_SetHwCompareAllEnableFlag(base, config[i].compareAllChannelEnable);
            }
        }
    }
}
#endif /* FEATURE_ADC_WDG_CHANNEL_COUNT */

/*!
 * @brief Read back the active hardware-compare configuration.
 */
void ADC_DRV_GetHwCompareConfig(const uint32_t instance,
                                adc_compare_config_t *const config)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    uint8_t wdgChannel = 0;
    ADC_Type *const base = s_adcBase[instance];
    config->compareEnable = ADC_GetHwCompareEnableFlag(base, wdgChannel);
    config->compareAllChannelEnable = ADC_GetHwCompareAllEnableFlag(base);
    config->compHigh = ADC_GetHwCompareCompHighValue(base, wdgChannel);
    config->compLow = ADC_GetHwCompareCompLowValue(base, wdgChannel);
}

/*******************************************************************************
 * Sequence Configuration
 ******************************************************************************/

/*!
 * @brief Fill a normal-sequence configuration structure with default values.
 */
void ADC_DRV_InitSequenceStruct(adc_sequence_config_t *const config)
{
    DEV_ASSERT(config != NULL);

    config->channels[0] = ADC_INPUTCHAN_EXT0;
    config->totalChannels = 1;
    config->sequenceMode = ADC_CONV_LOOP;
    config->sequenceIntEnable = false;
    config->convIntEnable = false;
}

/*!
 * @brief Program the normal conversion sequence.
 */
void ADC_DRV_ConfigSequence(const uint32_t instance,
                            const adc_sequence_config_t *const config)
{
    uint8_t i;
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);
    DEV_ASSERT(0 < config->totalChannels);
    DEV_ASSERT(ADC_CHSEL_COUNT >= config->totalChannels);

    ADC_Type *const base = s_adcBase[instance];
    /* Abort any active conversion before rewriting sequence registers. */
    ADC_Stop(base);
    for (i = 0; i < config->totalChannels; i++)
    {
        ADC_SetSequenceChannel(base, i, config->channels[i]);
    }
    ADC_SetSequenceTotalChannel(base, config->totalChannels);
    ADC_SetSequenceMode(base, config->sequenceMode);
    ADC_SetEocIntEnableFlag(base, config->convIntEnable);
    ADC_SetEoSeqIntEnableFlag(base, config->sequenceIntEnable);
    ADC_SetOvrIntEnableFlag(base, config->ovrunIntEnable);
    ADC_SetSampEndIntEnableFlag(base, config->sampIntEnable);
    ADC_SetReadyIntEnableFlag(base, config->readyIntEnable);
}

/*!
 * @brief Read back the normal-sequence configuration.
 */
void ADC_DRV_GetSequenceConfig(const uint32_t instance,
                               adc_sequence_config_t *const config)
{
    uint8_t i;
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    ADC_Type *const base = s_adcBase[instance];
    for (i = 0; i < ADC_CHSEL_COUNT; i++)
    {
        config->channels[i] = ADC_GetSequenceChannel(base, i);
    }
    config->totalChannels = ADC_GetSequenceTotalChannel(base);
    config->sequenceMode = ADC_GetSequenceMode(base);
    config->convIntEnable = ADC_GetEocIntEnableFlag(base);
    config->sequenceIntEnable = ADC_GetEoSeqIntEnableFlag(base);
}

#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
/*!
 * @brief Program the injected conversion sequence.
 */
void ADC_DRV_ConfigInject(const uint32_t instance,
                            const adc_inject_config_t *const config)
{
    uint8_t i;
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);
    if (config->injectEnable)
    {
        DEV_ASSERT(0 < config->totalChannels);
        DEV_ASSERT(ADC_INJCH_COUNT >= config->totalChannels);

        ADC_Type *const base = s_adcBase[instance];
        /* Abort any active conversion before rewriting injected-sequence registers. */
        ADC_Stop(base);
        for (i = 0; i < ADC_INJCH_COUNT; i++)
        {
            ADC_SetInjectChannel(base, i, config->channels[i]);
        }
        ADC_SetInjectTotalChannel(base, config->totalChannels);
        ADC_SetInjectIntEnableFlag(base, config->convIntEnable);
        ADC_SetInjectErrIntEnableFlag(base, config->errorIntEnable);
    }
}

/*!
 * @brief Read back the injected conversion sequence configuration.
 */
void ADC_DRV_GetInjectConfig(const uint32_t instance,
                               adc_inject_config_t *const config)
{
    uint8_t i;
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    if (config->injectEnable)
    {
        ADC_Type *const base = s_adcBase[instance];
        for (i = 0; i < ADC_INJCH_COUNT; i++)
        {
            config->channels[i] = ADC_GetSequenceChannel(base, i);
        }
        config->totalChannels = ADC_GetInjectTotalChannel(base);
        config->convIntEnable = ADC_GetInjectIntEnableFlag(base);
        config->errorIntEnable = ADC_GetInjectErrIntEnableFlag(base);
    }
}
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */

/*******************************************************************************
 * Conversion Control & Status
 ******************************************************************************/

/*!
 * @brief Poll until the current conversion is no longer active.
 */
void ADC_DRV_WaitConvDone(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const base = s_adcBase[instance];
    while (ADC_GetConvActiveFlag(base) == true)
    {
        /* Wait for the conversion-active flag to clear. */
    }
}

/*!
 * @brief Poll until the configured sequence reports completion.
 */
void ADC_DRV_WaitSequenceDone(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const base = s_adcBase[instance];
    while (ADC_GetSequenceDoneFlag(base) == false)
    {
        /* Wait for the sequence-done flag to assert. */
    }
}

/*!
 * @brief Get the conversion-complete flag state.
 */
bool ADC_DRV_GetConvCompleteFlag(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const base = s_adcBase[instance];

    return (ADC_GetConvCompleteFlag(base));
}

/*!
 * @brief Get the end-of-conversion flag state.
 */
bool ADC_DRV_GetEndOfConversionFlag(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const base = s_adcBase[instance];

    uint32_t tmp = base->STS;
    tmp = (tmp & ADC_STS_EOC_MASK) >> ADC_STS_EOC_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Get the result-FIFO full flag state.
 */
bool ADC_DRV_GetFullOfConversionFlag(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const base = s_adcBase[instance];

    uint32_t tmp = base->STS;
    tmp = (tmp & ADC_STS_FULL_MASK) >> ADC_STS_FULL_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Get the FIFO overrun flag state.
 */
bool ADC_DRV_GetOvrRunOfConversionFlag(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const base = s_adcBase[instance];

    uint32_t tmp = base->STS;
    tmp = (tmp & ADC_STS_OVR_MASK) >> ADC_STS_OVR_SHIFT;
    return (tmp != 0u) ? true : false;
}

#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
/*!
 * @brief Get the analog watchdog flag state.
 */
bool ADC_DRV_GetWatchdogFlag(const uint32_t instance, const uint8_t wdgChannel)
#else /* FEATURE_ADC_WDG_CHANNEL_COUNT == 1U */
/*!
 * @brief Get the analog watchdog flag state.
 */
bool ADC_DRV_GetWatchdogFlag(const uint32_t instance)
#endif /* FEATURE_ADC_WDG_CHANNEL_COUNT */
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const base = s_adcBase[instance];

    uint32_t tmp = base->STS;
#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
    tmp = (tmp >> (ADC_STS_WD0_SHIFT + wdgChannel)) & 0x01U;
#else
    tmp = (tmp & ADC_STS_WD_MASK) >> ADC_STS_WD_SHIFT;
#endif /* FEATURE_ADC_WDG_CHANNEL_COUNT */
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Get the end-of-sequence flag state.
 */
bool ADC_DRV_GetEndOfSequenceFlag(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const base = s_adcBase[instance];

    uint32_t tmp = base->STS;
    tmp = (tmp & ADC_STS_EOSEQ_MASK) >> ADC_STS_EOSEQ_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Get the ADC-ready flag state.
 */
bool ADC_DRV_GetReadyFlag(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const base = s_adcBase[instance];

    uint32_t tmp = base->STS;
    tmp = tmp & ADC_STS_ADRDY_MASK;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Get the sample-end flag state.
 */
bool ADC_DRV_GetSampEndFlag(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const base = s_adcBase[instance];

    uint32_t tmp = base->STS;
    tmp = (tmp & ADC_STS_EOSMP_MASK) >> ADC_STS_EOSMP_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Read one conversion result from the result FIFO.
 */
uint16_t ADC_DRV_ReadFIFO(const uint32_t instance)
{
    ADC_Type *const base = s_adcBase[instance];

    return (ADC_ReadFIFO(base));
}

/*!
 * @brief Read one tagged FIFO entry.
 */
uint32_t ADC_DRV_ReadSeqtagAndData(const uint32_t instance)
{
    ADC_Type *const base = s_adcBase[instance];

    return ((uint32_t) base->FIFO);
}

/*******************************************************************************
 * Interrupt & Flag Control
 ******************************************************************************/

/*!
 * @brief Return the IRQ number associated with an ADC instance.
 */
IRQn_Type ADC_DRV_GetInterruptNumber(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    static const IRQn_Type adcIrqId[ADC_INSTANCE_COUNT] = ADC_IRQS;
    IRQn_Type irqId = adcIrqId[instance];

    return irqId;
}

/*!
 * @brief Clear the ADC-ready flag.
 */
void ADC_DRV_ClearReadyFlagCmd(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const baseAddr = s_adcBase[instance];

    ADC_ClearReadyFlagCmd(baseAddr);
}

/*!
 * @brief Clear the sample-end flag.
 */
void ADC_DRV_ClearSampEndFlagCmd(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const baseAddr = s_adcBase[instance];

    ADC_ClearSampEndFlagCmd(baseAddr);
}

/*!
 * @brief Clear the watchdog event flag.
 */
void ADC_DRV_ClearWdFlagCmd(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const baseAddr = s_adcBase[instance];
    ADC_ClearWdFlagCmd(baseAddr, 0);
}

/*!
 * @brief Clear the FIFO overrun flag.
 */
void ADC_DRV_ClearOvrFlagCmd(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const baseAddr = s_adcBase[instance];

    ADC_ClearOvrFlagCmd(baseAddr);
}

/*!
 * @brief Clear the end-of-sequence flag.
 */
void ADC_DRV_ClearEoseqFlagCmd(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const baseAddr = s_adcBase[instance];

    ADC_ClearEoseqFlagCmd(baseAddr);
}

/*!
 * @brief Clear the end-of-conversion flag.
 */
void ADC_DRV_ClearEocFlagCmd(const uint32_t instance)
{
    DEV_ASSERT(instance < ADC_INSTANCE_COUNT);

    ADC_Type *const baseAddr = s_adcBase[instance];

    ADC_ClearEocFlagCmd(baseAddr);
}

/******************************************************************************
 * EOF
 *****************************************************************************/
