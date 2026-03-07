/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file flash_hw_access.h
 * @version 1.4.0
 */

#ifndef FLASH_ACCESS_H
#define FLASH_ACCESS_H


/*=================================================================================================
 *                                        INCLUDE FILES
=================================================================================================*/
#include "status.h"
#include "fls_async_driver.h"



/*==================================================================================================
 *                                       DEFINES AND MACROS
==================================================================================================*/
/**
* @brief the number of bytes uses to compare (1 byte).
*
*/
#define FLS_SIZE_1BYTE      1U

/**
* @brief the number of bytes uses to compare (2 bytes).
*
*/
#define FLS_SIZE_2BYTE      2U

/**
* @brief the number of bytes uses to compare (4 bytes).
*
*/
#define FLS_SIZE_4BYTE      4U

/*! @brief Max Count for Flash Erase Retry */
#define FLASH_RETRY_MAX_COUNT      (10U)
/*==================================================================================================
 *                                         EXTERNAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
 *                                             ENUMS
==================================================================================================*/
/*==================================================================================================
 *                                      GLOBAL VARIABLES
==================================================================================================*/



/*==================================================================================================
 *                                 PUBLIC API FUNCTIONS
==================================================================================================*/
#ifdef __cplusplus
extern "C"{
#endif

/**
 * @brief            FLS hardware initialization
 * @details          This function will initialize the FLS hardware 
 * @param[in]        void
 * @return           void
 */
void Fls_drvInit(void);

/**
 * @brief            Get flash hardware status
 * @details          This function will read flash hardware status
 * @param[in]        void
 * @return           void
 */
LOCAL_INLINE bool Fls_drvCheckIdle(void)
{
    return (EFM_STS_IDLE_MASK == (EFM->STS & EFM_STS_IDLE_MASK));
}

/**
 * @brief            Get flash hardware failure status
 * @details          This function will read flash hardware failure status
 * @param[in]        void
 * @return           bool - true: flash hardware failure, false: flash hardware normal
 */
LOCAL_INLINE bool Fls_drvCheckFail(void)
{
    return (0U != (EFM->STS & FEATURE_EFM_CMD_ERROR_MASK));
}

/**
 * @brief            Get flash erase job process
 * @details          This function will process the flash erase job
 * @param[in]        StartAddress - Start address of the flash memory to be erased
 * @param[in]        Async - Asynchronous or synchronous job
 * @return           Flash job result
 */
Fls_Drv_StatusType Fls_drvSectorErase(Fls_AddressType StartAddress, bool Async);

/**
 * @brief            Get flash erase quick job process
 * @details          This function will process the flash erase quick job, only for synchronous operation.
 * @param[in]        StartAddress - Start address of the flash memory to be erased
 * @return           Flash job result
 */
Fls_Drv_StatusType Fls_drvSectorEraseQuick(Fls_AddressType StartAddress);

/**
 * @brief            flash page write data flash job process
 * @details          This function will process the data flash write job
 * @param[in]        StartAddress - Start address of the flash memory to be written
 * @param[in]        SourceAddressPtr - Data address to be written
 * @param[in]        PageSize - Page size to be written
 * @return           Flash job result
 */
MemIf_JobResultType Fls_drvWriteDataPage(Fls_AddressType StartAddress, const uint8_t * SourceAddressPtr, uint32_t PageSize);

/**
 * @brief            flash read data flash job process
 * @details          This function will process the data flash read job
 * @param[in]        StartAddress - Start address of the flash memory to be read
 * @param[in]        Length - Data length to be read
 * @param[out]       TargetAddressPtr - Data address to be written
 * @return           Flash job result
 */
Fls_Drv_StatusType Fls_drvReadFlash(Fls_AddressType StartAddress, Fls_LengthType Length, uint8_t * TargetAddressPtr);

/**
 * @brief            flash data compare job process
 * @details          This function will process the data flash data compare job
 * @param[in]        StartAddress - Start address of the flash memory to be read
 * @param[in]        Length - Data length to be compared
 * @param[in]        SourceAddressPtr - Data pointer to be compared
 * @return           void
 */
Fls_Drv_StatusType Fls_drvCompareFlash(Fls_AddressType StartAddress, Fls_LengthType Length, const uint8_t * SourceAddressPtr);

/**
 * @brief            flash blank check job process
 * @details          This function will process the data flash blank check job
 * @param[in]        StartAddress - Start address of the flash memory to be read
 * @param[in]        Length - Data length to be checked
 * @return           void
 */
void Fls_drvBlankCheck(Fls_AddressType StartAddress, Fls_LengthType Length);
/**
* @brief          Process ongoing erase or write hardware job.
* @details        In case Async Operation is ongoing this function will complete the following job:
*                 - Erase
*                 - Erase on Interleaved sectors
*                 - Write
*                 - Erase blank Check
* @return         void
*/
void Fls_drvMainFunction( void );
/**
* @brief          IP wrapper sector write function.
* @details        Route the write job to appropriate low level IP function.
* @param[in]      u32DestAddr - Destination address where the data will be written.
* @param[in]      u32Length - Number of bytes to be written.
* @param[in]      pJobDataSrcPtr - Pointer to the source data buffer.
* @param[in]      bAsynch - TRUE: Asynchronous operation, FALSE: Synchronous operation.
* @return         Fls job result.
*/
Fls_LLDReturnType Fls_drvSectorWrite( const Fls_AddressType u32DestAddr, const Fls_AddressType u32Length, const uint8_t *pJobDataSrcPtr, const bool bAsynch);

/**
* @brief          IP cancel function, wait command done.
* @details        just wait previous command ends.
* @return         Fls job result.
*/
Fls_Drv_StatusType Fls_drvCancel(void);


#ifdef __cplusplus
}
#endif

#endif /* FLASH_ACCESS_H */

/** @} */

