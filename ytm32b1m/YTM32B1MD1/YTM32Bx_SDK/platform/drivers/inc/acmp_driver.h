/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file acmp_driver.h
 * @version 1.4.1
 *
 * @brief ACMP Driver — public API for analog comparison and threshold detection.
 *
 * This header defines the application-level interface for the Analog Comparator
 * peripheral. The driver aggregates comparator, DAC, input MUX, and continuous
 * scan settings into `acmp_config_t`, then applies those settings through
 * instance-based `ACMP_DRV_*` APIs.
 *
 * The APIs are organized into the following categories:
 *   - Initialization & Global Control
 *   - Configuration Snapshot
 *   - Comparator, DAC, MUX & Continuous Configuration
 *   - Continuous Runtime Control
 *   - Status & Output Query
 *
 * @note Configure the ACMP clock and input or output pins before enabling the
 *       module.
 */

#ifndef ACMP_DRIVER_H
#define ACMP_DRIVER_H

#include "device_registers.h"
#include "status.h"

/*!
 * @addtogroup acmp
 * @brief Analog Comparator peripheral driver — public API.
 * @details Provides instance-based APIs for configuring the comparator core,
 *          DAC threshold generator, input routing, and continuous scan mode.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ACMP_CONTINUOUS_MODE_CHANNELS_MASK  0xFF00
#define ACMP_CONTINUOUS_MODE_CHANNELS_SHIFT 8U
#define ACMP_STS_CH_OUT_MASK                0xFF000000U
#define ACMP_STS_CH_OUT_SHIFT               24U
#define ACMP_STS_CH_FLAG_MASK               0x00FF0000U
#define ACMP_STS_CH_FLAG_SHIFT              16U

/*******************************************************************************
 * Enumerations and Data Types
 ******************************************************************************/

/*!
 * @brief Comparator performance mode.
 *
 * Selects the comparator operating point for the desired power or speed trade-off.
 */
typedef enum
{
    ACMP_LOW_POWER = 0U,  /*!< Module in low power mode. */
    ACMP_HIGH_SPEED = 1U  /*!< Module in high speed mode. */
} acmp_power_mode_t;

#if FEATURE_ACMP_HAS_DAC_VOLTAGE_REF_SRC
/*!
 * @brief DAC reference source selection.
 */
typedef enum
{
    ACMP_EXT_REF = 0U, /*!< Use VREFH/VREFL as supply reference source for DAC. */
    ACMP_INT_REF = 1U  /*!< Use internal supply reference source(VDDA) for DAC. */
} acmp_voltage_reference_t;
#endif /* FEATURE_ACMP_HAS_DAC_VOLTAGE_REF_SRC */

/*!
 * @brief Comparator input source selection.
 */
typedef enum
{
    ACMP_INPUT_SRC_DAC = 0U, /*!< Select DAC as source for the comparator port. */
    ACMP_INPUT_SRC_MUX = 1U  /*!< Select MUX8 as source for the comparator port. */
} acmp_input_source_t;

/*!
 * @brief Comparator output polarity.
 */
typedef enum
{
    ACMP_OUT_POL_NOT_INVERT = 0U, /*!< ACMP output polarity isn't inverted. */
    ACMP_OUT_POL_INVERT = 1U      /*!< ACMP output polarity is inverted. */
} acmp_output_polarity_t;

/*!
 * @brief Comparator output path selection.
 */
typedef enum
{
    ACMP_OUTPUT_FILTERED = 0U, /*!< Route the filtered output to the pin. */
    ACMP_OUTPUT_DIRECTLY = 1U  /*!< Route the direct comparator output to the pin. */
} acmp_output_select_t;

/*!
 * @brief Comparator hysteresis level.
 */
typedef enum
{
    ACMP_HYS_LEVEL_0 = 0U, /*!< No internal hysteresis. */
    ACMP_HYS_LEVEL_1 = 1U, /*!< 20mV internal hysteresis. */
    ACMP_HYS_LEVEL_2 = 2U, /*!< 40mV internal hysteresis. */
    ACMP_HYS_LEVEL_3 = 3U  /*!< Reserved. */
} acmp_hysteresis_t;

/*!
 * @brief Fixed comparator port used in continuous mode.
 */
typedef enum
{
    ACMP_FIXED_NEG_PORT = 0U, /*!< Keep the negative input fixed during the scan. */
    ACMP_FIXED_POS_PORT = 1U  /*!< Keep the positive input fixed during the scan. */
} acmp_fixed_port_t;

/*!
 * @brief Comparator event selection for interrupt or DMA signaling.
 */
typedef enum
{
    ACMP_NO_EDGE = 0U,      /*!< Do not generate comparator interrupt. */
    ACMP_RISING_EDGE = 1U,  /*!< Comparator interrupt generated on rising edge. */
    ACMP_FALLING_EDGE = 2U, /*!< Comparator interrupt generated on falling edge. */
    ACMP_BOTH_EDGES = 3U,   /*!< Comparator interrupt generated on both edges. */
#if defined(FEATURE_ACMP_SUPPORT_LEVEL_DETECTION)
    ACMP_HIGH_LEVEL = 4U,   /*!< Comparator interrupt generated on high level. */
    ACMP_LOW_LEVEL = 5U     /*!< Comparator interrupt generated on low level. */
#endif
} acmp_edge_select_t;

/*!
 * @brief Digital filter clock source.
 */
typedef enum
{
    ACMP_FILTER_SEL_FUNC_CLK = 0U, /*!< Use the ACMP functional clock selected by IPC. */
    ACMP_FILTER_SEL_BUS_CLK = 1U   /*!< Use the bus clock. */
} acmp_filter_clk_src_t;

/*!
 * @brief Comparator sample mode.
 */
typedef enum
{
    ACMP_COMMON_MODE = 0U,   /*!< Normal comparator operation. */
    ACMP_TRIGGER_MODE = 1U,  /*!< Triggered or sampled operation. */
    ACMP_WINDOW_MODE = 2U,   /*!< Window comparison mode. */
#if FEATURE_ACMP_HAS_CONTINUOUS_MODE_IN_CTRL_REG
    ACMP_CONTINUOUS_MODE = 3U, /*!< Continuous mode. */
#else
    ACMP_RESERVED = 3U         /*!< Reserved. */
#endif
} acmp_sample_mode_t;

/*!
 * @brief Comparator clock source.
 */
typedef enum
{
    ACMP_CLK_SRC_IPC = 0U, /*!< Use the functional clock. */
    ACMP_CLK_SRC_SIRC = 1U, /*!< Use the SIRC clock. */
#if FEATURE_ACMP_SUPPORT_SXOSC_CLK
    ACMP_CLK_SRC_SXOSC = 2U /*!< Use the SXOSC clock. */
#endif
} acmp_clock_source_t;

/*!
 * @brief Continuous scan mode behavior.
 */
typedef enum
{
    ACMP_CONTINUOUS_LOOP_MODE = 0U,     /*!< Repeat the channel scan continuously. */
    ACMP_CONTINUOUS_ONE_SHOT_MODE = 1U  /*!< Run one scan sequence and then stop. */
} acmp_continuous_mode_t;

/*!
 * @brief Expected result for a channel monitored in continuous mode.
 */
typedef enum
{
    ACMP_EXPECT_POS_LESS_THAN_NEG = 0U,  /*!< Expect positive input to remain below negative input. */
    ACMP_EXPECT_POS_GREAT_THAN_NEG = 1U  /*!< Expect positive input to remain above negative input. */
} acmp_ch_expectation_config_t;

/*!
 * @brief Captured comparison result for a scanned channel.
 */
typedef enum
{
    ACMP_POS_LESS_THAN_NEG = 0U,  /*!< Positive input was below the negative input. */
    ACMP_POS_GREAT_THAN_NEG = 1U  /*!< Positive input was above the negative input. */
} acmp_ch_output_result_t;

/*!
 * @brief Per-channel configuration used by continuous scan mode.
 */
typedef struct
{
    bool enable;                                  /*!< Enables monitoring of the channel during the scan. */
    acmp_ch_expectation_config_t expectation;     /*!< Expected compare result for the channel. */
} acmp_ch_config_t;

/*!
 * @brief Packed 8-bit channel bitmap used by ACMP status and expectation APIs.
 *
 * Bit 0 corresponds to channel 0, and bit 7 corresponds to channel 7.
 */
typedef uint8_t acmp_ch_list_t;

/*!
 * @brief ACMP input channel index type.
 */
typedef uint8_t acmp_ch_number_t;

/*!
 * @brief Comparator core configuration.
 *
 * Controls the comparator operating mode, event generation, hysteresis, output
 * behavior, filter settings, and optional device-dependent controls.
 */
typedef struct
{
    acmp_sample_mode_t sampleMode;         /*!< Comparator sample mode. */
    acmp_edge_select_t edgeSelection;      /*!< Output event condition for interrupt or DMA signaling. */
    acmp_hysteresis_t hysteresisLevel;     /*!< Internal hysteresis level. */
#if FEATURE_ACMP_HAS_OFFSET_CONTROL
    bool hardBlockOffset;                  /*!< Enables hard-block offset behavior when supported. */
#endif
    acmp_output_select_t outputSelect;     /*!< Selects filtered or direct output routing. */
    acmp_output_polarity_t outputPolarity; /*!< Selects normal or inverted comparator output. */
    acmp_power_mode_t powerMode;           /*!< Selects the power or speed operating point. */
    bool filterEnable;                     /*!< Enables the digital output filter. */
    acmp_filter_clk_src_t filterClkSrc;    /*!< Selects the filter clock source. */
    uint8_t filterSamplePeriod;            /*!< Filter sampling period in clock cycles. */
    uint8_t filterSampleCount;             /*!< Number of agreeing samples required by the filter. */
#if FEATURE_ACMP_HAS_AUTODIS
    bool autoDisableHardBlock;             /*!< Auto-disables the hard block in one-shot mode when supported. */
#endif
#if FEATURE_ACMP_HAS_CLK_SRC_SEL
    acmp_clock_source_t acmpClkSrc;        /*!< Selects the ACMP module clock source. */
#endif
    bool interruptEnable;                  /*!< Enables comparator interrupt generation. */
    bool dmaTriggerEnable;                 /*!< Enables comparator DMA triggering. */
} acmp_comparator_config_t;

/*!
 * @brief DAC threshold generator configuration.
 */
typedef struct
{
#if FEATURE_ACMP_HAS_DAC_VOLTAGE_REF_SRC
    acmp_voltage_reference_t voltageReferenceSource; /*!< DAC reference source. */
#endif
    bool enable;                                     /*!< Enables the integrated DAC. */
#if FEATURE_ACMP_HAS_DAC_OUTPUT
    bool outputEnable;                               /*!< Enables the DAC analog output when supported. */
#endif
    uint8_t voltage;                                 /*!< DAC threshold code written to the hardware. */
} acmp_dac_config_t;

/*!
 * @brief Comparator input routing configuration.
 *
 * Selects the source and external channel index for each comparator input.
 */
typedef struct
{
    acmp_input_source_t positiveInputSrc; /*!< Source routed to the positive input. */
    acmp_input_source_t negativeInputSrc; /*!< Source routed to the negative input. */
    acmp_ch_number_t positiveInputChnSel; /*!< External channel routed to the positive input when using the MUX. */
    acmp_ch_number_t negativeInputChnSel; /*!< External channel routed to the negative input when using the MUX. */
} acmp_mux_config_t;

/*!
 * @brief Continuous scan configuration.
 *
 * Controls round-robin scan behavior, fixed-port selection, channel timing,
 * and per-channel enable or expectation state.
 */
typedef struct
{
    bool continuousEnable;                 /*!< Enables continuous scan mode. */
    acmp_continuous_mode_t continuousMode; /*!< Selects loop or one-shot scan behavior. */
#if FEATURE_ACMP_HAS_TRIG_MODE_GATE
    bool oneshotTriggerEnable;             /*!< Enables trigger gating for one-shot mode. */
#endif
    acmp_fixed_port_t fixedPort;           /*!< Selects the comparator port held constant during the scan. */
    uint8_t samplePeriod;                  /*!< Delay between successive channel samples. */
    uint8_t samplePosition;                /*!< Delay from channel switch to actual sampling instant. */
    bool continuousInterruptEnable;        /*!< Enables continuous-mode interrupt signaling. */
    acmp_ch_config_t channelConfig[8];     /*!< Per-channel enable and expected compare state. */
} acmp_continuous_config_t;

/*!
 * @brief Aggregate ACMP configuration container.
 *
 * The driver uses this structure to bundle the four ACMP sub-configuration
 * blocks. For `ACMP_DRV_GetDefaultConfig()` and `ACMP_DRV_GetConfigAll()`,
 * every member pointer must reference valid writable storage.
 */
typedef struct
{
    acmp_comparator_config_t *comparatorConfig;   /*!< Comparator core configuration block. */
    acmp_dac_config_t *dacConfig;                 /*!< DAC configuration block. */
    acmp_mux_config_t *muxConfig;                 /*!< Input routing configuration block. */
    acmp_continuous_config_t *continuousConfig;   /*!< Continuous scan configuration block. */
} acmp_config_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Initialization & Global Control
 ******************************************************************************/
/*!
 * @name Initialization & Global Control
 * @brief Functions for resetting, initializing, and enabling the ACMP instance.
 * @{
 */

/*!
 * @brief Reset an ACMP instance through the IPC software reset path.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Execution status.
 * @retval STATUS_SUCCESS Reset completed successfully.
 */
status_t ACMP_DRV_Reset(const uint32_t instance);

/*!
 * @brief Populate all ACMP sub-configuration blocks with default values.
 *
 * The driver writes defaults into the comparator, DAC, MUX, and continuous
 * configuration structures referenced by @a config.
 *
 * @param[in] config  Pointer to the configuration container. The pointer itself
 *                    must be valid, and all member pointers must reference
 *                    writable storage.
 * @return Execution status.
 * @retval STATUS_SUCCESS Default values were written successfully.
 *
 * @note Because the function signature uses `const acmp_config_t *`, the
 *       pointed-to sub-structures are updated through the member pointers.
 */
status_t ACMP_DRV_GetDefaultConfig(const acmp_config_t *config);

/*!
 * @brief Initialize an ACMP instance from the supplied configuration blocks.
 *
 * Applies any non-NULL configuration block referenced by @a config, clears
 * pending status flags, and configures continuous mode when requested.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] config    Pointer to the configuration container.
 * @return Execution status.
 * @retval STATUS_SUCCESS Initialization completed successfully.
 *
 * @pre The ACMP peripheral clock must already be enabled.
 * @note Individual sub-block pointers inside @a config may be NULL if that
 *       portion of the peripheral should be left untouched.
 */
status_t ACMP_DRV_Init(const uint32_t instance, const acmp_config_t *config);

/*!
 * @brief Enable the comparator block for the selected instance.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Execution status.
 * @retval STATUS_SUCCESS Enable completed successfully.
 */
status_t ACMP_DRV_Enable(const uint32_t instance);

/*!
 * @brief Disable the comparator block for the selected instance.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Execution status.
 * @retval STATUS_SUCCESS Disable completed successfully.
 */
status_t ACMP_DRV_Disable(const uint32_t instance);

/*! @} */ /* End of Initialization & Global Control */

/*******************************************************************************
 * Configuration Snapshot
 ******************************************************************************/
/*!
 * @name Configuration Snapshot
 * @brief Functions for reading back the full ACMP configuration state.
 * @{
 */

/*!
 * @brief Read back all ACMP configuration blocks for one instance.
 *
 * The driver writes the current comparator, DAC, MUX, and continuous-mode
 * state into the sub-structures referenced by @a config.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] config    Pointer to the configuration container. All member
 *                      pointers must reference writable storage.
 * @return Execution status.
 * @retval STATUS_SUCCESS Configuration was captured successfully.
 */
status_t ACMP_DRV_GetConfigAll(const uint32_t instance, const acmp_config_t *config);

/*! @} */ /* End of Configuration Snapshot */

/*******************************************************************************
 * Comparator, DAC, MUX & Continuous Configuration
 ******************************************************************************/
/*!
 * @name Comparator, DAC, MUX & Continuous Configuration
 * @brief Functions for configuring individual ACMP sub-blocks.
 * @{
 */

/*!
 * @brief Configure the comparator core.
 *
 * Applies sample mode, event selection, hysteresis, output behavior, filter
 * settings, interrupt enable, and DMA trigger control.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] config    Pointer to the comparator configuration structure.
 * @return Execution status.
 * @retval STATUS_SUCCESS Comparator configuration was applied successfully.
 */
status_t ACMP_DRV_ConfigComparator(const uint32_t instance, const acmp_comparator_config_t *config);

/*!
 * @brief Read back the comparator core configuration.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[out] config   Pointer to the structure that receives the comparator state.
 * @return Execution status.
 * @retval STATUS_SUCCESS Comparator configuration was read successfully.
 */
status_t ACMP_DRV_GetComparatorConfig(const uint32_t instance, acmp_comparator_config_t *config);

/*!
 * @brief Configure the integrated DAC threshold generator.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] config    Pointer to the DAC configuration structure.
 * @return Execution status.
 * @retval STATUS_SUCCESS DAC configuration was applied successfully.
 */
status_t ACMP_DRV_ConfigDac(const uint32_t instance, const acmp_dac_config_t *config);

/*!
 * @brief Read back the integrated DAC configuration.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[out] config   Pointer to the structure that receives the DAC state.
 * @return Execution status.
 * @retval STATUS_SUCCESS DAC configuration was read successfully.
 */
status_t ACMP_DRV_GetDacConfig(const uint32_t instance, acmp_dac_config_t *config);

/*!
 * @brief Configure the positive and negative input routes.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] config    Pointer to the MUX configuration structure.
 * @return Execution status.
 * @retval STATUS_SUCCESS Input routing was applied successfully.
 */
status_t ACMP_DRV_ConfigMux(const uint32_t instance, const acmp_mux_config_t *config);

/*!
 * @brief Read back the positive and negative input routes.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[out] config   Pointer to the structure that receives the routing state.
 * @return Execution status.
 * @retval STATUS_SUCCESS Input routing was read successfully.
 */
status_t ACMP_DRV_GetMuxConfig(const uint32_t instance, acmp_mux_config_t *config);

/*!
 * @brief Configure continuous scan mode.
 *
 * Programs the continuous-mode scan behavior, timing, fixed-port selection,
 * interrupt enable state, and per-channel expectations.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] config    Pointer to the continuous-mode configuration structure.
 * @return Execution status.
 * @retval STATUS_SUCCESS Continuous-mode configuration was applied successfully.
 */
status_t ACMP_DRV_ConfigContinuous(const uint32_t instance, const acmp_continuous_config_t *config);

/*!
 * @brief Read back continuous scan configuration.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[out] config   Pointer to the structure that receives the continuous-mode state.
 * @return Execution status.
 * @retval STATUS_SUCCESS Continuous-mode configuration was read successfully.
 */
status_t ACMP_DRV_GetContinuousConfig(const uint32_t instance, acmp_continuous_config_t *config);

/*! @} */ /* End of Comparator, DAC, MUX & Continuous Configuration */

/*******************************************************************************
 * Continuous Runtime Control
 ******************************************************************************/
/*!
 * @name Continuous Runtime Control
 * @brief Functions for adjusting or enabling continuous mode after initialization.
 * @{
 */

/*!
 * @brief Write the packed expectation bitmap used by continuous mode.
 *
 * Each bit in @a state corresponds to one channel expectation value.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] state     Packed expectation bitmap. Bit 0 controls channel 0 and
 *                      bit 7 controls channel 7.
 * @return Execution status.
 * @retval STATUS_SUCCESS Expectation state was updated successfully.
 */
status_t ACMP_DRV_SetExpectation(const uint32_t instance, uint8_t state);

/*!
 * @brief Request continuous scan mode through the runtime control path.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Execution status.
 * @retval STATUS_SUCCESS Continuous mode was enabled successfully.
 */
status_t ACMP_DRV_EnableContinuous(const uint32_t instance);

/*!
 * @brief Request continuous-mode disable through the runtime control path.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Execution status.
 * @retval STATUS_SUCCESS Continuous mode was disabled successfully.
 *
 * @note On devices that select continuous mode through the sample-mode field,
 *       the final behavior depends on the underlying hardware implementation.
 */
status_t ACMP_DRV_DisableContinuous(const uint32_t instance);

/*! @} */ /* End of Continuous Runtime Control */

/*******************************************************************************
 * Status & Output Query
 ******************************************************************************/
/*!
 * @name Status & Output Query
 * @brief Functions for reading ACMP status flags and comparison outputs.
 * @{
 */

/*!
 * @brief Read the comparator output event flags.
 *
 * Depending on device support, the returned bitmap may include edge flags only
 * or both edge and level-detection flags.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[out] flags    Pointer to the variable that receives the output flags.
 * @return Execution status.
 * @retval STATUS_SUCCESS Output flags were read successfully.
 */
status_t ACMP_DRV_GetOutputFlags(const uint32_t instance, uint8_t *flags);

/*!
 * @brief Clear the comparator output event flags.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Execution status.
 * @retval STATUS_SUCCESS Output flags were cleared successfully.
 */
status_t ACMP_DRV_ClearOutputFlags(const uint32_t instance);

/*!
 * @brief Read the per-channel change flags used by continuous mode.
 *
 * Bit 0 corresponds to channel 0 and bit 7 corresponds to channel 7.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[out] flags    Pointer to the variable that receives the channel flags.
 * @return Execution status.
 * @retval STATUS_SUCCESS Channel flags were read successfully.
 */
status_t ACMP_DRV_GetChannelFlags(const uint32_t instance, acmp_ch_list_t *flags);

/*!
 * @brief Clear all continuous-mode channel change flags.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Execution status.
 * @retval STATUS_SUCCESS Channel flags were cleared successfully.
 */
status_t ACMP_DRV_ClearChannelFlags(const uint32_t instance);

/*!
 * @brief Report the channel currently being sampled in continuous mode.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return Current channel identifier.
 */
uint8_t ACMP_DRV_GetChannelId(const uint32_t instance);

/*!
 * @brief Read the current comparator output state.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @return `true` when the comparator output is high, or `false` otherwise.
 */
bool ACMP_DRV_GetOutput(const uint32_t instance);

/*!
 * @brief Read the last captured output for a channel scanned in continuous mode.
 *
 * @param[in] instance  ACMP instance index (0-based).
 * @param[in] channel   Channel index to query.
 * @return `true` when the stored channel output is high, or `false` otherwise.
 */
bool ACMP_DRV_GetChannelOutput(const uint32_t instance, uint8_t channel);

/*! @} */ /* End of Status & Output Query */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* ACMP_DRIVER_H */

/*******************************************************************************
 * EOF
 ******************************************************************************/
