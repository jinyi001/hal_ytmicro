/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file crc_driver.c
 * @version 1.4.0
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 */

#include "device_registers.h"
#include "crc_hw_access.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*! @brief Table of base addresses for CRC instances. */
static CRC_Type *const s_crcBase[] = CRC_BASE_PTRS;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_DRV_Init
 * Description   : This function initializes CRC driver based on user configuration input.
 * The user must make sure that the clock is enabled.
 *
 * Implements    : CRC_DRV_Init_Activity
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_DRV_Deinit
 * Description   : This function sets the default configuration.
 *
 * Implements    : CRC_DRV_Deinit_Activity
 *END**************************************************************************/
status_t CRC_DRV_Deinit(uint32_t instance)
{
    DEV_ASSERT(instance < CRC_INSTANCE_COUNT);
    CRC_Type *base = s_crcBase[instance];

    /* Set the default configuration */
    CRC_Init(base);

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_DRV_GetCrc32
 * Description   : This function appends 32-bit data to the current CRC calculation
 * and returns new result. If the newSeed is true, seed set and result are calculated
 * from the seed new value (new CRC calculation).
 *
 * Implements    : CRC_DRV_GetCrc32_Activity
 *END**************************************************************************/
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
        /* Write a seed - initial checksum */
        CRC_SetSeedReg(base, seed);
    }

    /* Write 32-bit data */
    CRC_SetDataReg(base, data);

    /* Result of the CRC calculation */
    return CRC_GetCrcResult(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_DRV_GetCrc16
 * Description   : This function appends 16-bit data to the current CRC calculation
 * and returns new result. If the newSeed is true, seed set and result are calculated
 * from the seed new value (new CRC calculation).
 *
 * Implements    : CRC_DRV_GetCrc16_Activity
 *END**************************************************************************/
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
        /* Write a seed - initial checksum */
        CRC_SetSeedReg(base, seed);
    }
    /* Write 16-bit data */
    CRC_SetDataReg16(base, data);

    /* Result of the CRC calculation */
    return CRC_GetCrcResult(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_DRV_GetCrc8
 * Description   : This function appends 8-bit data to the current CRC calculation
 * and returns new result. If the newSeed is true, seed set and result are calculated
 * from the seed new value (new CRC calculation).
 *
 * Implements    : CRC_DRV_GetCrc8_Activity
 *END**************************************************************************/
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
        /* Write a seed - initial checksum */
        CRC_SetSeedReg(base, seed);
    }
    /* Write 8-bit data */
    CRC_SetDataReg8(base, data);

    /* Result of the CRC calculation */
    return CRC_GetCrcResult(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_DRV_WriteData
 * Description   : This function appends a block of bytes to the current CRC calculation.
 *
 * Implements    : CRC_DRV_WriteData_Activity
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_DRV_WriteData16
 * Description   : This function appends a block of bytes to the current CRC calculation.
 *
 * Implements    : CRC_DRV_WriteData_Activity
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_DRV_WriteData32
 * Description   : This function appends a block of bytes to the current CRC calculation.
 *
 * Implements    : CRC_DRV_WriteData_Activity
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_DRV_GetCrcResult
 * Description   : This function returns the current result of the CRC calculation.
 *
 * Implements    : CRC_DRV_GetCrcResult_Activity
 *END**************************************************************************/
uint32_t CRC_DRV_GetCrcResult(uint32_t instance)
{
    DEV_ASSERT(instance < CRC_INSTANCE_COUNT);
    const CRC_Type *base = s_crcBase[instance];

    /* Result of the CRC calculation */
    return CRC_GetCrcResult(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_DRV_Configure
 * Description   : This function configures the CRC module from a user configuration structure.
 *
 * Implements    : CRC_DRV_Configure_Activity
 *END**************************************************************************/
status_t CRC_DRV_Configure(uint32_t instance,
                           const crc_user_config_t *userConfigPtr)
{
    DEV_ASSERT(instance < CRC_INSTANCE_COUNT);
    DEV_ASSERT(userConfigPtr != NULL);
    CRC_Type *base = s_crcBase[instance];

    /* Set CRC mode */
    CRC_SetProtocolWidth(base, userConfigPtr->crcWidth);
    /* Set transposes options */
    CRC_SetReadTranspose(base, userConfigPtr->readTranspose);
    /* Set writes transposes */
    CRC_SetWriteTranspose(base, userConfigPtr->writeTranspose);
    /* Sets complement or inversion checksum */
    CRC_SetFXorMode(base, userConfigPtr->complementChecksum);
    /* Write a seed - initial checksum */
    CRC_SetSeedReg(base, userConfigPtr->seed);

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_DRV_GetConfig
 * Description   : This function Get configures of the CRC module currently
 *
 * Implements    : CRC_DRV_GetConfig_Activity
 *END**************************************************************************/
status_t CRC_DRV_GetConfig(uint32_t instance,
                           crc_user_config_t *userConfigPtr)
{
    DEV_ASSERT(instance < CRC_INSTANCE_COUNT);
    DEV_ASSERT(userConfigPtr != NULL);
    const CRC_Type *const base = s_crcBase[instance];

    /* Gets CRC mode */
    userConfigPtr->crcWidth = CRC_GetProtocolWidth(base);
    /* Gets transposes and complement options */
    userConfigPtr->readTranspose = CRC_GetReadTranspose(base);
    /* Gets transposes options */
    userConfigPtr->writeTranspose = CRC_GetWriteTranspose(base);
    /* Gets complement or inversion checksum */
    userConfigPtr->complementChecksum = CRC_GetFXorMode(base);
    /* Get a seed - initial checksum */
    userConfigPtr->seed = CRC_GetSeedReg(base);

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_DRV_GetDefaultConfig
 * Description   : This function Get default configures the CRC module for user configuration structure
 *
 * Implements    : CRC_DRV_GetDefaultConfig_Activity
 *END**************************************************************************/
status_t CRC_DRV_GetDefaultConfig(crc_user_config_t *userConfigPtr)
{
    DEV_ASSERT(userConfigPtr != NULL);

    /* Gets CRC mode default is 16 bit */
    userConfigPtr->crcWidth = FEATURE_CRC_DEFAULT_WIDTH;
    /* Gets default read transposes none */
    userConfigPtr->readTranspose = FEATURE_CRC_DEFAULT_READ_TRANSPOSE;
    /* Gets default write transpose none */
    userConfigPtr->writeTranspose = FEATURE_CRC_DEFAULT_WRITE_TRANSPOSE;
    /* Gets default no complement or inversion checksum */
    userConfigPtr->complementChecksum = false;
    /* Gets default a seed - initial checksum */
    userConfigPtr->seed = FEATURE_CRC_DEFAULT_SEED;

    return STATUS_SUCCESS;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
