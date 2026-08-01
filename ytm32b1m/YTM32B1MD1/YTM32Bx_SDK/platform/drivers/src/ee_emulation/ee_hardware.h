/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file ee_hardware.h
 * @version 1.4.1
 *
 * @brief EEPROM Emulation internal helper declarations.
 *
 * This header exposes the internal helper APIs shared by the EEPROM
 * emulation high-, middle-, and low-level implementation files. It also
 * provides the raw memory access macros used to read and program Flash-backed
 * record headers.
 *
 * Functions are organized into the following categories:
 *   - Data Migration & Record Programming
 *   - Search & Callback Service
 *   - Sector State Management
 *   - Sequence Maintenance
 *   - Flash Access
 *
 * @note This is an internal header. Application code should include
 *       `ee_emulation.h` instead.
 */

#ifndef EE_HARDWARE_H__
#define EE_HARDWARE_H__

#include "ee_emulation.h"

/*!
 * @defgroup ee_emulation_internal EEPROM Emulation Internal Helpers
 * @ingroup ee_emulation
 * @brief Internal helpers shared by the EEPROM emulation implementation
 *        layers.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @name Memory Access Helpers
 * @brief Convenience macros for raw memory access during Flash record and
 *        sector management.
 * @{
 */
#define READ8(address)                  ((uint8_t)(*(volatile uint8_t *)(address)))       /*!< Read one byte from the supplied address. */
#define WRITE32(address, value)         (*(volatile uint32_t *)((address)) = (value))     /*!< Write one 32-bit word to the supplied address. */
#define READ32(address)                 ((uint32_t)(*(volatile uint32_t *)(address)))      /*!< Read one 32-bit word from the supplied address. */
/*! @} */ /* End of Memory Access Helpers */

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @name Data Migration & Record Programming
 * @brief Helpers that move valid records and program new record images.
 * @{
 */

/*!
 * @brief Copy the newest valid records from the oldest active sector into the
 *        ready swap sector.
 *
 * @param[in,out] eepromConfig  Pointer to the EEPROM emulation runtime state.
 * @return Execution status.
 */
status_t Eed_CopyData(eeprom_config_t *eepromConfig);

/*!
 * @brief Program one record payload and header into the current blank slot.
 *
 * @param[in,out] eepromConfig  Pointer to the EEPROM emulation runtime state.
 * @param[in] source            Address of the source payload buffer.
 * @param[in] data_id           Logical record identifier to store.
 * @return Execution status.
 */
status_t Eed_WriteRecord(eeprom_config_t *eepromConfig, uint32_t source, uint16_t data_id);

/*! @} */ /* End of Data Migration & Record Programming */

/*!
 * @name Search & Callback Service
 * @brief Helpers for locating record slots and servicing the optional
 *        callback.
 * @{
 */

/*!
 * @brief Scan one active sector to locate its first blank record slot.
 *
 * @param[in] sec_addr  Base address of the active sector.
 * @return Address of the first writable record slot within that sector.
 */
uint32_t Eed_SearchBlankSpace(uint32_t sec_addr);

/*!
 * @brief Search all active sectors from newest to oldest for one record ID.
 *
 * @param[in,out] eepromConfig  Pointer to the EEPROM emulation runtime state.
 * @param[in] data_id           Logical record identifier to locate.
 * @return Record address, or `0xFFFFFFFFU` when the record is not present.
 */
uint32_t Eed_SearchInAllActives(eeprom_config_t *eepromConfig, uint16_t data_id);

/*!
 * @brief Search the cache first and then fall back to an active-sector scan.
 *
 * @param[in,out] eepromConfig  Pointer to the EEPROM emulation runtime state.
 * @param[in] data_id           Logical record identifier to locate.
 * @return Record address, or `0xFFFFFFFFU` when the record is not present.
 */
uint32_t Eed_SearchLoop(eeprom_config_t *eepromConfig, uint16_t data_id);

/*!
 * @brief Invoke the registered callback after the configured service interval.
 *
 * @param[in,out] counter  Loop counter used to schedule callback execution.
 */
void Eed_CallBack(uint32_t *counter);

/*! @} */ /* End of Search & Callback Service */

/*!
 * @name Sector State Management
 * @brief Helpers that validate sector headers and rebuild sector state.
 * @{
 */

/*!
 * @brief Mark one sector as dead and compact the live sector list.
 *
 * @param[in,out] eepromConfig  Pointer to the EEPROM emulation runtime state.
 * @param[in] index             Sector index within the current live sequence.
 * @return Execution status.
 */
status_t Eed_ValidateDeadIndicator(eeprom_config_t *eepromConfig, uint32_t index);

/*!
 * @brief Erase one sector, program its erase-cycle value, and retire it if
 *        repeated recovery attempts fail.
 *
 * @param[in,out] eepromConfig  Pointer to the EEPROM emulation runtime state.
 * @param[in] expErsVal         Erase-cycle value that should be programmed.
 * @param[in] index             Sector index within the current live sequence.
 * @return Execution status.
 */
status_t Eed_SyncValidateSector(eeprom_config_t *eepromConfig, uint32_t expErsVal, uint32_t index);

/*!
 * @brief Program the active indicator for one sector.
 *
 * @param[in,out] eepromConfig  Pointer to the EEPROM emulation runtime state.
 * @param[in] index             Sector index within the current live sequence.
 * @return Execution status.
 */
status_t Eed_ValidateActIndicator(eeprom_config_t *eepromConfig, uint8_t index);

/*!
 * @brief Retry programming one sector-state indicator until it reads back or
 *        the retry limit is exhausted.
 *
 * @param[in] dest  Address of the sector-state indicator to program.
 * @return Value read back from the destination after the retry loop.
 */
uint32_t Eed_OverProgram(uint32_t dest);

/*!
 * @brief Reconstruct the active or ready sector state after initialization.
 *
 * @param[in,out] eepromConfig  Pointer to the EEPROM emulation runtime state.
 * @return Execution status.
 */
status_t Eed_InitAllSectors(eeprom_config_t *eepromConfig);

/*! @} */ /* End of Sector State Management */

/*!
 * @name Sequence Maintenance
 * @brief Helpers that maintain the live sector ordering and cache contents.
 * @{
 */

/*!
 * @brief Rotate the sector-address array so one sector becomes the base active
 *        position.
 *
 * @param[in,out] eepromConfig  Pointer to the EEPROM emulation runtime state.
 * @param[in] current_idx       Sector index that should rotate into the base
 *                              active position.
 */
void Eed_ShiftIdxToBase(eeprom_config_t *eepromConfig, uint32_t current_idx);

/*!
 * @brief Move dead sectors to the end of the sector-address array.
 *
 * @param[in,out] eepromConfig  Pointer to the EEPROM emulation runtime state.
 * @return Execution status.
 */
status_t Eed_RemoveDeadSector(eeprom_config_t *eepromConfig);

/*!
 * @brief Clear or rebuild cached record locations.
 *
 * @param[in,out] eepromConfig  Pointer to the EEPROM emulation runtime state.
 * @param[in] init_flag         Pass a nonzero value to clear the cache table
 *                              without rebuilding it.
 */
void Eed_UpdateCacheTable(eeprom_config_t *eepromConfig, uint8_t init_flag);

/*! @} */ /* End of Sequence Maintenance */

/*!
 * @name Flash Access
 * @brief Low-level helpers that erase, verify, and program the backing Flash
 *        sectors.
 * @{
 */

/*!
 * @brief Erase one Flash sector and verify that it is blank.
 *
 * @param[in] dest  Base address of the Flash sector to erase.
 * @return Execution status.
 */
status_t Eed_FlashErase(uint32_t dest);

/*!
 * @brief Verify that a Flash region contains only erased long words.
 *
 * @param[in] dest    Base address of the Flash region to verify.
 * @param[in] number  Number of long words to verify.
 * @return Execution status.
 */
status_t Eed_FlashEraseVerify(uint32_t dest, uint16_t number);

/*!
 * @brief Program one Flash region and verify the written data.
 *
 * @param[in] dest    Destination Flash address.
 * @param[in] size    Number of bytes to program.
 * @param[in] source  Address of the source data buffer.
 * @return Execution status.
 */
status_t Eed_FlashProgram(uint32_t dest, uint8_t size, uint32_t source);

/*! @} */ /* End of Flash Access */

/*! @} */ /* End of ee_emulation_internal */

#endif /* EE_HARDWARE_H__ */
