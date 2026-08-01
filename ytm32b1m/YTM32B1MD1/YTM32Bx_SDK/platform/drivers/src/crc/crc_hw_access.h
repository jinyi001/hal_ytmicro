/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file crc_hw_access.h
 * @version 1.4.1
 *
 * @brief CRC Hardware Access Layer.
 *
 * This header provides low-level inline accessor functions and initialization
 * routines for the CRC peripheral registers. Functions are organized into the
 * following categories:
 *   - Initialization
 *   - Data Input (8/16/32-bit writes)
 *   - Result & Seed Access
 *   - Protocol Configuration (bit width, output XOR)
 *   - Transpose Configuration (input/output bit-order swap)
 *
 * @note This is an internal layer used by the CRC Driver (crc_driver.h/c).
 *       Application code should use the CRC_DRV_* APIs from crc_driver.h.
 */

#ifndef CRC_HW_ACCESS_H
#define CRC_HW_ACCESS_H

#include "crc_driver.h"

/*!
 * @addtogroup crc_hw_access CRC Hardware Access
 * @ingroup crc
 * @brief Low-level register access functions for the CRC peripheral.
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Initialization
 ******************************************************************************/
/*!
 * @name Initialization
 * @brief Functions for initializing the CRC hardware to a known default state.
 * @{
 */

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
void CRC_Init(CRC_Type *const base);

/*! @} */ /* End of Initialization */

/*******************************************************************************
 * Data Input
 ******************************************************************************/
/*!
 * @name Data Input
 * @brief Functions for feeding data into the CRC computation engine.
 *
 * The CRC hardware accepts 8-bit, 16-bit, or 32-bit data writes. Each write
 * is immediately consumed by the CRC engine and contributes to the running
 * checksum. The data width must match the register access size; use the
 * appropriately-sized function to avoid padding issues.
 * @{
 */

/*!
 * @brief Write a 32-bit data word into the CRC data register.
 *
 * Writes a full 32-bit value to the CRC DATA.IN32 register. The CRC engine
 * immediately processes all four bytes.
 *
 * @param[in] base   Pointer to the CRC peripheral base address.
 * @param[in] value  32-bit data to feed into the CRC computation.
 *
 * @note The byte order of the input data is affected by the write transpose
 *       setting. See CRC_SetWriteTranspose().
 */
static inline void CRC_SetDataReg(CRC_Type *const base,
                                  uint32_t value)
{
    base->DATA.IN32 = value;
}

/*!
 * @brief Write a 16-bit data half-word into the CRC data register.
 *
 * Writes a 16-bit value to the CRC DATA.IN16 register. The CRC engine
 * immediately processes two bytes.
 *
 * @param[in] base   Pointer to the CRC peripheral base address.
 * @param[in] value  16-bit data to feed into the CRC computation.
 *
 * @note The byte order of the input data is affected by the write transpose
 *       setting. See CRC_SetWriteTranspose().
 */
static inline void CRC_SetDataReg16(CRC_Type *const base,
                                    uint16_t value)
{
    base->DATA.IN16 = value;
}

/*!
 * @brief Write an 8-bit data byte into the CRC data register.
 *
 * Writes an 8-bit value to the CRC DATA.IN8 register. The CRC engine
 * immediately processes one byte.
 *
 * @param[in] base   Pointer to the CRC peripheral base address.
 * @param[in] value  8-bit data to feed into the CRC computation.
 *
 * @note The bit order within the byte is affected by the write transpose
 *       setting. See CRC_SetWriteTranspose().
 */
static inline void CRC_SetDataReg8(CRC_Type *const base,
                                   uint8_t value)
{
    base->DATA.IN8 = value;
}

/*! @} */ /* End of Data Input */

/*******************************************************************************
 * Result & Seed Access
 ******************************************************************************/
/*!
 * @name Result & Seed Access
 * @brief Functions for reading the CRC computation result and managing the
 *        seed (initial checksum) value.
 *
 * - **Result**: The final or intermediate CRC value after data has been fed.
 *   Use CRC_GetCrcResult() for width-aware result retrieval, or
 *   CRC_GetResultReg() for raw 32-bit register read.
 * - **Seed**: The initial value loaded into the CRC engine before computation
 *   begins. Setting a new seed effectively starts a new CRC calculation.
 * @{
 */

/*!
 * @brief Get the CRC computation result, masked to the configured protocol width.
 *
 * Returns the current CRC result from the RESULT register, truncated to
 * the active protocol width:
 *   - CRC_BITS_16: returns lower 16 bits.
 *   - CRC_BITS_32: returns full 32 bits.
 *   - CRC_BITS_8 (if supported): returns lower 8 bits.
 *   - CRC_BITS_4 (if supported): returns lower 8 bits.
 *
 * @param[in] base  Pointer to the CRC peripheral base address.
 * @return  CRC result value, width-adjusted.
 *
 * @note FEATURE_CRC_SUPPORT_CRC4 controls whether 4-bit mode is available.
 */
uint32_t CRC_GetCrcResult(const CRC_Type *const base);

/*!
 * @brief Read the raw 32-bit value of the CRC RESULT register.
 *
 * Returns the full, unmodified content of the RESULT register without any
 * width masking. Useful for direct register inspection or when the caller
 * handles width masking externally.
 *
 * @param[in] base  Pointer to the CRC peripheral base address.
 * @return  Raw 32-bit value from the RESULT register.
 */
static inline uint32_t CRC_GetResultReg(const CRC_Type *const base)
{
    return base->RESULT;
}

/*!
 * @brief Read the current seed value from the CRC INIT register.
 *
 * Returns the value previously written to the INIT register with
 * CRC_SetSeedReg(). This is the starting checksum that was used (or will
 * be used) at the beginning of a CRC computation.
 *
 * @param[in] base  Pointer to the CRC peripheral base address.
 * @return  Current seed (initial checksum) value.
 */
static inline uint32_t CRC_GetSeedReg(const CRC_Type *const base)
{
    return base->INIT;
}

/*!
 * @brief Write a new seed value to the CRC INIT register.
 *
 * Sets the initial checksum (seed) that the CRC engine will use for the
 * next computation. Writing a new seed effectively resets the CRC state
 * and starts a fresh calculation from this seed value.
 *
 * @param[in] base   Pointer to the CRC peripheral base address.
 * @param[in] value  New seed value to load into the INIT register.
 *
 * @note Common seed values:
 *       - 0xFFFF for CRC-16-CCITT
 *       - 0xFFFFFFFF for CRC-32
 *       - 0x00000000 for some custom protocols
 */
static inline void CRC_SetSeedReg(CRC_Type *const base,
                                  uint32_t value)
{
    base->INIT = value;
}

/*! @} */ /* End of Result & Seed Access */

/*******************************************************************************
 * Protocol Configuration
 ******************************************************************************/
/*!
 * @name Protocol Configuration
 * @brief Functions for configuring the CRC protocol parameters: bit width and
 *        output inversion (final XOR).
 *
 * - **Bit Width**: Selects whether the CRC engine computes a 16-bit or 32-bit
 *   checksum (4-bit and 8-bit on supported devices).
 * - **Final XOR (INV_OUT)**: Some CRC protocols (e.g., CRC-32/ISO) require
 *   the final checksum to be complemented (XORed with 0xFFFF or 0xFFFFFFFF).
 *   This feature performs the complement automatically on read.
 * @{
 */

/*!
 * @brief Get the currently configured CRC protocol bit width.
 *
 * Reads the MODE field of the CRC CTRL register to determine the active
 * CRC computation width.
 *
 * @param[in] base  Pointer to the CRC peripheral base address.
 * @return  CRC bit width enumeration:
 *          - CRC_BITS_16: 16-bit CRC protocol.
 *          - CRC_BITS_32: 32-bit CRC protocol.
 *          - CRC_BITS_4:  4-bit CRC (only if FEATURE_CRC_SUPPORT_CRC4).
 *          - CRC_BITS_8:  8-bit CRC (only if FEATURE_CRC_SUPPORT_CRC8).
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
 * @brief Set the CRC protocol bit width.
 *
 * Configures the MODE field of the CRC CTRL register. This determines
 * whether the CRC engine performs a 16-bit or 32-bit computation.
 *
 * @param[in] base   Pointer to the CRC peripheral base address.
 * @param[in] width  Desired CRC bit width:
 *                   - CRC_BITS_16: 16-bit CRC protocol.
 *                   - CRC_BITS_32: 32-bit CRC protocol.
 *                   - CRC_BITS_4:  4-bit (only if FEATURE_CRC_SUPPORT_CRC4).
 *                   - CRC_BITS_8:  8-bit (only if FEATURE_CRC_SUPPORT_CRC8).
 *
 * @note Changing the width mid-computation invalidates the running checksum.
 *       Always set the width before writing the seed and data.
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
 * @brief Get the output complement (final XOR) mode.
 *
 * Reads the INV_OUT bit of the CRC CTRL register. When enabled, the CRC
 * result is automatically XORed with 0xFFFFFFFF (32-bit) or 0xFFFF (16-bit)
 * when read, as required by certain CRC standards.
 *
 * @param[in] base  Pointer to the CRC peripheral base address.
 * @return  Complement mode state:
 *          - true:  Output result is inverted (complemented) on read.
 *          - false: Output result is returned as-is (no XOR).
 */
static inline bool CRC_GetFXorMode(const CRC_Type *const base)
{
    return ((base->CTRL & CRC_CTRL_INV_OUT_MASK) >> CRC_CTRL_INV_OUT_SHIFT) != 0U;
}

/*!
 * @brief Set the output complement (final XOR) mode.
 *
 * Configures the INV_OUT bit of the CRC CTRL register. Some CRC protocols
 * (e.g., CRC-32/ISO 3309, ITU-T V.42) require the final checksum to be
 * XORed with all-ones. This setting enables that transformation automatically.
 *
 * @param[in] base    Pointer to the CRC peripheral base address.
 * @param[in] enable  true = complement (invert) the checksum on read;
 *                    false = no complement.
 *
 * @note For CRC-32 ITU-T V.42 standard, this bit must be enabled together
 *       with SWAP_BYTEWISE (read transpose) and SWAP_IN (write transpose).
 */
static inline void CRC_SetFXorMode(CRC_Type *const base,
                                   bool enable)
{
    uint32_t ctrlTemp = base->CTRL;

    ctrlTemp &= ~(CRC_CTRL_INV_OUT_MASK);
    ctrlTemp |= CRC_CTRL_INV_OUT(enable ? 1UL : 0UL);
    base->CTRL = ctrlTemp;
}

/*! @} */ /* End of Protocol Configuration */

/*******************************************************************************
 * Transpose Configuration
 ******************************************************************************/
/*!
 * @name Transpose Configuration
 * @brief Functions for configuring bit-order transposition on input data
 *        (write path) and output result (read path).
 *
 * Transposition (also called "reflection" or "bit-reversal") controls whether
 * the bit order within each byte is reversed before CRC computation (write
 * transpose) or after result generation (read transpose).
 *
 * Many standard CRC algorithms (e.g., CRC-16-CCITT with 0x1021 polynomial)
 * require specific transpose settings to produce correct results.
 *
 * @note For CRC-16-CCITT (0x1021), set SWAP_BITWISE on the write path.
 * @note For CRC-32 ITU-T V.42, set both SWAP_BYTEWISE (read) and SWAP_IN
 *       (write) together with INV_OUT.
 * @{
 */

/*!
 * @brief Get the write-path (input) transpose mode.
 *
 * Reads the SWAP_IN bit of the CRC CTRL register to determine whether
 * bit-order transposition is applied to input data before CRC computation.
 *
 * @param[in] base  Pointer to the CRC peripheral base address.
 * @return  Write transpose setting:
 *          - CRC_TRANSPOSE_NONE: No transposition; data is fed as-is.
 *          - CRC_TRANSPOSE_BITS: Bits within each byte are reversed.
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
 * @brief Set the write-path (input) transpose mode.
 *
 * Configures the SWAP_IN field of the CRC CTRL register. When enabled,
 * the bit order within each input byte is reversed before the data enters
 * the CRC computation engine.
 *
 * @param[in] base    Pointer to the CRC peripheral base address.
 * @param[in] transp  Desired write transpose mode:
 *                    - CRC_TRANSPOSE_NONE: No transposition.
 *                    - CRC_TRANSPOSE_BITS: Reverse bits within each byte.
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
 * @brief Get the read-path (output) transpose mode.
 *
 * Reads the SWAP_OUT bit of the CRC CTRL register to determine whether
 * bit-order transposition is applied to the CRC result on readback.
 *
 * @param[in] base  Pointer to the CRC peripheral base address.
 * @return  Read transpose setting:
 *          - CRC_TRANSPOSE_NONE: No transposition; result is returned as-is.
 *          - CRC_TRANSPOSE_BITS: Bits within each byte of the result are reversed.
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
 * @brief Set the read-path (output) transpose mode.
 *
 * Configures the SWAP_OUT field of the CRC CTRL register. When enabled,
 * the bit order within each byte of the CRC result is reversed on readback.
 *
 * @param[in] base    Pointer to the CRC peripheral base address.
 * @param[in] transp  Desired read transpose mode:
 *                    - CRC_TRANSPOSE_NONE: No transposition.
 *                    - CRC_TRANSPOSE_BITS: Reverse bits within each byte.
 */
static inline void CRC_SetReadTranspose(CRC_Type *const base,
                                        crc_transpose_t transp)
{
    uint32_t ctrlTemp = base->CTRL;

    ctrlTemp &= ~(CRC_CTRL_SWAP_OUT_MASK);
    ctrlTemp |= CRC_CTRL_SWAP_OUT(transp);
    base->CTRL = ctrlTemp;
}

/*! @} */ /* End of Transpose Configuration */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of crc_hw_access group */

#endif /* CRC_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
