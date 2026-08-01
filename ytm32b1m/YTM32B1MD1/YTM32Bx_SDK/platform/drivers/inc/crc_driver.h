/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file crc_driver.h
 * @version 1.4.1
 *
 * @brief CRC Driver — Public API for Cyclic Redundancy Check computation.
 *
 * This header defines the application-level interface for the CRC peripheral.
 * It wraps the low-level hardware access layer and provides a convenient API
 * for configuring the CRC engine and computing checksums over data buffers.
 *
 * The APIs are organized into three categories:
 *   - **Initialization & De-initialization**: Set up or tear down the CRC instance.
 *   - **Configuration**: Apply or retrieve CRC protocol parameters at runtime.
 *   - **CRC Calculation**: Feed data and retrieve results.
 *
 * @note The CRC peripheral clock must be enabled (via clock_manager) before
 *       calling any function in this module.
 */

#ifndef CRC_DRIVER_H
#define CRC_DRIVER_H

#include <stddef.h>
#include "device_registers.h"
#include "status.h"

/*!
 * @addtogroup crc
 * @brief Cyclic Redundancy Check Peripheral Driver — Public API.
 * @details Provides instance-based APIs for CRC configuration and checksum
 *          computation. Supports CRC-16 and CRC-32 protocols (CRC-4 and CRC-8
 *          on supported devices). Multiple data-feed widths (8/16/32-bit) and
 *          streaming (multi-block) CRC computation are supported.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief CRC transpose (bit-order reflection) mode.
 *
 * Controls whether the bit order within each byte is reversed on the
 * input (write) or output (read) data path.
 *
 * | Value                | Effect                              |
 * |----------------------|-------------------------------------|
 * | CRC_TRANSPOSE_NONE   | No transposition — data as-is.      |
 * | CRC_TRANSPOSE_BITS   | Bits within each byte are reversed. |
 *
 * Implements : crc_transpose_t_Class
 */
typedef enum
{
    CRC_TRANSPOSE_NONE = 0x00U,    /*!< No transposition. */
    CRC_TRANSPOSE_BITS = 0x01U     /*!< Transpose (reverse) bits within each byte. */
} crc_transpose_t;

/*!
 * @brief CRC computation bit width.
 *
 * Selects the polynomial width used by the CRC engine. The available widths
 * depend on the device feature flags:
 *
 * | Value       | Width   | Availability                    |
 * |-------------|---------|----------------------------------|
 * | CRC_BITS_4  | 4-bit   | Only if FEATURE_CRC_SUPPORT_CRC4 |
 * | CRC_BITS_8  | 8-bit   | Only if FEATURE_CRC_SUPPORT_CRC8 |
 * | CRC_BITS_16 | 16-bit  | Always available                 |
 * | CRC_BITS_32 | 32-bit  | Always available                 |
 *
 * Implements : crc_bit_width_t_Class
 */
#if (FEATURE_CRC_SUPPORT_CRC4)
typedef enum
{
    CRC_BITS_4  = 0U,   /*!< Generate  4-bit CRC code. */
    CRC_BITS_16 = 1U,   /*!< Generate 16-bit CRC code. */
    CRC_BITS_32 = 2U,   /*!< Generate 32-bit CRC code. */
#if FEATURE_CRC_SUPPORT_CRC8
    CRC_BITS_8 =  3U    /*!< Generate 8-bit CRC code. */
#endif /* FEATURE_CRC_SUPPORT_CRC8 */
} crc_bit_width_t;
#else
typedef enum
{
    CRC_BITS_16 = 0U,   /*!< Generate 16-bit CRC code. */
    CRC_BITS_32 = 1U    /*!< Generate 32-bit CRC code. */
} crc_bit_width_t;
#endif /* FEATURE_CRC_SUPPORT_CRC4 */

/*!
 * @brief CRC user configuration structure.
 *
 * Holds all parameters needed to configure a CRC computation session.
 * Pass this structure to CRC_DRV_Init() or CRC_DRV_Configure() to apply
 * settings, or to CRC_DRV_GetConfig() to retrieve the current state.
 *
 * | Field              | Type             | Description                                      |
 * |--------------------|------------------|-------------------------------------------------|
 * | crcWidth           | crc_bit_width_t  | CRC polynomial width (16 or 32 bits).            |
 * | readTranspose      | crc_transpose_t  | Bit-order transpose on CRC result readback.      |
 * | writeTranspose     | crc_transpose_t  | Bit-order transpose on input data path.          |
 * | complementChecksum | bool             | If true, result is XORed with all-ones on read.  |
 * | seed               | uint32_t         | Initial checksum value (starting seed).          |
 *
 * Implements : crc_user_config_t_Class
 */
typedef struct
{
    crc_bit_width_t crcWidth;               /*!< CRC polynomial width: CRC_BITS_16 or CRC_BITS_32
                                                 (CRC_BITS_4 / CRC_BITS_8 on supported devices). */
    crc_transpose_t readTranspose;          /*!< Transpose mode applied to the CRC result on read.
                                                 CRC_TRANSPOSE_NONE or CRC_TRANSPOSE_BITS. */
    crc_transpose_t writeTranspose;         /*!< Transpose mode applied to input data on write.
                                                 CRC_TRANSPOSE_NONE or CRC_TRANSPOSE_BITS. */
    bool complementChecksum;                /*!< If true, the CRC result is complemented (XORed with
                                                 0xFFFF / 0xFFFFFFFF) when read. Required by some
                                                 protocols such as CRC-32 ITU-T V.42. */
    uint32_t seed;                          /*!< Starting checksum value (seed). Common values:
                                                 0xFFFF for CRC-16-CCITT, 0xFFFFFFFF for CRC-32. */
} crc_user_config_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/
/*!
 * @name Initialization & De-initialization
 * @brief Functions for setting up and tearing down a CRC driver instance.
 * @{
 */

/*!
 * @brief Initialize the CRC driver with user-provided configuration.
 *
 * Resets the CRC peripheral to its default state, then applies the settings
 * from @a userConfigPtr (width, transpose, XOR mode, seed).
 *
 * @param[in] instance       CRC instance index (0-based). Must be less than
 *                           CRC_INSTANCE_COUNT.
 * @param[in] userConfigPtr  Pointer to the configuration structure. Must not
 *                           be NULL.
 * @return Execution status.
 * @retval STATUS_SUCCESS    Initialization completed successfully.
 *
 * @pre  The CRC peripheral clock must be enabled via the clock_manager.
 * @post The CRC engine is configured and ready for data input.
 *
 * @note Use CRC_DRV_GetDefaultConfig() to populate a configuration structure
 *       with default values before calling this function.
 */
status_t CRC_DRV_Init(uint32_t instance,
                      const crc_user_config_t *userConfigPtr);

/*!
 * @brief De-initialize the CRC driver, resetting to default configuration.
 *
 * Restores the CRC peripheral to its power-on default state (32-bit width,
 * no transpose, no complement, seed = 0). After calling this function the
 * instance may be re-initialized with CRC_DRV_Init().
 *
 * @param[in] instance  CRC instance index (0-based).
 * @return Execution status.
 * @retval STATUS_SUCCESS  De-initialization completed successfully.
 *
 * @note This function does NOT disable the CRC peripheral clock. The caller
 *       is responsible for clock management.
 */
status_t CRC_DRV_Deinit(uint32_t instance);

/*!
 * @brief Populate a configuration structure with default CRC parameters.
 *
 * Fills @a userConfigPtr with the device-specific default values defined
 * by the FEATURE_CRC_* macros:
 *   - crcWidth:           FEATURE_CRC_DEFAULT_WIDTH (typically CRC_BITS_16)
 *   - readTranspose:      FEATURE_CRC_DEFAULT_READ_TRANSPOSE (typically NONE)
 *   - writeTranspose:     FEATURE_CRC_DEFAULT_WRITE_TRANSPOSE (typically NONE)
 *   - complementChecksum: false
 *   - seed:               FEATURE_CRC_DEFAULT_SEED (typically 0xFFFF)
 *
 * @param[out] userConfigPtr  Pointer to the configuration structure to fill.
 *                            Must not be NULL.
 * @return Execution status.
 * @retval STATUS_SUCCESS     Default configuration written successfully.
 *
 * @note The default configuration typically corresponds to CRC-16-CCITT
 *       (polynomial 0x1021) with no transposition. Adjust fields as needed
 *       for other CRC standards.
 */
status_t CRC_DRV_GetDefaultConfig(crc_user_config_t *userConfigPtr);

/*! @} */ /* End of Initialization & De-initialization */

/*******************************************************************************
 * Configuration
 ******************************************************************************/
/*!
 * @name Configuration
 * @brief Functions for applying or querying CRC runtime configuration.
 * @{
 */

/*!
 * @brief Apply a user configuration to the CRC peripheral.
 *
 * Writes all fields of @a userConfigPtr to the CRC hardware registers.
 * This can be used to reconfigure the CRC engine at runtime without a
 * full init/deinit cycle (e.g., switching between CRC-16 and CRC-32).
 *
 * @param[in] instance       CRC instance index (0-based).
 * @param[in] userConfigPtr  Pointer to the configuration structure. Must not
 *                           be NULL.
 * @return Execution status.
 * @retval STATUS_SUCCESS    Configuration applied successfully.
 *
 * @pre  CRC_DRV_Init() must have been called for this instance.
 * @note Calling this function resets the running checksum because a new seed
 *       is written.
 */
status_t CRC_DRV_Configure(uint32_t instance,
                           const crc_user_config_t *userConfigPtr);

/*!
 * @brief Read the current CRC configuration from the hardware.
 *
 * Reads the CRC control registers and populates @a userConfigPtr with the
 * currently active settings (width, transpose modes, complement flag, seed).
 *
 * @param[in]  instance       CRC instance index (0-based).
 * @param[out] userConfigPtr  Pointer to the structure to receive the current
 *                            configuration. Must not be NULL.
 * @return Execution status.
 * @retval STATUS_SUCCESS     Configuration retrieved successfully.
 */
status_t CRC_DRV_GetConfig(uint32_t instance,
                           crc_user_config_t *userConfigPtr);

/*! @} */ /* End of Configuration */

/*******************************************************************************
 * CRC Calculation
 ******************************************************************************/
/*!
 * @name CRC Calculation
 * @brief Functions for feeding data to the CRC engine and retrieving results.
 *
 * **Single-value APIs** (CRC_DRV_GetCrc32/16/8): convenience functions that
 * optionally set a new seed, feed one data element, and return the result —
 * all in a single call.
 *
 * **Block-write APIs** (CRC_DRV_WriteData/16/32): feed a contiguous buffer
 * of data into the running CRC calculation. Call CRC_DRV_GetCrcResult()
 * afterwards to retrieve the final checksum.
 *
 * **Result API** (CRC_DRV_GetCrcResult): returns the current (intermediate
 * or final) CRC result.
 * @{
 */

/*!
 * @brief Compute CRC over a single 32-bit data word.
 *
 * Optionally loads a new seed, then feeds a single 32-bit value into the
 * CRC engine and returns the updated checksum.
 *
 * @param[in] instance  CRC instance index (0-based).
 * @param[in] data      32-bit data word for CRC computation.
 * @param[in] newSeed   If true, @a seed is written to the INIT register
 *                      before computation (starts a new CRC).
 *                      If false, @a seed is ignored and the computation
 *                      continues from the previous state.
 * @param[in] seed      New seed value (only used when @a newSeed is true).
 * @return  Updated CRC result after processing @a data.
 *
 * @pre CRC_DRV_Init() or CRC_DRV_Configure() must have been called.
 */
uint32_t CRC_DRV_GetCrc32(uint32_t instance,
                          uint32_t data,
                          bool newSeed,
                          uint32_t seed);

/*!
 * @brief Compute CRC over a single 16-bit data half-word.
 *
 * Optionally loads a new seed, then feeds a single 16-bit value into the
 * CRC engine and returns the updated checksum.
 *
 * @param[in] instance  CRC instance index (0-based).
 * @param[in] data      16-bit data half-word for CRC computation.
 * @param[in] newSeed   If true, @a seed is written to the INIT register
 *                      before computation (starts a new CRC).
 *                      If false, @a seed is ignored.
 * @param[in] seed      New seed value (only used when @a newSeed is true).
 * @return  Updated CRC result after processing @a data.
 *
 * @pre CRC_DRV_Init() or CRC_DRV_Configure() must have been called.
 */
uint32_t CRC_DRV_GetCrc16(uint32_t instance,
                          uint16_t data,
                          bool newSeed,
                          uint32_t seed);

/*!
 * @brief Compute CRC over a single 8-bit data byte.
 *
 * Optionally loads a new seed, then feeds a single 8-bit value into the
 * CRC engine and returns the updated checksum.
 *
 * @param[in] instance  CRC instance index (0-based).
 * @param[in] data      8-bit data byte for CRC computation.
 * @param[in] newSeed   If true, @a seed is written to the INIT register
 *                      before computation (starts a new CRC).
 *                      If false, @a seed is ignored.
 * @param[in] seed      New seed value (only used when @a newSeed is true).
 * @return  Updated CRC result after processing @a data.
 *
 * @pre CRC_DRV_Init() or CRC_DRV_Configure() must have been called.
 */
uint32_t CRC_DRV_GetCrc8(uint32_t instance,
                         uint8_t data,
                         bool newSeed,
                         uint32_t seed);

/*!
 * @brief Feed a byte array into the running CRC calculation.
 *
 * Writes each byte of @a data sequentially to the CRC data register.
 * The running checksum accumulates across all bytes. Call
 * CRC_DRV_GetCrcResult() after this function to retrieve the result.
 *
 * This function can be called multiple times to process data in segments
 * (streaming CRC computation).
 *
 * @param[in] instance  CRC instance index (0-based).
 * @param[in] data      Pointer to the byte buffer. Must not be NULL.
 * @param[in] dataSize  Number of bytes to process.
 *
 * @pre The CRC engine must be configured (seed and protocol set) before the
 *      first call. Use CRC_DRV_Configure() to set the seed for a new
 *      calculation.
 */
void CRC_DRV_WriteData(uint32_t instance,
                       const uint8_t *data,
                       uint32_t dataSize);

/*!
 * @brief Feed a half-word array into the running CRC calculation.
 *
 * Writes each 16-bit element of @a data sequentially to the CRC data register.
 * Call CRC_DRV_GetCrcResult() afterwards to retrieve the checksum.
 *
 * @param[in] instance  CRC instance index (0-based).
 * @param[in] data      Pointer to the half-word buffer. Must not be NULL.
 * @param[in] dataSize  Number of 16-bit elements to process.
 *
 * @pre CRC engine must be configured before the first call.
 */
void CRC_DRV_WriteData16(uint32_t instance,
                         const uint16_t *data,
                         uint32_t dataSize);

/*!
 * @brief Feed a word array into the running CRC calculation.
 *
 * Writes each 32-bit element of @a data sequentially to the CRC data register.
 * Call CRC_DRV_GetCrcResult() afterwards to retrieve the checksum.
 *
 * @param[in] instance  CRC instance index (0-based).
 * @param[in] data      Pointer to the word buffer. Must not be NULL.
 * @param[in] dataSize  Number of 32-bit elements to process.
 *
 * @pre CRC engine must be configured before the first call.
 */
void CRC_DRV_WriteData32(uint32_t instance,
                         const uint32_t *data,
                         uint32_t dataSize);

/*!
 * @brief Retrieve the current CRC computation result.
 *
 * Returns the current checksum from the CRC result register. The value is
 * automatically adjusted to the configured protocol width (16-bit or 32-bit)
 * and reflects any complement/transpose settings.
 *
 * @param[in] instance  CRC instance index (0-based).
 * @return  Current CRC result value.
 *
 * @note This can be called at any point during a streaming calculation to
 *       obtain an intermediate checksum.
 */
uint32_t CRC_DRV_GetCrcResult(uint32_t instance);

/*! @} */ /* End of CRC Calculation */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of crc group */

#endif /* CRC_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
