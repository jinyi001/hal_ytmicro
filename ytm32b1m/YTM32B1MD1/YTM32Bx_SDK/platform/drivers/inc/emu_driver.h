/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file emu_driver.h
 * @version 1.4.1
 *
 * @brief EMU Driver - Public API for ECC error injection and reporting.
 *
 * This header defines the application-facing interface for the ECC Management
 * Unit (EMU). The driver configures per-channel error injection targets,
 * controls optional interrupt generation, and exposes status APIs for
 * retrieving report counters, report addresses, and syndrome-derived check
 * bit information.
 *
 * The APIs are organized into the following categories:
 *   - Initialization & De-initialization
 *   - Error Injection Control
 *   - Error Report & Counter Access
 *   - Interrupt Status & Control
 *
 * @note Enable the EMU peripheral clock before calling EMU_DRV_Init().
 * @warning EMU validation is intended for supervisor-mode 32-bit SRAM
 *          accesses.
 */

#ifndef EMU_DRIVER_H
#define EMU_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include "status.h"
#include "device_registers.h"
#include "emu_hw_access.h"

/*!
 * @addtogroup emu
 * @brief ECC Management Unit peripheral driver - public API.
 * @details Provides instance-based APIs for configuring per-channel SRAM ECC
 *          error injection, reading error reports, and managing interrupt
 *          status for single-bit correction and double-bit detection flows.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Data-bit selectors for injected ECC test errors.
 *
 * Selects which SRAM data bit is inverted when the injection type targets the
 * data path. Devices without `FEATURE_EMU_SUPPORT_64BIT_INJECTION` support
 * `EMU_DATABIT_0` through `EMU_DATABIT_31`. Supported 64-bit devices extend
 * the range to `EMU_DATABIT_63`.
 *
 * @note `EMU_DATABIT_MAX` is a sentinel upper bound and is not a selectable
 *       injection bit.
 */
typedef enum
{
    EMU_DATABIT_0,          /*!< Inject the ECC test error into data bit 0. */
    EMU_DATABIT_1,          /*!< Inject the ECC test error into data bit 1. */
    EMU_DATABIT_2,          /*!< Inject the ECC test error into data bit 2. */
    EMU_DATABIT_3,          /*!< Inject the ECC test error into data bit 3. */
    EMU_DATABIT_4,          /*!< Inject the ECC test error into data bit 4. */
    EMU_DATABIT_5,          /*!< Inject the ECC test error into data bit 5. */
    EMU_DATABIT_6,          /*!< Inject the ECC test error into data bit 6. */
    EMU_DATABIT_7,          /*!< Inject the ECC test error into data bit 7. */
    EMU_DATABIT_8,          /*!< Inject the ECC test error into data bit 8. */
    EMU_DATABIT_9,          /*!< Inject the ECC test error into data bit 9. */
    EMU_DATABIT_10,         /*!< Inject the ECC test error into data bit 10. */
    EMU_DATABIT_11,         /*!< Inject the ECC test error into data bit 11. */
    EMU_DATABIT_12,         /*!< Inject the ECC test error into data bit 12. */
    EMU_DATABIT_13,         /*!< Inject the ECC test error into data bit 13. */
    EMU_DATABIT_14,         /*!< Inject the ECC test error into data bit 14. */
    EMU_DATABIT_15,         /*!< Inject the ECC test error into data bit 15. */
    EMU_DATABIT_16,         /*!< Inject the ECC test error into data bit 16. */
    EMU_DATABIT_17,         /*!< Inject the ECC test error into data bit 17. */
    EMU_DATABIT_18,         /*!< Inject the ECC test error into data bit 18. */
    EMU_DATABIT_19,         /*!< Inject the ECC test error into data bit 19. */
    EMU_DATABIT_20,         /*!< Inject the ECC test error into data bit 20. */
    EMU_DATABIT_21,         /*!< Inject the ECC test error into data bit 21. */
    EMU_DATABIT_22,         /*!< Inject the ECC test error into data bit 22. */
    EMU_DATABIT_23,         /*!< Inject the ECC test error into data bit 23. */
    EMU_DATABIT_24,         /*!< Inject the ECC test error into data bit 24. */
    EMU_DATABIT_25,         /*!< Inject the ECC test error into data bit 25. */
    EMU_DATABIT_26,         /*!< Inject the ECC test error into data bit 26. */
    EMU_DATABIT_27,         /*!< Inject the ECC test error into data bit 27. */
    EMU_DATABIT_28,         /*!< Inject the ECC test error into data bit 28. */
    EMU_DATABIT_29,         /*!< Inject the ECC test error into data bit 29. */
    EMU_DATABIT_30,         /*!< Inject the ECC test error into data bit 30. */
    EMU_DATABIT_31,         /*!< Inject the ECC test error into data bit 31. */
#if defined(FEATURE_EMU_SUPPORT_64BIT_INJECTION) && (FEATURE_EMU_SUPPORT_64BIT_INJECTION == 1U)
    EMU_DATABIT_32,         /*!< Inject the ECC test error into data bit 32. */
    EMU_DATABIT_33,         /*!< Inject the ECC test error into data bit 33. */
    EMU_DATABIT_34,         /*!< Inject the ECC test error into data bit 34. */
    EMU_DATABIT_35,         /*!< Inject the ECC test error into data bit 35. */
    EMU_DATABIT_36,         /*!< Inject the ECC test error into data bit 36. */
    EMU_DATABIT_37,         /*!< Inject the ECC test error into data bit 37. */
    EMU_DATABIT_38,         /*!< Inject the ECC test error into data bit 38. */
    EMU_DATABIT_39,         /*!< Inject the ECC test error into data bit 39. */
    EMU_DATABIT_40,         /*!< Inject the ECC test error into data bit 40. */
    EMU_DATABIT_41,         /*!< Inject the ECC test error into data bit 41. */
    EMU_DATABIT_42,         /*!< Inject the ECC test error into data bit 42. */
    EMU_DATABIT_43,         /*!< Inject the ECC test error into data bit 43. */
    EMU_DATABIT_44,         /*!< Inject the ECC test error into data bit 44. */
    EMU_DATABIT_45,         /*!< Inject the ECC test error into data bit 45. */
    EMU_DATABIT_46,         /*!< Inject the ECC test error into data bit 46. */
    EMU_DATABIT_47,         /*!< Inject the ECC test error into data bit 47. */
    EMU_DATABIT_48,         /*!< Inject the ECC test error into data bit 48. */
    EMU_DATABIT_49,         /*!< Inject the ECC test error into data bit 49. */
    EMU_DATABIT_50,         /*!< Inject the ECC test error into data bit 50. */
    EMU_DATABIT_51,         /*!< Inject the ECC test error into data bit 51. */
    EMU_DATABIT_52,         /*!< Inject the ECC test error into data bit 52. */
    EMU_DATABIT_53,         /*!< Inject the ECC test error into data bit 53. */
    EMU_DATABIT_54,         /*!< Inject the ECC test error into data bit 54. */
    EMU_DATABIT_55,         /*!< Inject the ECC test error into data bit 55. */
    EMU_DATABIT_56,         /*!< Inject the ECC test error into data bit 56. */
    EMU_DATABIT_57,         /*!< Inject the ECC test error into data bit 57. */
    EMU_DATABIT_58,         /*!< Inject the ECC test error into data bit 58. */
    EMU_DATABIT_59,         /*!< Inject the ECC test error into data bit 59. */
    EMU_DATABIT_60,         /*!< Inject the ECC test error into data bit 60. */
    EMU_DATABIT_61,         /*!< Inject the ECC test error into data bit 61. */
    EMU_DATABIT_62,         /*!< Inject the ECC test error into data bit 62. */
    EMU_DATABIT_63,         /*!< Inject the ECC test error into data bit 63. */
    EMU_DATABIT_MAX = 64,   /*!< Sentinel upper bound for supported data-bit indices. */
#else
    EMU_DATABIT_MAX = 32,   /*!< Sentinel upper bound for supported data-bit indices. */
#endif
} e_emu_databit;

/*!
 * @brief Check-bit selectors for injected ECC test errors.
 *
 * Selects which ECC check bit is inverted when the injection type targets the
 * check-bit path.
 *
 * @note `EMU_CHKBIT_MAX` is a sentinel upper bound and is not a selectable
 *       injection bit.
 */
typedef enum
{
    EMU_CHKBIT_0,           /*!< Inject the ECC test error into check bit 0. */
    EMU_CHKBIT_1,           /*!< Inject the ECC test error into check bit 1. */
    EMU_CHKBIT_2,           /*!< Inject the ECC test error into check bit 2. */
    EMU_CHKBIT_3,           /*!< Inject the ECC test error into check bit 3. */
    EMU_CHKBIT_4,           /*!< Inject the ECC test error into check bit 4. */
    EMU_CHKBIT_5,           /*!< Inject the ECC test error into check bit 5. */
    EMU_CHKBIT_6,           /*!< Inject the ECC test error into check bit 6. */
    EMU_CHKBIT_MAX,         /*!< Sentinel upper bound for supported check-bit indices. */
} e_emu_chkbit;

/*!
 * @brief Injection target selection.
 *
 * Selects whether EMU injects an error on the SRAM data path, on the ECC
 * check-bit path, or on both paths at the same time.
 *
 * | Value | Description |
 * |-------|-------------|
 * | EMU_INJECT_BITS_TYPE_DATA | Inject one data-path bit error. |
 * | EMU_INJECT_BITS_TYPE_CHK | Inject one check-bit-path error. |
 * | EMU_INJECT_BITS_TYPE_MIXTURE | Inject one data-path bit and one check-bit error together. |
 */
typedef enum
{
    EMU_INJECT_BITS_TYPE_DATA,      /*!< Inject one error bit on the data path. */
    EMU_INJECT_BITS_TYPE_CHK,       /*!< Inject one error bit on the check-bit path. */
    EMU_INJECT_BITS_TYPE_MIXTURE,   /*!< Inject one data bit and one check bit together. */
} emu_inject_bits_type;

/*!
 * @brief Single-bit injection selection for one EMU channel.
 *
 * Describes which data bit and/or check bit will be programmed by the driver
 * when the application initializes one channel with EMU_DRV_Init().
 *
 * | Field | Type | Description |
 * |-------|------|-------------|
 * | injectBitsType | emu_inject_bits_type | Selects the active injection path or paths. |
 * | dataBit | e_emu_databit | Data-bit index used when the data path participates in the injection. |
 * | chkBit | e_emu_chkbit | Check-bit index used when the check-bit path participates in the injection. |
 */
typedef struct
{
    emu_inject_bits_type injectBitsType; /*!< Injection path selection. */
    e_emu_databit dataBit;               /*!< Data-bit index used for data-path injection. */
    e_emu_chkbit chkBit;                 /*!< Check-bit index used for check-bit-path injection. */
} emu_inject_err_t;

/*!
 * @brief EMU channel configuration.
 *
 * Holds the parameters applied by EMU_DRV_Init() for one channel of one EMU
 * instance.
 *
 * | Field | Type | Description |
 * |-------|------|-------------|
 * | channel | uint32_t | EMU channel index to configure. |
 * | injectErrAddr | uint32_t | SRAM word address monitored for error injection. |
 * | injectErrSetInfo | emu_inject_err_t | Single-bit or mixed-bit injection selection. |
 * | interruptEnable | bool | Enables both signal-bit and double-bit interrupts during initialization when `true`. |
 */
typedef struct
{
    uint32_t channel;                   /*!< Channel index for the target SRAM bank. */
    uint32_t injectErrAddr;             /*!< SRAM word address used as the injection target. */
    emu_inject_err_t injectErrSetInfo;  /*!< Injection-path and bit-selection parameters. */
    bool interruptEnable;               /*!< Enable both signal-bit and double-bit interrupts during init. */
} emu_config_t;

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/
/*!
 * @name Initialization & De-initialization
 * @brief Functions for resetting, configuring, and shutting down an EMU
 *        instance.
 * @{
 */

/*!
 * @brief Reset an EMU instance, apply one channel configuration, and control
 *        the initial enable state.
 *
 * This function restores the entire EMU instance to reset state, applies the
 * channel configuration from @a configPtr, and then enables the selected
 * channel only when @a startFlag is `1U`.
 *
 * @param[in] instance   EMU instance index. Must be less than
 *                       `EMU_INSTANCE_COUNT`.
 * @param[in] configPtr  Pointer to the user configuration structure.
 *                       Must not be NULL.
 * @param[in] startFlag  Pass `1U` to enable the configured channel
 *                       immediately. Any other value leaves the channel
 *                       disabled after configuration.
 *
 * @pre The EMU peripheral clock must already be enabled.
 * @post The complete EMU instance has been reset and the selected channel has
 *       been configured.
 * @warning Re-initializing one channel clears the configuration and report
 *          state of every other channel in the same EMU instance.
 */
void EMU_DRV_Init(uint32_t instance, const emu_config_t *configPtr, uint32_t startFlag);

/*!
 * @brief Disable the selected channel and restore the EMU instance to reset
 *        state.
 *
 * @param[in] instance  EMU instance index. Must be less than
 *                      `EMU_INSTANCE_COUNT`.
 * @param[in] channel   Channel index used for the initial disable operation.
 *
 * @post The entire EMU instance is reset to its default register state.
 */
void EMU_DRV_DeInit(uint32_t instance, uint32_t channel);

/*! @} */ /* End of Initialization & De-initialization */

/*******************************************************************************
 * Error Injection Control
 ******************************************************************************/
/*!
 * @name Error Injection Control
 * @brief Functions for programming EMU injection settings after initialization.
 * @{
 */

/*!
 * @brief Program raw multi-bit injection masks for one channel and re-enable
 *        EMU injection.
 *
 * This helper disables the selected channel, writes the target address, loads
 * the caller-provided data and check masks, and then enables the channel
 * again.
 *
 * @param[in] instance       EMU instance index. Must be less than
 *                           `EMU_INSTANCE_COUNT`.
 * @param[in] channel        Channel index. Must be less than
 *                           `EMU_EICHD_COUNT`.
 * @param[in] injectErrAddr  SRAM word address used as the injection target.
 * @param[in] dataMulti      Raw data-bit mask. Pass `NO_INJECTION_ERROR` to
 *                           leave the data path untouched.
 * @param[in] chkMulti       Raw check-bit mask. Pass `NO_INJECTION_ERROR` to
 *                           leave the check-bit path untouched.
 *
 * @post The selected channel is enabled with the newly programmed masks.
 */
void EMU_DRV_InjectError(uint32_t instance, uint32_t channel,
                         uint32_t injectErrAddr, emu_databit_type dataMulti,
                         emu_chkbit_type chkMulti);

/*! @} */ /* End of Error Injection Control */

/*******************************************************************************
 * Error Report & Counter Access
 ******************************************************************************/
/*!
 * @name Error Report & Counter Access
 * @brief Functions for reading and clearing EMU report data.
 * @{
 */

/*!
 * @brief Convert the latched syndrome for one channel into a reported
 *        check-bit index.
 *
 * @param[in] instance  EMU instance index. Must be less than
 *                      `EMU_INSTANCE_COUNT`.
 * @param[in] channel   Report channel index. Must be less than
 *                      `EMU_ERINFO_COUNT`.
 * @return Reported check-bit index derived from the syndrome value.
 *
 * @note The returned value reflects the highest set syndrome bit encoded by
 *       the hardware report register.
 */
uint8_t EMU_DRV_GetErrReportChkbit(uint32_t instance, uint8_t channel);

/*!
 * @brief Read the error report counter for one channel.
 *
 * @param[in] instance  EMU instance index. Must be less than
 *                      `EMU_INSTANCE_COUNT`.
 * @param[in] channel   Report channel index. Must be less than
 *                      `EMU_EICHD_COUNT`.
 * @return Current error report counter value for the selected channel.
 */
uint32_t EMU_DRV_GetErrReportCnt(uint32_t instance, uint8_t channel);

/*!
 * @brief Clear the error report counter for one channel.
 *
 * @param[in] instance  EMU instance index. Must be less than
 *                      `EMU_INSTANCE_COUNT`.
 * @param[in] channel   Report channel index. Must be less than
 *                      `EMU_ERINFO_COUNT`.
 */
void EMU_DRV_ClearErrReportCnt(uint32_t instance, uint8_t channel);

/*!
 * @brief Read the address latched for the reported ECC event on one channel.
 *
 * @param[in] instance  EMU instance index. Must be less than
 *                      `EMU_INSTANCE_COUNT`.
 * @param[in] channel   Report channel index. Must be less than
 *                      `EMU_EICHD_COUNT`.
 * @return Latched report address for the selected channel.
 */
uint32_t EMU_DRV_GetErrReportAddr(uint32_t instance, uint8_t channel);

/*! @} */ /* End of Error Report & Counter Access */

/*******************************************************************************
 * Interrupt Status & Control
 ******************************************************************************/
/*!
 * @name Interrupt Status & Control
 * @brief Functions for querying and maintaining EMU interrupt state.
 * @{
 */

/*!
 * @brief Clear the single-bit correction interrupt flag for one channel.
 *
 * @param[in] instance  EMU instance index. Must be less than
 *                      `EMU_INSTANCE_COUNT`.
 * @param[in] channel   Channel index. Must be less than `EMU_EICHD_COUNT`.
 */
void EMU_DRV_ClearInterruptFlagSignalBit(uint32_t instance, uint8_t channel);

/*!
 * @brief Enable the double-bit interrupt path for one channel.
 *
 * @param[in] instance  EMU instance index. Must be less than
 *                      `EMU_INSTANCE_COUNT`.
 * @param[in] channel   Channel index. Must be less than `EMU_EICHD_COUNT`.
 */
void EMU_DRV_EnableInterruptDoubleBit(uint32_t instance, uint8_t channel);

/*!
 * @brief Disable the double-bit interrupt path for one channel.
 *
 * @param[in] instance  EMU instance index. Must be less than
 *                      `EMU_INSTANCE_COUNT`.
 * @param[in] channel   Channel index. Must be less than `EMU_EICHD_COUNT`.
 */
void EMU_DRV_DisableInterruptDoubleBit(uint32_t instance, uint8_t channel);

/*!
 * @brief Clear the double-bit interrupt flag for one channel.
 *
 * @param[in] instance  EMU instance index. Must be less than
 *                      `EMU_INSTANCE_COUNT`.
 * @param[in] channel   Channel index. Must be less than `EMU_EICHD_COUNT`.
 */
void EMU_DRV_ClearInterruptFlagDoubleBit(uint32_t instance, uint8_t channel);

/*!
 * @brief Query the single-bit correction interrupt flag for one channel.
 *
 * @param[in] instance  EMU instance index. Must be less than
 *                      `EMU_INSTANCE_COUNT`.
 * @param[in] channel   Channel index. Must be less than `EMU_EICHD_COUNT`.
 * @return Non-zero value when the single-bit correction flag is asserted for
 *         the selected channel.
 */
uint8_t EMU_DRV_GetChannelSBInterruptFlag(uint32_t instance, uint8_t channel);

/*! @} */ /* End of Interrupt Status & Control */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of emu_driver */

#endif /* EMU_DRIVER_H */
