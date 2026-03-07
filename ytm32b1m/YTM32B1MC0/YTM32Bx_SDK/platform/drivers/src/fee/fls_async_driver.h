/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file fls_async_driver.h
 * @version 1.4.0
 */

/*
 * @page misra_violations MISRA-C:2012 violations list
 * PRQA S 0779 Rule 5.2: Identifiers declared in the same scope and name space shall be distinct.
 *
 */

#ifndef fls_async_driver_H
#define fls_async_driver_H

#ifdef __cplusplus
extern "C"{
#endif
/*=================================================================================================
 *                                        INCLUDE FILES
=================================================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include "device_registers.h"

/*!
 * @addtogroup fls_async_driver
 * @ingroup fee
 * @{
 */

/*==================================================================================================
 *                                       DEFINES AND MACROS
==================================================================================================*/
/** @brief        The contents of an erased flash memory cell.*/
#define FLS_ERASED_VALUE                        (0xFFFFFFFFU)

/** @brief Valid P_FLASH address */
#if FEATURE_EFM_MAIN_ARRAY_START_ADDRESS == 0x00000000U 
    /* Warning[Pe186]: pointless comparison of unsigned integer with zero */
    #define FLS_ADDRESS_VALID_P_FLASH(addr)     ( (addr) <= (FEATURE_EFM_MAIN_ARRAY_END_ADDRESS) )
#else
    #define FLS_ADDRESS_VALID_P_FLASH(addr)     ( ((addr) >= (FEATURE_EFM_MAIN_ARRAY_START_ADDRESS)) && ((addr) <= (FEATURE_EFM_MAIN_ARRAY_END_ADDRESS)) )
#endif

#if defined(FEATURE_EFM_HAS_DATA_FLASH) && (FEATURE_EFM_HAS_DATA_FLASH == 1)
    /* @brief Valid D_FLASH address */
    #define FLS_ADDRESS_VALID_D_FLASH(addr)     ( ((addr) >= (FEATURE_EFM_DATA_ARRAY_START_ADDRESS)) && ((addr) <= (FEATURE_EFM_DATA_ARRAY_END_ADDRESS)) )

    /* @brief Valid P_FLASH or D_FLASH address */
    #define FLS_ADDRESS_VALID(addr)             ( FLS_ADDRESS_VALID_P_FLASH(addr) || FLS_ADDRESS_VALID_D_FLASH(addr) )
#else
    /* Since there is no dflash, only pflash can be used */
    /* @brief Valid P_FLASH or D_FLASH address */
    #define FLS_ADDRESS_VALID(addr)             FLS_ADDRESS_VALID_P_FLASH(addr)
#endif


/** @brief  OK define */
#define E_OK (0U)
/** @brief  NOT OK define */
#define E_NOT_OK (1U)
/** @brief  invalid sector define */
#define FLS_INVALID_SECTOR_INDEX              (0xFFFFU)

#ifndef NULL_PTR
/**
 * @brief NULL_PTR define
 */
   #define NULL_PTR  ((void *)0)                       
#endif

/**
 * @brief inline abstraction
 */
#define INLINE  inline
/**
 * @brief static inline abstraction
 */
#define LOCAL_INLINE static inline



typedef uint8_t Std_ReturnType;

typedef unsigned char StatusType;


/**
 * @brief     Momoery Interface Job Result Type      
 */
typedef enum {
    MEMIF_JOB_OK                             = 0, /*!< The job is successful */
    MEMIF_JOB_FAILED                         = 1, /*!< The job is failed */
    MEMIF_JOB_PENDING                        = 2, /*!< The job is in progress */
    MEMIF_JOB_CANCELED                       = 3, /*!< The job is canceled */
    MEMIF_BLOCK_INCONSISTENT                 = 4, /*!< The block is inconsistent */
    MEMIF_BLOCK_INVALID                      = 5, /*!< The block is invalid */
} MemIf_JobResultType;


/**
 * @brief  Memory operation mode type         
 */
typedef enum {
    MEMIF_MODE_SLOW                          = 0, /*!< The slow mode */
    MEMIF_MODE_FAST                          = 1, /*!< The fast mode */
} MemIf_ModeType;


/**
 * @brief    Memory Interface Status Type       
 */
typedef enum {
    MEMIF_UNINIT                             = 0, /*!< The module is uninit */
    MEMIF_IDLE                               = 1, /*!< The module is idle */
    MEMIF_BUSY                               = 2, /*!< The module is busy */
    MEMIF_BUSY_INTERNAL                      = 3, /*!< The module is busy internal */
} MemIf_StatusType;


/**
 * @brief  Flash erase operation mode type         
 */
typedef enum {
    ERASE_MODE_NORMAL                          = 0, /*!< The normal erase mode */
    ERASE_MODE_QUICK                           = 1, /*!< The quick erase mode */
} Fls_EraseModeType;


/**
 * @brief      Target address type of the flash device.     
 */
typedef uint32_t  Fls_AddressType;

/**
 * @brief     Flash length type of the flash device.        
 */
typedef uint32_t Fls_LengthType;

/**
 * @brief     Flash sector config type.        
 */
typedef struct
{
    uint32_t sectorId;   /*!< Number of continuous sectors */
    Fls_AddressType sectorStartAddress;  /*!< Start address of this sector. */
    Fls_LengthType sectorSize;  /*!< Size of this sector. */
    Fls_LengthType pageSize; /*!< Size of one page of this sector. */
    Fls_AddressType sectorHwStartAddress; /*!< Start hw address of one page of this sector. */
    bool asyncAccess; /*!< Access type of this sector. */
} Fls_SectorType;

/**
 * @brief      Fls Job End Notification Pointer Type      
 */
typedef void (*Fls_JobEndNotificationPtrType)(void);
/**
 * @brief      Fls Job Error Notification Pointer Type      
 */
typedef void (*Fls_JobErrorNotificationPtrType)(void);
/**
 * @brief      Fls Callback Pointer Type      
 */
typedef void (*Fls_CallbackType)(void);
/**
 * @brief      Fls Ac Erase Pointer Type      
 */
typedef void (*Fls_AcErasePtrType)(const EFM_Type* base, Fls_CallbackType callBack);
/**
 * @brief      Fls Ac Write Pointer Type      
 */
typedef void (*Fls_AcWritePtrType)(const EFM_Type* base, Fls_CallbackType callBack);
/**
* @brief          Type of job currently executed by Fls_MainFunction.
*/
typedef enum
{
    FLS_JOB_ERASE = 0,      /*!< Erase one or more complete flash sectors */
    FLS_JOB_WRITE,          /*!< Write one or more complete flash pages */
    FLS_JOB_READ,           /*!< Read one or more bytes from flash memory */
    FLS_JOB_COMPARE,        /*!< Compare data buffer with content of flash memory */
    FLS_JOB_BLANK_CHECK,    /*!< Check content of erased flash memory area */
    FLS_JOB_IDLE            /*!< Job idle */
} Fls_JobType;

/**
* @brief          Result of low-level flash operation.
*/
typedef enum
{
    FLASH_E_OK = 0,                 /*!< Operation succeeded */
    FLASH_E_FAILED,                 /*!< Operation failed due to hardware error */
    FLASH_E_BLOCK_INCONSISTENT,     /*!< Data buffer doesn't match with content of flash memory */
    FLASH_E_PENDING,                /*!< Operation is pending */
    FLASH_E_PARTITION_ERR           /*!< FlexNVM partition ratio error */
} Fls_LLDReturnType;

/**
* @brief          Type of job currently executed by Fls_LLDMainFunction.
*/
typedef enum
{
    FLASH_JOB_NONE = 0,                 /*!< No job executed by Fls_LLDMainFunction */
    FLASH_JOB_ERASE,                    /*!< Erase one flash sector */
    FLASH_JOB_ERASE_TEMP,               /*!< Complete erase and start an interleaved erase flash sector */
    FLASH_JOB_WRITE,                    /*!< Write one or more complete flash pages */
    FLASH_JOB_ERASE_BLANK_CHECK         /*!< Erase blank check of flash sector */
} Fls_LLDJobType;

typedef struct 
{
    MemIf_JobResultType Fls_JobResult;      /*!<  Job result type */
    Fls_AddressType Fls_JobCurrentAddr;     /*!<  Job current address */
    Fls_AddressType Fls_JobEndAddr;         /*!<  Job end address */
    Fls_JobType Fls_JobType;                /*!<  Job type */
    const uint8_t * Fls_JobWriteDataPtr;      /*!<  Job write data pointer */
    uint8_t * Fls_JobReadDataPtr;             /*!<  Job read data pointer */
    uint8_t Fls_JobStart;                     /*!<  Job start flag data>*/
    uint16_t Fls_JobSectorIt;                 /*!<  Index of flash sector currently processed by a job>*/
    uint16_t Fls_JobSectorEnd;                /*!<  Last logical address to be processed by a job>*/
} Fls_JobStateType;

/**
    @brief Enumeration of checking status errors or not.
*/
typedef enum
{
    STATUS_FTFC_FLS_IP_SUCCESS                   = 0x5AA5U,    /*!< Successful job */
    STATUS_FTFC_FLS_IP_BUSY                      = 0xE742U,    /*!< IP is performing an operation */
    STATUS_FTFC_FLS_IP_ERROR                     = 0x27E4U,    /*!< Error - general code */
    STATUS_FTFC_FLS_IP_ERROR_TIMEOUT             = 0x2BD4U,    /*!< Error - exceeded timeout */
    STATUS_FTFC_FLS_IP_ERROR_INPUT_PARAM         = 0x2DB4U,    /*!< Error - wrong input parameter */
    STATUS_FTFC_FLS_IP_ERROR_BLANK_CHECK         = 0x2E74U,    /*!< Error - selected memory area is not erased */
    STATUS_FTFC_FLS_IP_ERROR_PROGRAM_VERIFY      = 0x33CCU,    /*!< Error - selected memory area doesn't contain desired value */
    STATUS_FTFC_FLS_IP_ERROR_USER_TEST_BREAK_SBC = 0x35ACU,    /*!< Error - single bit correction */
    STATUS_FTFC_FLS_IP_ERROR_USER_TEST_BREAK_DBD = 0x366CU,    /*!< Error - double bit detection */ /*PRQA S 0779*/
    STATUS_FTFC_FLS_IP_SECTOR_UNPROTECTED        = 0xFA22U,    /*!< Checked sector is unlocked */
    STATUS_FTFC_FLS_IP_SECTOR_PROTECTED          = 0xE8B8U     /*!< Checked sector is locked */
} Fls_Drv_StatusType;


/* FlsConfigSet */

/**
 * @brief           Container for runtime configuration parameters of the flash driver. 
 */
typedef struct{
    Fls_AcErasePtrType acEraseFunPtr;    /*!< Address offset in RAM to which the erase flash access code shall be loaded. */
    Fls_AcWritePtrType acWriteFunPtr;   /*!< Address offset in RAM to which the write flash access code shall be loaded. */
    Fls_JobEndNotificationPtrType jobEndNotificationFunPtr; /*!< Mapped to the job end notification routine provided by some upper layer module, typically the Fee module. */
    Fls_JobErrorNotificationPtrType jobErrorNotificationFunPtr; /*!< Mapped to the job error notification routine provided by some upper layer module, typically the Fee module.  */
    MemIf_ModeType eDefaultMode;    /*!< Default FLS device mode after initialization (MEMIF_MODE_FAST, MEMIF_MODE_SLOW). */
    Fls_LengthType maxReadFastMode; /*!< The maximum number of bytes to read or compare in one cycle of the flash driver's job processing function in fast mode. */
    Fls_LengthType maxReadNormalMode;   /*!< The maximum number of bytes to read or compare in one cycle of the flash driver's job processing function in normal mode. */
    Fls_LengthType maxWriteFastMode;    /*!< The maximum number of bytes to write in one cycle of the flash driver's job processing function in fast mode. */
    Fls_LengthType maxWriteNormalMode;  /*!< The maximum number of bytes to write in one cycle of the flash driver's job processing function in normal mode. */
	uint16_t ConfigSectorNum;   /*!< The totle sectors number to be configure  */
    Fls_SectorType const * sectorList;  /*!< List of flashable sectors and pages. */
    Fls_EraseModeType eraseMode;  /*!< Erase mode, it's only used for synchronous operating mode. */
} Fls_ConfigType;

/*==================================================================================================
 *                                         EXTERNAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
 *                                         EXTERNAL VARIABLE
==================================================================================================*/
/**
    @brief Result of last flash hardware job
 */
extern volatile MemIf_JobResultType Fls_eLLDJobResult;         
/**
    @brief Type of current flash hardware job - used for asynchronous operating mode.
 */
extern Fls_LLDJobType Fls_eLLDJob;   

/**
 * @brief          Fls global job result status variable
 */
extern Fls_JobStateType Fls_gJobState;
/**
 * @brief          Fls global configuration pointer
 */
extern const Fls_ConfigType * Fls_gConfigPtr;

/*==================================================================================================
 *                                             ENUMS
==================================================================================================*/

/**
 * @brief            Initializes the Flash Driver.
 * @details          
 * @param[in]        ConfigPtr Pointer to flash driver configuration set.
 * @return           void
 */
void Fls_Init (const Fls_ConfigType * ConfigPtr);

/**
 * @brief            Erases flash sector(s).
 * @details          
 * @param[in]        TargetAddress Target address in flash memory. This address offset will be added to the flash memory base address.
                    Min.:  0
                    Max.: FLS_SIZE - 1
 * @param[in]        Length Number of bytes to erase
                    Min.:  1
                    Max.: FLS_SIZE - TargetAddress
 * @return           Std_ReturnType
 */
Std_ReturnType Fls_Erase (Fls_AddressType TargetAddress, Fls_LengthType Length);

/**
 * @brief            Writes one or more complete flash pages.
 * @details          
 * @param[in]        TargetAddress Target address in flash memory. This address offset will be added to the flash memory base address.
                    Min.:  0
                    Max.: FLS_SIZE - 1
 * @param[in]        SourceAddressPtr Pointer to source data buffer
 * @param[in]        Length Number of bytes to write
                    Min.:  1
                    Max.: FLS_SIZE - TargetAddress
 * @return           Std_ReturnType
 */
Std_ReturnType Fls_Write (Fls_AddressType TargetAddress, const uint8_t * SourceAddressPtr, Fls_LengthType Length);

/**
 * @brief            Cancels an ongoing job.
 * @details          
 * @return           void
 */
MemIf_StatusType Fls_GetStatus (void);

/**
 * @brief            Returns the result of the last job.
 * @details          
 * @return           MemIf_JobResultType
 */
MemIf_JobResultType Fls_GetJobResult (void);


/**
 * @brief            Performs the processing of jobs.
 * @details          
 * @return           void
 */
void Fls_MainFunction (void);

/**
 * @brief            Reads from flash memory.
 * @details          
 * @param[in]        SourceAddress Source address in flash memory. This address offset will be added to the flash memory base address.
                    Min.:  0
                    Max.: FLS_SIZE - 1
 * @param[out]       TargetAddressPtr Pointer to target data buffer
 * @param[in]        Length Number of bytes to read
                    Min.:  1
                    Max.: FLS_SIZE - SourceAddress
 * @return           Std_ReturnType
 */
Std_ReturnType Fls_Read (Fls_AddressType SourceAddress, uint8_t * TargetAddressPtr, Fls_LengthType Length);
/**
 * @brief            Compares the contents of an area of flash memory with that of an application data buffer.
 * @details          
 * @param[in]        SourceAddress Source address in flash memory. This address offset will be added to the flash memory base address.
                    Min.:  0
                    Max.: FLS_SIZE - 1
 * @param[in]        TargetAddressPtr Pointer to target data buffer
 * @param[in]        Length Number of bytes to compare
                    Min.:  1
                    Max.: FLS_SIZE - SourceAddress
 * @return           Std_ReturnType
 */
Std_ReturnType Fls_Compare (Fls_AddressType SourceAddress, const uint8_t * TargetAddressPtr, Fls_LengthType Length);

/**
 * @brief            Sets the flash driver's operation mode.
 * @details          
 * @param[in]        TargetAddress
 * @param[in]        Length
 * @return           Std_ReturnType
 */

Std_ReturnType Fls_BlankCheck (Fls_AddressType TargetAddress, Fls_LengthType Length);

/**
 * @brief            Cancels an ongoing job.
 * @details          
 * @return           void
 */
void Fls_Cancel(void);



#ifdef __cplusplus
}
#endif

#endif /* fls_async_driver_H */

/** @} */
