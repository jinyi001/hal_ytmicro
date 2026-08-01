/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file ee_high_level.c
 * @version 1.4.1
 *
 * @brief EEPROM Emulation high-level service implementation.
 */

#include "ee_emulation.h"
#include "ee_hardware.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

#if EE_CALLBACK_ENABLE
/*! @brief Optional application callback serviced while Flash commands are pending. */
ee_callback_t g_EECallBack = EE_NULL_CALLBACK;
#endif

/*******************************************************************************
 * Initialization
 ******************************************************************************/

/*!
 * @brief Discover live sectors, repair sector state, and rebuild the cache.
 */
status_t Eed_InitEeprom(eeprom_config_t * eepromConfig)
{
    status_t ret_val;

    /* Seed the live-sector window before dead sectors are compacted out. */
    eepromConfig->maxValidIndex = (uint32_t)(EE_ALLOCATED_SECTORS - 0x1U);

    /* Move dead sectors to the tail of the round-robin list. */
    ret_val = Eed_RemoveDeadSector(eepromConfig);

    if (ret_val == STATUS_SUCCESS)
    {
        ret_val = Eed_InitAllSectors(eepromConfig);
    }

    if (STATUS_SUCCESS == ret_val)
    {
        /* Rebuild the optional cache against the finalized sector ordering. */
        Eed_UpdateCacheTable(eepromConfig, true);
        Eed_UpdateCacheTable(eepromConfig, false);
    }

    return (ret_val);
}

/*******************************************************************************
 * Record Access
 ******************************************************************************/

/*!
 * @brief Read the newest stored copy of one logical EEPROM record.
 */
status_t Eed_ReadEeprom(eeprom_config_t * eepromConfig, uint16_t data_id, uint8_t *pData)
{
    status_t ret_val = STATUS_SUCCESS;
    uint32_t rec_addr;
    uint32_t i;

    /* Reject record identifiers outside the emulated EEPROM address space. */
    if (EE_MAX_RECORD_NUMBER <= data_id)
    {
        ret_val = STATUS_EdPROM_ID_OUT_OF_RANGE;
    }
    else
    {
        /* Locate the newest record image across the active sector window. */
        rec_addr = Eed_SearchLoop(eepromConfig, data_id);
        if (rec_addr == 0xFFFFFFFFU)
        {
            ret_val = STATUS_EdPROM_ID_NOT_FOUND;
        }
        else
        {
            for (i = 0; i < EE_DATA_VALUE_SIZE; i++)
            {
                *(pData + i) = *(uint8_t *)(rec_addr + i);
            }
        }
    }
    return (ret_val);
}

/*!
 * @brief Append one logical EEPROM record and trigger a sector swap when needed.
 */
status_t Eed_WriteEeprom(eeprom_config_t * eepromConfig, uint16_t data_id, const uint8_t* pData)
{
    uint32_t count = 0x0U;
    uint32_t tmp;
    status_t ret_val;

    if (EE_MAX_RECORD_NUMBER <= data_id)
    {
        return STATUS_EdPROM_ID_OUT_OF_RANGE;
    }

    /* Bound the retry loop so a persistent write failure cannot spin forever. */
    while (count < EE_MAX_WRITE_LOOP)
    {
        if (eepromConfig->blankSpace == (eepromConfig->startSecAddr[EE_ACTIVE_SECTORS - 1U] +
            EE_SECTOR_SIZE - EE_SECTOR_WASTE))
        {
            /* The youngest active sector is full, so the write path must advance the active window. */
            if (0xFFFFFFFFU != READ32(eepromConfig->startSecAddr[0U] + EE_SECTOR_ACTIND_OFFSET))
            {
                /* Preserve the current blank slot in case the swap must be retried. */
                tmp = eepromConfig->blankSpace;

                ret_val = Eed_CopyData(eepromConfig);
                if (STATUS_SUCCESS == ret_val)
                {
                    ret_val = Eed_ValidateActIndicator(eepromConfig, (uint8_t)EE_ACTIVE_SECTORS);
                    if (STATUS_SUCCESS == ret_val)
                    {
                        /* Reinitialize the oldest active sector after the swap target becomes active. */
                        tmp = READ32(eepromConfig->startSecAddr[0U] + EE_SECTOR_ERSCYC_OFFSET) + 0x1U;
                        ret_val = Eed_SyncValidateSector(eepromConfig, tmp, 0U);
                        if (STATUS_SUCCESS == ret_val)
                        {
                            Eed_ShiftIdxToBase(eepromConfig, (uint8_t)EE_ACTIVE_SECTORS);
                            Eed_UpdateCacheTable(eepromConfig, false);
                        }
                        if (STATUS_EdPROM_MAKE_DEAD_SUCCESS == ret_val)
                        {
                            ret_val = STATUS_SUCCESS;
                        }
                    }
                }

                /* Restart the swap from the original blank slot when the failing sector was retired. */
                if (STATUS_EdPROM_MAKE_DEAD_SUCCESS == ret_val)
                {
                    eepromConfig->blankSpace = tmp;
                }
            }
            else
            {
                do
                {
                    ret_val = Eed_ValidateActIndicator(eepromConfig, (uint8_t)EE_ACTIVE_SECTORS);
                } while (STATUS_EdPROM_MAKE_DEAD_SUCCESS == ret_val);

                /* Advance the blank slot into the newly activated ready sector. */
                eepromConfig->blankSpace = eepromConfig->startSecAddr[EE_ACTIVE_SECTORS] + EE_SECTOR_HEADER_SIZE;
                Eed_ShiftIdxToBase(eepromConfig, (uint8_t)EE_ACTIVE_SECTORS);
            }

            if (STATUS_EdPROM_MAKE_DEAD_ERROR == ret_val)
            {
                return STATUS_EdPROM_MAKE_DEAD_ERROR;
            }
        }
        else
        {
            ret_val = Eed_WriteRecord(eepromConfig, (uint32_t)pData, data_id);
            if (STATUS_SUCCESS == ret_val)
            {
                /* Refresh the tracked erase-cycle value after a successful append. */
                eepromConfig->eraseCycValue = READ32(eepromConfig->startSecAddr[EE_ACTIVE_SECTORS - 1U] +
                                                    EE_SECTOR_ERSCYC_OFFSET);
                break;
            }
        }
        count ++;
    }
    return (ret_val);
}

/*******************************************************************************
 * Shutdown
 ******************************************************************************/

/*!
 * @brief Erase every Flash sector assigned to the EEPROM emulation pool.
 */
/*lint -e{818} */
status_t Eed_DeinitEeprom(eeprom_config_t * eepromConfig)
{
    /*lint -e(818) */
    uint32_t i;
    status_t ret_val;

    for (i = 0U; i < EE_ALLOCATED_SECTORS; i++)
    {
        ret_val = Eed_FlashErase(eepromConfig->startSecAddr[i]);
        if (STATUS_SUCCESS != ret_val)
        {
            ret_val = STATUS_EdPROM_SECTOR_ERASE_ERROR;
            break;
        }
    }
    return /*lint -e(539) */ (ret_val);
}
