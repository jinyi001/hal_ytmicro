/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file fee.h
 * @version 1.4.0
 */

#ifndef FEE_H
#define FEE_H

#ifdef __cplusplus
extern "C"{
#endif
/*=================================================================================================
 *                                        INCLUDE FILES
=================================================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include "fls_async_driver.h"


/*!
 * @addtogroup fee_driver
 * @ingroup fee
 * @{
 */

/*==================================================================================================
 *                                       DEFINES AND MACROS
==================================================================================================*/
/** @brief The size in bytes to which logical blocks shall be aligned */
#define FEE_VIRTUAL_PAGE_SIZE                    (8U)

/** @brief Management overhead per logical block in bytes */
#define FEE_BLOCK_OVERHEAD                       (32U)

/** @brief Management overhead per logical cluster in bytes */
#define FEE_CLUSTER_OVERHEAD                     (32U)

/** @brief Size of the data buffer in bytes */
#define FEE_DATA_BUFFER_SIZE                     (96U)

/** @brief The contents of an erased flash memory cell */
#define FEE_ERASED_VALUE                         (0xffU)

/** @brief Value of the block and cluster validation flag */
#define FEE_VALIDATED_VALUE                      (0x81U)

/** @brief Value of the block and cluster invalidation flag */
#define FEE_INVALIDATED_VALUE                    (0x18U)
/*==================================================================================================
 *                                         EXTERNAL CONSTANTS
==================================================================================================*/


/*==================================================================================================
 *                                             ENUMS
==================================================================================================*/

/**
* @brief        Fee cluster group run-time Information
*/
typedef struct
{
    Fls_AddressType  ClusterTotalSpace;      /*!< Total space in the selected cluster group */
    Fls_AddressType  ClusterFreeSpace;       /*!< Free space in the selected cluster group */
    uint16_t           BlockHeaderOverhead;  /*!< Block Overhead (header valid and inval flag)*/
    uint16_t           VirtualPageSize;      /*!< Fee Virtual Page Size */
    uint32_t           NumberOfSwap;         /*!< Number of cluster swap performed in the
                                                      selected cluster group */
} Fee_ClusterGroupRuntimeInfoType;

/**
* @brief Fee block assignment type
*/
typedef enum
{
    FEE_PROJECT_SHARED          = 0x01,     /*!< block is used for all the projects */
    FEE_PROJECT_APPLICATION     = 0x02,     /*!< block is used for the application project */
    FEE_PROJECT_BOOTLOADER      = 0x03,     /*!< block is used for the bootloader project */
    FEE_PROJECT_RESERVED        = 0xFF      /*!< the value is reserved */
} Fee_BlockAssignmentType;

/**
* @brief        Fee block configuration structure
* @implements   FeeBlockConfiguration_Object
*/
typedef struct
{
    uint16_t BlockNumber;                         /*!< Fee block number */
    uint16_t BlockSize;                           /*!< Size of Fee block in bytes */
    uint8_t ClrGrp;                               /*!< Index of cluster group the Fee block belongs to */
    bool ImmediateData;                           /*!< true if immediate data block */
    Fee_BlockAssignmentType BlockAssignment;      /*!< specifies which project uses this block */
} Fee_BlockConfigType;

/**
* @brief Fee cluster configuration structure
*/
typedef struct
{
    Fls_AddressType StartAddr;   /*!< Address of Fee cluster in flash */
    Fls_LengthType Length;       /*!< Size of Fee cluster in bytes */
} Fee_ClusterType;

/**
* @brief Fee cluster group configuration structure
* @implements   Fee_ClusterGroupType_struct
*/
typedef struct
{
    const Fee_ClusterType *const ClrPtr;    /*!< Pointer to array of Fee cluster configurations */
    uint32_t ClrCount;                      /*!< Number of clusters in cluster group */
    Fls_LengthType ReservedSize;            /*!< Size of reserved area in the given cluster group (memory occupied by immediate blocks) */
} Fee_ClusterGroupType;

/**
* @brief        Fee Configuration type is a stub type, not used, but required by ASR 4.2.2.
*/
typedef Fee_BlockConfigType Fee_ConfigType;

/**
* @brief        Fee block header configuration structure. This consists of block number and length of block and Type of Fee block
*/
typedef struct
{
    uint16_t  BlockNumber;     /*!< Number of block */
    uint16_t  Length;          /*!< Length of block */
    bool ImmediateBlock;    /*!< Type of Fee block. Set to true for immediate block */
} Fee_BlockType;

/**
* @brief        Fee cluster header configuration structure.
*/
typedef struct
{
    uint32_t                        ClrID;            /*!< 32-bit cluster ID */
    Fls_AddressType               StartAddr;          /*!< Start address of Fee cluster in Fls address space */
    Fls_LengthType                Length;             /*!< Length of Fee cluster in bytes */
} Fee_ClusterHeaderType;

/**
* @brief        Fee module configuration structure.
*/
typedef struct
{
	uint16_t blockCnt;                              /*!< Number of blocks configured */
	uint8_t clusterCnt;                             /*!< Number of clusters configured */
	Fee_ClusterGroupType const *clusterConfigPtr;   /*!< Pointer to array of Fee cluster group configurations */
	Fee_BlockConfigType const *blockConfigPtr;      /*!< Pointer to array of Fee block configurations */
	Fls_ConfigType const *flashConfigPtr;           /*!< Pointer to Fls configuration structure for Fee*/
}Fee_ModuleUserConfig_t;

/*==================================================================================================
*                                     FUNCTION PROTOTYPES
==================================================================================================*/


/**
 * @brief            Service to erase a logical block.
 * @details          
 * @param[in]        BlockNumber Number of logical block, also denoting start address of that block in EEPROM.
 * @return           Std_ReturnType
 */
Std_ReturnType Fee_EraseImmediateBlock (uint16_t BlockNumber);

/**
 * @brief            Service to query the result of the last accepted job issued by the upper layer software.
 * @details          
 * @return           MemIf_JobResultType
 */
MemIf_JobResultType Fee_GetJobResult (void);

/**
 * @brief            Service to return the status.
 * @details          
 * @return           MemIf_StatusType
 */
MemIf_StatusType Fee_GetStatus (void);

#if defined(FEE_VERSION_INFO_API) && (FEE_VERSION_INFO_API == STD_ON)
/**
 * @brief            Service to return the version information of the FEE module.
 * @details          
 * @param[out]       VersionInfoPtr Pointer to standard version information structure.
 * @return           void
 */
void Fee_GetVersionInfo (Std_VersionInfoType * VersionInfoPtr);
#endif /* FEE_VERSION_INFO_API == STD_ON */

/**
 * @brief            Service to initialize the FEE module.
 * @details          
 * @param[in]        ConfigPtr Pointer to the selected configuration set.
 * @return           void
 */
void Fee_Init (const Fee_ModuleUserConfig_t * ConfigPtr);

/**
 * @brief            Service to invalidate a logical block.
 * @details          
 * @param[in]        BlockNumber Number of logical block, also denoting start address of that block in flash memory.
 * @return           Std_ReturnType
 */
Std_ReturnType Fee_InvalidateBlock (uint16_t BlockNumber);

/**
 * @brief            Service to report to this module the successful end of an asynchronous operation.
 * @details          
 * @return           void
 */
void Fee_JobEndNotification (void);

/**
 * @brief            Service to report to this module the failure of an asynchronous operation.
 * @details          
 * @return           void
 */
void Fee_JobErrorNotification (void);

/**
 * @brief            Service to handle the requested read / write / erase jobs and the internal management operations.
 * @details          
 * @return           void
 */
void Fee_MainFunction (void);

/**
 * @brief            Service to initiate a read job.
 * @details          
 * @param[in]        BlockNumber Number of logical block, also denoting start address of that block in flash memory.
 * @param[in]        BlockOffset Read address offset inside the block
 * @param[out]       DataBufferPtr Pointer to data buffer
 * @param[in]        Length Number of bytes to read
 * @return           Std_ReturnType
 */
Std_ReturnType Fee_Read (uint16_t BlockNumber, uint16_t BlockOffset, uint8_t * DataBufferPtr, uint16_t Length);

/**
 * @brief            Service to initiate a write job.
 * @details          
 * @param[in]        BlockNumber Number of logical block, also denoting start address of that block in EEPROM.
 * @param[in]        DataBufferPtr Pointer to data buffer
 * @return           Std_ReturnType
 */
Std_ReturnType Fee_Write (uint16_t BlockNumber, const uint8_t * DataBufferPtr);

/**
 * @brief            Service to cancel the current job.
 * @details          
 * @return           Std_ReturnType
 */
Std_ReturnType Fee_Cancel(void);

#ifdef __cplusplus
}
#endif

#endif /* FEE_H */

/** @} */
