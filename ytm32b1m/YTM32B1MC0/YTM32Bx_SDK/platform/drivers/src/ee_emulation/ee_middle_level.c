/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ee_emulation.h"
#include "ee_hardware.h"

static uint32_t s_EE_CacheTable[EE_CACHE_TABLE_MAX_ENTRY];

/******************************************************************************
Function Name    : Eed_CopyData
Description      : This function is to copy all valid data from oldest active
                   sector to new sector.
Arguments        :
                   - eepromConfig: EEPROM configuration.
Return Value     :
                   - STATUS_EdPROM_MAKE_DEAD_SUCCESS: copy data fail and can do copy again from beginning.
                   - STATUS_EdPROM_MAKE_DEAD_ERROR: copy data fail and erase sector fail and CANNOT make it to dead.
                   - STATUS_SUCCESS: copy all data to new sector OK.
*******************************************************************************/
status_t Eed_CopyData(eeprom_config_t * eepromConfig)
{
    uint32_t    rec_addr; /* record address */
    uint32_t    dest;
    uint32_t    end_search_addr;
    uint32_t    temp;
    uint16_t    data_id;
    uint16_t    status;
    status_t     ret_val = STATUS_SUCCESS;

    /* point to last record in old active sector */
    rec_addr = eepromConfig->startSecAddr[0] + EE_SECTOR_SIZE - EE_SECTOR_WASTE - EE_RECORD_LENGTH;
    /* point to first record in sector*/
    end_search_addr = eepromConfig->startSecAddr[0] + EE_SECTOR_HEADER_SIZE;
    /* update blank space to new sector */
    dest = eepromConfig->startSecAddr[EE_ACTIVE_SECTORS] + EE_SECTOR_HEADER_SIZE;
    /* Loop through the records of the oldest ACTIVE sector to copy*/
    while (rec_addr >= end_search_addr)
    {
        /* only read record ID if record status is VALID*/
        temp = READ32(rec_addr + EE_RECORD_STATUS_OFFSET);
        status = (uint16_t)(temp >> 0x10U);
        if (EE_RECORD_STATUS_VALID == status)
        {
            data_id = (uint16_t)temp;
            /* search this ID in all active sectors from bottom up and from newest to oldest one */
            if (rec_addr == Eed_SearchLoop(eepromConfig, data_id))
            {
                /* Means this is the latest ID and need to copy it,*/
                if (STATUS_SUCCESS != Eed_FlashProgram(dest, EE_RECORD_LENGTH, rec_addr))
                {
                    /* fails in copying this record*/
                    /* erase this sector and program erase cycle and copy again*/
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
        /* Point to next record */
        rec_addr -= EE_RECORD_LENGTH;
    }

    /* Update blank space */
    eepromConfig->blankSpace = dest;
    return (ret_val);
}

/******************************************************************************
Function Name    : Eed_WriteRecord
Description      : This function is to write new data to current blank space of
                   EEPROM system.
Arguments        :
                   - eepromConfig: EEPROM configuration.
                   - source: source data to write.
                   - data_id: data ID to write.
Return Value     :
                   - STATUS_SUCCESS: write record successfully.
                   - STATUS_EdPROM_PROG_VERIFY_ERROR: write record unsuccessfully due to verify error.
*******************************************************************************/
status_t Eed_WriteRecord(eeprom_config_t * eepromConfig, uint32_t source, uint16_t data_id)
{
   uint32_t data;
#if (EE_DATA_VALUE_SIZE & EE_LONG_WORD_MASK)
    uint8_t i;
    uint8_t    remain_data[4] = {0xFFU,0xFFU,0xFFU,0xFFU};
#endif
    status_t    ret_val = STATUS_SUCCESS;

    /* program aligned part first*/
#if EE_DATA_VALUE_ALIGNED_PART > 0U
    ret_val = Eed_FlashProgram(eepromConfig->blankSpace, EE_DATA_VALUE_ALIGNED_PART, source);
#endif

#if (EE_DATA_VALUE_SIZE & EE_LONG_WORD_MASK)
    source += EE_DATA_VALUE_ALIGNED_PART;
    for ( i = 0U; i < (EE_DATA_VALUE_SIZE & EE_LONG_WORD_MASK); i ++)
    {
        /* reuse data variable here*/
        remain_data[i] = READ8(source);
        source ++;
    }
    ret_val |= Eed_FlashProgram(eepromConfig->blankSpace + EE_DATA_VALUE_ALIGNED_PART, FTM_LONGWORD_SIZE, (uint32_t)remain_data);
#endif
    /* write new data, need to program ID and over-program status*/
    if (STATUS_SUCCESS == ret_val)
    {
        data = 0x55550000U | data_id;
        ret_val = Eed_FlashProgram(eepromConfig->blankSpace + (EE_RECORD_LENGTH - EE_RECORD_HEADER_SIZE), EE_RECORD_HEADER_SIZE, (uint32_t)&data);
            if ((data_id < EE_CACHE_TABLE_MAX_ENTRY) && (STATUS_SUCCESS == ret_val))
            {
                /* update cache table */
                s_EE_CacheTable[data_id] = eepromConfig->blankSpace;
            }
    }

    eepromConfig->blankSpace += EE_RECORD_LENGTH;
    return (ret_val);
}

/******************************************************************************
Function Name    : Eed_SearchBlankSpace
Description      : This function is to find blank space of EEPROM system.
Arguments        :
                   - sec_addr: start address of current active sector.
Return Value     : blank space address of this sector.
*******************************************************************************/
uint32_t Eed_SearchBlankSpace(uint32_t sec_addr)
{
    uint32_t search_addr;
#if EE_CALLBACK_ENABLE
    uint32_t counter = 0x0U;
#endif
    search_addr = sec_addr + EE_SECTOR_SIZE - EE_SECTOR_WASTE;
    /* point to first record address in sector */
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
        /* Check if need to serve callback function */
        Eed_CallBack(&counter);
#endif
    }while (search_addr > sec_addr);
    return (sec_addr);
}

/******************************************************************************
Function Name    : Eed_SearchInAllActives
Description      : This function is to search an ID in all active sectors.
Arguments        :
                   - eepromConfig: EEPROM configuration.
                   - data_id: record ID to be searched.
Return Value     : address of found data. If not found, return 0xFFFFFFFF.
*******************************************************************************/
/*lint -e{818, 850} */
uint32_t Eed_SearchInAllActives(eeprom_config_t * eepromConfig, uint16_t data_id)
{
    uint32_t rec_addr;
    uint32_t start_rec_addr;
    uint32_t found_addr = 0xFFFFFFFFU; /* found address */
    uint32_t rec_head_val; /* data at record header */
    #if EE_CALLBACK_ENABLE
    uint32_t counter = 0U;
    #endif
    int32_t i = (int32_t)EE_ACTIVE_SECTORS;
    /* for safety, use rec_head_val 32 bits to store a 32 bit value, don't cast data_id to 32 bit type*/
    rec_head_val = EE_RECORD_STATUS_VALID;
    rec_head_val = (rec_head_val << 16U) | data_id;

    rec_addr = eepromConfig->blankSpace;
    for (i = (EE_ACTIVE_SECTORS - 1U); i >= 0; i--)
    {
        /* calculate the first record address in this sector */
        start_rec_addr = eepromConfig->startSecAddr[i];
        if (0xFFFFFFFFU == READ32(start_rec_addr + EE_SECTOR_ACTIND_OFFSET))
        {
            /* break if this is not active sector */
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
            /* Check if need to serve callback function */
            Eed_CallBack(&counter);
#endif
        }while (rec_addr > start_rec_addr);
        /* point to blank space of previous active sector */
        if (i >= 0x1)
        {
            rec_addr = eepromConfig->startSecAddr[i - 0x1] + EE_SECTOR_SIZE - EE_SECTOR_WASTE;
        }
    }
    return (found_addr);
}

/******************************************************************************
Function Name    : Eed_SearchInAllActives
Description      : This function is to search an ID in entire EEPROM system
                   including in cache table.
Arguments        :
                   - eepromConfig: EEPROM configuration.
                   - data_id: record ID to be searched.
Return Value     : address of found data. If not found, return 0xFFFFFFFF.
*******************************************************************************/
uint32_t Eed_SearchLoop (eeprom_config_t * eepromConfig, uint16_t data_id)
{
    /* Check if cache table is enabled and data is in valid range of cache, search in cache first */
    uint32_t rec_addr = 0xFFFFFFFFU;
    if (EE_CACHE_TABLE_MAX_ENTRY > data_id)
    {
        rec_addr = s_EE_CacheTable[data_id];
    }
    if (rec_addr == 0xFFFFFFFFU)
    {
        /* ID is not in the cache table, need to search in all active sectors */
        rec_addr = Eed_SearchInAllActives(eepromConfig, data_id);
    }
    return (rec_addr);
}

/******************************************************************************
Function Name    : Eed_CallBack
Description      : This function is to perform call back function.
Arguments        :
                   - counter:
Return Value     : None.
*******************************************************************************/
#if (EE_CALLBACK_ENABLE)
void Eed_CallBack(uint32_t* counter)
{
    uint32_t temp;
    temp = *counter;
    if (((temp++) >= EE_CALLBACK_COUNTER) && (EE_NULL_CALLBACK != p_gEECallBack))
    {
        p_gEECallBack();
        /* Reset counter */
        temp = 0x0U;

    }
  *counter = temp;
}
#endif

/******************************************************************************
Function Name    : Eed_ValidateDeadIndicator
Description      : This function is to make a sector as DEAD.
Arguments        :
                   - eepromConfig: EEPROM configuration.
                   - index: index of sector in valid round robin to be DEAD.
Return Value     :
                   - STATUS_EdPROM_MAKE_DEAD_ERROR: cannot make this sector as DEAD.
                   - STATUS_EdPROM_MAKE_DEAD_SUCCESS: make this sector as DEAD successfully.
*******************************************************************************/
status_t Eed_ValidateDeadIndicator(eeprom_config_t * eepromConfig, uint32_t index)
{
    status_t ret_val = STATUS_EdPROM_MAKE_DEAD_ERROR;

    if (0xFFFFFFFFU != Eed_OverProgram(eepromConfig->startSecAddr[index] + EE_SECTOR_DEADIND_OFFSET))
    {
        /* Arrange the index to move all dead sector to end of sequence */
        ret_val = Eed_RemoveDeadSector(eepromConfig);
        if (ret_val == STATUS_SUCCESS)
        {
            return STATUS_EdPROM_MAKE_DEAD_SUCCESS;
        }
    }
    return (ret_val);
}

/******************************************************************************
Function Name    : Eed_SyncValidateSector
Description      : This function is to erase and then program erase cycle to a
                   sector for several times if fails.
Arguments        :
                   - eepromConfig: EEPROM configuration.
                   - expErsVal: expected erase cycle of this sector.
                   - index: index of sector in valid round robin to be erased.
Return Value     :
                   - STATUS_SUCCESS: validate sector (erase sector and program erase cycle)
                     successfully.
                   - STATUS_EdPROM_MAKE_DEAD_ERROR: validate sector unsuccessfully and then make it
                     to DEAD unsuccessfully.
                   - STATUS_EdPROM_MAKE_DEAD_SUCCESS: validate sector unsuccessfully and then make it
                     to DEAD successfully.
*******************************************************************************/
status_t Eed_SyncValidateSector(eeprom_config_t * eepromConfig, uint32_t expErsVal, uint32_t index)
{
    status_t ret_val;
    uint8_t re_ers_num = 0x0U;

    do
    {
        /* Erase the sector */
        ret_val = Eed_FlashErase(eepromConfig->startSecAddr[index]);
        if (STATUS_SUCCESS == ret_val)
        {
            /* Program erase cycle */
            ret_val = Eed_FlashProgram(eepromConfig->startSecAddr[index] + EE_SECTOR_ERSCYC_OFFSET,
                                       EE_INDICATOR_SIZE,
                                       (uint32_t)&expErsVal);
        }
        re_ers_num++;
    }while ((STATUS_SUCCESS != ret_val) && (re_ers_num <= EE_REERASE_NUM));

    /* Make the sector to DEAD state if failed to make it alter state */
    if (STATUS_SUCCESS != ret_val)
    {
        ret_val = Eed_ValidateDeadIndicator(eepromConfig, index);
    }
    return (ret_val);
}

/******************************************************************************
Function Name    : Eed_ValidateActIndicator
Description      : This function is to program data to active indicator to make it
                   as ACTIVE. If fails, just make this sector as DEAD.
Arguments        :
                   - eepromConfig: EEPROM configuration.
                   - index: index of sector in valid round robin to be programmed.
Return Value     :
                   - STATUS_SUCCESS: program data to active indicator successfully.
                   - STATUS_EdPROM_MAKE_DEAD_ERROR: program data unsuccessfully and then make it
                     to DEAD unsuccessfully.
                   - STATUS_EdPROM_MAKE_DEAD_SUCCESS: validate sector unsuccessfully and then make it
                     to DEAD successfully.
*******************************************************************************/
status_t Eed_ValidateActIndicator(eeprom_config_t * eepromConfig, uint8_t index)
{
    status_t ret_val = STATUS_SUCCESS;
    if (0xFFFFFFFFU == Eed_OverProgram(eepromConfig->startSecAddr[index] + EE_SECTOR_ACTIND_OFFSET))
    {
        ret_val = Eed_ValidateDeadIndicator(eepromConfig, index);
    }
    return (ret_val);
}

/******************************************************************************
Function Name    : Eed_OverProgram
Description      : This function is to do over program data to the same destination.
Arguments        :
                   - dest: destination to program.
                   - index: index of sector in valid round robin to be programmed.
Return Value     :data value at destination after being programmed.
*******************************************************************************/
uint32_t Eed_OverProgram(uint32_t dest)
{
    uint32_t data = EE_DATA_INDICATOR;
    uint32_t act_val;           /* Actual value */
    uint8_t try_num = 0x0U;     /* Number of time to re-try the operation */
    do
    {
        try_num++;
        Eed_FlashProgram(dest, EE_INDICATOR_SIZE, (uint32_t)&data);
        act_val = READ32(dest);
    }while ((try_num <= EE_REPROGRAM_NUM) && (act_val == 0xFFFFFFFFU));
    return (act_val);
}

/******************************************************************************
Function Name    : Eed_InitAllSectors
Description      : This function is to initialize all sectors and allocate the
                   current active sector at index of (EE_ACTIVE_SECTORS - 1).
Arguments        :
                   - eepromConfig: EEPROM configuration.
Return Value     :
                   - STATUS_SUCCESS: all sectors are intialized successfully.
                   - STATUS_EdPROM_MAKE_DEAD_ERROR: sector is initialized unsuccessfully and then
                     make it to DEAD unsuccessfully.
*******************************************************************************/
status_t Eed_InitAllSectors(eeprom_config_t * eepromConfig)
{
    uint32_t ers_cyc_val;
    uint32_t active_num = 0U;
    status_t ret_val = STATUS_SUCCESS;
    uint32_t i;

    /* Find the newest active index and number of active sector
       ======================================================== */

    /* Use 'ers_cyc_val' as a temp variable to store newest active index */
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

    /* Shift all active sectors to beginning of sequence */
    Eed_ShiftIdxToBase(eepromConfig, ers_cyc_val);

    /* If base sector (at the 'EE_ACTIVE_SECTORS - 1' index) is not active,
        mean there is no active sector in entire EEPROM system.
        -> validate all sectors and then make the base sector as active */
    if (0xFFFFFFFFU == READ32(eepromConfig->startSecAddr[EE_ACTIVE_SECTORS - 0x1U] +
                                    EE_SECTOR_ACTIND_OFFSET))
    {
        /* Init erase cycle value */
        eepromConfig->eraseCycValue = 0x1U;

        /* Make all sectors to alter by erase and write the initial erase cycle value */
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

        /* Make the base sector to active until successfully or until cannot make it to dead*/
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
        /* If there is at least 1 active sector in entire EEPROM sytem, there is 2 possible sectors become
           invalid. These are: */

        /* 1 - The sector which is right after base sector (may be new sector in swapping progress)
           ======================================================================================= */

        /* Get state of the sector (Alternative or Copying) */
        ret_val = Eed_FlashEraseVerify(eepromConfig->startSecAddr[EE_ACTIVE_SECTORS] + EE_SECTOR_ERSCYC_OFFSET + EE_INDICATOR_SIZE,
                                  (EE_SECTOR_SIZE - EE_INDICATOR_SIZE) >> 2);
        if (STATUS_SUCCESS != ret_val)
        {
            /* Get erase cycle of the base sector */
            ers_cyc_val = READ32(eepromConfig->startSecAddr[EE_ACTIVE_SECTORS - 1U] + EE_SECTOR_ERSCYC_OFFSET);

            /* Make the sector to Alternative */
            do
            {
                ret_val = Eed_SyncValidateSector(eepromConfig, ers_cyc_val, EE_ACTIVE_SECTORS);
            } while (STATUS_EdPROM_MAKE_DEAD_SUCCESS == ret_val);

            if (STATUS_EdPROM_MAKE_DEAD_ERROR == ret_val)
            {
                return ret_val;
            }
        }

        /* 2 - Sector at end of sequence (may be an odest active sector in previous step)
           ============================================================================== */
        if (active_num >= EE_ACTIVE_SECTORS)
        {
            /* Get index of the sector */
            i = eepromConfig->maxValidIndex;

            /* Read erase cycle of the base sector */
            ers_cyc_val = READ32(eepromConfig->startSecAddr[0] + EE_SECTOR_ERSCYC_OFFSET);

            /* Get state of the sector (Alternative or Erasing) */
            ret_val = Eed_FlashEraseVerify(eepromConfig->startSecAddr[i] + EE_SECTOR_ERSCYC_OFFSET + EE_INDICATOR_SIZE,
                                      (EE_SECTOR_SIZE - EE_INDICATOR_SIZE) >> 2);
            if (ret_val == STATUS_SUCCESS)
            {
                /* Read erase cycle of the sector */
                eepromConfig->eraseCycValue = READ32(eepromConfig->startSecAddr[i] + EE_SECTOR_ERSCYC_OFFSET);

                if ((eepromConfig->eraseCycValue != ers_cyc_val) &&
                    (eepromConfig->eraseCycValue != (ers_cyc_val + 1)))
                {
                    ret_val = STATUS_EdPROM_PROG_VERIFY_ERROR;
                }
            }

            /* Make the sector to Alternative */
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

    /* In case ret_val = STATUS_EdPROM_MAKE_DEAD_SUCCESS, just update it to STATUS_SUCCESS
       Don't update blank space and erase cycle if ret_val = STATUS_EdPROM_MAKE_DEAD_ERROR */
    if (STATUS_EdPROM_MAKE_DEAD_ERROR != ret_val)
    {
        eepromConfig->blankSpace = Eed_SearchBlankSpace(eepromConfig->startSecAddr[EE_ACTIVE_SECTORS - 1U]);
        eepromConfig->eraseCycValue = READ32(eepromConfig->startSecAddr[EE_ACTIVE_SECTORS - 1U] +
                                                      EE_SECTOR_ERSCYC_OFFSET);
        ret_val = STATUS_SUCCESS;
    }
    return ret_val;
}

/******************************************************************************
Function Name    : Eed_ShiftIdxToBase
Description      : This function is to shift the current index to base index of
                   (EE_ACTIVE_SECTORS - 1).
Arguments        :
                   - eepromConfig: EEPROM configuration.
                   - current_idx: the newest active index.
Return Value     : None.
*******************************************************************************/
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

/******************************************************************************
Function Name    : Eed_RemoveDeadSector
Description      : This function is to allocate all DEAD sector to end of sequence.
Arguments        :
                   - eepromConfig: EEPROM configuration.
Return Value     : None.
*******************************************************************************/
status_t Eed_RemoveDeadSector(eeprom_config_t * eepromConfig)
{
    uint32_t i = 0x0U;
    uint32_t j;

    /* Remove dead sector out of the eeprom config */
    while(i <= eepromConfig->maxValidIndex)
    {
        if (0xFFFFFFFFU != READ32(eepromConfig->startSecAddr[i] + EE_SECTOR_DEADIND_OFFSET))
        {
            /* Found dead sector -> shift address to move dead sector to end of sequence*/
            for (j = i; j < eepromConfig->maxValidIndex; j++)
            {
                eepromConfig->startSecAddr[j] = eepromConfig->startSecAddr[j + 1U];
            }

            /* Decrease maxValidIndex */
            eepromConfig->maxValidIndex --;
        }
        else
        {
            i ++;
        }
    }

    /* Check number of sectors for operating properly */
    if (EE_EXTRA_READY_SECTORS < /*lint -e(9032)*/ (EE_ALLOCATED_SECTORS - (eepromConfig->maxValidIndex + 0x1U)))
    {
        return STATUS_EdPROM_MAKE_DEAD_ERROR;
    }
    return STATUS_SUCCESS;
}

/******************************************************************************
Function Name    : Eed_UpdateCacheTable
Description      : This function is to update cache table.
Arguments        :
                   - eepromConfig: EEPROM configuration.
                   - init_flag: indicate to clear all cache item or not.
Return Value     : None.
*******************************************************************************/
/* Initialize the cache table with all the values, if it is enabled */
void Eed_UpdateCacheTable(eeprom_config_t * eepromConfig, uint8_t init_flag)
{
    uint32_t rec_addr = 0xFFFFFFFFU;
    uint16_t data_id;

    /* Loop through the data ID that should be written to the Cache Table */
    for (data_id = 0x0U; data_id < EE_CACHE_TABLE_MAX_ENTRY; data_id++)
    {
        /* Search all the ACTIVE sectors */
        if (true != init_flag)
        {
            rec_addr = Eed_SearchInAllActives(eepromConfig, data_id);
        }
        /* Write the data ID to the cache table */
        s_EE_CacheTable[data_id] = rec_addr;
    }
}
