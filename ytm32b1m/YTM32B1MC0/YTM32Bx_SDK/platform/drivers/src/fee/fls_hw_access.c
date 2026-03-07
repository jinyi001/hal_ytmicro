/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file fls_hw_access.c
 * @version 1.4.0
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 * PRQA S 0310 Rule 11.3: Casting to different object pointer type.
 *
 * PRQA S 3305 Rule 11.3: A cast shall not be performed between a pointer to object type and a
 *                        pointer to a different object type
 *
 * PRQA S 2995 Rule 2.2: The result of this logical operation is always 'true'.
 *
 * PRQA S 2996 Rule 2.2: The result of this logical operation is always 'false'.
 *
 */

/*==================================================================================================
 *                                        INCLUDE FILES
==================================================================================================*/
#ifdef __cplusplus
extern "C"{
#endif
#include "fls_hw_access.h"
#include "interrupt_manager.h"
/*==================================================================================================
 *                                      LOCAL VARIABLES
==================================================================================================*/
static Fls_Drv_StatusType Fls_Drv_eReadStatus = STATUS_FTFC_FLS_IP_ERROR;

static Fls_Drv_StatusType Fls_Drv_eWriteStatus = STATUS_FTFC_FLS_IP_ERROR;

static Fls_Drv_StatusType Fls_Drv_eEraseStatus = STATUS_FTFC_FLS_IP_ERROR;
/*==================================================================================================
 *                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
static uint32_t Fls_drvComputeReadSize(uint32_t srcAddress, uint32_t desAddress, uint32_t byteRemain);
static Fls_Drv_StatusType Fls_drvSectorWriteStatus(void);
static Fls_Drv_StatusType Fls_drvSectorEraseStatus(void);
static Fls_Drv_StatusType Fls_drvGetCtrlStatus(void);
static void Fls_drvCompareCheck(uint32_t address, const uint8_t * data, uint32_t size);

START_FUNCTION_DECLARATION_RAMSECTION
static Fls_Drv_StatusType Fls_drvLoadCommandSync(uint8_t command) __attribute__((noinline))
END_FUNCTION_DECLARATION_RAMSECTION
/*==================================================================================================
 *                                     LOCAL INLINE FUNCTION 
==================================================================================================*/

/*==================================================================================================
 *                                       GLOBAL FUNCTIONS
==================================================================================================*/

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvInit
 * Description   : FLS hardware initialization.
 * 
 *END**************************************************************************/
void Fls_drvInit(void)
{
    /* check if Flash currently idle */
    if (true == Fls_drvCheckIdle()){
        /* clear all error flags */
        EFM->STS =  FEATURE_EFM_CMD_ERROR_MASK | EFM_STS_DONE_MASK;

#if defined(FEATURE_EFM_TIMING_MAX) && (FEATURE_EFM_TIMING_MAX == 1U)
        EFM->TIMING1 = FEATURE_EFM_TIMING1_MAX;
        EFM->TIMING2 = FEATURE_EFM_TIMING2_MAX;
#endif

#if defined(FEATURE_EFM_HAS_READONLY_BIT) && (FEATURE_EFM_HAS_READONLY_BIT == 1)
        /* Read only flash array */
        EFM->CTRL |= EFM_CTRL_READONLY_MASK;
#endif

        /* [SWS_Fls_00048] If supported by hardware, the function Fls_Init shall set the 
            flash memory erase/write protection as provided in the configuration set. */
        Fls_gJobState.Fls_JobResult = MEMIF_JOB_OK;  
        Fls_gJobState.Fls_JobCurrentAddr = (Fls_AddressType)0U;
        Fls_gJobState.Fls_JobEndAddr = (Fls_AddressType)0U;
        Fls_gJobState.Fls_JobType = (Fls_JobType)FLS_JOB_IDLE;
        Fls_gJobState.Fls_JobWriteDataPtr = NULL_PTR;
        Fls_gJobState.Fls_JobReadDataPtr = NULL_PTR;
        Fls_eLLDJobResult  = MEMIF_JOB_OK;
    }
    else
    {
        /* Flash is busy, return error */
        Fls_gJobState.Fls_JobResult = MEMIF_JOB_FAILED;  
	    Fls_eLLDJobResult  = MEMIF_JOB_FAILED;
    }
    Fls_eLLDJob = FLASH_JOB_NONE;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvLoadCommandSync
 * Description   : Fls load command in sync mode.
 * 
 *END**************************************************************************/
START_FUNCTION_DEFINITION_RAMSECTION
static Fls_Drv_StatusType Fls_drvLoadCommandSync(uint8_t command)
{
    Fls_Drv_StatusType eReturnCode = STATUS_FTFC_FLS_IP_SUCCESS;
    EFM->CMD = command;
    while ((EFM_STS_DONE_MASK != (EFM->STS & EFM_STS_DONE_MASK)) &&
           (EFM_STS_ACCERR_MASK != (EFM->STS & EFM_STS_ACCERR_MASK)))
    {
        /* wait until command is finished */
    }
    /* In ram function, should not call any function in flash. */
    if(0U == (EFM->STS & FEATURE_EFM_CMD_ERROR_MASK))
    {
        eReturnCode = STATUS_FTFC_FLS_IP_SUCCESS;
    }else{
        eReturnCode = STATUS_FTFC_FLS_IP_ERROR;
    }
    return eReturnCode;
}
END_FUNCTION_DEFINITION_RAMSECTION

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvSectorErase
 * Description   : Flash erase job process.
 * 
 *END**************************************************************************/
Fls_Drv_StatusType Fls_drvSectorErase(Fls_AddressType StartAddress, bool Async)
{
    Fls_Drv_StatusType eReturnCode;

    /*Check EFM Busy or not*/
    if(false ==Fls_drvCheckIdle())
    {
        eReturnCode = STATUS_FTFC_FLS_IP_BUSY;
	 	Fls_Drv_eEraseStatus = STATUS_FTFC_FLS_IP_BUSY;
    }
    else
    {
        if(true==FLS_ADDRESS_VALID(StartAddress)) /* PRQA S 2995 */
        {
            INT_SYS_DisableIRQGlobal();
#ifndef EFM_CTRL_WE_MASK
            /* Add NOP command to clear write buffer */
            EFM->CMD = 0;
#endif
#if defined(FEATURE_EFM_HAS_READONLY_BIT) && (FEATURE_EFM_HAS_READONLY_BIT == 1)
            /* Flash can be read, erased and programmed */
            EFM->CTRL &= ~EFM_CTRL_READONLY_MASK;
#endif
            /* Clear all error flags */
            EFM->STS =  FEATURE_EFM_CMD_ERROR_MASK | EFM_STS_DONE_MASK;
             /* parsing parameters to flash */
            EFM_ENABLE_WE_COMMAND(EFM);
            *(volatile uint32_t *)StartAddress = 0x01234567U;
#ifdef EFM_STS_ARRAY_SELECTED_MASK
            while (EFM_STS_ARRAY_SELECTED_MASK != (EFM->STS & EFM_STS_ARRAY_SELECTED_MASK))
            {
                /* Wait until the array is selected */
            }
#endif /* EFM_STS_SET_ADDR_MASK */
#ifdef EFM_STS_SET_ADDR_MASK
            while (EFM_STS_SET_ADDR_MASK != (EFM->STS & EFM_STS_SET_ADDR_MASK))
            {
                /* Wait until the address is set */
            }
#endif /* EFM_STS_SET_ADDR_MASK */
            EFM_DISABLE_WE_COMMAND(EFM);

#if defined(FEATURE_EFM_HAS_ERASE_TIMING_UNION) && (FEATURE_EFM_HAS_ERASE_TIMING_UNION == 1U)
            EFM->TIMING2 = FEATURE_EFM_SECTOR_ERASE_TIMING;
#endif
            EFM_UNLOCK_CMD_REGISTER(EFM);
            if (Async)
            {
                EFM->CMD = FEATURE_EFM_ERASE_SECTOR_CMD_CODE;
                Fls_eLLDJob = FLASH_JOB_ERASE;
                Fls_eLLDJobResult = MEMIF_JOB_PENDING;
                eReturnCode = STATUS_FTFC_FLS_IP_BUSY;
                Fls_Drv_eEraseStatus = STATUS_FTFC_FLS_IP_SUCCESS;
            }
            else
            {
                eReturnCode = Fls_drvLoadCommandSync(FEATURE_EFM_ERASE_SECTOR_CMD_CODE);
                if (eReturnCode == STATUS_FTFC_FLS_IP_SUCCESS)
                {
                    Fls_eLLDJobResult  = MEMIF_JOB_OK;
                }
                else
                {
                    Fls_eLLDJobResult  = MEMIF_JOB_FAILED;
                }
                Fls_Drv_eEraseStatus = eReturnCode;
            }
#if defined(FEATURE_EFM_HAS_READONLY_BIT) && (FEATURE_EFM_HAS_READONLY_BIT == 1)
            /* Read only flash array */
            EFM->CTRL |= EFM_CTRL_READONLY_MASK;
#endif
            INT_SYS_EnableIRQGlobal();
        }
        else
        {
            eReturnCode = STATUS_FTFC_FLS_IP_ERROR_INPUT_PARAM;
            Fls_Drv_eEraseStatus = STATUS_FTFC_FLS_IP_ERROR_INPUT_PARAM;
        }
    }
    
    return (eReturnCode);
}

#ifdef FEATURE_EFM_ERASE_SECTOR_RETRY_CMD_CODE
/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvSectorEraseQuick
 * Description   : Flash erase quick job process, only for synchronous mode.
 * 
 *END**************************************************************************/
Fls_Drv_StatusType Fls_drvSectorEraseQuick(Fls_AddressType StartAddress)
{
    Fls_Drv_StatusType eReturnCode;
    
    /*Check EFM Busy or not*/
    if(false ==Fls_drvCheckIdle())
    {
        eReturnCode = STATUS_FTFC_FLS_IP_BUSY;
	 	Fls_Drv_eEraseStatus = STATUS_FTFC_FLS_IP_BUSY;
    }
    else
    {
        if(true==FLS_ADDRESS_VALID(StartAddress)) /* PRQA S 2995 */
        {
#if defined(FEATURE_EFM_HAS_ERASE_TIMING_UNION) && (FEATURE_EFM_HAS_ERASE_TIMING_UNION == 1U)
            EFM->TIMING2 = FEATURE_EFM_SECTOR_ERASE_RETRY_TIMING;
#endif
#ifdef EFM_CTRL_RETRY_CFG_MASK
            EFM->CTRL &= ~EFM_CTRL_RETRY_CFG_MASK;
            EFM->CTRL |= EFM_CTRL_RETRY_CFG(0x07);
#endif /* EFM_CTRL_RETRY_CFG_MASK */

#ifdef EFM_CTRL_CMD_VERIFY_EN_MASK
#ifdef FEATURE_EFM_ERASE_SECTOR_VREAD_RETRY_CMD_CODE
            EFM->CTRL &= ~EFM_CTRL_CMD_VERIFY_EN_MASK;
#else
            EFM->CTRL |= EFM_CTRL_CMD_VERIFY_EN_MASK;
#endif
#endif
            for (uint32_t i = 0; i < FLASH_RETRY_MAX_COUNT; i++)
            {
                INT_SYS_DisableIRQGlobal();
#ifndef EFM_CTRL_WE_MASK
                /* Add NOP command to clear write buffer */
                EFM->CMD = 0;
#endif
#if defined(FEATURE_EFM_HAS_READONLY_BIT) && (FEATURE_EFM_HAS_READONLY_BIT == 1)
                /* Flash can be read, erased and programmed */
                EFM->CTRL &= ~EFM_CTRL_READONLY_MASK;
#endif
                /* Clear all error flags */
                EFM->STS = FEATURE_EFM_CMD_ERROR_MASK | EFM_STS_DONE_MASK;
                /* parsing parameters to flash */
                EFM_ENABLE_WE_COMMAND(EFM);
                *(volatile uint32_t *)StartAddress = 0x01234567U;
#ifdef EFM_STS_ARRAY_SELECTED_MASK
                while (EFM_STS_ARRAY_SELECTED_MASK != (EFM->STS & EFM_STS_ARRAY_SELECTED_MASK))
                {
                    /* Wait until the array is selected */
                }
#endif /* EFM_STS_SET_ADDR_MASK */
#ifdef EFM_STS_SET_ADDR_MASK
                while (EFM_STS_SET_ADDR_MASK != (EFM->STS & EFM_STS_SET_ADDR_MASK))
                {
                    /* Wait until the address is set */
                }
#endif /* EFM_STS_SET_ADDR_MASK */
                EFM_DISABLE_WE_COMMAND(EFM);

                EFM_UNLOCK_CMD_REGISTER(EFM);
#if FEATURE_EFM_ERASE_SECTOR_RETRY_CMD_CODE == 0xFE
                eReturnCode = Fls_drvLoadCommandSync(FEATURE_EFM_ERASE_SECTOR_CMD_CODE);
#else
                eReturnCode = Fls_drvLoadCommandSync(FEATURE_EFM_ERASE_SECTOR_RETRY_CMD_CODE);
#endif
#if defined(FEATURE_EFM_HAS_READONLY_BIT) && (FEATURE_EFM_HAS_READONLY_BIT == 1)
                /* Read only flash array */
                EFM->CTRL |= EFM_CTRL_READONLY_MASK;
#endif
                INT_SYS_EnableIRQGlobal();
                if (eReturnCode == STATUS_FTFC_FLS_IP_SUCCESS)
                {
                    break;
                }
            }
            Fls_Drv_eEraseStatus = eReturnCode;
            if (eReturnCode == STATUS_FTFC_FLS_IP_SUCCESS)
            {
                Fls_eLLDJobResult = MEMIF_JOB_OK;
            }
            else
            {
                Fls_eLLDJobResult = MEMIF_JOB_FAILED;
            }
        }
        else
        {
            eReturnCode = STATUS_FTFC_FLS_IP_ERROR_INPUT_PARAM;
            Fls_Drv_eEraseStatus = STATUS_FTFC_FLS_IP_ERROR_INPUT_PARAM;
        }
    }
    
    return (eReturnCode);
}
#endif /* FEATURE_EFM_ERASE_SECTOR_RETRY_CMD_CODE */

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvSectorEraseStatus
 * Description   : Checks the status of flash erase.
 * 
 *END**************************************************************************/
static Fls_Drv_StatusType Fls_drvSectorEraseStatus(void)
{
    Fls_Drv_StatusType eReturnCode;

    /* Check if the erase job started successfully */
    if (STATUS_FTFC_FLS_IP_SUCCESS == Fls_Drv_eEraseStatus)
    {
        /* The job started successfully, polling the controller status */
        eReturnCode = Fls_drvGetCtrlStatus();
    }
    else
    {
        /* The erase job did not start successfully, return error immediately */
        eReturnCode = STATUS_FTFC_FLS_IP_ERROR;
    }

    return eReturnCode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvGetCtrlStatus
 * Description   : Read flash EFM_STS register.
 * 
 *END**************************************************************************/
static Fls_Drv_StatusType Fls_drvGetCtrlStatus(void)
{
    Fls_Drv_StatusType eRetVal;
    uint32_t errorFlags;

    /* Check if the flash controller is idle, by polling the CCIF status bit. */
    if (true == Fls_drvCheckIdle())
    {
        /* OK, memory controller is idle */

        /* Check if any hardware errors happened */
        errorFlags = EFM->STS & (FEATURE_EFM_CMD_ERROR_MASK);
        if (0U == errorFlags)
        {
            eRetVal = STATUS_FTFC_FLS_IP_SUCCESS;
        }
        else
        {
            /* clear the error flags */
            EFM->STS = FEATURE_EFM_CMD_ERROR_MASK;
            eRetVal = STATUS_FTFC_FLS_IP_ERROR;
        }
    }
    else
    {
        eRetVal = STATUS_FTFC_FLS_IP_BUSY;
    }
    return eRetVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvWriteDataPage
 * Description   : Flash page write data flash job process.
 * 
 *END**************************************************************************/
MemIf_JobResultType Fls_drvWriteDataPage(Fls_AddressType StartAddress, const uint8_t * SourceAddressPtr, uint32_t PageSize)
{
    EFM_ENABLE_WE_COMMAND(EFM);
    /* parsing parameters to flash */
    for (uint32_t i = 0U; i < PageSize; i += 4U){
        *(volatile uint32_t *)(StartAddress + i) = *(const uint32_t *)(&SourceAddressPtr[i]); /* PRQA S 3305,0310 */
#ifdef EFM_STS_ARRAY_SELECTED_MASK
        while (EFM_STS_ARRAY_SELECTED_MASK != (EFM->STS & EFM_STS_ARRAY_SELECTED_MASK))
        {
            /* Wait until the array is selected */
        }
#endif /* EFM_STS_SET_ADDR_MASK */
    }
#ifdef EFM_STS_SET_ADDR_MASK
    while (EFM_STS_SET_ADDR_MASK != (EFM->STS & EFM_STS_SET_ADDR_MASK))
    {
        /* Wait until the address is set */
    }
#endif /* EFM_STS_SET_ADDR_MASK */
    EFM_DISABLE_WE_COMMAND(EFM);
    /* parsing parameters to flash */
    return MEMIF_JOB_OK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvReadFlash
 * Description   : Flash read data flash job process.
 * 
 *END**************************************************************************/
Fls_Drv_StatusType Fls_drvReadFlash(Fls_AddressType StartAddress, Fls_LengthType Length, uint8_t *TargetAddressPtr)
{
    uint32_t readAddress = StartAddress;
    uint32_t desAddress = (uint32_t)TargetAddressPtr;
    uint8_t *desAddressPtr = TargetAddressPtr;
    uint32_t bytesRemain = Length;
    uint32_t readSize;

    /* Checking before reading */
    if ((NULL_PTR == TargetAddressPtr) || (0U == Length) || (false == FLS_ADDRESS_VALID(StartAddress)) || /* PRQA S 2995 */
        (false == FLS_ADDRESS_VALID(StartAddress + Length - 1U))) /* PRQA S 2995 */
    {
        Fls_Drv_eReadStatus = STATUS_FTFC_FLS_IP_ERROR_INPUT_PARAM;
    }
    else
    {
        Fls_Drv_eReadStatus = STATUS_FTFC_FLS_IP_SUCCESS;
    }

    if (STATUS_FTFC_FLS_IP_SUCCESS == Fls_Drv_eReadStatus)
    {
        /* Change to busy state */
        Fls_Drv_eReadStatus = STATUS_FTFC_FLS_IP_BUSY;
        do
        {
            /* Compute the maximum read size */
            readSize = Fls_drvComputeReadSize(readAddress, desAddress, bytesRemain);
#ifdef EFM_STS_CI_UNRECOVERR_MASK
            EFM->STS = EFM_STS_CI_UNRECOVERR_MASK;
#else
            EFM->STS = EFM_STS_UNRECOVERR_MASK;
#endif /* EFM_STS_CI_UNRECOVERR_MASK */
            switch (readSize)
            {
                case FLS_SIZE_4BYTE:
                    *((uint32_t *)desAddress) = *((uint32_t *)readAddress);
                    break;

                case FLS_SIZE_2BYTE:
                    *((uint16_t *)desAddress) = *((uint16_t *)readAddress);
                    break;

                case FLS_SIZE_1BYTE:
                    *desAddressPtr = *((uint8_t *)readAddress); /* Using uint8_t directly to avoid pointer casting */
                    break;

                default:
                    /* Do nothing */
                    break;
            }
#ifdef EFM_STS_CI_UNRECOVERR_MASK
            if (EFM_STS_CI_UNRECOVERR_MASK == (EFM->STS & EFM_STS_CI_UNRECOVERR_MASK))
            {
                EFM->STS = EFM_STS_CI_UNRECOVERR_MASK;
                break;
            }
#else
            if (EFM_STS_UNRECOVERR_MASK == (EFM->STS & EFM_STS_UNRECOVERR_MASK))
            {
                EFM->STS = EFM_STS_UNRECOVERR_MASK;
                break;
            }
#endif /* EFM_STS_CI_UNRECOVERR_MASK */
            /* Move to the next data */
            readAddress += readSize;
            desAddress += readSize;
            desAddressPtr = &desAddressPtr[readSize];
            bytesRemain -= readSize;
        } while (0U < bytesRemain);
        /* Checking if working was successful: the requested bytes were copied and no errors happend */
        if (0UL == bytesRemain)
        {
            /* Mark as success */
            Fls_Drv_eReadStatus = STATUS_FTFC_FLS_IP_SUCCESS;
        }
        else
        {
            /* Mark as error */
            Fls_Drv_eReadStatus = STATUS_FTFC_FLS_IP_ERROR;
        }
    }

    return Fls_Drv_eReadStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvCompareFlash
 * Description   : Flash data compare job process.
 * 
 *END**************************************************************************/
Fls_Drv_StatusType Fls_drvCompareFlash(Fls_AddressType StartAddress, Fls_LengthType Length, const uint8_t * SourceAddressPtr)
{
    uint32_t srcAddress = (uint32_t)StartAddress;

      /* Checking before comparing */
     if((0U == Length)||(FLS_ADDRESS_VALID(StartAddress))||(FLS_ADDRESS_VALID(StartAddress+Length-1U))) /* PRQA S 2995 */
     {
         Fls_Drv_eReadStatus = STATUS_FTFC_FLS_IP_ERROR_INPUT_PARAM;
     }
     else
     {
         Fls_Drv_eReadStatus = STATUS_FTFC_FLS_IP_SUCCESS;
     }
	 
     if(STATUS_FTFC_FLS_IP_SUCCESS==Fls_Drv_eReadStatus)
    {
        /* Change to busy state */
        Fls_Drv_eReadStatus = STATUS_FTFC_FLS_IP_BUSY;
	 /* Choose the type of comparison */
        if (NULL_PTR != SourceAddressPtr)
        {
            /* Verify the written data */
            Fls_drvCompareCheck(srcAddress, SourceAddressPtr, Length);
        }
        else
        {
            /* Blank check */
            Fls_drvBlankCheck(srcAddress, Length);
        }
    }
    
    return Fls_Drv_eReadStatus;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvCompareFlash
 * Description   : Verifies data written in serial flash.
 * 
 *END**************************************************************************/
static void Fls_drvCompareCheck(uint32_t address, const uint8_t * data, uint32_t size)
{
    uint32_t verifyAddress = address;
    uint32_t verifyData    = (uint32_t)data;
    uint32_t verifySize    = size;
    uint32_t readSize;
    bool compareStatus = true;

    /* Start compare operation */
    do
    {
        /* Compute the maximum read size */
        readSize = Fls_drvComputeReadSize(verifyAddress, verifyData, verifySize);

        switch (readSize)
        {
            case FLS_SIZE_4BYTE:
                if ((*((uint32_t *)verifyData)) != (*((uint32_t *)verifyAddress)))
                {
                    compareStatus = false;
                }
                break;

            case FLS_SIZE_2BYTE:
                if ((*((uint16_t *)verifyData)) != (*((uint16_t *)verifyAddress)))
                {
                    compareStatus = false;
                }
                break;

            case FLS_SIZE_1BYTE:
                if ((*((uint16_t *)verifyData)) != (*((uint16_t *)verifyAddress)))
                {
                    compareStatus = false;
                }
                break;

            default:
                /* Do nothing */
                break;
        }
	 if (false != compareStatus)
        {
                /* Move to next data */
	        verifyAddress += readSize;
	        verifyData    += readSize;
	        verifySize    -= readSize;
        }
    } while ((false !=  compareStatus) && (0U < verifySize));

    /* Checking if working is successful: the requested bytes were copied and no errors happend */
    if (0U == verifySize)
    {
        /* Mark as success */
        Fls_Drv_eReadStatus = STATUS_FTFC_FLS_IP_SUCCESS;
    }
    else
    {
    	Fls_Drv_eReadStatus = STATUS_FTFC_FLS_IP_ERROR_PROGRAM_VERIFY;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvBlankCheck
 * Description   : Flash blank check job process.
 * 
 *END**************************************************************************/
void Fls_drvBlankCheck(Fls_AddressType StartAddress, Fls_LengthType Length)
{
    uint32_t verifyAddress = StartAddress;
    uint32_t verifySize = Length;
    uint32_t readSize;
    bool compareStatus = true;

   /* Start compare operation */
    do
    {
        /* Compute the maximum read size */
        readSize = Fls_drvComputeReadSize(verifyAddress, 0UL, verifySize);

        switch (readSize)
        {
            case FLS_SIZE_4BYTE:
                if ((uint32_t)FLS_ERASED_VALUE != (*((uint32_t *)verifyAddress)))
                {
                    compareStatus = false;
                }
                break;

            case FLS_SIZE_2BYTE:
                if ((uint16_t)FLS_ERASED_VALUE != (*((uint16_t *)verifyAddress)))
                {
                    compareStatus = false;
                }
                break;

            case FLS_SIZE_1BYTE:
                if ((uint8_t)FLS_ERASED_VALUE != (*((uint8_t *)verifyAddress)))
                {
                    compareStatus = false;
                }
                break;

            default:
                /* Do nothing */
                break;
        }

        /* Move to next data */
	 if (false != compareStatus)
        {
            verifyAddress += readSize;
            verifySize    -= readSize;
	  }
    } while((false !=  compareStatus) && (0U < verifySize));

    /* Checking if working is successful: the requested bytes were copied and no errors happend */
    if (0U == verifySize)
    {
        /* Mark as success */
        Fls_Drv_eReadStatus = STATUS_FTFC_FLS_IP_SUCCESS;
    }
    else
    {
    	Fls_Drv_eReadStatus = STATUS_FTFC_FLS_IP_ERROR_PROGRAM_VERIFY;
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvMainFunction
 * Description   : Process ongoing erase or write hardware job.
 * 
 *END**************************************************************************/
void Fls_drvMainFunction( void )
{
    Fls_Drv_StatusType eReturnCode = STATUS_FTFC_FLS_IP_ERROR;
    if (MEMIF_JOB_PENDING == Fls_eLLDJobResult)
    {
        /* some hardware job (asynchronous) is pending */
        if (FLASH_JOB_ERASE == Fls_eLLDJob )
        {
            eReturnCode = Fls_drvSectorEraseStatus();
            if (STATUS_FTFC_FLS_IP_BUSY != eReturnCode)
            {
                /* Erase operation finished at EFM level - successfully or with errors  */
                if (STATUS_FTFC_FLS_IP_SUCCESS != eReturnCode)
                {
                    /* Sector lock was ok, but IP operation failed */
                    Fls_eLLDJobResult = MEMIF_JOB_FAILED;
		        }
                else
                {
                    /* Everything was ok */
                    Fls_eLLDJob = FLASH_JOB_NONE;
                    Fls_eLLDJobResult = MEMIF_JOB_OK;
                    if((Fls_gJobState.Fls_JobCurrentAddr>=(Fls_gConfigPtr->sectorList[Fls_gJobState.Fls_JobSectorIt].sectorStartAddress+\
                        Fls_gConfigPtr->sectorList[Fls_gJobState.Fls_JobSectorIt].sectorSize))&&\
                        (Fls_gJobState.Fls_JobCurrentAddr <=Fls_gJobState.Fls_JobEndAddr)) 
                    {
                        /* Move on to the next sector */
                        Fls_gJobState.Fls_JobSectorIt++;
                    }
                }
            }
        }
        /* Write hardware job (asynchronous) is pending */
        else if (FLASH_JOB_WRITE == Fls_eLLDJob )
        {
            eReturnCode = Fls_drvSectorWriteStatus();
            if (STATUS_FTFC_FLS_IP_BUSY != eReturnCode)
            {
                /* Erase operation finished at IP level - successfully or with errors or timed out */
                if (STATUS_FTFC_FLS_IP_SUCCESS != eReturnCode)
                {
                    /* Sector lock was ok, but IP operation failed */
                    Fls_eLLDJobResult = MEMIF_JOB_FAILED;
		            /* Report Runtime error when comparing is incorrect */
                    if (STATUS_FTFC_FLS_IP_ERROR_BLANK_CHECK == eReturnCode)
                    {

		     		}
                }
                else
                {
                    /* Everything was ok */
                    Fls_eLLDJob = FLASH_JOB_NONE;
                    Fls_eLLDJobResult = MEMIF_JOB_OK;
                   if ((Fls_gJobState.Fls_JobCurrentAddr <= Fls_gJobState.Fls_JobEndAddr)&&\
		                (Fls_gJobState.Fls_JobCurrentAddr >= (Fls_gConfigPtr->sectorList[Fls_gJobState.Fls_JobSectorIt].sectorStartAddress+\
						Fls_gConfigPtr->sectorList[Fls_gJobState.Fls_JobSectorIt].sectorSize)))
                    {
                        /* Move on to the next sector */
                        Fls_gJobState.Fls_JobSectorIt++;
                    }
                }
            }
            else
            {    }
        }
        else
        {
            /* Do nothing - should not happen in Fully Trusted Environment;
            'else' clause added to fulfil MISRA Rule 14.10 */
        }
    }   /* if (MEMIF_JOB_PENDING == Fls_eLLDJobResult) */
    else
    {
        /* no hardware job (asynchronous) is pending, do nothing */
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvSectorWrite
 * Description   : IP wrapper sector write function.
 * 
 *END**************************************************************************/
Fls_LLDReturnType Fls_drvSectorWrite(const Fls_AddressType u32DestAddr,
                                      const Fls_AddressType u32Length,
                                      const uint8_t *pJobDataSrcPtr,
                                      const bool bAsynch)
{
	Fls_LLDReturnType eLldRetVal = FLASH_E_FAILED;
	Fls_Drv_StatusType eReturnCode = STATUS_FTFC_FLS_IP_SUCCESS;
    uint8_t commandCode = FEATURE_EFM_PROGRAM_CMD_CODE;
    uint32_t pageSize = FEATURE_EFM_WRITE_UNIT_SIZE;

	 /*Check EFM Busy or not*/
    if(false ==Fls_drvCheckIdle())
    {
        eLldRetVal = FLASH_E_PENDING;
        eReturnCode = STATUS_FTFC_FLS_IP_BUSY;
    }
    else
    {
        if (true == FLS_ADDRESS_VALID_P_FLASH(u32DestAddr))
        {
            if(((u32DestAddr%pageSize) != 0U)||\
                (u32Length!=pageSize)||\
                (NULL_PTR==pJobDataSrcPtr)||\
                (!FLS_ADDRESS_VALID_P_FLASH(u32DestAddr+u32Length-1U)) /* PRQA S 2995 */ /* PRQA S 2996 */
            )
            {
                eReturnCode = STATUS_FTFC_FLS_IP_ERROR_INPUT_PARAM;
            }
            else
            {
                eReturnCode = STATUS_FTFC_FLS_IP_SUCCESS;
            }
        }
#if defined(FEATURE_EFM_HAS_DATA_FLASH) && (FEATURE_EFM_HAS_DATA_FLASH == 1)
        else if(true == FLS_ADDRESS_VALID_D_FLASH(u32DestAddr)) /* PRQA S 2995 */
        {
#ifdef FEATURE_EFM_PROGRAM_DATA_CMD_CODE
            commandCode = FEATURE_EFM_PROGRAM_DATA_CMD_CODE;
            pageSize = FEATURE_EFM_WRITE_DATA_UNIT_SIZE;
#endif /* FEATURE_EFM_PROGRAM_DATA_CMD_CODE */
            if(((u32DestAddr%pageSize) != 0U)||\
               (u32Length!=pageSize)||\
               (NULL_PTR==pJobDataSrcPtr)||\
               (!FLS_ADDRESS_VALID_D_FLASH(u32DestAddr+u32Length-1U)) /* PRQA S 2995 */ /* PRQA S 2996 */
            )
            {
                eReturnCode = STATUS_FTFC_FLS_IP_ERROR_INPUT_PARAM;
            }
            else
            {
                eReturnCode = STATUS_FTFC_FLS_IP_SUCCESS;
            }
        }
#endif /* FEATURE_EFM_HAS_DATA_FLASH */
        else
        {
            /* report error, not valid flash address */
            eReturnCode = STATUS_FTFC_FLS_IP_ERROR_INPUT_PARAM;
        }

        if(STATUS_FTFC_FLS_IP_SUCCESS==eReturnCode)
        {
            INT_SYS_DisableIRQGlobal();
#ifndef EFM_CTRL_WE_MASK
            /* Add NOP command to clear write buffer */
            EFM->CMD = 0;
#endif
#if defined(FEATURE_EFM_HAS_READONLY_BIT) && (FEATURE_EFM_HAS_READONLY_BIT == 1)
            /* Flash can be read, erased and programmed */
            EFM->CTRL &= ~EFM_CTRL_READONLY_MASK;
#endif
            /* Clear all error flags */
            EFM->STS =  FEATURE_EFM_CMD_ERROR_MASK | EFM_STS_DONE_MASK;
            (void)Fls_drvWriteDataPage(u32DestAddr, pJobDataSrcPtr, pageSize);

            /*CMD unlock*/
            EFM_UNLOCK_CMD_REGISTER(EFM);
            if(false==bAsynch)
            {
                eReturnCode = Fls_drvLoadCommandSync(commandCode);
                if (eReturnCode == STATUS_FTFC_FLS_IP_SUCCESS)
                {
                    Fls_eLLDJobResult  = MEMIF_JOB_OK;
                    eLldRetVal = FLASH_E_OK;
                }
                else
                {
                    Fls_eLLDJobResult  = MEMIF_JOB_FAILED;
                    eLldRetVal = FLASH_E_FAILED;
                }
            }
            else
            {
                EFM->CMD = commandCode;
                Fls_eLLDJob = FLASH_JOB_WRITE;
                Fls_eLLDJobResult = MEMIF_JOB_PENDING;
                eLldRetVal = FLASH_E_PENDING;
            }
#if defined(FEATURE_EFM_HAS_READONLY_BIT) && (FEATURE_EFM_HAS_READONLY_BIT == 1)
            /* Read only flash array */
            EFM->CTRL |= EFM_CTRL_READONLY_MASK;
#endif
			INT_SYS_EnableIRQGlobal();
        }
        else
        {
            Fls_eLLDJobResult = MEMIF_JOB_FAILED;
            eLldRetVal = FLASH_E_BLOCK_INCONSISTENT;
            eReturnCode = STATUS_FTFC_FLS_IP_ERROR_INPUT_PARAM;
        }
    }
	Fls_Drv_eWriteStatus = eReturnCode;
    return(eLldRetVal);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvSectorWriteStatus
 * Description   : Checks the status of the hardware program started by the 
 * Fls_drvMainFunction function.
 * 
 *END**************************************************************************/
static Fls_Drv_StatusType Fls_drvSectorWriteStatus(void)
{
	Fls_Drv_StatusType eReturnCode;

     /* Check if the write job started successfully */
    if (STATUS_FTFC_FLS_IP_SUCCESS == Fls_Drv_eWriteStatus)
    {
        /* The job started successfully, polling the controller status */
        eReturnCode = Fls_drvGetCtrlStatus();

        /* The job has done, perform the verification */
        if (STATUS_FTFC_FLS_IP_SUCCESS == eReturnCode)
        {
        }
	
    }
    else
    {
        /* The write job did not start successfully, return error immediately */
        eReturnCode = STATUS_FTFC_FLS_IP_ERROR;
    }
    return eReturnCode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvComputeReadSize
 * Description   : Compute the data size, considering the addresses alignment 
 * and the remaining bytes.
 * 
 *END**************************************************************************/
static uint32_t Fls_drvComputeReadSize(uint32_t srcAddress,
                                       uint32_t desAddress,
                                       uint32_t byteRemain
                                      )
{
    uint32_t readSize = FLS_SIZE_1BYTE;
    /* Combine two addresses for faster alignment checking */
    uint32_t combinedAddress = srcAddress | desAddress;

    /* Both the lowest two bits are zero: 4 bytes aligned */
    if (0UL == (combinedAddress & 0x03UL))
    {
        if (byteRemain >= FLS_SIZE_4BYTE)
        {
            /* 4 bytes operation */
            readSize = FLS_SIZE_4BYTE;
        }
        else if (byteRemain >= FLS_SIZE_2BYTE)
        {
            /* 2 bytes operation */
            readSize = FLS_SIZE_2BYTE;
        }
        else
        {
            /* 1 byte operation */
        }
    }
    /* Both the lowest one bit are zero: 2 bytes aligned */
    else if (0UL == (combinedAddress & 0x01UL))
    {
        if (byteRemain >= FLS_SIZE_2BYTE)
        {
            /* 2 bytes operation */
            readSize = FLS_SIZE_2BYTE;
        }
        else
        {
            /* 1 byte operation */
        }
    }
    else
    {
        /* 1 byte operation */
    }

    return readSize;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : Fls_drvCancel
 * Description   : Cancel the current job.
 * 
 *END**************************************************************************/
Fls_Drv_StatusType Fls_drvCancel(void)
{
    Fls_Drv_StatusType RetVal;

    /* Wait for current pending operation to finish, as there is no hardware way to abort it */
    while (false == Fls_drvCheckIdle())
    {
        /* wait until command is finished */
    }
    if (true == Fls_drvCheckIdle())
    {
        /* OK, memory idle */
        RetVal = STATUS_FTFC_FLS_IP_SUCCESS;
    }
    else
    {
        /*The integration can't convered, this branch can be accessed only when the EFM hardware fails*/
        /* error, memory controller not idle */
        RetVal = STATUS_FTFC_FLS_IP_ERROR;
    }
    return RetVal;
}

#ifdef __cplusplus
}
#endif

/** @} */
