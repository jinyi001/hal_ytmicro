/* 
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.                                                                  
 *                                                                                       
 * SPDX-License-Identifier: BSD-3-Clause                          
 */

/*!
 * @file ee_emulation.h
 */

#ifndef EE_HARDWARE_H__
#define EE_HARDWARE_H__

#include "ee_emulation.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

/******************************************************************************/
/*               Read/Write/Set/Clear Operation Macros                        */
/******************************************************************************/

#define READ8(address)                  ((uint8_t)(*(volatile uint8_t *)(address)))
#define WRITE32(address, value)         (*(volatile uint32_t *)((address)) = (value))
#define READ32(address)                 ((uint32_t)(*(volatile uint32_t *)(address)))

/* Middle level functions */
status_t    Eed_CopyData(eeprom_config_t * eepromConfig);
status_t    Eed_WriteRecord(eeprom_config_t * eepromConfig, uint32_t source, uint16_t data_id);
uint32_t    Eed_SearchBlankSpace(uint32_t sec_addr);
uint32_t    Eed_SearchInAllActives(eeprom_config_t * eepromConfig, uint16_t data_id);
uint32_t    Eed_SearchLoop(eeprom_config_t * eepromConfig, uint16_t data_id);
void        Eed_CallBack(uint32_t* counter);
status_t    Eed_ValidateDeadIndicator(eeprom_config_t * eepromConfig, uint32_t index);
status_t    Eed_SyncValidateSector(eeprom_config_t * eepromConfig, uint32_t expErsVal, uint32_t index);
status_t    Eed_ValidateActIndicator(eeprom_config_t * eepromConfig, uint8_t index);
uint32_t    Eed_OverProgram(uint32_t dest);
status_t    Eed_InitAllSectors(eeprom_config_t * eepromConfig);
void        Eed_ShiftIdxToBase(eeprom_config_t * eepromConfig, uint32_t current_idx);
status_t    Eed_RemoveDeadSector(eeprom_config_t * eepromConfig);
void        Eed_UpdateCacheTable(eeprom_config_t * eepromConfig, uint8_t init_flag);

/* Low level functions */
status_t    Eed_FlashErase(uint32_t dest);
status_t    Eed_FlashEraseVerify(uint32_t dest, uint16_t number);
status_t    Eed_FlashProgram(uint32_t dest, uint8_t size, uint32_t source);

#endif /* EE_HARDWARE_H__ */
/*******************************************************************************
 * EOF
 ******************************************************************************/
