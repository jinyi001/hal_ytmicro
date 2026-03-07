/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file crc_hw_access.h
 * @version 1.4.0
 */

#ifndef CRC_HW_ACCESS_H
#define CRC_HW_ACCESS_H

#include "crc_driver.h"

/*!

 * @brief Cyclic Redundancy Check Hardware Access.
 *
 * This section describes the programming interface of the CRC HW ACCESS.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @name CRC HW ACCESS API
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initializes the CRC module
 *
 * This function initializes the module to default configuration
 * (Initial checksum: 0U,
 * Default polynomial: 0x1021U,
 * Type of read transpose: CRC_TRANSPOSE_NONE,
 * Type of write transpose: CRC_TRANSPOSE_NONE,
 * No complement of checksum read,
 * 32-bit CRC)
 *
 * @param[in] base The CRC peripheral base address
 */
void CRC_Init(CRC_Type *const base);

/*!
 * @brief Returns the current result of the CRC calculation
 *
 * This function returns the current result of the CRC calculation
 *
 * @param[in] base The CRC peripheral base address
 * @return Result of CRC calculation
 */
uint32_t CRC_GetCrcResult(const CRC_Type *const base);


/*!
 * @brief Gets the current CRC result
 *
 * This function gets the current CRC result from the data register
 *
 * @param[in] base The CRC peripheral base address
 * @return Returns the current CRC result
 */
static inline uint32_t CRC_GetResultReg(const CRC_Type *const base)
{
    return base->RESULT;
}

/*!
 * @brief Gets the current CRC seed
 *
 * This function gets the current CRC seed from the init register
 *
 * @param[in] base The CRC peripheral base address
 * @return Returns the current CRC seed
 */
static inline uint32_t CRC_GetSeedReg(const CRC_Type *const base)
{
    return base->INIT;
}

/*!
 * @brief Sets the 32 bits of CRC data register
 *
 * This function sets the 32 bits of CRC data register
 *
 * @param[in] base The CRC peripheral base address
 * @param[in] value New data for CRC computation
 */
static inline void CRC_SetDataReg(CRC_Type *const base,
                                  uint32_t value)
{
    base->DATA.IN32 = value;
}

/*!
 * @brief Sets the 16 bits of CRC data register
 *
 * This function sets the 16 bits of CRC data register
 *
 * @param[in] base The CRC peripheral base address
 * @param[in] value New data for CRC computation
 */
static inline void CRC_SetDataReg16(CRC_Type *const base,
                                    uint16_t value)
{
    base->DATA.IN16 = value;
}

/*!
 * @brief Sets the 8 bits of CRC data register
 *
 * This function sets the 8 bits of CRC data register
 *
 * @param[in] base The CRC peripheral base address
 * @param[in] value New data for CRC computation
 */
static inline void CRC_SetDataReg8(CRC_Type *const base,
                                   uint8_t value)
{
    base->DATA.IN8 = value;
}

/*!
 * @brief Sets the init seed register
 *
 * This function sets the polynomial register
 *
 * @param[in] base The CRC peripheral base address
 * @param[in] value Polynomial value
 */
static inline void CRC_SetSeedReg(CRC_Type *const base,
                                  uint32_t value)
{
    base->INIT = value;
}


/*!
 * @brief Gets complement read of CRC data register
 *
 * This function gets complement read of CRC data register.
 * Some CRC protocols require the final checksum to be XORed with 0xFFFFFFFF
 * or 0xFFFF. Complement mode enables "on the fly" complementing of read data
 *
 * @param[in] base The CRC peripheral base address
 * @return Complement read
 *         -true: Invert or complement the read value of the CRC Data register.
 *         -false: No XOR on reading.
 */
static inline bool CRC_GetFXorMode(const CRC_Type *const base)
{
    return ((base->CTRL & CRC_CTRL_INV_OUT_MASK) >> CRC_CTRL_INV_OUT_SHIFT) != 0U;
}

/*!
 * @brief Sets complement read of CRC data register
 *
 * This function sets complement read of CRC data register.
 * Some CRC protocols require the final checksum to be XORed with 0xFFFFFFFF
 * or 0xFFFF. Complement mode enables "on the fly" complementing of read data
 *
 * @param[in] base The CRC peripheral base address
 * @param[in] enable Enable or disable complementing of read data
 */
static inline void CRC_SetFXorMode(CRC_Type *const base,
                                   bool enable)
{
    uint32_t ctrlTemp = base->CTRL;

    ctrlTemp &= ~(CRC_CTRL_INV_OUT_MASK);
    ctrlTemp |= CRC_CTRL_INV_OUT(enable ? 1UL : 0UL);
    base->CTRL = ctrlTemp;
}

/*!
 * @brief Gets the CRC protocol width
 *
 * This function gets the CRC protocol width
 *
 * @param[in] base The CRC peripheral base address
 * @return CRC protocol width
 *         - CRC_BITS_16: 16-bit CRC protocol.
 *         - CRC_BITS_32: 32-bit CRC protocol.
 */
static inline crc_bit_width_t CRC_GetProtocolWidth(const CRC_Type *const base)
{
    crc_bit_width_t retVal = CRC_BITS_16;
    uint8_t mode = 0;
    mode = (uint8_t)((base->CTRL & CRC_CTRL_MODE_MASK) >> CRC_CTRL_MODE_SHIFT);
    retVal = (crc_bit_width_t)mode;

    return retVal;
}

/*!
 * @brief Sets the CRC protocol width
 *
 * This function sets the CRC protocol width
 *
 * @param[in] base The CRC peripheral base address
 * @param[in] width The CRC protocol width
 *            - CRC_BITS_16: 16-bit CRC protocol.
 *            - CRC_BITS_32: 32-bit CRC protocol.
 */
static inline void CRC_SetProtocolWidth(CRC_Type *const base,
                                        crc_bit_width_t width)
{
    uint32_t ctrlTemp = base->CTRL;

    ctrlTemp &= ~(CRC_CTRL_MODE_MASK);
    ctrlTemp |= CRC_CTRL_MODE(width);
    base->CTRL = ctrlTemp;
}

/*!
 * @brief Gets the CRC transpose type for writes
 *
 * This function gets the CRC transpose type for writes
 *
 * @param[in] base The CRC peripheral base address
 * @return CRC input transpose type for writes
 */
static inline crc_transpose_t CRC_GetWriteTranspose(const CRC_Type *const base)
{
    crc_transpose_t type;
    if ((base->CTRL & CRC_CTRL_SWAP_IN_MASK) != 0u)
    {
        type = CRC_TRANSPOSE_BITS;
    } else
    {
        type = CRC_TRANSPOSE_NONE;
    }

    return type;
}

/*!
 * @brief Sets the CRC transpose type for writes
 *
 * This function sets the CRC transpose type for writes
 *
 * @param[in] base The CRC peripheral base address
 * @param[in] transp The CRC input transpose type
 */
static inline void CRC_SetWriteTranspose(CRC_Type *const base,
                                         crc_transpose_t transp)
{
    uint32_t ctrlTemp = base->CTRL;

    ctrlTemp &= ~(CRC_CTRL_SWAP_IN_MASK);
    ctrlTemp |= CRC_CTRL_SWAP_IN(transp);
    base->CTRL = ctrlTemp;
}

/*!
 * @brief Gets the CRC transpose type for reads
 *
 * This function gets the CRC transpose type for reads
 *
 * @param[in] base The CRC peripheral base address
 * @return CRC output transpose type
 */
static inline crc_transpose_t CRC_GetReadTranspose(const CRC_Type *const base)
{
    crc_transpose_t type;
    if ((base->CTRL & CRC_CTRL_SWAP_OUT_MASK) != 0u)
    {
        type = CRC_TRANSPOSE_BITS;
    } else
    {
        type = CRC_TRANSPOSE_NONE;
    }

    return type;
}

/*!
 * @brief Sets the CRC transpose type for reads
 *
 * This function sets the CRC transpose type for reads
 *
 * @param[in] base The CRC peripheral base address
 * @param[in] transp The CRC output transpose type
 */
static inline void CRC_SetReadTranspose(CRC_Type *const base,
                                        crc_transpose_t transp)
{
    uint32_t ctrlTemp = base->CTRL;

    ctrlTemp &= ~(CRC_CTRL_SWAP_OUT_MASK);
    ctrlTemp |= CRC_CTRL_SWAP_OUT(transp);
    base->CTRL = ctrlTemp;
}

/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* CRC_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
