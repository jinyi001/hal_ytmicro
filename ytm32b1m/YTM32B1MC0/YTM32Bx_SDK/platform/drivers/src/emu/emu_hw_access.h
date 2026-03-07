/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file emu_hw_access.h
 * @version 1.4.0
 */

#ifndef EMU_HW_ACCESS_H
#define EMU_HW_ACCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device_registers.h"

/*!
 * @defgroup emu_hw_access
 * @ingroup emu
 * @details This section describes the programming interface of the EMU  Driver.
 * @{
 */

/*!
 * @addtogroup emu_hw_access
 * @{
 */


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define emu_chkbit_type uint32_t
#if defined(FEATURE_EMU_SUPPORT_64BIT_INJECTION) && (FEATURE_EMU_SUPPORT_64BIT_INJECTION == 1U)
#define emu_databit_type uint64_t
#else
#define emu_databit_type uint32_t
#endif /* FEATURE_EMU_SUPPORT_64BIT_INJECTION */

#define NO_INJECTION_ERROR   (0xFFFFFFFFU)
/*******************************************************************************
 * Function Prototypes for Emu
 ******************************************************************************/
/*!
 * @name Emu driver APIs
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initialize the EMU instance to reset values.
 *
 * This function initializes all registers of the EMU instance to a known state (the register
 * are written with their reset values from the Reference Manual).
 *
 * @param[in] base - EMU base pointer
 *
 */
void EMU_Init(EMU_Type *const base);

/*!
 * @brief Clear error report count.
 *
 * This function clear error report count.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
void EMU_ClearErrReportCnt(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Disable the EMU module.
 *
 * This function disable the EMU module.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
void EMU_Disable(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Enable the EMU module.
 *
 * This function enable the EMU module.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
void EMU_Enable(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Get error report count.
 *
 * This function get error report count.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
uint32_t EMU_GetErrReportCnt(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Set error inject address.
 *
 * This function set error inject address.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
void EMU_SetErrInjectAddr(EMU_Type *const base, uint8_t channel, volatile uint32_t addr);

/*!
 * @brief Set error injection check bit.
 *
 * This function set error injection check bit.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 * @param[in] chkbit            The EMU injection check bit
 */
void EMU_SetErrInjectChkbit(EMU_Type *const base, uint8_t channel, emu_chkbit_type chkbit);

/*!
 * @brief Set error injection check bit.
 *
 * This function set error injection check bit.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 * @param[in] chkMulti          The EMU multiple injection check bit
 */
void EMU_SetErrInjectChkMulti(EMU_Type *const base, uint8_t channel, emu_chkbit_type chkMulti);


/*!
 * @brief Clear error inject check bit.
 *
 * This function clear error inject check bit.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
void EMU_ClearChkbit(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Set error inject data bit.
 *
 * This function set error inject data bit.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 * @param[in] databit           The EMU injection error bit of data
 */
void EMU_SetErrInjectData(EMU_Type *const base, uint8_t channel, emu_databit_type databit);

/*!
 * @brief Set error inject multiple data bits.
 *
 * This function set error inject multiple data bits.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 * @param[in] dataMux           The EMU injection multiple error bits of data
 * 
 */
void EMU_SetErrInjectDataMulti(EMU_Type *const base, uint8_t channel, emu_databit_type dataMulti);

/*!
 * @brief Clear error inject data bit.
 *
 * This function clear error inject data bit.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
void EMU_ClearData(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Get error report address.
 *
 * This function Get error report address.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
uint32_t EMU_GetErrReportAddr(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Enable interrupt signal bit.
 *
 * This function enable interrupt signal bit.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
void EMU_EnableInterruptSignalBit(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Disable interrupt signal bit.
 *
 * This function disable interrupt signal bit.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
void EMU_DisableInterruptSignalBit(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Clear interrupt signal bit flag.
 *
 * This function clear interrupt signal bit flag.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
void EMU_ClearInterruptFlagSignalBit(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Enable interrupt double bit.
 *
 * This function enable interrupt double bit.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
void EMU_EnableInterruptDoubleBit(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Disable interrupt double bit.
 *
 * This function disable interrupt double bit.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
void EMU_DisableInterruptDoubleBit(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Clear interrupt double bit.
 *
 * This function clear interrupt double bit.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
void EMU_ClearInterruptFlagDoubleBit(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Get syndrome.
 *
 * This function get syndrome.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
uint32_t EMU_GetSyndrome(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Get single bit error correct check interrupt flag.
 * 
 * @param[in] base              The EMU peripheral base address
 * @param[in] channel           The EMU channel
 */
uint8_t EMU_GetChannelSBInterruptFlag(EMU_Type *const base, uint8_t channel);

#if defined(__cplusplus)
}
#endif

/*! @}*/ /* End of Emu hardware driver APIs*/
/*! @}*/ /* End of addtogroup emu_hw_access */

#endif /* EMU_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
