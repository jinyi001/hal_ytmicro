/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "ee_emulation.h"
#include "ee_hardware.h"

/******************************************************************************
*                             Global variables                                *
*******************************************************************************/
#if EE_CALLBACK_ENABLE
/* A function pointer to the CallBack function */
ee_callback_t             g_EECallBack = EE_NULL_CALLBACK;
#endif



/******************************************************************************
Function Name    : Eed_InitEeprom
Description      : This function is to initialize EEPROM system including:
                   - remove dead sector from EEPROM system and determine maximum
                     valid index in array of start sector address after that.
                   - Initialize all sectors by erasing the "possible-error-sector".
                   - Initialize and update cache table if enabled.
Arguments        :
                   - eepromConfig: EEPROM configuration.
Return Value     :
                   - STATUS_SUCCESS: all sectors are initialized successfully.
                   - STATUS_EdPROM_MAKE_DEAD_SUCCESS: sector is initialized unsuccessfully and then
                     make it to DEAD successfully.
                   - STATUS_EdPROM_MAKE_DEAD_ERROR: sector is initialized unsuccessfully and then
                     make it to DEAD unsuccessfully.
*******************************************************************************/
status_t Eed_InitEeprom(eeprom_config_t * eepromConfig)
{
    status_t ret_val;

    /* Init maxValidIndex value */
    eepromConfig->maxValidIndex = (uint32_t)(EE_ALLOCATED_SECTORS - 0x1U);

    /* Arrange sector index to allocate dead sector to end of sequence */
    ret_val = Eed_RemoveDeadSector(eepromConfig);

    if (ret_val == STATUS_SUCCESS)
    {
        /* Initialize all sectors */
        ret_val = Eed_InitAllSectors(eepromConfig);
    }

    if (STATUS_SUCCESS == ret_val)
    {
        /* Clear cache */
        Eed_UpdateCacheTable (eepromConfig, true);
        /* Update cache */
        Eed_UpdateCacheTable (eepromConfig, false);
    }

    return (ret_val);
}

/******************************************************************************
Function Name    : Eed_ReadEeprom
Description      : This function is used to read the specific data record.
                   The starting address of the record data will be returned.
Arguments        :
                   - eepromConfig: EEPROM configuration.
                   - data_id: the record ID to be read.
                   - rec_addr: address of found record.
Return Value     :
                   - STATUS_SUCCESS: found the expected ID.
                   - STATUS_EdPROM_ID_NOT_FOUND: cannot find the expected ID.
*******************************************************************************/
status_t Eed_ReadEeprom (eeprom_config_t * eepromConfig, uint16_t data_id, uint8_t *pData)
{
    status_t ret_val = STATUS_SUCCESS;
    uint32_t rec_addr;
    uint32_t i;
   /* Check if the ID is in the range or equal to zero */
    if (EE_MAX_RECORD_NUMBER <= data_id)
    {
        ret_val = STATUS_EdPROM_ID_OUT_OF_RANGE;
    }
    else
    {
        /* Search data for the record in all active sectors*/
        rec_addr = Eed_SearchLoop(eepromConfig, data_id);
        if (rec_addr == 0xFFFFFFFFU)
        {
            ret_val = STATUS_EdPROM_ID_NOT_FOUND;
        }
        /* Copy data to the pointer */
        else
        {
            for(i = 0; i < EE_DATA_VALUE_SIZE; i++)
            {
                *(pData + i) = *(uint8_t *)(rec_addr + i);
            }
        }
    }
    return (ret_val);
}

/******************************************************************************
Function Name    : Eed_WriteEeprom
Description      : This function will be used to write new data record to
                   EEPROM emulation. Swapping may occur if there is not enough
                   free space. If write a record fails, this function will re-write
                   it in next address.
Arguments        :
                   - eepromConfig: EEPROM configuration.
                   - data_id: the record ID to be read.
                   - pData: source data to write record.
Return Value     :
                   - STATUS_SUCCESS: Write EEPROM operation success.
                   - STATUS_EdPROM_ID_NOT_FOUND: cannot find the expected ID.
*******************************************************************************/
status_t Eed_WriteEeprom (eeprom_config_t * eepromConfig, uint16_t data_id, const uint8_t* pData)
{
    uint32_t count = 0x0U;
    uint32_t tmp;
    status_t ret_val;

    /* Check if the ID is within the range or equal to zero */
    if (EE_MAX_RECORD_NUMBER <= data_id)
    {
        return STATUS_EdPROM_ID_OUT_OF_RANGE;
    }

    /* In case writing fails continuously, just terminate function after EE_MAX_WRITE_LOOP times
       to avoid unless loop */
    while(count < EE_MAX_WRITE_LOOP)
    {
        /* If no free space anymore */
        if (eepromConfig->blankSpace == (eepromConfig->startSecAddr[EE_ACTIVE_SECTORS - 1U] +
            EE_SECTOR_SIZE - EE_SECTOR_WASTE))
        {
            /* If the first sector is active, it means there is no more sector can be made as active
               and swapping must occur */
            if (0xFFFFFFFFU != READ32(eepromConfig->startSecAddr[0U] + EE_SECTOR_ACTIND_OFFSET))
            {
                /* Backup blank space before copying */
                tmp = eepromConfig->blankSpace;

                /* ret_val = STATUS_EdPROM_MAKE_DEAD_SUCCESS: restore blank space to the previous sector
                   ret_val = STATUS_EdPROM_MAKE_DEAD_ERROR: don't execute later steps and terminate function */
                ret_val = Eed_CopyData(eepromConfig);
                if (STATUS_SUCCESS == ret_val)
                {
                    /* ret_val = STATUS_EdPROM_MAKE_DEAD_SUCCESS: restore blank space to the previous sector
                       ret_val = STATUS_EdPROM_MAKE_DEAD_ERROR: don't execute later steps and terminate function */
                    ret_val = Eed_ValidateActIndicator(eepromConfig, (uint8_t)EE_ACTIVE_SECTORS);
                    if (STATUS_SUCCESS == ret_val)
                    {
                        /* Erase and update erase cycle for the oldest active cluster */
                        tmp = READ32(eepromConfig->startSecAddr[0U] + EE_SECTOR_ERSCYC_OFFSET) + 0x1U;
                        /* ret_val = STATUS_EdPROM_MAKE_DEAD_SUCCESS: don't execute later steps and back to while() loop
                           ret_val = STATUS_EdPROM_MAKE_DEAD_ERROR: don't execute later steps and terminate function */
                        ret_val = Eed_SyncValidateSector(eepromConfig, tmp, 0U);
                        if (STATUS_SUCCESS == ret_val)
                        {
                            /* Shift and update cache table */
                            Eed_ShiftIdxToBase(eepromConfig, (uint8_t)EE_ACTIVE_SECTORS);
                            Eed_UpdateCacheTable(eepromConfig, false);
                        }
                        if (STATUS_EdPROM_MAKE_DEAD_SUCCESS == ret_val)
                        {
                            ret_val = STATUS_SUCCESS;
                        }
                    }
                }

                /* Restore blank space to the previous sector if the steps ('Eed_CopyData' and
                   'Eed_ValidateActIndicator') return STATUS_EdPROM_MAKE_DEAD_SUCCESS -> Re swapping in the next loop */
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

                /* Update blank space to new sector -> write the new record in the next loop */
                eepromConfig->blankSpace = eepromConfig->startSecAddr[EE_ACTIVE_SECTORS] + EE_SECTOR_HEADER_SIZE;
                Eed_ShiftIdxToBase(eepromConfig, (uint8_t)EE_ACTIVE_SECTORS);
            }

            /* STATUS_EdPROM_MAKE_DEAD_ERROR handling */
            if (STATUS_EdPROM_MAKE_DEAD_ERROR == ret_val)
            {
                return STATUS_EdPROM_MAKE_DEAD_ERROR;
            }
        }
        else
        {
            /* Write data and update cache table */
            ret_val = Eed_WriteRecord(eepromConfig, (uint32_t)pData, data_id);
            if (STATUS_SUCCESS == ret_val)
            {
                /* Update erase cycle and then exit from while loop */
                eepromConfig->eraseCycValue = READ32(eepromConfig->startSecAddr[EE_ACTIVE_SECTORS - 1U] +
                                                    EE_SECTOR_ERSCYC_OFFSET);
                break;
            }
        }
        count ++;
    }
    return (ret_val);
}


/******************************************************************************
Function Name    : Eed_DeinitEeprom
Description      : This function is to release all the Flash used to
                   EEPROM emulation. After de-initialized, the Flash
                   for emulation will be fully erased.
Arguments        :
                   - eepromConfig: EEPROM configuration.
Return Value     :
                   - STATUS_SUCCESS: all sectors are erased successfully.
                   - STATUS_EdPROM_SECTOR_ERASE_ERROR: fails in erasing a sector.
*******************************************************************************/
/*lint -e{818} */
status_t Eed_DeinitEeprom (eeprom_config_t * eepromConfig)
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
