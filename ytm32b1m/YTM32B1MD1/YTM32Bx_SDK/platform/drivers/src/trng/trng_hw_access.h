/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*!
 * @file trng_hw_access.h
 * @version 1.4.1
 *
 * @brief TRNG Hardware Access Layer.
 *
 * This header provides the low-level helper functions and configuration types
 * used to program the TRNG register block. The helpers are organized into
 * module-control, register-programming, and data/status-access categories.
 *
 * @note This is the internal layer used by trng_driver.h/c. Application code
 *       should normally call the TRNG_DRV_* APIs unless direct register access
 *       is required.
 */

#ifndef TRNG_HW_ACCESS_H
#define TRNG_HW_ACCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "status.h"
#include "device_registers.h"

/*!
 * @defgroup trng_hw_access TRNG Hardware Access
 * @ingroup trng
 * @brief Low-level register access helpers for the TRNG peripheral.
 * @{
 */

/*!
 * @brief TRNG status selectors used by the low-level status helpers.
 *
 * The enumerators map to the status conditions handled by
 * TRNG_GetStatusFlag() and TRNG_ClearStatusFlag().
 *
 * | Value                      | Description                                                |
 * |----------------------------|------------------------------------------------------------|
 * | TRNG_FREQUENCY_COUNT_VALID | Frequency-count result is ready for validation.            |
 * | TRNG_BUSY                  | Generation-progress condition used by TRNG_DRV_GetStatus().|
 * | TRNG_HARDWARE_ERROR        | Hardware error condition.                                  |
 * | TRNG_ENTROPY_VALID         | Entropy block is ready to read.                            |
 * | TRNG_FREQUENCY_COUNT_FAIL  | Frequency-count test failed.                               |
 * | TRNG_ALL_STATUS            | Special clear mask for all writable TRNG status bits.      |
 */
typedef enum
{
    TRNG_FREQUENCY_COUNT_VALID = 0x1,   /*!< Frequency-count-valid selector. */
    TRNG_BUSY = 0x5,                    /*!< Busy-state selector used by the driver. */
    TRNG_HARDWARE_ERROR = 0x6,          /*!< Hardware-error selector. */
    TRNG_ENTROPY_VALID = 0x9,           /*!< Entropy-valid selector. */
    TRNG_FREQUENCY_COUNT_FAIL = 0x0AU,  /*!< Frequency-count-fail selector. */
    TRNG_ALL_STATUS = 0x722,            /*!< Aggregate mask for clearing all status flags. */
} trng_status_flag_t;

/*!
 * @brief Ring-oscillator divider selection.
 *
 * Selects the divider applied to the internal ring-oscillator clock before it
 * is used by the TRNG logic or routed to the optional clock-output path.
 *
 * | Value               | Description                      |
 * |---------------------|----------------------------------|
 * | TRNG_OSC_NO_DEVIDER | Use the ring oscillator directly.|
 * | TRNG_OSC_DIV_2      | Divide the ring oscillator by 2. |
 * | TRNG_OSC_DIV_4      | Divide the ring oscillator by 4. |
 * | TRNG_OSC_DIV_8      | Divide the ring oscillator by 8. |
 */
typedef enum
{
    TRNG_OSC_NO_DEVIDER = 0,  /*!< No divider. */
    TRNG_OSC_DIV_2 = 1,       /*!< Divide the oscillator clock by 2. */
    TRNG_OSC_DIV_4 = 2,       /*!< Divide the oscillator clock by 4. */
    TRNG_OSC_DIV_8 = 3,       /*!< Divide the oscillator clock by 8. */
} trng_osc_div_t;

/*!
 * @brief TRNG configuration shadow used by the register-programming helpers.
 *
 * The low-level setters read configuration values from this structure when
 * programming the CTRL, SDCTL, FRQMIN, FRQMAX, and SCML registers.
 *
 * | Field                  | Type            | Description                                                |
 * |------------------------|-----------------|------------------------------------------------------------|
 * | lrunMax                | uint32_t        | Long-run test threshold.                                   |
 * | retryCnt               | uint8_t         | Retry count before the hardware reports an error.          |
 * | forceSystemClockEn     | bool            | Select the system clock instead of the ring oscillator.    |
 * | clkOutputEn            | bool            | Output the divided TRNG clock for debug observation.       |
 * | oscDiv                 | trng_osc_div_t  | Divider applied to the ring oscillator.                    |
 * | frqCntInterruptEn      | bool            | Enable frequency-count-fail interrupt generation.          |
 * | entroptValidInterruptEn| bool            | Enable entropy-valid interrupt generation.                 |
 * | hwErrInterruptEn       | bool            | Enable hardware-error interrupt generation.                |
 * | sampleSize             | uint32_t        | Number of entropy samples collected per generation cycle.  |
 * | entropyDelay           | uint32_t        | Delay inserted between entropy samples.                    |
 * | frqMin                 | uint32_t        | Minimum frequency-count threshold.                         |
 * | frqMax                 | uint32_t        | Maximum frequency-count threshold.                         |
 * | monobitRange           | uint32_t        | Monobit range threshold.                                   |
 * | monobitMax             | uint32_t        | Monobit maximum threshold.                                 |
 * | pEntValue              | uint32_t *      | Optional pointer reserved for entropy-buffer management.   |
 */
typedef struct
{
    uint32_t lrunMax;               /*!< Long-run test threshold. */
    uint8_t retryCnt;               /*!< Retry count before reporting an error. */
    bool forceSystemClockEn;        /*!< Use the system clock instead of the ring oscillator. */
    bool clkOutputEn;               /*!< Enable divided TRNG clock output for observation. */
    trng_osc_div_t oscDiv;          /*!< Ring-oscillator divider selection. */
    bool frqCntInterruptEn;         /*!< Enable frequency-count-fail interrupt generation. */
    bool entroptValidInterruptEn;   /*!< Enable entropy-valid interrupt generation. */
    bool hwErrInterruptEn;          /*!< Enable hardware-error interrupt generation. */
    uint32_t sampleSize;            /*!< Number of entropy samples in one generation cycle. */
    uint32_t entropyDelay;          /*!< Delay inserted between entropy samples. */
    uint32_t frqMin;                /*!< Minimum accepted frequency-count value. */
    uint32_t frqMax;                /*!< Maximum accepted frequency-count value. */
    uint32_t monobitRange;          /*!< Monobit range threshold. */
    uint32_t monobitMax;            /*!< Monobit maximum threshold. */
    uint32_t *pEntValue;            /*!< Optional pointer reserved for entropy-buffer storage. */
} trng_config_setting_t;

/*! @brief Shared TRNG configuration shadow consumed by the low-level setters. */
extern trng_config_setting_t trngCfgSetting;

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Module Control
 ******************************************************************************/
/*!
 * @name Module Control
 * @brief Functions for enabling, disabling, resetting, and restoring the TRNG.
 * @{
 */

/*!
 * @brief Enable the TRNG module.
 *
 * @param[in] base  Pointer to the TRNG peripheral base address.
 */
void TRNG_Enable(TRNG_Type *const base);

/*!
 * @brief Restore the TRNG register block to its reset configuration.
 *
 * @param[in] base  Pointer to the TRNG peripheral base address.
 */
void TRNG_HW_Init(TRNG_Type *const base);

/*!
 * @brief Disable the TRNG module.
 *
 * @param[in] base  Pointer to the TRNG peripheral base address.
 */
void TRNG_Disable(TRNG_Type *const base);

/*!
 * @brief Request a software reset of the TRNG module.
 *
 * @param[in] base  Pointer to the TRNG peripheral base address.
 */
void TRNG_SWReset(TRNG_Type *const base);

/*! @} */ /* End of Module Control */

/*******************************************************************************
 * Configuration Register Programming
 ******************************************************************************/
/*!
 * @name Configuration Register Programming
 * @brief Functions that program protected TRNG configuration registers from
 *        the current trngCfgSetting shadow.
 * @{
 */

/*!
 * @brief Program the CTRL register from trngCfgSetting.
 *
 * @param[in] base  Pointer to the TRNG peripheral base address.
 */
void TRNG_Set_CTRL(TRNG_Type *const base);

/*!
 * @brief Program the SDCTL register from trngCfgSetting.
 *
 * @param[in] base  Pointer to the TRNG peripheral base address.
 */
void TRNG_Set_SDCTL(TRNG_Type *const base);

/*!
 * @brief Program the FRQMIN register from trngCfgSetting.
 *
 * @param[in] base  Pointer to the TRNG peripheral base address.
 */
void TRNG_Set_FRQMIN(TRNG_Type *const base);

/*!
 * @brief Program the FRQMAX register from trngCfgSetting.
 *
 * @param[in] base  Pointer to the TRNG peripheral base address.
 */
void TRNG_Set_FRQMAX(TRNG_Type *const base);

/*!
 * @brief Program the SCML register from trngCfgSetting.
 *
 * @param[in] base  Pointer to the TRNG peripheral base address.
 */
void TRNG_Set_SCML(TRNG_Type *const base);

/*! @} */ /* End of Configuration Register Programming */

/*******************************************************************************
 * Data & Status Access
 ******************************************************************************/
/*!
 * @name Data & Status Access
 * @brief Functions for reading entropy data, statistical counters, and status.
 * @{
 */

/*!
 * @brief Read the frequency-count result from the FRQCNT register.
 *
 * The function waits until the frequency-count-valid condition is asserted
 * before returning the counter value.
 *
 * @param[in] base  Pointer to the TRNG peripheral base address.
 * @return Frequency-count result from the FRQCNT register.
 */
uint32_t TRNG_Get_FRQCNT(TRNG_Type *const base);

/*!
 * @brief Read the monobit count from the SCMC register.
 *
 * @param[in] base  Pointer to the TRNG peripheral base address.
 * @return Monobit count sampled by the TRNG hardware.
 */
uint32_t TRNG_Get_SCMC(TRNG_Type *const base);

/*!
 * @brief Read one entropy word from the ENT register bank.
 *
 * @param[in] base   Pointer to the TRNG peripheral base address.
 * @param[in] index  Entropy word index to read from the ENT array.
 * @return Selected 32-bit entropy word.
 *
 * @warning Reading the highest valid entropy word clears the current block and
 *          starts the next entropy-generation cycle.
 */
uint32_t TRNG_Get_ENT(TRNG_Type *const base, uint8_t index);

/*!
 * @brief Get the state of one TRNG status condition.
 *
 * @param[in] base        Pointer to the TRNG peripheral base address.
 * @param[in] statusFlag  Status selector from trng_status_flag_t.
 * @return true if the requested condition is asserted, or false otherwise.
 */
bool TRNG_GetStatusFlag(TRNG_Type *const base, trng_status_flag_t statusFlag);

/*!
 * @brief Clear one TRNG status condition or all writable status bits.
 *
 * @param[in] base        Pointer to the TRNG peripheral base address.
 * @param[in] statusFlag  Status selector from trng_status_flag_t. Pass
 *                        TRNG_ALL_STATUS to clear all writable status bits.
 * @return STATUS_SUCCESS after issuing the flag-clear write.
 */
status_t TRNG_ClearStatusFlag(TRNG_Type *const base, trng_status_flag_t statusFlag);

/*! @} */ /* End of Data & Status Access */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of trng_hw_access group */

#endif /* TRNG_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
