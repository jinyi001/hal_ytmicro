/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file fls_async_driver.c
 * @version 1.4.0
 */

/*==================================================================================================
 *                                        INCLUDE FILES
==================================================================================================*/
#include "fls_async_driver.h"
#include "fls_hw_access.h"
/*==================================================================================================
 *                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/

/*==================================================================================================
 *                                       LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
 *                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
 *                                      GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
 *                                      GLOBAL VARIABLES
==================================================================================================*/

/**
 * @brief          Fls global configuration pointer
 */
const Fls_ConfigType * Fls_gConfigPtr = NULL_PTR;
/**
 * @brief          Fls global job result status variable
 */
Fls_JobStateType Fls_gJobState = 
{
    .Fls_JobResult = MEMIF_JOB_OK,
    .Fls_JobCurrentAddr = 0U,
    .Fls_JobEndAddr = 0U,
    .Fls_JobType = FLS_JOB_IDLE,
    .Fls_JobWriteDataPtr = NULL_PTR,
    .Fls_JobReadDataPtr = NULL_PTR,
    .Fls_JobStart = 0U,
    .Fls_JobSectorIt = 0U,
    .Fls_JobSectorEnd = 0U
};

/**
    @brief Result of last flash hardware job
 */
volatile MemIf_JobResultType Fls_eLLDJobResult;          /* implicit zero initialization: MEMIF_JOB_OK */
/**
    @brief Type of current flash hardware job - used for asynchronous operating mode.
 */
Fls_LLDJobType Fls_eLLDJob;    /* implicit zero initialization: FLASH_JOB_NONE */

/*==================================================================================================
 *                                      LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
 *                                      LOCAL VARIABLES
==================================================================================================*/
/**
 * @brief Maximum number of bytes to read or compare in one cycle of Fls_MainFunction
 */
static Fls_LengthType Fls_u32MaxRead;
/**
 * @brief Maximum number of bytes to write in one cycle of Fls_MainFunction
 */
static Fls_LengthType Fls_u32MaxWrite;

/*==================================================================================================
 *                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

LOCAL_INLINE uint16_t Fls_GetSectorIndexByAddr(Fls_AddressType LogicAddress);
static void Fls_MainFunction_CheckJobResult(MemIf_JobResultType WorkResult);
MemIf_JobResultType Fls_ExecuteJobErase(void);
Fls_LLDReturnType Fls_ExecuteJobWrite(const Fls_AddressType SectorOffset,const Fls_AddressType Length);
Fls_LLDReturnType Fls_ExecuteJobReadBlankCheckCompare(const Fls_AddressType u32SectorOffset,const Fls_AddressType u32Length,uint8_t * pJobDataDestPtr,const uint8_t *pJobDataSrcPtr);
static MemIf_JobResultType Fls_ProcessRequestedJobs( void );
static Fls_LLDReturnType Fls_drvTranslateReturnCode(Fls_Drv_StatusType eReturnCode);
static MemIf_JobResultType Fls_LLDGetJobResult(void);
static void Fls_LLDClrJobResult(void);
static MemIf_JobResultType Fls_DoJobDataTransfer(const Fls_LengthType MaxTransfer);
static Fls_AddressType Fls_CalcMaxTransferAddrEnd(const Fls_LengthType MaxTransfer);
static Fls_AddressType Fls_CalcSectorTransferLength(const Fls_AddressType MaxTransferEndAddr);
static Fls_LLDReturnType Fls_ProcessJobDataTransfer(const Fls_AddressType SectorOffset, const Fls_AddressType Length );
static bool Fls_IsSectorStartAddrSectorAligned(Fls_AddressType TargetAddress);
static bool Fls_IsSectorAddrPageAligned(Fls_AddressType TargetAddress);
static bool Fls_IsAddrInRange(Fls_AddressType TargetAddress);
static bool Fls_IsSectorEndAddrSectorAligned(Fls_AddressType TargetAddress);
/*==================================================================================================
 *                                       GLOBAL FUNCTIONS
==================================================================================================*/
#ifdef __cplusplus
extern "C"{
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_Init
 * Description   : Initializes the Flash Driver.
 * 
 *END**************************************************************************/
void Fls_Init (const Fls_ConfigType * ConfigPtr)
{

	DEV_ASSERT(ConfigPtr!=NULL_PTR);
	DEV_ASSERT(Fls_gJobState.Fls_JobResult!=MEMIF_JOB_PENDING);
	
	if (NULL_PTR != ConfigPtr)
    {
 		Fls_gConfigPtr = ConfigPtr;
    }
    else
    {}
    
    if (MEMIF_JOB_PENDING == Fls_gJobState.Fls_JobResult)
    {
        Fls_gConfigPtr = NULL_PTR;
    }
    else
    {
    	     /* Set the max number of bytes to read/write
                during Fls_MainFunction call */
            if (MEMIF_MODE_SLOW == Fls_gConfigPtr->eDefaultMode)
            {
                Fls_u32MaxRead = Fls_gConfigPtr->maxReadNormalMode;
                Fls_u32MaxWrite = Fls_gConfigPtr->maxWriteNormalMode;
            }
            else
            {
                Fls_u32MaxRead = Fls_gConfigPtr->maxReadFastMode;
                Fls_u32MaxWrite = Fls_gConfigPtr->maxWriteFastMode;
            }
        /* [SWS_Fls_00014] The function Fls_Init shall initialize the FLS module
            (software) and all flash memory relevant registers (hardware) with parameters
            provided in the given configuration set. */
        Fls_drvInit();
        if (MEMIF_JOB_OK != Fls_gJobState.Fls_JobResult)
        {
            /* Fls module not initialized */
            Fls_gConfigPtr = NULL_PTR;
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_Erase
 * Description   : Erases flash sector(s).
 * 
 *END**************************************************************************/
Std_ReturnType Fls_Erase (Fls_AddressType TargetAddress, Fls_LengthType Length)
{
    Std_ReturnType retVal = E_NOT_OK;
	
    DEV_ASSERT(Fls_gConfigPtr!=NULL_PTR);
    DEV_ASSERT(Fls_IsSectorStartAddrSectorAligned(TargetAddress)==true);
    DEV_ASSERT(Fls_IsSectorEndAddrSectorAligned(TargetAddress+Length)==true);	
    if((MEMIF_JOB_PENDING == Fls_gJobState.Fls_JobResult)||\
        (Fls_gConfigPtr==NULL_PTR)||\
        (Fls_IsSectorStartAddrSectorAligned(TargetAddress)==false)||\
        (Fls_IsSectorEndAddrSectorAligned(TargetAddress+Length)==false))  
    {
        /* the function Fls_Erase will check that the FLS module is currently not busy. 
            If this check fails, the function Fls_Erase shall reject the erase request, 
            raise the development error FLS_E_BUSY and return with E_NOT_OK. */
        retVal = E_NOT_OK;
    }
    else
    {
        /* the function Fls_Erase will copy the given parameters to Fls 
            module internal variables and initiate a Erase job*/
        Fls_gJobState.Fls_JobCurrentAddr = TargetAddress;
        Fls_gJobState.Fls_JobEndAddr = TargetAddress + Length - 1U;
        Fls_gJobState.Fls_JobSectorIt = Fls_GetSectorIndexByAddr(TargetAddress);
        Fls_gJobState.Fls_JobSectorEnd = Fls_GetSectorIndexByAddr(Fls_gJobState.Fls_JobEndAddr);
        Fls_gJobState.Fls_JobType = FLS_JOB_ERASE;
        /* After initiating the erase job, the function Fls_Erase shall set 
            the job result to MEMIF_JOB_PENDING*/
        Fls_gJobState.Fls_JobResult = MEMIF_JOB_PENDING;
        /*write flag to start request*/
        Fls_gJobState.Fls_JobStart = 1;
        /* Write command accepted */
        retVal = E_OK;
    }
    return retVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_Write
 * Description   : Writes one or more complete flash pages.
 * 
 *END**************************************************************************/
Std_ReturnType Fls_Write (Fls_AddressType TargetAddress, const uint8_t * SourceAddressPtr, Fls_LengthType Length)
{
    Std_ReturnType retVal = E_NOT_OK;
	
	DEV_ASSERT(Fls_gConfigPtr!=NULL_PTR);
    DEV_ASSERT(SourceAddressPtr!=NULL_PTR);
    DEV_ASSERT(Fls_IsSectorAddrPageAligned(TargetAddress)==true);
    DEV_ASSERT(Length != 0);
    DEV_ASSERT(Fls_IsSectorAddrPageAligned(TargetAddress+Length)==true);
	
    if ((MEMIF_JOB_PENDING == Fls_gJobState.Fls_JobResult)||\
        (Fls_gConfigPtr==NULL_PTR)||\
        (SourceAddressPtr==NULL_PTR)||\
        (Fls_IsSectorAddrPageAligned(TargetAddress)==false)||\
        (Length == 0U)||\
        (Fls_IsSectorAddrPageAligned(TargetAddress+Length)==false)) 
    {
        /* The function Fls_Write shall check that the FLS module is currently not busy. 
            If this check fails, the function Fls_Write shall reject the write request, 
            raise the development error FLS_E_BUSY and return with E_NOT_OK. */
        retVal = E_NOT_OK;
    }
    else
    {
        Fls_gJobState.Fls_JobCurrentAddr = TargetAddress;
        Fls_gJobState.Fls_JobEndAddr = TargetAddress + Length - 1U;
        Fls_gJobState.Fls_JobSectorIt = Fls_GetSectorIndexByAddr(TargetAddress);
        Fls_gJobState.Fls_JobSectorEnd = Fls_GetSectorIndexByAddr(Fls_gJobState.Fls_JobEndAddr);
        Fls_gJobState.Fls_JobType = FLS_JOB_WRITE;
        Fls_gJobState.Fls_JobWriteDataPtr = SourceAddressPtr;
        Fls_gJobState.Fls_JobResult = MEMIF_JOB_PENDING;
        Fls_gJobState.Fls_JobStart = 1;
        /* Write command accepted */
        retVal = E_OK;
    }
    return retVal;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_GetStatus
 * Description   : Returns the driver state.
 * 
 *END**************************************************************************/
MemIf_StatusType Fls_GetStatus (void)
{
    MemIf_StatusType retVal = MEMIF_UNINIT;
    /* check if flash module is initialized */
    if (NULL_PTR == Fls_gConfigPtr)
    {
        retVal = MEMIF_UNINIT;
    }
    else if (MEMIF_JOB_PENDING == Fls_gJobState.Fls_JobResult)
    {
        retVal = MEMIF_BUSY;
    }
    else
    {
        retVal = MEMIF_IDLE;
    }
    return retVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_GetJobResult
 * Description   : Returns the result of the last job.
 * 
 *END**************************************************************************/
MemIf_JobResultType Fls_GetJobResult (void)
{
    MemIf_JobResultType retVal = MEMIF_JOB_OK;
	
	DEV_ASSERT(Fls_gConfigPtr!=NULL_PTR);

    if (NULL_PTR == Fls_gConfigPtr)
    {
        /* The function Fls_GetJobResult shall check that the FLS module has been initialized. 
            If this check fails, the function Fls_GetJobResult shall raise the development error
            FLS_E_UNINIT and return with MEMIF_JOB_FAILED*/
        retVal = MEMIF_JOB_FAILED;
    }
    else
    {
        retVal = Fls_gJobState.Fls_JobResult;
    }
    return retVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_Read
 * Description   : Reads from flash memory.
 * 
 *END**************************************************************************/
Std_ReturnType Fls_Read (Fls_AddressType SourceAddress, uint8_t * TargetAddressPtr, Fls_LengthType Length)
{
    Std_ReturnType retVal = E_NOT_OK;

	DEV_ASSERT(Fls_gConfigPtr!=NULL_PTR);
    DEV_ASSERT(TargetAddressPtr!=NULL_PTR);
    DEV_ASSERT(Length != 0);
    DEV_ASSERT(Fls_IsAddrInRange(SourceAddress)==true);
    DEV_ASSERT(Fls_IsAddrInRange(SourceAddress+Length)==true);

    if ((MEMIF_JOB_PENDING == Fls_gJobState.Fls_JobResult)||\
        (Fls_gConfigPtr==NULL_PTR)||\
        (TargetAddressPtr==NULL_PTR)||\
        (Length == 0U)||\
        (Fls_IsAddrInRange(SourceAddress)==false)||\
        (Fls_IsAddrInRange(SourceAddress+Length)==false))
    {
        /* The function Fls_Read shall check that the driver is currently not busy. If this 
            check fails, the function Fls_Read shall reject the read request, raise the 
            development error FLS_E_BUSY and return with E_NOT_OK.*/
        retVal = E_NOT_OK;
    }
    else
    {
        /* The function Fls_Read shall copy the given parameters to FLS
            module internal variables and initiate a read job.*/
        Fls_gJobState.Fls_JobCurrentAddr = SourceAddress;
        Fls_gJobState.Fls_JobEndAddr = SourceAddress + Length - 1U;
        Fls_gJobState.Fls_JobSectorIt = Fls_GetSectorIndexByAddr(SourceAddress);
        Fls_gJobState.Fls_JobSectorEnd = Fls_GetSectorIndexByAddr(Fls_gJobState.Fls_JobEndAddr);
        Fls_gJobState.Fls_JobType = FLS_JOB_READ;
        Fls_gJobState.Fls_JobReadDataPtr = TargetAddressPtr;
        /* After initiating a read job, the function Fls_Read shall set the
            FLS module status to MEMIF_BUSY*/
        /* After initiating a read job, the function Fls_Read shall set the
            FLS module job result to MEMIF_JOB_PENDING. */
        Fls_gJobState.Fls_JobResult = MEMIF_JOB_PENDING;
        Fls_gJobState.Fls_JobStart = 1;
        /* Write command accepted */
        retVal = E_OK;
    }
    
    return retVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_MainFunction
 * Description   : Performs the processing of jobs.
 * 
 *END**************************************************************************/
void Fls_MainFunction (void)
{
	MemIf_JobResultType WorkResult = MEMIF_JOB_OK;
    DEV_ASSERT(Fls_gConfigPtr!=NULL_PTR);
    if ((NULL_PTR != Fls_gConfigPtr) /* FLS is initialized */
        && (MEMIF_JOB_PENDING == Fls_gJobState.Fls_JobResult) /* FLS job is pending */
        )
    {
        
        if(1u==Fls_gJobState.Fls_JobStart)
        {
            /* Clear result of hardware job */
            Fls_LLDClrJobResult();
            Fls_gJobState.Fls_JobStart = 0;
        }
        /* Check the status of pending jobs. */		 
        if(FLASH_JOB_NONE!=Fls_eLLDJob)
        {
            Fls_drvMainFunction();
        }
        WorkResult = Fls_LLDGetJobResult();
        if (MEMIF_JOB_OK == WorkResult)
        {
            /* Flash is initialized and a job is pending */
	        WorkResult = Fls_ProcessRequestedJobs();
        }
		Fls_MainFunction_CheckJobResult(WorkResult);     
    }
    else
    {
        /* no pending job */
        /* The function Fls_MainFunction shall return without any action if no job is pending. */
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_LLDGetJobResult
 * Description   : Returns the LLD JobResult.
 * 
 *END**************************************************************************/
static MemIf_JobResultType Fls_LLDGetJobResult(void)
{
    return Fls_eLLDJobResult;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_LLDClrJobResult
 * Description   : Clear status of erase or write hardware job result.
 * 
 *END**************************************************************************/
static void Fls_LLDClrJobResult(void)
{
    Fls_eLLDJobResult = MEMIF_JOB_OK;
    Fls_eLLDJob = FLASH_JOB_NONE;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_Compare
 * Description   : Compares the contents of an area of flash memory with that 
 * of an application data buffer.
 * 
 *END**************************************************************************/
Std_ReturnType Fls_Compare (Fls_AddressType SourceAddress, const uint8_t * TargetAddressPtr, Fls_LengthType Length)
{
    Std_ReturnType retVal = E_NOT_OK;
	DEV_ASSERT(Fls_gConfigPtr!=NULL_PTR);
    DEV_ASSERT(TargetAddressPtr!=NULL_PTR);
    DEV_ASSERT(Length != 0);
    DEV_ASSERT(Fls_IsAddrInRange(SourceAddress)==true);
    DEV_ASSERT(Fls_IsAddrInRange(SourceAddress+Length)==true);

    if ((MEMIF_JOB_PENDING == Fls_gJobState.Fls_JobResult)||\
        (Fls_gConfigPtr==NULL_PTR)||\
        (TargetAddressPtr==NULL_PTR)||\
        (Length == 0U)||\
        (Fls_IsAddrInRange(SourceAddress)==false)||\
        (Fls_IsAddrInRange(SourceAddress+Length)==false))   
    {
        /* The function Fls_Compare shall check that the driver is currently not busy. 
            If this check fails, the function Fls_Compare shall reject the compare job, 
            raise the development error FLS_E_BUSY and return with E_NOT_OK. */
    }
    else
    {
        /* The function Fls_Compare shall copy the given parameters to
            Fls module internal variables and initiate a compare job.*/
        Fls_gJobState.Fls_JobCurrentAddr = SourceAddress;
        Fls_gJobState.Fls_JobEndAddr = SourceAddress + Length - 1U;
        Fls_gJobState.Fls_JobSectorIt = Fls_GetSectorIndexByAddr(SourceAddress);
        Fls_gJobState.Fls_JobSectorEnd = Fls_GetSectorIndexByAddr(Fls_gJobState.Fls_JobEndAddr);
        Fls_gJobState.Fls_JobType = FLS_JOB_COMPARE;
        /* Compare will reuse write data pointer */
        Fls_gJobState.Fls_JobWriteDataPtr = TargetAddressPtr;
        /* After initiating the compare job, the function Fls_Compare shall
            set the status to MEMIF_BUSY. */
        /* After initiating the compare job, the function Fls_Compare shall
            set the job result to MEMIF_JOB_PENDING. */
        Fls_gJobState.Fls_JobResult = MEMIF_JOB_PENDING;
        /* After initiating the compare job, the function Fls_Compare shall
            return with E_OK. */
        /* Write command accepted */
        retVal = E_OK;
    }
    
    return retVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_BlankCheck
 * Description   : The function Fls_BlankCheck shall verify, whether a given 
 * memory area has been erased but not (yet) programmed. The function shall 
 * limit the maximum number of checked flash cells per main function cycle 
 * to the configured value FlsMaxReadNormalMode or FlsMaxReadFastMode 
 * respectively.
 * 
 *END**************************************************************************/
Std_ReturnType Fls_BlankCheck (Fls_AddressType TargetAddress, Fls_LengthType Length)
{
    Std_ReturnType retVal = E_NOT_OK;
	DEV_ASSERT(Fls_gConfigPtr!=NULL_PTR);
    DEV_ASSERT(Length != 0);
    DEV_ASSERT(Fls_IsAddrInRange(TargetAddress)==true);
    DEV_ASSERT(Fls_IsAddrInRange(TargetAddress+Length)==true);
	
    if((MEMIF_JOB_PENDING == Fls_gJobState.Fls_JobResult)||\
        (Fls_gConfigPtr==NULL_PTR)||\
        (Length == 0U)||\
        (Fls_IsAddrInRange(TargetAddress)==false)||\
        (Fls_IsAddrInRange(TargetAddress+Length)==false))
    {
        /* The function Fls_BlankCheck shall check that the driver is currently not busy. 
            If this check fails, the function Fls_BlankCheck shall reject the verification 
            request, raise the development error FLS_E_BUSY and return with E_NOT_OK.*/
    }
    else
    {
        Fls_gJobState.Fls_JobCurrentAddr = TargetAddress;
        Fls_gJobState.Fls_JobEndAddr = TargetAddress + Length - 1U;
        Fls_gJobState.Fls_JobSectorIt = Fls_GetSectorIndexByAddr(TargetAddress);
        Fls_gJobState.Fls_JobSectorEnd = Fls_GetSectorIndexByAddr(Fls_gJobState.Fls_JobEndAddr);
        Fls_gJobState.Fls_JobType = FLS_JOB_BLANK_CHECK;
        /*  After initiating the verification job, the function 
        Fls_BlankCheck shall set the FLS module status to MEMIF_BUSY. */
        /* After initiating the verification job, the function 
        Fls_BlankCheck shall set the FLS module job result to MEMIF_JOB_PENDING.*/
        Fls_gJobState.Fls_JobResult = MEMIF_JOB_PENDING;
        retVal = E_OK;
    }
    
    return retVal;
}

/*==================================================================================================
 *                                       LOCAL FUNCTIONS
==================================================================================================*/
/**
 * @brief   Sector start address aligned check
 * @details Check the target address is sector start address or not.
 * @param[in] TargetAddress - The target address
 * @return   bool - true: TargetAddress is sector start address
*/
static bool Fls_IsSectorStartAddrSectorAligned(Fls_AddressType TargetAddress)
{
    bool retVal = (bool)false;
    uint16_t sectorIndex;
    sectorIndex = Fls_GetSectorIndexByAddr(TargetAddress);
    if (FLS_INVALID_SECTOR_INDEX != sectorIndex)
    {
        if (TargetAddress == Fls_gConfigPtr->sectorList[sectorIndex].sectorStartAddress)
        {
            retVal = (bool)true;
        }
    }
    return retVal;
}
/**
 * @brief   Sector end address aligned check
 * @details Check the target address is sector end address or not.
 * @param[in] TargetAddress - The target address
 * @return   boolean - TRUE: TargetAddress is sector end address
*/
static bool Fls_IsSectorEndAddrSectorAligned(Fls_AddressType TargetAddress)
{
    bool retVal = (bool)false;
    uint16_t sectorIndex;
    sectorIndex = Fls_GetSectorIndexByAddr(TargetAddress - 1U);
    if (FLS_INVALID_SECTOR_INDEX != sectorIndex)
    {
        if (TargetAddress == (Fls_gConfigPtr->sectorList[sectorIndex].sectorStartAddress + 
                              Fls_gConfigPtr->sectorList[sectorIndex].sectorSize))
        {
            retVal = (bool)true;
        }
    }
    return retVal;
}
/**
 * @brief   Page address aligned check
 * @details Check the target address is the sector or not.
 * @param[in] TargetAddress - The target address.
 * @return   bool - true: TargetAddress is in a sector.
*/
static bool Fls_IsSectorAddrPageAligned(Fls_AddressType TargetAddress)
{
    bool retVal = (bool)false;
    uint16_t sectorIndex;
    sectorIndex = Fls_GetSectorIndexByAddr(TargetAddress);
    if (sectorIndex < Fls_gConfigPtr->ConfigSectorNum)
    {
        if (0U == (TargetAddress & (Fls_gConfigPtr->sectorList[sectorIndex].pageSize - 1U)))
        {
            retVal = (bool)true;
        }
    }
    else if(sectorIndex == Fls_gConfigPtr->ConfigSectorNum)
    {
        if(TargetAddress == (Fls_gConfigPtr->sectorList[sectorIndex-1U].sectorStartAddress+\
                             Fls_gConfigPtr->sectorList[sectorIndex-1U].sectorSize))
        {
            retVal = (bool)true;
        }
    }
    else
    {
        /*Nothing to do*/
    }
    return retVal;
}

/**
 * @brief   Address range check
 * @details Check the target address is in the flash range or not.
 * @param[in] TargetAddress - The target address.
 * @return   bool - true: TargetAddress is in the flash range.
*/
static bool Fls_IsAddrInRange(Fls_AddressType TargetAddress)
{
    bool retVal = (bool)false;
    uint16_t sectorIndex;
    sectorIndex = Fls_GetSectorIndexByAddr(TargetAddress);
    if (sectorIndex < Fls_gConfigPtr->ConfigSectorNum)
    {
        retVal = (bool)true;
    }
    else if(sectorIndex == Fls_gConfigPtr->ConfigSectorNum)
    {
        if(TargetAddress == (Fls_gConfigPtr->sectorList[sectorIndex-1U].sectorStartAddress+\
                             Fls_gConfigPtr->sectorList[sectorIndex-1U].sectorSize))
        {
            retVal = (bool)true;
        }
    }
    else
    {
        /*Nothing to do*/
    }
    return retVal;
}
/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_GetSectorIndexByAddr
 * Description   : FLS Logic Address to Physical Address conversion
 * 
 *END**************************************************************************/
LOCAL_INLINE uint16_t Fls_GetSectorIndexByAddr(Fls_AddressType LogicAddress)
{
    uint16_t sectorIndex = FLS_INVALID_SECTOR_INDEX;
    for (sectorIndex = 0; sectorIndex < Fls_gConfigPtr->ConfigSectorNum; sectorIndex++)
    {
        if ((LogicAddress >= Fls_gConfigPtr->sectorList[sectorIndex].sectorStartAddress)  
            && (LogicAddress < (Fls_gConfigPtr->sectorList[sectorIndex].sectorStartAddress + 
                                 Fls_gConfigPtr->sectorList[sectorIndex].sectorSize)))
        {
            break;
        }
    }
    return sectorIndex;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_MainFunction_CheckJobResult
 * Description   : Check the result of the job and call the notification 
 * function in Fls main function.
 * 
 *END**************************************************************************/
static void Fls_MainFunction_CheckJobResult(MemIf_JobResultType WorkResult)
{
    Fls_gJobState.Fls_JobResult = WorkResult;

    if (MEMIF_JOB_OK == WorkResult)
    {
        if (NULL_PTR != Fls_gConfigPtr->jobEndNotificationFunPtr)
        {
            /* Call FlsJobEndNotification function if configured */
            Fls_gConfigPtr->jobEndNotificationFunPtr();
        }
    }
    else if ( (MEMIF_JOB_FAILED == WorkResult) ||
              (MEMIF_JOB_CANCELED == WorkResult) ||
              (MEMIF_BLOCK_INCONSISTENT == WorkResult)
            )
    {
        if (NULL_PTR != Fls_gConfigPtr->jobErrorNotificationFunPtr)
        {
            /* Call FlsJobErrorNotification function if configured */
            Fls_gConfigPtr->jobErrorNotificationFunPtr();
        }
    }
    else
    {
        ; /* Empty clause added to fulfill MISRA. */
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_ExecuteJobErase
 * Description   : FLS Execute Erase job.
 * 
 *END**************************************************************************/
MemIf_JobResultType Fls_ExecuteJobErase(void)
{
    uint16_t sectorIt;
    MemIf_JobResultType jobResult = MEMIF_JOB_PENDING;
    Fls_Drv_StatusType eFlsRet;
    bool Asynch;
    Fls_LLDReturnType LldRetVal;

    if(Fls_gJobState.Fls_JobCurrentAddr>Fls_gJobState.Fls_JobEndAddr)
    {
        /* No more sectors to erase */
        jobResult = MEMIF_JOB_OK;
    }
    else
    {
        sectorIt = Fls_gJobState.Fls_JobSectorIt;
         /* Get the Fls_IPW_SectorErase function operation mode */
        if ( true == Fls_gConfigPtr->sectorList[sectorIt].asyncAccess)
        {
            /* The sector write is performed in an asynchronous manner, it is ASYNC mode. */
            Asynch = true;
        }
        else
        {
            /* The sector write is SYNC */
            Asynch = false;
        }
#ifdef FEATURE_EFM_ERASE_SECTOR_RETRY_CMD_CODE
        if ((false == Asynch) && (ERASE_MODE_QUICK == Fls_gConfigPtr->eraseMode)){
            eFlsRet = Fls_drvSectorEraseQuick(Fls_gConfigPtr->sectorList[sectorIt].sectorHwStartAddress);
            if(STATUS_FTFC_FLS_IP_SUCCESS == eFlsRet)
            {
                /* An additional one must be appended to make the erase more thorough */
                eFlsRet = Fls_drvSectorEraseQuick(Fls_gConfigPtr->sectorList[sectorIt].sectorHwStartAddress);
            }else{
                /* Hardware failures don't require another attempt */
            }
        }
        else
        {
            eFlsRet = Fls_drvSectorErase(Fls_gConfigPtr->sectorList[sectorIt].sectorHwStartAddress, Asynch);
        }
#else
        eFlsRet = Fls_drvSectorErase(Fls_gConfigPtr->sectorList[sectorIt].sectorHwStartAddress, Asynch);
#endif
        LldRetVal = Fls_drvTranslateReturnCode(eFlsRet);
        Fls_gJobState.Fls_JobCurrentAddr += Fls_gConfigPtr->sectorList[sectorIt].sectorSize;

        if ( (Fls_gJobState.Fls_JobCurrentAddr >Fls_gJobState.Fls_JobEndAddr) && (FLASH_E_OK == LldRetVal) )
        {
            jobResult = MEMIF_JOB_OK;
        }
        else if (FLASH_E_FAILED == LldRetVal)
        {
            jobResult = MEMIF_JOB_FAILED;
        }
        else
        {
            /* The erase operation is either:
                1. For ASYNCHRONOUS operation: pending (FLASH_E_PENDING) in hardware, or
                2. For SYNCHRONOUS operation: finished (FLASH_E_OK) and there are more sectors to erase.

                1. If the operation is Async, than the sector is increased in
                   Fls_IPW_LLDMainFunction()(for Async erase).
                   when completing the job.
                2. If the operation is Sync erase, than the sector is increased below because the job is complete.
            */
            if (false == Fls_gConfigPtr->sectorList[sectorIt].asyncAccess)
            {
                 if((Fls_gJobState.Fls_JobCurrentAddr>=(Fls_gConfigPtr->sectorList[Fls_gJobState.Fls_JobSectorIt].sectorStartAddress+\
                        Fls_gConfigPtr->sectorList[Fls_gJobState.Fls_JobSectorIt].sectorSize))&&\
                        (Fls_gJobState.Fls_JobCurrentAddr <=Fls_gJobState.Fls_JobEndAddr)) 
                {
                    Fls_gJobState.Fls_JobSectorIt++;
                }
            }
        }
    }
    
    return(jobResult);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_ExecuteJobWrite
 * Description   : FLS Execute Write job.
 * 
 *END**************************************************************************/
Fls_LLDReturnType Fls_ExecuteJobWrite(const Fls_AddressType SectorOffset,
                                                    const Fls_AddressType Length)
{
    uint16_t sectorIndex;
    Fls_LLDReturnType jobResult;
    Fls_AddressType writeHwAddr;
    sectorIndex = Fls_gJobState.Fls_JobSectorIt;
    writeHwAddr = SectorOffset + Fls_gConfigPtr->sectorList[sectorIndex].sectorHwStartAddress;
    /* Page program */	
    jobResult = Fls_drvSectorWrite(writeHwAddr,Length,Fls_gJobState.Fls_JobWriteDataPtr,Fls_gConfigPtr->sectorList[sectorIndex].asyncAccess);

    return(jobResult);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_ExecuteJobReadBlankCheckCompare
 * Description   : FLS Execute Read job.
 * 
 *END**************************************************************************/
Fls_LLDReturnType Fls_ExecuteJobReadBlankCheckCompare(const Fls_AddressType u32SectorOffset,
                                     const Fls_AddressType u32Length,
                                     uint8_t * pJobDataDestPtr,
                                     const uint8_t *pJobDataSrcPtr
                                    )
{
    Fls_LLDReturnType eLldRetVal = FLASH_E_FAILED;
    Fls_Drv_StatusType eReturnCode = STATUS_FTFC_FLS_IP_ERROR;
    uint32_t u32ReadStartAddress = 0U;

    /* Get the base address of the sector. */
    u32ReadStartAddress = Fls_gConfigPtr->sectorList[Fls_gJobState.Fls_JobSectorIt].sectorHwStartAddress;
    /* Add the offset */
    u32ReadStartAddress += u32SectorOffset;

    /* Decide whether the job is a Read, a compare, or a blank check. */
    if (NULL_PTR != pJobDataDestPtr)
    {
        eReturnCode = Fls_drvReadFlash(u32ReadStartAddress, u32Length,pJobDataDestPtr);
    }
    else
    {
        eReturnCode = Fls_drvCompareFlash(u32ReadStartAddress,u32Length,pJobDataSrcPtr);
    }
        eLldRetVal = Fls_drvTranslateReturnCode(eReturnCode);

    return(eLldRetVal);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_ProcessRequestedJobs
 * Description   : Decides which sub-function to call based on the job type: 
 * Modify (Write, Erase) or Read (Read, Compare).
 * 
 *END**************************************************************************/
static MemIf_JobResultType Fls_ProcessRequestedJobs( void )
{
    MemIf_JobResultType WorkResult;

    /* Flash is initialized and a job is pending */
	switch (Fls_gJobState.Fls_JobType)
	{
         /**** MODIFY jobs ****/
        /* Start current job or start a new batch of current job or verify status of current job. */
	    case FLS_JOB_ERASE:
	        WorkResult=Fls_ExecuteJobErase();
	        break;
	    case FLS_JOB_WRITE:
	        WorkResult=Fls_DoJobDataTransfer(Fls_u32MaxWrite);
	        break;
        /**** READ jobs ****/
        /* FLS_JOB_READ, FLS_JOB_BLANK_CHECK and FLS_JOB_COMPARE jobs are always synchronous */
	    case FLS_JOB_READ:
	    case FLS_JOB_COMPARE:
	    case FLS_JOB_BLANK_CHECK:
	        WorkResult=Fls_DoJobDataTransfer(Fls_u32MaxRead);
	        break;
	    default:
            WorkResult=MEMIF_JOB_PENDING;
	        break;
	}

    return WorkResult;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvTranslateReturnCode
 * Description   : Translate the return code from drv.
 * 
 *END**************************************************************************/
static Fls_LLDReturnType Fls_drvTranslateReturnCode(Fls_Drv_StatusType eReturnCode)
{
    Fls_LLDReturnType eLldRetVal;

    /* Translate the return code from IPV to HLD */
    switch (eReturnCode)
    {
        case STATUS_FTFC_FLS_IP_SUCCESS:
            /* Operation succeeded */
            eLldRetVal = FLASH_E_OK;
            break;

        case STATUS_FTFC_FLS_IP_BUSY:
            /* Operation is pending */
            eLldRetVal = FLASH_E_PENDING;
            break;

        case STATUS_FTFC_FLS_IP_ERROR_BLANK_CHECK:
            /* Content of flash memory doesn't match with erased value */
            eLldRetVal = FLASH_E_BLOCK_INCONSISTENT;
            break;

        case STATUS_FTFC_FLS_IP_ERROR_PROGRAM_VERIFY:
            /* Content of flash memory doesn't match with data buffer */
            eLldRetVal = FLASH_E_BLOCK_INCONSISTENT;
            break;

        default:
            /* Operation failed due to hardware error */
            eLldRetVal = FLASH_E_FAILED;
            break;
    }

    return eLldRetVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_DoJobDataTransfer
 * Description   : Read, write, or compare flash data.
 * 
 *END**************************************************************************/
static MemIf_JobResultType Fls_DoJobDataTransfer(const Fls_LengthType MaxTransfer)
{
    MemIf_JobResultType RetVal = MEMIF_JOB_PENDING;
    uint16_t sectorItTemp = Fls_gJobState.Fls_JobSectorIt;
    Fls_AddressType sectorAddrEndTemp = 0;


    /* No more data to write */
    if (Fls_gJobState.Fls_JobCurrentAddr > Fls_gJobState.Fls_JobEndAddr)
    {
        RetVal = MEMIF_JOB_OK;
    }
    else
    {
        Fls_AddressType u32MaxTransferAddrEnd;
        Fls_LLDReturnType TransferRetVal;

        /* Last address to transfer data to/from in this call of the Fls_MainFunction() functions */
        u32MaxTransferAddrEnd = Fls_CalcMaxTransferAddrEnd( MaxTransfer );

         /* Transfer sector by sector until the u32MaxTransferAddrEnd address is reached or error is detected */
        do
        {
            Fls_AddressType SectorTransferOffset;
            Fls_AddressType u32SectorTransferLength;
            sectorItTemp = Fls_gJobState.Fls_JobSectorIt;

            /* Calculate offset and length of the sector data transfer */
            SectorTransferOffset = Fls_gJobState.Fls_JobCurrentAddr - Fls_gConfigPtr->sectorList[sectorItTemp].sectorStartAddress;
            u32SectorTransferLength = Fls_CalcSectorTransferLength( u32MaxTransferAddrEnd );

            /* Update the Fls_JobAddressIt iterator.*/
            Fls_gJobState.Fls_JobCurrentAddr += u32SectorTransferLength;

            /* Call Low-level driver to transfer data to/from physical flash device */
            TransferRetVal = Fls_ProcessJobDataTransfer(SectorTransferOffset, u32SectorTransferLength);

            if ( FLASH_E_OK == TransferRetVal ) /* If the previous(SYNC job) has finished successfully */
            {
                sectorAddrEndTemp = Fls_gConfigPtr->sectorList[sectorItTemp].sectorStartAddress +
                                    Fls_gConfigPtr->sectorList[sectorItTemp].sectorSize;
                if (Fls_gJobState.Fls_JobCurrentAddr >= sectorAddrEndTemp)
                {
                    /* Move on to the next sector */
                    Fls_gJobState.Fls_JobSectorIt++;
                }
            }
        }
        while ( (Fls_gJobState.Fls_JobCurrentAddr <= u32MaxTransferAddrEnd) &&
                (FLASH_E_OK == TransferRetVal)
              );

        if (FLASH_E_FAILED == TransferRetVal)
        {
            RetVal = MEMIF_JOB_FAILED;
        }
        else if (FLASH_E_BLOCK_INCONSISTENT == TransferRetVal)
        {
            /* compare job only */
            RetVal = MEMIF_BLOCK_INCONSISTENT;
        }
        else if ( (Fls_gJobState.Fls_JobCurrentAddr > Fls_gJobState.Fls_JobEndAddr) &&
                  (FLASH_E_OK == TransferRetVal)
                )
        {
            /* All desired job data has been successfully transferred */
            RetVal = MEMIF_JOB_OK;
        }
        else
        {
            /* The write operation is either pending (FLASH_E_PENDING)
            in hardware in case of asynchronous operation or
            finished (FLASH_E_OK) in case of synchronous operation
            and there is more pages to write */
        }
    }
    return(RetVal);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_CalcMaxTransferAddrEnd
 * Description   : Calculate last logical address to read, write, or compare
 *  from in current cycle of Fls_MainFunction
 * 
 *END**************************************************************************/
static Fls_AddressType Fls_CalcMaxTransferAddrEnd(const Fls_LengthType MaxTransfer)
{
    Fls_AddressType MaxTransferEndAddr = Fls_gJobState.Fls_JobEndAddr;

    /* Adjust the MaxTransferEndAddr address to transfer only
        the MaxTransfer bytes in one Fls_MainFunction() call */
    if ( MaxTransferEndAddr >= (Fls_gJobState.Fls_JobCurrentAddr + MaxTransfer) )
    {
        MaxTransferEndAddr = (Fls_gJobState.Fls_JobCurrentAddr + MaxTransfer) - 1U;
    }
    else
    {
        /* No adjustment is necessary. Job will finish in this cycle
            of Fls_MainFunction */
    }

    return MaxTransferEndAddr;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_CalcSectorTransferLength
 * Description   : Calculate number of bytes to read, write, or compare
 *  from current sector
 * 
 *END**************************************************************************/
static Fls_AddressType Fls_CalcSectorTransferLength(const Fls_AddressType MaxTransferEndAddr)
{
    Fls_AddressType SectorEndAddr;
    Fls_AddressType MaxTransferAddr = MaxTransferEndAddr;
    uint32_t TransferSize = 0U;
    uint32_t InterfaceSize = FEATURE_EFM_WRITE_UNIT_SIZE;

    /* Get last address of Fls_u32JobSectorIt sector */
    SectorEndAddr = Fls_gConfigPtr->sectorList[Fls_gJobState.Fls_JobSectorIt].sectorStartAddress+\
                    Fls_gConfigPtr->sectorList[Fls_gJobState.Fls_JobSectorIt].sectorSize-1U;

    /* Adjust SectorEndAddr based on hardware implementation */
    if (FLS_JOB_WRITE == Fls_gJobState.Fls_JobType)
    {
#ifdef FEATURE_EFM_PROGRAM_DATA_CMD_CODE
        if((true == FLS_ADDRESS_VALID_D_FLASH(Fls_gJobState.Fls_JobCurrentAddr))){
            InterfaceSize = FEATURE_EFM_WRITE_DATA_UNIT_SIZE;
        }
#endif
        TransferSize = MaxTransferAddr - Fls_gJobState.Fls_JobCurrentAddr + 1U;
        if (TransferSize > (InterfaceSize - (Fls_gJobState.Fls_JobCurrentAddr % InterfaceSize)))
        {
            MaxTransferAddr = (Fls_gJobState.Fls_JobCurrentAddr + (InterfaceSize -\
                              (Fls_gJobState.Fls_JobCurrentAddr % InterfaceSize))) - 1U;
        }
    }
    /* Adjust the SectorEndAddr according to max number of bytes to transfer
        during one cycle of Fls_MainFunction */
    if (SectorEndAddr > MaxTransferAddr)
    {
        SectorEndAddr = MaxTransferAddr;
    }
    else
    {
        /* No adjustment is necessary. There is more sectors to transfer */
    }

    /* Calculate number of bytes to read from the sector */
    return ((SectorEndAddr - Fls_gJobState.Fls_JobCurrentAddr) + 1U);
}
/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_ProcessJobDataTransfer
 * Description   : Fls Process Job Data Transfer
 * 
 *END**************************************************************************/
static Fls_LLDReturnType Fls_ProcessJobDataTransfer(const Fls_AddressType SectorOffset, const Fls_AddressType Length )
{
    Fls_LLDReturnType TransferRetVal = FLASH_E_FAILED;

    /* Call Low-level driver to transfer data to/from physical flash device */
    switch (Fls_gJobState.Fls_JobType)
    {
        case FLS_JOB_WRITE:
            TransferRetVal =  Fls_ExecuteJobWrite( SectorOffset, Length );
            /* Update the source data pointer for next write */
            Fls_gJobState.Fls_JobWriteDataPtr = &(Fls_gJobState.Fls_JobWriteDataPtr[Length]);
            break;

        case FLS_JOB_READ:
            TransferRetVal =  Fls_ExecuteJobReadBlankCheckCompare(SectorOffset, Length, Fls_gJobState.Fls_JobReadDataPtr, NULL_PTR);
            /* Update the destination data pointer for next read */
            Fls_gJobState.Fls_JobReadDataPtr = &(Fls_gJobState.Fls_JobReadDataPtr[Length]);
            break;

        case FLS_JOB_COMPARE:
            TransferRetVal =  Fls_ExecuteJobReadBlankCheckCompare(SectorOffset, Length, NULL_PTR, Fls_gJobState.Fls_JobWriteDataPtr);
            /* Update the source data pointer for next compare */
            Fls_gJobState.Fls_JobWriteDataPtr = &(Fls_gJobState.Fls_JobWriteDataPtr[Length]);
            break;

        case FLS_JOB_BLANK_CHECK:
            TransferRetVal =  Fls_ExecuteJobReadBlankCheckCompare(SectorOffset, Length, NULL_PTR, NULL_PTR);
            break;

        default:
            ; /* Do nothing - should not happen in Fully Trusted Environment;
            "default" clause added to fulfil MISRA Rule 15.3 */
            break;
    }

    return TransferRetVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_Cancel
 * Description   : Cancel the current job.
 *END**************************************************************************/
void Fls_Cancel(void)
{
    DEV_ASSERT(Fls_gConfigPtr != NULL_PTR);

    if (MEMIF_JOB_PENDING == Fls_gJobState.Fls_JobResult)
    {
        /* Get channel type(INTERNAL, QSPI,...) to determine the HW IP used(internal or external flash). */
        if (MEMIF_JOB_PENDING == Fls_LLDGetJobResult())
        {
            if (STATUS_FTFC_FLS_IP_SUCCESS == Fls_drvCancel())
            {
                /* Mark the job as canceled.*/
                Fls_eLLDJob = FLASH_JOB_NONE;
                Fls_eLLDJobResult = MEMIF_JOB_CANCELED;
            }
        }
        else
        {
            /* no hardware job (asynchronous) is pending, do nothing */
            /* Mark the internal job as canceled.*/
            Fls_eLLDJob = FLASH_JOB_NONE;
            Fls_eLLDJobResult = MEMIF_JOB_CANCELED;
        }

        /* If underlying hardware job was successfully cancelled */
        if (MEMIF_JOB_CANCELED == Fls_LLDGetJobResult())
        {
            /* Update global job result status */
            Fls_gJobState.Fls_JobResult = MEMIF_JOB_CANCELED;
        }
        if (NULL_PTR != Fls_gConfigPtr->jobErrorNotificationFunPtr)
        {
            /* Call FlsJobErrorNotification function if configured */
            Fls_gConfigPtr->jobErrorNotificationFunPtr();
        }
        else
        {
            /* Callback notification configured as null pointer */
        }
    }
    else
    {
        /* no pending job */
    }
}

#ifdef __cplusplus
}
#endif

/** @} */
