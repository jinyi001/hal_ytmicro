/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file ee_middle_level.c
 * @version 1.4.1
 *
 * @brief EEPROM Emulation middle-level runtime helper implementation.
 */

#include "ee_emulation.h"
#include "ee_hardware.h"

/*! @brief Optional cache table that stores record addresses for the lowest IDs. */
static uint32_t s_EE_CacheTable[EE_CACHE_TABLE_MAX_ENTRY];

/*******************************************************************************
 * Data Migration & Record Programming
 ******************************************************************************/

/*!
 * @brief Copy the newest valid records from the oldest active sector into the
 *        ready swap sector.
 */
status_t Eed_CopyData(eeprom_config_t * eepromConfig)
{
    uint32_t    rec_addr;
    uint32_t    dest;
    uint32_t    end_search_addr;
    uint32_t    temp;
    uint16_t    data_id;
    uint16_t    status;
    status_t     ret_val = STATUS_SUCCESS;

    rec_addr = eepromConfig->startSecAddr[0] + EE_SECTOR_SIZE - EE_SECTOR_WASTE - EE_RECORD_LENGTH;
    end_search_addr = eepromConfig->startSecAddr[0] + EE_SECTOR_HEADER_SIZE;
    dest = eepromConfig->startSecAddr[EE_ACTIVE_SECTORS] + EE_SECTOR_HEADER_SIZE;

    /* Walk backward through the oldest active sector from newest record to oldest. */
    while (rec_addr >= end_search_addr)
    {
        temp = READ32(rec_addr + EE_RECORD_STATUS_OFFSET);
        status = (uint16_t)(temp >> 0x10U);
        if (EE_RECORD_STATUS_VALID == status)
        {
            data_id = (uint16_t)temp;
            if (rec_addr == Eed_SearchLoop(eepromConfig, data_id))
            {
                /* Copy only the newest stored image of each logical record. */
                if (STATUS_SUCCESS != Eed_FlashProgram(dest, EE_RECORD_LENGTH, rec_addr))
                {
                    ret_val = Eed_SyncValidateSector(eepromConfig, eepromConfig->eraseCycValue, (uint8_t)EE_ACTIVE_SECTORS);
                    if (STATUS_SUCCESS == ret_val)
                    {
                        ret_val = STATUS_EdPROM_MAKE_DEAD_SUCCESS;
                    }
                    break;
                }
                else
                {
                    dest += EE_RECORD_LENGTH;
                }
            }
        }
        rec_addr -= EE_RECORD_LENGTH;
    }

    eepromConfig->blankSpace = dest;
    return (ret_val);
}

/*!
 * @brief Program one record payload and header into the current blank slot.
 */
status_t Eed_WriteRecord(eeprom_config_t * eepromConfig, uint32_t source, uint16_t data_id)
{
   uint32_t data;
#if (EE_DATA_VALUE_SIZE & EE_LONG_WORD_MASK)
    uint8_t i;
    uint8_t    remain_data[4] = {0xFFU,0xFFU,0xFFU,0xFFU};
#endif
    status_t    ret_val = STATUS_SUCCESS;

#if EE_DATA_VALUE_ALIGNED_PART > 0U
    ret_val = Eed_FlashProgram(eepromConfig->blankSpace, EE_DATA_VALUE_ALIGNED_PART, source);
#endif

#if (EE_DATA_VALUE_SIZE & EE_LONG_WORD_MASK)
    source += EE_DATA_VALUE_ALIGNED_PART;
    for ( i = 0U; i < (EE_DATA_VALUE_SIZE & EE_LONG_WORD_MASK); i ++)
    {
        remain_data[i] = READ8(source);
        source ++;
    }
    ret_val |= Eed_FlashProgram(eepromConfig->blankSpace + EE_DATA_VALUE_ALIGNED_PART, FTM_LONGWORD_SIZE, (uint32_t)remain_data);
#endif
    if (STATUS_SUCCESS == ret_val)
    {
        data = 0x55550000U | data_id;
        ret_val = Eed_FlashProgram(eepromConfig->blankSpace + (EE_RECORD_LENGTH - EE_RECORD_HEADER_SIZE), EE_RECORD_HEADER_SIZE, (uint32_t)&data);
            if ((data_id < EE_CACHE_TABLE_MAX_ENTRY) && (STATUS_SUCCESS == ret_val))
            {
                s_EE_CacheTable[data_id] = eepromConfig->blankSpace;
            }
    }

    eepromConfig->blankSpace += EE_RECORD_LENGTH;
    return (ret_val);
}

/*******************************************************************************
 * Search & Callback Service
 ******************************************************************************/

/*!
 * @brief Scan one active sector backward to locate its first blank record
 *        slot.
 */
uint32_t Eed_SearchBlankSpace(uint32_t sec_addr)
{
    uint32_t search_addr;
#if EE_CALLBACK_ENABLE
    uint32_t counter = 0x0U;
#endif
    search_addr = sec_addr + EE_SECTOR_SIZE - EE_SECTOR_WASTE;
    sec_addr += EE_SECTOR_HEADER_SIZE;
    do
    {
        search_addr -= EE_RECORD_LENGTH;
        if (STATUS_SUCCESS != Eed_FlashEraseVerify(search_addr, EE_RECORD_LENGTH >> 2U))
        {
            sec_addr = search_addr + EE_RECORD_LENGTH;
            break;
        }
#if EE_CALLBACK_ENABLE
        Eed_CallBack(&counter);
#endif
    }while (search_addr > sec_addr);
    return (sec_addr);
}

/*!
 * @brief Search all active sectors from newest to oldest for one record ID.
 */
/*lint -e{818, 850} */
uint32_t Eed_SearchInAllActives(eeprom_config_t * eepromConfig, uint16_t data_id)
{
    uint32_t rec_addr;
    uint32_t start_rec_addr;
    uint32_t found_addr = 0xFFFFFFFFU;
    uint32_t rec_head_val;
    #if EE_CALLBACK_ENABLE
    uint32_t counter = 0U;
    #endif
    int32_t i = (int32_t)EE_ACTIVE_SECTORS;
    rec_head_val = EE_RECORD_STATUS_VALID;
    rec_head_val = (rec_head_val << 16U) | data_id;

    rec_addr = eepromConfig->blankSpace;
    for (i = (EE_ACTIVE_SECTORS - 1U); i >= 0; i--)
    {
        start_rec_addr = eepromConfig->startSecAddr[i];
        if (0xFFFFFFFFU == READ32(start_rec_addr + EE_SECTOR_ACTIND_OFFSET))
        {
            break;
        }
        start_rec_addr += EE_SECTOR_HEADER_SIZE;
        do
        {
            rec_addr -= EE_RECORD_LENGTH;
            if (rec_head_val == READ32(rec_addr + EE_RECORD_STATUS_OFFSET))
            {
                found_addr = rec_addr;
                i = 0;
                break;
            }
#if EE_CALLBACK_ENABLE
            Eed_CallBack(&counter);
#endif
        }while (rec_addr > start_rec_addr);
        if (i >= 0x1)
        {
            rec_addr = eepromConfig->startSecAddr[i - 0x1] + EE_SECTOR_SIZE - EE_SECTOR_WASTE;
        }
    }
    return (found_addr);
}

/*!
 * @brief Search the cache first and then fall back to an active-sector scan.
 */
uint32_t Eed_SearchLoop (eeprom_config_t * eepromConfig, uint16_t data_id)
{
    uint32_t rec_addr = 0xFFFFFFFFU;
    if (EE_CACHE_TABLE_MAX_ENTRY > data_id)
    {
        rec_addr = s_EE_CacheTable[data_id];
    }
    if (rec_addr == 0xFFFFFFFFU)
    {
        rec_addr = Eed_SearchInAllActives(eepromConfig, data_id);
    }
    return (rec_addr);
}

#if (EE_CALLBACK_ENABLE)
/*!
 * @brief Invoke the registered callback after the configured polling interval.
 */
void Eed_CallBack(uint32_t* counter)
{
    uint32_t temp;
    temp = *counter;
    if (((temp++) >= EE_CALLBACK_COUNTER) && (EE_NULL_CALLBACK != p_gEECallBack))
    {
        p_gEECallBack();
        temp = 0x0U;

    }
  *counter = temp;
}
#endif

/*******************************************************************************
 * Sector State Management
 ******************************************************************************/

/*!
 * @brief Mark one sector as dead and compact the live sector list.
 */
status_t Eed_ValidateDeadIndicator(eeprom_config_t * eepromConfig, uint32_t index)
{
    status_t ret_val = STATUS_EdPROM_MAKE_DEAD_ERROR;

    if (0xFFFFFFFFU != Eed_OverProgram(eepromConfig->startSecAddr[index] + EE_SECTOR_DEADIND_OFFSET))
    {
        ret_val = Eed_RemoveDeadSector(eepromConfig);
        if (ret_val == STATUS_SUCCESS)
        {
            return STATUS_EdPROM_MAKE_DEAD_SUCCESS;
        }
    }
    return (ret_val);
}

/*!
 * @brief Erase one sector, program its erase-cycle value, and retire it on
 *        repeated failure.
 */
status_t Eed_SyncValidateSector(eeprom_config_t * eepromConfig, uint32_t expErsVal, uint32_t index)
{
    status_t ret_val;
    uint8_t re_ers_num = 0x0U;

    do
    {
        ret_val = Eed_FlashErase(eepromConfig->startSecAddr[index]);
        if (STATUS_SUCCESS == ret_val)
        {
            ret_val = Eed_FlashProgram(eepromConfig->startSecAddr[index] + EE_SECTOR_ERSCYC_OFFSET,
                                       EE_INDICATOR_SIZE,
                                       (uint32_t)&expErsVal);
        }
        re_ers_num++;
    }while ((STATUS_SUCCESS != ret_val) && (re_ers_num <= EE_REERASE_NUM));

    if (STATUS_SUCCESS != ret_val)
    {
        ret_val = Eed_ValidateDeadIndicator(eepromConfig, index);
    }
    return (ret_val);
}

/*!
 * @brief Program the active indicator for one sector.
 */
status_t Eed_ValidateActIndicator(eeprom_config_t * eepromConfig, uint8_t index)
{
    status_t ret_val = STATUS_SUCCESS;
    if (0xFFFFFFFFU == Eed_OverProgram(eepromConfig->startSecAddr[index] + EE_SECTOR_ACTIND_OFFSET))
    {
        ret_val = Eed_ValidateDeadIndicator(eepromConfig, index);
    }
    return (ret_val);
}

/*!
 * @brief Retry programming one sector-state indicator until it reads back or
 *        the retry limit is exhausted.
 */
uint32_t Eed_OverProgram(uint32_t dest)
{
    uint32_t data = EE_DATA_INDICATOR;
    uint32_t act_val;
    uint8_t try_num = 0x0U;
    do
    {
        try_num++;
        Eed_FlashProgram(dest, EE_INDICATOR_SIZE, (uint32_t)&data);
        act_val = READ32(dest);
    }while ((try_num <= EE_REPROGRAM_NUM) && (act_val == 0xFFFFFFFFU));
    return (act_val);
}

/*!
 * @brief Reconstruct active or ready sector state during initialization.
 */
status_t Eed_InitAllSectors(eeprom_config_t * eepromConfig)
{
    uint32_t ers_cyc_val;
    uint32_t active_num = 0U;
    status_t ret_val = STATUS_SUCCESS;
    uint32_t i;

    /* Count active sectors and capture the newest active index. */
    ers_cyc_val = eepromConfig->maxValidIndex;
    for (i = 0x0U; i <= eepromConfig->maxValidIndex; i++)
    {
        if (0xFFFFFFFFU != READ32(eepromConfig->startSecAddr[i] + EE_SECTOR_ACTIND_OFFSET))
        {
            active_num++;
            if (i < eepromConfig->maxValidIndex)
            {
                if (0xFFFFFFFFU == READ32(eepromConfig->startSecAddr[i + 1U] + EE_SECTOR_ACTIND_OFFSET))
                {
                    ers_cyc_val = i;
                }
            }
        }
    }

    /* Rotate the live sector list so the newest active sector becomes the base slot. */
    Eed_ShiftIdxToBase(eepromConfig, ers_cyc_val);

    if (0xFFFFFFFFU == READ32(eepromConfig->startSecAddr[EE_ACTIVE_SECTORS - 0x1U] +
                                    EE_SECTOR_ACTIND_OFFSET))
    {
        eepromConfig->eraseCycValue = 0x1U;

        /* Cold-start path: initialize every live sector before activating the base slot. */
        for (i = 0x0U; i <= eepromConfig->maxValidIndex; i++)
        {
            do
            {
                ret_val = Eed_SyncValidateSector(eepromConfig, eepromConfig->eraseCycValue, i);
            } while (STATUS_EdPROM_MAKE_DEAD_SUCCESS == ret_val);

            if (STATUS_EdPROM_MAKE_DEAD_ERROR == ret_val)
            {
                break;
            }
        }

        if (STATUS_EdPROM_MAKE_DEAD_ERROR != ret_val)
        {
            do
            {
                ret_val = Eed_ValidateActIndicator(eepromConfig, (uint8_t)(EE_ACTIVE_SECTORS - 1U));
            } while (STATUS_EdPROM_MAKE_DEAD_SUCCESS == ret_val);
        }
    }
    else
    {
        /* Recovery path 1: repair the sector immediately after the active window if a swap was interrupted. */
        ret_val = Eed_FlashEraseVerify(eepromConfig->startSecAddr[EE_ACTIVE_SECTORS] + EE_SECTOR_ERSCYC_OFFSET + EE_INDICATOR_SIZE,
                                  (EE_SECTOR_SIZE - EE_INDICATOR_SIZE) >> 2);
        if (STATUS_SUCCESS != ret_val)
        {
            ers_cyc_val = READ32(eepromConfig->startSecAddr[EE_ACTIVE_SECTORS - 1U] + EE_SECTOR_ERSCYC_OFFSET);

            do
            {
                ret_val = Eed_SyncValidateSector(eepromConfig, ers_cyc_val, EE_ACTIVE_SECTORS);
            } while (STATUS_EdPROM_MAKE_DEAD_SUCCESS == ret_val);

            if (STATUS_EdPROM_MAKE_DEAD_ERROR == ret_val)
            {
                return ret_val;
            }
        }

        /* Recovery path 2: repair the tail sector when the previous oldest active sector is inconsistent. */
        if (active_num >= EE_ACTIVE_SECTORS)
        {
            i = eepromConfig->maxValidIndex;

            ers_cyc_val = READ32(eepromConfig->startSecAddr[0] + EE_SECTOR_ERSCYC_OFFSET);

            ret_val = Eed_FlashEraseVerify(eepromConfig->startSecAddr[i] + EE_SECTOR_ERSCYC_OFFSET + EE_INDICATOR_SIZE,
                                      (EE_SECTOR_SIZE - EE_INDICATOR_SIZE) >> 2);
            if (ret_val == STATUS_SUCCESS)
            {
                eepromConfig->eraseCycValue = READ32(eepromConfig->startSecAddr[i] + EE_SECTOR_ERSCYC_OFFSET);

                if ((eepromConfig->eraseCycValue != ers_cyc_val) &&
                    (eepromConfig->eraseCycValue != (ers_cyc_val + 1)))
                {
                    ret_val = STATUS_EdPROM_PROG_VERIFY_ERROR;
                }
            }

            if (ret_val != STATUS_SUCCESS)
            {
                do
                {
                    ret_val = Eed_SyncValidateSector(eepromConfig, (ers_cyc_val + 1), i);
                } while (STATUS_EdPROM_MAKE_DEAD_SUCCESS == ret_val);

                if (STATUS_EdPROM_MAKE_DEAD_ERROR == ret_val)
                {
                    return ret_val;
                }
            }
        }
    }

    if (STATUS_EdPROM_MAKE_DEAD_ERROR != ret_val)
    {
        /* Publish the recovered blank slot and erase-cycle state only after successful initialization. */
        eepromConfig->blankSpace = Eed_SearchBlankSpace(eepromConfig->startSecAddr[EE_ACTIVE_SECTORS - 1U]);
        eepromConfig->eraseCycValue = READ32(eepromConfig->startSecAddr[EE_ACTIVE_SECTORS - 1U] +
                                                      EE_SECTOR_ERSCYC_OFFSET);
        ret_val = STATUS_SUCCESS;
    }
    return ret_val;
}

/*******************************************************************************
 * Sequence Maintenance
 ******************************************************************************/

/*!
 * @brief Rotate the sector-address array so a selected sector becomes the base
 *        active position.
 */
void Eed_ShiftIdxToBase(eeprom_config_t * eepromConfig, uint32_t current_idx)
{
    uint32_t temp;
    uint32_t i;
    uint32_t size;
    uint32_t j;

    if (current_idx >= (EE_ACTIVE_SECTORS - 0x1U))
    {
        size = current_idx - (uint8_t)(EE_ACTIVE_SECTORS - 0x1U);
    }else
    {
        size = eepromConfig->maxValidIndex + 0x1U - (uint8_t)(EE_ACTIVE_SECTORS - 0x1U) + current_idx;
    }

    for (i = 0x0U; i < size; i++)
    {
        temp = eepromConfig->startSecAddr[0U];
        for (j = 0x0U; j < eepromConfig->maxValidIndex; j++)
        {
            eepromConfig->startSecAddr[j] = eepromConfig->startSecAddr[j+1U];
        }
        eepromConfig->startSecAddr[j] = temp;
    }
}

/*!
 * @brief Move dead sectors to the end of the live sector-address array.
 */
status_t Eed_RemoveDeadSector(eeprom_config_t * eepromConfig)
{
    uint32_t i = 0x0U;
    uint32_t j;

    /* Compact live sectors toward the front of the address list. */
    while (i <= eepromConfig->maxValidIndex)
    {
        if (0xFFFFFFFFU != READ32(eepromConfig->startSecAddr[i] + EE_SECTOR_DEADIND_OFFSET))
        {
            for (j = i; j < eepromConfig->maxValidIndex; j++)
            {
                eepromConfig->startSecAddr[j] = eepromConfig->startSecAddr[j + 1U];
            }

            eepromConfig->maxValidIndex --;
        }
        else
        {
            i ++;
        }
    }

    if (EE_EXTRA_READY_SECTORS < /*lint -e(9032)*/ (EE_ALLOCATED_SECTORS - (eepromConfig->maxValidIndex + 0x1U)))
    {
        return STATUS_EdPROM_MAKE_DEAD_ERROR;
    }
    return STATUS_SUCCESS;
}

/*!
 * @brief Clear or rebuild cached record locations.
 */
void Eed_UpdateCacheTable(eeprom_config_t * eepromConfig, uint8_t init_flag)
{
    uint32_t rec_addr = 0xFFFFFFFFU;
    uint16_t data_id;

    for (data_id = 0x0U; data_id < EE_CACHE_TABLE_MAX_ENTRY; data_id++)
    {
        if (true != init_flag)
        {
            rec_addr = Eed_SearchInAllActives(eepromConfig, data_id);
        }
        s_EE_CacheTable[data_id] = rec_addr;
    }
}
