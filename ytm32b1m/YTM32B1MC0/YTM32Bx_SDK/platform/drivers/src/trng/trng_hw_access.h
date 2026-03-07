/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*!
 * @file trng_hw_access.h
 * @version 1.4.0
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
 * @details This section describes the programming interface of the TRNG Hardware Access.
 * @{
 */

/*!
 * @addtogroup trng_hw_access
 * @{
 */
/*******************************************************************************
* Definitions
*******************************************************************************/
/*! 
 * @brief TRNG status flags.
 */
typedef enum
{
    TRNG_FREQUENCY_COUNT_VALID              = 0x1,      /*!< Frequency Count Valid. */
    TRNG_BUSY                               = 0x5,      /*!< IDLE flag. */
    TRNG_HARDWARE_ERROR                     = 0x6,      /*!< Hardware Error Interrupt Flag */
    TRNG_ENTROPY_VALID                      = 0x9,      /*!< Entropy Valid Interrupt Flag. */
    TRNG_FREQUENCY_COUNT_FAIL               = 0x0A,     /*!< Frequency Count Fail Interrupt Flag*/
    TRNG_ALL_STATUS                         = 0x722,    /*!< Used for clearing all trng status flags */
} trng_status_flag_t;

/*! 
 * @brief Prescaler values for TRNG OSC clock source.
 */
typedef enum{
    TRNG_OSC_NO_DEVIDER = 0,            /*!< No devider.*/
    TRNG_OSC_DIV_2 = 1,                 /*!< Ring OSC divided by 2.*/
    TRNG_OSC_DIV_4 = 2,                 /*!< Ring OSC divided by 4.*/
    TRNG_OSC_DIV_8 = 3,                 /*!< Ring OSC divided by 8.*/
}trng_osc_div_t;

/*!
 * @brief Data structure containing information about a trng.
 *
 * The user must populate these members to set up the trng.
 * Implements : trng_config_setting_t_Class
 */
typedef struct 
{
    uint32_t lrunMax;   /*!< the largest allowable number of consecutive samples of all 1 or all 0 during the entropy generation.*/
    uint8_t retryCnt;           /*!< indecates the number of times a retry should occur before generating an error. */
    bool forceSystemClockEn;      /*!< the system clock is used to operate the TRNG instead of the ring OSC.*/
    bool clkOutputEn;             /*!< After divided by 4, trng_osc_clk is output for test.*/
    trng_osc_div_t  oscDiv;     /*!< OSC Divider*/
    bool frqCntInterruptEn;     /*!< Frequency Count Fail Interrupt Enable*/
    bool entroptValidInterruptEn; /*!< Entropy Valid Interrupt Enable*/
    bool hwErrInterruptEn;      /*!< Hardware Error Interrupt Enable*/

    uint32_t sampleSize;        /*!< Defines the total number of entropy samples */
    uint32_t entropyDelay;      /*!< Entropy delay*/

    uint32_t frqMin;            /*!< Frequency minimum limit value*/
    uint32_t frqMax;            /*!< Frequency Maximum Limit Value*/

    uint32_t monobitRange;      /*!< Monobit Range*/
    uint32_t monobitMax;        /*!< Monobit Maximum Limit*/
    uint32_t *pEntValue;        /*!< the pointer for reading entropy Value*/
}trng_config_setting_t;

extern trng_config_setting_t trngCfgSetting;

/*******************************************************************************
* API
*******************************************************************************/
/*!
 * @name Trng driver APIs
 * @{
 */
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Enable the TRNG module.
 *
 * This function enable the TRNG module.
 *
 * @param[in] base   The TRNG peripheral base address
 *           
 */
void TRNG_Enable(TRNG_Type *const base);

/*!
 * @brief Restore the TRNG module to reset value.
 *
 * This function restores the TRNG module to reset value.
 *
 * @param[in] base   The TRNG peripheral base address
 *           
 */
void TRNG_HW_Init(TRNG_Type *const base);

/*!
 * @brief Disable the TRNG module.
 *
 * This function disable the TRNG module.
 *
 * @param[in] base   The TRNG peripheral base address
 *           
 */
void TRNG_Disable(TRNG_Type *const base);

/*!
 * @brief Reset the TRNG module by software.
 *
 * This function reset the TRNG module by software.
 *
 * @param[in] base   The TRNG peripheral base address
 *           
 */
void TRNG_SWReset(TRNG_Type *const base);

/*!
 * @brief Set the CTRL register of TRNG module.
 *
 * This function set the CTRL register of TRNG module.
 *
 * @param[in] base   The TRNG peripheral base address
 *           
 */
void TRNG_Set_CTRL(TRNG_Type *const base);

/*!
 * @brief Set the SDCTL register of TRNG module.
 *
 * This function set the SDCTL register of TRNG module.
 *
 * @param[in] base   The TRNG peripheral base address
 *  
 */  
void TRNG_Set_SDCTL(TRNG_Type * const base);

/*!
 * @brief Set the FRQMIN register of TRNG module.
 *
 * This function set the FRQMIN register of TRNG module.
 *
 * @param[in] base   The TRNG peripheral base address
 *  
 */  
void TRNG_Set_FRQMIN(TRNG_Type *const base);

/*!
 * @brief Set the FRQMAX register of TRNG module.
 *
 * This function set the FRQMAX register of TRNG module.
 *
 * @param[in] base   The TRNG peripheral base address
 *  
 */ 
void TRNG_Set_FRQMAX(TRNG_Type *const base);

/*!
 * @brief Get the FRQCNT register value.
 *
 * This function get the FRQCNT register value.
 *
 * @param[in] base   The TRNG peripheral base address
 * @return           the value of FRQCNT register
 */ 
uint32_t TRNG_Get_FRQCNT(TRNG_Type *const base);

/*!
 * @brief Set the SCML register of TRNG module.
 *
 * This function set the SCML register of TRNG module.
 *
 * @param[in] base   The TRNG peripheral base address
 *  
 */ 
void TRNG_Set_SCML(TRNG_Type *const base);

/*!
 * @brief Get the SCMC register value.
 *
 * This function get the SCMC register value.
 *
 * @param[in] base   The TRNG peripheral base address
 * @return           the value of SCMC register
 */ 
uint32_t TRNG_Get_SCMC(TRNG_Type *const base);

/*!
 * @brief Get the ENT register value.
 *
 * This function get the ENT register value.
 *
 * @param[in] base   The TRNG peripheral base address
 * @return           the value of ENT register
 */ 
uint32_t TRNG_Get_ENT(TRNG_Type *const base , uint8_t index);

/*!
 * @brief Gets the TRNG status flag state.
 *
 * This function returns the state of one of the Trng status flags as requested by
 * the user.
 *
 * @param base Module base pointer of type TRNG_Type.
 * @param statusFlag The status flag, of type trng_status_flag_t
 * @return State of the status flag: asserted (true) or not-asserted (false)
 */
bool TRNG_GetStatusFlag(TRNG_Type *const base, trng_status_flag_t statusFlag);

/*!
 * @brief Clears the TRNG status flag.
 *
 * This function clears the state of one of the TRNG status flags as requested by the user. 
 * 
 * @param base Module base pointer of type TRNG_Type.
 * @param statusFlag The status flag, of type trng_status_flag_t
 * @return STATUS_SUCCESS or TRNG_STS_INVALID_PARAMETER
 */
status_t TRNG_ClearStatusFlag(TRNG_Type *const base, trng_status_flag_t statusFlag);

#if defined(__cplusplus)
}
#endif

/*! @}*/ /* End of trng hardware driver APIs*/
/*! @}*/ /* End of addtogroup trng_hw_driver */

#endif /* TRNG_HW_ACCESS_H */
/*******************************************************************************
* EOF
*******************************************************************************/
