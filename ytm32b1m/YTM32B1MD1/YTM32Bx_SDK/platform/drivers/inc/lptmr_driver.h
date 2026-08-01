/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file lptmr_driver.h
 * @version 1.4.1
 *
 * @brief lpTMR Driver — public API for low-power timer and pulse-counter control.
 *
 * This header defines the application-level interface for the lpTMR peripheral.
 * The driver layer resolves instance indices to peripheral base addresses,
 * applies timer or pulse-counter configuration, converts compare values between
 * hardware ticks and microseconds, and exposes runtime helpers for compare
 * flags, interrupts, counter control, and pulse input selection.
 *
 * The APIs are organized into five categories:
 *   - **Initialization & De-initialization**: fill defaults, configure, and reset an instance.
 *   - **Configuration**: apply or read back the active runtime configuration.
 *   - **Compare Value Access**: program or retrieve compare values in ticks or microseconds.
 *   - **Runtime Status & Control**: monitor flags, enable interrupts, and start or stop counting.
 *   - **Pulse Counter Pin Configuration**: select the pulse source and active edge.
 *
 * @note The lpTMR peripheral clock must be enabled before calling driver APIs.
 */

#ifndef lpTMR_DRIVER_H
#define lpTMR_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "status.h"
#include "device_registers.h"

/*!
 * @addtogroup lptmr
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Pulse input source selection for Pulse-Counter Mode.
 *
 * Selects which trigger path increments the lpTMR counter when the module runs
 * in `lpTMR_WORKMODE_PULSECOUNTER`.
 *
 * | Value | Description |
 * |-------|-------------|
 * | `lpTMR_PINSELECT_TMU` | Count pulses from the TMU trigger path. |
 * | `lpTMR_PINSELECT_ALT1` | Count pulses from alternate input 1 when supported by the device. |
 * | `lpTMR_PINSELECT_ALT2` | Count pulses from alternate input 2. |
 * | `lpTMR_PINSELECT_ALT3` | Count pulses from alternate input 3. |
 */
typedef enum
{
    lpTMR_PINSELECT_TMU = 0x00u, /*!< Count pulses from the TMU trigger path. */
#if FEATURE_lpTMR_HAS_INPUT_ALT1_SELECTION
    lpTMR_PINSELECT_ALT1 = 0x01u, /*!< Count pulses from alternate input 1. */
#endif
    lpTMR_PINSELECT_ALT2 = 0x02u, /*!< Count pulses from alternate input 2. */
    lpTMR_PINSELECT_ALT3 = 0x03u  /*!< Count pulses from alternate input 3. */
} lptmr_pinselect_t;

/*!
 * @brief Active edge selection for Pulse-Counter Mode.
 *
 * | Value | Description |
 * |-------|-------------|
 * | `lpTMR_PINPOLARITY_RISING` | Increment on the rising edge of the selected pulse input. |
 * | `lpTMR_PINPOLARITY_FALLING` | Increment on the falling edge of the selected pulse input. |
 */
typedef enum
{
    lpTMR_PINPOLARITY_RISING = 0u, /*!< Count a pulse on the rising edge. */
    lpTMR_PINPOLARITY_FALLING = 1u  /*!< Count a pulse on the falling edge. */
} lptmr_pinpolarity_t;

/*!
 * @brief lpTMR operating mode selection.
 *
 * | Value | Description |
 * |-------|-------------|
 * | `lpTMR_WORKMODE_TIMER` | Increment from the selected lpTMR clock source. |
 * | `lpTMR_WORKMODE_PULSECOUNTER` | Increment from the selected pulse input. |
 */
typedef enum
{
    lpTMR_WORKMODE_TIMER = 0u, /*!< Timer mode driven by the configured clock source. */
    lpTMR_WORKMODE_PULSECOUNTER = 1u  /*!< Pulse-counter mode driven by the selected input. */
} lptmr_workmode_t;

/*!
 * @brief Prescaler selection for Timer Mode or glitch-filter depth for Pulse-Counter Mode.
 *
 * The same field is interpreted differently by the hardware depending on the
 * active work mode:
 *   - In `lpTMR_WORKMODE_TIMER`, it selects the clock prescaler.
 *   - In `lpTMR_WORKMODE_PULSECOUNTER`, it selects the glitch-filter width.
 *
 * | Value | Timer Mode Meaning | Pulse-Counter Meaning |
 * |-------|--------------------|-----------------------|
 * | `lpTMR_PRESCALE_2` | Divide clock by 2 | Invalid glitch-filter setting |
 * | `lpTMR_PRESCALE_4_GLITCHFILTER_2` | Divide clock by 4 | Filter pulses shorter than 2 clocks |
 * | `lpTMR_PRESCALE_8_GLITCHFILTER_4` | Divide clock by 8 | Filter pulses shorter than 4 clocks |
 * | `lpTMR_PRESCALE_16_GLITCHFILTER_8` | Divide clock by 16 | Filter pulses shorter than 8 clocks |
 * | `lpTMR_PRESCALE_32_GLITCHFILTER_16` | Divide clock by 32 | Filter pulses shorter than 16 clocks |
 * | `lpTMR_PRESCALE_64_GLITCHFILTER_32` | Divide clock by 64 | Filter pulses shorter than 32 clocks |
 * | `lpTMR_PRESCALE_128_GLITCHFILTER_64` | Divide clock by 128 | Filter pulses shorter than 64 clocks |
 * | `lpTMR_PRESCALE_256_GLITCHFILTER_128` | Divide clock by 256 | Filter pulses shorter than 128 clocks |
 * | `lpTMR_PRESCALE_512_GLITCHFILTER_256` | Divide clock by 512 | Filter pulses shorter than 256 clocks |
 * | `lpTMR_PRESCALE_1024_GLITCHFILTER_512` | Divide clock by 1024 | Filter pulses shorter than 512 clocks |
 * | `lpTMR_PRESCALE_2048_GLITCHFILTER_1024` | Divide clock by 2048 | Filter pulses shorter than 1024 clocks |
 * | `lpTMR_PRESCALE_4096_GLITCHFILTER_2048` | Divide clock by 4096 | Filter pulses shorter than 2048 clocks |
 * | `lpTMR_PRESCALE_8192_GLITCHFILTER_4096` | Divide clock by 8192 | Filter pulses shorter than 4096 clocks |
 * | `lpTMR_PRESCALE_16384_GLITCHFILTER_8192` | Divide clock by 16384 | Filter pulses shorter than 8192 clocks |
 * | `lpTMR_PRESCALE_32768_GLITCHFILTER_16384` | Divide clock by 32768 | Filter pulses shorter than 16384 clocks |
 * | `lpTMR_PRESCALE_65536_GLITCHFILTER_32768` | Divide clock by 65536 | Filter pulses shorter than 32768 clocks |
 */
typedef enum
{
    lpTMR_PRESCALE_2 = 0x00u, /*!< Timer prescaler divide-by-2; glitch filter setting is invalid. */
    lpTMR_PRESCALE_4_GLITCHFILTER_2 = 0x01u, /*!< Timer prescaler divide-by-4; glitch filter length 2 clocks. */
    lpTMR_PRESCALE_8_GLITCHFILTER_4 = 0x02u, /*!< Timer prescaler divide-by-8; glitch filter length 4 clocks. */
    lpTMR_PRESCALE_16_GLITCHFILTER_8 = 0x03u, /*!< Timer prescaler divide-by-16; glitch filter length 8 clocks. */
    lpTMR_PRESCALE_32_GLITCHFILTER_16 = 0x04u, /*!< Timer prescaler divide-by-32; glitch filter length 16 clocks. */
    lpTMR_PRESCALE_64_GLITCHFILTER_32 = 0x05u, /*!< Timer prescaler divide-by-64; glitch filter length 32 clocks. */
    lpTMR_PRESCALE_128_GLITCHFILTER_64 = 0x06u, /*!< Timer prescaler divide-by-128; glitch filter length 64 clocks. */
    lpTMR_PRESCALE_256_GLITCHFILTER_128 = 0x07u, /*!< Timer prescaler divide-by-256; glitch filter length 128 clocks. */
    lpTMR_PRESCALE_512_GLITCHFILTER_256 = 0x08u, /*!< Timer prescaler divide-by-512; glitch filter length 256 clocks. */
    lpTMR_PRESCALE_1024_GLITCHFILTER_512 = 0x09u, /*!< Timer prescaler divide-by-1024; glitch filter length 512 clocks. */
    lpTMR_PRESCALE_2048_GLITCHFILTER_1024 = 0x0Au, /*!< Timer prescaler divide-by-2048; glitch filter length 1024 clocks. */
    lpTMR_PRESCALE_4096_GLITCHFILTER_2048 = 0x0Bu, /*!< Timer prescaler divide-by-4096; glitch filter length 2048 clocks. */
    lpTMR_PRESCALE_8192_GLITCHFILTER_4096 = 0x0Cu, /*!< Timer prescaler divide-by-8192; glitch filter length 4096 clocks. */
    lpTMR_PRESCALE_16384_GLITCHFILTER_8192 = 0x0Du, /*!< Timer prescaler divide-by-16384; glitch filter length 8192 clocks. */
    lpTMR_PRESCALE_32768_GLITCHFILTER_16384 = 0x0Eu, /*!< Timer prescaler divide-by-32768; glitch filter length 16384 clocks. */
    lpTMR_PRESCALE_65536_GLITCHFILTER_32768 = 0x0Fu  /*!< Timer prescaler divide-by-65536; glitch filter length 32768 clocks. */
} lptmr_prescaler_t;

/*!
 * @brief Units used to interpret or report the compare value.
 *
 * | Value | Description |
 * |-------|-------------|
 * | `lpTMR_COUNTER_UNITS_TICKS` | Compare value is expressed in raw counter ticks. |
 * | `lpTMR_COUNTER_UNITS_MICROSECONDS` | Compare value is expressed in microseconds and converted by the driver. |
 */
typedef enum
{
    lpTMR_COUNTER_UNITS_TICKS = 0x00U, /*!< Compare value is expressed in counter ticks. */
    lpTMR_COUNTER_UNITS_MICROSECONDS = 0x01U /*!< Compare value is expressed in microseconds. */
} lptmr_counter_units_t;

#if defined(FEATURE_lpTMR_HAS_CLOCK_SELECTION) && (FEATURE_lpTMR_HAS_CLOCK_SELECTION == 1U)
/*!
 * @brief lpTMR counter clock source selection.
 *
 * The available enum entries depend on the device feature macros enabled for
 * the current target.
 */
typedef enum
{
#if defined(FEATURE_lpTMR_CLKSRC_SUPPORT_FIRC) && (FEATURE_lpTMR_CLKSRC_SUPPORT_FIRC == 1U)
    lpTMR_CLOCK_SOURCE_FIRC = 0x00U, /*!< Fast internal RC clock source. */
#endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_FIRC */
#if defined(FEATURE_lpTMR_CLKSRC_SUPPORT_IPC) && (FEATURE_lpTMR_CLKSRC_SUPPORT_IPC == 1U)
    lpTMR_CLOCK_SOURCE_IPC = 0x00U, /*!< Instance peripheral clock source. */
#endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_IPC */
#if defined(FEATURE_lpTMR_CLKSRC_SUPPORT_SIRC) && (FEATURE_lpTMR_CLKSRC_SUPPORT_SIRC == 1U)
    lpTMR_CLOCK_SOURCE_SIRC = 0x01U, /*!< Slow internal RC clock source. */
#endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_SIRC */
#if defined(FEATURE_lpTMR_CLKSRC_SUPPORT_SIRC_DIV4) && (FEATURE_lpTMR_CLKSRC_SUPPORT_SIRC_DIV4 == 1U)
    lpTMR_CLOCK_SOURCE_SIRC_DIV4 = 0x01U, /*!< Divided slow internal RC clock source. */
#endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_SIRC_DIV4 */
#if defined(FEATURE_lpTMR_CLKSRC_SUPPORT_SXOSC) && (FEATURE_lpTMR_CLKSRC_SUPPORT_SXOSC == 1U)
    lpTMR_CLOCK_SOURCE_SXOSC = 0x02U, /*!< Secondary crystal oscillator source. */
#endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_SXOSC */
#if defined(FEATURE_lpTMR_CLKSRC_SUPPORT_FXOSC) && (FEATURE_lpTMR_CLKSRC_SUPPORT_FXOSC == 1U)
    lpTMR_CLOCK_SOURCE_FXOSC = 0x02U, /*!< Fast crystal oscillator source. */
#endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_FXOSC */
#if defined(FEATURE_lpTMR_CLKSRC_SUPPORT_LPO) && (FEATURE_lpTMR_CLKSRC_SUPPORT_LPO == 1U)
#if defined(MCU_YTM32Z1MD0)
    lpTMR_CLOCK_SOURCE_OSC = 0x00U, /*!< Device oscillator clock source. */
    lpTMR_CLOCK_SOURCE_LPO = 0x01U, /*!< Low-power oscillator clock source. */
#else
    lpTMR_CLOCK_SOURCE_LPO = 0x03U, /*!< Low-power oscillator clock source. */
#endif /* MCU_YTM32Z1MD0 */
#endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_LPO */
} lptmr_clock_source_t;
#endif /* FEATURE_lpTMR_HAS_CLOCK_SELECTION */

/*!
 * @brief User configuration structure for lpTMR driver initialization.
 *
 * This structure captures all runtime settings applied by `lpTMR_DRV_Init()`
 * and `lpTMR_DRV_SetConfig()`.
 *
 * | Field | Description |
 * |-------|-------------|
 * | `dmaRequest` | Enables DMA requests on compare match when the device supports it. |
 * | `interruptEnable` | Enables compare-match interrupts. |
 * | `freeRun` | Selects overflow-based free-running operation. |
 * | `workMode` | Selects Timer Mode or Pulse-Counter Mode. |
 * | `prescaler` | Selects the timer prescaler or glitch-filter depth. |
 * | `bypassPrescaler` | Bypasses the prescaler or glitch filter when set. |
 * | `compareValue` | Requested compare value in ticks or microseconds. |
 * | `counterUnits` | Selects how `compareValue` is interpreted. |
 * | `pinSelect` | Selects the pulse input source. |
 * | `pinPolarity` | Selects the counted pulse edge. |
 * | `clockSource` | Selects the counter clock on supported devices. |
 */
typedef struct
{
    /* General parameters */
#if defined(lpTMR_DIE_DMAEN_MASK)
    bool dmaRequest;                    /*!< Enables DMA requests on compare match. */
#endif /* lpTMR_DIE_DMAEN_MASK */
    bool interruptEnable;               /*!< Enables compare-match interrupts. */
    bool freeRun;                       /*!< Enables overflow-based free-running operation. */
    lptmr_workmode_t workMode;          /*!< Selects Timer Mode or Pulse-Counter Mode. */
    /* Counter parameters */
    lptmr_prescaler_t prescaler;        /*!< Selects the timer prescaler or glitch-filter depth. */
    bool bypassPrescaler;               /*!< Bypasses the prescaler or glitch filter when set. */
    uint32_t compareValue;              /*!< Requested compare value in ticks or microseconds. */
    lptmr_counter_units_t counterUnits; /*!< Selects how compareValue is interpreted. */
    /* Pulse Counter specific parameters */
    lptmr_pinselect_t pinSelect;        /*!< Selects the pulse input source. */
    lptmr_pinpolarity_t pinPolarity;    /*!< Selects the counted pulse edge. */
#ifdef FEATURE_lpTMR_HAS_CLOCK_SELECTION
    lptmr_clock_source_t clockSource;   /*!< Selects the timer clock source on supported devices. */
#endif /* FEATURE_lpTMR_HAS_CLOCK_SELECTION */
} lptmr_config_t;

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/
/*!
 * @name Initialization & De-initialization
 * @brief Functions for preparing, starting, and resetting an lpTMR instance.
 * @{
 */

/*!
 * @brief Populate a configuration structure with lpTMR driver defaults.
 *
 * Initializes all fields in `config` with the module defaults used by the
 * driver. The resulting configuration represents Timer Mode in tick units with
 * the counter stopped and all optional features disabled.
 *
 * @param[out] config  Pointer to the configuration structure to initialize.
 *
 * @pre `config` must not be `NULL`.
 * @post `config` contains a complete default lpTMR configuration.
 * @note On devices with clock-source selection, the default `clockSource`
 *       value is platform specific and matches the implementation in
 *       `lpTMR_DRV_InitConfigStruct()`.
 */
void lpTMR_DRV_InitConfigStruct(lptmr_config_t *const config);

/*!
 * @brief Initialize an lpTMR instance from a user configuration.
 *
 * Applies all fields from `config` to the selected instance. When
 * `config->counterUnits` is `lpTMR_COUNTER_UNITS_MICROSECONDS`, the driver
 * converts the requested time period to hardware ticks and automatically
 * selects prescaler settings that best fit the configured timer clock.
 *
 * @param[in] instance      lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 * @param[in] config        Pointer to the user configuration structure.
 * @param[in] startCounter  If `true`, the counter is enabled immediately after configuration.
 *
 * @pre The lpTMR peripheral clock must be enabled.
 * @pre `config` must not be `NULL`.
 * @post The selected instance is configured according to `config`.
 * @note `lpTMR_COUNTER_UNITS_MICROSECONDS` is valid only in `lpTMR_WORKMODE_TIMER`.
 * @warning In Pulse-Counter Mode, `compareValue` must be specified in ticks.
 */
void lpTMR_DRV_Init(const uint32_t instance,
                    const lptmr_config_t *const config,
                    const bool startCounter);

/*!
 * @brief Stop an lpTMR instance and restore register reset values.
 *
 * Disables the counter and reinitializes the underlying peripheral registers
 * to the reset configuration.
 *
 * @param[in] instance  lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 *
 * @pre The lpTMR peripheral clock must be enabled.
 * @post The counter is stopped and the peripheral returns to its default state.
 */
void lpTMR_DRV_Deinit(const uint32_t instance);

/*! @} */ /* End of Initialization & De-initialization */

/*******************************************************************************
 * Configuration
 ******************************************************************************/
/*!
 * @name Configuration
 * @brief Functions for applying or inspecting lpTMR runtime configuration.
 * @{
 */

/*!
 * @brief Apply a new configuration to an lpTMR instance.
 *
 * Reprograms the selected instance using the same conversion rules as
 * `lpTMR_DRV_Init()`. When the compare value is supplied in microseconds, the
 * prescaler selection is recalculated automatically for Timer Mode.
 *
 * @param[in] instance  lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 * @param[in] config    Pointer to the user configuration structure.
 *
 * @pre The lpTMR peripheral clock must be enabled.
 * @pre `config` must not be `NULL`.
 * @post The selected instance reflects the settings in `config`.
 * @note `lpTMR_COUNTER_UNITS_MICROSECONDS` is valid only in `lpTMR_WORKMODE_TIMER`.
 * @warning In Pulse-Counter Mode, the compare value is interpreted only in ticks.
 */
void lpTMR_DRV_SetConfig(const uint32_t instance,
                         const lptmr_config_t *const config);

/*!
 * @brief Read back the active configuration of an lpTMR instance.
 *
 * Populates `config` with the configuration values that can be observed from
 * the hardware registers.
 *
 * @param[in] instance  lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 * @param[out] config   Pointer to the structure that receives the active configuration.
 *
 * @pre `config` must not be `NULL`.
 * @post `config->compareValue` is reported in raw counter ticks.
 * @note This API always reports `counterUnits` as `lpTMR_COUNTER_UNITS_TICKS`.
 */
void lpTMR_DRV_GetConfig(const uint32_t instance,
                         lptmr_config_t *const config);

/*! @} */ /* End of Configuration */

/*******************************************************************************
 * Compare Value Access
 ******************************************************************************/
/*!
 * @name Compare Value Access
 * @brief Functions for programming or retrieving compare values.
 * @{
 */

/*!
 * @brief Program the compare value directly in counter ticks.
 *
 * Updates the compare register with a raw tick value. The write succeeds only
 * when the counter is disabled or when the compare flag is already asserted.
 *
 * @param[in] instance             lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 * @param[in] compareValueByCount  Compare value expressed in counter ticks.
 * @return Execution status.
 * @retval STATUS_SUCCESS  The compare register was updated successfully.
 * @retval STATUS_ERROR    The counter was running and the compare flag was not set.
 * @retval STATUS_TIMEOUT  The new compare value is not greater than the current counter value.
 *
 * @note The caller is responsible for choosing a tick value that matches the
 *       active mode and prescaler settings.
 */
status_t lpTMR_DRV_SetCompareValueByCount(const uint32_t instance,
                                          const uint16_t compareValueByCount);

/*!
 * @brief Read the compare value in counter ticks.
 *
 * @param[in] instance              lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 * @param[out] compareValueByCount  Pointer that receives the active compare value in ticks.
 *
 * @pre `compareValueByCount` must not be `NULL`.
 */
void lpTMR_DRV_GetCompareValueByCount(const uint32_t instance,
                                      uint16_t *const compareValueByCount);

/*!
 * @brief Program the compare value in microseconds for Timer Mode.
 *
 * Converts `compareValueUs` to hardware ticks using the cached timer clock
 * frequency, then updates the compare register and prescaler settings.
 *
 * @param[in] instance        lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 * @param[in] compareValueUs  Requested compare period in microseconds.
 * @return Execution status.
 * @retval STATUS_SUCCESS  The compare period was updated successfully.
 * @retval STATUS_ERROR    The counter was running and the compare flag was not set.
 * @retval STATUS_TIMEOUT  The converted compare value is not greater than the current counter value.
 *
 * @pre The selected instance must be configured in `lpTMR_WORKMODE_TIMER`.
 * @note This API relies on the clock frequency cached during timer configuration.
 * @warning Calling this API before a valid timer clock has been configured triggers `DEV_ASSERT`.
 */
status_t lpTMR_DRV_SetCompareValueByUs(const uint32_t instance,
                                       const uint32_t compareValueUs);

/*!
 * @brief Read the active compare value as microseconds.
 *
 * Converts the current compare and prescaler settings back to a time period in
 * microseconds using the cached timer clock frequency.
 *
 * @param[in] instance         lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 * @param[out] compareValueUs  Pointer that receives the compare period in microseconds.
 *
 * @pre The selected instance must be configured in `lpTMR_WORKMODE_TIMER`.
 * @pre `compareValueUs` must not be `NULL`.
 * @note This API relies on the clock frequency cached during timer configuration.
 */
void lpTMR_DRV_GetCompareValueByUs(const uint32_t instance,
                                   uint32_t *const compareValueUs);

/*! @} */ /* End of Compare Value Access */

/*******************************************************************************
 * Runtime Status & Control
 ******************************************************************************/
/*!
 * @name Runtime Status & Control
 * @brief Functions for reading status flags and controlling counter operation.
 * @{
 */

/*!
 * @brief Read the compare-match flag state.
 *
 * @param[in] instance  lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 * @return `true` if a compare event is pending, `false` otherwise.
 */
bool lpTMR_DRV_GetCompareFlag(const uint32_t instance);

/*!
 * @brief Clear the compare-match flag.
 *
 * @param[in] instance  lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 */
void lpTMR_DRV_ClearCompareFlag(const uint32_t instance);

/*!
 * @brief Report whether the counter is currently enabled.
 *
 * @param[in] instance  lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 * @return `true` when the counter is running, `false` when it is stopped.
 */
bool lpTMR_DRV_IsRunning(const uint32_t instance);

/*!
 * @brief Enable or disable compare interrupts.
 *
 * @param[in] instance         lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 * @param[in] enableInterrupt  `true` to enable compare interrupts, `false` to disable them.
 */
void lpTMR_DRV_SetInterrupt(const uint32_t instance,
                            const bool enableInterrupt);

/*!
 * @brief Read the current counter value in raw ticks.
 *
 * @param[in] instance  lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 * @return Current counter value in ticks.
 */
uint16_t lpTMR_DRV_GetCounterValueByCount(const uint32_t instance);

/*!
 * @brief Enable the lpTMR counter.
 *
 * @param[in] instance  lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 *
 * @note Any mode or prescaler changes must be applied before starting the counter.
 */
void lpTMR_DRV_StartCounter(const uint32_t instance);

/*!
 * @brief Disable the lpTMR counter.
 *
 * @param[in] instance  lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 *
 * @post The instance can be safely reconfigured after the counter stops.
 */
void lpTMR_DRV_StopCounter(const uint32_t instance);

/*! @} */ /* End of Runtime Status & Control */

/*******************************************************************************
 * Pulse Counter Pin Configuration
 ******************************************************************************/
/*!
 * @name Pulse Counter Pin Configuration
 * @brief Functions for updating pulse-counter input routing.
 * @{
 */

/*!
 * @brief Update the pulse input source and active edge.
 *
 * @param[in] instance     lpTMR instance index. Must be less than `lpTMR_INSTANCE_COUNT`.
 * @param[in] pinSelect    Pulse input source selection.
 * @param[in] pinPolarity  Active edge selection for pulse counting.
 *
 * @note This API is intended for `lpTMR_WORKMODE_PULSECOUNTER`.
 */
void lpTMR_DRV_SetPinConfiguration(const uint32_t instance,
                                   const lptmr_pinselect_t pinSelect,
                                   const lptmr_pinpolarity_t pinPolarity);

/*! @} */ /* End of Pulse Counter Pin Configuration */

#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* lpTMR_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
