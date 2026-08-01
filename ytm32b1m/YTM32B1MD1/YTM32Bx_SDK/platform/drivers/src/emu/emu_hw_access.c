/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file emu_hw_access.c
 * @version 1.4.1
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 2985 Rule 2.2 : This operation is redundant. The value of the result is 
 *                        always that of the left-hand operand.
 *
 */

#include "emu_hw_access.h"

/*******************************************************************************
 * Initialization
 ******************************************************************************/

/*!
 * @brief Initialize the EMU peripheral registers to their reset values.
 */
void EMU_Init(EMU_Type *const base)
{
    uint8_t idx = 0U;
    base->EIGCR = 0x00000000U;
    base->EICHCR = 0x00000000U;
#if (EMU_EICHD_COUNT > 1U)
    for (idx = 0; idx < EMU_EICHD_COUNT; idx++)
#endif
    {
        base->EICHD[idx].CHKBIT = 0x00000000U;
#if defined(FEATURE_EMU_SUPPORT_64BIT_INJECTION) && (FEATURE_EMU_SUPPORT_64BIT_INJECTION == 1U)
        base->EICHD[idx].DATAL = 0x00000000U;
        base->EICHD[idx].DATAH = 0x00000000U;
#else
        base->EICHD[idx].DATA = 0x00000000U;
#endif /* FEATURE_EMU_SUPPORT_64BIT_INJECTION */
        base->EICHD[idx].ADDR = 0x00000000U;

        base->ERINFO[idx].CNT = 0x00000000U;
    }

    base->ERIE = 0x00000000U;
    base->ERIF = 0xFFFFFFFFU;
}

/*******************************************************************************
 * Module Enable Control
 ******************************************************************************/

/*!
 * @brief Disable one EMU channel and clear the global enable latch.
 */
void EMU_Disable(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    base->EICHCR &= (~((uint32_t)1U << channel));
    base->EIGCR = 0x00;
}

/*!
 * @brief Enable one EMU channel and program the required global enable value.
 */
void EMU_Enable(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    base->EICHCR |= ((uint32_t)1U << channel);
#if ((defined FEATURE_EMU_GLOBAL_ENABLE_KEY) && ( FEATURE_EMU_GLOBAL_ENABLE_KEY == 1))
    base->EIGCR = 0x89abcdefU;
#else
    base->EIGCR = 0x01U;
#endif
}

/*******************************************************************************
 * Injection Configuration
 ******************************************************************************/

/*!
 * @brief Program the SRAM word address used to trigger an injected ECC event.
 */
void EMU_SetErrInjectAddr(EMU_Type *const base, uint8_t channel, volatile uint32_t addr)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
    base->EICHD[channel].ADDR = addr;
}

/*!
 * @brief Program one check-bit index for single-bit injection.
 */
void EMU_SetErrInjectChkbit(EMU_Type *const base, uint8_t channel, emu_chkbit_type chkbit)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
    base->EICHD[channel].CHKBIT = (uint32_t)EMU_EICHD_CHKBIT_CHKBIT_MASK & ((uint32_t)1U << chkbit);
}

/*!
 * @brief Program a raw multi-bit check mask for one channel.
 */
void EMU_SetErrInjectChkMulti(EMU_Type *const base, uint8_t channel, emu_chkbit_type chkMulti)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    base->EICHD[channel].CHKBIT = (uint32_t)EMU_EICHD_CHKBIT_CHKBIT_MASK & chkMulti;
}

/*!
 * @brief Clear the programmed check-bit injection mask for one channel.
 */
void EMU_ClearChkbit(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    base->EICHD[channel].CHKBIT = 0x00;
}

/*!
 * @brief Program one data-bit index for single-bit injection.
 */
void EMU_SetErrInjectData(EMU_Type *const base, uint8_t channel, emu_databit_type databit)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

#if defined(FEATURE_EMU_SUPPORT_64BIT_INJECTION) && (FEATURE_EMU_SUPPORT_64BIT_INJECTION == 1U)
    if (databit < 32U)
    {
        base->EICHD[channel].DATAL = ((uint32_t)0x1U << databit);
    }
    else
    {
        base->EICHD[channel].DATAH = ((uint32_t)0x1U << (databit - 32U));
    }
#else
    base->EICHD[channel].DATA = ((uint32_t)0x1U << databit);
#endif /* FEATURE_EMU_SUPPORT_64BIT_INJECTION */
}

/*!
 * @brief Program a raw multi-bit data mask for one channel.
 */
void EMU_SetErrInjectDataMulti(EMU_Type *const base, uint8_t channel, emu_databit_type dataMulti)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

#if defined(FEATURE_EMU_SUPPORT_64BIT_INJECTION) && (FEATURE_EMU_SUPPORT_64BIT_INJECTION == 1U)
    base->EICHD[channel].DATAL = (uint32_t)(dataMulti);
    base->EICHD[channel].DATAH = (uint32_t)((dataMulti >> 32U));
#else
    base->EICHD[channel].DATA = (uint32_t)(dataMulti);
#endif /* FEATURE_EMU_SUPPORT_64BIT_INJECTION */
}

/*!
 * @brief Clear the programmed data-bit injection mask for one channel.
 */
void EMU_ClearData(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);
#if defined(FEATURE_EMU_SUPPORT_64BIT_INJECTION) && (FEATURE_EMU_SUPPORT_64BIT_INJECTION == 1U)
    base->EICHD[channel].DATAL = 0x00;
    base->EICHD[channel].DATAH = 0x00;
#else
    base->EICHD[channel].DATA = 0x00;
#endif /* FEATURE_EMU_SUPPORT_64BIT_INJECTION */
}

/*******************************************************************************
 * Error Reporting
 ******************************************************************************/

/*!
 * @brief Read the error report counter for one channel.
 */
uint32_t EMU_GetErrReportCnt(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);
    return base->ERINFO[channel].CNT;
}

/*!
 * @brief Clear the error report counter for one channel.
 */
void EMU_ClearErrReportCnt(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);
    base->ERINFO[channel].CNT = 0x00U & EMU_ERINFO_CNT_CNT_MASK;
}

/*!
 * @brief Read the latched report address for one channel.
 */
uint32_t EMU_GetErrReportAddr(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_EICHD_COUNT);

    return base->ERINFO[channel].ADDR;
}

/*!
 * @brief Read the raw syndrome field for one channel.
 */
uint32_t EMU_GetSyndrome(EMU_Type *const base, uint8_t channel)
{
    return (base->ERINFO[channel].SYND & EMU_ERINFO_SYND_SYNDROME_MASK) >> EMU_ERINFO_SYND_SYNDROME_SHIFT;
}

/*******************************************************************************
 * Interrupt Control
 ******************************************************************************/

/*!
 * @brief Enable single-bit correction interrupt reporting for one channel.
 */
void EMU_EnableInterruptSignalBit(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);

    base->ERIE |= EMU_ERIE_SCIE((uint32_t)1U << channel); /*PRQA S 2985*/
}

/*!
 * @brief Disable single-bit correction interrupt reporting for one channel.
 */
void EMU_DisableInterruptSignalBit(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);

    base->ERIE &= ~EMU_ERIE_SCIE((uint32_t)1U << channel); /*PRQA S 2985*/
}

/*!
 * @brief Clear the single-bit correction interrupt flag for one channel.
 */
void EMU_ClearInterruptFlagSignalBit(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);

    base->ERIF = EMU_ERIF_SCIF((uint32_t)1U << channel); /*PRQA S 2985*/
}

/*!
 * @brief Enable double-bit interrupt reporting for one channel.
 */
void EMU_EnableInterruptDoubleBit(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);

    base->ERIE |= EMU_ERIE_NCIE((uint32_t)1U << channel);
}

/*!
 * @brief Disable double-bit interrupt reporting for one channel.
 */
void EMU_DisableInterruptDoubleBit(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);

    base->ERIE &= ~EMU_ERIE_NCIE((uint32_t)1U << channel);
}

/*!
 * @brief Clear the double-bit interrupt flag for one channel.
 */
void EMU_ClearInterruptFlagDoubleBit(EMU_Type *const base, uint8_t channel)
{
    DEV_ASSERT(channel < EMU_ERINFO_COUNT);

    base->ERIF = EMU_ERIF_NCIF((uint32_t)1U << channel);
}

/*!
 * @brief Query the single-bit correction interrupt flag field for one channel.
 */
uint8_t EMU_GetChannelSBInterruptFlag(EMU_Type *const base, uint8_t channel)
{
    return (uint8_t)((((base->ERIF & EMU_ERIF_SCIF_MASK) >> EMU_ERIF_SCIF_SHIFT) >> channel));
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
