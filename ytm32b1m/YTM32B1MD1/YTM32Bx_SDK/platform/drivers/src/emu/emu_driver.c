/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file emu_driver.c
 * @version 1.4.1
 *
 * @brief EMU Driver - implementation of the public EMU_DRV_* API.
 *
 * This file implements the application-level ECC Management Unit driver
 * declared in emu_driver.h. Each public API resolves the EMU instance base
 * address and delegates register programming to the internal helpers in
 * emu_hw_access.h.
 */

#include "emu_driver.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Table of base addresses for EMU instances. */
static EMU_Type *g_emuBase[EMU_INSTANCE_COUNT] = EMU_BASE_PTRS;

/*******************************************************************************
 * Internal Helpers
 ******************************************************************************/

/*!
 * @brief Apply the caller-provided channel configuration to one EMU instance.
 */
static void EMU_DRV_SetConfig(uint32_t instance, const emu_config_t *configPtr)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(configPtr != NULL);

    uint8_t channel = (uint8_t)configPtr->channel;
    uint32_t injectErrAddr = configPtr->injectErrAddr;
    emu_databit_type dataBit = (uint32_t)configPtr->injectErrSetInfo.dataBit;
    emu_chkbit_type chkBit = (uint32_t)configPtr->injectErrSetInfo.chkBit;
    emu_inject_bits_type injectBitsType = configPtr->injectErrSetInfo.injectBitsType;
    bool interruptEnable = configPtr->interruptEnable;

    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    EMU_Type *const base = g_emuBase[instance];

    /* Program the target address used to trigger the ECC test event. */
    EMU_SetErrInjectAddr(base, channel, injectErrAddr);
    if (injectBitsType == EMU_INJECT_BITS_TYPE_CHK)
    {
        /* Program the requested single check-bit injection. */
        if (chkBit != NO_INJECTION_ERROR)
        {
            EMU_SetErrInjectChkbit(base, channel, chkBit);
        }
    }
    else if (injectBitsType == EMU_INJECT_BITS_TYPE_DATA)
    {
        /* Program the requested single data-bit injection. */
        if (dataBit != NO_INJECTION_ERROR)
        {
            EMU_SetErrInjectData(base, channel, dataBit);
        }
    }
    else if (injectBitsType == EMU_INJECT_BITS_TYPE_MIXTURE)
    {
        /* Program the requested check-bit portion of the mixed injection. */
        if (chkBit != NO_INJECTION_ERROR)
        {
            EMU_SetErrInjectChkbit(base, channel, chkBit);
        }
        /* Program the requested data-bit portion of the mixed injection. */
        if (dataBit != NO_INJECTION_ERROR)
        {
            EMU_SetErrInjectData(base, channel, dataBit);
        }
    }
    else
    {
        /* Leave the injection path unchanged for unsupported selector values. */
    }

    if (true == interruptEnable)
    {
        /* Enable both single-bit and double-bit interrupt signaling. */
        EMU_EnableInterruptSignalBit(base, channel);
        EMU_EnableInterruptDoubleBit(base, channel);
    }
    else
    {
        /* Disable both single-bit and double-bit interrupt signaling. */
        EMU_DisableInterruptSignalBit(base, channel);
        EMU_DisableInterruptDoubleBit(base, channel);
    }
}

/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/

/*!
 * @brief Reset an EMU instance, apply one channel configuration, and control
 *        the initial enable state.
 */
void EMU_DRV_Init(uint32_t instance, const emu_config_t *configPtr, uint32_t startFlag)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(configPtr != NULL);

    EMU_Type *const base = g_emuBase[instance];
    uint8_t injectErrChannel = (uint8_t)configPtr->channel;

    /* Restore the full EMU instance to reset state before programming it. */
    EMU_Init(base);

    /* Apply the caller-provided configuration for the selected channel. */
    EMU_DRV_SetConfig(instance, configPtr);

    if (startFlag == 1U)
    {
        /* Start injection immediately for the configured channel. */
        EMU_Enable(base, injectErrChannel);
    }
    else
    {
        /* Leave the configured channel disabled until the caller enables it. */
        EMU_Disable(base, injectErrChannel);
    }
}

/*!
 * @brief Disable the selected channel and restore the EMU instance to reset
 *        state.
 */
void EMU_DRV_DeInit(uint32_t instance, uint32_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    EMU_Type *const base = g_emuBase[instance];

    /* Disable the requested channel before clearing the instance state. */
    EMU_Disable(base, (uint8_t)channel);
    /* Reset every EMU register back to its default value. */
    EMU_Init(base);
}

/*******************************************************************************
 * Error Injection Control
 ******************************************************************************/

/*!
 * @brief Program raw multi-bit injection masks for one channel and re-enable
 *        EMU injection.
 */
void EMU_DRV_InjectError(uint32_t instance, uint32_t channel, uint32_t injectErrAddr,
                         emu_databit_type dataMulti, emu_chkbit_type chkMulti)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    EMU_Type *const base = g_emuBase[instance];

    EMU_Disable(base, (uint8_t)channel);

    /* Load the new target address for the injected error. */
    EMU_SetErrInjectAddr(base, (uint8_t)channel, injectErrAddr);

    /* Load the raw check-bit mask when the caller requests it. */
    if (chkMulti != NO_INJECTION_ERROR)
    {
        EMU_SetErrInjectChkMulti(base, (uint8_t)channel, chkMulti);
    }

    /* Load the raw data-bit mask when the caller requests it. */
    if (dataMulti != NO_INJECTION_ERROR)
    {
        EMU_SetErrInjectDataMulti(base, (uint8_t)channel, dataMulti);
    }

    EMU_Enable(base, (uint8_t)channel);
}

/*******************************************************************************
 * Error Report & Counter Access
 ******************************************************************************/

/*!
 * @brief Convert the latched syndrome for one channel into a reported
 *        check-bit index.
 */
uint8_t EMU_DRV_GetErrReportChkbit(uint32_t instance, uint8_t channel)
{
    uint8_t chkbit = 0;
    volatile uint32_t value;
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);
    EMU_Type *const base = g_emuBase[instance];

    value = EMU_GetSyndrome(base, channel);
    while (value > 0U)
    {
        value = value >> 1U;
        if (value == 0U)
        {
            break;
        }
        else
        {
            chkbit++;
        }
    }
    return chkbit;
}

/*!
 * @brief Read the error report counter for one channel.
 */
uint32_t EMU_DRV_GetErrReportCnt(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
    EMU_Type *const base = g_emuBase[instance];

    return EMU_GetErrReportCnt(base, channel);
}

/*!
 * @brief Clear the error report counter for one channel.
 */
void EMU_DRV_ClearErrReportCnt(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);
    EMU_Type *const base = g_emuBase[instance];
    EMU_ClearErrReportCnt(base, channel);
}

/*!
 * @brief Read the address latched for the reported ECC event on one channel.
 */
uint32_t EMU_DRV_GetErrReportAddr(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    EMU_Type *const base = g_emuBase[instance];
    return EMU_GetErrReportAddr(base, channel);
}

/*******************************************************************************
 * Interrupt Status & Control
 ******************************************************************************/

/*!
 * @brief Clear the single-bit correction interrupt flag for one channel.
 */
void EMU_DRV_ClearInterruptFlagSignalBit(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
    EMU_Type *const base = g_emuBase[instance];

    EMU_ClearInterruptFlagSignalBit(base, channel);
}

/*!
 * @brief Enable the double-bit interrupt path for one channel.
 */
void EMU_DRV_EnableInterruptDoubleBit(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
    EMU_Type *const base = g_emuBase[instance];

    EMU_EnableInterruptDoubleBit(base, channel);
}

/*!
 * @brief Disable the double-bit interrupt path for one channel.
 */
void EMU_DRV_DisableInterruptDoubleBit(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
    EMU_Type *const base = g_emuBase[instance];

    EMU_DisableInterruptDoubleBit(base, channel);
}

/*!
 * @brief Clear the double-bit interrupt flag for one channel.
 */
void EMU_DRV_ClearInterruptFlagDoubleBit(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
    EMU_Type *const base = g_emuBase[instance];

    EMU_ClearInterruptFlagDoubleBit(base, channel);
}

/*!
 * @brief Query the single-bit correction interrupt flag for one channel.
 */
uint8_t EMU_DRV_GetChannelSBInterruptFlag(uint32_t instance, uint8_t channel)
{
    DEV_ASSERT(instance < EMU_INSTANCE_COUNT);
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    EMU_Type *const base = g_emuBase[instance];
    return EMU_GetChannelSBInterruptFlag(base, channel);
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
