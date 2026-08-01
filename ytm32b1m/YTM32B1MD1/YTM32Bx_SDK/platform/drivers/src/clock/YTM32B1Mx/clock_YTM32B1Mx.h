/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file clock_YTM32B1Mx.h
 * @version 1.4.1
 *
 * @brief Clock Manager — YTM32B1Mx device-specific type definitions and API.
 *
 * This header defines the configuration structures, enumerations, and function
 * prototypes used by the Clock Manager on the YTM32B1Mx device family. It
 * covers the following subsystems:
 *   - SCU clock source, divider, oscillator, and PLL configuration.
 *   - IPC peripheral clock gate, source, and divider configuration.
 *   - CMU clock monitor channel configuration.
 *   - Clock Manager framework (multi-config switching with callbacks).
 *
 * @note Application code should include clock_manager.h (which includes this
 *       file via clock.h dispatch) rather than including this header directly.
 */

#ifndef CLOCK_YTM32B1Mx_H
#define CLOCK_YTM32B1Mx_H

#include "device_registers.h"
#include "status.h"

/*!
 * @addtogroup clock_manager_YTM32B1Mx
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if !(defined(CMU_DEFAULT_OFF))
#define CMU_ALWAYS_ON
#endif /* CMU_DEFAULT_OFF */

/*! @brief Peripheral features list.
 *
 * Constant array storing the mappings between clock names of the peripherals
 * and their associated feature bitmask.
 */
extern const uint8_t peripheralFeaturesList[CLOCK_NAME_COUNT];


/*! @brief EXTAL0 clock frequency (Hz), set by the application at startup. */
extern uint32_t g_xtal0ClkFreq;

/*! @brief Maximum number of system clock divider stages. */
#define SYS_CLK_MAX_NO    3U
/*! @brief Index for the system (core) clock divider. */
#define SYSTEM_CLK_INDEX  0U
/*! @brief Index for the bus clock divider. */
#define BUS_CLK_INDEX     1U

/*******************************************************************************
 * Oscillator & PLL Configuration Types
 ******************************************************************************/
/*!
 * @name Oscillator & PLL Configuration Types
 * @brief Structures and enumerations for FXOSC, SXOSC, and PLL configuration.
 * @{
 */

/*!
 * @brief SCU clock output configuration.
 *
 * Configures the clock output (CLKOUT) pin source and divider.
 */
typedef struct
{
    bool              enable;           /*!< Enable clock output pin.              */
    uint8_t           divider;          /*!< Clock output divider (0 = /1).        */
    scu_clkout_src_t  source;           /*!< Clock output source selection.        */
} scu_clock_out_config_t;

/*!
 * @brief SCU fast external oscillator (FXOSC) configuration.
 *
 * | Field            | Description                                               |
 * |------------------|-----------------------------------------------------------|
 * | enable           | Enable FXOSC.                                             |
 * | bypassMode       | true = external clock input; false = crystal mode.        |
 * | gainSelection    | Oscillator gain selection value.                          |
 * | frequency        | External crystal / clock frequency in Hz.                 |
 * | autoGainDisable  | true = fixed gain (ALC disabled); false = auto gain.      |
 */
typedef struct
{
    bool enable;                       /*!< FXOSC enable.                                        */
    bool bypassMode;                   /*!< FXOSC bypass mode (external clock input).            */
    uint8_t gainSelection;             /*!< FXOSC gain selection.                                */
    uint32_t frequency;                /*!< FXOSC input frequency in Hz.                         */
    bool autoGainDisable;              /*!< Disable automatic gain control (ALCDIS). false = ALC enabled. */
} scu_fxosc_config_t;

/*!
 * @brief SCU slow external oscillator (SXOSC) configuration.
 *
 * | Field           | Description                                         |
 * |-----------------|-----------------------------------------------------|
 * | enable          | Enable SXOSC.                                       |
 * | gainSelection   | Gain selection (if SCU_SXOSC_CTRL_GMSEL supported). |
 * | bypassMode      | true = external clock input; false = crystal mode.  |
 * | deepsleepEnable | Keep SXOSC running in deep-sleep mode.              |
 * | standbyEnable   | Keep SXOSC running in standby mode.                 |
 * | powerdownEnable | Keep SXOSC running in power-down mode.              |
 * | frequency       | SXOSC frequency in Hz (typically 32.768 kHz).       |
 */
typedef struct
{
    bool enable;                       /*!< SXOSC enable.                          */
#if defined(SCU_SXOSC_CTRL_GMSEL_MASK)
    uint8_t gainSelection;             /*!< SXOSC gain selection.                  */
#endif /* SCU_SXOSC_CTRL_GMSEL_MASK */
    bool bypassMode;                   /*!< SXOSC bypass mode.                     */
    bool deepsleepEnable;              /*!< Keep SXOSC on in deep-sleep.           */
    bool standbyEnable;                /*!< Keep SXOSC on in standby.              */
    bool powerdownEnable;              /*!< Keep SXOSC on in power-down.           */
    uint32_t frequency;                /*!< SXOSC frequency in Hz.                 */
} scu_sxosc_config_t;

/*!
 * @brief PLL reference clock source selection.
 *
 * | Value                 | Description                     |
 * |-----------------------|---------------------------------|
 * | SCU_PLL_REF_FXOSC_CLK | Use FXOSC as PLL reference.    |
 * | SCU_PLL_REF_FIRC_CLK  | Use FIRC as PLL reference.     |
 */
typedef enum
{
    SCU_PLL_REF_FXOSC_CLK      = 0U,     /*!< PLL reference: FXOSC. */
    SCU_PLL_REF_FIRC_CLK       = 1U,     /*!< PLL reference: FIRC.  */
} scu_pll_ref_clk_t;

/*!
 * @brief SCU PLL configuration.
 *
 * PLL output frequency: Fout = Fref × pllFeedBackDiv / (2 × pllRefClkDiv).
 *
 * | Field          | Description                                    |
 * |----------------|------------------------------------------------|
 * | enable         | Enable PLL.                                    |
 * | pllRefClock    | PLL reference clock source.                    |
 * | pllFeedBackDiv | PLL feedback divider (Ndiv).                   |
 * | pllRefClkDiv   | PLL reference clock divider (Npre).            |
 */
typedef struct
{
    bool enable;                   /*!< PLL enable.                       */
    scu_pll_ref_clk_t pllRefClock; /*!< PLL reference clock source.       */
    uint8_t pllFeedBackDiv;        /*!< PLL feedback divider (Ndiv).      */
    uint8_t pllRefClkDiv;          /*!< PLL reference clock divider (Npre). */
} scu_pll_config_t;

/*! @} */ /* End of Oscillator & PLL Configuration Types */

/*******************************************************************************
 * System Clock Configuration Types
 ******************************************************************************/
/*!
 * @name System Clock Configuration Types
 * @brief System clock source selection and top-level SCU configuration.
 * @{
 */

/*!
 * @brief System clock source enumeration.
 *
 * | Value                       | Description              |
 * |-----------------------------|--------------------------|
 * | SCU_SYSTEM_CLOCK_SRC_FIRC   | Fast Internal RC (48 MHz). |
 * | SCU_SYSTEM_CLOCK_SRC_PLL    | PLL output (if supported). |
 * | SCU_SYSTEM_CLOCK_SRC_FXOSC  | Fast External Oscillator.  |
 * | SCU_SYSTEM_CLOCK_SRC_SIRC   | Slow Internal RC (8 MHz).  |
 * | SCU_SYSTEM_CLOCK_SRC_NONE   | No source (sentinel).      |
 */
typedef enum {
    SCU_SYSTEM_CLOCK_SRC_FIRC     = 0U,       /*!< Fast IRC (48 MHz).   */
#if (defined(FEATURE_SCU_SUPPORT_PLL) && FEATURE_SCU_SUPPORT_PLL)
    SCU_SYSTEM_CLOCK_SRC_PLL      = 1U,       /*!< PLL output clock.    */
#endif /* FEATURE_SCU_SUPPORT_PLL */
    SCU_SYSTEM_CLOCK_SRC_FXOSC    = 2U,       /*!< External oscillator. */
    SCU_SYSTEM_CLOCK_SRC_SIRC     = 3U,       /*!< Slow IRC (8 MHz).    */
    SCU_SYSTEM_CLOCK_SRC_NONE     = 255U      /*!< No source selected.  */
} scu_system_clock_src_t;

/*!
 * @brief SCU top-level configuration structure.
 *
 * Aggregates all system clock parameters: IRC enable/low-power settings,
 * system clock source, oscillator and PLL configuration, dividers, and
 * clock output routing.
 */
typedef struct
{
    bool                      sircDeepSleepEnable;              /*!< Keep SIRC on in deep-sleep.                  */
    bool                      sircStandbyEnable;                /*!< Keep SIRC on in standby.                     */
    bool                      sircPowerDownEnable;              /*!< Keep SIRC on in power-down.                  */
    bool                      fircEnable;                       /*!< Enable FIRC (48 MHz).                        */
    bool                      fircDeepSleepEnable;              /*!< Keep FIRC on in deep-sleep.                  */
    scu_system_clock_src_t    sysClkSrc;                        /*!< System clock source selection.               */
    scu_fxosc_config_t        fxoscConfig;                      /*!< Fast external oscillator configuration.      */
#if FEATURE_SCU_SUPPORT_SXOSC
    scu_sxosc_config_t        sxoscConfig;                      /*!< Slow external oscillator configuration.      */
#endif /* FEATURE_SCU_SUPPORT_SXOSC */
#if (defined(FEATURE_SCU_SUPPORT_PLL) && FEATURE_SCU_SUPPORT_PLL)
    scu_pll_config_t          pllConfig;                        /*!< PLL configuration.                           */
#endif /* FEATURE_SCU_SUPPORT_PLL */
    uint8_t                   sysDiv;                           /*!< System (core) clock divider (0=/1 … 15=/16). */
    uint8_t                   fastBusDiv;                       /*!< Fast bus clock divider.                      */
    uint8_t                   slowBusDiv;                       /*!< Slow bus clock divider.                      */
    uint8_t                   flashDiv;                         /*!< Flash clock divider (EFM RWS).               */
    uint8_t                   flashPrs;                         /*!< Flash clock prescaler.                       */
    scu_clock_out_config_t    clockOutConfig;                   /*!< Clock output pin configuration.              */
} scu_config_t;

/*! @} */ /* End of System Clock Configuration Types */

/*******************************************************************************
 * CMU Configuration Types
 ******************************************************************************/
/*!
 * @name CMU Configuration Types
 * @brief Clock Monitor Unit reference clock and channel configuration.
 * @{
 */

/*!
 * @brief CMU reference clock source.
 *
 * | Value            | Description                                    |
 * |------------------|------------------------------------------------|
 * | CMU_REF_SIRC_CLOCK  | Use SIRC as CMU reference clock.            |
 * | CMU_REF_FXOSC_CLOCK | Use FXOSC as CMU reference clock.           |
 */
typedef enum
{
    CMU_REF_SIRC_CLOCK  = 0, /*!< SIRC reference clock.  */
    CMU_REF_FXOSC_CLOCK = 1, /*!< FXOSC reference clock. */
} cmu_refclk_t;

/*!
 * @brief CMU channel configuration.
 *
 * | Field       | Description                                              |
 * |-------------|----------------------------------------------------------|
 * | enable      | Enable this monitoring channel.                          |
 * | resetEnable | Generate system reset on clock error.                    |
 * | refClock    | Reference clock source (SIRC or FXOSC).                  |
 * | compareHigh | Upper frequency threshold (counter compare value).       |
 * | compareLow  | Lower frequency threshold (counter compare value).       |
 */
typedef struct
{
    bool enable;                 /*!< Channel enable.                      */
    bool resetEnable;            /*!< Reset-on-error enable.               */
    cmu_refclk_t refClock;       /*!< CMU reference clock selection.       */
    uint16_t compareHigh;        /*!< Counter compare high threshold.      */
    uint16_t compareLow;         /*!< Counter compare low threshold.       */
} cmu_ch_config_t;

/*!
 * @brief CMU configuration — aggregates all monitoring channels.
 */
typedef struct
{
#if (defined(FEATURE_SCU_SUPPORT_PLL) && FEATURE_SCU_SUPPORT_PLL)
    cmu_ch_config_t slowBusMonitor;            /*!< Slow bus clock monitor channel.   */
#endif /* FEATURE_SCU_SUPPORT_PLL */
    cmu_ch_config_t fircClockMonitor;          /*!< FIRC clock monitor channel.       */
#if (defined(FEATURE_SCU_SUPPORT_PLL) && FEATURE_SCU_SUPPORT_PLL)
    cmu_ch_config_t pllClockMonitor;           /*!< PLL clock monitor channel.        */
#endif /* FEATURE_SCU_SUPPORT_PLL */
    cmu_ch_config_t fxoscClockMonitor;         /*!< FXOSC clock monitor channel.      */
} cmu_config_t;

/*! @} */ /* End of CMU Configuration Types */

/*******************************************************************************
 * Clock Divider Types
 ******************************************************************************/
/*!
 * @name Clock Divider Types
 * @brief System clock and peripheral clock divider enumerations.
 * @{
 */

/*!
 * @brief System clock divider values (core / fast-bus / slow-bus).
 */
typedef enum {
    SCU_SYS_CLK_DIV_BY_1  = 0,     /*!< Divided by 1.  */
    SCU_SYS_CLK_DIV_BY_2  = 1,     /*!< Divided by 2.  */
    SCU_SYS_CLK_DIV_BY_3  = 2,     /*!< Divided by 3.  */
    SCU_SYS_CLK_DIV_BY_4  = 3,     /*!< Divided by 4.  */
    SCU_SYS_CLK_DIV_BY_5  = 4,     /*!< Divided by 5.  */
    SCU_SYS_CLK_DIV_BY_6  = 5,     /*!< Divided by 6.  */
    SCU_SYS_CLK_DIV_BY_7  = 6,     /*!< Divided by 7.  */
    SCU_SYS_CLK_DIV_BY_8  = 7,     /*!< Divided by 8.  */
    SCU_SYS_CLK_DIV_BY_9  = 8,     /*!< Divided by 9.  */
    SCU_SYS_CLK_DIV_BY_10 = 9,     /*!< Divided by 10. */
    SCU_SYS_CLK_DIV_BY_11 = 10,    /*!< Divided by 11. */
    SCU_SYS_CLK_DIV_BY_12 = 11,    /*!< Divided by 12. */
    SCU_SYS_CLK_DIV_BY_13 = 12,    /*!< Divided by 13. */
    SCU_SYS_CLK_DIV_BY_14 = 13,    /*!< Divided by 14. */
    SCU_SYS_CLK_DIV_BY_15 = 14,    /*!< Divided by 15. */
    SCU_SYS_CLK_DIV_BY_16 = 15,    /*!< Divided by 16. */
} system_clock_divider_t;

/*!
 * @brief IPC clock divider values (SCU-level IPC clock prescaler).
 */
typedef enum
{
    SCU_IPC_CLK_DIV_BY_1  = 0,     /*!< Divided by 1.  */
    SCU_IPC_CLK_DIV_BY_2  = 1,     /*!< Divided by 2.  */
    SCU_IPC_CLK_DIV_BY_3  = 2,     /*!< Divided by 3.  */
    SCU_IPC_CLK_DIV_BY_4  = 3,     /*!< Divided by 4.  */
    SCU_IPC_CLK_DIV_BY_5  = 4,     /*!< Divided by 5.  */
    SCU_IPC_CLK_DIV_BY_6  = 5,     /*!< Divided by 6.  */
    SCU_IPC_CLK_DIV_BY_7  = 6,     /*!< Divided by 7.  */
    SCU_IPC_CLK_DIV_BY_8  = 7,     /*!< Divided by 8.  */
    SCU_IPC_CLK_DIV_BY_9  = 8,     /*!< Divided by 9.  */
    SCU_IPC_CLK_DIV_BY_10 = 9,     /*!< Divided by 10. */
    SCU_IPC_CLK_DIV_BY_11 = 10,    /*!< Divided by 11. */
    SCU_IPC_CLK_DIV_BY_12 = 11,    /*!< Divided by 12. */
    SCU_IPC_CLK_DIV_BY_13 = 12,    /*!< Divided by 13. */
    SCU_IPC_CLK_DIV_BY_14 = 13,    /*!< Divided by 14. */
    SCU_IPC_CLK_DIV_BY_15 = 14,    /*!< Divided by 15. */
    SCU_IPC_CLK_DIV_BY_16 = 15,    /*!< Divided by 16. */
} ipc_clock_divider_t;

/*!
 * @brief Peripheral clock (IPC) divider values.
 */
typedef enum
{
    DIV_BY_1  =  0U,        /*!< Divided by 1.  */
    DIV_BY_2  =  1U,        /*!< Divided by 2.  */
    DIV_BY_3  =  2U,        /*!< Divided by 3.  */
    DIV_BY_4  =  3U,        /*!< Divided by 4.  */
    DIV_BY_5  =  4U,        /*!< Divided by 5.  */
    DIV_BY_6  =  5U,        /*!< Divided by 6.  */
    DIV_BY_7  =  6U,        /*!< Divided by 7.  */
    DIV_BY_8  =  7U,        /*!< Divided by 8.  */
    DIV_BY_9  =  8U,        /*!< Divided by 9.  */
    DIV_BY_10 =  9U,        /*!< Divided by 10. */
    DIV_BY_11 = 10U,        /*!< Divided by 11. */
    DIV_BY_12 = 11U,        /*!< Divided by 12. */
    DIV_BY_13 = 12U,        /*!< Divided by 13. */
    DIV_BY_14 = 13U,        /*!< Divided by 14. */
    DIV_BY_15 = 14U,        /*!< Divided by 15. */
    DIV_BY_16 = 15U         /*!< Divided by 16. */
} peripheral_clock_divider_t;

/*! @} */ /* End of Clock Divider Types */

/*******************************************************************************
 * Peripheral Clock Configuration Types
 ******************************************************************************/
/*!
 * @name Peripheral Clock Configuration Types
 * @brief Structures for per-peripheral IPC clock gate, source, and divider.
 * @{
 */

/*!
 * @brief IPC peripheral clock configuration for a single peripheral.
 *
 * | Field   | Description                                                    |
 * |---------|----------------------------------------------------------------|
 * | clkName | Peripheral clock name (e.g., IPC_SPI0_CLOCK).                 |
 * | clkGate | true = enable peripheral clock; false = gate (disable) it.    |
 * | clkSrc  | Peripheral clock source (FIRC / SIRC / FXOSC / PLL / etc.).   |
 * | divider | Peripheral clock divider value (DIV_BY_1 … DIV_BY_16).        |
 */
typedef struct
{
    clock_names_t clkName;                                 /*!< Peripheral clock name from clock_names_t.   */
    bool clkGate;                                          /*!< Peripheral clock gate (true = enabled).     */
    peripheral_clock_source_t clkSrc;                      /*!< Peripheral clock source.                    */
    peripheral_clock_divider_t divider;                    /*!< Peripheral clock divider value.              */
} peripheral_clock_config_t;

/*!
 * @brief IPC configuration — array of peripheral clock configurations.
 */
typedef struct
{
    uint32_t count;                                        /*!< Number of peripherals to configure.                  */
    peripheral_clock_config_t const * peripheralClocks;    /*!< Pointer to peripheral clock configuration array.     */
} ipc_config_t;

/*! @} */ /* End of Peripheral Clock Configuration Types */

/*******************************************************************************
 * Clock Manager Framework Types
 ******************************************************************************/
/*!
 * @name Clock Manager Framework Types
 * @brief Top-level configuration, multi-config switching, and callback types.
 * @{
 */

/*!
 * @brief Top-level clock configuration structure.
 *
 * Combines IPC peripheral clocks, SCU system clocks, and CMU monitoring
 * into a single configuration. Passed to CLOCK_DRV_Init().
 */
typedef struct
{
    ipc_config_t                 ipcConfig;       /*!< IPC peripheral clock configuration.   */
    scu_config_t const           *scuConfigPtr;   /*!< SCU system clock configuration.       */
    cmu_config_t const           *cmuConfigPtr;   /*!< CMU clock monitoring configuration.   */
} clock_manager_user_config_t;

/*! @brief Convenience alias for the clock configuration structure. */
typedef clock_manager_user_config_t clock_user_config_t;

/*!
 * @brief System clock configuration (runtime snapshot).
 *
 * Stores the current system clock source and divider values.
 */
typedef struct
{
    clock_names_t src;                         /*!< System clock source.                                */
    uint16_t dividers[SYS_CLK_MAX_NO];         /*!< System clock dividers (0 = undivided).              */
} sys_clk_config_t;

/*!
 * @brief Clock source configuration (for external oscillators).
 *
 * Generic structure for describing a clock source with optional output
 * dividers, primarily used for external oscillator configuration.
 */
typedef struct
{
	bool       enable;                   /*!< Enable or disable this clock source.                     */
    uint32_t   refFreq;                  /*!< Input reference clock frequency in Hz.                   */
	uint16_t   outputDiv1;               /*!< First output divider. 0 = disabled. Valid: 1,2,4,8,16,32,64.   /
										  *   Used as protocol clock by peripheral modules.                  /
										  */
	uint16_t   outputDiv2;               /*!< Second output divider. 0 = disabled. Valid: 1,2,4,8,16,32,64.  /
										  *   Used as protocol clock by peripheral modules.                  /
										  */

} clock_source_config_t;

/*!
 * @brief Clock notification type — indicates the phase of a clock transition.
 *
 * | Value                         | Description                                    |
 * |-------------------------------|------------------------------------------------|
 * | CLOCK_MANAGER_NOTIFY_RECOVER  | Recover to previous state (transition aborted).|
 * | CLOCK_MANAGER_NOTIFY_BEFORE   | About to change clock setting.                 |
 * | CLOCK_MANAGER_NOTIFY_AFTER    | Clock setting change complete.                 |
 */
typedef enum
{
    CLOCK_MANAGER_NOTIFY_RECOVER = 0x00U,  /*!< Recover to previous work state.          */
    CLOCK_MANAGER_NOTIFY_BEFORE  = 0x01U,  /*!< System will change clock setting.        */
    CLOCK_MANAGER_NOTIFY_AFTER   = 0x02U,  /*!< System has changed to new clock setting. */
} clock_manager_notify_t;

/*!
 * @brief Clock callback type — specifies which notification phases a callback handles.
 *
 * | Value                              | Description                            |
 * |------------------------------------|----------------------------------------|
 * | CLOCK_MANAGER_CALLBACK_BEFORE       | Handle BEFORE notification only.      |
 * | CLOCK_MANAGER_CALLBACK_AFTER        | Handle AFTER notification only.       |
 * | CLOCK_MANAGER_CALLBACK_BEFORE_AFTER | Handle both BEFORE and AFTER.         |
 */
typedef enum
{
    CLOCK_MANAGER_CALLBACK_BEFORE       = 0x01U, /*!< BEFORE callback.           */
    CLOCK_MANAGER_CALLBACK_AFTER        = 0x02U, /*!< AFTER callback.            */
    CLOCK_MANAGER_CALLBACK_BEFORE_AFTER = 0x03U  /*!< BEFORE and AFTER callback. */
} clock_manager_callback_type_t;

/*!
 * @brief Clock transition policy — graceful vs. forceful switching.
 *
 * | Value                          | Description                              |
 * |--------------------------------|------------------------------------------|
 * | CLOCK_MANAGER_POLICY_AGREEMENT | Graceful: all callbacks must agree.      |
 * | CLOCK_MANAGER_POLICY_FORCIBLE  | Forceful: switch regardless of callbacks.|
 */
typedef enum
{
    CLOCK_MANAGER_POLICY_AGREEMENT,  /*!< Graceful clock transition. */
    CLOCK_MANAGER_POLICY_FORCIBLE    /*!< Forceful clock transition. */
} clock_manager_policy_t;

/*!
 * @brief Clock notification structure passed to callback functions.
 */
typedef struct
{
    uint8_t targetClockConfigIndex;    /*!< Target clock configuration index. */
    clock_manager_policy_t policy;     /*!< Clock transition policy.          */
    clock_manager_notify_t notifyType; /*!< Notification type (phase).        */
} clock_notify_struct_t;

/*!
 * @brief Callback function prototype for clock transition notifications.
 *
 * @param[in] notify        Pointer to the notification structure.
 * @param[in] callbackData  User-defined data passed to the callback.
 * @return STATUS_SUCCESS if the callback agrees to the transition.
 */
typedef status_t (*clock_manager_callback_t)(clock_notify_struct_t *notify,
                                             void* callbackData);

/*!
 * @brief Callback registration structure.
 */
typedef struct
{
    clock_manager_callback_t      callback;      /*!< Callback function pointer.   */
    clock_manager_callback_type_t callbackType;  /*!< Notification types handled.  */
    void* callbackData;                          /*!< User-defined callback data.  */
} clock_manager_callback_user_config_t;

/*!
 * @brief Clock Manager internal state.
 */
typedef struct
{
    clock_manager_user_config_t const **configTable;        /*!< Configuration table pointer.      */
    uint8_t clockConfigNum;                                 /*!< Number of configurations.         */
    uint8_t curConfigIndex;                                 /*!< Current active config index.      */
    clock_manager_callback_user_config_t **callbackConfig;  /*!< Callback table pointer.           */
    uint8_t callbackNum;                                    /*!< Number of registered callbacks.   */
    uint8_t errorCallbackIndex;                             /*!< Index of callback that returned error. */
} clock_manager_state_t;

/*! @} */ /* End of Clock Manager Framework Types */


/*******************************************************************************
 * API
 ******************************************************************************/

#if defined (__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*******************************************************************************
 * Peripheral Clock Control
 ******************************************************************************/
/*!
 * @name Peripheral Clock Control
 * @brief Functions for controlling individual peripheral clock gates and resets.
 * @{
 */

/*!
 * @brief Assert and de-assert software reset for a peripheral module.
 *
 * Performs a software reset of the peripheral identified by @a clockName
 * using the IPC peripheral clock control register.
 *
 * @param[in] clockName  Peripheral clock identifier (from clock_names_t).
 */
void CLOCK_DRV_ResetModule(clock_names_t clockName);

/*!
 * @brief Configure an individual peripheral clock gate, source, and divider.
 *
 * Sets the clock gate, source selection, and divider for the peripheral
 * identified by @a clockName using the IPC control register.
 *
 * @param[in] clockName  Peripheral clock identifier (from clock_names_t).
 * @param[in] clockGate  true = enable peripheral clock; false = disable.
 * @param[in] clkSrc     Clock source value (cast from peripheral_clock_source_t).
 * @param[in] divider    Divider value (cast from peripheral_clock_divider_t).
 */
void CLOCK_DRV_SetModuleClock(clock_names_t clockName, bool clockGate, uint32_t clkSrc, uint32_t divider);

/*! @} */ /* End of Peripheral Clock Control */

/*******************************************************************************
 * Clock Manager Framework
 ******************************************************************************/
/*!
 * @name Clock Manager Framework
 * @brief Functions for multi-configuration clock management with callback support.
 * @{
 */

/*!
 * @brief Install pre-defined clock configurations and callback table.
 *
 * Registers an array of clock configurations and an array of callback
 * descriptors with the Clock Manager. After calling this function, use
 * CLOCK_SYS_UpdateConfiguration() to switch between configurations.
 *
 * @param[in] clockConfigsPtr  Pointer to the clock configuration pointer array.
 * @param[in] configsNumber    Number of clock configurations in the table.
 * @param[in] callbacksPtr     Pointer to the callback configuration pointer array.
 * @param[in] callbacksNumber  Number of callback configurations in the table.
 * @return Execution status.
 * @retval STATUS_SUCCESS  Configuration table installed successfully.
 */
status_t CLOCK_SYS_Init(clock_manager_user_config_t const **clockConfigsPtr,
                        uint8_t configsNumber,
                        clock_manager_callback_user_config_t **callbacksPtr,
                        uint8_t callbacksNumber);

/*!
 * @brief Switch to a pre-defined clock configuration by index.
 *
 * Transitions the system to the clock configuration at @a targetConfigIndex.
 * Before switching, the Clock Manager notifies all registered BEFORE callbacks.
 * With CLOCK_MANAGER_POLICY_AGREEMENT, if any callback returns an error the
 * switch is aborted. With CLOCK_MANAGER_POLICY_FORCIBLE, the switch proceeds
 * regardless.
 *
 * @param[in] targetConfigIndex  Index into the configuration table.
 * @param[in] policy             Transition policy (graceful or forceful).
 * @return Execution status.
 * @retval STATUS_SUCCESS                 Configuration applied successfully.
 * @retval STATUS_MCU_NOTIFY_BEFORE_ERROR A BEFORE callback rejected the switch.
 *
 * @note This function should only be called in Run mode.
 */
status_t CLOCK_SYS_UpdateConfiguration(uint8_t targetConfigIndex,
                                       clock_manager_policy_t policy);

/*!
 * @brief Apply a clock configuration directly (without callback notifications).
 *
 * Programs the clock hardware registers for the given configuration.
 * Unlike CLOCK_SYS_UpdateConfiguration(), this function does not invoke
 * any registered callbacks.
 *
 * @param[in] config  Pointer to the clock configuration to apply.
 * @return Execution status.
 *
 * @note This function should only be called in Run mode.
 */
status_t CLOCK_SYS_SetConfiguration(clock_manager_user_config_t const * config);

/*! @} */ /* End of Clock Manager Framework */

/*******************************************************************************
 * Frequency Query & Status
 ******************************************************************************/
/*!
 * @name Frequency Query & Status
 * @brief Functions for querying clock frequencies and manager state.
 * @{
 */

/*!
 * @brief Get the index of the currently active clock configuration.
 *
 * @return Index into the configuration table installed via CLOCK_SYS_Init().
 */
uint8_t CLOCK_SYS_GetCurrentConfiguration(void);

/*!
 * @brief Get the callback that returned an error during the last clock switch.
 *
 * When using CLOCK_MANAGER_POLICY_AGREEMENT, if a callback rejects the
 * transition, this function returns a pointer to that callback's descriptor.
 *
 * @return Pointer to the callback configuration that returned error, or NULL.
 */
clock_manager_callback_user_config_t* CLOCK_SYS_GetErrorCallback(void);

/*!
 * @brief Get the frequency of a named clock (legacy wrapper).
 *
 * Wrapper over CLOCK_DRV_GetFreq() for backward compatibility.
 *
 * @param[in]  clockName  Clock identifier (from clock_names_t).
 * @param[out] frequency  Pointer to receive the clock frequency in Hz.
 * @return Execution status.
 * @retval STATUS_SUCCESS       Frequency retrieved successfully.
 * @retval STATUS_UNSUPPORTED   Clock name not supported.
 * @retval STATUS_MCU_GATED_OFF Peripheral clock is gated.
 */
status_t CLOCK_SYS_GetFreq(clock_names_t clockName, uint32_t *frequency);

/*!
 * @brief Wait for FXOSC to become valid after startup or low-power recovery.
 *
 * Polls the FXOSC valid flag and performs an automatic gain sweep if needed.
 * Returns an error status if FXOSC does not become valid within the timeout.
 *
 * @return Execution status.
 * @retval STATUS_SUCCESS                 FXOSC is valid.
 * @retval STATUS_SCU_FXOSC_READY_TIMEOUT FXOSC did not start within timeout.
 */
status_t CLOCK_SYS_WaitFXOSCValid(void);

/*! @} */ /* End of Frequency Query & Status */

/*******************************************************************************
 * Clock Measurement (Device-Dependent)
 ******************************************************************************/
/*!
 * @name Clock Measurement
 * @brief Clock frequency measurement via CFMU hardware (device-dependent).
 * @{
 */

#if defined(FEATURE_SOC_HAS_SEPARATE_CMU_AND_CLKOUT) && (FEATURE_SOC_HAS_SEPARATE_CMU_AND_CLKOUT == 1)
/*!
 * @brief Measure the frequency of a specified clock source via CFMU.
 *
 * Uses the Clock Frequency Measurement Unit (CFMU) hardware to measure
 * the actual frequency of the selected clock source.
 *
 * @param[in]  measuredClkSrc      Clock source to measure.
 * @param[in]  measuredClkDivider  Measurement divider (0 = /1, 1 = /2, … 255 = /256).
 * @param[out] measuredClkFreq     Pointer to receive the measured frequency in Hz.
 * @return Execution status.
 * @retval STATUS_SUCCESS  Measurement completed successfully.
 */
status_t CLOCK_SYS_MeasureClkFreq(scu_clkout_src_t measuredClkSrc, uint8_t measuredClkDivider, uint32_t * const measuredClkFreq);
#endif

/*! @} */ /* End of Clock Measurement */

#if defined (__cplusplus)
}
#endif /* __cplusplus*/

/*! @} */ /* End of clock_manager_YTM32B1Mx group */

#endif /* CLOCK_YTM32B1Mx_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
