/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file emu_driver.h
 * @version 1.4.0
 */

#ifndef EMU_DRIVER_H
#define EMU_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include "status.h"
#include "device_registers.h"
#include "emu_hw_access.h"

/*!
 * @ingroup emu
 * @addtogroup emu_driver
 * @{
 */


/*******************************************************************************
* Definitions
******************************************************************************/
/*!
 * @brief emu bit-mapped mask
 *
 * This enumeration type indicates which data bit is injected when the injected error occurs on the data .
 * Implements : e_emu_databit_Class
 */
typedef enum
{
    EMU_DATABIT_0,          /*!< Error code is injected at bit 0 of the data.*/
    EMU_DATABIT_1,          /*!< Error code is injected at bit 1 of the data.*/
    EMU_DATABIT_2,          /*!< Error code is injected at bit 2 of the data.*/
    EMU_DATABIT_3,          /*!< Error code is injected at bit 3 of the data.*/
    EMU_DATABIT_4,          /*!< Error code is injected at bit 4 of the data.*/
    EMU_DATABIT_5,          /*!< Error code is injected at bit 5 of the data.*/
    EMU_DATABIT_6,          /*!< Error code is injected at bit 6 of the data.*/
    EMU_DATABIT_7,          /*!< Error code is injected at bit 7 of the data.*/
    EMU_DATABIT_8,          /*!< Error code is injected at bit 8 of the data.*/
    EMU_DATABIT_9,          /*!< Error code is injected at bit 9 of the data.*/
    EMU_DATABIT_10,         /*!< Error code is injected at bit 10 of the data.*/
    EMU_DATABIT_11,         /*!< Error code is injected at bit 11 of the data.*/
    EMU_DATABIT_12,         /*!< Error code is injected at bit 12 of the data.*/
    EMU_DATABIT_13,         /*!< Error code is injected at bit 13 of the data.*/
    EMU_DATABIT_14,         /*!< Error code is injected at bit 14 of the data.*/
    EMU_DATABIT_15,         /*!< Error code is injected at bit 15 of the data.*/
    EMU_DATABIT_16,         /*!< Error code is injected at bit 16 of the data.*/
    EMU_DATABIT_17,         /*!< Error code is injected at bit 17 of the data.*/
    EMU_DATABIT_18,         /*!< Error code is injected at bit 18 of the data.*/
    EMU_DATABIT_19,         /*!< Error code is injected at bit 19 of the data.*/
    EMU_DATABIT_20,         /*!< Error code is injected at bit 20 of the data.*/
    EMU_DATABIT_21,         /*!< Error code is injected at bit 21 of the data.*/
    EMU_DATABIT_22,         /*!< Error code is injected at bit 22 of the data.*/
    EMU_DATABIT_23,         /*!< Error code is injected at bit 23 of the data.*/
    EMU_DATABIT_24,         /*!< Error code is injected at bit 24 of the data.*/
    EMU_DATABIT_25,         /*!< Error code is injected at bit 25 of the data.*/
    EMU_DATABIT_26,         /*!< Error code is injected at bit 26 of the data.*/
    EMU_DATABIT_27,         /*!< Error code is injected at bit 27 of the data.*/
    EMU_DATABIT_28,         /*!< Error code is injected at bit 28 of the data.*/
    EMU_DATABIT_29,         /*!< Error code is injected at bit 29 of the data.*/
    EMU_DATABIT_30,         /*!< Error code is injected at bit 30 of the data.*/
    EMU_DATABIT_31,         /*!< Error code is injected at bit 31 of the data.*/
    #if defined(FEATURE_EMU_SUPPORT_64BIT_INJECTION) && (FEATURE_EMU_SUPPORT_64BIT_INJECTION == 1U)
    EMU_DATABIT_32,         /*!< Error code is injected at bit 32 of the data.*/
    EMU_DATABIT_33,         /*!< Error code is injected at bit 33 of the data.*/
    EMU_DATABIT_34,         /*!< Error code is injected at bit 34 of the data.*/
    EMU_DATABIT_35,         /*!< Error code is injected at bit 35 of the data.*/
    EMU_DATABIT_36,         /*!< Error code is injected at bit 36 of the data.*/
    EMU_DATABIT_37,         /*!< Error code is injected at bit 37 of the data.*/
    EMU_DATABIT_38,         /*!< Error code is injected at bit 38 of the data.*/
    EMU_DATABIT_39,         /*!< Error code is injected at bit 39 of the data.*/
    EMU_DATABIT_40,         /*!< Error code is injected at bit 40 of the data.*/
    EMU_DATABIT_41,         /*!< Error code is injected at bit 41 of the data.*/
    EMU_DATABIT_42,         /*!< Error code is injected at bit 42 of the data.*/
    EMU_DATABIT_43,         /*!< Error code is injected at bit 43 of the data.*/
    EMU_DATABIT_44,         /*!< Error code is injected at bit 44 of the data.*/
    EMU_DATABIT_45,         /*!< Error code is injected at bit 45 of the data.*/
    EMU_DATABIT_46,         /*!< Error code is injected at bit 46 of the data.*/
    EMU_DATABIT_47,         /*!< Error code is injected at bit 47 of the data.*/
    EMU_DATABIT_48,         /*!< Error code is injected at bit 48 of the data.*/
    EMU_DATABIT_49,         /*!< Error code is injected at bit 49 of the data.*/
    EMU_DATABIT_50,         /*!< Error code is injected at bit 50 of the data.*/
    EMU_DATABIT_51,         /*!< Error code is injected at bit 51 of the data.*/
    EMU_DATABIT_52,         /*!< Error code is injected at bit 52 of the data.*/
    EMU_DATABIT_53,         /*!< Error code is injected at bit 53 of the data.*/
    EMU_DATABIT_54,         /*!< Error code is injected at bit 54 of the data.*/
    EMU_DATABIT_55,         /*!< Error code is injected at bit 55 of the data.*/
    EMU_DATABIT_56,         /*!< Error code is injected at bit 56 of the data.*/
    EMU_DATABIT_57,         /*!< Error code is injected at bit 57 of the data.*/
    EMU_DATABIT_58,         /*!< Error code is injected at bit 58 of the data.*/
    EMU_DATABIT_59,         /*!< Error code is injected at bit 59 of the data.*/
    EMU_DATABIT_60,         /*!< Error code is injected at bit 60 of the data.*/
    EMU_DATABIT_61,         /*!< Error code is injected at bit 61 of the data.*/
    EMU_DATABIT_62,         /*!< Error code is injected at bit 62 of the data.*/
    EMU_DATABIT_63,         /*!< Error code is injected at bit 63 of the data.*/
    EMU_DATABIT_MAX = 64,   /*!< The data bit of the injected error cannot reach the 64th bit at most.*/
    #else
    EMU_DATABIT_MAX = 32,   /*!< The data bit of the injected error cannot reach the 32nd bit at most.*/
    #endif
} e_emu_databit;

/*!
 * @brief emu check-mapped mask
 *
 * This enumeration type indicates which check bit is injected when the injected error occurs on the check .
 * Implements : e_emu_chkbit_Class
 */
typedef enum
{
    EMU_CHKBIT_0,           /*!< Error code is injected at bit 0 of the check.*/
    EMU_CHKBIT_1,           /*!< Error code is injected at bit 1 of the check.*/
    EMU_CHKBIT_2,           /*!< Error code is injected at bit 2 of the check.*/
    EMU_CHKBIT_3,           /*!< Error code is injected at bit 3 of the check.*/
    EMU_CHKBIT_4,           /*!< Error code is injected at bit 4 of the check.*/
    EMU_CHKBIT_5,           /*!< Error code is injected at bit 5 of the check.*/
    EMU_CHKBIT_6,           /*!< Error code is injected at bit 6 of the check.*/
    EMU_CHKBIT_MAX,         /*!< The check bit of the injected error cannot reach the seventh bit at most.*/
} e_emu_chkbit;

/*!
 * @brief emu error inject bit
 *
 * The injection error can be injected in the data bit, in the check bit, 
 * or mixed injection.
 * Implements : emu_inject_bits_type_Class
 */
typedef enum
{
    EMU_INJECT_BITS_TYPE_DATA,      /*!< Inject one error bit on data*/
    EMU_INJECT_BITS_TYPE_CHK,       /*!< Inject one error bit on check*/
    EMU_INJECT_BITS_TYPE_MIXTURE,   /*!< Inject two error bits on data and check*/
} emu_inject_bits_type;

/*!
 * @brief emu error inject type
 * 
 * Injection error information, including the type of injection error, 
 * which data bit to inject, or which check bit.
 * Implements : emu_inject_err_t_Class
 */
typedef struct
{
    emu_inject_bits_type injectBitsType; /*!< Inject error type.*/
    e_emu_databit dataBit;               /*!< Inject error on which data bit.*/
    e_emu_chkbit chkBit;                 /*!< Inject error on which check bit.*/
} emu_inject_err_t;

/*!
 * @brief Emu Configuration Structure
 *
 * The structure includes the static parameters for Emu which are
 * device-dependent. The fields including
 *
 * Implements : emu_config_t_Class
 */
typedef struct
{
    uint32_t channel;                   /*!< channel 0 and channel 1 corresponds to SRAM_L and SRAM_U.*/
    uint32_t injectErrAddr;             /*!< Error injection address*/
    emu_inject_err_t injectErrSetInfo;  /*!< Error injection Setting information*/
    bool interruptEnable;               /*!< Enable interrupt*/
} emu_config_t;

/*******************************************************************************
* Function Prototypes for Emu 
*******************************************************************************/
/*!
 * @name Emu driver APIs
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initialize a Emu instance for operation.
 *
 * This function first resets the source triggers of all EMU target modules to their default values,
 * then configures the EMU with all the user defined in-out mappings.
 * This example shows how to set up the emu_config_t parameters and how to call the
 * EMU_DRV_Init() function with the required parameters:
 *  @code
 *   emu_config_t EmuInitConfig0 = {
        .channel = 0,
        .injectErrAddr = 0x20000000,
        .injectErrSetInfo = 
        {
            .injectBitsType = EMU_INJECT_BITS_TYPE_DATA,
            .dataBit = 7,
        },
        .interruptEnable = false,
    };
 *
 *   EMU_DRV_Init(instance, &EmuInitConfig0,1);
 *   @endcode
 *
 * @param[in] instance          The EMU instance number.
 * @param[in] configPtr         Pointer to the user configuration structure.
 * @param[in] startFlag         Whether to enable EMU module.
 */
void EMU_DRV_Init(uint32_t instance, const emu_config_t *configPtr, uint32_t startFlag);

/*!
 * @brief Reset to default values the source triggers corresponding to all target 
 *
 * @param[in] instance          The EMU instance number.
 * @param[in] channel           The EMU SRAML or SRAMU.
 */
void EMU_DRV_DeInit(uint32_t instance, uint32_t channel);

/*!
 * @brief Get error report check bit.
 *
 * @param[in] instance          The EMU instance number.
 * @param[in] channel           The EMU SRAML or SRAMU.
 */
uint8_t EMU_DRV_GetErrReportChkbit(uint32_t instance, uint8_t channel);

/*!
 * @brief Clear signal bit interrupt flag.
 *
 * @param[in] instance          The EMU instance number.
 * @param[in] channel           The EMU SRAML or SRAMU.
 */
void EMU_DRV_ClearInterruptFlagSignalBit(uint32_t instance, uint8_t channel);

/*!
 * @brief Enable double bit interrupt.
 *
 * @param[in] instance          The EMU instance number.
 * @param[in] channel           The EMU SRAML or SRAMU.
 */
void EMU_DRV_EnableInterruptDoubleBit(uint32_t instance, uint8_t channel);

/*!
 * @brief Disable double bit interrupt.
 *
 * @param[in] instance          The EMU instance number.
 * @param[in] channel           The EMU SRAML or SRAMU.
 */
void EMU_DRV_DisableInterruptDoubleBit(uint32_t instance, uint8_t channel);

/*!
 * @brief Clear double bit interrupt flag.
 *
 * @param[in] instance          The EMU instance number.
 * @param[in] channel           The EMU SRAML or SRAMU.
 */
void EMU_DRV_ClearInterruptFlagDoubleBit(uint32_t instance, uint8_t channel);

/*!
 * @brief Get error report count.
 *
 * @param[in] instance          The EMU instance number.
 * @param[in] channel           The EMU SRAML or SRAMU.
 */
uint32_t EMU_DRV_GetErrReportCnt(uint32_t instance, uint8_t channel);

/*!
 * @brief Clear error report count.
 *
 * @param[in] instance          The EMU instance number.
 * @param[in] channel           The EMU SRAML or SRAMU.
 */
void EMU_DRV_ClearErrReportCnt(uint32_t instance, uint8_t channel);

/*!
 * @brief Get error report address.
 *
 * @param[in] instance          The EMU instance number.
 * @param[in] channel           The EMU SRAML or SRAMU.
 */
uint32_t EMU_DRV_GetErrReportAddr(uint32_t instance, uint8_t channel);


/*!
 * @brief Inject error data.
 *
 * @param[in] instance          The EMU instance number.
 * @param[in] channel           The EMU SRAML or SRAMU.
 * @param[in] injectErrAddr     Error Injection Address.
 * @param[in] dataMulti         Multiple error injection databit.
 * @param[in] chkMulti          Multiple error injection checkbit.
 */
void EMU_DRV_InjectError(uint32_t instance, uint32_t channel,
                        uint32_t injectErrAddr,emu_databit_type dataMulti,
                        emu_chkbit_type chkMulti);

/*!
 * @brief Get single bit error correct check interrupt flag.
 *
 * @param[in] instance          The EMU instance number.
 * @param[in] channel           The EMU SRAML, SRAMU or ROM.
 */
uint8_t EMU_DRV_GetChannelSBInterruptFlag(uint32_t instance, uint8_t channel);

#if defined(__cplusplus)
}
#endif

/*! @}*/ /* End of Emu driver APIs*/
/*! @}*/ /* End of addtogroup emu_driver */

#endif /* EMU_DRIVER_H */
