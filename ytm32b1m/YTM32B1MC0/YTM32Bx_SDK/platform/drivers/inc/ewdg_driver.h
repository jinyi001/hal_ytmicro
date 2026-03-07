/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file ewdg_driver.h
 * @version 1.4.0
 */

#ifndef EWDG_DRIVER_H
#define EWDG_DRIVER_H

#include "device_registers.h"
#include "status.h"

/*******************************************************************************
* Definitions
******************************************************************************/
#if (defined(EWDG_CLK_CTRL_CLKSEL_MASK))
/*!
 * @brief EWDG clock source.
 * Implements : ewdg_clock_source_t_Class
 */
typedef enum
{
    EWDG_SIRC_CLOCK = 0x00U,    /*!< SIRC clock */
#if defined(FEATURE_EWDG_SUPPORT_LPO_CLOCK) && (FEATURE_EWDG_SUPPORT_LPO_CLOCK == 1)
    EWDG_LPO_CLOCK = 0x01U,     /*!< LPO clock */
#else
    EWDG_SXOSC_CLOCK = 0x01U,   /*!< SXOSC clock */
#endif
} ewdg_clock_source_t;
#endif

/*!
 * @brief EWDG input pin configuration
 * Configures if the input pin is enabled and when is asserted
 * Implements : ewdg_in_assert_logic_t_Class
 */
typedef enum
{
    EWDG_IN_ASSERT_DISABLED = 0x00U,            /*!< Input pin disabled                    */
    EWDG_IN_ASSERT_ON_LOGIC_ZERO = 0x01U,       /*!< Input pin asserts EWDG when on logic 0 */
    EWDG_IN_ASSERT_ON_LOGIC_ONE = 0x02U         /*!< Input pin asserts EWDG when on logic 1 */
} ewdg_in_assert_logic_t;

/*!
 * @brief EWDG configuration structure
 * This structure is used to configure the EWDG prescaler, window, interrupt
 * and input pin.
 *
 * Implements : ewdg_init_config_t_Class
 */
typedef struct
{
#if (defined(EWDG_CLK_CTRL_CLKSEL_MASK))
    ewdg_clock_source_t clockSource;        /*!< EWDG clock source */
#endif
    ewdg_in_assert_logic_t assertLogic;     /*!< Assert logic for EWDG input pin */
    bool interruptEnable;                   /*!< Enable EWDG interrupt           */
    uint8_t prescaler;                      /*!< EWDG clock prescaler            */
    uint16_t compareLow;                     /*!< Compare low value              */
    uint16_t compareHigh;                    /*!< Compare high value             */
} ewdg_init_config_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Init EWDG. This method initializes EWDG instance to the configuration
 * from the passed structure. The user must make sure that the clock is
 * enabled. This is the only method needed to be called to start the module.
 *
 *  Example configuration structure:
 *  @code
 *      ewdg_init_config_t ewdgUserCfg = {
 *          .assertLogic        = EWDG_IN_ASSERT_ON_LOGIC_ZERO,
 *          .interruptEnable    = true,
 *          .prescaler          = 128,
 *          .compareLow         = 0,
 *          .compareHigh        = 254
 *      };
 *  @endcode
 *      This configuration will enable the peripheral, with input pin configured
 *      to assert on logic low, interrupt enabled, prescaler 128 and maximum
 *      refresh window.
 *
 *      The EWDG can be initialized only once per CPU reset as the registers
 *      are write once.
 *
 * @param[in] instance EWDG instance number
 * @param[in] config  Pointer to the module configuration structure.
 *
 * @return status_t Will return the status of the operation:
 *          - STATUS_SUCCESS            if the operation is successful
 *          - STATUS_ERROR              if the windows values are not
 *                                        correct or if the instance is
 *                                        already enabled
 *
 */
status_t EWDG_DRV_Init(uint32_t instance, const ewdg_init_config_t *config);

/*!
 * @brief Init configuration structure to default values.
 *
 * @param[out] config Pointer to the configuration structure to initialize
 *
 * @return None
 *
 */
void EWDG_DRV_GetDefaultConfig(ewdg_init_config_t *config);

/*!
 * @brief Refresh EWDG. This method needs to be called within the window period
 * specified by the Compare Low and Compare High registers.
 *
 * @param[in] instance EWDG instance number
 *
 * @return None
 *
 */
void EWDG_DRV_Refresh(uint32_t instance);

/*!
 * @brief Get the Input pin assert logic.
 * @param[in] instance EWDG instance number
 * @return The input pin assert logic:
 *      -   EWDG_IN_ASSERT_DISABLED      - EWDG in disabled
 *      -   EWDG_IN_ASSERT_ON_LOGIC_ZERO - EWDG is asserted when EWDG_in is logic 0
 *      -   EWDG_IN_ASSERT_ON_LOGIC_ONE  - EWDG is asserted when EWDG_in is logic 1
 */
ewdg_in_assert_logic_t EWDG_DRV_GetInputPinAssertLogic(uint32_t instance);

/*!
 * @brief DeInit the EWDG module.
 * @param[in] instance EWDG instance number
 * @return None
 */
void EWDG_DRV_DeInit(uint32_t instance);

/*!
 * @brief Disable the EWDG interrupt.
 * @param[in] instance EWDG instance number
 * @return None
 */
void EWDG_DRV_DisableInterrupt(uint32_t instance);

#if defined(__cplusplus)
}
#endif

#endif /* EWDG_DRIVER_H */

/*! @}*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
