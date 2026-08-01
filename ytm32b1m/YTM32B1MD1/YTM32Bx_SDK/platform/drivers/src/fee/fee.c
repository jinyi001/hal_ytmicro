/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file fee.c
 * @version 1.4.1
 *
 * @brief FEE driver implementation and internal flash job scheduler.
 *
 * This file implements the public Fee_* services together with the helper
 * routines that serialize metadata, scan configured clusters, migrate valid
 * blocks during cluster swap, and coordinate asynchronous flash operations
 * through the FLS backend.
 *
 * The implementation is organized into four main areas:
 *   - metadata serialization and lookup helpers
 *   - cluster scan and swap state-machine jobs
 *   - flash read, write, erase, and validation adapters
 *   - public API entry points and FLS callback handlers
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 2461 Rule 14.2: A for loop shall be well-formed.
 * 
 * PRQA S 2843 Rule 18.1: A pointer resulting from arithmetic on a pointer operand shall address an
 *                        element of the same array as that pointer operand
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
 *                                        INCLUDE FILES
==================================================================================================*/
#include "fee.h"
#include "fee_config.h"

/*==================================================================================================
 *                                       LOCAL MACROS
==================================================================================================*/
/*! @brief Metadata overhead reserved for each logical block. */
#if defined(FEE_LIGHT_MODE) && (FEE_LIGHT_MODE == 1U)
#define FEE_BLOCK_OVERHEAD                       (16U)
#else
#define FEE_BLOCK_OVERHEAD                       (32U)
#endif

/*! @brief Metadata overhead reserved for each physical cluster. */
#if defined(FEE_LIGHT_MODE) && (FEE_LIGHT_MODE == 1U)
#define FEE_CLUSTER_OVERHEAD                     (16U)
#else
#define FEE_CLUSTER_OVERHEAD                     (32U)
#endif

/* Offset of the validation-state area within a serialized cluster header. */
#if defined(FEE_LIGHT_MODE) && (FEE_LIGHT_MODE == 1U)
#define FEE_CLUSTER_HEADER_STATUS_OFFSET     (FEE_CLUSTER_OVERHEAD - FEE_VIRTUAL_PAGE_SIZE)
#else
#define FEE_CLUSTER_HEADER_STATUS_OFFSET     (FEE_CLUSTER_OVERHEAD - (2U * FEE_VIRTUAL_PAGE_SIZE))
#endif

/* Offset of the validation-state area within a serialized block header. */
#if defined(FEE_LIGHT_MODE) && (FEE_LIGHT_MODE == 1U)
#define FEE_BLOCK_HEADER_STATUS_OFFSET      (FEE_BLOCK_OVERHEAD - FEE_VIRTUAL_PAGE_SIZE)
#else
#define FEE_BLOCK_HEADER_STATUS_OFFSET      (FEE_BLOCK_OVERHEAD - (2U * FEE_VIRTUAL_PAGE_SIZE))
#endif

/* Size of the block validation area in light mode or standard mode. */
#if defined(FEE_LIGHT_MODE) && (FEE_LIGHT_MODE == 1U)
#define FEE_BLOCK_VALID_FLAG_AREA_SIZE      (FEE_VIRTUAL_PAGE_SIZE)
#else
#define FEE_BLOCK_VALID_FLAG_AREA_SIZE      (2U * FEE_VIRTUAL_PAGE_SIZE)
#endif

#ifndef FEE_ENTER_CRITICAL_SECTION
#define FEE_ENTER_CRITICAL_SECTION()        Fee_EnterCriticalSection()
#endif

#ifndef FEE_EXIT_CRITICAL_SECTION
#define FEE_EXIT_CRITICAL_SECTION(state)    Fee_ExitCriticalSection((state))
#endif

/*==================================================================================================
 *                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/
/*!
 * @brief Decoded state of a logical block header stored in flash.
 */
typedef enum
{
    FEE_BLOCK_VALID = 0,         /**< Block contains the newest valid payload. */
    FEE_BLOCK_INVALID,           /**< Block was superseded or explicitly invalidated. */
    FEE_BLOCK_INCONSISTENT,      /**< Block metadata exists but the payload is unusable. */
    FEE_BLOCK_HEADER_INVALID,    /**< Block header cannot be parsed correctly. */
    FEE_BLOCK_INVALIDATED,       /**< Block carries the explicit invalidation marker. */
    FEE_BLOCK_HEADER_BLANK,      /**< Block-header slot is blank and marks the parse boundary. */
    FEE_BLOCK_INCONSISTENT_COPY, /**< Block copy failed while a swap operation was active. */
    FEE_BLOCK_NEVER_WRITTEN      /**< No valid payload has ever been written for the block. */
} Fee_BlockStatusType;

/*!
 * @brief Decoded state of a cluster header stored in flash.
 */
typedef enum
{
    FEE_CLUSTER_VALID = 0,       /**< Cluster is formatted and currently valid. */
    FEE_CLUSTER_INVALID,         /**< Cluster is marked invalid and cannot accept data. */
    FEE_CLUSTER_INCONSISTENT,    /**< Cluster header or status is partially programmed. */
    FEE_CLUSTER_HEADER_INVALID   /**< Cluster header cannot be parsed correctly. */
} Fee_ClusterStatusType;

/*!
 * @brief Internal job identifiers processed by the FEE state machine.
 */
typedef enum
{
    /* Jobs entered from Fee_Read(). */
    FEE_JOB_READ = 0,                /**< Read a logical block payload. */

    /* Jobs entered from Fee_Write(). */
    FEE_JOB_WRITE,                   /**< Prepare the block-write sequence. */
    FEE_JOB_WRITE_DATA,              /**< Program aligned block payload data. */
    FEE_JOB_WRITE_UNALIGNED_DATA,    /**< Program any trailing unaligned payload bytes. */
    FEE_JOB_WRITE_VALIDATE,          /**< Commit the block validation marker. */
    FEE_JOB_WRITE_DONE,              /**< Finalize a completed block write. */

    /* Jobs entered from Fee_InvalidateBlock(). */
    FEE_JOB_INVAL_BLOCK,             /**< Program the block invalidation marker. */
    FEE_JOB_INVAL_BLOCK_DONE,        /**< Finalize logical block invalidation. */

    /* Jobs entered from Fee_EraseImmediateBlock(). */
    FEE_JOB_ERASE_IMMEDIATE,         /**< Re-prepare the target immediate block area. */

    /* Jobs entered from Fee_Init(). */
    FEE_JOB_INT_SCAN,                /**< Start the initialization scan pipeline. */
    FEE_JOB_INT_SCAN_CLR,            /**< Scan the active cluster in the current group. */
    FEE_JOB_INT_SCAN_CLR_HDR_PARSE,  /**< Parse the serialized cluster header. */
    FEE_JOB_INT_SCAN_CLR_FMT,        /**< Format the first cluster in the group. */
    FEE_JOB_INT_SCAN_CLR_FMT_DONE,   /**< Finalize cluster formatting. */
    FEE_JOB_INT_SCAN_BLOCK_HDR_PARSE,/**< Parse serialized block headers. */

    /* Internal cluster-swap jobs. */
    FEE_JOB_INT_SWAP_CLR_FMT,        /**< Format the target cluster before swap. */
    FEE_JOB_INT_SWAP_BLOCK,          /**< Select the next block that must be migrated. */
    FEE_JOB_INT_SWAP_DATA_READ,      /**< Read source block data into the shared buffer. */
    FEE_JOB_INT_SWAP_DATA_WRITE,     /**< Write buffered block data into the target cluster. */
    FEE_JOB_INT_SWAP_CLR_VLD_DONE,   /**< Finalize validation of the target cluster. */

    /* Terminal scheduler states. */
    FEE_JOB_DONE,                    /**< No further job step remains to be scheduled. */

    /* Reserved bridge to the flash backend. */
    FEE_JOB_SETMODE                  /**< Propagate the requested mode to FLS. */
} Fee_JobType;

/*!
 * @brief Runtime cursor values for one cluster group.
 */
typedef struct
{
    Fls_AddressType DataAddrIt;  /**< Current data-write cursor inside the cluster group. */
    Fls_AddressType HdrAddrIt;   /**< Current header-write cursor inside the cluster group. */
    uint32_t ActClrID;           /**< Identifier of the active cluster. */
    uint8_t ActClr;              /**< Index of the active cluster. */
} Fee_ClusterGroupInfoType;

/*!
 * @brief Runtime state tracked for each configured logical block.
 */
typedef struct
{
    Fls_AddressType DataAddr;        /**< Flash address of the newest block payload. */
    Fls_AddressType InvalidAddr;     /**< Flash address of the invalidation marker. */
    Fee_BlockStatusType BlockStatus; /**< Current decoded block state. */
} Fee_BlockInfoType;

/*==================================================================================================
 *                                      GLOBAL CONSTANTS
==================================================================================================*/
static Fee_ModuleUserConfig_t const * Fee_ConfigPtr = NULL_PTR;
/*==================================================================================================
 *                                      GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
 *                                      LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
 *                                      LOCAL VARIABLES
==================================================================================================*/

/*! @brief Indicates whether the current request must trigger a cluster swap. */
static bool bSwapToBePerformed;

/*! @brief Destination buffer supplied by the active read request. */
static uint8_t *Fee_pJobReadDataDestPtr = (uint8_t *)NULL_PTR;

/*! @brief Source buffer supplied by the active write request. */
static const uint8_t *Fee_pJobWriteDataDestPtr = (uint8_t *)NULL_PTR;

/*! @brief Shared internal data buffer used by read, write, scan, and swap jobs. */
static uint8_t Fee_aDataBuffer[FEE_DATA_BUFFER_SIZE] = {0};
/*!
 * @brief Internal cluster-group iterator used by scan and swap jobs.
 *
 * This cursor is valid only while the internal scan or swap state machine is
 * active.
 */
static uint8_t Fee_uJobIntClrGrpIt;

/*! @brief Internal cluster iterator used by scan and swap jobs. */
static uint8_t Fee_uJobIntClrIt;


/*! @brief Logical block index associated with the active public request. */
static uint16_t Fee_uJobBlockIndex;
/*! @brief Internal block iterator used while migrating blocks during swap. */
static uint16_t Fee_uJobIntBlockIt;

/*!
 * @brief Bitset that tracks which logical blocks currently touch reserved areas.
 */
static uint32_t Fee_aReservedAreaTouched[(FEE_CRT_CFG_NR_OF_BLOCKS + ((sizeof(uint32_t) * 8U)-1U)) / (sizeof(uint32_t) * 8U)];

/*! @brief Currently executing internal or public job identifier. */
static Fee_JobType Fee_eJob = FEE_JOB_DONE;

/*! @brief Public job that triggered the active internal maintenance pipeline. */
static Fee_JobType Fee_eJobIntOriginalJob = FEE_JOB_DONE;

/*! @brief Current MEMIF runtime state reported by the module. */
static MemIf_StatusType Fee_eModuleStatus = MEMIF_UNINIT;

/*! @brief Result of the most recently completed or active FEE job. */
static MemIf_JobResultType Fee_eJobResult = MEMIF_JOB_OK;
/*! @brief Requested offset inside the active read block. */
static Fls_LengthType Fee_uJobBlockOffset;

/*! @brief Requested length of the active read job. */
static Fls_LengthType Fee_uJobBlockLength;

/*!
 * @brief Internal flash-address iterator shared by scan and swap helpers.
 */
static Fls_AddressType Fee_uJobIntAddrIt;

/*! @brief Flash address of the current block header handled during swap. */
static Fls_AddressType Fee_uJobIntHdrAddr;

/*! @brief Flash address of the current block payload handled during swap. */
static Fls_AddressType Fee_uJobIntDataAddr;

/*!
 * @brief Runtime state table for all configured logical blocks.
 */
static Fee_BlockInfoType Fee_aBlockInfo[FEE_CRT_CFG_NR_OF_BLOCKS];

/*! @brief Runtime cursor table for all configured cluster groups. */
static Fee_ClusterGroupInfoType Fee_aClrGrpInfo[FEE_NUMBER_OF_CLUSTER_GROUPS];

/*==================================================================================================
 *                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

static void Fee_SerializeBlockHdr(const Fee_BlockType *BlockHder,
                                  const Fls_AddressType TargetAddress,
                                  uint8_t *BlockHdrPtr
                                 );

static uint32_t Fee_EnterCriticalSection(void);

static void Fee_ExitCriticalSection(uint32_t primaskState);

static Std_ReturnType Fee_BlankCheck(const uint8_t *TargetPtr, const uint8_t *const TargetEndPtr);

static Std_ReturnType Fee_DeserializeFlag(const uint8_t *const TargetPtr, const uint8_t FlagPattern, bool *pFlagValue);

static Fee_BlockStatusType Fee_DeserializeBlockHdr(Fee_BlockType *const BlockHder,
                                                   Fls_AddressType *const TargetAddress,
                                                   const uint8_t *BlockHdrPtr
                                                  );

static Fee_ClusterStatusType Fee_DeserializeClusterHdr(Fee_ClusterHeaderType *ClrHdr,const uint8_t *ClrHdrPtr);

static void Fee_SerializeClusterHdr(const Fee_ClusterHeaderType *ClrHdr,uint8_t *ClrHdrPtr);

static uint16_t Fee_GetBlockIndex(const uint16_t BlockNumber);

static inline uint16_t Fee_GetBlockNumber(const uint16_t BlockRuntimeInfoIndex);

static inline uint16_t Fee_GetBlockSize(const uint16_t BlockRuntimeInfoIndex);

static inline uint8_t Fee_GetBlockClusterGrp(const uint16_t BlockRuntimeInfoIndex);

static inline bool Fee_GetBlockImmediate(const uint16_t BlockRuntimeInfoIndex);

static uint16_t Fee_AlignToVirtualPageSize(uint16_t BlockSize);

static void Fee_CopyDataToPageBuffer(const uint8_t *SourcePtr, uint8_t *TargetPtr, const uint16_t Length);

static void Fee_SerializeFlag(uint8_t *TargetPtr, const uint8_t FlagPattern);

static MemIf_JobResultType Fee_JobInternalSwapClusterVld(void);

static MemIf_JobResultType Fee_JobInternalSwapBlock(void);

static MemIf_JobResultType Fee_JobInternalSwapClusterFmt(void);

static MemIf_JobResultType Fee_JobInternalSwapClusterErase(void);

static MemIf_JobResultType Fee_JobInternalSwap(void);

static MemIf_JobResultType Fee_JobInternalScanBlockHdrRead(void);

static MemIf_JobResultType Fee_JobInternalScanClusterErase(void);

static MemIf_JobResultType Fee_JobInternalScanCluster(void);

static MemIf_JobResultType Fee_JobInternalScanClusterFmt(void);

static MemIf_JobResultType Fee_JobInternalScanClusterFmtDone(void);

static MemIf_JobResultType Fee_JobInternalScanBlkHdrParse(const bool BufferValid);

static inline bool Fee_JobInternalScanClusterHdrDone(void);

static MemIf_JobResultType Fee_JobInternalScanClusterHdrRead(void);

static MemIf_JobResultType Fee_JobInternalScan(void);

static MemIf_JobResultType Fee_JobIntalScanCluHdrParse(const bool BufferValid);

static MemIf_JobResultType Fee_JobReadBlock(void);

static MemIf_JobResultType Fee_JobInternalSwapBlockVld(void);

static MemIf_JobResultType Fee_JobInternalSwapDataRead(const bool BufferValid);

static MemIf_JobResultType Fee_JobInternalSwapDataWrite(const bool BufferValid);

static MemIf_JobResultType Fee_JobInternalSwapClusterVldDone(void);

static MemIf_JobResultType Fee_JobWriteHdr(void);

static MemIf_JobResultType Fee_JobWriteBlockData(void);

static MemIf_JobResultType Fee_JobWriteBlock(void);

static MemIf_JobResultType Fee_JobWriteBlockUnalignedData(void);

static MemIf_JobResultType Fee_JobWriteBlockValidate(void);

static MemIf_JobResultType Fee_JobWriteBlockDone(void);

static MemIf_JobResultType Fee_JobInvalidateBlock(void);

static MemIf_JobResultType Fee_JobInvalidateBlockDone(void);

static MemIf_JobResultType Fee_JobEraseImmediateBlock(void);

static MemIf_JobResultType Fee_JobSchedule(void);

static MemIf_JobResultType Fee_JobScheduleAfterSwapClusterVldDone(void);

static bool Fee_ReservedAreaWritable(void);

static void Fee_JobErrorSchedule(void);


static inline bool Fee_ReservedAreaTargetedInClrGrp(const uint8_t ClrGrpIndex);

static inline uint32_t Fee_PowerOf2Of5LSB(const uint32_t InVal);

static inline bool Fee_ReservedAreaTouchedByBlock(const uint16_t BlockNumber);

static inline void Fee_TouchReservedAreaByBlock(const uint16_t BlockNumber);

static inline void Fee_UntouchReservedAreaByClrGrp(const uint8_t ClrGrpIndex);

static inline bool Fee_IsBlockMatchedConfig(uint16_t BlockIndex,
                                               uint16_t BlockRuntimeInfoIndex,
                                               const Fee_BlockType *BlockHder,
                                               Fls_AddressType DataAddr
                                              );

static inline void Fee_UpdateBlockRuntimeInfo(uint16_t BlockRuntimeInfoIndex,
                                              Fee_BlockStatusType BlockStatus,
                                              Fls_AddressType DataAddr,
                                              bool ImmediateBlock
                                             );

static Std_ReturnType Fee_ReadFromFls(Fls_AddressType SourceAddress,
                                      uint8_t *TargetAddressPtr,
                                      Fls_LengthType Length
                                     );

static Std_ReturnType Fee_WriteToFls(Fls_AddressType TargetAddress,
                                     const uint8_t *SourceAddressPtr,
                                     Fls_LengthType Length
                                    );

static Std_ReturnType Fee_EraseCluster(uint8_t ClrGrpIt,
                                       uint8_t ClrIt
                                      );

static inline Fls_LengthType Fee_GetClusterLength(uint8_t ClrGrpIt,
                                                  uint8_t ClrIt
                                                 );

static inline uint8_t Fee_GetNextClusterToSwap(uint8_t CurrentCluster);

/*==================================================================================================
*                                       FEE JOB SCHEDULE LOOKUP TABLE AREA
==================================================================================================*/
/* Function prototypes */
static inline MemIf_JobResultType Fee_JobIntalScanCluHdrParse_Wrapper(void);
static inline MemIf_JobResultType Fee_JobInternalScanBlkHdrParse_Wrapper(void);
static inline MemIf_JobResultType Fee_JobInternalSwapDataRead_Wrapper(void);
static inline MemIf_JobResultType Fee_JobInternalSwapDataWrite_Wrapper(void);
static inline MemIf_JobResultType Fee_JobSystemDone(void);

/*!
 * @brief Enter the short critical section used by public API state staging.
 */
static uint32_t Fee_EnterCriticalSection(void)
{
    uint32_t primaskState = __get_PRIMASK();

    __disable_irq();

    return primaskState;
}

/*!
 * @brief Restore the interrupt mask captured on critical-section entry.
 */
static void Fee_ExitCriticalSection(uint32_t primaskState)
{
    __set_PRIMASK(primaskState);
}

/*!
 * @brief Serialize a 16-bit unsigned integer into the byte buffer and advance the pointer.
 */
static inline void Fee_SerializeUint16(uint16_t ParamVal, uint8_t **SerialPtr)
{
    *((uint16_t*)((uint32_t)(*SerialPtr))) = ParamVal;
    (*SerialPtr) = &((*SerialPtr)[sizeof(uint16_t)]);
}

/*!
 * @brief Serialize a 32-bit unsigned integer into the byte buffer and advance the pointer.
 */
static inline void Fee_SerializeUint32(uint32_t ParamVal, uint8_t **SerialPtr)
{
    *((uint32_t*)((uint32_t)(*SerialPtr))) = ParamVal;
    (*SerialPtr) = &((*SerialPtr)[sizeof(uint32_t)]);
}

/*!
 * @brief Serialize a flash address value (Fls_AddressType width) into the byte buffer and advance the pointer.
 */
static inline void Fee_SerializeAddress(Fls_AddressType ParamVal, uint8_t **SerialPtr)
{
    *((Fls_AddressType*)((uint32_t)(*SerialPtr))) = ParamVal;
    (*SerialPtr) = &((*SerialPtr)[sizeof(Fls_AddressType)]);
}

/*!
 * @brief Serialize scalar parameter into the buffer.
 */
static inline void Fee_SerializeLength(Fls_LengthType ParamVal, uint8_t **SerialPtr)
{
    *((Fls_LengthType*)((uint32_t)(*SerialPtr))) = ParamVal;
    (*SerialPtr) = &((*SerialPtr)[sizeof(Fls_LengthType)]);
}

/*!
 * @brief Deserialize scalar parameter from the buffer.
 */
static inline void Fee_DeserializeUint8(const uint8_t **DeserialPtr, uint8_t *ParamVal)
{
    *ParamVal = *(const uint8_t*)((uint32_t)(*DeserialPtr));
    (*DeserialPtr) = &((*DeserialPtr)[sizeof(uint8_t)]);
}

/*!
 * @brief Deserialize scalar parameter from the buffer.
 */
static inline void Fee_DeserializeUint16(const uint8_t **DeserialPtr, uint16_t *ParamVal)
{
    *ParamVal = *(const uint16_t*)((uint32_t)(*DeserialPtr));
    (*DeserialPtr) = &((*DeserialPtr)[sizeof(uint16_t)]);
}

/*!
 * @brief Deserialize scalar parameter from the buffer.
 */
static inline void Fee_DeserializeUint32(const uint8_t **DeserialPtr, uint32_t *ParamVal)
{
    *ParamVal = *(const uint32_t*)((uint32_t)(*DeserialPtr));
    (*DeserialPtr) = &((*DeserialPtr)[sizeof(uint32_t)]);
}

/*!
 * @brief Deserialize scalar parameter from the buffer.
 */
static inline void Fee_DeserializeAddress(const uint8_t **DeserialPtr, Fls_AddressType *ParamVal)
{
    *ParamVal = *(const Fls_AddressType*)((uint32_t)(*DeserialPtr));
    (*DeserialPtr) = &((*DeserialPtr)[sizeof(Fls_AddressType)]);
}

/*!
 * @brief Deserialize scalar parameter from the buffer.
 */
static inline void Fee_DeserializeLength(const uint8_t **DeserialPtr, Fls_LengthType *ParamVal)
{
    *ParamVal = *(const Fls_LengthType*)((uint32_t)(*DeserialPtr));
    (*DeserialPtr) = &((*DeserialPtr)[sizeof(Fls_LengthType)]);
}

#if defined(FEE_LIGHT_MODE) && (FEE_LIGHT_MODE == 1U)
/*!
 * @brief Serialize 24-bit (3 bytes) address into the buffer (Light mode).
 */
static inline void Fee_SerializeAddress24(Fls_AddressType ParamVal, uint8_t **SerialPtr)
{
    (*SerialPtr)[0] = (uint8_t)(ParamVal & 0xFFU);
    (*SerialPtr)[1] = (uint8_t)((ParamVal >> 8U) & 0xFFU);
    (*SerialPtr)[2] = (uint8_t)((ParamVal >> 16U) & 0xFFU);
    (*SerialPtr) = &((*SerialPtr)[3U]);
}

/*!
 * @brief Deserialize 24-bit (3 bytes) address from the buffer (Light mode).
 */
static inline void Fee_DeserializeAddress24(const uint8_t **DeserialPtr, Fls_AddressType *ParamVal)
{
    *ParamVal = (Fls_AddressType)((*DeserialPtr)[0]) |
                ((Fls_AddressType)((*DeserialPtr)[1]) << 8U) |
                ((Fls_AddressType)((*DeserialPtr)[2]) << 16U);
    (*DeserialPtr) = &((*DeserialPtr)[3U]);
}

/*!
 * @brief Serialize 8-bit checksum into the buffer (Light mode).
 */
static inline void Fee_SerializeUint8(uint8_t ParamVal, uint8_t **SerialPtr)
{
    **SerialPtr = ParamVal;
    (*SerialPtr) = &((*SerialPtr)[sizeof(uint8_t)]);
}
#endif /* FEE_LIGHT_MODE */

/*!
 * @brief Invoke Fee_JobIntalScanCluHdrParse() with a valid buffer state.
 */
static inline MemIf_JobResultType Fee_JobIntalScanCluHdrParse_Wrapper(void)
{
    return Fee_JobIntalScanCluHdrParse(true);
}

/*!
 * @brief Invoke Fee_JobInternalScanBlkHdrParse() with a valid buffer state.
 */
static inline MemIf_JobResultType Fee_JobInternalScanBlkHdrParse_Wrapper(void)
{
    return Fee_JobInternalScanBlkHdrParse(true);
}

/*!
 * @brief Invoke Fee_JobInternalSwapDataRead() with a valid buffer state.
 */
static inline MemIf_JobResultType Fee_JobInternalSwapDataRead_Wrapper(void)
{
    return Fee_JobInternalSwapDataRead(true);
}

/*!
 * @brief Invoke Fee_JobInternalSwapDataWrite() with a valid buffer state.
 */
static inline MemIf_JobResultType Fee_JobInternalSwapDataWrite_Wrapper(void)
{
    return Fee_JobInternalSwapDataWrite(true);
}

/*!
 * @brief Return the terminal result for the synthetic scheduler done state.
 */
static inline MemIf_JobResultType Fee_JobSystemDone(void)
{
    return MEMIF_JOB_FAILED;
}

/*!
 * @brief Dispatch table used by Fee_JobSchedule().
 *
 * The order of entries must remain aligned with the Fee_JobType enumeration.
 */
static MemIf_JobResultType (*Fee_JobScheduleLookupTable[])(void) =
{
    /* Jobs entered from Fee_Read(). */
    Fee_JobReadBlock,                                /* FEE_JOB_READ */

    /* Jobs entered from Fee_Write(). */
    Fee_JobWriteBlock,                               /* FEE_JOB_WRITE */
    Fee_JobWriteBlockData,                           /* FEE_JOB_WRITE_DATA */
    Fee_JobWriteBlockUnalignedData,                  /* FEE_JOB_WRITE_UNALIGNED_DATA */
    Fee_JobWriteBlockValidate,                       /* FEE_JOB_WRITE_VALIDATE */
    Fee_JobWriteBlockDone,                           /* FEE_JOB_WRITE_DONE */

    /* Jobs entered from Fee_InvalidateBlock(). */
    Fee_JobInvalidateBlock,                          /* FEE_JOB_INVAL_BLOCK */
    Fee_JobInvalidateBlockDone,                      /* FEE_JOB_INVAL_BLOCK_DONE */

    /* Jobs entered from Fee_EraseImmediateBlock(). */
    Fee_JobEraseImmediateBlock,                      /* FEE_JOB_ERASE_IMMEDIATE */

    /* Jobs entered from Fee_Init(). */
    Fee_JobInternalScan,                             /* FEE_JOB_INT_SCAN */
    Fee_JobInternalScanCluster,                      /* FEE_JOB_INT_SCAN_CLR */
    Fee_JobIntalScanCluHdrParse_Wrapper,             /* FEE_JOB_INT_SCAN_CLR_HDR_PARSE */
    Fee_JobInternalScanClusterFmt,                   /* FEE_JOB_INT_SCAN_CLR_FMT */
    Fee_JobInternalScanClusterFmtDone,               /* FEE_JOB_INT_SCAN_CLR_FMT_DONE */
    Fee_JobInternalScanBlkHdrParse_Wrapper,          /* FEE_JOB_INT_SCAN_BLOCK_HDR_PARSE */

    /* Internal cluster-swap jobs. */
    Fee_JobInternalSwapClusterFmt,                   /* FEE_JOB_INT_SWAP_CLR_FMT */
    Fee_JobInternalSwapBlock,                        /* FEE_JOB_INT_SWAP_BLOCK */
    Fee_JobInternalSwapDataRead_Wrapper,             /* FEE_JOB_INT_SWAP_DATA_READ */
    Fee_JobInternalSwapDataWrite_Wrapper,            /* FEE_JOB_INT_SWAP_DATA_WRITE */
    Fee_JobInternalSwapClusterVldDone,               /* FEE_JOB_INT_SWAP_CLR_VLD_DONE */

    /* Terminal scheduler state. */
    Fee_JobSystemDone,                               /* FEE_JOB_DONE */
};
/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/
/*!
 * @brief Return the configured cluster-group index for a logical block.
 */
static inline uint8_t Fee_GetBlockClusterGrp(const uint16_t BlockRuntimeInfoIndex)
{
    uint8_t BlockClusterGrp;

    /* Read the cluster-group index from the generated block table. */
    BlockClusterGrp = Fee_ConfigPtr->blockConfigPtr[BlockRuntimeInfoIndex].ClrGrp;

    return BlockClusterGrp;
}

/*!
 * @brief Check whether the active cluster cursor has reached the reserved area.
 */
static inline bool Fee_ReservedAreaTargetedInClrGrp(const uint8_t ClrGrpIndex)
{
    bool RetVal;
    Fls_LengthType AvailClrSpace;
    uint32_t ReservedSpace;

    /* Read the configured reserved size for the cluster group. */
    ReservedSpace = Fee_ConfigPtr->clusterConfigPtr[ ClrGrpIndex ].ReservedSize;

    /* Compute the remaining writable span in the active cluster. */
    AvailClrSpace = Fee_aClrGrpInfo[ ClrGrpIndex ].DataAddrIt -
                    Fee_aClrGrpInfo[ ClrGrpIndex ].HdrAddrIt;

    if ((FEE_BLOCK_OVERHEAD + ReservedSpace) > AvailClrSpace)
    {
        RetVal = true;
    }
    else
    {
        RetVal = false;
    }

    return RetVal;
}

/*!
 * @brief Function to compute the power of 2 out of the 5 LSB bits of InVal value.
 */
static inline uint32_t Fee_PowerOf2Of5LSB(const uint32_t InVal)
{
    return (uint32_t)(0x00000001UL << (InVal & 0x1FUL));
}

/*!
 * @brief Report whether the logical block currently touches reserved space.
 */
static inline bool Fee_ReservedAreaTouchedByBlock(const uint16_t BlockNumber)
{
    uint32_t Idx;
    bool RetVal;

    /* Each array element stores one bit per logical block. */
    Idx = ((uint32_t)BlockNumber) >> 5U;
    /* Check whether the bit assigned to this block is set. */
    if (0U != (Fee_aReservedAreaTouched[ Idx ] & Fee_PowerOf2Of5LSB((uint32_t)BlockNumber)))
    {
        RetVal = true;
    }
    else
    {
        RetVal = false;
    }
    return RetVal;
}

/*!
 * @brief Stores the information about touching the Reserved Area for the block specified by BlockNumber.
 */
static inline void Fee_TouchReservedAreaByBlock(const uint16_t BlockNumber)
{
    uint32_t Idx;

    Idx = ((uint32_t)BlockNumber) >> 5U;

    Fee_aReservedAreaTouched[Idx] |= Fee_PowerOf2Of5LSB((uint32_t)BlockNumber);
}

/*!
 * @brief Removes the information about touching the Reserved Area for all blocks within a cluster group specified by ClrGrpIndex.
 */
static inline void Fee_UntouchReservedAreaByClrGrp(const uint8_t ClrGrpIndex)
{
    uint32_t BlockIt;
    uint32_t Idx = 0UL;
    uint32_t Mask = 0x00000001UL;
    uint8_t  BlockClusterGrp;


    for (BlockIt = 0U; BlockIt < FEE_CRT_CFG_NR_OF_BLOCKS; BlockIt++)
    {
        /*Get cluster group of block "BlockIt"*/
        BlockClusterGrp = Fee_GetBlockClusterGrp((uint16_t)BlockIt);

        /*Check if the block "BlockIt" is in ClrGrpIndex*/
        if (ClrGrpIndex == BlockClusterGrp)
        {
            /*Remove information bit of the block*/
            Fee_aReservedAreaTouched[Idx] &= (~Mask);
        }
        /*Check if the block is the last block in one element of Fee_aReservedAreaTouched*/
        if (0x80000000UL == Mask)
        {
            /*Reset Mask to the 1st bit*/
            Mask = 0x00000001UL;
            /*Move to the next element of Fee_aReservedAreaTouched*/
            Idx++;
        }
        else
        {
            /*Move to next bit for next block "BlockIt+1"*/
            Mask = Mask << 1U;
        }
    }
}

/*!
 * @brief Serialize Fee block parameters into a write buffer.
 */
static void Fee_SerializeBlockHdr(const Fee_BlockType *BlockHder,
                                  const Fls_AddressType TargetAddress,
                                  uint8_t *BlockHdrPtr
                                 )
{
#if defined(FEE_LIGHT_MODE) && (FEE_LIGHT_MODE == 1U)
    /* Light mode: 8-bit checksum, 3-byte address, no Invalid Flag */
    /* Layout: BlockNum(2) + Length(2) + TargetAddr(3) + CheckSum(1) + ValidFlag(8) = 16 bytes */
    uint8_t CheckSum = 0U;
    const uint8_t *TargetEndPtr;
    uint8_t *TempBlockHdrPtr = BlockHdrPtr;

    TargetEndPtr = &BlockHdrPtr[FEE_BLOCK_OVERHEAD];

    /* Calculate the 8-bit block header Checksum (7-bit value + 1-bit immediate flag) */
    /* Use only the bits that will be serialized: 16-bit BlockNumber, 16-bit Length, 24-bit TargetAddress */
    CheckSum = (uint8_t)(BlockHder->BlockNumber + BlockHder->Length + (TargetAddress & 0xFFFFFFU));

    /* Use MSB of the Checksum for the immediate block flag so the Checksum is 7-bit */
    if (true == (BlockHder->ImmediateBlock))
    {
        CheckSum += 1U;
        CheckSum |= 0x80U;
    }
    else
    {
        CheckSum &= 0x7fU;
    }

    /* Serialize Fee block parameters into a write buffer */
    Fee_SerializeUint16(BlockHder->BlockNumber, &TempBlockHdrPtr);
    Fee_SerializeUint16(BlockHder->Length, &TempBlockHdrPtr);
    Fee_SerializeAddress24(TargetAddress, &TempBlockHdrPtr);
    Fee_SerializeUint8(CheckSum, &TempBlockHdrPtr);

    /* Fill rest of the header with the erase pattern */
    for ( ; TempBlockHdrPtr < TargetEndPtr; TempBlockHdrPtr++)
    {
        *TempBlockHdrPtr = FEE_ERASED_VALUE;
    }
#else
    /* Standard mode: Original 32-bit checksum and header layout */
    uint32_t CheckSum = 0UL;
    const uint8_t *TargetEndPtr;
    uint8_t *TempBlockHdrPtr = BlockHdrPtr;

    TargetEndPtr = &BlockHdrPtr[FEE_BLOCK_OVERHEAD];

    /* Calculate the block header Checksum */
    CheckSum = ((uint32_t) BlockHder->BlockNumber) + ((uint32_t) BlockHder->Length) + TargetAddress;

    /* Use MSB of the Checksum for the immediate block flag so the Checksum is 31-bit */
    if (true == (BlockHder->ImmediateBlock))
    {
        CheckSum += 1U;
        CheckSum |= 0x80000000U;
    }
    else
    {
        CheckSum &= 0x7fffffffU;
    }
    /*Serialize Fee block parameters into a write buffer*/
    Fee_SerializeUint16(BlockHder->BlockNumber, &TempBlockHdrPtr);

    Fee_SerializeUint16(BlockHder->Length, &TempBlockHdrPtr);

    Fee_SerializeAddress(TargetAddress, &TempBlockHdrPtr);

    Fee_SerializeUint32(CheckSum, &TempBlockHdrPtr);

    /* Fill rest of the header with the erase pattern */
    for ( ; TempBlockHdrPtr < TargetEndPtr; TempBlockHdrPtr++)
    {
        *TempBlockHdrPtr = FEE_ERASED_VALUE;
    }
#endif
}

/*!
 * @brief Check whether specified data buffer contains only the FEE_ERASED_VALUE value.
 */
static Std_ReturnType Fee_BlankCheck(const uint8_t *TargetPtr, const uint8_t *const TargetEndPtr)
{
    Std_ReturnType RetVal = (Std_ReturnType)E_OK;
    const uint8_t *TempTargetPtr = TargetPtr;

    for ( ; TempTargetPtr < TargetEndPtr; TempTargetPtr++)
    {
        if (FEE_ERASED_VALUE == *TempTargetPtr)
        {
            /* Pattern match */
        }
        else
        {
            RetVal = (Std_ReturnType)E_NOT_OK;
            break;
        }
    }

    return RetVal;
}

/*!
 * @brief Deserialize the valid or invalid flag from a read buffer.
 */
static Std_ReturnType Fee_DeserializeFlag(const uint8_t *const TargetPtr, const uint8_t FlagPattern, bool *pFlagValue)
{
    Std_ReturnType RetVal;

    if ((FlagPattern == *TargetPtr) || (FEE_ERASED_VALUE == *TargetPtr))
    {
        /* The bytePattern can be either present or not */
        if (FlagPattern == *TargetPtr)
        {
            *pFlagValue = true;
        }
        else
        {
            *pFlagValue = false;
        }

        /* Check rest of the FEE_VIRTUAL_PAGE_SIZE */
        RetVal = Fee_BlankCheck(&TargetPtr[1], &TargetPtr[FEE_VIRTUAL_PAGE_SIZE]);
    }
    else
    {
        RetVal = (Std_ReturnType)E_NOT_OK;
    }

    return RetVal;
}

/*!
 * @brief Deserialize Fee block header parameters from read buffer.
 */
static Fee_BlockStatusType Fee_DeserializeBlockHdr(Fee_BlockType *const BlockHder,
                                                   Fls_AddressType *const TargetAddress,
                                                   const uint8_t *BlockHdrPtr
                                                  )
{
#if defined(FEE_LIGHT_MODE) && (FEE_LIGHT_MODE == 1U)
    /* Light mode: 8-bit checksum, 3-byte address, no Invalid Flag support */
    /* Layout: BlockNum(2) + Length(2) + TargetAddr(3) + CheckSum(1) + ValidFlag(8) = 16 bytes */
    Fee_BlockStatusType RetVal = FEE_BLOCK_INVALID;
    uint8_t ReadCheckSum;
    uint8_t CalcCheckSum;
    const uint8_t *TargetEndPtr;
    const uint8_t *TempBlockHdrPtr = BlockHdrPtr;
    bool FlagValid = false;
    Std_ReturnType BlankCheckResult;
    Std_ReturnType DeserializeValidResult;

    if (Fee_BlankCheck(BlockHdrPtr, &BlockHdrPtr[FEE_BLOCK_OVERHEAD]) == ((Std_ReturnType)E_OK))
    {
        RetVal = FEE_BLOCK_HEADER_BLANK;
    }
    else
    {
        /* Check 1st part of the header */
        TargetEndPtr = &BlockHdrPtr[FEE_BLOCK_HEADER_STATUS_OFFSET];

        Fee_DeserializeUint16((const uint8_t **)&TempBlockHdrPtr, &BlockHder->BlockNumber);
        Fee_DeserializeUint16((const uint8_t **)&TempBlockHdrPtr, &BlockHder->Length);
        Fee_DeserializeAddress24((const uint8_t **)&TempBlockHdrPtr, TargetAddress);
        Fee_DeserializeUint8((const uint8_t **)&TempBlockHdrPtr, &ReadCheckSum);

        /* Use MSB of Checksum for immediate block flag so the Checksum is 7-bit long */
        /* Use same formula as serialization with 24-bit address mask */
        CalcCheckSum = (uint8_t)(BlockHder->BlockNumber + BlockHder->Length + (*TargetAddress & 0xFFFFFFU));

        if (0U == (ReadCheckSum & 0x80U))
        {
            BlockHder->ImmediateBlock = false;
        }
        else
        {
            BlockHder->ImmediateBlock = true;
            CalcCheckSum += 1U;
        }

        BlankCheckResult = Fee_BlankCheck(TempBlockHdrPtr, TargetEndPtr);
        DeserializeValidResult = Fee_DeserializeFlag(TargetEndPtr, FEE_VALIDATED_VALUE, &FlagValid);

        if (((ReadCheckSum & 0x7fU) != (CalcCheckSum & 0x7fU)) ||
            (E_OK != BlankCheckResult) ||
            (E_OK != DeserializeValidResult)
           )
        {
            RetVal = FEE_BLOCK_HEADER_INVALID;
        }
        else
        {
            /* Block header is valid */
            if (true == FlagValid)
            {
                RetVal = FEE_BLOCK_VALID;
            }
            else
            {
                RetVal = FEE_BLOCK_INCONSISTENT;
            }
        }
    }

    return RetVal;
#else
    Fee_BlockStatusType RetVal = FEE_BLOCK_INVALID;
    uint32_t ReadCheckSum;
    uint32_t CalcCheckSum;
    const uint8_t *TargetEndPtr;
    const uint8_t *TempBlockHdrPtr = BlockHdrPtr;
    bool FlagValid = false;
    bool FlagInvalid = false;
    uint8_t ReservedSpace;
    Std_ReturnType BlankCheckResult;
    Std_ReturnType DeserializeValidResult;
    Std_ReturnType DeserializeInvalidResult;

    if (Fee_BlankCheck(BlockHdrPtr, &BlockHdrPtr[FEE_BLOCK_OVERHEAD]) == ((Std_ReturnType)E_OK))
    {
        RetVal = FEE_BLOCK_HEADER_BLANK;
    }
    else
    {
        /* Check 1st part of the header */
        TargetEndPtr = &BlockHdrPtr[FEE_BLOCK_HEADER_STATUS_OFFSET];

        Fee_DeserializeUint16((const uint8_t **)&TempBlockHdrPtr, &BlockHder->BlockNumber);

        Fee_DeserializeUint16((const uint8_t **)&TempBlockHdrPtr, &BlockHder->Length);

        Fee_DeserializeAddress((const uint8_t **)&TempBlockHdrPtr, TargetAddress);

        Fee_DeserializeUint32((const uint8_t **)&TempBlockHdrPtr, &ReadCheckSum);

        /* BlockHdrPtr must be advanced with 1 bytes.
         * the reason is to avoid blank check for this area reserved for BlockAssignment even when
         * SWAP_FOREIGN is disabled to allow disabling SWAP_FOREIGN at production time. */
        Fee_DeserializeUint8((const uint8_t **)&TempBlockHdrPtr, &ReservedSpace);

        /* variable ReservedSpace not used */
        (void)ReservedSpace;

        /* Use MSB of Checksum for immediate block flag so
           the Checksum is 31-bit long */
        CalcCheckSum = BlockHder->Length + *TargetAddress + BlockHder->BlockNumber;

        if (0U == (ReadCheckSum & 0x80000000U))
        {
            BlockHder->ImmediateBlock = false;
        }
        else
        {
            BlockHder->ImmediateBlock = true;
            CalcCheckSum += 1U;
        }

        /* Ignore MSB since it's used for ImmediateBlock flag */
        BlankCheckResult = Fee_BlankCheck(TempBlockHdrPtr, TargetEndPtr);
        DeserializeValidResult = Fee_DeserializeFlag(TargetEndPtr, FEE_VALIDATED_VALUE, &FlagValid);
        DeserializeInvalidResult = Fee_DeserializeFlag(&TargetEndPtr[FEE_VIRTUAL_PAGE_SIZE], FEE_INVALIDATED_VALUE, &FlagInvalid);
        
        if (((ReadCheckSum & 0x7fffffffU) != (CalcCheckSum & 0x7fffffffU)) ||
            (E_OK != BlankCheckResult) ||
            (E_OK != DeserializeValidResult) ||
            (E_OK != DeserializeInvalidResult)
           )
        {
            RetVal = FEE_BLOCK_HEADER_INVALID;
        }
        else
        {
            /* Block header is valid */
            if (true == FlagValid)
            {
                if (false == FlagInvalid)
                {
                    RetVal = FEE_BLOCK_VALID;           /* true vs false */
                }
                else
                {
                    RetVal = FEE_BLOCK_INVALIDATED;     /* TRUE vs TRUE */
                }
            }
            else
            {
                if (false == FlagInvalid)
                {
                    RetVal = FEE_BLOCK_INCONSISTENT;    /* false vs false */
                }
            }
        }
    }

    return RetVal;
#endif
}

/*!
 * @brief Deserialize Fee cluster header parameters from read buffer.
 */
static Fee_ClusterStatusType Fee_DeserializeClusterHdr(Fee_ClusterHeaderType *ClrHdr,
                                                       const uint8_t *ClrHdrPtr
                                                      )
{
#if defined(FEE_LIGHT_MODE) && (FEE_LIGHT_MODE == 1U)
    /* Light mode: 8-bit checksum, 3-byte address, no Invalid Flag support */
    /* Layout: ClrID(2) + Length(2) + StartAddr(3) + CheckSum(1) + ValidFlag(8) = 16 bytes */
    Fee_ClusterStatusType RetVal = FEE_CLUSTER_HEADER_INVALID;
    uint8_t CheckSumHdr;
    uint8_t CheckSumCal;
    bool FlagValid = false;
    const uint8_t *TargetEndPtr;
    const uint8_t *TempClrHdrPtr = ClrHdrPtr;
    Std_ReturnType BlankCheckResult;
    Std_ReturnType DeserializeValidResult;

    /* Check 1st part of the header */
    TargetEndPtr = &ClrHdrPtr[FEE_CLUSTER_HEADER_STATUS_OFFSET];

    /* Initialize 32-bit fields to 0 before reading 16-bit values */
    ClrHdr->ClrID = 0U;
    ClrHdr->Length = 0U;
    ClrHdr->StartAddr = 0U;
    
    Fee_DeserializeUint16(&TempClrHdrPtr, (uint16_t *)&ClrHdr->ClrID);
    Fee_DeserializeUint16(&TempClrHdrPtr, (uint16_t *)&ClrHdr->Length);
    Fee_DeserializeAddress24(&TempClrHdrPtr, &ClrHdr->StartAddr);
    Fee_DeserializeUint8(&TempClrHdrPtr, &CheckSumHdr);

    /* Calculate checksum using same formula as serialization */
    CheckSumCal = (uint8_t)((uint16_t)ClrHdr->ClrID + (ClrHdr->StartAddr & 0xFFFFFFU) + (uint16_t)ClrHdr->Length);

    BlankCheckResult = Fee_BlankCheck(TempClrHdrPtr, TargetEndPtr);
    DeserializeValidResult = Fee_DeserializeFlag(TargetEndPtr, FEE_VALIDATED_VALUE, &FlagValid);

    if ((CheckSumCal != CheckSumHdr) ||
        (E_OK != BlankCheckResult) ||
        (E_OK != DeserializeValidResult)
       )
    {
        /* FEE_CLUSTER_HEADER_INVALID */
    }
    else
    {
        if (false == FlagValid)
        {
            RetVal = FEE_CLUSTER_INCONSISTENT;
        }
        else
        {
            RetVal = FEE_CLUSTER_VALID;
        }
    }
    return(RetVal);
#else
    Fee_ClusterStatusType RetVal = FEE_CLUSTER_HEADER_INVALID;
    uint32_t CheckSumHdr;
    uint32_t CheckSumCal;
    bool FlagValid = false;
    bool FlagInvalid = false;
    const uint8_t *TargetEndPtr;
    const uint8_t *TempClrHdrPtr = ClrHdrPtr;
    Std_ReturnType BlankCheckResult;
    Std_ReturnType DeserializeValidResult;
    Std_ReturnType DeserializeInvalidResult;

    /* Check 1st part of the header */
    TargetEndPtr = &ClrHdrPtr[FEE_CLUSTER_HEADER_STATUS_OFFSET];

    Fee_DeserializeUint32(&TempClrHdrPtr, &ClrHdr->ClrID);

    Fee_DeserializeAddress(&TempClrHdrPtr, &ClrHdr->StartAddr);

    Fee_DeserializeLength(&TempClrHdrPtr, &ClrHdr->Length);

    CheckSumCal = ClrHdr->ClrID + ClrHdr->StartAddr + ClrHdr->Length;


    Fee_DeserializeUint32(&TempClrHdrPtr, &CheckSumHdr);

    BlankCheckResult = Fee_BlankCheck(TempClrHdrPtr, TargetEndPtr);
    DeserializeValidResult = Fee_DeserializeFlag(TargetEndPtr, FEE_VALIDATED_VALUE, &FlagValid);
    DeserializeInvalidResult = Fee_DeserializeFlag(&TargetEndPtr[FEE_VIRTUAL_PAGE_SIZE], FEE_INVALIDATED_VALUE, &FlagInvalid);
    
    if ((CheckSumCal != CheckSumHdr) ||
        (E_OK != BlankCheckResult) ||
        (E_OK != DeserializeValidResult) ||
        (E_OK != DeserializeInvalidResult)
       )
    {
        /* FEE_CLUSTER_HEADER_INVALID */
    }
    else if (false == FlagInvalid)
    {
        if (false == FlagValid)
        {
            RetVal = FEE_CLUSTER_INCONSISTENT;
        }
        else
        {
            RetVal = FEE_CLUSTER_VALID;
        }
    }
    else
    {
        RetVal = FEE_CLUSTER_INVALID;
    }
    return(RetVal);
#endif
}

/*!
 * @brief Serialize Fee cluster header parameters to write buffer.
 */
static void Fee_SerializeClusterHdr(const Fee_ClusterHeaderType *ClrHdr,
                                    uint8_t *ClrHdrPtr
                                   )
{
#if defined(FEE_LIGHT_MODE) && (FEE_LIGHT_MODE == 1U)
    /* Light mode: 8-bit checksum, 3-byte address */
    /* Layout: ClrID(2) + Length(2) + StartAddr(3) + CheckSum(1) + ValidFlag(8) = 16 bytes */
    uint8_t CheckSum;
    const uint8_t *TargetEndPtr;
    uint8_t *TempClrHdrPtr = ClrHdrPtr;

    TargetEndPtr = &ClrHdrPtr[FEE_CLUSTER_OVERHEAD];

    /* Calculate the cluster header Checksum (8-bit for Light mode) */
    /* Use only the bits that will be serialized: 16-bit ClrID, 16-bit Length, 24-bit StartAddr */
    CheckSum = (uint8_t)((uint16_t)ClrHdr->ClrID + (ClrHdr->StartAddr & 0xFFFFFFU) + (uint16_t)ClrHdr->Length);

    /* Fee cluster header parameters to write buffer */
    Fee_SerializeUint16((uint16_t)ClrHdr->ClrID, &TempClrHdrPtr);
    Fee_SerializeUint16((uint16_t)ClrHdr->Length, &TempClrHdrPtr);
    Fee_SerializeAddress24(ClrHdr->StartAddr, &TempClrHdrPtr);
    Fee_SerializeUint8(CheckSum, &TempClrHdrPtr);

    /* Fill rest of the header with the erase pattern */
    for ( ; TempClrHdrPtr < TargetEndPtr; TempClrHdrPtr++)
    {
        *TempClrHdrPtr = FEE_ERASED_VALUE;
    }
#else
    uint32_t CheckSum;
    const uint8_t *TargetEndPtr;
    uint8_t *TempClrHdrPtr = ClrHdrPtr;

    TargetEndPtr = &ClrHdrPtr[FEE_CLUSTER_OVERHEAD];

    /* Calculate the cluster header Checksum */
    CheckSum = ClrHdr->ClrID + ClrHdr->StartAddr + ClrHdr->Length;

    /*Fee cluster header parameters to write buffer*/
    Fee_SerializeUint32(ClrHdr->ClrID, &TempClrHdrPtr);

    Fee_SerializeAddress(ClrHdr->StartAddr, &TempClrHdrPtr);

    Fee_SerializeLength(ClrHdr->Length, &TempClrHdrPtr);

    Fee_SerializeUint32(CheckSum, &TempClrHdrPtr);

    /* Fill rest of the header with the erase pattern */
    for ( ; TempClrHdrPtr < TargetEndPtr; TempClrHdrPtr++)
    {
        *TempClrHdrPtr = FEE_ERASED_VALUE;
    }
#endif
}

/*!
 * @brief Searches ordered list of Fee blocks and returns index of block with matching BlockNumber.
 */
static uint16_t Fee_GetBlockIndex(const uint16_t BlockNumber)
{
    int32_t Low = 0;
    int32_t High = (int32_t)FEE_CRT_CFG_NR_OF_BLOCKS - 1;
    int32_t Middle;
    uint16_t RetVal = 0xFFFFU;

    /*Using binary search to find the index of BlockNumber*/
    while (Low <= High)
    {
        Middle = Low + ((High - Low) / 2);

        if (BlockNumber < Fee_ConfigPtr->blockConfigPtr[Middle].BlockNumber)
        {
            /*Store High for next searching*/
            /*This judgment avoids the High overflow problem. When High=Low=0,
              if BlockNumber is still smaller than the value corresponding to Middle,
              High will become 0xFFFFFFFF, resulting in pointer overflow and an error.*/
            if (Middle > 0)
            {
                High = Middle - 1;
            }
            else
            {
                /*Change the value of Low so that it is greater than High, thus ending the loop.
                  Use this method instead of break to avoid failure of SONAR check.*/
                Low = High + 1;
            }
        }
        else if (BlockNumber > Fee_ConfigPtr->blockConfigPtr[Middle].BlockNumber)
        {
            /*Store Low for next searching*/
            Low = Middle + 1;
        }
        else
        {
            /*BlockNumber is matched, store the index and return*/
            RetVal = (uint16_t)Middle;
            break;
        }
    }
    /*RetVal will be 0xFFFFU if BlockNumber is not matched with any index*/
    return RetVal;
}

/*!
 * @brief Returns the block size for a block specified by its index in the Fee_aBlockInfo array.
 */
static inline uint16_t Fee_GetBlockSize(const uint16_t BlockRuntimeInfoIndex)
{
    uint16_t BlockSize;

    /* the config is part of Fee_BlockConfig*/
    BlockSize = Fee_ConfigPtr->blockConfigPtr[BlockRuntimeInfoIndex].BlockSize;
    return BlockSize;
}

/*!
 * @brief Returns the block number for a block specified by its index in the Fee_aBlockInfo array.
 */
static inline uint16_t Fee_GetBlockNumber(const uint16_t BlockRuntimeInfoIndex)
{
    uint16_t BlockNumber = 0U;
    /* the config is part of Fee_BlockConfig*/
    BlockNumber = Fee_ConfigPtr->blockConfigPtr[BlockRuntimeInfoIndex].BlockNumber;
    return BlockNumber;
}

/*!
 * @brief Returns the immediate attribute for a block specified by its index in the Fee_aBlockInfo array.
 */
static inline bool Fee_GetBlockImmediate(const uint16_t BlockRuntimeInfoIndex)
{
    bool Immediate;
    /* the config is part of Fee_BlockConfig*/
    Immediate = Fee_ConfigPtr->blockConfigPtr[BlockRuntimeInfoIndex].ImmediateData;
    return Immediate;
}

/*!
 * @brief Adjusts passed size so it's integer multiple of pre-configured + FEE_VIRTUAL_PAGE_SIZE.
 */
static uint16_t Fee_AlignToVirtualPageSize(uint16_t BlockSize)
{
    uint16_t Retval = 0U;
    /* Check whether the block size is integer multiple of FEE_VIRTUAL_PAGE_SIZE */
    if (0U == (BlockSize % FEE_VIRTUAL_PAGE_SIZE))
    {
        /* block size is an integer multiple of FEE_VIRTUAL_PAGE_SIZE */
        Retval = BlockSize;
    }
    else
    {
        Retval = ((BlockSize / FEE_VIRTUAL_PAGE_SIZE) + 1U) * FEE_VIRTUAL_PAGE_SIZE;
    }

    return(Retval);
}

/*!
 * @brief Copy data from user to internal write buffer and fills rest of the write buffer with FEE_ERASED_VALUE.
 */
static void Fee_CopyDataToPageBuffer(const uint8_t *SourcePtr, uint8_t *TargetPtr, const uint16_t Length)
{
    const uint8_t *TargetEndPtr = &TargetPtr[Length];
    const uint8_t *PageEndPtr = &TargetPtr[FEE_VIRTUAL_PAGE_SIZE];
    uint8_t *TempTargetPtr = TargetPtr;
    const uint8_t *TempSourcePtr = SourcePtr;

    /* Copy data to page buffer */
    for ( ; TempTargetPtr < TargetEndPtr; TempTargetPtr++)
    {
        *TempTargetPtr = *TempSourcePtr;
        TempSourcePtr++;
    }

    /* Fill rest of the page buffer with FEE_ERASED_VALUE */
    for ( ; TempTargetPtr < PageEndPtr; TempTargetPtr++)
    {
        *TempTargetPtr = FEE_ERASED_VALUE;
    }
}

/*!
 * @brief Serialize validation or invalidation flag to write buffer.
 */
static void Fee_SerializeFlag(uint8_t *TargetPtr, const uint8_t FlagPattern)
{
    const uint8_t *TargetEndPtr = &TargetPtr[FEE_VIRTUAL_PAGE_SIZE];
    uint8_t *TempPtr = TargetPtr;

    *TempPtr = FlagPattern;
    TempPtr++;


    for (; TempPtr < TargetEndPtr; TempPtr++)
    {
        /*Store erase value to target address*/
        *TempPtr = FEE_ERASED_VALUE;
    }
}

/*!
 * @brief Validate current Fee cluster in current Fee cluster group by writing FEE_VALIDATED_VALUE into flash.
 */
static MemIf_JobResultType Fee_JobInternalSwapClusterVld(void)
{
    MemIf_JobResultType RetVal;
    Fls_AddressType StartAddr;

    /* Get start address of swap cluster */
    StartAddr = Fee_ConfigPtr->clusterConfigPtr[Fee_uJobIntClrGrpIt].ClrPtr[Fee_uJobIntClrIt].StartAddr;

    /* Serialize validation pattern to write buffer */
    Fee_SerializeFlag(Fee_aDataBuffer, FEE_VALIDATED_VALUE);

    /* Write validation pattern to flash */
    if (((Std_ReturnType)E_OK) == Fee_WriteToFls(StartAddr + FEE_CLUSTER_HEADER_STATUS_OFFSET, Fee_aDataBuffer, FEE_VIRTUAL_PAGE_SIZE))
    {
        /* Fls write job has been accepted */
        RetVal = MEMIF_JOB_PENDING;
    }
    else
    {
        /* Fls write job hasn't been accepted */
        RetVal = MEMIF_JOB_FAILED;
    }

    /* Schedule subsequent Fee job */
    Fee_eJob = FEE_JOB_INT_SWAP_CLR_VLD_DONE;

    return RetVal;
}

/*!
 * @brief Copy next block from source to target cluster.
 */
static MemIf_JobResultType Fee_JobInternalSwapBlock(void)
{
    MemIf_JobResultType RetVal;
    Fls_AddressType DataAddr;
    Fee_BlockType BlockHder;
    uint16_t AlignedBlockSize;
    uint8_t BlockClusterGrp;


    /* Find first valid or inconsistent block in same cluster group */
    /* PRQA S 2461 Rule 14.2: the Fee_JobIntBlockIt in for loop also used in function 'Fee_GetBlockClusterGrp',
     *                       so it couldn't adhere to M3CM Rule-14.2
     */
    for ( ; Fee_uJobIntBlockIt < FEE_CRT_CFG_NR_OF_BLOCKS; Fee_uJobIntBlockIt++) /*PRQA S 2461*/
    {
        /* get cluster group */
        BlockClusterGrp = Fee_GetBlockClusterGrp(Fee_uJobIntBlockIt);
        /* process only valid and inconsistent blocks from the current cluster */
        if ((Fee_uJobIntClrGrpIt == BlockClusterGrp) &&
            ((FEE_BLOCK_VALID == Fee_aBlockInfo[Fee_uJobIntBlockIt].BlockStatus) ||
             (FEE_BLOCK_INCONSISTENT == Fee_aBlockInfo[Fee_uJobIntBlockIt].BlockStatus) ||
             (FEE_BLOCK_INCONSISTENT_COPY == Fee_aBlockInfo[Fee_uJobIntBlockIt].BlockStatus)
            )
           )

        {
            break;
        }
        else
        {
            /* No match. Try next one... */
        }
    }

    if (FEE_CRT_CFG_NR_OF_BLOCKS == Fee_uJobIntBlockIt)
    {
        /* No more matching blocks. Validate the cluster */
        RetVal = Fee_JobInternalSwapClusterVld();
    }
    else
    {
        BlockHder.Length = Fee_GetBlockSize(Fee_uJobIntBlockIt);
        BlockHder.ImmediateBlock = Fee_GetBlockImmediate(Fee_uJobIntBlockIt);
        BlockHder.BlockNumber = Fee_GetBlockNumber(Fee_uJobIntBlockIt);
        /* Data space no more allocated even for inconsistent immediate (pre-erased) blocks */
        /*
         * MR12 Rule 18.1 VIOLATION: The value of Fee_JobIntBlockIt is affirm by other functions in MCAL driver,
         *                           so it couldn't adhere to M3CM Rule-18.1.
         */
        if (FEE_BLOCK_VALID == Fee_aBlockInfo[Fee_uJobIntBlockIt].BlockStatus) /*PRQA S 2843*/
        {
            /* Align Fee block size to the virtual page boundary */
            AlignedBlockSize = Fee_AlignToVirtualPageSize(BlockHder.Length);

            /* Calculate data address */
            DataAddr = Fee_uJobIntDataAddr - AlignedBlockSize;

            /* Update the block address info */
            Fee_uJobIntDataAddr = DataAddr;

            /* Initialize the data transfer iterator */
            Fee_uJobIntAddrIt = DataAddr;
        }
        else
        {
            /* Don't allocate data space for inconsistent block */
            DataAddr = 0U;
        }

        /* Serialize block header to the write buffer */
        Fee_SerializeBlockHdr(&BlockHder, DataAddr,Fee_aDataBuffer);

        if (FEE_BLOCK_VALID == Fee_aBlockInfo[Fee_uJobIntBlockIt].BlockStatus)
        {
            /* Read block data */
            Fee_eJob = FEE_JOB_INT_SWAP_DATA_READ;
        }
        else
        {
            /* Nothing to read. Move on to next block */
            Fee_uJobIntBlockIt++;
            Fee_eJob = FEE_JOB_INT_SWAP_BLOCK;
        }

        /* Write header to flash */
        if (((Std_ReturnType)E_OK) != Fee_WriteToFls(Fee_uJobIntHdrAddr, Fee_aDataBuffer, FEE_BLOCK_HEADER_STATUS_OFFSET))
        {
            /* Fls write job hasn't been accepted */
            RetVal = MEMIF_JOB_FAILED;
        }
        else
        {
            /* Fls write job has been accepted */
            RetVal = MEMIF_JOB_PENDING;
        }

        /* Move on to next block header */
        Fee_uJobIntHdrAddr += FEE_BLOCK_OVERHEAD;
    }

    return RetVal;
}

/*!
 * @brief Format current Fee cluster in current Fee cluster group by writing cluster header into flash.
 */
static MemIf_JobResultType Fee_JobInternalSwapClusterFmt(void)
{
    MemIf_JobResultType RetVal;
    Fee_ClusterHeaderType ClrHdr;
    Fls_LengthType ClusterLength;

    ClusterLength = Fee_GetClusterLength(Fee_uJobIntClrGrpIt, Fee_uJobIntClrIt);

    ClrHdr.StartAddr = Fee_ConfigPtr->clusterConfigPtr[Fee_uJobIntClrGrpIt].ClrPtr[Fee_uJobIntClrIt].StartAddr;
    ClrHdr.Length = ClusterLength;
    ClrHdr.ClrID = Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].ActClrID + 1U;

    /* Build cluster header with new ActClrID + 1 */
    Fee_SerializeClusterHdr(&ClrHdr, Fee_aDataBuffer);

    /* Write the cluster header to flash */
    if (((Std_ReturnType)E_OK) == Fee_WriteToFls(ClrHdr.StartAddr, Fee_aDataBuffer, FEE_CLUSTER_HEADER_STATUS_OFFSET))
    {
        /* Fls read job has been accepted */
        RetVal = MEMIF_JOB_PENDING;
    }
    else
    {
        /* Fls write job hasn't been accepted */
        RetVal = MEMIF_JOB_FAILED;
    }

    /* Calculate header address: Move on to the first block header */
    Fee_uJobIntHdrAddr = ClrHdr.StartAddr + FEE_CLUSTER_OVERHEAD;

    /* Calculate data address: Move on to the end of cluster */
    Fee_uJobIntDataAddr = ClrHdr.StartAddr + ClrHdr.Length;

    /* Reset the block iterator */
    Fee_uJobIntBlockIt = 0U;

    /* Schedule cluster format done job */
    Fee_eJob = FEE_JOB_INT_SWAP_BLOCK;
    return RetVal;
}

/*!
 * @brief Erase current Fee cluster in current Fee cluster group by erasing flash.
 */
static MemIf_JobResultType Fee_JobInternalSwapClusterErase(void)
{
    MemIf_JobResultType RetVal;

    /* Erase cluster */
    if (((Std_ReturnType)E_OK) == Fee_EraseCluster(Fee_uJobIntClrGrpIt, Fee_uJobIntClrIt))
    {
        /* Fls read job has been accepted */
        RetVal = MEMIF_JOB_PENDING;
    }
    else
    {
        /* Fls read job hasn't been accepted */
        RetVal = MEMIF_JOB_FAILED;
    }

    /* Schedule cluster format job */
    Fee_eJob = FEE_JOB_INT_SWAP_CLR_FMT;

    return RetVal;
}

/*!
 * @brief Calculate the index of the next cluster in current cluster group. In sector retirement mode, find the next good cluster with Length greater than zero.
 */
static inline uint8_t Fee_GetNextClusterToSwap(uint8_t CurrentCluster)
{
    uint8_t NextCluster = CurrentCluster;

    /* Calculate index of cluster to swap to */
    NextCluster++;
	
    if (NextCluster == Fee_ConfigPtr->clusterConfigPtr[Fee_uJobIntClrGrpIt].ClrCount)
    {
        /* Cluster roll over */
        NextCluster = 0U;
    }
	
    return NextCluster;
}

/*!
 * @brief Initialize the cluster swap internal operation on current cluster group.
 */
static MemIf_JobResultType Fee_JobInternalSwap(void)
{
    MemIf_JobResultType RetVal = MEMIF_JOB_FAILED;
    uint8_t CurrentCluster;
    uint8_t NextCluster;

    /* Get cluster index the block belongs to */
    CurrentCluster = Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].ActClr;
    /* Get the next cluster to erase */
    NextCluster = Fee_GetNextClusterToSwap(CurrentCluster);

    /* Save index of cluster to swap to */
    Fee_uJobIntClrIt = NextCluster;

    /* Store type of original job so Fee can re-schedule
           this job once the swap is complete */
    Fee_eJobIntOriginalJob = Fee_eJob;

    /* Erase the swap cluster */
    RetVal = Fee_JobInternalSwapClusterErase();

    return RetVal;
}

/*!
 * @brief Read the Fee block header into internal buffer.
 */
static MemIf_JobResultType Fee_JobInternalScanBlockHdrRead(void)
{
    MemIf_JobResultType RetVal = MEMIF_JOB_OK;

    /* Read the block header */
    if (((Std_ReturnType)E_OK) != Fee_ReadFromFls(Fee_uJobIntAddrIt, Fee_aDataBuffer, FEE_BLOCK_OVERHEAD))
    {
        /* Fls read job hasn't been accepted */
        RetVal = MEMIF_JOB_FAILED;
    }
    else
    {
        /* Fls read job has been accepted */
        RetVal = MEMIF_JOB_PENDING;
    }

    Fee_eJob = FEE_JOB_INT_SCAN_BLOCK_HDR_PARSE;

    return RetVal;
}

/*!
 * @brief Erase first Fee cluster in current cluster group.
 */
static MemIf_JobResultType Fee_JobInternalScanClusterErase(void)
{
    MemIf_JobResultType RetVal;

    /* Erase first cluster in the current cluster group */
    if (((Std_ReturnType)E_OK) == Fee_EraseCluster(Fee_uJobIntClrGrpIt, Fee_uJobIntClrIt))
    {
        /* Fls read job has been accepted */
        RetVal = MEMIF_JOB_PENDING;
    }
    else
    {
        /* Fls read job hasn't been accepted */
        RetVal = MEMIF_JOB_FAILED;
    }

    /* Schedule cluster format job */
    Fee_eJob = FEE_JOB_INT_SCAN_CLR_FMT;

    return RetVal;
}

/*!
 * @brief Scan active cluster of current cluster group or erase and format first cluster if an active cluster can't be found.
 */
static MemIf_JobResultType Fee_JobInternalScanCluster(void)
{
    MemIf_JobResultType RetVal = MEMIF_JOB_OK;
    uint8_t ClrIndex;
    Fls_AddressType ClrStartAddr;
    Fls_LengthType ClrLength;

    if (FEE_NUMBER_OF_CLUSTER_GROUPS == Fee_uJobIntClrGrpIt)
    {
        /* Nor more cluster groups to scan */
        Fee_eJob = FEE_JOB_DONE;
    }
    else
    {
        if (0U == Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].ActClrID)
        {
            /* No active cluster found in current group so erase and format the first one... */
            Fee_uJobIntClrIt = 0U;
            RetVal = Fee_JobInternalScanClusterErase();
        }
        else
        {
            /* Active cluster found */
            /* Get index of active cluster */
            ClrIndex = Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].ActClr;

            ClrStartAddr = Fee_ConfigPtr->clusterConfigPtr[Fee_uJobIntClrGrpIt].ClrPtr[ClrIndex].StartAddr;

            /* Calculate logical address of first block header in active cluster */
            Fee_uJobIntAddrIt = FEE_CLUSTER_OVERHEAD + ClrStartAddr;

            /* Initialize the block header pointer */
            Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].HdrAddrIt = Fee_uJobIntAddrIt;

            /* Initialize the block data pointer */
            ClrLength = Fee_GetClusterLength(Fee_uJobIntClrGrpIt, ClrIndex);
            Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].DataAddrIt = ClrStartAddr + ClrLength;

            /* Active cluster found so read the first block header */
            RetVal = Fee_JobInternalScanBlockHdrRead();
        }
    }

    return RetVal;
}

/*!
 * @brief Format first Fee cluster in current Fee cluster group by writing cluster header into flash.
 */
static MemIf_JobResultType Fee_JobInternalScanClusterFmt(void)
{
    MemIf_JobResultType RetVal;
    Fee_ClusterHeaderType ClrHdr;
    Fls_LengthType ClusterLength;

    ClusterLength = Fee_GetClusterLength(Fee_uJobIntClrGrpIt, Fee_uJobIntClrIt);
	
    /* Get address and size of first cluster in the current cluster group */
    ClrHdr.StartAddr = Fee_ConfigPtr->clusterConfigPtr[Fee_uJobIntClrGrpIt].ClrPtr[Fee_uJobIntClrIt].StartAddr;
    ClrHdr.Length = ClusterLength;
    ClrHdr.ClrID = 1U;

    /* Build cluster header with clusterID = 1 */
    Fee_SerializeClusterHdr(&ClrHdr, Fee_aDataBuffer);

    /* Make the cluster valid right away as it's empty anyway... */
    Fee_SerializeFlag(&Fee_aDataBuffer[FEE_CLUSTER_HEADER_STATUS_OFFSET], FEE_VALIDATED_VALUE);

    /* Write the cluster header to flash */
    if (((Std_ReturnType)E_OK) == Fee_WriteToFls(ClrHdr.StartAddr, Fee_aDataBuffer, FEE_CLUSTER_HEADER_STATUS_OFFSET + FEE_VIRTUAL_PAGE_SIZE))
    {
        /* Fls read job has been accepted */
        RetVal = MEMIF_JOB_PENDING;
    }
    else
    {
        /* Fls write job hasn't been accepted */
        RetVal = MEMIF_JOB_FAILED;
    }

    /* Schedule cluster format done job */
    Fee_eJob = FEE_JOB_INT_SCAN_CLR_FMT_DONE;
    
    return RetVal;
}

/*!
 * @brief Finalize format of first Fee cluster in current Fee cluster group.
 */
static MemIf_JobResultType Fee_JobInternalScanClusterFmtDone(void)
{
    MemIf_JobResultType RetVal;
    Fls_AddressType ClrStartAddr;
    Fls_LengthType ClrLength;

    /* Assign Index and ID of active cluster */
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].ActClr = Fee_uJobIntClrIt;
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].ActClrID = 1U;

    ClrStartAddr = Fee_ConfigPtr->clusterConfigPtr[Fee_uJobIntClrGrpIt].ClrPtr[Fee_uJobIntClrIt].StartAddr;
    ClrLength = Fee_GetClusterLength(Fee_uJobIntClrGrpIt, Fee_uJobIntClrIt);

    /* Initialize the block header pointer */
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].HdrAddrIt = ClrStartAddr + FEE_CLUSTER_OVERHEAD;

    /* Initialize the block data pointer */
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].DataAddrIt = ClrStartAddr + ClrLength;

    /* Move on to next cluster group */
    Fee_uJobIntClrGrpIt++;

    /* Scan next cluster group if any */
    RetVal = Fee_JobInternalScanCluster();

    return(RetVal);
}

/*!
 * @brief Check the block's size and type match the configuration, and its data pointer points to an acceptable area.
 */
static inline bool Fee_IsBlockMatchedConfig(uint16_t BlockIndex,
                                               uint16_t BlockRuntimeInfoIndex,
                                               const Fee_BlockType *BlockHder,
                                               Fls_AddressType DataAddr
                                              )
{
    bool RetVal = true;
    uint16_t AlignedBlockSize;

    if ( (0xFFFFU != BlockIndex) || (0xFFFFU != BlockRuntimeInfoIndex) )
    {
        AlignedBlockSize = Fee_AlignToVirtualPageSize(BlockHder->Length);

        if ((DataAddr <= (Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].DataAddrIt - AlignedBlockSize)) &&
            (DataAddr >= (Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].HdrAddrIt + (2U * FEE_BLOCK_OVERHEAD))))
        {
            if (0xFFFFU == BlockRuntimeInfoIndex)  /* Only checks non-foreign blocks */
            {
                if ((Fee_ConfigPtr->blockConfigPtr[BlockIndex].ClrGrp != Fee_uJobIntClrGrpIt) ||
                    (Fee_ConfigPtr->blockConfigPtr[BlockIndex].BlockSize != BlockHder->Length) ||
                    (Fee_ConfigPtr->blockConfigPtr[BlockIndex].ImmediateData != BlockHder->ImmediateBlock)
                   )
                {
                    /* Not matched with the configuration */
                    RetVal = false;;
                }
            }
        }
        else
        {
            /* Invalid area */
            RetVal = false;
        }
    }
    else
    {
        /* Both indexes are FFFF: Invalid block number */
        RetVal = false;
    }

    return RetVal;
}

/*!
 * @brief Update block run-time information.
 */
static inline void Fee_UpdateBlockRuntimeInfo(uint16_t BlockRuntimeInfoIndex,
                                              Fee_BlockStatusType BlockStatus,
                                              Fls_AddressType DataAddr,
                                              bool ImmediateBlock
                                             )
{

    /* Update block run-time information when:
     - new valid block is found
     - block was invalidated
     - block contains immediate data (basically just ignore INCONSISTENT non-immediate blocks) */

    if (FEE_BLOCK_INCONSISTENT != BlockStatus)
    {
        /* Mark invalidated blocks as invalid */
        if (FEE_BLOCK_INVALIDATED == BlockStatus)
        {
            Fee_aBlockInfo[BlockRuntimeInfoIndex].BlockStatus = FEE_BLOCK_INVALID;
        }
        else
        {
            Fee_aBlockInfo[BlockRuntimeInfoIndex].BlockStatus = BlockStatus;
        }

        /* Update the block data pointer */
        Fee_aBlockInfo[BlockRuntimeInfoIndex].DataAddr = DataAddr;
        Fee_aBlockInfo[BlockRuntimeInfoIndex].InvalidAddr = (Fee_uJobIntAddrIt + FEE_BLOCK_OVERHEAD) - FEE_VIRTUAL_PAGE_SIZE;
        Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].DataAddrIt = DataAddr;

    }
    else
    {
        /* header is valid but data is corrupted, in this case it is important to update
           DataAddrIt to avoid corrupted space, cluster swap is not needed */
        Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].DataAddrIt = DataAddr;
    }

    /* Check for blocks potentially overlapping the Reserved Area */

    /* Let Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt] be completely updated before calling Fee_ReservedAreaTargetedInClrGrp() */
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].HdrAddrIt = Fee_uJobIntAddrIt + FEE_BLOCK_OVERHEAD;

    if (false != Fee_ReservedAreaTargetedInClrGrp(Fee_uJobIntClrGrpIt))
    {
        if (true == ImmediateBlock)
        {
            /* Mark the imm. block as touching the Reserved Area (we are esp.
               interested only about inconsistent imm. blocks here, but it does not
               matter if also consistent imm. block will be marked) */
            Fee_TouchReservedAreaByBlock(BlockRuntimeInfoIndex);

        }
        else
        {
            /* Non-immediate data found in the Reserved Area => swap */
            bSwapToBePerformed = true;
            /* bSwapToBePerformed is not reset to FALSE upon recognition of the next written header =>
               the swap is mandatory. */
        }
    }

}

/*!
 * @brief Parse Fee block header.
 */
static MemIf_JobResultType Fee_JobInternalScanBlkHdrParse(const bool BufferValid)
{
    MemIf_JobResultType RetVal = MEMIF_JOB_OK;
    Fee_BlockStatusType BlockStatus;
    Fee_BlockType BlockHder;
    Fls_AddressType DataAddr;
    uint16_t BlockIndex;
    uint16_t BlockRuntimeInfoIndex = 0xFFFFU;  /* Default value: invalid block number */


    /* Deserialize block header from read buffer */
    BlockStatus = Fee_DeserializeBlockHdr(&BlockHder, &DataAddr, Fee_aDataBuffer);

    /* bSwapToBePerformed was set because previously processed block wasn't properly recognized.
       => perform a swap to prevent a risk that this block overlaps the Reserved Area. */
    if ((FEE_BLOCK_HEADER_INVALID  == BlockStatus) || (false == BufferValid))
    {
        /* Invalidate the block index so Fee won't skip the Fee_uJobBlockIndex block */
        Fee_uJobBlockIndex = 0xffffU;

        /* Move on to next block header */
        Fee_uJobIntAddrIt += FEE_BLOCK_OVERHEAD;

        /* Update the block header pointer */
        Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].HdrAddrIt = Fee_uJobIntAddrIt;

        /* Set swap-to-be-performed flag */
        bSwapToBePerformed = true;

        /* Read next header */
        RetVal = Fee_JobInternalScanBlockHdrRead();

    }
    else if (FEE_BLOCK_HEADER_BLANK == BlockStatus)
    {
        /* If SwapToBePerformed is set, it means that last header is corrupted and
           ONLY now the DataAddrIt variable will be updated to cause swap. */
        if (true == bSwapToBePerformed)
        {
           bSwapToBePerformed = false;
           Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].DataAddrIt = Fee_uJobIntAddrIt + (2U * FEE_BLOCK_OVERHEAD);
        }

        /* End of header list so move on to next cluster group */
        Fee_uJobIntClrGrpIt++;

        RetVal = Fee_JobInternalScanCluster();
    }
    else
    {
        /* Look for found block in the block configuration */
        BlockIndex = Fee_GetBlockIndex(BlockHder.BlockNumber);

        /* If block header matches Fee config and its data pointer is in an acceptable area: update its runtime info */
        if (true == Fee_IsBlockMatchedConfig(BlockIndex, BlockRuntimeInfoIndex, &BlockHder, DataAddr))
        {
           /* No foreign blocks found, use the index from our configuration */
           Fee_UpdateBlockRuntimeInfo(BlockIndex, BlockStatus, DataAddr, BlockHder.ImmediateBlock);
        }
        else
        {
           /* 1. Invalid block number (removed from Fee config...) next write cluster swap occurs. Or */
           /* 2. Block header doesn't match Fee config or the FEE algorithm... during next write cluster swap occurs */
           bSwapToBePerformed = true;
        }

        /* Move on to next block header */
        Fee_uJobIntAddrIt += FEE_BLOCK_OVERHEAD;

        /* Update the block header pointer */
        Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].HdrAddrIt = Fee_uJobIntAddrIt;

        /* Read next header */
        RetVal = Fee_JobInternalScanBlockHdrRead();
    }

    return RetVal;
}

/*!
 * @brief Read Fee cluster header.
 */
static MemIf_JobResultType Fee_JobInternalScanClusterHdrRead(void)
{
    MemIf_JobResultType RetVal;
    Fls_AddressType ReadAddress;


    /* Get address of current cluster */
    ReadAddress = Fee_ConfigPtr->clusterConfigPtr[Fee_uJobIntClrGrpIt].ClrPtr[Fee_uJobIntClrIt].StartAddr;

    /* Cluster header will be read directly from logical address, don't need translation here */
    if (((Std_ReturnType)E_OK) != Fls_Read(ReadAddress, Fee_aDataBuffer, FEE_CLUSTER_OVERHEAD))
    {
        /* Fls read job hasn't been accepted */
        RetVal = MEMIF_JOB_FAILED;
    }
    else
    {
        /* Fls read job has been accepted */
        RetVal = MEMIF_JOB_PENDING;
    }

    /* Schedule cluster scan job */
    Fee_eJob = FEE_JOB_INT_SCAN_CLR_HDR_PARSE;

    return RetVal;
}

/*!
 * @brief Initialize the cluster scan job.
 */
static MemIf_JobResultType Fee_JobInternalScan(void)
{
    MemIf_JobResultType RetVal = MEMIF_JOB_OK;

    /* Reset iterators to first cluster of first cluster group */
    Fee_uJobIntClrGrpIt = 0U;
    Fee_uJobIntClrIt = 0U;

    /* Schedule reading of first cluster header */
    RetVal = Fee_JobInternalScanClusterHdrRead();

    return RetVal;
}

/*!
 * @brief Check if all cluster have been scanned or not.
 */
static inline bool Fee_JobInternalScanClusterHdrDone(void)
{
    bool RetVal;

    /* Move on to next cluster */
    Fee_uJobIntClrIt++;

    if (Fee_uJobIntClrIt == Fee_ConfigPtr->clusterConfigPtr[Fee_uJobIntClrGrpIt].ClrCount)
    {
        /* Move on to next cluster group */
        Fee_uJobIntClrGrpIt++;
        Fee_uJobIntClrIt = 0U;
    }

    if (FEE_NUMBER_OF_CLUSTER_GROUPS == Fee_uJobIntClrGrpIt)
    {
        Fee_uJobIntClrGrpIt = 0U;

        /* Done scanning all cluster headers */
        RetVal = true;
    }
    else
    {
        /* There are still clusters to scan */
        RetVal = false;
    }

    return RetVal;
}

/*!
 * @brief Parse Fee cluster header.
 */
static MemIf_JobResultType Fee_JobIntalScanCluHdrParse(const bool BufferValid)
{
    MemIf_JobResultType RetVal;
    Fee_ClusterStatusType ClrStatus;
    Fls_AddressType CfgStartAddr;
    Fee_ClusterHeaderType ClrHdr;
    Fls_LengthType CfgClrSize;
    bool bScanClusterDone = false;

    /* Deserialize cluster parameters from the read buffer */
    ClrStatus = Fee_DeserializeClusterHdr(&ClrHdr, Fee_aDataBuffer);

    /* Get configured cluster start address and size */
    CfgStartAddr = Fee_ConfigPtr->clusterConfigPtr[Fee_uJobIntClrGrpIt].ClrPtr[Fee_uJobIntClrIt].StartAddr;
    CfgClrSize = Fee_GetClusterLength(Fee_uJobIntClrGrpIt, Fee_uJobIntClrIt);

    if ((true == BufferValid) &&
        (FEE_CLUSTER_VALID == ClrStatus) &&
        (ClrHdr.StartAddr == CfgStartAddr) &&
        (ClrHdr.Length == CfgClrSize)
       )
    {

        if (ClrHdr.ClrID > Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].ActClrID)
        {
            /* Found active cluster */
            Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].ActClr = Fee_uJobIntClrIt;
            Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].ActClrID = ClrHdr.ClrID;
        }
    }
    else
    {
        /* Invalid, inconsistent, move to the next one */
    }

    /* Move on to next cluster */
    bScanClusterDone = Fee_JobInternalScanClusterHdrDone();
    

    if (true == bScanClusterDone)
    {
        /* Done reading all cluster headers. Now scan active clusters */
        RetVal = Fee_JobInternalScanCluster();
    }
    else
    {
        /* Read next cluster header */
        RetVal = Fee_JobInternalScanClusterHdrRead();
    }

    return RetVal;
}

/*!
 * @brief Read Fee block.
 */
static MemIf_JobResultType Fee_JobReadBlock(void)
{
    MemIf_JobResultType RetVal = MEMIF_JOB_FAILED;
    Fee_BlockStatusType BlockStatus = FEE_BLOCK_VALID;
    Fls_AddressType BlockAddress = 0UL;
    BlockStatus = Fee_aBlockInfo[Fee_uJobBlockIndex].BlockStatus;
    BlockAddress = Fee_aBlockInfo[Fee_uJobBlockIndex].DataAddr;

    if (FEE_BLOCK_VALID == BlockStatus)
    {
        if (((Std_ReturnType)E_OK) != Fee_ReadFromFls(BlockAddress+Fee_uJobBlockOffset, Fee_pJobReadDataDestPtr, Fee_uJobBlockLength))
        {
            /* Fls read job hasn't been accepted */
            RetVal = MEMIF_JOB_FAILED;
        }
        else
        {
            /* Fls read job has been accepted */
            RetVal = MEMIF_JOB_PENDING;
        }
    }
    else if (FEE_BLOCK_INVALID == BlockStatus)
    {
        RetVal = MEMIF_BLOCK_INVALID;
    }
    else if ((FEE_BLOCK_INCONSISTENT == BlockStatus) || (FEE_BLOCK_INCONSISTENT_COPY == BlockStatus))
    {
        /* No special treatment of immediate blocks needed anymore */
        RetVal = MEMIF_BLOCK_INCONSISTENT;
    }
    else
    {
        /* Something is wrong... MEMIF_JOB_FAILED */
    }

    /* There is no consecutive job to schedule */
    Fee_eJob = FEE_JOB_DONE;

    return RetVal;
}

/*!
 * @brief Validate Fee block.
 */
static MemIf_JobResultType Fee_JobInternalSwapBlockVld(void)
{
    MemIf_JobResultType RetVal = MEMIF_JOB_OK;

    /* Serialize validation pattern to buffer */
    Fee_SerializeFlag(Fee_aDataBuffer, FEE_VALIDATED_VALUE);

    /* Write header to flash */
    if (((Std_ReturnType)E_OK) != Fee_WriteToFls(Fee_uJobIntHdrAddr - FEE_BLOCK_VALID_FLAG_AREA_SIZE, Fee_aDataBuffer, FEE_VIRTUAL_PAGE_SIZE))
    {
        /* Fls read job hasn't been accepted */
        RetVal = MEMIF_JOB_FAILED;
    }
    else
    {
        /* Fls read job has been accepted */
        RetVal = MEMIF_JOB_PENDING;
    }

    /* Finalize the write operation */
    Fee_eJob = FEE_JOB_INT_SWAP_BLOCK;

    return RetVal;
}

/*!
 * @brief Read data from source cluster to internal Fee buffer.
 */
static MemIf_JobResultType Fee_JobInternalSwapDataRead(const bool BufferValid)
{
    MemIf_JobResultType RetVal;
    Fls_AddressType ReadAddr;
    uint16_t AlignedBlockSize;
    uint16_t BlockSize;

    /* Get size of swapped block */
    BlockSize = Fee_GetBlockSize(Fee_uJobIntBlockIt);

    /* Get size of swapped block aligned to virtual page boundary */
    AlignedBlockSize = Fee_AlignToVirtualPageSize(BlockSize);

    /* Calculate remaining number of bytes to read */
    Fee_uJobBlockLength = (Fee_uJobIntDataAddr + AlignedBlockSize) - Fee_uJobIntAddrIt;

    if (false == BufferValid)
    {
        /* There was error while reading the source cluster.
           Change the block status to FEE_BLOCK_INCONSISTENT_COPY to mark the status change
           during swap, leave the block (in flash) INCONSITENT and move on to next block */
        Fee_aBlockInfo[Fee_uJobIntBlockIt].BlockStatus = FEE_BLOCK_INCONSISTENT_COPY;

        Fee_uJobIntBlockIt++;

        RetVal = Fee_JobInternalSwapBlock();
    }
    else if (0U == Fee_uJobBlockLength)
    {
        /* No more bytes to copy. Validate the block and move on to next block... */
        Fee_uJobIntBlockIt++;

        RetVal = Fee_JobInternalSwapBlockVld();
    }
    else
    {
        if (Fee_uJobBlockLength > FEE_DATA_BUFFER_SIZE)
        {
            Fee_uJobBlockLength = FEE_DATA_BUFFER_SIZE;
        }
        else
        {
            /* Block data fits into the buffer */
        }

        /* Calculate source address */
        ReadAddr = (Fee_aBlockInfo[Fee_uJobIntBlockIt].DataAddr + Fee_uJobIntAddrIt) - Fee_uJobIntDataAddr;

        /* Read the block data */
        if (((Std_ReturnType)E_OK) != Fee_ReadFromFls(ReadAddr, Fee_aDataBuffer, Fee_uJobBlockLength))
        {
            /* Fls read job hasn't been accepted */
            RetVal = MEMIF_JOB_FAILED;
        }
        else
        {
            /* Fls read job has been accepted */
            RetVal = MEMIF_JOB_PENDING;
        }

        Fee_eJob = FEE_JOB_INT_SWAP_DATA_WRITE;
    }

    return RetVal;
}

/*!
 * @brief Write data from internal Fee buffer to target cluster.
 */
static MemIf_JobResultType Fee_JobInternalSwapDataWrite(const bool BufferValid)
{
    MemIf_JobResultType RetVal = MEMIF_JOB_OK;

    if (true == BufferValid)
    {
        if (((Std_ReturnType)E_OK) == Fee_WriteToFls(Fee_uJobIntAddrIt, Fee_aDataBuffer, Fee_uJobBlockLength))
        {
            /* Fls read job has been accepted */
            RetVal = MEMIF_JOB_PENDING;
        }
        else
        {
            /* Fls write job hasn't been accepted */
            RetVal = MEMIF_JOB_FAILED;
        }

        Fee_uJobIntAddrIt += Fee_uJobBlockLength;

        Fee_eJob = FEE_JOB_INT_SWAP_DATA_READ;
    }
    else
    {
        RetVal = Fee_JobInternalSwapDataRead(false);
    }

    return RetVal;
}

/*!
 * @brief Finalize cluster validation.
 */
static MemIf_JobResultType Fee_JobInternalSwapClusterVldDone(void)
{
    MemIf_JobResultType RetVal;
    uint16_t BlockIt;
    Fls_AddressType HdrAddrIt;
    Fls_AddressType DataAddrIt;
    uint16_t BlockSize;
    uint16_t AlignedBlockSize;
    uint8_t BlockClusterGrp;


    /* Swap is done => re-initialize info about the blocks touching the Reserved Area within a cluster group where the swap has happened. */
    Fee_UntouchReservedAreaByClrGrp(Fee_uJobIntClrGrpIt);

    HdrAddrIt = Fee_ConfigPtr->clusterConfigPtr[Fee_uJobIntClrGrpIt].ClrPtr[Fee_uJobIntClrIt].StartAddr;
    DataAddrIt = HdrAddrIt + Fee_GetClusterLength(Fee_uJobIntClrGrpIt, Fee_uJobIntClrIt);

    /* Move on to the first block header */
    HdrAddrIt += FEE_CLUSTER_OVERHEAD;

    /* Sync block info */
   for (BlockIt = 0U; BlockIt < FEE_CRT_CFG_NR_OF_BLOCKS; BlockIt++)
    {
        BlockClusterGrp = Fee_GetBlockClusterGrp(BlockIt);

        if (Fee_uJobIntClrGrpIt == BlockClusterGrp)
        {
            /* Valid blocks and inconsistent blocks with allocated data were copied,
               update the block info and internal pointers accordingly.
            */
            if ((FEE_BLOCK_VALID == Fee_aBlockInfo[BlockIt].BlockStatus) ||
                (FEE_BLOCK_INCONSISTENT == Fee_aBlockInfo[BlockIt].BlockStatus) ||
                (FEE_BLOCK_INCONSISTENT_COPY == Fee_aBlockInfo[BlockIt].BlockStatus)
               )
            {
                /* Sync InvalidAddr */
                Fee_aBlockInfo[BlockIt].InvalidAddr = (HdrAddrIt + FEE_BLOCK_OVERHEAD) - FEE_VIRTUAL_PAGE_SIZE;

                /* Update the block data address info */
                if (FEE_BLOCK_VALID == Fee_aBlockInfo[BlockIt].BlockStatus)
                {
                    BlockSize = Fee_GetBlockSize(BlockIt);
                    /* Align block size to the virtual page boundary */
                    AlignedBlockSize = Fee_AlignToVirtualPageSize(BlockSize);
                    /* "Allocate" data already swapped (written to the flash) */
                    DataAddrIt -= AlignedBlockSize;
                    /* store the data address to the block runtime record */
                    Fee_aBlockInfo[BlockIt].DataAddr = DataAddrIt;
                }
                else
                {
                    /* (Inconsistent plain block in LEGACY == ON) or
                       (Inconsistent plain or immediate block in LEGACY == OFF) -- no data written (swapped, allocated)/available */
                    Fee_aBlockInfo[BlockIt].DataAddr = 0U;
                }

                if (FEE_BLOCK_INCONSISTENT_COPY == Fee_aBlockInfo[ BlockIt ].BlockStatus)
                {
                    BlockSize = Fee_GetBlockSize(BlockIt);
                    /* Align block size to the virtual page boundary */
                    AlignedBlockSize = Fee_AlignToVirtualPageSize(BlockSize);
                    /* "Allocate" data already swapped (written to the flash) */
                    DataAddrIt -= AlignedBlockSize;
                    /* Damaged data read during copy, but already allocated => inconsistent */
                    Fee_aBlockInfo[ BlockIt ].BlockStatus = FEE_BLOCK_INCONSISTENT;
                }

                /* Advance the header pointer */
                HdrAddrIt += FEE_BLOCK_OVERHEAD;
            }
            else
            {
                /* Not a usable block */
                /* clear the addresses of the block not present in the new active cluster */
                Fee_aBlockInfo[BlockIt].DataAddr = 0U;
                Fee_aBlockInfo[BlockIt].InvalidAddr = 0U;
            }
        }
    }

    /* Switch to the new cluster */
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].ActClr = Fee_uJobIntClrIt;
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].ActClrID++;
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].HdrAddrIt = HdrAddrIt;
    Fee_aClrGrpInfo[Fee_uJobIntClrGrpIt].DataAddrIt = DataAddrIt;

    /* restore original Fee_eJob */
    if (FEE_JOB_INT_SCAN_CLR == Fee_eJobIntOriginalJob)
    {
        Fee_uJobIntClrGrpIt++;
    }
    else
    {
        /* Do nothing */
    }

    /* Internal job has finished so transition from MEMIF_BUSYINTERNAL to MEMIF_BUSY */
    Fee_eModuleStatus = MEMIF_BUSY;

    /* And now cross fingers and re-schedule original job ... */
    Fee_eJob = Fee_eJobIntOriginalJob;

    RetVal = Fee_JobScheduleAfterSwapClusterVldDone();

    return RetVal;
}

/*!
 * @brief Write Fee block header to flash.
 */
static MemIf_JobResultType Fee_JobWriteHdr(void)
{
    MemIf_JobResultType RetVal;
    Fee_BlockType BlockHder;
    uint16_t AlignedBlockSize;
    uint8_t ClrGrpIndex;
    Fls_AddressType DataAddr;
    Fls_AddressType HdrAddr;

        /* Index of cluster group the Fee block belongs to */
        ClrGrpIndex = Fee_ConfigPtr->blockConfigPtr[Fee_uJobBlockIndex].ClrGrp;

        /* Get size, Blocknumber, type of Fee block */
        BlockHder.Length = Fee_ConfigPtr->blockConfigPtr[Fee_uJobBlockIndex].BlockSize;
        BlockHder.BlockNumber = Fee_ConfigPtr->blockConfigPtr[Fee_uJobBlockIndex].BlockNumber;
        BlockHder.ImmediateBlock = Fee_ConfigPtr->blockConfigPtr[Fee_uJobBlockIndex].ImmediateData;
        /* Align Fee block size to the virtual page boundary */
        AlignedBlockSize = Fee_AlignToVirtualPageSize(BlockHder.Length);

        if (false == Fee_ReservedAreaWritable())
        {
            /* The block won't fit into current cluster. Init swap sequence... */
            /* Get cluster group index the block belongs to */
            Fee_uJobIntClrGrpIt = ClrGrpIndex;

            RetVal = Fee_JobInternalSwap();
        }
        else
        {
            /* Calculate data and header addresses */
            DataAddr = Fee_aClrGrpInfo[ClrGrpIndex].DataAddrIt - AlignedBlockSize;
            HdrAddr = Fee_aClrGrpInfo[ClrGrpIndex].HdrAddrIt;

            /* Serialize block header to the write buffer */
            Fee_SerializeBlockHdr(&BlockHder,DataAddr,Fee_aDataBuffer);

            /* Write header to flash */
            if (((Std_ReturnType)E_OK) != Fee_WriteToFls(HdrAddr, Fee_aDataBuffer, FEE_BLOCK_HEADER_STATUS_OFFSET))
            {
                /* Fls read job hasn't been accepted */
                RetVal = MEMIF_JOB_FAILED;
            }
            else
            {
                /* Fls read job has been accepted */
                RetVal = MEMIF_JOB_PENDING;
            }
        }
    return RetVal;
}

/*!
 * @brief Write Fee block data to flash.
 */
static MemIf_JobResultType Fee_JobWriteBlockData(void)
{
    Fls_AddressType DataAddr;
    MemIf_JobResultType RetVal;
    uint16_t BlockSize;
    uint16_t AlignedBlockSize;
    Fls_LengthType WriteLength;
    uint8_t ClrGrp;
    const uint8_t *WriteDataPtr;
    bool IsReservedAreaTouched;

        /* Get size of Fee block */
        BlockSize = Fee_ConfigPtr->blockConfigPtr[Fee_uJobBlockIndex].BlockSize;

        /* Align Fee block size to the virtual page boundary */
        AlignedBlockSize = Fee_AlignToVirtualPageSize(BlockSize);

        ClrGrp = Fee_ConfigPtr->blockConfigPtr[Fee_uJobBlockIndex].ClrGrp;

        /* Header has been written so update both header and data addresses;
           no special treatment of immediate blocks needed anymore */
        Fee_aClrGrpInfo[ClrGrp].HdrAddrIt += FEE_BLOCK_OVERHEAD;
        Fee_aClrGrpInfo[ClrGrp].DataAddrIt -= AlignedBlockSize;
        /* When FEE_BLOCK_ALWAYS_AVAILABLE is enabled, Fee_BlockInfo is not updated immediatelly
           upon a header write. In case of FEE Cancel, the information about the incosistent
           (due to incomplete write) immediate block potentially residing in the Reserved Area
           might be lost and therefore it need to be extra handled. */
        IsReservedAreaTouched = Fee_ReservedAreaTargetedInClrGrp(ClrGrp);
        if ((false != Fee_ConfigPtr->blockConfigPtr[Fee_uJobBlockIndex].ImmediateData) && (false != IsReservedAreaTouched))
        {
            /* Store the information about immediate blook touching the Reserved Area. */
            Fee_TouchReservedAreaByBlock(Fee_uJobBlockIndex);
        }

        if (BlockSize < FEE_VIRTUAL_PAGE_SIZE)
        {
            /* Go through buffer... */
            Fee_CopyDataToPageBuffer(Fee_pJobWriteDataDestPtr, Fee_aDataBuffer, BlockSize);
            WriteDataPtr = Fee_aDataBuffer;
            WriteLength = FEE_VIRTUAL_PAGE_SIZE;

            /* Schedule write of validation pattern */
            Fee_eJob = FEE_JOB_WRITE_VALIDATE;
        }
        else
        {
            WriteDataPtr = Fee_pJobWriteDataDestPtr;

            if (AlignedBlockSize == BlockSize)
            {
                WriteLength = BlockSize;

                /* Schedule write of validation pattern */
                Fee_eJob = FEE_JOB_WRITE_VALIDATE;
            }
            else
            {
                WriteLength = ((uint32_t)AlignedBlockSize) - FEE_VIRTUAL_PAGE_SIZE;

                /* Schedule write of unaligned data */
                Fee_eJob = FEE_JOB_WRITE_UNALIGNED_DATA;
            }
        }

        DataAddr = Fee_aClrGrpInfo[ClrGrp].DataAddrIt;

        if (((Std_ReturnType)E_OK) == Fee_WriteToFls(DataAddr, WriteDataPtr,WriteLength))
        {
            /* Fls write job has been accepted */
            RetVal = MEMIF_JOB_PENDING;
        }
        else
        {
            /* Fls write job hasn't been accepted */
            RetVal = MEMIF_JOB_FAILED;
        }
    return RetVal;
}

/*!
 * @brief Write Fee block to flash.
 */
static MemIf_JobResultType Fee_JobWriteBlock(void)
{
    MemIf_JobResultType RetVal;

            /* Write block header */
            RetVal = Fee_JobWriteHdr();

            /* Schedule next job */
            if (FEE_JOB_INT_SWAP_CLR_FMT == Fee_eJob)
            {
                /* Block didn't fit into the cluster. Cluster swap has been enforced... */
            }
            else
            {
                Fee_eJob = FEE_JOB_WRITE_DATA;
            }

    return RetVal;
}

/*!
 * @brief Write unaligned rest of Fee block data to flash.
 */
static MemIf_JobResultType Fee_JobWriteBlockUnalignedData(void)
{
    MemIf_JobResultType RetVal;
    Fls_AddressType DataAddr;
    uint16_t WriteOffset;
    uint16_t WriteLength;
    uint16_t BlockSize;
    uint16_t ClrGrp;

        /* Index of cluster group the Fee block belongs to */
        ClrGrp = Fee_ConfigPtr->blockConfigPtr[Fee_uJobBlockIndex].ClrGrp;
        /* Get size of Fee block */
        BlockSize = Fee_ConfigPtr->blockConfigPtr[Fee_uJobBlockIndex].BlockSize;

        /* Calculate number of bytes already written */
        WriteOffset = (BlockSize / FEE_VIRTUAL_PAGE_SIZE) * FEE_VIRTUAL_PAGE_SIZE;

        /* Calculate Length remaining data to write */
        WriteLength = BlockSize % FEE_VIRTUAL_PAGE_SIZE;

        Fee_CopyDataToPageBuffer(&Fee_pJobWriteDataDestPtr[WriteOffset], Fee_aDataBuffer,WriteLength);

        /* No special treatment of immediate data needed any more */
        DataAddr = Fee_aClrGrpInfo[ClrGrp].DataAddrIt;


        if (((Std_ReturnType)E_OK) == Fee_WriteToFls(DataAddr+WriteOffset, Fee_aDataBuffer, FEE_VIRTUAL_PAGE_SIZE))
        {
            /* Fls write job has been accepted */
            RetVal = MEMIF_JOB_PENDING;
        }
        else
        {
            /* Fls write job hasn't been accepted */
            RetVal = MEMIF_JOB_FAILED;
        }

        /* Schedule write of validation pattern */
        Fee_eJob = FEE_JOB_WRITE_VALIDATE;
    return RetVal;
}

/*!
 * @brief Validate Fee block by writing validation flag to flash.
 */
static MemIf_JobResultType Fee_JobWriteBlockValidate(void)
{
    MemIf_JobResultType RetVal;
    Fls_AddressType HdrAddr;
    uint8_t ClrGrpIndex;

        /* Index of cluster group the Fee block belongs to */
        ClrGrpIndex = Fee_ConfigPtr->blockConfigPtr[Fee_uJobBlockIndex].ClrGrp;
		
        /* Serialize validation pattern to buffer */
        Fee_SerializeFlag(Fee_aDataBuffer, FEE_VALIDATED_VALUE);

        /* Use ClrGrp info */
        HdrAddr = Fee_aClrGrpInfo[ClrGrpIndex].HdrAddrIt - FEE_BLOCK_VALID_FLAG_AREA_SIZE;

        /* Write validation pattern to flash */
        if (((Std_ReturnType)E_OK) == Fee_WriteToFls(HdrAddr, Fee_aDataBuffer, FEE_VIRTUAL_PAGE_SIZE))
        {
            /* Fls read job has been accepted */
            RetVal = MEMIF_JOB_PENDING;
        }
        else
        {
            /* Fls write job hasn't been accepted */
            RetVal = MEMIF_JOB_FAILED;
        }

        /* Finalize the write operation */
        Fee_eJob = FEE_JOB_WRITE_DONE;
    return RetVal;
}

/*!
 * @brief Finalize validation of Fee block.
 */
static MemIf_JobResultType Fee_JobWriteBlockDone(void)
{

    Fls_AddressType DataAddr;
    Fls_AddressType HdrAddr;
    uint8_t ClrGrpIndex;

    /* Index of cluster group the Fee block belongs to */
    ClrGrpIndex = Fee_ConfigPtr->blockConfigPtr[Fee_uJobBlockIndex].ClrGrp;

    /* Calculate data and header addresses */
    DataAddr = Fee_aClrGrpInfo[ClrGrpIndex].DataAddrIt;
    HdrAddr = Fee_aClrGrpInfo[ClrGrpIndex].HdrAddrIt;

    /* Update the block address info */
    Fee_aBlockInfo[Fee_uJobBlockIndex].DataAddr = DataAddr;
    Fee_aBlockInfo[Fee_uJobBlockIndex].InvalidAddr = HdrAddr - FEE_VIRTUAL_PAGE_SIZE;
    /* Mark the Fee block as valid */
    Fee_aBlockInfo[Fee_uJobBlockIndex].BlockStatus = FEE_BLOCK_VALID;

    /* No more Fls jobs to schedule */
    Fee_eJob = FEE_JOB_DONE;

    return(MEMIF_JOB_OK);
}

/*!
 * @brief Invalidate Fee block by writing the invalidation flag to flash.
 */
static MemIf_JobResultType Fee_JobInvalidateBlock(void)
{
    MemIf_JobResultType RetVal = MEMIF_JOB_OK;

    if (FEE_BLOCK_INVALID == Fee_aBlockInfo[Fee_uJobBlockIndex].BlockStatus)
    {
        /* No more subsequent Fls jobs to schedule */
        Fee_eJob = FEE_JOB_DONE;

        /* Fee job completed */
        RetVal = MEMIF_JOB_OK;
    }
    else
    {
        if (FEE_BLOCK_NEVER_WRITTEN == Fee_aBlockInfo[Fee_uJobBlockIndex].BlockStatus)
        {
            /* set status as INVALID */
            Fee_aBlockInfo[Fee_uJobBlockIndex].BlockStatus = FEE_BLOCK_INVALID;

            /* No more subsequent Fls jobs to schedule */
            Fee_eJob = FEE_JOB_DONE;

            /* Fee job completed */
            RetVal = MEMIF_JOB_OK;
        }
        else
        {
            /* Serialize invalidation pattern to buffer */
            Fee_SerializeFlag(Fee_aDataBuffer, FEE_INVALIDATED_VALUE);

            /* Write invalidation pattern to flash */
            if (((Std_ReturnType)E_OK) == Fee_WriteToFls(Fee_aBlockInfo[Fee_uJobBlockIndex].InvalidAddr, Fee_aDataBuffer, FEE_VIRTUAL_PAGE_SIZE))
            {
                /* Fls read job has been accepted */
                RetVal = MEMIF_JOB_PENDING;
            }
            else
            {
                /* Fls write job hasn't been accepted */
                RetVal = MEMIF_JOB_FAILED;
            }

            /* Finalize the invalidation operation */
            Fee_eJob = FEE_JOB_INVAL_BLOCK_DONE;
        }
    }

    return RetVal;
}

/*!
 * @brief Finalize invalidation of Fee block.
 */
static MemIf_JobResultType Fee_JobInvalidateBlockDone(void)
{
    /* Mark the Fee block as in valid */
    Fee_aBlockInfo[Fee_uJobBlockIndex].BlockStatus = FEE_BLOCK_INVALID;

    /* No more subsequent Fls jobs to schedule */
    Fee_eJob = FEE_JOB_DONE;

    /* Fee job completed */
    return(MEMIF_JOB_OK);
}

/*!
 * @brief Erase (pre-allocate) immediate Fee block.
 */
static MemIf_JobResultType Fee_JobEraseImmediateBlock(void)
{
    MemIf_JobResultType RetVal = MEMIF_JOB_OK;


        /* In standard mode, perform swap if reserved area has been touched, or do nothing. */
        if (false == Fee_ReservedAreaWritable())
        {
            /* Reserved area is not writable, force the swap */
            Fee_uJobIntClrGrpIt = Fee_ConfigPtr->blockConfigPtr[Fee_uJobBlockIndex].ClrGrp;              /* Get cluster group index */
            RetVal = Fee_JobInternalSwap();
        }
        else
        {
            /* Do effectively nothing */
            Fee_eJob = FEE_JOB_DONE;
        }

    return RetVal;
}

/*!
 * @brief Schedule subsequent jobs.
 */
static MemIf_JobResultType Fee_JobSchedule(void)
{
    /* Jump to the current Fee job through function pointer */
    return Fee_JobScheduleLookupTable[Fee_eJob]();
}

/*!
 * @brief Schedule subsequent jobs.
 */
static MemIf_JobResultType Fee_JobScheduleAfterSwapClusterVldDone(void)
{
    MemIf_JobResultType eRetVal = MEMIF_JOB_FAILED;

    switch (Fee_eJob)
    {
        /* Fee_Write() related jobs */
        case FEE_JOB_WRITE:
            eRetVal = Fee_JobWriteBlock();
            break;
        /* Fee_EraseImmediateBlock() related jobs */
        case FEE_JOB_ERASE_IMMEDIATE:
            eRetVal = Fee_JobEraseImmediateBlock();
            break;
        default:
            /* Compiler_Warning: This default branch will never be reached in FTE */
            break;
    }

    return(eRetVal);
}

/*!
 * @brief Checks whether the block specified by Fee_JobBlockIndex is writable into the reserved area.
 */
static bool Fee_ReservedAreaWritable(void)
{
    bool RetVal;
    uint16_t BlockSize;
    uint16_t AlignedBlockSize;
    Fls_LengthType AvailClrSpace;
    uint8_t ClrGrpIndex;
    uint32_t ReservedSpace;
    bool ImmediateData;
    bool IsReservedAreaTouched;

    /* Index of cluster group the Fee block belongs to */
    ClrGrpIndex = Fee_GetBlockClusterGrp(Fee_uJobBlockIndex);

    /* Reserved space of cluster group*/
    ReservedSpace = Fee_ConfigPtr->clusterConfigPtr[ClrGrpIndex].ReservedSize;

    /* Calculate available space in active cluster */
    AvailClrSpace = Fee_aClrGrpInfo[ClrGrpIndex].DataAddrIt - Fee_aClrGrpInfo[ClrGrpIndex].HdrAddrIt;

    /* Get size of Fee block */
    BlockSize = Fee_GetBlockSize(Fee_uJobBlockIndex);

    /* Align Fee block size to the virtual page boundary */
    AlignedBlockSize = Fee_AlignToVirtualPageSize(BlockSize);

    /* Hard Stop condition: One FEE_BLOCK_OVERHEAD must be left blank to have a clear separation
       between header block space and data block space */
    if ((((uint32_t)AlignedBlockSize) + (2U * FEE_BLOCK_OVERHEAD)) > AvailClrSpace)
    {
        RetVal = false;
    }
    else if ((((uint32_t)AlignedBlockSize) + (2U * FEE_BLOCK_OVERHEAD) + ReservedSpace) > AvailClrSpace)
    {
        ImmediateData = Fee_GetBlockImmediate(Fee_uJobBlockIndex);
        /* Block targets reserved area */
        IsReservedAreaTouched = Fee_ReservedAreaTouchedByBlock(Fee_uJobBlockIndex);
        if ((true == ImmediateData) && (false == IsReservedAreaTouched))
        {
            /* Immediate block - only writable if not already present in the reserved area */
            if (((0UL == Fee_aBlockInfo[Fee_uJobBlockIndex].DataAddr) && (0UL == Fee_aBlockInfo[Fee_uJobBlockIndex].InvalidAddr)) ||
               /* The block is written in the cluster - is it written in the reserved area? */
                ((FEE_BLOCK_OVERHEAD + ReservedSpace) <= (Fee_aBlockInfo[Fee_uJobBlockIndex].DataAddr - (Fee_aBlockInfo[Fee_uJobBlockIndex].InvalidAddr + FEE_VIRTUAL_PAGE_SIZE)))
               )
            {
                /* The block is not written in the cluster at all => writable */
                /* or The block is not written in the reserved area */
                RetVal = true;
            }
            else
            {
                RetVal = false;
            }
        }
        else
        {
            RetVal = false;
        }
    }
    else
    {
        /* Block does not target reserved area */
        RetVal = true;
    }
    return RetVal;
}

/*!
 * @brief Schedule the error jobs of Fee.
 */
static void Fee_JobErrorSchedule(void)
{
    uint8_t ClrGrpIndex;

    switch (Fee_eJob)
    {
        /* Error while reading block header. Treat block header as invalid and stop reading further block headers */
        case FEE_JOB_INT_SCAN_BLOCK_HDR_PARSE:
            Fee_eJobResult = Fee_JobInternalScanBlkHdrParse(false);
            break;

        /* Error while reading cluster header. Treat cluster as invalid and move on to next cluster */
        case FEE_JOB_INT_SCAN_CLR_HDR_PARSE:
            Fee_eJobResult = Fee_JobIntalScanCluHdrParse(false);
            break;

        /* Error while reading data from source cluster. Leave block as INCONSISTENT and move on to next block */
        case FEE_JOB_INT_SWAP_DATA_WRITE:
            Fee_eJobResult = Fee_JobInternalSwapDataWrite(false);
            break;

        /* for all the following cases based on the Fee module status error notification will be called */
        case FEE_JOB_WRITE:
        case FEE_JOB_WRITE_DATA:
        case FEE_JOB_WRITE_UNALIGNED_DATA:
        case FEE_JOB_ERASE_IMMEDIATE:
        case FEE_JOB_WRITE_VALIDATE:
        case FEE_JOB_WRITE_DONE:
        {
            ClrGrpIndex = Fee_GetBlockClusterGrp(Fee_uJobBlockIndex);

            /* Something wrong in FlashMemory (cluster swap occurs to next write) */
            Fee_aClrGrpInfo[ClrGrpIndex].DataAddrIt = Fee_aClrGrpInfo[ClrGrpIndex].HdrAddrIt + (2U * FEE_BLOCK_OVERHEAD);
            Fee_eJobResult = Fls_GetJobResult();
            Fee_eModuleStatus = MEMIF_IDLE;

            /* Call job error notification function */
            break;
        }

        case FEE_JOB_READ:
        case FEE_JOB_INVAL_BLOCK:
        case FEE_JOB_INVAL_BLOCK_DONE:
        case FEE_JOB_INT_SCAN:
        case FEE_JOB_INT_SCAN_CLR:
        case FEE_JOB_INT_SCAN_CLR_FMT:
        case FEE_JOB_INT_SCAN_CLR_FMT_DONE:
        case FEE_JOB_INT_SWAP_BLOCK:
        case FEE_JOB_INT_SWAP_CLR_FMT:
        case FEE_JOB_INT_SWAP_DATA_READ:
        case FEE_JOB_INT_SWAP_CLR_VLD_DONE:
        case FEE_JOB_DONE:
        default:
            Fee_eJobResult = Fls_GetJobResult();

            Fee_eModuleStatus = MEMIF_IDLE;

            /* Call job error notification function */

            break;
    }
}

/*!
 * @brief Read data from Fls, translate the emulation to logical address in sector retirement mode.
 */
static Std_ReturnType Fee_ReadFromFls(Fls_AddressType SourceAddress,
                                      uint8_t *TargetAddressPtr,
                                      Fls_LengthType Length
                                     )
{
    return Fls_Read(SourceAddress, TargetAddressPtr, Length);
}

/*!
 * @brief Write data to Fls, translate the emulation to logical address in sector retirement mode.
 */
static Std_ReturnType Fee_WriteToFls(Fls_AddressType TargetAddress,
                                     const uint8_t *SourceAddressPtr,
                                     Fls_LengthType Length
                                    )
{
    return Fls_Write(TargetAddress, SourceAddressPtr, Length);
}

/*!
 * @brief Erase the whole cluster, translate the emulation to logical address in sector retirement mode and erase a sector at a time.
 */
static Std_ReturnType Fee_EraseCluster(uint8_t ClrGrpIt,
                                       uint8_t ClrIt
                                      )
{

    Fls_AddressType clusterAddress;
    Fls_LengthType  ClusterLength;

    /* Get address and size of cluster in the cluster group */
    clusterAddress = Fee_ConfigPtr->clusterConfigPtr[ClrGrpIt].ClrPtr[ClrIt].StartAddr;
    ClusterLength  = Fee_GetClusterLength(ClrGrpIt, ClrIt);

    return Fls_Erase(clusterAddress, ClusterLength);
}

/*!
 * @brief Return the configured length of the selected cluster.
 */
static inline Fls_LengthType Fee_GetClusterLength(uint8_t ClrGrpIt,
                                                  uint8_t ClrIt
                                                 )
{
    return Fee_ConfigPtr->clusterConfigPtr[ClrGrpIt].ClrPtr[ClrIt].Length; /* Read the value from the generated cluster table. */

}



/*==================================================================================================
 *                                       GLOBAL FUNCTIONS
==================================================================================================*/
/*!
 * @brief Start the erase-immediate service for a logical block.
 */
#if !defined(FEE_LIGHT_MODE) || (FEE_LIGHT_MODE == 0U)
Std_ReturnType Fee_EraseImmediateBlock (uint16_t BlockNumber)
{
    Std_ReturnType RetVal;
    uint16_t BlockIndex = Fee_GetBlockIndex(BlockNumber);
    uint32_t criticalState = FEE_ENTER_CRITICAL_SECTION();

   if((0xFFFFU == BlockIndex) || (MEMIF_IDLE != Fee_eModuleStatus))
    {
        RetVal = (Std_ReturnType)E_NOT_OK;
    }
    else
    {
        /* Stage the erase-immediate job context. */
        Fee_uJobBlockIndex = BlockIndex;
        Fee_eJob = FEE_JOB_ERASE_IMMEDIATE;
        Fee_eModuleStatus = MEMIF_BUSY;

        /* Mark the job as pending so the scheduler can advance it. */
        Fee_eJobResult = MEMIF_JOB_PENDING;

        RetVal = (Std_ReturnType)E_OK;
    }

    FEE_EXIT_CRITICAL_SECTION(criticalState);

    return RetVal;
}
#endif /* !FEE_LIGHT_MODE */

/*!
 * @brief Return the result of the last accepted FEE job.
 */
MemIf_JobResultType Fee_GetJobResult (void)
{
    MemIf_JobResultType RetVal = Fee_eJobResult;
    return RetVal;
}

/*!
 * @brief Return the current MEMIF state reported by the FEE driver.
 */
MemIf_StatusType Fee_GetStatus (void)
{
    return(Fee_eModuleStatus);
}

/*!
 * @brief Initialize the FEE runtime and schedule the startup scan.
 */
void Fee_Init (const Fee_ModuleUserConfig_t * ConfigPtr)
{
    uint32_t InvalIndex;
	DEV_ASSERT(NULL_PTR != ConfigPtr);
	DEV_ASSERT(MEMIF_UNINIT == Fee_eModuleStatus);

    (void)ConfigPtr;
	if (NULL_PTR == ConfigPtr)
    {
        
    }
    else if (MEMIF_UNINIT!= Fee_eModuleStatus)
    {
        
    }
	else
	{
		
		Fee_ConfigPtr = ConfigPtr;
		/* Initialize the flash backend used by FEE. */
		Fls_Init(Fee_ConfigPtr->flashConfigPtr);
		
        /* Clear all logical block runtime records. */
        for (InvalIndex = 0U; InvalIndex < Fee_ConfigPtr->blockCnt; InvalIndex++)
        {

            /* Start from the never-written state until the scan discovers valid data. */
            Fee_aBlockInfo[InvalIndex].BlockStatus = FEE_BLOCK_NEVER_WRITTEN;
            Fee_aBlockInfo[InvalIndex].DataAddr = 0U;
            Fee_aBlockInfo[InvalIndex].InvalidAddr = 0U;
        }
        /* Clear the active-cluster identifiers for every cluster group. */
        for (InvalIndex = 0U; InvalIndex < Fee_ConfigPtr->clusterCnt; InvalIndex++)
        {
            Fee_aClrGrpInfo[InvalIndex].ActClrID = 0U;
        }
        /* Queue the initialization scan as the first background job. */
        Fee_eJob = FEE_JOB_INT_SCAN;
        Fee_eJobResult = MEMIF_JOB_PENDING;
	}
}

/*!
 * @brief Start the invalidate-block service for a logical block.
 */
#if !defined(FEE_LIGHT_MODE) || (FEE_LIGHT_MODE == 0U)
Std_ReturnType Fee_InvalidateBlock (uint16_t BlockNumber)
{
    Std_ReturnType RetVal = (Std_ReturnType)E_OK;
    uint16_t BlockIndex = Fee_GetBlockIndex(BlockNumber);
    uint32_t criticalState = FEE_ENTER_CRITICAL_SECTION();

   if((0xFFFFU == BlockIndex) || (MEMIF_IDLE != Fee_eModuleStatus))
    {
        RetVal = (Std_ReturnType)E_NOT_OK;
    }
    else
    {
        /* Stage the invalidate-block job context. */
        Fee_uJobBlockIndex = BlockIndex;

        Fee_eJob = FEE_JOB_INVAL_BLOCK;

        Fee_eModuleStatus = MEMIF_BUSY;

        /* Mark the job as pending so the scheduler can advance it. */
        Fee_eJobResult = MEMIF_JOB_PENDING;

        RetVal = (Std_ReturnType)E_OK;
    }

    FEE_EXIT_CRITICAL_SECTION(criticalState);

    return RetVal;
}
#endif /* !FEE_LIGHT_MODE */

/*!
 * @brief Handle a successful completion callback from the flash backend.
 */
void Fee_JobEndNotification (void)
{
    if ((MEMIF_UNINIT == Fee_eModuleStatus) && (MEMIF_JOB_OK == Fee_eJobResult) && (FEE_JOB_DONE == Fee_eJob))
    { }
    else
    {
        if (FEE_JOB_DONE == Fee_eJob)
        {
            /* The final flash step of the current job has completed. */
            Fee_eJobResult = Fls_GetJobResult();
            Fee_eModuleStatus = MEMIF_IDLE;

            /* No additional action is required in this callback path. */
        }
        else
        {
            Fee_eJobResult = Fee_JobSchedule();
            if (MEMIF_JOB_OK == Fee_eJobResult)
            {
                Fee_eModuleStatus = MEMIF_IDLE;

                /* The state machine has reached its terminal success state. */
             }
             else if (MEMIF_JOB_PENDING == Fee_eJobResult)
             {
                /* Another flash step is still in progress. */
             }
             else
             {
                 Fee_eModuleStatus = MEMIF_IDLE;

                 /* The state machine reported an error condition. */
              }
          }
    }
}

/*!
 * @brief Handle a failed completion callback from the flash backend.
 */
void Fee_JobErrorNotification (void)
{
    if ((MEMIF_UNINIT == Fee_eModuleStatus) && (MEMIF_JOB_OK == Fee_eJobResult) && (FEE_JOB_DONE == Fee_eJob))
    {}
    else
    {
        if (MEMIF_JOB_CANCELED == Fee_eJobResult)
        {
            /* Ignore the backend callback for explicit cancel requests. The cancel path already owns follow-up handling. */
        }
        else
        {
              /* Hand control to the internal error scheduler. */
              Fee_JobErrorSchedule();
        }
    }
}

/*!
 * @brief Advance the active public or internal FEE job.
 */
void Fee_MainFunction (void)
{
	Fls_MainFunction();
    if (MEMIF_JOB_PENDING == Fee_eJobResult)
    {
        switch (Fee_eJob)
        {
            /* These entry jobs ask the scheduler for the next state transition. */
            case FEE_JOB_INT_SCAN:
            case FEE_JOB_READ:
            case FEE_JOB_WRITE:
            case FEE_JOB_INVAL_BLOCK:
            case FEE_JOB_ERASE_IMMEDIATE:
                Fee_eJobResult = Fee_JobSchedule();
                break;
            /* These jobs wait for notification callbacks or terminal status. */
            case FEE_JOB_WRITE_DATA:
            case FEE_JOB_WRITE_UNALIGNED_DATA:
            case FEE_JOB_WRITE_VALIDATE:
            case FEE_JOB_WRITE_DONE:
            case FEE_JOB_INVAL_BLOCK_DONE:
            case FEE_JOB_INT_SCAN_CLR_HDR_PARSE:
            case FEE_JOB_INT_SCAN_CLR:
            case FEE_JOB_INT_SCAN_CLR_FMT:
            case FEE_JOB_INT_SCAN_CLR_FMT_DONE:
            case FEE_JOB_INT_SCAN_BLOCK_HDR_PARSE:
            case FEE_JOB_INT_SWAP_BLOCK:
            case FEE_JOB_INT_SWAP_CLR_FMT:
            case FEE_JOB_INT_SWAP_DATA_READ:
            case FEE_JOB_INT_SWAP_DATA_WRITE:
            case FEE_JOB_INT_SWAP_CLR_VLD_DONE:
            case FEE_JOB_DONE:
            default:
                /* Internal state already owns the next transition. */
                break;
        }

        if (MEMIF_JOB_PENDING == Fee_eJobResult)
        {
            /* The active flash step is still running. */
        }
        else if (MEMIF_JOB_OK == Fee_eJobResult)
        {
            Fee_eModuleStatus = MEMIF_IDLE;
            /* The job completed successfully. */
        }
        else
        {
            Fee_eModuleStatus = MEMIF_IDLE;
            /* The job completed with an error result. */
        }
    }
}

/*!
 * @brief Start an asynchronous read for the requested logical block.
 */
Std_ReturnType Fee_Read (uint16_t BlockNumber, uint16_t BlockOffset, uint8_t * DataBufferPtr, uint16_t Length)
{
    Std_ReturnType RetVal = (Std_ReturnType)E_NOT_OK;
    uint16_t BlockIndex = Fee_GetBlockIndex(BlockNumber);
    uint32_t criticalState = FEE_ENTER_CRITICAL_SECTION();

    if((0xFFFFU == BlockIndex) || (MEMIF_IDLE != Fee_eModuleStatus))
    {
        RetVal = (Std_ReturnType)E_NOT_OK;
    }
    else
    {
        /* Stage the read job context. */
        Fee_uJobBlockIndex = BlockIndex;

        Fee_uJobBlockOffset = BlockOffset;

        Fee_uJobBlockLength = Length;

        Fee_pJobReadDataDestPtr = DataBufferPtr;

        Fee_eJob = FEE_JOB_READ;

        Fee_eModuleStatus = MEMIF_BUSY;

        /* Mark the job as pending so the scheduler can advance it. */
        Fee_eJobResult = MEMIF_JOB_PENDING;

        RetVal = (Std_ReturnType)E_OK;
    }

    FEE_EXIT_CRITICAL_SECTION(criticalState);

    return RetVal;
}

/*!
 * @brief Start an asynchronous write for the requested logical block.
 */
Std_ReturnType Fee_Write (uint16_t BlockNumber, const uint8_t * DataBufferPtr)
{
    Std_ReturnType RetVal = (Std_ReturnType)E_OK;
    uint16_t BlockIndex = Fee_GetBlockIndex(BlockNumber);
    uint32_t criticalState = FEE_ENTER_CRITICAL_SECTION();

   if((0xFFFFU == BlockIndex) || (MEMIF_IDLE != Fee_eModuleStatus))
    {
        RetVal = (Std_ReturnType)E_NOT_OK;
    }
    else
    {
        /* Stage the write job context. */
        Fee_uJobBlockIndex = BlockIndex;

        Fee_pJobWriteDataDestPtr = DataBufferPtr;

        Fee_eJob = FEE_JOB_WRITE;

        Fee_eModuleStatus = MEMIF_BUSY;

        /* Mark the job as pending so the scheduler can advance it. */
        Fee_eJobResult = MEMIF_JOB_PENDING;
    }

    FEE_EXIT_CRITICAL_SECTION(criticalState);

    return RetVal;
}

/*!
 * @brief Cancel the currently active FEE job when possible.
 */
Std_ReturnType Fee_Cancel(void)
{
    Std_ReturnType RetVal = (Std_ReturnType)E_OK;
    bool callErrorNotification = false;
    uint32_t criticalState = FEE_ENTER_CRITICAL_SECTION();

    if (MEMIF_UNINIT == Fee_eModuleStatus)
    {
        RetVal = (Std_ReturnType)E_NOT_OK;
    }
    else
    {
        /* Cancel the active flash job if the backend is busy. */
        if (MEMIF_BUSY == Fee_eModuleStatus)
        {
            Fee_eJobResult = MEMIF_JOB_CANCELED;
            Fee_eJob = FEE_JOB_DONE;
            Fls_Cancel();
            Fee_eModuleStatus = MEMIF_IDLE;
            /* Internal jobs do not raise additional notifications on cancel. */
        }
        else
        {
            /* Reuse the standard error callback path when no job is active. */
            callErrorNotification = true;
        }
    }

    FEE_EXIT_CRITICAL_SECTION(criticalState);

    if (callErrorNotification)
    {
        Fee_JobErrorNotification();
    }

    return RetVal;
}

#ifdef __cplusplus
}
#endif

/** @} */
