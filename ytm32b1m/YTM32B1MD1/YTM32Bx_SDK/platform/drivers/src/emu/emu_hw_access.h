/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file emu_hw_access.h
 * @version 1.4.1
 *
 * @brief EMU Hardware Access Layer.
 *
 * This header provides register-level helper functions for the ECC Management
 * Unit (EMU). It is used internally by the EMU driver to reset the module,
 * control channel enables, program injection registers, read report data, and
 * maintain interrupt state.
 *
 * Functions are organized into the following categories:
 *   - Initialization
 *   - Module Enable Control
 *   - Injection Configuration
 *   - Error Reporting
 *   - Interrupt Control
 *
 * @note This is an internal layer. Application code should use the
 *       `EMU_DRV_*` APIs from emu_driver.h.
 */

#ifndef EMU_HW_ACCESS_H
#define EMU_HW_ACCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device_registers.h"

/*!
 * @defgroup emu_hw_access EMU Hardware Access
 * @ingroup emu
 * @brief Low-level register access functions for the ECC Management Unit.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Raw check-bit mask type used by the low-level injection helpers. */
#define emu_chkbit_type uint32_t

#if defined(FEATURE_EMU_SUPPORT_64BIT_INJECTION) && (FEATURE_EMU_SUPPORT_64BIT_INJECTION == 1U)
/*! @brief Raw data-bit mask type used on devices with 64-bit injection support. */
#define emu_databit_type uint64_t
#else
/*! @brief Raw data-bit mask type used on devices with 32-bit injection support. */
#define emu_databit_type uint32_t
#endif /* FEATURE_EMU_SUPPORT_64BIT_INJECTION */

/*! @brief Sentinel value used to skip programming one injection path. */
#define NO_INJECTION_ERROR   (0xFFFFFFFFU)

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Initialization
 ******************************************************************************/
/*!
 * @name Initialization
 * @brief Functions for restoring the EMU hardware to a known reset state.
 * @{
 */

/*!
 * @brief Initialize the EMU peripheral registers to their reset values.
 *
 * Clears the global enable control, per-channel injection programming,
 * accumulated report counters, and pending interrupt flags.
 *
 * @param[in] base  Pointer to the EMU peripheral base address.
 *
 * @pre The EMU peripheral clock must already be enabled.
 */
void EMU_Init(EMU_Type *const base);

/*! @} */ /* End of Initialization */

/*******************************************************************************
 * Module Enable Control
 ******************************************************************************/
/*!
 * @name Module Enable Control
 * @brief Functions for enabling or disabling one EMU injection channel.
 * @{
 */

/*!
 * @brief Disable one EMU channel and clear the global enable latch.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Channel index. Must be less than `EMU_EICHD_COUNT`.
 */
void EMU_Disable(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Enable one EMU channel and program the required global enable value.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Channel index. Must be less than `EMU_EICHD_COUNT`.
 *
 * @note Some devices require a key value in the global enable register. This
 *       helper handles that device-specific detail internally.
 */
void EMU_Enable(EMU_Type *const base, uint8_t channel);

/*! @} */ /* End of Module Enable Control */

/*******************************************************************************
 * Injection Configuration
 ******************************************************************************/
/*!
 * @name Injection Configuration
 * @brief Functions for programming EMU address, data, and check-bit injection
 *        registers.
 * @{
 */

/*!
 * @brief Program the SRAM word address used to trigger an injected ECC event.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Channel index. Must be less than `EMU_EICHD_COUNT`.
 * @param[in] addr     Target SRAM word address for the injection.
 */
void EMU_SetErrInjectAddr(EMU_Type *const base, uint8_t channel, volatile uint32_t addr);

/*!
 * @brief Program one check-bit index for single-bit injection.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Channel index. Must be less than `EMU_EICHD_COUNT`.
 * @param[in] chkbit   Check-bit index to inject.
 */
void EMU_SetErrInjectChkbit(EMU_Type *const base, uint8_t channel, emu_chkbit_type chkbit);

/*!
 * @brief Program a raw multi-bit check mask for one channel.
 *
 * @param[in] base      Pointer to the EMU peripheral base address.
 * @param[in] channel   Channel index. Must be less than `EMU_EICHD_COUNT`.
 * @param[in] chkMulti  Raw check-bit mask to write into the injection register.
 */
void EMU_SetErrInjectChkMulti(EMU_Type *const base, uint8_t channel, emu_chkbit_type chkMulti);

/*!
 * @brief Clear the programmed check-bit injection mask for one channel.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Channel index. Must be less than `EMU_EICHD_COUNT`.
 */
void EMU_ClearChkbit(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Program one data-bit index for single-bit injection.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Channel index. Must be less than `EMU_EICHD_COUNT`.
 * @param[in] databit  Data-bit index to inject.
 */
void EMU_SetErrInjectData(EMU_Type *const base, uint8_t channel, emu_databit_type databit);

/*!
 * @brief Program a raw multi-bit data mask for one channel.
 *
 * @param[in] base      Pointer to the EMU peripheral base address.
 * @param[in] channel   Channel index. Must be less than `EMU_EICHD_COUNT`.
 * @param[in] dataMulti Raw data-bit mask to write into the injection register
 *                      or registers.
 */
void EMU_SetErrInjectDataMulti(EMU_Type *const base, uint8_t channel, emu_databit_type dataMulti);

/*!
 * @brief Clear the programmed data-bit injection mask for one channel.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Channel index. Must be less than `EMU_EICHD_COUNT`.
 */
void EMU_ClearData(EMU_Type *const base, uint8_t channel);

/*! @} */ /* End of Injection Configuration */

/*******************************************************************************
 * Error Reporting
 ******************************************************************************/
/*!
 * @name Error Reporting
 * @brief Functions for reading and clearing EMU report information.
 * @{
 */

/*!
 * @brief Read the error report counter for one channel.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Report channel index. Must be less than
 *                     `EMU_ERINFO_COUNT`.
 * @return Current error report counter value.
 */
uint32_t EMU_GetErrReportCnt(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Clear the error report counter for one channel.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Report channel index. Must be less than
 *                     `EMU_ERINFO_COUNT`.
 */
void EMU_ClearErrReportCnt(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Read the latched report address for one channel.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Report channel index. Must be less than
 *                     `EMU_EICHD_COUNT`.
 * @return Reported SRAM address for the selected channel.
 */
uint32_t EMU_GetErrReportAddr(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Read the raw syndrome field for one channel.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Report channel index. Must be less than
 *                     `EMU_ERINFO_COUNT`.
 * @return Raw syndrome field value after mask-and-shift processing.
 */
uint32_t EMU_GetSyndrome(EMU_Type *const base, uint8_t channel);

/*! @} */ /* End of Error Reporting */

/*******************************************************************************
 * Interrupt Control
 ******************************************************************************/
/*!
 * @name Interrupt Control
 * @brief Functions for enabling, clearing, and querying EMU interrupt state.
 * @{
 */

/*!
 * @brief Enable single-bit correction interrupt reporting for one channel.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Report channel index. Must be less than
 *                     `EMU_ERINFO_COUNT`.
 */
void EMU_EnableInterruptSignalBit(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Disable single-bit correction interrupt reporting for one channel.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Report channel index. Must be less than
 *                     `EMU_ERINFO_COUNT`.
 */
void EMU_DisableInterruptSignalBit(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Clear the single-bit correction interrupt flag for one channel.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Report channel index. Must be less than
 *                     `EMU_ERINFO_COUNT`.
 */
void EMU_ClearInterruptFlagSignalBit(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Enable double-bit interrupt reporting for one channel.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Report channel index. Must be less than
 *                     `EMU_ERINFO_COUNT`.
 */
void EMU_EnableInterruptDoubleBit(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Disable double-bit interrupt reporting for one channel.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Report channel index. Must be less than
 *                     `EMU_ERINFO_COUNT`.
 */
void EMU_DisableInterruptDoubleBit(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Clear the double-bit interrupt flag for one channel.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Report channel index. Must be less than
 *                     `EMU_ERINFO_COUNT`.
 */
void EMU_ClearInterruptFlagDoubleBit(EMU_Type *const base, uint8_t channel);

/*!
 * @brief Query the single-bit correction interrupt flag field for one channel.
 *
 * @param[in] base     Pointer to the EMU peripheral base address.
 * @param[in] channel  Channel index. Must be less than `EMU_EICHD_COUNT`.
 * @return Non-zero value when the single-bit correction flag is asserted for
 *         the selected channel.
 */
uint8_t EMU_GetChannelSBInterruptFlag(EMU_Type *const base, uint8_t channel);

/*! @} */ /* End of Interrupt Control */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of emu_hw_access */

#endif /* EMU_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
