/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file fee.h
 * @version 1.4.1
 *
 * @brief FEE Driver — public API for flash-backed EEPROM emulation.
 *
 * This header declares the asynchronous Flash EEPROM Emulation (FEE) services
 * that store logical data blocks in flash memory through the FLS driver. The
 * module tracks block headers, cluster-group layout, reserved space for
 * immediate blocks, and background maintenance jobs such as cluster scan and
 * swap.
 *
 * The public interface is organized into five categories:
 *   - Module constants and configuration data types
 *   - Initialization and runtime control
 *   - Job status and version reporting
 *   - Data transfer for logical blocks
 *   - Block-maintenance and FLS callback hooks
 *
 * @note Configure the underlying FLS instance so its job callbacks invoke
 *       Fee_JobEndNotification() and Fee_JobErrorNotification().
 * @note Call Fee_MainFunction() periodically after Fee_Init() until
 *       Fee_GetStatus() returns MEMIF_IDLE for the accepted job.
 */

#ifndef FEE_H
#define FEE_H

#ifdef __cplusplus
extern "C" {
#endif
/*=================================================================================================
 *                                        INCLUDE FILES
=================================================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include "fls_async_driver.h"


/*!
 * @defgroup fee_driver FEE Driver
 * @ingroup fee
 * @brief Public API for logical EEPROM-style storage on top of flash memory.
 * @details The FEE driver exposes asynchronous read, write, invalidation, and
 *          maintenance services for configured logical blocks. It uses cluster
 *          rotation and block metadata in flash to emulate EEPROM semantics
 *          while relying on the FLS layer for the physical flash operations.
 * @{
 */

/*==================================================================================================
 *                                       DEFINES AND MACROS
==================================================================================================*/
/*!
 * @name Module Constants
 * @brief Fixed parameters used by the FEE runtime and metadata layout.
 * @{
 */

/*! @brief Logical block alignment granularity in bytes. */
#define FEE_VIRTUAL_PAGE_SIZE                    (8U)

/*! @brief Size of the shared internal scratch buffer in bytes. */
#define FEE_DATA_BUFFER_SIZE                     (96U)

/*! @brief Erased-state byte value returned by the flash array. */
#define FEE_ERASED_VALUE                         (0xffU)

/*! @brief Marker programmed into block and cluster valid flags. */
#define FEE_VALIDATED_VALUE                      (0x81U)

/*! @brief Marker programmed into block and cluster invalid flags. */
#define FEE_INVALIDATED_VALUE                    (0x18U)

/*! @} */
/*==================================================================================================
 *                                         EXTERNAL CONSTANTS
==================================================================================================*/


/*==================================================================================================
 *                                             ENUMS
==================================================================================================*/

/*!
 * @brief Runtime capacity snapshot for one configured cluster group.
 *
 * This structure is used internally and by configuration tooling to report the
 * currently usable space, metadata overhead, and swap count for a cluster
 * group.
 */
typedef struct
{
    Fls_AddressType ClusterTotalSpace;   /*!< Total flash space managed by the cluster group. */
    Fls_AddressType ClusterFreeSpace;    /*!< Remaining writable space in the active cluster. */
    uint16_t BlockHeaderOverhead;        /*!< Metadata bytes reserved per logical block header. */
    uint16_t VirtualPageSize;            /*!< Effective alignment unit used by the FEE layout. */
    uint32_t NumberOfSwap;               /*!< Number of cluster-swap operations completed. */
} Fee_ClusterGroupRuntimeInfoType;

/*!
 * @brief Ownership classification for a configured logical block.
 *
 * | Value                   | Description                                      |
 * |-------------------------|--------------------------------------------------|
 * | FEE_PROJECT_SHARED      | Block content is shared across all projects.     |
 * | FEE_PROJECT_APPLICATION | Block belongs to the application image.          |
 * | FEE_PROJECT_BOOTLOADER  | Block belongs to the bootloader image.           |
 * | FEE_PROJECT_RESERVED    | Reserved value that must not be used directly.   |
 */
typedef enum
{
    FEE_PROJECT_SHARED      = 0x01,  /*!< Block is shared across all software images. */
    FEE_PROJECT_APPLICATION = 0x02,  /*!< Block is reserved for the application image. */
    FEE_PROJECT_BOOTLOADER  = 0x03,  /*!< Block is reserved for the bootloader image. */
    FEE_PROJECT_RESERVED    = 0xFF   /*!< Reserved encoding. */
} Fee_BlockAssignmentType;

/*!
 * @brief Logical block configuration entry.
 *
 * Each entry describes one user-visible logical block, including its block ID,
 * payload size, owning cluster group, immediate-data behavior, and assignment
 * to a software image.
 */
typedef struct
{
    uint16_t BlockNumber;                    /*!< Logical block identifier used by the public APIs. */
    uint16_t BlockSize;                      /*!< Configured payload size of the logical block. */
    uint8_t ClrGrp;                          /*!< Cluster-group index that stores the block. */
    bool ImmediateData;                      /*!< Set to true for an immediate-data block. */
    Fee_BlockAssignmentType BlockAssignment; /*!< Software image that owns the block content. */
} Fee_BlockConfigType;

/*!
 * @brief Flash range assigned to a single FEE cluster.
 */
typedef struct
{
    Fls_AddressType StartAddr;  /*!< Start address of the cluster in flash. */
    Fls_LengthType Length;      /*!< Total cluster size in bytes. */
} Fee_ClusterType;

/*!
 * @brief Cluster-group configuration entry.
 *
 * A cluster group contains one or more physical clusters and reserves optional
 * space for immediate blocks that must remain writable during swap.
 */
typedef struct
{
    const Fee_ClusterType *const ClrPtr;  /*!< Pointer to the cluster array owned by the group. */
    uint32_t ClrCount;                    /*!< Number of clusters in the group. */
    Fls_LengthType ReservedSize;          /*!< Bytes reserved for immediate-block handling. */
} Fee_ClusterGroupType;

/*!
 * @brief AUTOSAR compatibility alias for the block-configuration type.
 *
 * This alias is kept for compatibility with integration code that expects the
 * classic Fee_ConfigType name.
 */
typedef Fee_BlockConfigType Fee_ConfigType;

/*!
 * @brief Serialized block-header representation used inside flash metadata.
 */
typedef struct
{
    uint16_t BlockNumber;    /*!< Logical block identifier stored in the header. */
    uint16_t Length;         /*!< Payload length encoded in the header. */
    bool ImmediateBlock;     /*!< Set to true when the block is immediate data. */
} Fee_BlockType;

/*!
 * @brief Serialized cluster-header representation stored at cluster start.
 */
typedef struct
{
    uint32_t ClrID;                /*!< Monotonic cluster identifier. */
    Fls_AddressType StartAddr;     /*!< Cluster start address in the FLS address space. */
    Fls_LengthType Length;         /*!< Cluster length in bytes. */
} Fee_ClusterHeaderType;

/*!
 * @brief Top-level FEE module configuration.
 *
 * Supply one initialized instance of this structure to Fee_Init() so the
 * driver can discover the block table, cluster layout, and underlying flash
 * driver configuration.
 */
typedef struct
{
    uint16_t blockCnt;                            /*!< Number of logical blocks in blockConfigPtr. */
    uint8_t clusterCnt;                           /*!< Number of configured cluster groups. */
    const Fee_ClusterGroupType *clusterConfigPtr; /*!< Pointer to the cluster-group configuration table. */
    const Fee_BlockConfigType *blockConfigPtr;    /*!< Pointer to the logical block configuration table. */
    const Fls_ConfigType *flashConfigPtr;         /*!< Pointer to the FLS configuration used by FEE. */
} Fee_ModuleUserConfig_t;

/*==================================================================================================
*                                     FUNCTION PROTOTYPES
==================================================================================================*/

/*!
 * @name Initialization & Runtime Control
 * @brief Services for starting the driver, polling its state machine, and
 *        canceling accepted jobs.
 * @{
 */

/*!
 * @brief Initialize the FEE driver with the selected configuration.
 *
 * The driver stores @a ConfigPtr, initializes the underlying FLS driver, clears
 * internal runtime state, and schedules the initial cluster scan.
 *
 * @param[in] ConfigPtr  Pointer to the module configuration structure.
 *
 * @pre ConfigPtr must not be NULL.
 * @pre The driver must currently be in MEMIF_UNINIT state.
 * @post The initial scan job is pending until Fee_MainFunction() completes it.
 */
void Fee_Init(const Fee_ModuleUserConfig_t *ConfigPtr);

/*!
 * @brief Advance the FEE background state machine.
 *
 * This function services the outstanding FEE job and any required internal
 * scan, swap, validation, or metadata updates. It also advances the FLS main
 * function used by the flash backend.
 *
 * @pre Fee_Init() must have been called.
 * @note Call this function periodically until Fee_GetStatus() returns
 *       MEMIF_IDLE after a job is accepted.
 */
void Fee_MainFunction(void);

/*!
 * @brief Cancel the current FEE job when the driver is busy.
 *
 * @return E_OK if the request was accepted, or E_NOT_OK if the driver is still
 *         uninitialized.
 *
 * @note Canceling a busy job propagates the cancel request to the underlying
 *       FLS driver.
 */
Std_ReturnType Fee_Cancel(void);

/*! @} */

/*!
 * @name Job Status & Version Information
 * @brief Services for observing runtime state and optional version metadata.
 * @{
 */

/*!
 * @brief Query the status of the most recently accepted FEE job.
 *
 * @return Result code reported by the FEE state machine.
 */
MemIf_JobResultType Fee_GetJobResult(void);

/*!
 * @brief Get the current module status.
 *
 * @return MEMIF_UNINIT before initialization, MEMIF_BUSY while a job is in
 *         progress, or MEMIF_IDLE when the driver is ready to accept a new job.
 */
MemIf_StatusType Fee_GetStatus(void);

#if defined(FEE_VERSION_INFO_API) && (FEE_VERSION_INFO_API == STD_ON)
/*!
 * @brief Return the compiled version information of the FEE module.
 *
 * @param[out] VersionInfoPtr  Pointer to the destination version-info structure.
 */
void Fee_GetVersionInfo(Std_VersionInfoType *VersionInfoPtr);
#endif /* FEE_VERSION_INFO_API == STD_ON */

/*! @} */

/*!
 * @name Data Transfer
 * @brief Asynchronous services for reading and writing configured logical blocks.
 * @{
 */

/*!
 * @brief Start an asynchronous read from a logical block.
 *
 * @param[in] BlockNumber    Logical block identifier defined by the board
 *                           configuration.
 * @param[in] BlockOffset    Byte offset inside the logical block.
 * @param[out] DataBufferPtr Pointer to the caller-provided receive buffer.
 * @param[in] Length         Number of bytes to read.
 * @return E_OK if the read job was accepted, otherwise E_NOT_OK.
 *
 * @pre Fee_GetStatus() must return MEMIF_IDLE before the request is issued.
 * @note The requested offset and length must stay within the configured block
 *       size.
 */
Std_ReturnType Fee_Read(uint16_t BlockNumber,
                        uint16_t BlockOffset,
                        uint8_t *DataBufferPtr,
                        uint16_t Length);

/*!
 * @brief Start an asynchronous write to a logical block.
 *
 * @param[in] BlockNumber    Logical block identifier defined by the board
 *                           configuration.
 * @param[in] DataBufferPtr  Pointer to the source buffer that contains the
 *                           complete block payload.
 * @return E_OK if the write job was accepted, otherwise E_NOT_OK.
 *
 * @pre Fee_GetStatus() must return MEMIF_IDLE before the request is issued.
 * @note The source buffer must provide at least the configured block size in
 *       bytes for the selected block.
 */
Std_ReturnType Fee_Write(uint16_t BlockNumber, const uint8_t *DataBufferPtr);

/*! @} */

/*!
 * @name Block Maintenance
 * @brief Services for invalidating or pre-erasing logical blocks in standard mode.
 * @{
 */

#if !defined(FEE_LIGHT_MODE) || (FEE_LIGHT_MODE == 0U)
/*!
 * @brief Invalidate the latest instance of a logical block.
 *
 * @param[in] BlockNumber  Logical block identifier to invalidate.
 * @return E_OK if the invalidation job was accepted, otherwise E_NOT_OK.
 *
 * @note This API is not available when FEE_LIGHT_MODE is enabled.
 */
Std_ReturnType Fee_InvalidateBlock(uint16_t BlockNumber);
#endif

#if !defined(FEE_LIGHT_MODE) || (FEE_LIGHT_MODE == 0U)
/*!
 * @brief Erase and re-prepare an immediate block for the next update.
 *
 * @param[in] BlockNumber  Logical block identifier to erase.
 * @return E_OK if the erase-immediate job was accepted, otherwise E_NOT_OK.
 *
 * @note This API is not available when FEE_LIGHT_MODE is enabled.
 */
Std_ReturnType Fee_EraseImmediateBlock(uint16_t BlockNumber);
#endif

/*! @} */

/*!
 * @name Flash Callback Hooks
 * @brief Callbacks that must be wired into the FLS configuration used by FEE.
 * @{
 */

/*!
 * @brief Report successful completion of the current asynchronous FLS step.
 *
 * The configured FLS job-end callback must call this function so the FEE job
 * scheduler can continue the next state transition.
 */
void Fee_JobEndNotification(void);

/*!
 * @brief Report failure of the current asynchronous FLS step.
 *
 * The configured FLS job-error callback must call this function so the FEE
 * state machine can enter its error-handling path.
 */
void Fee_JobErrorNotification(void);

/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* FEE_H */

/** @} */
