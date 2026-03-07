/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file crc_hw_access.c
 * @version 1.4.0
 */

#include "crc_hw_access.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Initial checksum */
#define CRC_INITIAL_SEED        (0U)

/*******************************************************************************
 * Code
 ******************************************************************************/
/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_Init
 * Description   : This function initializes the module to default configuration
 * (Initial checksum: 0U,
 * Default polynomial: 0x1021U,
 * Type of read transpose: CRC_TRANSPOSE_NONE,
 * Type of write transpose: CRC_TRANSPOSE_NONE,
 * No complement of checksum read,
 * 32-bit CRC).
 *
 *END**************************************************************************/
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


/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_GetCrcResult
 * Description   : This function returns the current result of the CRC calculation.
 *
 *END**************************************************************************/
uint32_t CRC_GetCrcResult(const CRC_Type *const base)
{
    crc_bit_width_t width = CRC_GetProtocolWidth(base);
    uint32_t result;

    if (width == CRC_BITS_16)
    {
        result = (uint16_t)CRC_GetResultReg(base);
    }
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
