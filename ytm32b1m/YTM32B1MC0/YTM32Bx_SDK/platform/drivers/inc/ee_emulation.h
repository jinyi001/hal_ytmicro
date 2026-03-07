/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EE_EMULATION_H_
#define _EE_EMULATION_H_

#include <stdbool.h>
#include <stddef.h>
#include "status.h"
#include "device_registers.h"
#include "ee_user_config.h"

/*!
 *
 * @ingroup ee_emulation
 * @details This section describes the EEPROM api in ee_emulation module Driver.
 * @{
 */

/*!
 * @addtogroup ee_emulation_driver
 * @{
 */

/******************************************************************************/
/*                            Macros related to a record                      */
/******************************************************************************/
/* macros relating to size of each element*/
#define EE_RECORD_HEADER_SIZE           (FEATURE_EFM_WRITE_UNIT_SIZE)
/* macros relating to offset of each element*/
#define EE_RECORD_STATUS_OFFSET         (EE_RECORD_LENGTH - EE_RECORD_HEADER_SIZE)

/******************************************************************************/
/*                            Macros related to a sector                      */
/******************************************************************************/
/* macros relating to size of each element*/
#define EE_INDICATOR_SIZE               (FEATURE_EFM_WRITE_UNIT_SIZE)
#define EE_SECTOR_HEADER_SIZE           (EE_INDICATOR_SIZE * 3) /* sector header = 3*EE_INDICATOR_SIZE */
#if FEATURE_EFM_HAS_DATA_FLASH
#define EE_SECTOR_SIZE                  FEATURE_EFM_DATA_ARRAY_SECTOR_SIZE
#else
#define EE_SECTOR_SIZE                  FEATURE_EFM_MAIN_ARRAY_SECTOR_SIZE
#endif
/* macros relating to offset of each element*/
#define EE_SECTOR_ACTIND_OFFSET         (EE_INDICATOR_SIZE * 2) /* active indicator */
#define EE_SECTOR_DEADIND_OFFSET        (EE_INDICATOR_SIZE * 1) /* dead indicator */
#define EE_SECTOR_ERSCYC_OFFSET         (EE_INDICATOR_SIZE * 0) /* erase cycle */

/* bit mask of long word (4 bytes) size*/
#define EE_LONG_WORD_MASK              (0x3U)

#define EE_MAX_WRITE_LOOP               (0x1000U)

/******************************************************************************/
/* Macros related to value used to program to sector header to make different */
/* sector status                                                              */
/******************************************************************************/
#define EE_DATA_INDICATOR               (0x55555555U) /* data to program to active/dead indicator */
#define EE_RECORD_STATUS_VALID          (0x5555U) /* data to program to record status */

/******************************************************************************/
/*                  Macros to calculate internal value                        */
/******************************************************************************/
#define GET_MOD(divisor, dividend) ((divisor) % (dividend))
#define GET_INT(divisor, dividend) ((divisor)/(dividend))

/* Length of 1 record  */
#define EE_DATA_VALUE_ALIGNED_PART      (EE_DATA_VALUE_SIZE & (~EE_LONG_WORD_MASK))
#if (EE_DATA_VALUE_SIZE & EE_LONG_WORD_MASK)
#define EE_RECORD_LENGTH                (EE_DATA_VALUE_ALIGNED_PART + 0x04U + EE_RECORD_HEADER_SIZE)
#else
#define EE_RECORD_LENGTH                (EE_DATA_VALUE_ALIGNED_PART + EE_RECORD_HEADER_SIZE)
#endif

/* Number of Data Records is calculated if we know the total Data Size. */
#if (GET_MOD(EE_EEPROM_SIZE, EE_DATA_VALUE_SIZE) != 0U)
#define EE_MAX_RECORD_NUMBER            (GET_INT(EE_EEPROM_SIZE, EE_DATA_VALUE_SIZE) + 1U)
#else
#define EE_MAX_RECORD_NUMBER            (GET_INT(EE_EEPROM_SIZE, EE_DATA_VALUE_SIZE))
#endif

/******************************************************************************/
/*              Macros to determine the total number of sectors               */
/*                      allocated for EEPROM Emulation                        */
/******************************************************************************/
/* Total number of alternative sectors used */
/* Number of alternative sectors needed for Round Robin scheme. This number must be minimum equal to 2 */
#define EE_ACTUAL_READY_SECTORS         0x2U
/* This should be a minimum of 3 */
#define EE_READY_SECTORS                (EE_ACTUAL_READY_SECTORS + EE_EXTRA_READY_SECTORS)
/* Number of records that can be stored in a sector */
#define EE_SECTOR_CAPACITY              GET_INT(EE_SECTOR_SIZE - EE_SECTOR_HEADER_SIZE, EE_RECORD_LENGTH)

/* Number of bytes in a sector that are not used for emulation */
#define EE_SECTOR_WASTE                 GET_MOD(EE_SECTOR_SIZE- EE_SECTOR_HEADER_SIZE, EE_RECORD_LENGTH)
/* Number of sectors required to store the specified EEPROM size */
#if GET_MOD(EE_MAX_RECORD_NUMBER + 0x1U, EE_SECTOR_CAPACITY)
#define EE_ACTIVE_SECTOR_REQUIRED       (GET_INT(EE_MAX_RECORD_NUMBER + 0x1U, EE_SECTOR_CAPACITY) + 0x1U)
#else
#define EE_ACTIVE_SECTOR_REQUIRED       GET_INT(EE_MAX_RECORD_NUMBER + 0x1U, EE_SECTOR_CAPACITY)
#endif

/* Total number of ACTIVE sectors allocated */
#define EE_ACTIVE_SECTORS               (EE_EXTRA_ACTIVE_SECTORS + EE_ACTIVE_SECTOR_REQUIRED)

/* Total number of sectors allocated will also include some 'alternative sectors' */
#define EE_ALLOCATED_SECTORS            (EE_ACTIVE_SECTORS + EE_READY_SECTORS)


/* Callback function prototype */
typedef void (*ee_callback_t)(void);

/* define Eeprom Config type */
typedef struct
{
    uint32_t startSecAddr[EE_ALLOCATED_SECTORS];      /* pointer to point to start address of each emulated sector */
    uint32_t blankSpace;       /* the blank space to write new record */
    uint32_t eraseCycValue; /* erase cycle of current active sector */
    uint32_t maxValidIndex; /* maximum valid index */
} eeprom_config_t;

/******************************************************************************/
/*                             Global variables                               */
/******************************************************************************/
/* Null Callback function definition */
#define EE_NULL_CALLBACK ((ee_callback_t)0x00000000U)
#if EE_CALLBACK_ENABLE
/* A function pointer to the CallBack function */
#define p_gEECallBack   g_EECallBack
extern  ee_callback_t    g_EECallBack;
#else
#define p_gEECallBack   EE_NULL_CALLBACK
#endif

/******************************************************************************/
/*                          Function declarations                             */
/******************************************************************************/

/*!
 * @brief Emulated EEPROM Initialize.
 *
 * This function is used to initialize EEPROM system including:
 *      - remove dead sector from EEPROM system and determine maximum
 *        valid index in array of start sector address after that.
 *      - Initialize all sectors by erasing the "possible-error-sector".
 *      - Initialize and update cache table if enabled.
 *
 * @param[in] eepromConfig: EEPROM configuration.
 * @return operation status
 * @retval STATUS_SUCCESS: all sectors are initialized successfully.
 * @retval STATUS_EdPROM_MAKE_DEAD_SUCCESS: sector is initialized unsuccessfully and then
 *                                          make it to DEAD successfully.
 * @retval STATUS_EdPROM_MAKE_DEAD_ERROR: sector is initialized unsuccessfully and then
 *                                         make it to DEAD unsuccessfully.
 */
status_t Eed_InitEeprom(eeprom_config_t *eepromConfig);

/*!
 * @brief Emulated EEPROM Write.
 *
 * This function is used to write the specific data record.
 * The data in parameter "p_data" will be written to the specific record.
 *
 * @note The data size is defined by EE_DATA_VALUE_SIZE, user have to make sure
 * EE_DATA_VALUE_SIZE of pData is readable.
 *
 * @param[in] eepromConfig: EEPROM configuration.
 * @param[in] data_id: index of the record to be written.
 * @param[in] pData: pointer to the data to be written.
 * @return operation status
 * @retval STATUS_SUCCESS: write operation is successful.
 * @retval STATUS_EdPROM_MAKE_DEAD_ERROR: write operation is unsuccessful.
 */
status_t Eed_WriteEeprom(eeprom_config_t *eepromConfig, uint16_t data_id, const uint8_t *pData);

/*!
 * @brief Emulated EEPROM Read.
 *
 * This function is used to read the specific data record.
 * This function will update data in pData with the data in the specific record.
 *
 * @note User have to make sure EE_DATA_VALUE_SIZE of pData is writable.
 *
 * @param[in] eepromConfig: EEPROM configuration.
 * @param[in] data_id: index of the record to be read.
 * @param[out] pData: pointer to the data to be read.
 * @return operation status
 * @retval STATUS_SUCCESS: read operation is successful.
 * @retval STATUS_EdPROM_ID_OUT_OF_RANGE: Given data_id is out of range.
 * @retval STATUS_EdPROM_ID_NOT_FOUND: Given data_id is not found.
 */
status_t Eed_ReadEeprom(eeprom_config_t *eepromConfig, uint16_t data_id, uint8_t *pData);

/*!
 * @brief Emulated EEPROM De-Init.
 *
 * This function is to release all the Flash used to EEPROM emulation. After de-initialized, the Flash
 * for emulation will be fully erased.
 *
 * @param[in] eepromConfig: EEPROM configuration.
 * @param[in] data_id: index of the record to be erased.
 * @return operation status
 * @retval STATUS_SUCCESS: erase operation is successful.
 * @retval STATUS_EdPROM_SECTOR_ERASE_ERROR: EEPROM erase operation is unsuccessful.
 */

status_t Eed_DeinitEeprom(eeprom_config_t *eepromConfig);

#endif /* _EE_EMULATION_H_ */
