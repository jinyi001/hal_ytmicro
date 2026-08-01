/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file acmp_driver.c
 * @version 1.4.1
 *
 * @brief ACMP Driver — implementation of the public ACMP_DRV_* API.
 *
 * This file implements the instance-based ACMP driver declared in
 * `acmp_driver.h`. Each function resolves the hardware base address for the
 * selected instance and delegates the register-level work to the internal
 * helpers declared in `acmp_hw_access.h`.
 */

#include "acmp_driver.h"
#include "acmp_hw_access.h"
#include <stddef.h>

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Table of base addresses for ACMP instances. */
static ACMP_Type *const g_acmpBase[] = ACMP_BASE_PTRS;

/*!
 * @brief Reset an ACMP instance through the IPC software reset path.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Execution status.
 */
status_t ACMP_DRV_Reset(const uint32_t instance)
{
    status_t status = STATUS_SUCCESS;
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);

    IPC->CTRL[instance + IPC_ACMP0_INDEX] |= IPC_CTRL_SWREN_MASK;
    IPC->CTRL[instance + IPC_ACMP0_INDEX] &= ~IPC_CTRL_SWREN_MASK;

    return status;
}

/*!
 * @brief Populate all ACMP sub-configuration blocks with default values.
 *
 * @param[in] config  Pointer to the configuration container whose member
 *                    blocks receive default values.
 * @return Execution status.
 */
status_t ACMP_DRV_GetDefaultConfig(const acmp_config_t *config)
{
    status_t status = STATUS_SUCCESS;
    DEV_ASSERT(config != NULL);
    DEV_ASSERT(config->comparatorConfig != NULL);
    DEV_ASSERT(config->dacConfig != NULL);
    DEV_ASSERT(config->muxConfig != NULL);
    DEV_ASSERT(config->continuousConfig != NULL);

#if FEATURE_ACMP_HAS_CLK_SRC_SEL
    (config->comparatorConfig)->acmpClkSrc = ACMP_CLK_SRC_SIRC;
#endif
    (config->comparatorConfig)->sampleMode = ACMP_COMMON_MODE;
    (config->comparatorConfig)->edgeSelection = ACMP_RISING_EDGE;
#if FEATURE_ACMP_HAS_OFFSET_CONTROL
    (config->comparatorConfig)->hardBlockOffset = false;
#endif
    (config->comparatorConfig)->hysteresisLevel = ACMP_HYS_LEVEL_0;
    (config->comparatorConfig)->outputSelect = ACMP_OUTPUT_DIRECTLY;
    (config->comparatorConfig)->outputPolarity = ACMP_OUT_POL_NOT_INVERT;
    (config->comparatorConfig)->filterEnable = false;
#if FEATURE_ACMP_HAS_AUTODIS
    (config->comparatorConfig)->autoDisableHardBlock = false;
#endif
    (config->comparatorConfig)->powerMode = ACMP_LOW_POWER;
    (config->comparatorConfig)->filterClkSrc = ACMP_FILTER_SEL_FUNC_CLK;
    (config->comparatorConfig)->interruptEnable = true;
    (config->comparatorConfig)->dmaTriggerEnable = false;

    (config->dacConfig)->enable = true;
    (config->dacConfig)->voltage = 127U;

    (config->muxConfig)->positiveInputSrc = ACMP_INPUT_SRC_MUX;
    (config->muxConfig)->negativeInputSrc = ACMP_INPUT_SRC_MUX;
    (config->muxConfig)->positiveInputChnSel = 0U;
    (config->muxConfig)->negativeInputChnSel = 1U;

    (config->continuousConfig)->continuousEnable = false;
    (config->continuousConfig)->continuousMode = ACMP_CONTINUOUS_LOOP_MODE;
    (config->continuousConfig)->continuousInterruptEnable = false;
    (config->continuousConfig)->fixedPort = ACMP_FIXED_NEG_PORT;
    (config->continuousConfig)->samplePeriod = FEATURE_ACMP_DEFAULT_CONT_PERIOD;
    (config->continuousConfig)->samplePosition = FEATURE_ACMP_DEFAULT_CONT_POS;
    for (uint8_t i = 0; i < 8; i++)
    {
        (config->continuousConfig)->channelConfig[i].enable = false;
        (config->continuousConfig)->channelConfig[i].expectation = ACMP_EXPECT_POS_LESS_THAN_NEG;
    }

    return status;
}

/*!
 * @brief Initialize an ACMP instance from the supplied configuration blocks.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] config    Pointer to the configuration container.
 * @return Execution status.
 */
status_t ACMP_DRV_Init(const uint32_t instance, const acmp_config_t *config)
{
    status_t status = STATUS_SUCCESS;
    DEV_ASSERT(config != NULL);
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);

    /* Configure acmp comparator mode */
    if (config->comparatorConfig != NULL)
    {
        (void)ACMP_DRV_ConfigComparator(instance, config->comparatorConfig);
    }
    /* Configure acmp dac */
    if (config->dacConfig != NULL)
    {
        (void)ACMP_DRV_ConfigDac(instance, config->dacConfig);
    }
    /* Configure acmp multiplexer */
    if (config->muxConfig != NULL)
    {
        (void)ACMP_DRV_ConfigMux(instance, config->muxConfig);
    }
    /* Clear all flags*/
    (void)ACMP_DRV_ClearOutputFlags(instance);
    (void)ACMP_DRV_ClearChannelFlags(instance);
    /* Configure continuous mode if it is enabled */
    if (config->continuousConfig != NULL)
    {
        if (config->continuousConfig->continuousEnable)
        {
            (void)ACMP_DRV_ConfigContinuous(instance, config->continuousConfig);
        }
    }

    return status;
}

/*!
 * @brief Write the packed expectation bitmap used by continuous mode.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] state     Packed expectation bitmap. Bit 0 controls channel 0 and
 *                      bit 7 controls channel 7.
 * @return Execution status.
 */
status_t ACMP_DRV_SetExpectation(const uint32_t instance, uint8_t state)
{
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const base = g_acmpBase[instance];
    /* Set channels expectation */
    ACMP_SetExpectation(base, state);
    return STATUS_SUCCESS;
}

/*!
 * @brief Request continuous scan mode through the runtime control path.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Execution status.
 */
status_t ACMP_DRV_EnableContinuous(const uint32_t instance)
{
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const base = g_acmpBase[instance];
    /* Set continuous mode enable state */
    ACMP_SetContinuousModeEnState(base, true);
    return STATUS_SUCCESS;
}

/*!
 * @brief Request continuous-mode disable through the runtime control path.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Execution status.
 */
status_t ACMP_DRV_DisableContinuous(const uint32_t instance)
{
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const base = g_acmpBase[instance];
    /* Disable continuous mode */
    ACMP_SetContinuousModeEnState(base, false);
    return STATUS_SUCCESS;
}

/*!
 * @brief Read back all ACMP configuration blocks for one instance.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] config    Pointer to the configuration container. All member
 *                      pointers receive the current ACMP state.
 * @return Execution status.
 */
status_t ACMP_DRV_GetConfigAll(const uint32_t instance, const acmp_config_t *config)
{
    DEV_ASSERT(config != NULL);
    DEV_ASSERT(config->comparatorConfig != NULL);
    DEV_ASSERT(config->dacConfig != NULL);
    DEV_ASSERT(config->muxConfig != NULL);
    DEV_ASSERT(config->continuousConfig != NULL);
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);

    /* Get ACMP sample mode configuration */
    (void)ACMP_DRV_GetComparatorConfig(instance, config->comparatorConfig);
    /* Get DAC configuration */
    (void)ACMP_DRV_GetDacConfig(instance, config->dacConfig);
    /* Get multiplexer configuration */
    (void)ACMP_DRV_GetMuxConfig(instance, config->muxConfig);
    /* Get continuous mode configuration */
    (void)ACMP_DRV_GetContinuousConfig(instance, config->continuousConfig);

    return STATUS_SUCCESS;
}

/*!
 * @brief Configure the comparator core.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] config    Pointer to the comparator configuration structure.
 * @return Execution status.
 */
status_t ACMP_DRV_ConfigComparator(const uint32_t instance, const acmp_comparator_config_t *config)
{
    DEV_ASSERT(config != NULL);
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const base = g_acmpBase[instance];

#if FEATURE_ACMP_HAS_CLK_SRC_SEL
    /* Set ACMP clock source */
    ACMP_SetClockSrc(base, (uint8_t)config->acmpClkSrc);
#endif
    /* Set ACMP sample mode */
    ACMP_SetSampleMode(base, config->sampleMode);
    /* Set edge selection */
    ACMP_SetEdgeSelection(base, config->edgeSelection);
#if FEATURE_ACMP_HAS_OFFSET_CONTROL
    /* Set hard block offset control */
    ACMP_SetHardBlockOffset(base, (config->hardBlockOffset ? 1U : 0U));
#endif
    /* Set hysteresis */
    ACMP_SetHysteresis(base, config->hysteresisLevel);
    /* Set output selection for filter or not */
    ACMP_SetOutputFilterSelection(base, config->outputSelect);
    /* Set output polarity */
    ACMP_SetOutputPolarity(base, config->outputPolarity);
    /* Set power mode */
    ACMP_SetPowerMode(base, (config)->powerMode);

    /* Set filter clock source */
    ACMP_SetFilterClockSrc(base, (uint8_t)config->filterClkSrc);
    
    if (config->filterEnable)
    {
#if defined(FEATURE_ACMP_HAS_FILTER_BYPASS) && (FEATURE_ACMP_HAS_FILTER_BYPASS == 1)
        /* Enable filter */
        ACMP_SetFilterEnState(base, true);
#endif
        /* Set filter sample period */
        ACMP_SetFilterSamplePeriod(base, config->filterSamplePeriod);
        /* Set filter sample count */
        ACMP_SetFilterSampleCount(base, config->filterSampleCount);
    }
    else
    {
#if FEATURE_ACMP_HAS_FILTER_BYPASS
        ACMP_SetFilterEnState(base, false);
#endif
        /* Set filter sample period */
        ACMP_SetFilterSamplePeriod(base, 0U);
        /* Set filter sample count */
        ACMP_SetFilterSampleCount(base, 0U);
    }
#if FEATURE_ACMP_HAS_AUTODIS
    /* Set auto disable hard block */
    ACMP_SetAutoDisableHardBlock(base, config->autoDisableHardBlock);
#endif
    /* Set interrupt enable state */
    ACMP_SetInterruptEnState(base, config->interruptEnable);
    /* Set DMA enable state */
    ACMP_SetDmaEnState(base, config->dmaTriggerEnable);

    return STATUS_SUCCESS;
}

/*!
 * @brief Read back the comparator core configuration.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[out] config   Pointer to the structure that receives the comparator state.
 * @return Execution status.
 */
status_t ACMP_DRV_GetComparatorConfig(const uint32_t instance, acmp_comparator_config_t *config)
{
    DEV_ASSERT(config != NULL);
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const base = g_acmpBase[instance];

#if FEATURE_ACMP_HAS_CLK_SRC_SEL
    config->acmpClkSrc = (acmp_clock_source_t)ACMP_GetClockSrc(base);
#endif
    config->sampleMode = (acmp_sample_mode_t)ACMP_GetSampleMode(base);
    config->edgeSelection = (acmp_edge_select_t)ACMP_GetEdgeSelection(base);
#if FEATURE_ACMP_HAS_OFFSET_CONTROL
    config->hardBlockOffset = ACMP_GetHardBlockOffset(base);
#endif
    config->hysteresisLevel = (acmp_hysteresis_t)ACMP_GetHysteresis(base);
    config->outputSelect = (acmp_output_select_t)ACMP_GetOutputFilterSelection(base);
    config->outputPolarity = (acmp_output_polarity_t)ACMP_GetOutputPolarity(base);
    config->powerMode = (acmp_power_mode_t)ACMP_GetPowerMode(base);
#if FEATURE_ACMP_HAS_AUTODIS
    config->autoDisableHardBlock = ACMP_GetAutoDisableHardBlock(base);
#endif

#if defined(FEATURE_ACMP_HAS_FILTER_BYPASS) && (FEATURE_ACMP_HAS_FILTER_BYPASS == 1)
    if (ACMP_GetFilterEnState(base))
    {
        config->filterEnable = true;
    }
    else
    {
        config->filterEnable = false;
    }
#else
    if (ACMP_GetFilterSampleCount(base) != 0U && ACMP_GetFilterSamplePeriod(base) != 0U)
    {
        config->filterEnable = true;
    }
    else
    {
        config->filterEnable = false;
    }
#endif
    config->filterClkSrc = (acmp_filter_clk_src_t)ACMP_GetFilterClockSrc(base);
    config->filterSampleCount = ACMP_GetFilterSampleCount(base);
    config->filterSamplePeriod = ACMP_GetFilterSamplePeriod(base);
    config->interruptEnable = ACMP_GetInterruptEnState(base);
    config->dmaTriggerEnable = ACMP_GetDmaTriggerEnable(base);

    return STATUS_SUCCESS;
}

/*!
 * @brief Configure the integrated DAC threshold generator.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] config    Pointer to the DAC configuration structure.
 * @return Execution status.
 */
status_t ACMP_DRV_ConfigDac(const uint32_t instance, const acmp_dac_config_t *config)
{
    DEV_ASSERT(config != NULL);
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
#if ACMP_DAC_RESOLUTION != 255U
    DEV_ASSERT(config->voltage <= ACMP_DAC_RESOLUTION);
#endif
    ACMP_Type *const base = g_acmpBase[instance];

    ACMP_SetDacEnState(base, config->enable);
#if FEATURE_ACMP_HAS_DAC_OUTPUT
    ACMP_SetDacOutEnState(base, config->outputEnable);
#endif
    ACMP_SetVoltage(base, config->voltage);

    return STATUS_SUCCESS;
}

/*!
 * @brief Read back the integrated DAC configuration.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[out] config   Pointer to the structure that receives the DAC state.
 * @return Execution status.
 */
status_t ACMP_DRV_GetDacConfig(const uint32_t instance, acmp_dac_config_t *config)
{
    DEV_ASSERT(config != NULL);
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const base = g_acmpBase[instance];

    config->enable = ACMP_GetDacEnState(base);
#if FEATURE_ACMP_HAS_DAC_OUTPUT
    config->outputEnable = ACMP_GetDacOutEnState(base);
#endif
    config->voltage = ACMP_GetVoltage(base);

    return STATUS_SUCCESS;
}

/*!
 * @brief Configure the positive and negative input routes.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] config    Pointer to the MUX configuration structure.
 * @return Execution status.
 */
status_t ACMP_DRV_ConfigMux(const uint32_t instance, const acmp_mux_config_t *config)
{
    DEV_ASSERT(config != NULL);
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const base = g_acmpBase[instance];
    /* Set positive port input source */
    ACMP_SetPositivePortInputSrc(base, config->positiveInputSrc);
    /* Set negative port input source */
    ACMP_SetNegativePortInputSrc(base, config->negativeInputSrc);
    /* Set positive port input channel */
    ACMP_SetPositivePortInputChannel(base, config->positiveInputChnSel);
    /* Set negative port input channel */
    ACMP_SetNegativePortInputChannel(base, config->negativeInputChnSel);

    return STATUS_SUCCESS;
}

/*!
 * @brief Read back the positive and negative input routes.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[out] config   Pointer to the structure that receives the routing state.
 * @return Execution status.
 */
status_t ACMP_DRV_GetMuxConfig(const uint32_t instance, acmp_mux_config_t *config)
{
    status_t status = STATUS_SUCCESS;
    DEV_ASSERT(config != NULL);
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const base = g_acmpBase[instance];
    /* Get positive port input source */
    config->positiveInputSrc = (acmp_input_source_t)ACMP_GetPositivePortInputSrc(base);
    /* Get negative port input source */
    config->negativeInputSrc = (acmp_input_source_t)ACMP_GetNegativePortInputSrc(base);
    /* Get positive port input channel */
    config->positiveInputChnSel = ACMP_GetPositivePortInputChannel(base);
    /* Get negative port input channel */
    config->negativeInputChnSel = ACMP_GetNegativePortInputChannel(base);

    return status;
}

/*!
 * @brief Configure continuous scan mode.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] config    Pointer to the continuous-mode configuration structure.
 * @return Execution status.
 */
status_t ACMP_DRV_ConfigContinuous(const uint32_t instance, const acmp_continuous_config_t *config)
{
    DEV_ASSERT(config != NULL);
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const base = g_acmpBase[instance];
    uint8_t enTemp = 0;
    uint8_t expectTemp = 0;

    /* Set continuous one-shot or loop mode */
    ACMP_SetContinuousMode(base, config->continuousMode);

#if FEATURE_ACMP_HAS_TRIG_MODE_GATE
    if ((config->continuousMode == ACMP_CONTINUOUS_ONE_SHOT_MODE) &&
        (config->oneshotTriggerEnable == true))
    {
        ACMP_SetOneshotTriggerMode(base, true);
    }
#endif

    /* Set continuous interrupt enable state */
    ACMP_SetContinuousModeInterruptEnState(base, config->continuousInterruptEnable);
    /* Set fixed port */
    ACMP_SetFixedPort(base, config->fixedPort);
    /* Set continuous mode sample period */
    ACMP_SetContinuousModeSamplePeriod(base, config->samplePeriod);
    /* Set continuous mode sample position */
    ACMP_SetContinuousModeSamplePosition(base, config->samplePosition);
    for (uint8_t i = 0; i < 8U; i++)
    {
        enTemp |= (config->channelConfig[i].enable ? 1U : 0U) << i;
        expectTemp |= (uint8_t)config->channelConfig[i].expectation << i;
    }
    /* Set continuous mode input channels */
    ACMP_SetContinuousModeChannels(base, enTemp);
    /* Set expectation for each channel comparison */
    ACMP_SetExpectation(base, expectTemp);
    /* Set acmp continuous mode */
#if FEATURE_ACMP_HAS_CONTINUOUS_MODE_IN_CTRL_REG
    if (config->continuousEnable)
    {
        ACMP_SetSampleMode(base, ACMP_CONTINUOUS_MODE);
    }
#else
    if (config->continuousEnable)
    {
        ACMP_SetContinuousModeEnState(base, true);
    }
    else
    {
        ACMP_SetContinuousModeEnState(base, false);
    }
#endif

    return STATUS_SUCCESS;
}

/*!
 * @brief Read back continuous scan configuration.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[out] config   Pointer to the structure that receives the continuous-mode state.
 * @return Execution status.
 */
status_t ACMP_DRV_GetContinuousConfig(const uint32_t instance, acmp_continuous_config_t *config)
{
    DEV_ASSERT(config != NULL);
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const base = g_acmpBase[instance];

#if FEATURE_ACMP_HAS_CONTINUOUS_MODE_IN_CTRL_REG
    config->continuousEnable = (ACMP_GetSampleMode(base) == (uint8_t)ACMP_CONTINUOUS_MODE) ? true : false;
#else
    config->continuousEnable = ACMP_GetContinuousModeEnState(base);
#endif
    config->continuousInterruptEnable = ACMP_GetContinuousModeInterruptEnState(base);
    config->continuousMode = (acmp_continuous_mode_t)ACMP_GetContinuousMode(base);
    config->fixedPort = (acmp_fixed_port_t)ACMP_GetFixedPort(base);
    config->samplePeriod = ACMP_GetContinuousModeSamplePeriod(base);
    config->samplePosition = ACMP_GetContinuousModeSamplePosition(base);
    for (uint8_t i = 0; i < 8U; i++)
    {
        config->channelConfig[i].enable = ACMP_GetContinuousChannelEnState(base, i);
        config->channelConfig[i].expectation = (acmp_ch_expectation_config_t)ACMP_GetContinuousChannelExpectation(base, i);
    }

    return STATUS_SUCCESS;
}

/*!
 * @brief Read the comparator output event flags.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[out] flags    Pointer to the variable that receives the output flags.
 * @return Execution status.
 */
status_t ACMP_DRV_GetOutputFlags(const uint32_t instance, uint8_t *flags)
{
    DEV_ASSERT(flags != NULL);
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const baseAddr = g_acmpBase[instance];
    *flags = ACMP_GetOutputFlags(baseAddr);
    return STATUS_SUCCESS;
}

/*!
 * @brief Clear the comparator output event flags.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Execution status.
 */
status_t ACMP_DRV_ClearOutputFlags(const uint32_t instance)
{
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const baseAddr = g_acmpBase[instance];
    ACMP_ClearOutputFlags(baseAddr);
    return STATUS_SUCCESS;
}

/*!
 * @brief Read the per-channel change flags used by continuous mode.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[out] flags    Pointer to the variable that receives the channel flags.
 * @return Execution status.
 */
status_t ACMP_DRV_GetChannelFlags(const uint32_t instance, acmp_ch_list_t *flags)
{
    DEV_ASSERT(flags != NULL);
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const baseAddr = g_acmpBase[instance];
    *flags = ACMP_GetChannelFlags(baseAddr);
    return STATUS_SUCCESS;
}

/*!
 * @brief Clear all continuous-mode channel change flags.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Execution status.
 */
status_t ACMP_DRV_ClearChannelFlags(const uint32_t instance)
{
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const baseAddr = g_acmpBase[instance];
    ACMP_ClearChannelFlags(baseAddr);
    return STATUS_SUCCESS;
}

/*!
 * @brief Enable the comparator block for the selected instance.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Execution status.
 */
status_t ACMP_DRV_Enable(const uint32_t instance)
{
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const baseAddr = g_acmpBase[instance];
    ACMP_Enable(baseAddr);
    return STATUS_SUCCESS;
}

/*!
 * @brief Disable the comparator block for the selected instance.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Execution status.
 */
status_t ACMP_DRV_Disable(const uint32_t instance)
{
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    ACMP_Type *const baseAddr = g_acmpBase[instance];
    ACMP_Disable(baseAddr);
    return STATUS_SUCCESS;
}

/*!
 * @brief Report the channel currently being sampled in continuous mode.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Current channel identifier.
 */
uint8_t ACMP_DRV_GetChannelId(const uint32_t instance)
{
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);
    uint8_t channelId = 0U;
    ACMP_Type *const baseAddr = g_acmpBase[instance];
    /* Get channel ID when sampling */
    channelId = ACMP_GetChannelId(baseAddr);
    return channelId;
}

/*!
 * @brief Read the current comparator output state.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return `true` when the comparator output is high, or `false` otherwise.
 */
bool ACMP_DRV_GetOutput(const uint32_t instance)
{
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);

    ACMP_Type *baseAddr = g_acmpBase[instance];

    return ACMP_GetOutput(baseAddr);
}

/*!
 * @brief Read the last captured output for a channel scanned in continuous mode.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] channel   Channel index to query.
 * @return `true` when the stored channel output is high, or `false` otherwise.
 */
bool ACMP_DRV_GetChannelOutput(const uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < ACMP_INSTANCE_COUNT);

    ACMP_Type *baseAddr = g_acmpBase[instance];
    
    return (ACMP_GetContinuousChannelOutput(baseAddr, channel)) != 0U;
}

/******************************************************************************
 * EOF
 *****************************************************************************/
