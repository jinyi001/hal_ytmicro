/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_hw_access.c
 * @version 1.4.1
 *
 * @brief eTMR HW Access Layer — implementation.
 *
 * This file implements the hardware-register access functions that
 * cannot be inlined, including status flag read/clear operations,
 * interrupt enable/disable, IO status queries, channel mode control,
 * and fault configuration.
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 2985 Rule 2.2: This operation is redundant. The value of the result is
 *                       always that of the left-hand operand.
 * PRQA S 2986 Rule 2.2: This operation is redundant. The value of the result is
 *                       always that of the right-hand operand.
 *
 */

#include "etmr_hw_access.h"


#if defined(FEATURE_eTMR_HAS_HARDWARE_CAPTURE) && (FEATURE_eTMR_HAS_HARDWARE_CAPTURE == 1U)
/*!
 * @brief Get the negative-pulse ready flag for one channel.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @param[in] channel   Channel index.
 * @return `true` if the negative-pulse measurement is ready, `false` otherwise.
 */
bool eTMR_DRV_GetChnNegPulseRdyFlag(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    return ((etmrBase->STS & (eTMR_STS_CH0NPF_MASK << channel)) >> (eTMR_STS_CH0NPF_SHIFT + channel)) != 0U;
}

/*!
 * @brief Get the positive-pulse ready flag for one channel.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @param[in] channel   Channel index.
 * @return `true` if the positive-pulse measurement is ready, `false` otherwise.
 */
bool eTMR_DRV_GetChnPosPulseRdyFlag(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    return ((etmrBase->STS & (eTMR_STS_CH0PPF_MASK << channel)) >> (eTMR_STS_CH0PPF_SHIFT + channel)) != 0U;
}
#endif

/*!
 * @brief Get the current quadrature decoder direction.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return `true` when the quadrature decoder is counting up, `false` otherwise.
 */
bool eTMR_DRV_GetQuadDecodeDir(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    return (((etmrBase->STS) & eTMR_STS_QDDIR_MASK) >> eTMR_STS_QDDIR_SHIFT) != 0U;
}

/*!
 * @brief Get the quadrature decoder counter overflow flag.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return `true` if the overflow flag is set, `false` otherwise.
 */
bool eTMR_DRV_GetQuadDecodeTofFlag(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    return (((etmrBase->STS) & eTMR_STS_QDTOF_MASK) >> eTMR_STS_QDTOF_SHIFT) != 0U;
}

/*!
 * @brief Get the eTMR counter overflow flag.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return `true` if the counter overflow flag is set, `false` otherwise.
 */
bool eTMR_DRV_GetTofFlag(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    return (((etmrBase->STS) & eTMR_STS_TOF_MASK) >> eTMR_STS_TOF_SHIFT) != 0U;
}

/*!
 * @brief Get the eTMR reload flag.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return `true` if the reload flag is set, `false` otherwise.
 */
bool eTMR_DRV_GetReloadFlag(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    return (((etmrBase->STS) & eTMR_STS_RF_MASK) >> eTMR_STS_RF_SHIFT) != 0U;
}

/*!
 * @brief Get the fault flag for one fault channel.
 *
 * @param[in] instance    eTMR instance index (0-based).
 * @param[in] fltChannel  Fault channel index.
 * @return `true` if the fault flag is set, `false` otherwise.
 */
bool eTMR_DRV_GetFaultFlag(uint32_t instance, uint8_t fltChannel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(fltChannel < (FEATURE_eTMR_CHANNEL_MAX_COUNT >> 1));
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    return ((etmrBase->STS & ((uint32_t)eTMR_STS_F0F_MASK << fltChannel)) >> (eTMR_STS_F0F_SHIFT + fltChannel)) != 0U;
}

/*!
 * @brief Get the interrupt flag for one eTMR channel.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @param[in] channel   Channel index.
 * @return `true` if the channel flag is set, `false` otherwise.
 */
bool eTMR_DRV_GetChnFlag(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    return (((etmrBase->STS) & ((uint32_t)0x1U << channel)) >> channel) != 0U;
}

#if defined(FEATURE_eTMR_HAS_HARDWARE_CAPTURE) && (FEATURE_eTMR_HAS_HARDWARE_CAPTURE == 1U)
/*!
 * @brief Clear the negative-pulse ready flag for one channel.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @param[in] channel   Channel index.
 */
void eTMR_DRV_ClearChnNegPulseRdyFlag(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    etmrBase->STS = eTMR_STS_CH0NPF_MASK << channel;
}

/*!
 * @brief Clear the positive-pulse ready flag for one channel.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @param[in] channel   Channel index.
 */
void eTMR_DRV_ClearChnPosPulseRdyFlag(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    etmrBase->STS = eTMR_STS_CH0PPF_MASK << channel;
}
#endif

/*!
 * @brief Clear the quadrature decoder counter overflow flag.
 *
 * @param[in] instance  eTMR instance index (0-based).
 */
void eTMR_DRV_ClearQuadDecodeTofFlag(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    etmrBase->STS = eTMR_STS_QDTOF_MASK;
}

/*!
 * @brief Clear the eTMR counter overflow flag.
 *
 * @param[in] instance  eTMR instance index (0-based).
 */
void eTMR_DRV_ClearTofFlag(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    etmrBase->STS = eTMR_STS_TOF_MASK;
}

/*!
 * @brief Clear the eTMR reload flag.
 *
 * @param[in] instance  eTMR instance index (0-based).
 */
void eTMR_DRV_ClearReloadFlag(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    etmrBase->STS = eTMR_STS_RF_MASK;
}

/*!
 * @brief Clear the fault flag for one fault channel.
 *
 * @param[in] instance    eTMR instance index (0-based).
 * @param[in] fltChannel  Fault channel index.
 */
void eTMR_DRV_ClearFaultFlag(uint32_t instance, uint8_t fltChannel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(fltChannel < FEATURE_eTMR_FAULT_CHANNELS);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    etmrBase->STS = (uint32_t)eTMR_STS_F0F_MASK << fltChannel;
}

/*!
 * @brief Clear one or more channel interrupt flags.
 *
 * @param[in] instance    eTMR instance index (0-based).
 * @param[in] channelSet  Bitmask of channels to clear.
 */
void eTMR_DRV_ClearChnFlag(uint32_t instance, uint8_t channelSet)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    etmrBase->STS = ((uint32_t)channelSet << eTMR_STS_CH0F_SHIFT); /*PRQA S 2985*/
}

/*!
 * @brief Get the current phase-B input status in quadrature decode mode.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return Phase-B input status.
 */
uint8_t eTMR_DRV_GetPhaseBStatus(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    return (uint8_t)(((etmrBase->IOSTS) & eTMR_IOSTS_PHB_MASK) >> eTMR_IOSTS_PHB_SHIFT);
}

/*!
 * @brief Get the current phase-A input status in quadrature decode mode.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return Phase-A input status.
 */
uint8_t eTMR_DRV_GetPhaseAStatus(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    return (uint8_t)(((etmrBase->IOSTS) & eTMR_IOSTS_PHA_MASK) >> eTMR_IOSTS_PHA_SHIFT);
}

/*!
 * @brief Get the live input status of one fault channel.
 *
 * @param[in] instance    eTMR instance index (0-based).
 * @param[in] fltChannel  Fault channel index.
 * @return Fault input status.
 */
uint8_t eTMR_DRV_GetFaultInputStatus(uint32_t instance, uint8_t fltChannel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(fltChannel < FEATURE_eTMR_FAULT_CHANNELS);
    eTMR_Type *const etmrBase = g_etmrBase[instance];
    return (uint8_t)((etmrBase->IOSTS & ((uint32_t)eTMR_IOSTS_F0_MASK << fltChannel)) >> (eTMR_IOSTS_F0_SHIFT + fltChannel));
}

/*!
 * @brief Get the IO status of one eTMR channel.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @param[in] channel   Channel index.
 * @return Current IO status for the channel.
 */
uint8_t eTMR_DRV_GetChnIoStatus(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    return (uint8_t)((etmrBase->IOSTS >> channel) & eTMR_IOSTS_CH0IO_MASK);
}

/*!
 * @brief Configure one channel pair for PWM complementary or independent mode.
 *
 * @param[in] instance     eTMR instance index (0-based).
 * @param[in] channelPair  Channel-pair index.
 * @param[in] mode         PWM mode selection for the pair.
 */
void eTMR_DRV_SetChnCompMode(uint32_t instance, uint8_t channelPair, etmr_pwm_mode_t mode)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channelPair < (FEATURE_eTMR_CHANNEL_MAX_COUNT >> 1));
    eTMR_Type *etmrBase = g_etmrBase[instance];
    etmrBase->CTRL &= ~(eTMR_CTRL_COMPL01_MASK << channelPair);
    etmrBase->CTRL |= ((uint32_t)mode << (eTMR_CTRL_COMPL01_SHIFT + channelPair));
}

#if FEATURE_eTMR_HAS_DOUBLE_SWITCH
/*!
 * @brief Enable or disable double-switch mode for one channel pair.
 *
 * @param[in] instance     eTMR instance index (0-based).
 * @param[in] channelPair  Channel-pair index.
 * @param[in] en           `true` to enable double-switch mode.
 */
void eTMR_DRV_SetChnDoubleSwitch(uint32_t instance, uint8_t channelPair, bool en)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channelPair < (FEATURE_eTMR_CHANNEL_MAX_COUNT >> 1));
    eTMR_Type *etmrBase = g_etmrBase[instance];
    etmrBase->CTRL &= ~(eTMR_CTRL_DBSW01_MASK << channelPair);
    etmrBase->CTRL |= ((uint32_t)(en ? 1U : 0U) << (eTMR_CTRL_DBSW01_SHIFT + channelPair));
}
#endif

#if FEATURE_eTMR_HAS_COMBINATION_CAPTURE
/*!
 * @brief Enable or disable combination-capture mode for one channel pair.
 *
 * @param[in] instance     eTMR instance index (0-based).
 * @param[in] channelPair  Channel-pair index.
 * @param[in] en           `true` to enable combination capture.
 */
void eTMR_DRV_SetChnCombMode(uint32_t instance, uint8_t channelPair, bool en)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channelPair < (FEATURE_eTMR_CHANNEL_MAX_COUNT >> 1));
    eTMR_Type *etmrBase = g_etmrBase[instance];
    etmrBase->CTRL &= ~(eTMR_CTRL_COMB01_MASK << channelPair);
    etmrBase->CTRL |= ((uint32_t)(en ? 1U : 0U) << (eTMR_CTRL_COMB01_SHIFT + channelPair));
}

/*!
 * @brief Select the combination-capture source for one channel pair.
 *
 * @param[in] instance     eTMR instance index (0-based).
 * @param[in] channelPair  Channel-pair index.
 * @param[in] src          Combination-capture source selection.
 */
void eTMR_DRV_SetChnCombSrc(uint32_t instance, uint8_t channelPair, uint8_t src)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channelPair < (FEATURE_eTMR_CHANNEL_MAX_COUNT >> 1));
    eTMR_Type *etmrBase = g_etmrBase[instance];
    etmrBase->CTRL &= ~(eTMR_CTRL_COMBSRC01_MASK << channelPair);
    etmrBase->CTRL |= ((uint32_t)src << (eTMR_CTRL_COMBSRC01_SHIFT + channelPair));
}
#endif

/*!
 * @brief Set the input polarity for one fault channel.
 *
 * @param[in] instance    eTMR instance index (0-based).
 * @param[in] fltChannel  Fault channel index.
 * @param[in] pol         Fault input polarity value.
 */
void eTMR_DRV_SetFaultChnPol(uint32_t instance, uint8_t fltChannel, uint32_t pol)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(fltChannel < FEATURE_eTMR_FAULT_CHANNELS);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    etmrBase->FAULT &= ~((uint32_t)eTMR_FAULT_F0POL_MASK << fltChannel);
    etmrBase->FAULT |= (pol << (eTMR_FAULT_F0POL_SHIFT + fltChannel));
}

/*!
 * @brief Enable or disable one fault channel.
 *
 * @param[in] instance    eTMR instance index (0-based).
 * @param[in] fltChannel  Fault channel index.
 * @param[in] enable      `true` to enable the fault channel.
 */
void eTMR_DRV_SetFaultChnEnable(uint32_t instance, uint8_t fltChannel, bool enable)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(fltChannel < FEATURE_eTMR_FAULT_CHANNELS);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    etmrBase->FAULT &= ~((uint32_t)eTMR_FAULT_F0EN_MASK << fltChannel);
    etmrBase->FAULT |= ((uint32_t)(enable ? 1U : 0U) << (eTMR_FAULT_F0EN_SHIFT + fltChannel)); /*PRQA S 2986*/
}

/*!
 * @brief Enable the quadrature decoder overflow interrupt.
 *
 * @param[in] instance  eTMR instance index (0-based).
 */
void eTMR_DRV_EnableQuadDecodeTofInt(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    eTMR_SetQdTofIntEnable(etmrBase, true);
}

/*!
 * @brief Disable the quadrature decoder overflow interrupt.
 *
 * @param[in] instance  eTMR instance index (0-based).
 */
void eTMR_DRV_DisableQuadDecodeTofInt(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    eTMR_SetQdTofIntEnable(etmrBase, false);
}

/*!
 * @brief Enable the eTMR counter overflow interrupt.
 *
 * @param[in] instance  eTMR instance index (0-based).
 */
void eTMR_DRV_EnableTofInt(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    eTMR_SetTofIntEnable(etmrBase, true);
}

/*!
 * @brief Disable the eTMR counter overflow interrupt.
 *
 * @param[in] instance  eTMR instance index (0-based).
 */
void eTMR_DRV_DisableTofInt(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    eTMR_SetTofIntEnable(etmrBase, false);
}

/*!
 * @brief Enable the interrupt for one channel.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @param[in] channel   Channel index.
 */
void eTMR_DRV_EnableChnInt(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    etmrBase->INTE &= ~((uint32_t)eTMR_INTE_CH0IE_MASK << channel);
    etmrBase->INTE |= ((uint32_t)0x1U << (eTMR_INTE_CH0IE_SHIFT + channel)); /*PRQA S 2986*/
}

/*!
 * @brief Enable interrupts for a set of channels.
 *
 * @param[in] instance    eTMR instance index (0-based).
 * @param[in] channelSet  Bitmask of channels to enable.
 */
void eTMR_DRV_SetChnsetIntEnable(uint32_t instance, uint8_t channelSet)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    etmrBase->INTE |= ((uint32_t)channelSet << eTMR_INTE_CH0IE_SHIFT); /*PRQA S 2985*/
}

/*!
 * @brief Disable the interrupt for one channel.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @param[in] channel   Channel index.
 */
void eTMR_DRV_DisableChnInt(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    etmrBase->INTE &= ~((uint32_t)eTMR_INTE_CH0IE_MASK << channel);
}

/*!
 * @brief Disable interrupts for a set of channels.
 *
 * @param[in] instance    eTMR instance index (0-based).
 * @param[in] channelSet  Bitmask of channels to disable.
 */
void eTMR_DRV_SetChnsetIntDisable(uint32_t instance, uint8_t channelSet)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    etmrBase->INTE &= ~((uint32_t)channelSet << eTMR_INTE_CH0IE_SHIFT); /*PRQA S 2985*/
}

/*!
 * @brief Check whether the counter overflow interrupt is enabled.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return `true` if the interrupt is enabled, `false` otherwise.
 */
bool eTMR_DRV_IsTofIntEnabled(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    return (((etmrBase->INTE) & eTMR_INTE_TOIE_MASK) >> eTMR_INTE_TOIE_SHIFT) != 0U;
}

/*!
 * @brief Check whether the quadrature decoder overflow interrupt is enabled.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return `true` if the interrupt is enabled, `false` otherwise.
 */
bool eTMR_DRV_IsQuadDecodeTofIntEnabled(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    return (((etmrBase->INTE) & eTMR_INTE_QDTOIE_MASK) >> eTMR_INTE_QDTOIE_SHIFT) != 0U;
}

/*!
 * @brief Check whether the fault interrupt is enabled.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return `true` if the interrupt is enabled, `false` otherwise.
 */
bool eTMR_DRV_IsFaultIntEnabled(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    return (((etmrBase->INTE) & eTMR_INTE_FIE_MASK) >> eTMR_INTE_FIE_SHIFT) != 0U;
}

/*!
 * @brief Check whether the interrupt for one channel is enabled.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @param[in] channel   Channel index.
 * @return `true` if the channel interrupt is enabled, `false` otherwise.
 */
bool eTMR_DRV_IsChnIntEnabled(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    DEV_ASSERT(channel < g_etmrChannelNum[instance]);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    return ((etmrBase->INTE & ((uint32_t)eTMR_INTE_CH0IE_MASK << channel)) >> (eTMR_INTE_CH0IE_SHIFT + channel)) != 0U; /*PRQA S 2986*/
}

/*!
 * @brief Get the current eTMR counter value.
 *
 * @param[in] instance  eTMR instance index (0-based).
 * @return Current counter value.
 */
uint32_t eTMR_DRV_GetCntVal(uint32_t instance)
{
    DEV_ASSERT(instance < eTMR_INSTANCE_COUNT);
    eTMR_Type *etmrBase = g_etmrBase[instance];
    return eTMR_GetCntVal(etmrBase);
}

/*! @}*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
