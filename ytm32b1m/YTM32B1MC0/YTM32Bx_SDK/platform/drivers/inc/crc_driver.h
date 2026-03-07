/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file crc_driver.h
 * @version 1.4.0
 */

#ifndef CRC_DRIVER_H
#define CRC_DRIVER_H

#include <stddef.h>
#include "device_registers.h"
#include "status.h"

/*!
 * @defgroup crc_driver CRC Driver
 * @ingroup crc
 * @brief Cyclic Redundancy Check Peripheral Driver
 * @details This section describes the programming interface of the CRC driver.
 * @{
 */
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief CRC type of transpose of read write data
 * Implements : crc_transpose_t_Class
 */
typedef enum
{
    CRC_TRANSPOSE_NONE = 0x00U,    /*!< No transpose */
    CRC_TRANSPOSE_BITS = 0x01U     /*!< Transpose bits in bytes */
} crc_transpose_t;

/*!
 * @brief CRC bit width
 * Implements : crc_bit_width_t_Class
 */
#if (FEATURE_CRC_SUPPORT_CRC4)
typedef enum
{
    CRC_BITS_4  = 0U,   /*!< Generate  4-bit CRC code */
    CRC_BITS_16 = 1U,   /*!< Generate 16-bit CRC code */
    CRC_BITS_32 = 2U,   /*!< Generate 32-bit CRC code */
#if FEATURE_CRC_SUPPORT_CRC8
    CRC_BITS_8 =  3U    /*!< Generate 8-bit CRC code */
#endif /* FEATURE_CRC_SUPPORT_CRC8 */
} crc_bit_width_t;
#else
typedef enum
{
    CRC_BITS_16 = 0U,   /*!< Generate 16-bit CRC code */
    CRC_BITS_32 = 1U    /*!< Generate 32-bit CRC code */
} crc_bit_width_t;
#endif /* FEATURE_CRC_SUPPORT_CRC4 */

/*!
 * @brief CRC configuration structure.
 * Implements : crc_user_config_t_Class
 */
typedef struct
{
    crc_bit_width_t crcWidth;               /*!< Selects 16-bit or 32-bit CRC protocol. */
    crc_transpose_t readTranspose;          /*!< Type of transpose when reading CRC result. */
    crc_transpose_t writeTranspose;         /*!< Type of transpose when writing CRC input data. */
    bool complementChecksum;                /*!< True if the result shall be complement of the actual checksum. */
    uint32_t seed;                          /*!< Starting checksum value. */
} crc_user_config_t;

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @name CRC DRIVER API
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initializes the CRC module
 *
 * This function initializes CRC driver based on user configuration input.
 * The user must make sure that the clock is enabled
 *
 * @param[in] instance The CRC instance number
 * @param[in] userConfigPtr Pointer to structure of initialization
 * @return Execution status (success)
 * @retval STATUS_SUCCESS Operation was successful
 */
status_t CRC_DRV_Init(uint32_t instance,
                      const crc_user_config_t *userConfigPtr);

/*!
 * @brief Sets the default configuration
 *
 * This function sets the default configuration
 *
 * @param[in] instance The CRC instance number
 * @return Execution status (success)
 * @retval STATUS_SUCCESS Operation was successful
 */
status_t CRC_DRV_Deinit(uint32_t instance);

/*!
 * @brief Appends 32-bit data to the current CRC calculation and returns new result
 *
 * This function appends 32-bit data to the current CRC calculation
 * and returns new result. If the newSeed is true, seed set and
 * result are calculated from the seed new value (new CRC calculation)
 *
 * @param[in] instance The CRC instance number
 * @param[in] data Input data for CRC calculation
 * @param[in] newSeed Sets new CRC calculation
 *            - true: New seed set and used for new calculation.
 *            - false: Seed argument ignored, continues old calculation.
 * @param[in] seed New seed if newSeed is true, else ignored
 * @return New CRC result
 */
uint32_t CRC_DRV_GetCrc32(uint32_t instance,
                          uint32_t data,
                          bool newSeed,
                          uint32_t seed);

/*!
 * @brief Appends 16-bit data to the current CRC calculation and returns new result
 *
 * This function appends 16-bit data to the current CRC calculation
 * and returns new result. If the newSeed is true, seed set and
 * result are calculated from the seed new value (new CRC calculation)
 *
 * @param[in] instance The CRC instance number
 * @param[in] data Input data for CRC calculation
 * @param[in] newSeed Sets new CRC calculation
 *            - true: New seed set and used for new calculation.
 *            - false: Seed argument ignored, continues old calculation.
 * @param[in] seed New seed if newSeed is true, else ignored
 * @return New CRC result
 */
uint32_t CRC_DRV_GetCrc16(uint32_t instance,
                          uint16_t data,
                          bool newSeed,
                          uint32_t seed);

/*!
 * @brief Appends 8-bit data to the current CRC calculation and returns new result
 *
 * This function appends 8-bit data to the current CRC calculation
 * and returns new result. If the newSeed is true, seed set and
 * result are calculated from the seed new value (new CRC calculation)
 *
 * @param[in] instance The CRC instance number
 * @param[in] data Input data for CRC calculation
 * @param[in] newSeed Sets new CRC calculation
 *            - true: New seed set and used for new calculation.
 *            - false: Seed argument ignored, continues old calculation.
 * @param[in] seed New seed if newSeed is true, else ignored
 * @return New CRC result
 */
uint32_t CRC_DRV_GetCrc8(uint32_t instance,
                         uint8_t data,
                         bool newSeed,
                         uint32_t seed);

/*!
 * @brief Appends a block of bytes to the current CRC calculation
 *
 * This function appends a block of bytes to the current CRC calculation
 *
 * @param[in] instance The CRC instance number
 * @param[in] data Data for current CRC calculation
 * @param[in] dataSize Length of data to be calculated
 */
void CRC_DRV_WriteData(uint32_t instance,
                       const uint8_t *data,
                       uint32_t dataSize);

/*!
 * @brief Appends a block of half word to the current CRC calculation
 *
 * This function appends a block of half words to the current CRC calculation
 *
 * @param[in] instance The CRC instance number
 * @param[in] data Data for current CRC calculation
 * @param[in] dataSize Length of data in half words to be calculated
 */
void CRC_DRV_WriteData16(uint32_t instance,
                         const uint16_t *data,
                         uint32_t dataSize);

/*!
 * @brief Appends a block of word to the current CRC calculation
 *
 * This function appends a block of words to the current CRC calculation
 *
 * @param[in] instance The CRC instance number
 * @param[in] data Data for current CRC calculation
 * @param[in] dataSize Length of data in words to be calculated
 */
void CRC_DRV_WriteData32(uint32_t instance,
                         const uint32_t *data,
                         uint32_t dataSize);

/*!
 * @brief Returns the current result of the CRC calculation
 *
 * This function returns the current result of the CRC calculation
 *
 * @param[in] instance The CRC instance number
 * @return Result of CRC calculation
 */
uint32_t CRC_DRV_GetCrcResult(uint32_t instance);

/*!
 * @brief Configures the CRC module from a user configuration structure
 *
 * This function configures the CRC module from a user configuration structure
 *
 * @param[in] instance The CRC instance number
 * @param[in] userConfigPtr Pointer to structure of initialization
 * @return Execution status (success)
 */
status_t CRC_DRV_Configure(uint32_t instance,
                           const crc_user_config_t *userConfigPtr);

/*!
 * @brief Get configures of the CRC module currently
 *
 * This function Get configures of the CRC module currently
 *
 * @param[in] instance The CRC instance number
 * @param[out] userConfigPtr Pointer to structure of initialization
 * @return Execution status (success)
 */
status_t CRC_DRV_GetConfig(uint32_t instance,
                           crc_user_config_t *userConfigPtr);

/*!
 * @brief Get default configures the CRC module for configuration structure
 *
 * This function Get default configures the CRC module for user configuration structure
 *
 * @param[out] userConfigPtr Pointer to structure of initialization
 * @return Execution status (success)
 */
status_t CRC_DRV_GetDefaultConfig(crc_user_config_t *userConfigPtr);

/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* CRC_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
