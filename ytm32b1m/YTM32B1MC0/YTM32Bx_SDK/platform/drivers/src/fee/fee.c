/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file fee.c
 * @version 1.4.0
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
/* The Offset of flag status part in the cluster header */
#define FEE_CLUSTER_HEADER_STATUS_OFFSET     (FEE_CLUSTER_OVERHEAD - (2U * FEE_VIRTUAL_PAGE_SIZE))
/* The Offset of flag status part in the block header */
#define FEE_BLOCK_HEADER_STATUS_OFFSET      (FEE_BLOCK_OVERHEAD - (2U * FEE_VIRTUAL_PAGE_SIZE))

/*==================================================================================================
 *                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/
/**
* @brief        Status of Fee block header
*/
typedef enum
{
    FEE_BLOCK_VALID = 0,         /**< @brief Fee block is valid */
    FEE_BLOCK_INVALID,           /**< @brief Fee block is invalid (has been invalidated) */
    FEE_BLOCK_INCONSISTENT,      /**< @brief Fee block is inconsistent (contains bogus data) */
    FEE_BLOCK_HEADER_INVALID,    /**< @brief Fee block header is garbled */
    FEE_BLOCK_INVALIDATED,       /**< @brief Fee block header is invalidated by Fee_InvalidateBlock(BlockNumber)(not used when
                                      FEE_BLOCK_ALWAYS_AVAILABLE == STD_OFF) */
    FEE_BLOCK_HEADER_BLANK,      /**< @brief Fee block header is blank, it is used to mark the end of Fee block header list
                                      when parsing the memory at initialization*/
    FEE_BLOCK_INCONSISTENT_COPY, /**< @brief FEE data read error during swap (ie data area was allocated) */
    FEE_BLOCK_NEVER_WRITTEN      /**< @brief FEE block was never written in data flash */
} Fee_BlockStatusType;

/**
* @brief        Status of Fee cluster header
*/
typedef enum
{
    FEE_CLUSTER_VALID = 0,       /**< @brief Fee cluster is valid */
    FEE_CLUSTER_INVALID,         /**< @brief Fee cluster is invalid */
    FEE_CLUSTER_INCONSISTENT,    /**< @brief Fee cluster is inconsistent (contains bogus data) */
    FEE_CLUSTER_HEADER_INVALID   /**< @brief Fee cluster header is garbled */
} Fee_ClusterStatusType;

/**
* @brief        Type of job currently executed by Fee_MainFunction
*/
typedef enum
{
    /* Fee_Read() related jobs */
    FEE_JOB_READ = 0,                /**< @brief Read Fee block */

    /* Fee_Write() related jobs */
    FEE_JOB_WRITE,                   /**< @brief Write Fee block to flash */
    FEE_JOB_WRITE_DATA,              /**< @brief Write Fee block data to flash */
    FEE_JOB_WRITE_UNALIGNED_DATA,    /**< @brief Write unaligned rest of Fee block data to flash */
    FEE_JOB_WRITE_VALIDATE,          /**< @brief Validate Fee block by writing validation flag to flash */
    FEE_JOB_WRITE_DONE,              /**< @brief Finalize validation of Fee block */

    /* Fee_InvalidateBlock() related jobs */
    FEE_JOB_INVAL_BLOCK,             /**< @brief Invalidate Fee block by writing the invalidation flag to flash */
    FEE_JOB_INVAL_BLOCK_DONE,        /**< @brief Finalize invalidation of Fee block */

    /* Fee_EraseImmediateBlock() related jobs */
    FEE_JOB_ERASE_IMMEDIATE,         /**< @brief Erase (pre-allocate) immediate Fee block */

    /* Fee_Init() realted jobs */
    FEE_JOB_INT_SCAN,                /**< @brief Initialize the cluster scan job */
    FEE_JOB_INT_SCAN_CLR,            /**< @brief Scan active cluster of current cluster group */
    FEE_JOB_INT_SCAN_CLR_HDR_PARSE,  /**< @brief Parse Fee cluster header */
    FEE_JOB_INT_SCAN_CLR_FMT,        /**< @brief Format first Fee cluster */
    FEE_JOB_INT_SCAN_CLR_FMT_DONE,   /**< @brief Finalize format of first Fee cluster */
    FEE_JOB_INT_SCAN_BLOCK_HDR_PARSE,/**< @brief Parse Fee block header */

    /* Internal swap jobs */
    FEE_JOB_INT_SWAP_CLR_FMT,        /**< @brief Format current Fee cluster in current Fee cluster group */
    FEE_JOB_INT_SWAP_BLOCK,          /**< @brief Copy next block from source to target cluster */
    FEE_JOB_INT_SWAP_DATA_READ,      /**< @brief Read data from source cluster to internal Fee buffer */
    FEE_JOB_INT_SWAP_DATA_WRITE,     /**< @brief Write data from internal Fee buffer to target cluster */
    FEE_JOB_INT_SWAP_CLR_VLD_DONE,   /**< @brief Finalize cluster validation */

    /* Fee system jobs */
    FEE_JOB_DONE,                    /**< @brief No more subsequent jobs to schedule */

    /* Fee_Setmode() related job */
    FEE_JOB_SETMODE                  /**< @brief Setmode to fls */
} Fee_JobType;

/**
* @brief        Fee cluster group run-time status
*/
typedef struct
{
    Fls_AddressType DataAddrIt;        /**< @brief Address of current Fee data block in flash */
    Fls_AddressType HdrAddrIt;         /**< @brief Address of current Fee block header in flash */
    uint32_t ActClrID;                   /**< @brief ID of active cluster */
    uint8_t ActClr;                      /**< @brief Index of active cluster */
} Fee_ClusterGroupInfoType;

/**
* @brief        Fee block run-time status
*/
typedef struct
{
    Fls_AddressType DataAddr;          /**< @brief Address of Fee block data in flash */
    Fls_AddressType InvalidAddr;       /**< @brief Address of Fee block invalidation field in flash */
    Fee_BlockStatusType BlockStatus;   /**< @brief Current status of Fee block */
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

static bool bSwapToBePerformed;

/**
* @brief        Pointer to user data buffer. Used by the read Fee jobs
*/
static uint8_t *Fee_pJobReadDataDestPtr = (uint8_t *)NULL_PTR;

/**
* @brief        Pointer to user data buffer. Used by the write Fee jobs
*/
static const uint8_t *Fee_pJobWriteDataDestPtr = (uint8_t *)NULL_PTR;

/**
* @brief        Data buffer used by all jobs to store immediate data
*/
static uint8_t Fee_aDataBuffer[FEE_DATA_BUFFER_SIZE] = {0};
/**
* @brief        Internal cluster group iterator. Used by the scan and swap jobs
*               Warning: do not use it outside scan and swap functions
*                        (because it will be Out of Range)
*/
static uint8_t Fee_uJobIntClrGrpIt;

/**
* @brief        Internal cluster iterator. Used by the scan and swap jobs
*/
static uint8_t Fee_uJobIntClrIt;


/**
* @brief        Fee block index. Used by all Fee jobs
*/
static uint16_t Fee_uJobBlockIndex;
/**
* @brief        Internal block iterator. Used by the swap job
*/
static uint16_t Fee_uJobIntBlockIt;

/**
* @brief        Run-time information about blocks touching the Reserved Area
* @implements   Fee_aReservedAreaTouched_Object
*/
static uint32_t Fee_aReservedAreaTouched[(FEE_CRT_CFG_NR_OF_BLOCKS + ((sizeof(uint32_t) * 8U)-1U)) / (sizeof(uint32_t) * 8U)];

/**
* @brief        Currently executed job (including internal one)
*/
static Fee_JobType Fee_eJob = FEE_JOB_DONE;

/**
* @brief        Fee job which started internal management job(s) such as swap...
*/
static Fee_JobType Fee_eJobIntOriginalJob = FEE_JOB_DONE;

/**
* @brief        Internal state of Fee module
*/
static MemIf_StatusType Fee_eModuleStatus = MEMIF_UNINIT;

/**
* @brief        Result of last Fee module job
*/
static MemIf_JobResultType Fee_eJobResult = MEMIF_JOB_OK;
/**
* @brief        Fee block Offset. Used by the read Fee job
*/
static Fls_LengthType Fee_uJobBlockOffset;

/**
* @brief        Number of bytes to read. Used by the read Fee job
*/
static Fls_LengthType Fee_uJobBlockLength;

/**
* @brief        Internal flash helper address iterator. Used by the scan and
*               swap jobs
*/
static Fls_AddressType Fee_uJobIntAddrIt;

/**
* @brief        Internal address of current block header. Used by the swap job
*/
static Fls_AddressType Fee_uJobIntHdrAddr;

/**
* @brief        Internal address of current data block. Used by the swap job.
*/
static Fls_AddressType Fee_uJobIntDataAddr;

/**
* @brief        Run-time information of all configured Fee blocks. Contains
*               status, and data information. Used by all jobs
* @implements   Fee_aBlockInfo_Object
*/
static Fee_BlockInfoType Fee_aBlockInfo[FEE_CRT_CFG_NR_OF_BLOCKS];

/**
* @brief        Run-time information of all configured cluster groups
*/
static Fee_ClusterGroupInfoType Fee_aClrGrpInfo[FEE_NUMBER_OF_CLUSTER_GROUPS];

/*==================================================================================================
 *                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

static void Fee_SerializeBlockHdr(const Fee_BlockType *BlockHder,
                                  const Fls_AddressType TargetAddress,
                                  uint8_t *BlockHdrPtr
                                 );

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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_SerializeUint16
 * Description   : Serialize scalar parameter into the buffer
 *
 *END**************************************************************************/
static inline void Fee_SerializeUint16(uint16_t ParamVal, uint8_t **SerialPtr)
{
    *((uint16_t*)((uint32_t)(*SerialPtr))) = ParamVal;
    (*SerialPtr) = &((*SerialPtr)[sizeof(uint16_t)]);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_SerializeUint32
 * Description   : Serialize scalar parameter into the buffer
 *
 *END**************************************************************************/
static inline void Fee_SerializeUint32(uint32_t ParamVal, uint8_t **SerialPtr)
{
    *((uint32_t*)((uint32_t)(*SerialPtr))) = ParamVal;
    (*SerialPtr) = &((*SerialPtr)[sizeof(uint32_t)]);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_SerializeAddress
 * Description   : Serialize scalar parameter into the buffer
 *
 *END**************************************************************************/
static inline void Fee_SerializeAddress(Fls_AddressType ParamVal, uint8_t **SerialPtr)
{
    *((Fls_AddressType*)((uint32_t)(*SerialPtr))) = ParamVal;
    (*SerialPtr) = &((*SerialPtr)[sizeof(Fls_AddressType)]);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_SerializeLength
 * Description   : Serialize scalar parameter into the buffer
 *
 *END**************************************************************************/
static inline void Fee_SerializeLength(Fls_LengthType ParamVal, uint8_t **SerialPtr)
{
    *((Fls_LengthType*)((uint32_t)(*SerialPtr))) = ParamVal;
    (*SerialPtr) = &((*SerialPtr)[sizeof(Fls_LengthType)]);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_DeserializeUint8
 * Description   : Deserialize scalar parameter from the buffer
 *
 *END**************************************************************************/
static inline void Fee_DeserializeUint8(const uint8_t **DeserialPtr, uint8_t *ParamVal)
{
    *ParamVal = *(const uint8_t*)((uint32_t)(*DeserialPtr));
    (*DeserialPtr) = &((*DeserialPtr)[sizeof(uint8_t)]);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_DeserializeUint16
 * Description   : Deserialize scalar parameter from the buffer
 *
 *END**************************************************************************/
static inline void Fee_DeserializeUint16(const uint8_t **DeserialPtr, uint16_t *ParamVal)
{
    *ParamVal = *(const uint16_t*)((uint32_t)(*DeserialPtr));
    (*DeserialPtr) = &((*DeserialPtr)[sizeof(uint16_t)]);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_DeserializeUint32
 * Description   : Deserialize scalar parameter from the buffer
 *
 *END**************************************************************************/
static inline void Fee_DeserializeUint32(const uint8_t **DeserialPtr, uint32_t *ParamVal)
{
    *ParamVal = *(const uint32_t*)((uint32_t)(*DeserialPtr));
    (*DeserialPtr) = &((*DeserialPtr)[sizeof(uint32_t)]);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_DeserializeAddress
 * Description   : Deserialize scalar parameter from the buffer
 *
 *END**************************************************************************/
static inline void Fee_DeserializeAddress(const uint8_t **DeserialPtr, Fls_AddressType *ParamVal)
{
    *ParamVal = *(const Fls_AddressType*)((uint32_t)(*DeserialPtr));
    (*DeserialPtr) = &((*DeserialPtr)[sizeof(Fls_AddressType)]);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_DeserializeLength
 * Description   : Deserialize scalar parameter from the buffer
 *
 *END**************************************************************************/
static inline void Fee_DeserializeLength(const uint8_t **DeserialPtr, Fls_LengthType *ParamVal)
{
    *ParamVal = *(const Fls_LengthType*)((uint32_t)(*DeserialPtr));
    (*DeserialPtr) = &((*DeserialPtr)[sizeof(Fls_LengthType)]);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobIntalScanCluHdrParse_Wrapper
 * Description   :  A wrapper function for Fee_JobIntalScanCluHdrParse
 * 
 *END**************************************************************************/
static inline MemIf_JobResultType Fee_JobIntalScanCluHdrParse_Wrapper(void)
{
    return Fee_JobIntalScanCluHdrParse(true);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalScanBlkHdrParse_Wrapper
 * Description   :  A wrapper function for Fee_JobInternalScanBlkHdrParse
 * 
 *END**************************************************************************/
static inline MemIf_JobResultType Fee_JobInternalScanBlkHdrParse_Wrapper(void)
{
    return Fee_JobInternalScanBlkHdrParse(true);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalSwapDataRead_Wrapper
 * Description   :  A wrapper function for Fee_JobInternalSwapDataRead
 * 
 *END**************************************************************************/
static inline MemIf_JobResultType Fee_JobInternalSwapDataRead_Wrapper(void)
{
    return Fee_JobInternalSwapDataRead(true);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalSwapDataWrite_Wrapper
 * Description   :  A wrapper function for Fee_JobInternalSwapDataWrite
 * 
 *END**************************************************************************/
static inline MemIf_JobResultType Fee_JobInternalSwapDataWrite_Wrapper(void)
{
    return Fee_JobInternalSwapDataWrite(true);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : MemIf_JobResultType
 * Description   : Fee job done
 * 
 *END**************************************************************************/
static inline MemIf_JobResultType Fee_JobSystemDone(void)
{
    return MEMIF_JOB_FAILED;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobScheduleLookupTable
 * Description   : List of Fee job functions, used by Fee_JobSchedule
 * The order of functions in this table must be identical
 * with the enumeration Fee_JobType from Fee_InternalTypes.h 
 * 
 *END**************************************************************************/
static MemIf_JobResultType (*Fee_JobScheduleLookupTable[])(void) =
{
    /* Fee_Read() related jobs */
    Fee_JobReadBlock,                                /* FEE_JOB_READ */

    /* Fee_Write() related jobs */
    Fee_JobWriteBlock,                               /* FEE_JOB_WRITE */
    Fee_JobWriteBlockData,                           /* FEE_JOB_WRITE_DATA */
    Fee_JobWriteBlockUnalignedData,                  /* FEE_JOB_WRITE_UNALIGNED_DATA */
    Fee_JobWriteBlockValidate,                       /* FEE_JOB_WRITE_VALIDATE */
    Fee_JobWriteBlockDone,                           /* FEE_JOB_WRITE_DONE */

    /* Fee_InvalidateBlock() related jobs */
    Fee_JobInvalidateBlock,                          /* FEE_JOB_INVAL_BLOCK */
    Fee_JobInvalidateBlockDone,                      /* FEE_JOB_INVAL_BLOCK_DONE */

    /* Fee_EraseImmediateBlock() related jobs */
    Fee_JobEraseImmediateBlock,                      /* FEE_JOB_ERASE_IMMEDIATE */

    /* Fee_Init() realted jobs */
    Fee_JobInternalScan,                             /* FEE_JOB_INT_SCAN */
    Fee_JobInternalScanCluster,                      /* FEE_JOB_INT_SCAN_CLR */
    Fee_JobIntalScanCluHdrParse_Wrapper,      /* FEE_JOB_INT_SCAN_CLR_HDR_PARSE */
    Fee_JobInternalScanClusterFmt,                   /* FEE_JOB_INT_SCAN_CLR_FMT */
    Fee_JobInternalScanClusterFmtDone,               /* FEE_JOB_INT_SCAN_CLR_FMT_DONE */
    Fee_JobInternalScanBlkHdrParse_Wrapper,        /* FEE_JOB_INT_SCAN_BLOCK_HDR_PARSE */

    /* Internal swap jobs */
    Fee_JobInternalSwapClusterFmt,                   /* FEE_JOB_INT_SWAP_CLR_FMT */
    Fee_JobInternalSwapBlock,                        /* FEE_JOB_INT_SWAP_BLOCK */
    Fee_JobInternalSwapDataRead_Wrapper,             /* FEE_JOB_INT_SWAP_DATA_READ */
    Fee_JobInternalSwapDataWrite_Wrapper,            /* FEE_JOB_INT_SWAP_DATA_WRITE */
    Fee_JobInternalSwapClusterVldDone,               /* FEE_JOB_INT_SWAP_CLR_VLD_DONE */

    /* Fee system jobs done */
    Fee_JobSystemDone,                               /* FEE_JOB_DONE */
};
/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/
/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_GetBlockClusterGrp
 * Description   : Returns the cluster group for a block specified by its 
 * index in the Fee_aBlockInfo array
 * 
 *END**************************************************************************/
static inline uint8_t Fee_GetBlockClusterGrp(const uint16_t BlockRuntimeInfoIndex)
{
    uint8_t BlockClusterGrp;

    /* the config is part of Fee_BlockConfig*/
    BlockClusterGrp = Fee_ConfigPtr->blockConfigPtr[BlockRuntimeInfoIndex].ClrGrp;

    return BlockClusterGrp;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_ReservedAreaTargetedInClrGrp
 * Description   : Checks whether the area specified by 
 * Fee_aClrGrpInfo[ClrGrpIndex].DataAddrIt and 
 * Fee_aClrGrpInfo[ClrGrpIndex].HdrAddrIt touches the Reserved Area.
 * 
 *END**************************************************************************/
static inline bool Fee_ReservedAreaTargetedInClrGrp(const uint8_t ClrGrpIndex)
{
    bool RetVal;
    Fls_LengthType AvailClrSpace;
    uint32_t ReservedSpace;

    /* Reserved space of cluster group*/
    ReservedSpace = Fee_ConfigPtr->clusterConfigPtr[ ClrGrpIndex ].ReservedSize;

    /* Calculate available space in active cluster */
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_PowerOf2Of5LSB
 * Description   : Function to compute the power of 2 out of the 5 LSB bits 
 * of InVal value.
 * 
 *END**************************************************************************/
static inline uint32_t Fee_PowerOf2Of5LSB(const uint32_t InVal)
{
    return (uint32_t)(0x00000001UL << (InVal & 0x1FUL));
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_ReservedAreaTouchedByBlock
 * Description   :  Returns the information about touching the Reserved Area 
 * by the block specified by uBlockuNumber.
 * 
 *END**************************************************************************/
static inline bool Fee_ReservedAreaTouchedByBlock(const uint16_t BlockNumber)
{
    uint32_t Idx;
    bool RetVal;

    /*The Block information is stored by 1 bit, 32 blocks will be stored in one element of Fee_aReservedAreaTouched*/
    /*Calculate index of the element that store the block information*/
    Idx = ((uint32_t)BlockNumber) >> 5U;
    /*Check if the bit is 1*/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_TouchReservedAreaByBlock
 * Description   :  Stores the information about touching the Reserved Area 
 * for the block specified by BlockNumber.
 * 
 *END**************************************************************************/
static inline void Fee_TouchReservedAreaByBlock(const uint16_t BlockNumber)
{
    uint32_t Idx;

    Idx = ((uint32_t)BlockNumber) >> 5U;

    Fee_aReservedAreaTouched[Idx] |= Fee_PowerOf2Of5LSB((uint32_t)BlockNumber);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_UntouchReservedAreaByClrGrp
 * Description   :  Removes the information about touching the Reserved Area
 *  for all blocks within a cluster group specified by ClrGrpIndex.
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_SerializeBlockHdr
 * Description   :   Serialize Fee block parameters into a write buffer.
 * 
 *END**************************************************************************/
static void Fee_SerializeBlockHdr(const Fee_BlockType *BlockHder,
                                  const Fls_AddressType TargetAddress,
                                  uint8_t *BlockHdrPtr
                                 )
{
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
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_BlankCheck
 * Description   :   Check whether specified data buffer contains only
 * the FEE_ERASED_VALUE value
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_DeserializeFlag
 * Description   :   Deserialize the valid or invalid flag from a read buffer. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_DeserializeBlockHdr
 * Description   :   Deserialize Fee block header parameters from read buffer. 
 * 
 *END**************************************************************************/
static Fee_BlockStatusType Fee_DeserializeBlockHdr(Fee_BlockType *const BlockHder,
                                                   Fls_AddressType *const TargetAddress,
                                                   const uint8_t *BlockHdrPtr
                                                  )
{
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
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_DeserializeClusterHdr
 * Description   :   Deserialize Fee cluster header parameters from read buffer. 
 * 
 *END**************************************************************************/
static Fee_ClusterStatusType Fee_DeserializeClusterHdr(Fee_ClusterHeaderType *ClrHdr,
                                                       const uint8_t *ClrHdrPtr
                                                      )
{
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
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_SerializeClusterHdr
 * Description   :   Serialize Fee cluster header parameters to write buffer. 
 * 
 *END**************************************************************************/
static void Fee_SerializeClusterHdr(const Fee_ClusterHeaderType *ClrHdr,
                                    uint8_t *ClrHdrPtr
                                   )
{
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
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_GetBlockIndex
 * Description   :  Searches ordered list of Fee blocks and returns index of 
 * block with matching BlockNumber. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_GetBlockSize
 * Description   :  Returns the block size for a block specified by its index
 *  in the Fee_aBlockInfo array. 
 * 
 *END**************************************************************************/
static inline uint16_t Fee_GetBlockSize(const uint16_t BlockRuntimeInfoIndex)
{
    uint16_t BlockSize;

    /* the config is part of Fee_BlockConfig*/
    BlockSize = Fee_ConfigPtr->blockConfigPtr[BlockRuntimeInfoIndex].BlockSize;
    return BlockSize;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_GetBlockNumber
 * Description   :  Returns the block number for a block specified by its 
 * index in the Fee_aBlockInfo array. 
 * 
 *END**************************************************************************/
static inline uint16_t Fee_GetBlockNumber(const uint16_t BlockRuntimeInfoIndex)
{
    uint16_t BlockNumber = 0U;
    /* the config is part of Fee_BlockConfig*/
    BlockNumber = Fee_ConfigPtr->blockConfigPtr[BlockRuntimeInfoIndex].BlockNumber;
    return BlockNumber;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_GetBlockImmediate
 * Description   :  Returns the immediate attribute for a block specified by 
 * its index in the Fee_aBlockInfo array. 
 * 
 *END**************************************************************************/
static inline bool Fee_GetBlockImmediate(const uint16_t BlockRuntimeInfoIndex)
{
    bool Immediate;
    /* the config is part of Fee_BlockConfig*/
    Immediate = Fee_ConfigPtr->blockConfigPtr[BlockRuntimeInfoIndex].ImmediateData;
    return Immediate;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_AlignToVirtualPageSize
 * Description   :  Adjusts passed size so it's integer multiple of 
 * pre-configured + FEE_VIRTUAL_PAGE_SIZE. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_CopyDataToPageBuffer
 * Description   :  Copy data from user to internal write buffer and fills
 * rest of the write buffer with FEE_ERASED_VALUE. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_SerializeFlag
 * Description   :  Serialize validation or invalidation flag to write buffer. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalSwapClusterVld
 * Description   :  Validate current Fee cluster in current Fee cluster group 
 * by writing FEE_VALIDATED_VALUE into flash. 
 * 
 *END**************************************************************************/
static MemIf_JobResultType Fee_JobInternalSwapClusterVld(void)
{
    MemIf_JobResultType RetVal;
    Fls_AddressType StartAddr;

    /* Get start address of swap cluster */
    StartAddr = Fee_ConfigPtr->clusterConfigPtr[Fee_uJobIntClrGrpIt].ClrPtr[Fee_uJobIntClrIt].StartAddr;

    /* Serialize validation pattern to write buffer */
    Fee_SerializeFlag(Fee_aDataBuffer, FEE_VALIDATED_VALUE);

    /* Write validation pattern to flash */
    if (((Std_ReturnType)E_OK) == Fee_WriteToFls((StartAddr + FEE_CLUSTER_OVERHEAD) - (2U * FEE_VIRTUAL_PAGE_SIZE),
                                                  Fee_aDataBuffer, FEE_VIRTUAL_PAGE_SIZE
                                                 )
       )
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalSwapBlock
 * Description   :  Copy next block from source to target cluster. 
 * 
 *END**************************************************************************/
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
        if (((Std_ReturnType)E_OK) != Fee_WriteToFls(Fee_uJobIntHdrAddr, Fee_aDataBuffer, FEE_BLOCK_OVERHEAD - (2U * FEE_VIRTUAL_PAGE_SIZE)))
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalSwapClusterFmt
 * Description   :  Format current Fee cluster in current Fee cluster group by
 *  writing cluster header into flash. 
 * 
 *END**************************************************************************/
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
    if (((Std_ReturnType)E_OK) == Fee_WriteToFls(ClrHdr.StartAddr, Fee_aDataBuffer, FEE_CLUSTER_OVERHEAD - (2U * FEE_VIRTUAL_PAGE_SIZE)))
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalSwapClusterErase
 * Description   :  Erase current Fee cluster in current Fee cluster group 
 * by erasing flash. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_GetNextClusterToSwap
 * Description   :  Calculate the index of the next cluster in current 
 * cluster group. In sector retirement mode, find the next good cluster
 *  with Length greater than zero.
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalSwap
 * Description   :  Initialize the cluster swap internal operation on
 * current cluster group.
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalScanBlockHdrRead
 * Description   :  Read the Fee block header into internal buffer.
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalScanClusterErase
 * Description   :  Erase first Fee cluster in current cluster group.
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalScanCluster
 * Description   :  Scan active cluster of current cluster group or erase 
 * and format first cluster if an active cluster can't be found
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalScanClusterFmt
 * Description   : Format first Fee cluster in current Fee cluster group by 
 * writing cluster header into flash .
 * 
 *END**************************************************************************/
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
    if (((Std_ReturnType)E_OK) == Fee_WriteToFls(ClrHdr.StartAddr, Fee_aDataBuffer, FEE_CLUSTER_OVERHEAD - FEE_VIRTUAL_PAGE_SIZE))
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalScanClusterFmtDone
 * Description   : Finalize format of first Fee cluster in current Fee 
 * cluster group.
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_IsBlockMatchedConfig
 * Description   : Check the block's size and type match the configuration,
 *  and its data pointer points to an acceptable area 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_UpdateBlockRuntimeInfo
 * Description   : Update block run-time information. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalScanBlkHdrParse
 * Description   : Parse Fee block header. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalScanClusterHdrRead
 * Description   : Read Fee cluster header. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalScan
 * Description   : Initialize the cluster scan job. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalScanClusterHdrDone
 * Description   : Check if all cluster have been scanned or not. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobIntalScanCluHdrParse
 * Description   : Parse Fee cluster header. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobReadBlock
 * Description   : Read Fee block. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalSwapBlockVld
 * Description   : Validate Fee block. 
 * 
 *END**************************************************************************/
static MemIf_JobResultType Fee_JobInternalSwapBlockVld(void)
{
    MemIf_JobResultType RetVal = MEMIF_JOB_OK;

    /* Serialize validation pattern to buffer */
    Fee_SerializeFlag(Fee_aDataBuffer, FEE_VALIDATED_VALUE);

    /* Write header to flash */
    if (((Std_ReturnType)E_OK) != Fee_WriteToFls(Fee_uJobIntHdrAddr - (2U*FEE_VIRTUAL_PAGE_SIZE), Fee_aDataBuffer, FEE_VIRTUAL_PAGE_SIZE))
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalSwapDataRead
 * Description   : Read data from source cluster to internal Fee buffer. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalSwapDataWrite
 * Description   : Write data from internal Fee buffer to target cluster. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInternalSwapClusterVldDone
 * Description   : Finalize cluster validation. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobWriteHdr
 * Description   : Write Fee block header to flash. 
 * 
 *END**************************************************************************/
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
            if (((Std_ReturnType)E_OK) != Fee_WriteToFls(HdrAddr, Fee_aDataBuffer, FEE_BLOCK_OVERHEAD - (2U * FEE_VIRTUAL_PAGE_SIZE)))
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobWriteBlockData
 * Description   : Write Fee block data to flash. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobWriteBlock
 * Description   : Write Fee block to flash. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobWriteBlockUnalignedData
 * Description   : Write unaligned rest of Fee block data to flash. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobWriteBlockValidate
 * Description   : Validate Fee block by writing validation flag to flash. 
 * 
 *END**************************************************************************/
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
        HdrAddr = Fee_aClrGrpInfo[ClrGrpIndex].HdrAddrIt - (2U *  FEE_VIRTUAL_PAGE_SIZE);

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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobWriteBlockDone
 * Description   : Finalize validation of Fee block. 
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInvalidateBlock
 * Description   : Invalidate Fee block by writing the invalidation flag 
 * to flash.
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobInvalidateBlockDone
 * Description   : Finalize invalidation of Fee block.
 * 
 *END**************************************************************************/
static MemIf_JobResultType Fee_JobInvalidateBlockDone(void)
{
    /* Mark the Fee block as in valid */
    Fee_aBlockInfo[Fee_uJobBlockIndex].BlockStatus = FEE_BLOCK_INVALID;

    /* No more subsequent Fls jobs to schedule */
    Fee_eJob = FEE_JOB_DONE;

    /* Fee job completed */
    return(MEMIF_JOB_OK);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobEraseImmediateBlock
 * Description   : Erase (pre-allocate) immediate Fee block.
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobSchedule
 * Description   : Schedule subsequent jobs
 * 
 *END**************************************************************************/
static MemIf_JobResultType Fee_JobSchedule(void)
{
    /* Jump to the current Fee job through function pointer */
    return Fee_JobScheduleLookupTable[Fee_eJob]();
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobScheduleAfterSwapClusterVldDone
 * Description   : Schedule subsequent jobs
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_ReservedAreaWritable
 * Description   :   Checks whether the block specified by Fee_JobBlockIndex
 *  is writable into the reserved area.
 * 
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobErrorSchedule
 * Description   :   Schedule the error jobs of Fee 
 *
 *END**************************************************************************/
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

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_ReadFromFls
 * Description   :   Read data from Fls, translate the emulation to logical
 *  address in sector retirement mode 
 *
 *END**************************************************************************/
static Std_ReturnType Fee_ReadFromFls(Fls_AddressType SourceAddress,
                                      uint8_t *TargetAddressPtr,
                                      Fls_LengthType Length
                                     )
{
    return Fls_Read(SourceAddress, TargetAddressPtr, Length);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_WriteToFls
 * Description   :   Write data to Fls, translate the emulation to logical 
 * address in sector retirement mode 
 *
 *END**************************************************************************/
static Std_ReturnType Fee_WriteToFls(Fls_AddressType TargetAddress,
                                     const uint8_t *SourceAddressPtr,
                                     Fls_LengthType Length
                                    )
{
    return Fls_Write(TargetAddress, SourceAddressPtr, Length);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_EraseCluster
 * Description   :   Erase the whole cluster, translate the emulation to 
 * logical address in sector retirement mode and erase a sector at a time. 
 *
 *END**************************************************************************/
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

/**
* @brief        Get the Length of the cluster by the group index and cluster index
*
*/
static inline Fls_LengthType Fee_GetClusterLength(uint8_t ClrGrpIt,
                                                  uint8_t ClrIt
                                                 )
{
    return Fee_ConfigPtr->clusterConfigPtr[ClrGrpIt].ClrPtr[ClrIt].Length;          /* Get from constant data configuration */

}



/*==================================================================================================
 *                                       GLOBAL FUNCTIONS
==================================================================================================*/
/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_EraseImmediateBlock
 * Description   :  Service to erase a logical block.
 *
 *END**************************************************************************/
Std_ReturnType Fee_EraseImmediateBlock (uint16_t BlockNumber)
{
    Std_ReturnType RetVal;
    uint16_t BlockIndex = Fee_GetBlockIndex(BlockNumber);
   if((0xFFFFU == BlockIndex) || (MEMIF_IDLE != Fee_eModuleStatus))
    {
        RetVal = (Std_ReturnType)E_NOT_OK;
    }
    else
    {
        /* Configure the erase immediate block job */
        Fee_uJobBlockIndex = BlockIndex;
        Fee_eJob = FEE_JOB_ERASE_IMMEDIATE;
        Fee_eModuleStatus = MEMIF_BUSY;

        /* Execute the erase immediate block job */
        Fee_eJobResult = MEMIF_JOB_PENDING;

        RetVal = (Std_ReturnType)E_OK;
    }
    return RetVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_GetJobResult
 * Description   :  Service to query the result of the last accepted job
 *  issued by the upper layer software.
 *
 *END**************************************************************************/
MemIf_JobResultType Fee_GetJobResult (void)
{
    MemIf_JobResultType RetVal = Fee_eJobResult;
    return RetVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_GetStatus
 * Description   :   Service to return the status.
 *
 *END**************************************************************************/
MemIf_StatusType Fee_GetStatus (void)
{
    return(Fee_eModuleStatus);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_Init
 * Description   :  Service to initialize the FEE module.
 *
 *END**************************************************************************/
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
		/*Initialize fee flash*/
		Fls_Init(Fee_ConfigPtr->flashConfigPtr);
		
        /* Initialize all block info records */
        for (InvalIndex = 0U; InvalIndex < Fee_ConfigPtr->blockCnt; InvalIndex++)
        {

            /* for blocks which were never written Fee returns INCONSISTENT status */
            Fee_aBlockInfo[InvalIndex].BlockStatus = FEE_BLOCK_NEVER_WRITTEN;
            Fee_aBlockInfo[InvalIndex].DataAddr = 0U;
            Fee_aBlockInfo[InvalIndex].InvalidAddr = 0U;
        }
        /* Invalidate all cluster groups */
        for (InvalIndex = 0U; InvalIndex < Fee_ConfigPtr->clusterCnt; InvalIndex++)
        {
            Fee_aClrGrpInfo[InvalIndex].ActClrID = 0U;
        }
        /* Schedule init job */
        Fee_eJob = FEE_JOB_INT_SCAN;
        Fee_eJobResult = MEMIF_JOB_PENDING;
	}
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_InvalidateBlock
 * Description   :  Service to invalidate a logical block.
 *
 *END**************************************************************************/
Std_ReturnType Fee_InvalidateBlock (uint16_t BlockNumber)
{
    Std_ReturnType RetVal = (Std_ReturnType)E_OK;
    uint16_t BlockIndex = Fee_GetBlockIndex(BlockNumber);

   if((0xFFFFU == BlockIndex) || (MEMIF_IDLE != Fee_eModuleStatus))
    {
        RetVal = (Std_ReturnType)E_NOT_OK;
    }
    else
    {
        /* Configure the invalidate block job */
        Fee_uJobBlockIndex = BlockIndex;

        Fee_eJob = FEE_JOB_INVAL_BLOCK;

        Fee_eModuleStatus = MEMIF_BUSY;

        /* Execute the invalidate block job */
        Fee_eJobResult = MEMIF_JOB_PENDING;

        RetVal = (Std_ReturnType)E_OK;
    }

    return RetVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobEndNotification
 * Description   :   Service to report to this module the successful end 
 * of an asynchronous operation.
 *
 *END**************************************************************************/
void Fee_JobEndNotification (void)
{
    if ((MEMIF_UNINIT == Fee_eModuleStatus) && (MEMIF_JOB_OK == Fee_eJobResult) && (FEE_JOB_DONE == Fee_eJob))
    { }
    else
    {
        if (FEE_JOB_DONE == Fee_eJob)
        {
            /* Last schedule Fls job finished */
            Fee_eJobResult = Fls_GetJobResult();
            Fee_eModuleStatus = MEMIF_IDLE;

            /* Call job end notification function */
        }
        else
        {
            Fee_eJobResult = Fee_JobSchedule();
            if (MEMIF_JOB_OK == Fee_eJobResult)
            {
                Fee_eModuleStatus = MEMIF_IDLE;

                /* Call job end notification function */
             }
             else if (MEMIF_JOB_PENDING == Fee_eJobResult)
             {
                /* Nothing to do (ongoing Fls job) */
             }
             else
             {
                 Fee_eModuleStatus = MEMIF_IDLE;

                 /* Call job error notification function */
              }
          }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_JobErrorNotification
 * Description   :   Service to report to this module the failure of an 
 * asynchronous operation.
 *
 *END**************************************************************************/
void Fee_JobErrorNotification (void)
{
    if ((MEMIF_UNINIT == Fee_eModuleStatus) && (MEMIF_JOB_OK == Fee_eJobResult) && (FEE_JOB_DONE == Fee_eJob))
    {}
    else
    {
        if (MEMIF_JOB_CANCELED == Fee_eJobResult)
        {
            /* Fls job has been canceled. Do nothing in this callback.
            The NvM_JobErrorNotification() callback will be called from the Fee_Cancel()function which called the Fls_Cancel() function */
        }
        else
        {
              /* Schedule the error job */
              Fee_JobErrorSchedule();
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_MainFunction
 * Description   :   Service to handle the requested read / write / erase 
 * jobs and the internal management operations.
 *
 *END**************************************************************************/
void Fee_MainFunction (void)
{
	Fls_MainFunction();
    if (MEMIF_JOB_PENDING == Fee_eJobResult)
    {
        switch (Fee_eJob)
        {
            /* for all the following jobs subsequent jobs will be called in Fee job schedule function based on Job */
            case FEE_JOB_INT_SCAN:
            case FEE_JOB_READ:
            case FEE_JOB_WRITE:
            case FEE_JOB_INVAL_BLOCK:
            case FEE_JOB_ERASE_IMMEDIATE:
                Fee_eJobResult = Fee_JobSchedule();
                break;
            /* for all the following jobs job end or job error notification will be called based on the job result */
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
                /* Internal or subsequent job */
                break;
        }

        if (MEMIF_JOB_PENDING == Fee_eJobResult)
        {
            /* Nothing to do */
        }
        else if (MEMIF_JOB_OK == Fee_eJobResult)
        {
            Fee_eModuleStatus = MEMIF_IDLE;
            /* Call job end notification function */
        }
        else
        {
            Fee_eModuleStatus = MEMIF_IDLE;
            /* Call job error notification function */
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_Read
 * Description   :   Service to initiate a read job. 
 *
 *END**************************************************************************/
Std_ReturnType Fee_Read (uint16_t BlockNumber, uint16_t BlockOffset, uint8_t * DataBufferPtr, uint16_t Length)
{
    Std_ReturnType RetVal = (Std_ReturnType)E_NOT_OK;
    uint16_t BlockIndex = Fee_GetBlockIndex(BlockNumber);

    if((0xFFFFU == BlockIndex) || (MEMIF_IDLE != Fee_eModuleStatus))
    {
        RetVal = (Std_ReturnType)E_NOT_OK;
    }
    else
    {
        /* Configure the read job */
        Fee_uJobBlockIndex = BlockIndex;

        Fee_uJobBlockOffset = BlockOffset;

        Fee_uJobBlockLength = Length;

        Fee_pJobReadDataDestPtr = DataBufferPtr;

        Fee_eJob = FEE_JOB_READ;

        Fee_eModuleStatus = MEMIF_BUSY;

        /* Execute the read job */
        Fee_eJobResult = MEMIF_JOB_PENDING;

        RetVal = (Std_ReturnType)E_OK;
    }
    return RetVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fee_Write
 * Description   :   Service to initiate a write job. 
 *
 *END**************************************************************************/
Std_ReturnType Fee_Write (uint16_t BlockNumber, const uint8_t * DataBufferPtr)
{
    Std_ReturnType RetVal = (Std_ReturnType)E_OK;
    uint16_t BlockIndex = Fee_GetBlockIndex(BlockNumber);

   if((0xFFFFU == BlockIndex) || (MEMIF_IDLE != Fee_eModuleStatus))
    {
        RetVal = (Std_ReturnType)E_NOT_OK;
    }
    else
    {
        /* Configure the write job */
        Fee_uJobBlockIndex = BlockIndex;

        Fee_pJobWriteDataDestPtr = DataBufferPtr;

        Fee_eJob = FEE_JOB_WRITE;

        Fee_eModuleStatus = MEMIF_BUSY;

        /* Execute the write job */
        Fee_eJobResult = MEMIF_JOB_PENDING;
    }
    return RetVal;
}

Std_ReturnType Fee_Cancel(void)
{
    Std_ReturnType RetVal = (Std_ReturnType)E_OK;
    if (MEMIF_UNINIT == Fee_eModuleStatus)
    {
        RetVal = (Std_ReturnType)E_NOT_OK;
    }
    else
    {
        /* Cancel ongoing Fls job if any */
        if (MEMIF_BUSY == Fee_eModuleStatus)
        {
            Fee_eJobResult = MEMIF_JOB_CANCELED;
            Fee_eJob = FEE_JOB_DONE;
            Fls_Cancel();
            Fee_eModuleStatus = MEMIF_IDLE;
            /* No notifications from internal jobs */
        }
        else
        {
            Fee_JobErrorNotification();
        }
    }
    return RetVal;
}

#ifdef __cplusplus
}
#endif

/** @} */
