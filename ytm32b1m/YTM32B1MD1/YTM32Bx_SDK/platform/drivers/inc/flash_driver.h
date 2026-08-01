/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file flash_driver.h
 * @version 1.4.1
 *
 * @brief Flash Driver — Public API for embedded flash memory operations.
 *
 * This header defines the application-level interface for the EFM (Embedded
 * Flash Module) peripheral. It provides APIs for erasing, programming, and
 * verifying flash memory contents, as well as managing NVR regions, boot
 * swap, and security key loading on supported devices.
 *
 * The APIs are organized into the following categories:
 *   - **Initialization & De-initialization**: Set up or tear down the flash driver.
 *   - **Configuration**: Adjust operating mode, interrupt, and verification settings.
 *   - **Erase Operations**: Array, block, and sector erase commands.
 *   - **Program Operations**: Write data to P-Flash and D-Flash regions.
 *   - **Data Integrity**: Checksum computation over flash ranges.
 *   - **Status Query**: Check driver busy state and command completion.
 *   - **Interrupt Management**: Enable/disable flash-related interrupts.
 *   - **NVR Operations**: Erase, program, and read NVR regions (conditional).
 *   - **Boot Swap**: Swap boot flash blocks (conditional).
 *   - **Security Key Operations**: Load AES/RSA keys for HCU (conditional).
 *
 * @note The flash module depends on the clock_manager. The clock manager must
 *       be initialized and the EFM clock must be configured before calling
 *       any function in this module.
 */

#ifndef FLASH_DRIVER_H
#define FLASH_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include "status.h"
#include "device_registers.h"

/*!
 * @addtogroup flash
 * @brief Embedded Flash Memory Peripheral Driver — Public API.
 * @details Provides instance-based APIs for flash memory erase, program, and
 *          verification operations. Supports synchronous and asynchronous
 *          (interrupt-driven) operation modes, NVR region access, boot block
 *          swap, and HCU security key loading on supported devices.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Maximum retry count for flash erase operations with retry logic. */
#define FLASH_RETRY_MAX_COUNT      (10U)

/*!
 * @brief Starting value for the EFM erase retry configuration field.
 *
 * When an erase error occurs in retry mode, the retry configuration value
 * automatically increments by 1 until it reaches 7. The minimum allowed
 * value is 4, but starting at 0x7 is recommended for maximum stability.
 */
#define FLASH_RETRY_CFG_START      (0x7U)

#ifndef FLASH_CALLBACK_CS
/*!
 * @brief Callback invocation period for FLASH_DRV_CheckSum().
 *
 * Defines the number of iterations between successive callback invocations
 * during checksum computation. A higher value improves performance at the
 * cost of reduced callback responsiveness. Other flash operations invoke
 * the callback as often as possible while waiting for the controller.
 */
#define FLASH_CALLBACK_CS 0x0AU
#endif

/*!
 * @name Null Callback Definitions
 * @brief Sentinel values representing unregistered callback function pointers.
 * @{
 */
/*! @brief Null asynchronous event callback pointer. */
#define NULL_CALLBACK          ((flash_callback_t)0x0U)
/*! @brief Null synchronous polling callback pointer. */
#define NULL_SYNCCALLBACK      ((flash_syncCallback_t)0x0U)
/*! @} */

/*!
 * @brief Flash event types reported through the callback function.
 *
 * These event codes are passed to the user-registered flash_callback_t
 * function to indicate the type of event that occurred.
 *
 * | Value                    | Description                                       |
 * |--------------------------|---------------------------------------------------|
 * | FLASH_EVENT_CHECKSUM     | Checksum callback period reached (periodic)       |
 * | FLASH_EVENT_COMPLETE     | Erase/program operation completed successfully    |
 * | FLASH_EVENT_ACCESS_ERROR | Read-collision error during command execution      |
 * | FLASH_EVENT_ERROR        | Unrecoverable error during erase/program operation|
 */
typedef enum
{
    FLASH_EVENT_CHECKSUM         = 0x01U,    /*!< Check sum reach the FLASH_CALLBACK_CS */
    FLASH_EVENT_COMPLETE         = 0x02U,    /*!< Erase/program operation completed */
    FLASH_EVENT_ACCESS_ERROR     = 0x03U,    /*!< An access error occured when command launch */
    FLASH_EVENT_ERROR            = 0x04U,    /*!< An error occured during erase/program operation */
} flash_event_t;

/*!
 * @brief Synchronous polling callback function pointer type.
 *
 * Called repeatedly during synchronous flash operations while waiting for
 * the command to complete. This allows the application to perform other
 * tasks (e.g., watchdog refresh) during potentially long erase operations.
 *
 * @warning Any code reachable from this function must NOT reside in a flash
 *          block targeted for the current program/erase operation. Place the
 *          callback in RAM using START/END_FUNCTION_DEFINITION_RAMSECTION macros.
 */
typedef void (*flash_syncCallback_t)(void);

/*!
 * @brief Asynchronous event callback function pointer type.
 *
 * Called from the flash interrupt handler (in async mode) or from the driver
 * (in sync mode) when a flash event occurs.
 *
 * @param[in] event  The flash event that triggered the callback.
 *
 * @warning Any code reachable from this function must NOT reside in a flash
 *          block targeted for the current program/erase operation.
 */
typedef void (*flash_callback_t)(flash_event_t event);

/*!
 * @brief Flash driver user configuration structure.
 *
 * Holds all parameters needed to initialize a flash driver instance.
 * Pass this structure to FLASH_DRV_Init() to configure the driver.
 *
 * | Field        | Type             | Description                                          | Default   |
 * |--------------|------------------|------------------------------------------------------|-----------|
 * | async        | bool             | true = asynchronous (interrupt-driven, RWW only)     | false     |
 * | disGlobalInt | bool             | true = disable global interrupts in sync mode        | true      |
 * | readVerify   | bool             | true = hardware read-verify after erase/program      | true      |
 * | wdgEnable    | bool             | true = enable EFM watchdog (device-specific)         | false     |
 * | callback     | flash_callback_t | User event callback (NULL_CALLBACK if not used)      | NULL      |
 */
typedef struct {
    bool async;                /*!< true: asynchronous (Only support for RWW), false: synchronous */
    bool disGlobalInt;         /*!< true: disable global interrupt during flash operation, only for sync mode */
    bool readVerify;           /*!< true: enable read verify after erase or program, false: disable read verify */
#ifdef EFM_WDG_WDG_EN_MASK
    bool wdgEnable;              /*!< true: enable WDG, false: disable WDG */
#endif /* EFM_WDG_WDG_EN_MASK */
    flash_callback_t callback; /*!< Callback function pointer */
} flash_user_config_t;

/*! @cond DRIVER_INTERNAL_USE_ONLY */
typedef struct{
    volatile uint32_t cmdCode;
    volatile uint32_t *dest;
    volatile const uint32_t *pdata;
    volatile uint32_t word_size;
} flash_cmd_handle_t;
/*! @endcond */

/*!
 * @brief Flash driver runtime state structure.
 *
 * Maintains the internal state of an active flash driver instance. Allocated
 * by the application and passed to FLASH_DRV_Init(). Must persist for the
 * lifetime of the driver instance.
 *
 * @note All fields are managed internally by the driver. The application
 *       should not modify any fields directly.
 */
typedef struct {
/*! @cond DRIVER_INTERNAL_USE_ONLY */
    bool async;                     /*!< true: asynchronous, false: synchronous */
    bool disGlobalInt;              /*!< true: disable global interrupt, only for sync mode */
    bool readVerify;                /*!< true: enable read verify after erase or program, false: disable read verify */
    flash_callback_t callback;      /*!< Callback function pointer */
    volatile bool driverBusy;       /*!< Driver is busy with an operation */
    flash_cmd_handle_t cmdParam;    /*!< Command parameter */
    volatile int32_t leftSize;      /*!< Left size of erase or program operation */
    volatile uint32_t retryCount;   /*!< Retry count for erase retry manual */
#ifdef FEATURE_EFM_PROGRAM_NVR_CMD_CODE
    volatile uint32_t nvr_addr;
    volatile uint32_t *nvr_data;
    volatile uint32_t efm_data[FEATURE_EFM_WRITE_UNIT_WORD_SIZE + 1U]; /*!< Internal NVR program buffer, avoids stack-dangling pointer */
#endif /* FEATURE_EFM_PROGRAM_NVR_CMD_CODE */
    volatile uint32_t shiftsum;     /*!< Shiftsum value to protect cmdParam and leftSize */
/*! @endcond */
} flash_state_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/
/*!
 * @name Initialization & De-initialization
 * @brief Functions for setting up and tearing down a flash driver instance.
 * @{
 */

/*!
 * @brief Initialize the flash driver with user-provided configuration.
 *
 * Configures the EFM peripheral with the specified operating mode, interrupt
 * settings, and verification options. In asynchronous mode, the command-complete
 * and read-collision interrupts are automatically enabled.
 *
 * @param[in] instance       Flash instance index (0-based). Must be less than
 *                           EFM_INSTANCE_COUNT.
 * @param[in] userConfigPtr  Pointer to the user configuration structure. Must
 *                           not be NULL.
 * @param[in] state          Pointer to the driver state structure. Must not be
 *                           NULL. Must persist for the lifetime of the instance.
 * @return Execution status.
 * @retval STATUS_SUCCESS    Initialization completed successfully.
 * @retval STATUS_ERROR      An error occurred during initialization.
 * @retval STATUS_EFM_BUSY   Flash controller is not idle.
 *
 * @pre  The EFM peripheral clock must be enabled via the clock_manager.
 * @post The flash driver is configured and ready for erase/program operations.
 */
status_t FLASH_DRV_Init(uint32_t instance, const flash_user_config_t * userConfigPtr, flash_state_t * state);

/*!
 * @brief De-initialize the flash driver and release resources.
 *
 * Disables flash interrupts (if in async mode), clears error and done flags,
 * and releases the driver state. After calling this function the instance may
 * be re-initialized with FLASH_DRV_Init().
 *
 * @param[in] instance  Flash instance index (0-based).
 * @return Execution status.
 * @retval STATUS_SUCCESS    De-initialization completed successfully.
 * @retval STATUS_EFM_BUSY   Flash controller is not idle; cannot de-initialize.
 *
 * @note This function does NOT disable the EFM peripheral clock.
 */
status_t FLASH_DRV_Deinit(uint32_t instance);

/*!
 * @brief Populate a configuration structure with default flash parameters.
 *
 * Fills @a userConfigPtr with safe default values:
 *   - async:        false (synchronous mode)
 *   - disGlobalInt: true (disable interrupts during sync operations)
 *   - readVerify:   true (hardware verification enabled)
 *   - callback:     NULL_CALLBACK (no callback)
 *
 * @param[out] userConfigPtr  Pointer to the configuration structure to fill.
 *                            Must not be NULL.
 */
void FLASH_DRV_GetDefaultConfig(flash_user_config_t * const userConfigPtr);

/*! @} */ /* End of Initialization & De-initialization */

/*******************************************************************************
 * Configuration
 ******************************************************************************/
/*!
 * @name Configuration
 * @brief Functions for adjusting flash driver runtime parameters.
 * @{
 */

/*!
 * @brief Switch the flash driver between synchronous and asynchronous mode.
 *
 * When switching to asynchronous mode, the command-complete and read-collision
 * interrupts are automatically enabled. The flash controller must be idle
 * before mode switching.
 *
 * @param[in] instance  Flash instance index (0-based).
 * @param[in] async     true = asynchronous (interrupt-driven) mode;
 *                      false = synchronous (blocking) mode.
 * @return Execution status.
 * @retval STATUS_SUCCESS    Mode switched successfully.
 * @retval STATUS_EFM_BUSY   Flash controller or driver is busy.
 *
 * @pre  FLASH_DRV_Init() must have been called for this instance.
 */
status_t FLASH_DRV_SetAsyncMode(uint32_t instance, bool async);

/*!
 * @brief Enable or disable global interrupt suppression in synchronous mode.
 *
 * When enabled, global interrupts are disabled during the entire flash
 * command execution in synchronous mode to prevent interference.
 *
 * @param[in] instance      Flash instance index (0-based).
 * @param[in] disGlobalInt  true = disable global interrupts during sync operations;
 *                          false = keep global interrupts enabled.
 *
 * @pre  FLASH_DRV_Init() must have been called for this instance.
 */
void FLASH_DRV_SetDisableGlobalInt(uint32_t instance, bool disGlobalInt);

/*!
 * @brief Enable or disable hardware read verification after erase/program.
 *
 * When enabled, the flash controller automatically verifies the data written
 * or erased by performing a read-back check. This increases reliability at
 * the cost of additional time per operation.
 *
 * @param[in] instance    Flash instance index (0-based).
 * @param[in] readVerify  true = enable read verification;
 *                        false = disable read verification.
 *
 * @pre  FLASH_DRV_Init() must have been called for this instance.
 */
void FLASH_DRV_SetReadVerify(uint32_t instance, bool readVerify);

/*! @} */ /* End of Configuration */

/*******************************************************************************
 * Erase Operations
 ******************************************************************************/
/*!
 * @name Erase Operations
 * @brief Functions for erasing flash memory at various granularity levels
 *        (array, block, sector). Supports both synchronous and asynchronous
 *        modes.
 *
 * @warning Erasing flash in synchronous mode with global interrupts disabled
 *          blocks the CPU for the duration of the erase operation.
 * @{
 */

#ifdef FEATURE_EFM_ERASE_ARRAY_CMD_CODE
/*!
 * @brief Erase an entire flash array.
 *
 * Erases all sectors within the specified flash array. The destination address
 * identifies which array to erase.
 *
 * @param[in] instance  Flash instance index (0-based).
 * @param[in] dest      Start address of the target flash array. Must be aligned
 *                      to the flash minimum sector size.
 * @return Execution status.
 * @retval STATUS_SUCCESS    Erase operation completed (sync) or started (async).
 * @retval STATUS_ERROR      An error occurred during the erase operation.
 * @retval STATUS_EFM_BUSY   Flash controller or driver is busy.
 *
 * @warning This operation erases ALL data in the target array. Ensure no
 *          critical data resides in the array before calling this function.
 */
status_t FLASH_DRV_EraseArray(uint32_t instance, uint32_t dest);
#endif /* FEATURE_EFM_ERASE_ARRAY_CMD_CODE */

/*!
 * @brief Erase a flash block.
 *
 * Erases all sectors within the specified flash block. The destination address
 * identifies which block to erase.
 *
 * @param[in] instance  Flash instance index (0-based).
 * @param[in] dest      Start address of the target flash block. Must be aligned
 *                      to the flash minimum sector size.
 * @return Execution status.
 * @retval STATUS_SUCCESS    Erase operation completed (sync) or started (async).
 * @retval STATUS_ERROR      An error occurred during the erase operation.
 * @retval STATUS_EFM_BUSY   Flash controller or driver is busy.
 *
 * @warning This operation erases ALL data in the target block.
 */
status_t FLASH_DRV_EraseBlock(uint32_t instance, uint32_t dest);

/*!
 * @brief Erase one or more flash sectors.
 *
 * Erases the specified number of bytes starting from @a dest in main flash
 * or data flash memory. In synchronous mode, the function blocks until all
 * sectors have been erased. In asynchronous mode, the erase operation runs
 * in the background and the callback is invoked upon completion.
 *
 * @param[in] instance  Flash instance index (0-based).
 * @param[in] dest      Start address of the first sector to erase. Must be
 *                      aligned to the sector size (FEATURE_EFM_MAIN_ARRAY_SECTOR_SIZE
 *                      or FEATURE_EFM_DATA_ARRAY_SECTOR_SIZE).
 * @param[in] size      Total number of bytes to erase. Must be a multiple of
 *                      the sector size and at least one sector.
 * @return Execution status.
 * @retval STATUS_SUCCESS                   Erase completed (sync) or started (async) successfully.
 * @retval STATUS_EFM_ADDRESS_OUT_OF_RANGE  Invalid erase range (dest/size crosses flash region boundaries).
 * @retval STATUS_ERROR                     Erase operation failed.
 * @retval STATUS_EFM_BUSY                  Flash controller or driver is busy.
 *
 * @note Returns STATUS_SUCCESS immediately if @a size is zero.
 */
status_t FLASH_DRV_EraseSector(uint32_t instance, uint32_t dest, uint32_t size);

/*!
 * @brief Quick-erase one or more flash sectors with automatic retry.
 *
 * Similar to FLASH_DRV_EraseSector() but uses split-time erase commands with
 * automatic retry logic for faster erase operations on supported devices.
 * If an erase fails, the operation retries up to FLASH_RETRY_MAX_COUNT times
 * with increasing retry configuration values.
 *
 * @param[in] instance  Flash instance index (0-based).
 * @param[in] dest      Start address of the first sector to erase. Must be
 *                      aligned to the sector size.
 * @param[in] size      Total number of bytes to erase. Must be a multiple of
 *                      the sector size and at least one sector.
 * @return Execution status.
 * @retval STATUS_SUCCESS                   Erase completed (sync) or started (async) successfully.
 * @retval STATUS_EFM_ADDRESS_OUT_OF_RANGE  Invalid erase range (dest/size crosses flash region boundaries).
 * @retval STATUS_ERROR                     Erase failed after maximum retry attempts.
 * @retval STATUS_EFM_BUSY                  Flash controller or driver is busy.
 *
 * @note Returns STATUS_SUCCESS immediately if @a size is zero.
 */
status_t FLASH_DRV_EraseSectorQuick(uint32_t instance, uint32_t dest, uint32_t size);

/*! @} */ /* End of Erase Operations */

/*******************************************************************************
 * Program Operations
 ******************************************************************************/
/*!
 * @name Program Operations
 * @brief Functions for writing data to flash memory regions.
 *
 * Data is written in units of FEATURE_EFM_WRITE_UNIT_SIZE bytes per flash
 * command. The driver handles splitting large buffers into multiple write
 * commands automatically.
 * @{
 */

/*!
 * @brief Program data to P-Flash or D-Flash memory.
 *
 * Writes @a size bytes from @a pData to flash starting at @a dest. In
 * synchronous mode, the function blocks until all data is written. In
 * asynchronous mode, the write runs in the background with callback notification.
 *
 * @param[in] instance  Flash instance index (0-based).
 * @param[in] dest      Destination address in flash. Must be aligned to
 *                      FEATURE_EFM_WRITE_UNIT_SIZE.
 * @param[in] size      Number of bytes to program. Must be a multiple of
 *                      FEATURE_EFM_WRITE_UNIT_SIZE and at least one unit.
 * @param[in] pData     Pointer to the source data buffer. Must be aligned to
 *                      a 32-bit boundary.
 * @return Execution status.
 * @retval STATUS_SUCCESS    Program completed (sync) or started (async) successfully.
 * @retval STATUS_ERROR      An error occurred during the program operation.
 * @retval STATUS_EFM_BUSY   Flash controller or driver is busy.
 *
 * @pre  The target flash area must be erased before programming.
 * @note Returns STATUS_SUCCESS immediately if @a size is zero.
 */
status_t FLASH_DRV_Program(uint32_t instance, uint32_t dest, uint32_t size, const void *pData);

#ifdef FEATURE_EFM_PROGRAM_DATA_CMD_CODE
/*!
 * @brief Program data to D-Flash (data flash) memory.
 *
 * Writes @a size bytes from @a pData to D-Flash starting at @a dest. This API
 * uses the data-flash-specific program command for devices with separate data
 * flash write units.
 *
 * @param[in] instance  Flash instance index (0-based).
 * @param[in] dest      Destination address in D-Flash. Must be aligned to
 *                      FEATURE_EFM_WRITE_DATA_UNIT_SIZE.
 * @param[in] size      Number of bytes to program. Must be a multiple of
 *                      FEATURE_EFM_WRITE_DATA_UNIT_SIZE and at least one unit.
 * @param[in] pData     Pointer to the source data buffer. Must be aligned to
 *                      a 32-bit boundary.
 * @return Execution status.
 * @retval STATUS_SUCCESS    Program completed (sync) or started (async) successfully.
 * @retval STATUS_ERROR      An error occurred during the program operation.
 * @retval STATUS_EFM_BUSY   Flash controller or driver is busy.
 *
 * @pre  The target D-Flash area must be erased before programming.
 * @note Returns STATUS_SUCCESS immediately if @a size is zero.
 */
status_t FLASH_DRV_ProgramDFlash(uint32_t instance, uint32_t dest, uint32_t size, const void *pData);
#endif /* FEATURE_EFM_PROGRAM_DATA_CMD_CODE */

/*! @} */ /* End of Program Operations */

/*******************************************************************************
 * Data Integrity
 ******************************************************************************/
/*!
 * @name Data Integrity
 * @brief Functions for verifying flash data integrity.
 * @{
 */

/*!
 * @brief Compute a 32-bit byte-sum checksum over a flash memory range.
 *
 * Performs a 32-bit sum of each byte over the specified address range without
 * carry, providing a rapid method for checking data integrity. The callback
 * is invoked periodically (every FLASH_CALLBACK_CS iterations) during
 * computation.
 *
 * @param[in]  instance  Flash instance index (0-based).
 * @param[in]  dest      Start address of the flash range to checksum.
 * @param[in]  size      Number of bytes to sum.
 * @param[out] pSum      Pointer to receive the computed checksum value.
 *                       Must not be NULL.
 * @return Execution status.
 * @retval STATUS_SUCCESS    Checksum computed successfully.
 * @retval STATUS_ERROR      An error occurred during computation.
 *
 * @note Returns STATUS_SUCCESS with *pSum = 0 if @a size is zero.
 */
status_t FLASH_DRV_CheckSum(uint32_t instance, uint32_t dest, uint32_t size, uint32_t *pSum);

/*! @} */ /* End of Data Integrity */

/*******************************************************************************
 * Status Query
 ******************************************************************************/
/*!
 * @name Status Query
 * @brief Functions for querying the flash driver and controller status.
 * @{
 */

/*!
 * @brief Check whether the flash driver is busy with an operation.
 *
 * Returns the software-level busy flag from the driver state. This indicates
 * whether an asynchronous erase or program operation is currently in progress.
 *
 * @param[in] instance  Flash instance index (0-based).
 * @return Busy status:
 *         - true:  Driver is busy with an active operation.
 *         - false: Driver is idle and ready for a new command.
 *
 * @pre  FLASH_DRV_Init() must have been called for this instance.
 */
bool FLASH_DRV_GetBusyStatus(uint32_t instance);

/*!
 * @brief Check whether the last flash command has completed.
 *
 * Reads the DONE status flag from the EFM hardware status register.
 *
 * @param[in] instance  Flash instance index (0-based).
 * @return Command completion state:
 *         - true:  Last command completed.
 *         - false: Command still in progress or no command issued.
 */
bool FLASH_DRV_GetDoneStatus(uint32_t instance);

/*!
 * @brief Check whether a read-collision error has occurred.
 *
 * Reads the ACCERR (access error) flag from the EFM hardware status register.
 * A read-collision occurs when reading from a flash block undergoing erase
 * or program.
 *
 * @param[in] instance  Flash instance index (0-based).
 * @return Read-collision state:
 *         - true:  Read-collision error detected.
 *         - false: No read-collision error.
 */
bool FLASH_DRV_GetReadCollisionFlag(uint32_t instance);

/*! @} */ /* End of Status Query */

/*******************************************************************************
 * Interrupt Management
 ******************************************************************************/
/*!
 * @name Interrupt Management
 * @brief Functions for enabling and disabling flash-related interrupt sources.
 *
 * The flash module supports several interrupt sources:
 * - **Command Complete**: Generated when an erase/program command finishes.
 * - **Read Collision**: Generated when a read-while-write conflict occurs.
 * - **Single-Bit ECC Fault**: Generated on recoverable flash read errors.
 * - **Double-Bit ECC Fault**: Generated on unrecoverable flash read errors.
 * @{
 */

/*!
 * @brief Enable the command-complete interrupt.
 *
 * When enabled, an interrupt is generated each time an EFM command completes.
 * This is required for asynchronous (interrupt-driven) operation mode.
 *
 * @param[in] instance  Flash instance index (0-based).
 */
void FLASH_DRV_EnableCmdCompleteInterrupt(uint32_t instance);

/*!
 * @brief Disable the command-complete interrupt.
 *
 * @param[in] instance  Flash instance index (0-based).
 */
void FLASH_DRV_DisableCmdCompleteInterrupt(uint32_t instance);

/*!
 * @brief Enable the read-collision error interrupt.
 *
 * When enabled, an interrupt is generated when a read-while-write collision
 * is detected on the flash bus.
 *
 * @param[in] instance  Flash instance index (0-based).
 */
void FLASH_DRV_EnableReadCollisionInterrupt(uint32_t instance);

/*!
 * @brief Disable the read-collision error interrupt.
 *
 * @param[in] instance  Flash instance index (0-based).
 */
void FLASH_DRV_DisableReadCollisionInterrupt(uint32_t instance);

/*!
 * @brief Enable the single-bit ECC fault interrupt.
 *
 * When enabled, an interrupt is generated when a recoverable (single-bit)
 * ECC error is detected during a flash read access.
 *
 * @param[in] instance  Flash instance index (0-based).
 */
void FLASH_DRV_EnableSingleBitFaultInterrupt(uint32_t instance);

/*!
 * @brief Disable the single-bit ECC fault interrupt.
 *
 * @param[in] instance  Flash instance index (0-based).
 */
void FLASH_DRV_DisableSingleBitFaultInterrupt(uint32_t instance);

/*!
 * @brief Enable the double-bit ECC fault interrupt.
 *
 * When enabled, an interrupt is generated when an unrecoverable (double-bit)
 * ECC error is detected during a flash read access.
 *
 * @param[in] instance  Flash instance index (0-based).
 */
void FLASH_DRV_EnableDoubleBitFaultInterrupt(uint32_t instance);

/*!
 * @brief Disable the double-bit ECC fault interrupt.
 *
 * @param[in] instance  Flash instance index (0-based).
 */
void FLASH_DRV_DisableDoubleBitFaultInterrupt(uint32_t instance);

/*! @} */ /* End of Interrupt Management */

/*******************************************************************************
 * NVR Operations
 ******************************************************************************/
#ifdef FEATURE_EFM_ERASE_NVR_CMD_CODE
/*!
 * @name NVR Operations
 * @brief Functions for accessing the Non-Volatile Region (NVR) of flash memory.
 *
 * NVR regions are used for storing customer-specific data such as calibration
 * parameters, serial numbers, or encryption keys. NVR access requires
 * explicit unlock before erase/program operations.
 *
 * @warning NVR operations are single-sector only for safety. Multi-sector
 *          NVR erase/program is not supported.
 * @{
 */

/*!
 * @brief Erase a single NVR sector.
 *
 * Erases one sector in the NVR region. Multi-sector erase is not supported
 * for safety reasons.
 *
 * @param[in] instance  Flash instance index (0-based).
 * @param[in] dest      Start address of the target NVR sector. Must be aligned
 *                      to the NVR sector size.
 * @return Execution status.
 * @retval STATUS_SUCCESS    Erase completed (sync) or started (async) successfully.
 * @retval STATUS_ERROR      An error occurred during the erase operation.
 * @retval STATUS_EFM_BUSY   Flash controller or driver is busy.
 *
 * @pre  The NVR sector must be unlocked before calling this function.
 */
status_t FLASH_DRV_EraseNVR(uint32_t instance, uint32_t dest);

/*!
 * @brief Program data to an NVR sector.
 *
 * Programs data from @a pData to the NVR region starting at @a dest.
 * Can be used to program AES keys, customer configuration data, etc.
 * Multi-sector program is not supported for safety reasons.
 *
 * @param[in] instance  Flash instance index (0-based).
 * @param[in] dest      Start address in the NVR region. Must be aligned to
 *                      FEATURE_EFM_WRITE_UNIT_SIZE.
 * @param[in] size      Number of bytes to program. Must be a multiple of
 *                      FEATURE_EFM_WRITE_UNIT_SIZE.
 * @param[in] pData     Pointer to the source data buffer. Must be aligned to
 *                      a 32-bit boundary.
 * @return Execution status.
 * @retval STATUS_SUCCESS    Program completed (sync) or started (async) successfully.
 * @retval STATUS_ERROR      An error occurred during the program operation.
 * @retval STATUS_EFM_BUSY   Flash controller or driver is busy.
 *
 * @pre  The NVR sector must be unlocked before calling this function.
 */
status_t FLASH_DRV_ProgramNVR(uint32_t instance, uint32_t dest, uint32_t size, const void *pData);

/*!
 * @brief Read data from an NVR region.
 *
 * Reads data from the NVR region into @a dest. AES key regions are not
 * readable for security reasons.
 *
 * @param[in]  instance  Flash instance index (0-based).
 * @param[in]  address   Start address in the NVR region. Must be aligned to
 *                       FEATURE_EFM_WRITE_UNIT_SIZE.
 * @param[in]  size      Number of bytes to read. Must be a multiple of
 *                       FEATURE_EFM_WRITE_UNIT_SIZE and at least one unit.
 * @param[out] dest      Pointer to the destination buffer. Must be aligned to
 *                       a 32-bit boundary.
 * @return Execution status.
 * @retval STATUS_SUCCESS    Read completed successfully.
 * @retval STATUS_ERROR      An error occurred during the read operation.
 * @retval STATUS_EFM_BUSY   Flash controller is not idle.
 *
 * @pre  The NVR sector must be unlocked before calling this function.
 * @note This function always operates synchronously, even if the driver is
 *       configured for async mode.
 */
status_t FLASH_DRV_ReadNVR(uint32_t instance, uint32_t address, uint32_t size, void *dest);

/*! @} */ /* End of NVR Operations */
#endif /* FEATURE_EFM_ERASE_NVR_CMD_CODE */

/*******************************************************************************
 * Boot Swap
 ******************************************************************************/
#ifdef FEATURE_EFM_BOOT_SWAP_CMD_CODE
/*!
 * @name Boot Swap
 * @brief Functions for swapping the active boot flash block.
 *
 * Boot swap exchanges the primary and secondary flash blocks so that the
 * MCU boots from the alternate block on the next reset. This is useful for
 * implementing fail-safe firmware update mechanisms.
 * @{
 */

/*!
 * @brief Swap the active boot flash block.
 *
 * Exchanges the primary and secondary boot flash blocks. The swap takes
 * effect on the next system reset. This function always operates synchronously.
 *
 * @param[in] instance  Flash instance index (0-based).
 * @return Execution status.
 * @retval STATUS_SUCCESS    Boot swap command executed successfully.
 * @retval STATUS_ERROR      An error occurred during the swap operation.
 * @retval STATUS_EFM_BUSY   Flash controller or driver is busy.
 *
 * @note The boot swap status can be read from the flash registers after reset.
 * @note This function temporarily switches to synchronous mode internally.
 */
status_t FLASH_DRV_BootSwap(uint32_t instance);

/*! @} */ /* End of Boot Swap */
#endif

/*******************************************************************************
 * Security Key Operations
 ******************************************************************************/
#ifdef FEATURE_EFM_LOAD_AES_KEY_CMD_CODE
/*!
 * @name Security Key Operations
 * @brief Functions for loading cryptographic keys into the HCU (Hardware
 *        Crypto Unit) from flash NVR storage.
 * @{
 */

/*!
 * @brief Load an AES key from NVR into the HCU.
 *
 * Reads an AES key from the specified NVR address and loads it into the
 * hardware crypto unit. The operation is performed synchronously.
 *
 * @param[in] instance  Flash instance index (0-based).
 * @param[in] address   NVR address of the AES key. Must be aligned to
 *                      FEATURE_EFM_AES_KEY_SIZE bytes.
 * @return Execution status.
 * @retval STATUS_SUCCESS    Key loaded successfully.
 * @retval STATUS_ERROR      Invalid address alignment.
 * @retval STATUS_EFM_BUSY   Flash controller is not idle.
 *
 * @note This function temporarily switches to synchronous mode internally.
 */
status_t FLASH_DRV_LoadAESKey(uint32_t instance, uint32_t address);
#endif /* FEATURE_EFM_LOAD_AES_KEY_CMD_CODE */

#ifdef FEATURE_EFM_LOAD_RSA_KEY_CMD_CODE
/*!
 * @brief Load an RSA key from NVR into the HCU.
 *
 * Reads an RSA key from the specified NVR address and loads it into the
 * hardware crypto unit. The key length is specified in units of 32-bit words.
 *
 * @param[in] instance  Flash instance index (0-based).
 * @param[in] address   NVR address of the RSA key. Must be aligned to
 *                      FEATURE_EFM_RSA_KEY_SIZE bytes.
 * @param[in] keyLen    RSA key length in 32-bit words minus 1 (i.e., actual
 *                      key is (keyLen + 1) * 32 bits).
 * @return Execution status.
 * @retval STATUS_SUCCESS    Key loaded successfully.
 * @retval STATUS_ERROR      Invalid address alignment.
 * @retval STATUS_EFM_BUSY   Flash controller is not idle.
 *
 * @note This function temporarily switches to synchronous mode internally.
 */
status_t FLASH_DRV_LoadRSAKey(uint32_t instance, uint32_t address, uint8_t keyLen);
#endif /* FEATURE_EFM_LOAD_RSA_KEY_CMD_CODE */

#if defined(FEATURE_EFM_LOAD_AES_KEY_CMD_CODE) || defined(FEATURE_EFM_LOAD_RSA_KEY_CMD_CODE)
/*! @} */ /* End of Security Key Operations */
#endif

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of flash group */

#endif /* FLASH_DRIVER_H */
