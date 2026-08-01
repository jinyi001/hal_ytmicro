/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file ee_emulation.h
 * @version 1.4.1
 *
 * @brief EEPROM Emulation driver - public API for Flash-backed fixed-size
 *        records.
 *
 * This header defines the application-facing APIs and derived layout macros
 * used by the EEPROM emulation module. The module stores fixed-size records in
 * a round-robin pool of Flash sectors, rebuilds sector state during
 * initialization, and exposes simple read, write, and erase-style lifecycle
 * operations to application code.
 *
 * The APIs are organized into the following categories:
 *   - Initialization
 *   - Record Access
 *   - Shutdown
 *
 * @note The application must provide `ee_user_config.h` with the EEPROM size,
 *       record size, sector allocation policy, retry limits, callback
 *       settings, and start address definitions used by this module.
 * @warning Every entry in `eeprom_config_t::startSecAddr` must reference a
 *          valid erasable Flash sector dedicated to EEPROM emulation.
 */

#ifndef _EE_EMULATION_H_
#define _EE_EMULATION_H_

#include <stdbool.h>
#include <stddef.h>
#include "status.h"
#include "device_registers.h"
#include "ee_user_config.h"

/*!
 * @defgroup ee_emulation_driver EEPROM Emulation Driver
 * @ingroup ee_emulation
 * @brief Public APIs and layout helpers for Flash-backed EEPROM emulation.
 * @details Provides derived record or sector layout macros, runtime state
 *          containers, and application-facing services for initializing the
 *          emulation pool, reading records, writing updated records, and
 *          erasing the allocated sectors during shutdown.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @name Record Layout
 * @brief Macros that describe one emulated EEPROM record stored in Flash.
 * @{
 */
#define EE_RECORD_HEADER_SIZE           (FEATURE_EFM_WRITE_UNIT_SIZE)        /*!< Bytes reserved for the record header programmed after the payload. */
#define EE_RECORD_STATUS_OFFSET         (EE_RECORD_LENGTH - EE_RECORD_HEADER_SIZE) /*!< Offset of the status-and-ID header within one stored record. */
/*! @} */ /* End of Record Layout */

/*!
 * @name Sector Layout
 * @brief Macros that describe one Flash sector participating in EEPROM
 *        emulation.
 * @{
 */
#define EE_INDICATOR_SIZE               (FEATURE_EFM_WRITE_UNIT_SIZE)        /*!< Bytes reserved for each sector-state indicator. */
#define EE_SECTOR_HEADER_SIZE           (EE_INDICATOR_SIZE * 3)              /*!< Bytes consumed by erase-cycle, dead, and active indicators. */
#if FEATURE_EFM_HAS_DATA_FLASH
#define EE_SECTOR_SIZE                  FEATURE_EFM_DATA_ARRAY_SECTOR_SIZE   /*!< Physical sector size when the device provides dedicated data Flash. */
#else
#define EE_SECTOR_SIZE                  FEATURE_EFM_MAIN_ARRAY_SECTOR_SIZE   /*!< Physical sector size when EEPROM emulation uses main-array Flash. */
#endif
#define EE_SECTOR_ACTIND_OFFSET         (EE_INDICATOR_SIZE * 2)              /*!< Offset of the active indicator within the sector header. */
#define EE_SECTOR_DEADIND_OFFSET        (EE_INDICATOR_SIZE * 1)              /*!< Offset of the dead-sector indicator within the sector header. */
#define EE_SECTOR_ERSCYC_OFFSET         (EE_INDICATOR_SIZE * 0)              /*!< Offset of the erase-cycle value within the sector header. */
#define EE_LONG_WORD_MASK               (0x3U)                               /*!< Mask used to detect payload alignment against the 4-byte write granule. */
#define EE_MAX_WRITE_LOOP               (0x1000U)                            /*!< Maximum high-level write retries before aborting a stalled update loop. */
/*! @} */ /* End of Sector Layout */

/*!
 * @name Sector State Marker Values
 * @brief Constants programmed into record and sector headers.
 * @{
 */
#define EE_DATA_INDICATOR               (0x55555555U)                        /*!< Pattern written into active or dead indicators once the state is valid. */
#define EE_RECORD_STATUS_VALID          (0x5555U)                            /*!< Record-status value that marks one stored record as valid. */
/*! @} */ /* End of Sector State Marker Values */

/*!
 * @name Derived Geometry Helpers
 * @brief Macros that derive record counts and storage geometry from the user
 *        configuration.
 * @{
 */
#define GET_MOD(divisor, dividend)      ((divisor) % (dividend))             /*!< Integer remainder helper used by the storage geometry macros. */
#define GET_INT(divisor, dividend)      ((divisor) / (dividend))             /*!< Integer division helper used by the storage geometry macros. */

#define EE_DATA_VALUE_ALIGNED_PART      (EE_DATA_VALUE_SIZE & (~EE_LONG_WORD_MASK)) /*!< Largest record payload prefix aligned to the Flash write granule. */
#if (EE_DATA_VALUE_SIZE & EE_LONG_WORD_MASK)
#define EE_RECORD_LENGTH                (EE_DATA_VALUE_ALIGNED_PART + 0x04U + EE_RECORD_HEADER_SIZE) /*!< Total stored record length when a padded tail write is required. */
#else
#define EE_RECORD_LENGTH                (EE_DATA_VALUE_ALIGNED_PART + EE_RECORD_HEADER_SIZE) /*!< Total stored record length when the payload is already aligned. */
#endif

#if (GET_MOD(EE_EEPROM_SIZE, EE_DATA_VALUE_SIZE) != 0U)
#define EE_MAX_RECORD_NUMBER            (GET_INT(EE_EEPROM_SIZE, EE_DATA_VALUE_SIZE) + 1U) /*!< Number of logical record identifiers needed to cover the requested EEPROM size. */
#else
#define EE_MAX_RECORD_NUMBER            (GET_INT(EE_EEPROM_SIZE, EE_DATA_VALUE_SIZE)) /*!< Number of logical record identifiers needed to cover the requested EEPROM size. */
#endif
/*! @} */ /* End of Derived Geometry Helpers */

/*!
 * @name Sector Pool Sizing
 * @brief Macros that derive active, ready, and total sector counts for the
 *        round-robin pool.
 * @{
 */
#define EE_ACTUAL_READY_SECTORS         0x2U                                 /*!< Minimum number of ready sectors required by the round-robin scheme. */
#define EE_READY_SECTORS                (EE_ACTUAL_READY_SECTORS + EE_EXTRA_READY_SECTORS) /*!< Total ready-sector budget, including user-reserved spare sectors. */
#define EE_SECTOR_CAPACITY              GET_INT(EE_SECTOR_SIZE - EE_SECTOR_HEADER_SIZE, EE_RECORD_LENGTH) /*!< Number of records that fit in one sector after the header area is removed. */
#define EE_SECTOR_WASTE                 GET_MOD(EE_SECTOR_SIZE - EE_SECTOR_HEADER_SIZE, EE_RECORD_LENGTH) /*!< Unused bytes left at the end of one sector after record packing. */
#if GET_MOD(EE_MAX_RECORD_NUMBER + 0x1U, EE_SECTOR_CAPACITY)
#define EE_ACTIVE_SECTOR_REQUIRED       (GET_INT(EE_MAX_RECORD_NUMBER + 0x1U, EE_SECTOR_CAPACITY) + 0x1U) /*!< Minimum active sectors required to hold every record plus the swap slot. */
#else
#define EE_ACTIVE_SECTOR_REQUIRED       GET_INT(EE_MAX_RECORD_NUMBER + 0x1U, EE_SECTOR_CAPACITY) /*!< Minimum active sectors required to hold every record plus the swap slot. */
#endif
#define EE_ACTIVE_SECTORS               (EE_EXTRA_ACTIVE_SECTORS + EE_ACTIVE_SECTOR_REQUIRED) /*!< Total active-sector budget after applying the user-reserved active margin. */
#define EE_ALLOCATED_SECTORS            (EE_ACTIVE_SECTORS + EE_READY_SECTORS) /*!< Total sectors that must be mapped into `startSecAddr`. */
/*! @} */ /* End of Sector Pool Sizing */

/*!
 * @brief Callback type invoked while long Flash operations are polled.
 */
typedef void (*ee_callback_t)(void);

/*!
 * @brief EEPROM emulation runtime state container.
 *
 * The application fills `startSecAddr` before initialization. The remaining
 * fields are maintained by the driver while it discovers the active sector
 * window, tracks the next blank record slot, and preserves erase-cycle
 * information for swap recovery.
 *
 * | Field | Type | Description |
 * |-------|------|-------------|
 * | startSecAddr | `uint32_t[EE_ALLOCATED_SECTORS]` | Ordered list of Flash sector base addresses reserved for EEPROM emulation. |
 * | blankSpace | `uint32_t` | Address of the next writable record slot in the youngest active sector. |
 * | eraseCycValue | `uint32_t` | Erase-cycle value associated with the current youngest active sector. |
 * | maxValidIndex | `uint32_t` | Highest live entry index in `startSecAddr` after dead sectors are removed. |
 */
typedef struct
{
    uint32_t startSecAddr[EE_ALLOCATED_SECTORS]; /*!< Flash sector base addresses assigned to the emulation pool. */
    uint32_t blankSpace;                         /*!< Next writable record address inside the current youngest active sector. */
    uint32_t eraseCycValue;                     /*!< Erase-cycle value tracked for the current active write sector. */
    uint32_t maxValidIndex;                     /*!< Highest live sector index after dead sectors are compacted out. */
} eeprom_config_t;

/*!
 * @name Callback Support
 * @brief Macros and variables related to the optional long-operation callback.
 * @{
 */
#define EE_NULL_CALLBACK                ((ee_callback_t)0x00000000U)         /*!< Sentinel used when no callback is registered. */
#if EE_CALLBACK_ENABLE
#define p_gEECallBack                   g_EECallBack                          /*!< Alias used by the implementation to access the registered callback. */
extern ee_callback_t g_EECallBack;                                           /*!< Application callback invoked while Flash erase or program commands are pending. */
#else
#define p_gEECallBack                   EE_NULL_CALLBACK                      /*!< Alias that collapses to a null callback when callbacks are disabled. */
#endif
/*! @} */ /* End of Callback Support */

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @name Initialization
 * @brief Functions for discovering sector state and preparing the emulation
 *        pool for record access.
 * @{
 */

/*!
 * @brief Discover live sectors, repair incomplete state, and prepare the
 *        emulation pool for record access.
 *
 * This function removes sectors already marked dead, reconstructs the active
 * sector window, initializes missing sector headers when necessary, and
 * rebuilds the optional cache table.
 *
 * @param[in,out] eepromConfig  Pointer to the EEPROM emulation runtime state.
 *                              `startSecAddr` must already contain
 *                              `EE_ALLOCATED_SECTORS` Flash sector base
 *                              addresses reserved for EEPROM emulation.
 * @return Execution status.
 * @retval STATUS_SUCCESS                   Initialization completed
 *                                          successfully.
 * @retval STATUS_EdPROM_MAKE_DEAD_SUCCESS  One recovery step failed, the
 *                                          sector was retired successfully,
 *                                          and initialization can continue.
 * @retval STATUS_EdPROM_MAKE_DEAD_ERROR    A recovery step failed and the
 *                                          sector could not be retired.
 *
 * @post `blankSpace`, `eraseCycValue`, and `maxValidIndex` reflect the
 *       reconstructed runtime state of the emulation pool.
 */
status_t Eed_InitEeprom(eeprom_config_t *eepromConfig);

/*! @} */ /* End of Initialization */

/*!
 * @name Record Access
 * @brief Functions for writing and reading fixed-size EEPROM records.
 * @{
 */

/*!
 * @brief Write one logical EEPROM record.
 *
 * The driver writes the record payload pointed to by @a pData into the slot
 * identified by @a data_id. When the current active sector is full, the
 * driver performs the required sector-swap sequence before retrying the
 * record write.
 *
 * @param[in,out] eepromConfig  Pointer to the EEPROM emulation runtime state.
 * @param[in] data_id           Logical record identifier. Must be less than
 *                              `EE_MAX_RECORD_NUMBER`.
 * @param[in] pData             Pointer to `EE_DATA_VALUE_SIZE` bytes of source
 *                              data.
 * @return Execution status.
 * @retval STATUS_SUCCESS                   The record was written
 *                                          successfully.
 * @retval STATUS_EdPROM_ID_OUT_OF_RANGE    `data_id` is outside the emulated
 *                                          EEPROM address space.
 * @retval STATUS_EdPROM_MAKE_DEAD_ERROR    The write or swap flow failed and
 *                                          the affected sector could not be
 *                                          retired safely.
 *
 * @note `pData` must reference at least `EE_DATA_VALUE_SIZE` readable bytes.
 */
status_t Eed_WriteEeprom(eeprom_config_t *eepromConfig, uint16_t data_id, const uint8_t *pData);

/*!
 * @brief Read the newest stored copy of one logical EEPROM record.
 *
 * @param[in,out] eepromConfig  Pointer to the EEPROM emulation runtime state.
 * @param[in] data_id           Logical record identifier. Must be less than
 *                              `EE_MAX_RECORD_NUMBER`.
 * @param[out] pData            Pointer to `EE_DATA_VALUE_SIZE` writable bytes
 *                              that receive the record payload.
 * @return Execution status.
 * @retval STATUS_SUCCESS                The requested record was found and
 *                                       copied into @a pData.
 * @retval STATUS_EdPROM_ID_OUT_OF_RANGE `data_id` is outside the emulated
 *                                       EEPROM address space.
 * @retval STATUS_EdPROM_ID_NOT_FOUND    No valid record with the requested ID
 *                                       is currently stored.
 *
 * @note `pData` must reference at least `EE_DATA_VALUE_SIZE` writable bytes.
 */
status_t Eed_ReadEeprom(eeprom_config_t *eepromConfig, uint16_t data_id, uint8_t *pData);

/*! @} */ /* End of Record Access */

/*!
 * @name Shutdown
 * @brief Functions for releasing the Flash region reserved for EEPROM
 *        emulation.
 * @{
 */

/*!
 * @brief Erase every sector assigned to the EEPROM emulation pool.
 *
 * @param[in,out] eepromConfig  Pointer to the EEPROM emulation runtime state.
 * @return Execution status.
 * @retval STATUS_SUCCESS                    All allocated sectors were erased
 *                                           successfully.
 * @retval STATUS_EdPROM_SECTOR_ERASE_ERROR  At least one sector erase failed.
 *
 * @post The Flash sectors listed in `startSecAddr` are returned to the blank
 *       state expected by a fresh EEPROM emulation initialization.
 */
status_t Eed_DeinitEeprom(eeprom_config_t *eepromConfig);

/*! @} */ /* End of Shutdown */

/*! @} */ /* End of ee_emulation_driver */

#endif /* _EE_EMULATION_H_ */
