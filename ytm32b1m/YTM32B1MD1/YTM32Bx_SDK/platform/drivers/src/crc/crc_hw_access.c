/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file crc_hw_access.c
 * @version 1.4.1
 *
 * @brief CRC Hardware Access Layer — non-inline function implementations.
 *
 * This file contains functions that are too complex or have conditional
 * compilation paths that prevent them from being declared as static inline
 * in the header.
 */

#include "crc_hw_access.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Default initial seed value used by CRC_Init(). */
#define CRC_INITIAL_SEED        (0U)

/*******************************************************************************
 * Initialization
 ******************************************************************************/

/*!
 * @brief Initialize the CRC peripheral to its default configuration.
 *
 * Resets all CRC control fields to a known state:
 *   - CRC width:           32-bit (CRC_BITS_32)
 *   - Write transpose:     none (CRC_TRANSPOSE_NONE)
 *   - Read transpose:      none (CRC_TRANSPOSE_NONE)
 *   - Output XOR (INV):    disabled
 *   - Seed (INIT):         0x00000000
 *
 * @param[in] base  Pointer to the CRC peripheral base address (CRC_Type).
 *
 * @pre  The CRC peripheral clock must be enabled before calling this function.
 */
void CRC_Init(CRC_Type *const base)
{
    /* Set CRC mode to 32-bit */
    CRC_SetProtocolWidth(base, CRC_BITS_32);

    /* Set read/write transpose and complement checksum to none */
    CRC_SetWriteTranspose(base, CRC_TRANSPOSE_NONE);
    CRC_SetReadTranspose(base, CRC_TRANSPOSE_NONE);
    CRC_SetFXorMode(base, false);

    /* Write seed to zero */
    CRC_SetSeedReg(base, CRC_INITIAL_SEED);
}

/*******************************************************************************
 * Result & Seed Access
 ******************************************************************************/

/*!
 * @brief Get the CRC computation result, masked to the configured protocol width.
 *
 * Returns the current CRC result from the RESULT register, truncated to
 * the active protocol width:
 *   - CRC_BITS_16: returns lower 16 bits (cast to uint16_t).
 *   - CRC_BITS_32: returns full 32 bits.
 *   - CRC_BITS_8 (if supported): returns lower 8 bits.
 *   - CRC_BITS_4 (if supported): returns lower 8 bits (cast to uint8_t).
 *
 * @param[in] base  Pointer to the CRC peripheral base address.
 * @return  CRC result value, width-adjusted.
 *
 * @note If output complement (INV_OUT) is enabled, the returned value is
 *       the complemented checksum. See CRC_SetFXorMode().
 * @note FEATURE_CRC_SUPPORT_CRC4 controls whether 4-bit mode is available.
 */
uint32_t CRC_GetCrcResult(const CRC_Type *const base)
{
    crc_bit_width_t width = CRC_GetProtocolWidth(base);
    uint32_t result;

    if (width == CRC_BITS_16)
    {
        result = (uint16_t)CRC_GetResultReg(base);
    }
#if (FEATURE_CRC_SUPPORT_CRC8)
    else if (width == CRC_BITS_8)
    {
        result = (uint8_t)CRC_GetResultReg(base);
    }
#endif /* FEATURE_CRC_SUPPORT_CRC8 */
#if (FEATURE_CRC_SUPPORT_CRC4)
    else if (width == CRC_BITS_4)
    {
        result = (uint8_t)CRC_GetResultReg(base);
    }
#endif /* FEATURE_CRC_SUPPORT_CRC4 */
    else
    {
        /* width 32 */
        result = CRC_GetResultReg(base);
    }

    return result;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
