/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file flash_hw_access.h
 * @version 1.4.1
 *
 * @brief Flash Hardware Access Layer.
 *
 * This header provides low-level inline accessor functions for the EFM
 * (Embedded Flash Module) peripheral registers. Functions are organized
 * into the following categories:
 *   - Status Query — idle, done, and error status checks
 *   - Error Flag Management — clear error and done flags, read-collision
 *   - ECC Fault Detection — single-bit and double-bit fault flags
 *   - Command Control — flash command code register write
 *
 * @note This is an internal layer used by the Flash Driver (flash_driver.h/c).
 *       Application code should use the FLASH_DRV_* APIs from flash_driver.h.
 */

#ifndef FLASH_HW_ACCESS_H
#define FLASH_HW_ACCESS_H

#include <stdbool.h>
#include "flash_driver.h"

/*!
 * @addtogroup flash_hw_access Flash Hardware Access
 * @ingroup flash
 * @brief Low-level register access functions for the EFM peripheral.
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Status Query
 ******************************************************************************/
/*!
 * @name Status Query
 * @brief Functions for querying the current operating status of the flash
 *        controller (idle, done, error).
 * @{
 */

/*!
 * @brief Check whether the flash controller is idle.
 *
 * Reads the IDLE bit from the EFM status register to determine if the
 * flash controller has finished all pending operations and is ready to
 * accept a new command.
 *
 * @param[in] efmBase  Pointer to the EFM peripheral base address.
 * @return Flash idle state:
 *         - true:  Flash controller is idle and ready.
 *         - false: Flash controller is busy with an operation.
 */
static inline bool FLASH_GetIdleStatus(EFM_Type *efmBase)
{
    return (bool)(EFM_STS_IDLE_MASK == (efmBase->STS & EFM_STS_IDLE_MASK));
}

/*!
 * @brief Check whether a flash command error has occurred.
 *
 * Reads the error status bits from the EFM status register. The error
 * mask covers all command-related error conditions defined by the device.
 *
 * @param[in] efmBase  Pointer to the EFM peripheral base address.
 * @return Error state:
 *         - true:  One or more command errors detected.
 *         - false: No command errors.
 */
static inline bool FLASH_GetErrorStatus(EFM_Type *efmBase)
{
    return (bool)(0U != (efmBase->STS & FEATURE_EFM_CMD_ERROR_MASK));
}

/*!
 * @brief Check whether the last flash command has completed.
 *
 * Reads the DONE bit from the EFM status register. This flag is set by
 * hardware when a flash command (erase, program, etc.) finishes execution.
 *
 * @param[in] efmBase  Pointer to the EFM peripheral base address.
 * @return Command completion state:
 *         - true:  Flash command has completed.
 *         - false: Flash command is still in progress.
 */
static inline bool FLASH_GetDoneStatus(EFM_Type *efmBase)
{
    return (bool)(EFM_STS_DONE_MASK == (efmBase->STS & EFM_STS_DONE_MASK));
}

/*! @} */ /* End of Status Query */

/*******************************************************************************
 * Error Flag Management
 ******************************************************************************/
/*!
 * @name Error Flag Management
 * @brief Functions for clearing status flags and managing the read-collision
 *        error indicator.
 *
 * Status flags are write-1-to-clear (W1C): writing the corresponding mask
 * bit to the STS register clears the flag.
 * @{
 */

/*!
 * @brief Clear all command error flags.
 *
 * Writes the error mask to the STS register to clear all pending command
 * error flags (W1C operation).
 *
 * @param[in] efmBase  Pointer to the EFM peripheral base address.
 */
static inline void FLASH_ClearErrorFlags(EFM_Type *efmBase)
{
    efmBase->STS = FEATURE_EFM_CMD_ERROR_MASK;
}

/*!
 * @brief Clear the command-done status flag.
 *
 * Writes the DONE mask to the STS register to clear the done flag
 * (W1C operation). Typically called before launching a new command or
 * inside the done interrupt handler.
 *
 * @param[in] efmBase  Pointer to the EFM peripheral base address.
 */
static inline void FLASH_ClearDoneStatusFlag(EFM_Type *efmBase)
{
    efmBase->STS = EFM_STS_DONE_MASK;
}

/*!
 * @brief Check whether a read-collision error has occurred.
 *
 * A read-collision error is flagged when the CPU attempts to read from a
 * flash block that is currently being erased or programmed.
 *
 * @param[in] efmBase  Pointer to the EFM peripheral base address.
 * @return Read-collision state:
 *         - true:  Collision error detected.
 *         - false: No collision error detected.
 */
static inline bool FLASH_GetReadCollisionFlag(EFM_Type *efmBase)
{
    return (EFM_STS_ACCERR_MASK == (efmBase->STS & EFM_STS_ACCERR_MASK));
}

/*!
 * @brief Clear the read-collision error flag.
 *
 * Writes the ACCERR mask to the STS register to clear the read-collision
 * flag (W1C operation).
 *
 * @param[in] efmBase  Pointer to the EFM peripheral base address.
 */
static inline void FLASH_ClearReadCollisionFlag(EFM_Type *efmBase)
{
    efmBase->STS = EFM_STS_ACCERR_MASK;
}

/*! @} */ /* End of Error Flag Management */

/*******************************************************************************
 * ECC Fault Detection
 ******************************************************************************/
/*!
 * @name ECC Fault Detection
 * @brief Functions for detecting and clearing ECC (Error Correcting Code)
 *        fault flags during flash read operations.
 *
 * - **Single-bit fault**: A recoverable ECC error — the hardware can
 *   correct the data automatically, but the flag indicates data degradation.
 * - **Double-bit fault**: An unrecoverable ECC error — the data cannot be
 *   corrected and must be treated as corrupted.
 * @{
 */

/*!
 * @brief Check whether a single-bit (recoverable) ECC fault was detected.
 *
 * This flag is set when a single-bit ECC error is detected during a valid
 * flash read access from the platform flash controller. The data is
 * automatically corrected by hardware.
 *
 * @param[in] efmBase  Pointer to the EFM peripheral base address.
 * @return Single-bit fault state:
 *         - true:  Single-bit fault detected.
 *         - false: No single-bit fault detected.
 */
static inline bool FLASH_GetSingleBitFaultFlag(EFM_Type *efmBase)
{
    return (EFM_STS_RECOVERR_MASK == (efmBase->STS & EFM_STS_RECOVERR_MASK));
}

/*!
 * @brief Clear the single-bit ECC fault flag.
 *
 * Writes the RECOVERR mask to the STS register to clear the single-bit
 * fault flag (W1C operation).
 *
 * @param[in] efmBase  Pointer to the EFM peripheral base address.
 */
static inline void FLASH_ClearSingleBitFaultFlag(EFM_Type *efmBase)
{
    efmBase->STS = EFM_STS_RECOVERR_MASK;
}

/*!
 * @brief Check whether a double-bit (unrecoverable) ECC fault was detected.
 *
 * This flag is set when a double-bit ECC error is detected during a valid
 * flash read access from the platform flash controller. The data cannot
 * be corrected and should be treated as corrupted.
 *
 * @param[in] efmBase  Pointer to the EFM peripheral base address.
 * @return Double-bit fault state:
 *         - true:  Double-bit fault detected.
 *         - false: No double-bit fault detected.
 */
static inline bool FLASH_GetDoubleBitFaultFlag(EFM_Type *efmBase)
{
    return (EFM_STS_UNRECOVERR_MASK == (efmBase->STS & EFM_STS_UNRECOVERR_MASK));
}

/*!
 * @brief Clear the double-bit ECC fault flag.
 *
 * Writes the UNRECOVERR mask to the STS register to clear the double-bit
 * fault flag (W1C operation).
 *
 * @param[in] efmBase  Pointer to the EFM peripheral base address.
 */
static inline void FLASH_ClearDoubleBitFaultFlag(EFM_Type *efmBase)
{
    efmBase->STS = EFM_STS_UNRECOVERR_MASK;
}

/*! @} */ /* End of ECC Fault Detection */

/*******************************************************************************
 * Command Control
 ******************************************************************************/
/*!
 * @name Command Control
 * @brief Functions for writing flash command codes to the EFM command register.
 * @{
 */

/*!
 * @brief Write a command code to the EFM command register.
 *
 * Loads the specified command code into the EFM CMD register to initiate
 * a flash operation (erase, program, etc.). The command code values are
 * defined by the device-specific FEATURE_EFM_*_CMD_CODE macros.
 *
 * @param[in] efmBase  Pointer to the EFM peripheral base address.
 * @param[in] cmdCode  Flash command code to execute.
 *
 * @warning This function writes directly to the EFM command register.
 *          The EFM unlock sequence must be performed before issuing any
 *          command code. In normal usage this is handled internally by
 *          FLASH_LaunchCommandSequence().
 */
static inline void FLASH_SetCmdCode(EFM_Type *efmBase, uint32_t cmdCode)
{
    efmBase->CMD = EFM_CMD_CMD(cmdCode);
}

/*! @} */ /* End of Command Control */

#ifdef __cplusplus
}
#endif

/*! @} */ /* End of flash_hw_access group */

#endif /* FLASH_HW_ACCESS_H */

/*******************************************************************************
 * EOF
 ******************************************************************************/
