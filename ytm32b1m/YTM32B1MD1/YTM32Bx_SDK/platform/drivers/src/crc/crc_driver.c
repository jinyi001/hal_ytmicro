/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file crc_driver.c
 * @version 1.4.1
 *
 * @brief CRC Driver — implementation of the public CRC_DRV_* API.
 *
 * This file implements the application-level CRC driver functions declared
 * in crc_driver.h. Each function wraps the low-level hardware access layer
 * (crc_hw_access.h) and provides instance-based access to the CRC peripheral.
 */

#include "device_registers.h"
#include "crc_hw_access.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Table of base addresses for CRC peripheral instances. */
static CRC_Type *const s_crcBase[] = CRC_BASE_PTRS;

/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/

/*!
 * @brief Initialize the CRC driver with user-provided configuration.
 */
status_t CRC_DRV_Init(uint32_t instance,
                      const crc_user_config_t *userConfigPtr)
{
    DEV_ASSERT(instance < CRC_INSTANCE_COUNT);
    DEV_ASSERT(userConfigPtr != NULL);
    CRC_Type *base = s_crcBase[instance];
    status_t retStatus;

    /* Set the default configuration */
    CRC_Init(base);
    /* Set the CRC configuration */
    retStatus = CRC_DRV_Configure(instance, userConfigPtr);

    return retStatus;
}

/*!
 * @brief De-initialize the CRC driver, resetting to default configuration.
 */
status_t CRC_DRV_Deinit(uint32_t instance)
{
    DEV_ASSERT(instance < CRC_INSTANCE_COUNT);
    CRC_Type *base = s_crcBase[instance];

    /* Set the default configuration */
    CRC_Init(base);

    return STATUS_SUCCESS;
}

/*!
 * @brief Populate a configuration structure with default CRC parameters.
 */
status_t CRC_DRV_GetDefaultConfig(crc_user_config_t *userConfigPtr)
{
    DEV_ASSERT(userConfigPtr != NULL);

    /* Default CRC mode: typically 16-bit */
    userConfigPtr->crcWidth = FEATURE_CRC_DEFAULT_WIDTH;
    /* Default read transpose: none */
    userConfigPtr->readTranspose = FEATURE_CRC_DEFAULT_READ_TRANSPOSE;
    /* Default write transpose: none */
    userConfigPtr->writeTranspose = FEATURE_CRC_DEFAULT_WRITE_TRANSPOSE;
    /* Default: no complement / inversion of checksum */
    userConfigPtr->complementChecksum = false;
    /* Default seed value */
    userConfigPtr->seed = FEATURE_CRC_DEFAULT_SEED;

    return STATUS_SUCCESS;
}

/*******************************************************************************
 * Configuration
 ******************************************************************************/

/*!
 * @brief Apply a user configuration to the CRC peripheral.
 */
status_t CRC_DRV_Configure(uint32_t instance,
                           const crc_user_config_t *userConfigPtr)
{
    DEV_ASSERT(instance < CRC_INSTANCE_COUNT);
    DEV_ASSERT(userConfigPtr != NULL);
    CRC_Type *base = s_crcBase[instance];

    /* Set CRC mode */
    CRC_SetProtocolWidth(base, userConfigPtr->crcWidth);
    /* Set read transpose */
    CRC_SetReadTranspose(base, userConfigPtr->readTranspose);
    /* Set write transpose */
    CRC_SetWriteTranspose(base, userConfigPtr->writeTranspose);
    /* Set complement / inversion checksum */
    CRC_SetFXorMode(base, userConfigPtr->complementChecksum);
    /* Write seed (initial checksum) */
    CRC_SetSeedReg(base, userConfigPtr->seed);

    return STATUS_SUCCESS;
}

/*!
 * @brief Read the current CRC configuration from the hardware.
 */
status_t CRC_DRV_GetConfig(uint32_t instance,
                           crc_user_config_t *userConfigPtr)
{
    DEV_ASSERT(instance < CRC_INSTANCE_COUNT);
    DEV_ASSERT(userConfigPtr != NULL);
    const CRC_Type *const base = s_crcBase[instance];

    /* Read CRC mode */
    userConfigPtr->crcWidth = CRC_GetProtocolWidth(base);
    /* Read transpose settings */
    userConfigPtr->readTranspose = CRC_GetReadTranspose(base);
    userConfigPtr->writeTranspose = CRC_GetWriteTranspose(base);
    /* Read complement / inversion setting */
    userConfigPtr->complementChecksum = CRC_GetFXorMode(base);
    /* Read seed */
    userConfigPtr->seed = CRC_GetSeedReg(base);

    return STATUS_SUCCESS;
}

/*******************************************************************************
 * CRC Calculation
 ******************************************************************************/

/*!
 * @brief Compute CRC over a single 32-bit data word.
 */
uint32_t CRC_DRV_GetCrc32(uint32_t instance,
                          uint32_t data,
                          bool newSeed,
                          uint32_t seed)
{
    DEV_ASSERT(instance < CRC_INSTANCE_COUNT);
    CRC_Type *base = s_crcBase[instance];

    /* If newSeed is true then write a seed to initial checksum */
    if (newSeed)
    {
        CRC_SetSeedReg(base, seed);
    }

    /* Write 32-bit data */
    CRC_SetDataReg(base, data);

    /* Result of the CRC calculation */
    return CRC_GetCrcResult(base);
}

/*!
 * @brief Compute CRC over a single 16-bit data half-word.
 */
uint32_t CRC_DRV_GetCrc16(uint32_t instance,
                          uint16_t data,
                          bool newSeed,
                          uint32_t seed)
{
    DEV_ASSERT(instance < CRC_INSTANCE_COUNT);
    CRC_Type *base = s_crcBase[instance];

    /* If newSeed is true then write a seed to initial checksum */
    if (newSeed)
    {
        CRC_SetSeedReg(base, seed);
    }

    /* Write 16-bit data */
    CRC_SetDataReg16(base, data);

    /* Result of the CRC calculation */
    return CRC_GetCrcResult(base);
}

/*!
 * @brief Compute CRC over a single 8-bit data byte.
 */
uint32_t CRC_DRV_GetCrc8(uint32_t instance,
                         uint8_t data,
                         bool newSeed,
                         uint32_t seed)
{
    DEV_ASSERT(instance < CRC_INSTANCE_COUNT);
    CRC_Type *base = s_crcBase[instance];

    /* If newSeed is true then write a seed to initial checksum */
    if (newSeed)
    {
        CRC_SetSeedReg(base, seed);
    }

    /* Write 8-bit data */
    CRC_SetDataReg8(base, data);

    /* Result of the CRC calculation */
    return CRC_GetCrcResult(base);
}

/*!
 * @brief Feed a byte array into the running CRC calculation.
 */
void CRC_DRV_WriteData(uint32_t instance,
                       const uint8_t *data,
                       uint32_t dataSize)
{
    DEV_ASSERT(instance < CRC_INSTANCE_COUNT);
    DEV_ASSERT(data != NULL);
    uint32_t i;
    CRC_Type *base = s_crcBase[instance];

    /* 8-bit writes till end of data buffer */
    for (i = 0U; i < dataSize; i++)
    {
        CRC_SetDataReg8(base, data[i]);
    }
}

/*!
 * @brief Feed a half-word array into the running CRC calculation.
 */
void CRC_DRV_WriteData16(uint32_t instance,
                         const uint16_t *data,
                         uint32_t dataSize)
{
    DEV_ASSERT(instance < CRC_INSTANCE_COUNT);
    DEV_ASSERT(data != NULL);
    uint32_t i;
    CRC_Type *base = s_crcBase[instance];

    /* 16-bit writes till end of data buffer */
    for (i = 0U; i < dataSize; i++)
    {
        CRC_SetDataReg16(base, data[i]);
    }
}

/*!
 * @brief Feed a word array into the running CRC calculation.
 */
void CRC_DRV_WriteData32(uint32_t instance,
                         const uint32_t *data,
                         uint32_t dataSize)
{
    DEV_ASSERT(instance < CRC_INSTANCE_COUNT);
    DEV_ASSERT(data != NULL);
    uint32_t i;
    CRC_Type *base = s_crcBase[instance];

    /* 32-bit writes till end of data buffer */
    for (i = 0U; i < dataSize; i++)
    {
        CRC_SetDataReg(base, data[i]);
    }
}

/*!
 * @brief Retrieve the current CRC computation result.
 */
uint32_t CRC_DRV_GetCrcResult(uint32_t instance)
{
    DEV_ASSERT(instance < CRC_INSTANCE_COUNT);
    const CRC_Type *base = s_crcBase[instance];

    /* Result of the CRC calculation */
    return CRC_GetCrcResult(base);
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
