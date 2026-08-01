/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file adc_driver.h
 * @version 1.4.1
 *
 * @brief ADC Driver - Public API for analog-to-digital conversion control.
 *
 * This header defines the application-facing interface for the ADC peripheral.
 * It provides configuration helpers for converter timing, trigger routing,
 * scan sequences, analog watchdog monitoring, FIFO access, and runtime status
 * polling.
 *
 * The APIs are organized into five categories:
 *   - Converter Configuration
 *   - Hardware Compare Configuration
 *   - Sequence Configuration
 *   - Conversion Control & Status
 *   - Interrupt & Flag Control
 *
 * @note The ADC driver does not configure clocks or I/O pins. Enable the ADC
 *       peripheral clock and configure analog pins before calling these APIs.
 */

#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "device_registers.h"
#include "status.h"

/*!
 * @addtogroup adc
 * @brief Analog-to-digital converter driver - public API.
 * @details Provides instance-based APIs for ADC converter setup, sequence
 *          programming, analog watchdog configuration, FIFO access, and
 *          polled runtime control.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief ADC conversion resolution.
 *
 * Selects the number of valid bits produced by the SAR converter for each
 * completed sample.
 *
 * | Value | Resolution |
 * |------|------------|
 * | `ADC_RESOLUTION_12BIT` | 12-bit conversion result. |
 * | `ADC_RESOLUTION_10BIT` | 10-bit conversion result. |
 * | `ADC_RESOLUTION_8BIT`  | 8-bit conversion result. |
 * | `ADC_RESOLUTION_6BIT`  | 6-bit conversion result. |
 *
 * Implements : adc_resolution_t_Class
 */
typedef enum
{
    ADC_RESOLUTION_12BIT = 0x00U,   /*!< 12-bit conversion result. */
    ADC_RESOLUTION_10BIT = 0x01U,   /*!< 10-bit conversion result. */
    ADC_RESOLUTION_8BIT = 0x02U,    /*!< 8-bit conversion result. */
    ADC_RESOLUTION_6BIT = 0x03U     /*!< 6-bit conversion result. */
} adc_resolution_t;

/*!
 * @brief ADC result alignment.
 *
 * Controls whether conversion results are left-aligned or right-aligned when
 * read back from the ADC result path.
 *
 * Implements : adc_align_t_Class
 */
typedef enum
{
#if defined (CPU_YTM32Z1MD0)
    ADC_ALIGN_RIGHT = 0x00U,   /*!< Right-align the conversion result. */
    ADC_ALIGN_LEFT = 0x01U,    /*!< Left-align the conversion result. */
#else
    ADC_ALIGN_LEFT = 0x00U,    /*!< Left-align the conversion result. */
    ADC_ALIGN_RIGHT = 0x01U    /*!< Right-align the conversion result. */
#endif /* defined (CPU_YTM32Z1MD0) */
} adc_align_t;

/*!
 * @brief ADC trigger mode selection.
 *
 * On devices without injected conversion support, this type selects standard
 * software or hardware triggering for the normal conversion path. On devices
 * with injected conversion support, the encoded value selects the trigger
 * source for both the normal and injected paths.
 *
 * Implements : adc_trigger_t_Class
 */
typedef enum
{
#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
    ADC_TRIG_INJECT_SW_NORMAL_SW = 0x00U,        /*!< Software trigger normal conversions; injected path disabled. */
    ADC_TRIG_INJECT_SW_NORMAL_HW = 0x01U,        /*!< Hardware trigger normal conversions; injected path disabled. */
    ADC_TRIG_INJECT_HW_NORMAL_SW = 0x02U,        /*!< Software trigger normal conversions; hardware-trigger injected conversions. */
    ADC_TRIG_INJECT_HW_NORMAL_HW = 0x03U,        /*!< Hardware trigger both normal and injected conversions. */
#else
    ADC_TRIGGER_SOFTWARE = 0x00U,               /*!< Start conversions through software control. */
    ADC_TRIGGER_HARDWARE = 0x01U,               /*!< Start conversions from the configured hardware trigger. */
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */
} adc_trigger_t;

/*!
 * @brief Normal-sequence execution mode.
 *
 * Determines how the ADC advances through the configured sequence after a
 * trigger event arrives.
 *
 * Implements : adc_sequence_mode_t_Class
 */
typedef enum
{
    ADC_CONV_STEP = 0x00U,          /*!< Convert one channel for each trigger event. */
    ADC_CONV_LOOP = 0x01U,          /*!< Convert the full programmed sequence for each trigger event. */
    ADC_CONV_CONTINUOUS = 0x02U     /*!< Continuously restart sequence conversion after launch. */
} adc_sequence_mode_t;

#if defined (CPU_YTM32Z1MD0)
/*!
 * @brief Independent-sequence operating mode.
 *
 * Implements : adc_sequence_independent_mode_t_Class
 */
typedef enum
{
    ADC_SEQUENCE_MODE = 0x00U,      /*!< Run the selected sequence as a grouped scan. */
    ADC_SINGLE_MODE = 0x01U         /*!< Treat the selected sequence as a single-conversion path. */
} adc_sequence_independent_mode_t;

/*!
 * @brief Sequence-combine layout for devices with split sequence blocks.
 *
 * Implements : adc_sequence_combine_mode_t_Class
 */
typedef enum
{
    ADC_MODE_SEQ0123 = 0x1U,             /*!< Combine sequences 0-3 into one 16-channel sequence. */
    ADC_MODE_SEQ0_SEQ123 = 0x3U,         /*!< Split into one 4-channel sequence and one 12-channel sequence. */
    ADC_MODE_SEQ01_SEQ23 = 0x5U,         /*!< Split into two 8-channel sequences. */
    ADC_MODE_SEQ0_SEQ1_SEQ23 = 0x7U,     /*!< Split into two 4-channel sequences and one 8-channel sequence. */
    ADC_MODE_SEQ0_SEQ1_SEQ2_SEQ3 = 0xFU, /*!< Keep four independent 4-channel sequences. */
} adc_sequence_combine_mode_t;
#endif /* defined (CPU_YTM32Z1MD0) */

/*!
 * @brief Selectable ADC input channels.
 *
 * Identifies the external analog inputs that can be assigned to a sequence
 * slot or watchdog channel. The number of available enumerators depends on
 * `FEATURE_ADC_MAX_NUM_EXT_CHANS`.
 *
 * Implements : adc_inputchannel_t_Class
 */
typedef enum
{
    ADC_INPUTCHAN_EXT0 = 0x00U,               /*!< External analog input channel 0. */
    ADC_INPUTCHAN_EXT1 = 0x01U,               /*!< External analog input channel 1. */
    ADC_INPUTCHAN_EXT2 = 0x02U,               /*!< External analog input channel 2. */
    ADC_INPUTCHAN_EXT3 = 0x03U,               /*!< External analog input channel 3. */
    ADC_INPUTCHAN_EXT4 = 0x04U,               /*!< External analog input channel 4. */
    ADC_INPUTCHAN_EXT5 = 0x05U,               /*!< External analog input channel 5. */
    ADC_INPUTCHAN_EXT6 = 0x06U,               /*!< External analog input channel 6. */
    ADC_INPUTCHAN_EXT7 = 0x07U,               /*!< External analog input channel 7. */
    ADC_INPUTCHAN_EXT8 = 0x08U,               /*!< External analog input channel 8. */
    ADC_INPUTCHAN_EXT9 = 0x09U,               /*!< External analog input channel 9. */
    ADC_INPUTCHAN_EXT10 = 0x0AU,              /*!< External analog input channel 10. */
    ADC_INPUTCHAN_EXT11 = 0x0BU,              /*!< External analog input channel 11. */
    ADC_INPUTCHAN_EXT12 = 0x0CU,              /*!< External analog input channel 12. */
    ADC_INPUTCHAN_EXT13 = 0x0DU,              /*!< External analog input channel 13. */
    ADC_INPUTCHAN_EXT14 = 0x0EU,              /*!< External analog input channel 14. */
    ADC_INPUTCHAN_EXT15 = 0x0FU,              /*!< External analog input channel 15. */
 #if (FEATURE_ADC_MAX_NUM_EXT_CHANS > 16)
    ADC_INPUTCHAN_EXT16 = 0x10U,              /*!< External analog input channel 16. */
    ADC_INPUTCHAN_EXT17 = 0x11U,              /*!< External analog input channel 17. */
    ADC_INPUTCHAN_EXT18 = 0x12U,              /*!< External analog input channel 18. */
    ADC_INPUTCHAN_EXT19 = 0x13U,              /*!< External analog input channel 19. */
    ADC_INPUTCHAN_EXT20 = 0x14U,              /*!< External analog input channel 20. */
    ADC_INPUTCHAN_EXT21 = 0x15U,              /*!< External analog input channel 21. */
    ADC_INPUTCHAN_EXT22 = 0x16U,              /*!< External analog input channel 22. */
    ADC_INPUTCHAN_EXT23 = 0x17U,              /*!< External analog input channel 23. */
 #endif
 #if (FEATURE_ADC_MAX_NUM_EXT_CHANS > 24)
    ADC_INPUTCHAN_EXT24 = 0x18U,              /*!< External analog input channel 24. */
    ADC_INPUTCHAN_EXT25 = 0x19U,              /*!< External analog input channel 25. */
    ADC_INPUTCHAN_EXT26 = 0x1AU,              /*!< External analog input channel 26. */
    ADC_INPUTCHAN_EXT27 = 0x1BU,              /*!< External analog input channel 27. */
    ADC_INPUTCHAN_EXT28 = 0x1CU,              /*!< External analog input channel 28. */
    ADC_INPUTCHAN_EXT29 = 0x1DU,              /*!< External analog input channel 29. */
    ADC_INPUTCHAN_EXT30 = 0x1EU,              /*!< External analog input channel 30. */
    ADC_INPUTCHAN_EXT31 = 0x1FU,              /*!< External analog input channel 31. */
 #endif
 #if (FEATURE_ADC_MAX_NUM_EXT_CHANS > 32)
    ADC_INPUTCHAN_EXT32 = 0x20U,              /*!< External analog input channel 32. */
    ADC_INPUTCHAN_EXT33 = 0x21U,              /*!< External analog input channel 33. */
    ADC_INPUTCHAN_EXT34 = 0x22U,              /*!< External analog input channel 34. */
    ADC_INPUTCHAN_EXT35 = 0x23U,              /*!< External analog input channel 35. */
    ADC_INPUTCHAN_EXT36 = 0x24U,              /*!< External analog input channel 36. */
    ADC_INPUTCHAN_EXT37 = 0x25U,              /*!< External analog input channel 37. */
    ADC_INPUTCHAN_EXT38 = 0x26U,              /*!< External analog input channel 38. */
    ADC_INPUTCHAN_EXT39 = 0x27U,              /*!< External analog input channel 39. */
 #endif
 #if (FEATURE_ADC_MAX_NUM_EXT_CHANS > 40)
    ADC_INPUTCHAN_EXT40 = 0x28U,              /*!< External analog input channel 40. */
    ADC_INPUTCHAN_EXT41 = 0x29U,              /*!< External analog input channel 41. */
    ADC_INPUTCHAN_EXT42 = 0x2AU,              /*!< External analog input channel 42. */
    ADC_INPUTCHAN_EXT43 = 0x2BU,              /*!< External analog input channel 43. */
    ADC_INPUTCHAN_EXT44 = 0x2CU,              /*!< External analog input channel 44. */
    ADC_INPUTCHAN_EXT45 = 0x2DU,              /*!< External analog input channel 45. */
    ADC_INPUTCHAN_EXT46 = 0x2EU,              /*!< External analog input channel 46. */
    ADC_INPUTCHAN_EXT47 = 0x2FU,              /*!< External analog input channel 47. */
 #endif
 #if (FEATURE_ADC_MAX_NUM_EXT_CHANS > 48)
    ADC_INPUTCHAN_EXT48 = 0x30U,              /*!< External analog input channel 48. */
    ADC_INPUTCHAN_EXT49 = 0x31U,              /*!< External analog input channel 49. */
    ADC_INPUTCHAN_EXT50 = 0x32U,              /*!< External analog input channel 50. */
    ADC_INPUTCHAN_EXT51 = 0x33U,              /*!< External analog input channel 51. */
    ADC_INPUTCHAN_EXT52 = 0x34U,              /*!< External analog input channel 52. */
    ADC_INPUTCHAN_EXT53 = 0x35U,              /*!< External analog input channel 53. */
    ADC_INPUTCHAN_EXT54 = 0x36U,              /*!< External analog input channel 54. */
    ADC_INPUTCHAN_EXT55 = 0x37U,              /*!< External analog input channel 55. */
 #endif
} adc_inputchannel_t;

#if defined (CPU_YTM32Z1MD0)
/*!
 * @brief Channel entry for an independently programmed sequence slot.
 *
 * Implements : adc_independent_channel_config_t_Class
 */
typedef struct
{
    uint8_t sampleTimeSelect;          /*!< Select which sample-time register is used for this slot. */
    adc_inputchannel_t channel;        /*!< Input channel assigned to the slot. */
} adc_independent_channel_config_t;

/*!
 * @brief Independent sequence configuration for split-sequence devices.
 *
 * Implements : adc_sequence_independent_config_t_Class
 */
typedef struct
{
    uint8_t sequenceLen;                                                 /*!< Number of active entries in this sequence. */
    adc_independent_channel_config_t channels[ADC_MAX_SUPPORT_SEQUENCE_CHANS]; /*!< Per-slot channel and sample-time selection. */
    adc_sequence_independent_mode_t sequenceMode;                        /*!< Single or grouped sequence behavior. */
    adc_trigger_t trigger;                                               /*!< Trigger mode applied to this independent sequence. */
    bool sequenceIntEnable;                                              /*!< Enable end-of-sequence interrupt reporting. */
} adc_sequence_independent_config_t;
#endif /* defined (CPU_YTM32Z1MD0) */

/*!
 * @brief Normal-sequence conversion configuration.
 *
 * Describes the ordered channel list and interrupt behavior used by the ADC
 * normal conversion path.
 *
 * Implements : adc_sequence_config_t_Class
 */
typedef struct
{
#if defined (CPU_YTM32Z1MD0)
    bool adcConvertContinue;                                                /*!< Keep sequence conversion running continuously. */
    adc_sequence_combine_mode_t seqCombineMode;                             /*!< Sequence block combine layout. */
    adc_sequence_independent_config_t sequenceIndependentConfig[ADC_MAX_SUPPORT_SEQUENCE_NUM]; /*!< Per-sequence configuration array. */
    bool sequenceErrIntEnable;                                              /*!< Enable sequence error interrupt reporting. */
    bool singleErrIntEnable;                                                /*!< Enable single-conversion error interrupt reporting. */
#else
    adc_inputchannel_t channels[ADC_CHSEL_COUNT];   /*!< Ordered channel list for the normal sequence. */
    uint8_t totalChannels;                          /*!< Number of active channels in the sequence. */
    adc_sequence_mode_t sequenceMode;               /*!< Sequence execution mode. */
    bool sequenceIntEnable;                         /*!< Enable end-of-sequence interrupts. */
#endif /* defined (CPU_YTM32Z1MD0) */
    bool convIntEnable;                             /*!< Enable end-of-conversion interrupts. */
    bool ovrunIntEnable;                            /*!< Enable FIFO overrun interrupts. */
    bool sampIntEnable;                             /*!< Enable sample-end interrupts. */
    bool readyIntEnable;                            /*!< Enable ADC-ready interrupts. */
} adc_sequence_config_t;

#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
/*!
 * @brief Injected-sequence conversion configuration.
 *
 * Holds the input-channel list and interrupt settings for the injected
 * conversion path on devices that support it.
 *
 * Implements : adc_inject_config_t_Class
 */
typedef struct
{
    bool injectEnable;                              /*!< Enable the injected conversion sequence. */
    adc_inputchannel_t channels[ADC_INJCH_COUNT];   /*!< Ordered channel list for the injected sequence. */
    uint8_t totalChannels;                          /*!< Number of active injected channels. */
    bool convIntEnable;                             /*!< Enable injected conversion-complete interrupts. */
    bool errorIntEnable;                            /*!< Enable injected-sequence error interrupts. */
} adc_inject_config_t;
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */

/*!
 * @brief Raw ADC clock-divider encoding type.
 *
 * This type stores the divider selector value written to the hardware clock
 * divider field.
 */
typedef uint8_t adc_clk_divide_t;

/*!
 * @brief Analog watchdog effective mode.
 *
 * Selects whether the watchdog event becomes active when a sample falls inside
 * or outside the configured threshold window.
 *
 * Implements : adc_awg_effective_mode_t_Class
 */
typedef enum
{
    ADC_AWG_EFFECTIVE_OUTSIDE = 0x00U,   /*!< Report matches outside the configured window. */
    ADC_AWG_EFFECTIVE_INSIDE = 0x01U     /*!< Report matches inside the configured window. */
} adc_awg_effective_mode_t;

/*!
 * @brief Analog watchdog configuration.
 *
 * Describes the threshold window and channel-selection settings used by the
 * ADC hardware compare / watchdog logic.
 *
 * Implements : adc_compare_config_t_Class
 */
typedef struct
{
#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
    uint8_t comWdchIndex;                   /*!< Watchdog channel index to configure. */
#endif /* FEATURE_ADC_WDG_CHANNEL_COUNT */
    bool compareEnable;                     /*!< Enable watchdog comparison. */
    bool compareAllChannelEnable;           /*!< Monitor all sequence channels when supported. */
    adc_inputchannel_t compChalSel;         /*!< Channel selected for single-channel watchdog mode. */
    uint16_t compHigh;                      /*!< Upper threshold value in 12-bit scale. */
    uint16_t compLow;                       /*!< Lower threshold value in 12-bit scale. */
    adc_awg_effective_mode_t effectiveMode; /*!< Choose inside-window or outside-window match behavior. */
} adc_compare_config_t;


/*!
 * @brief Top-level ADC converter configuration.
 *
 * Collects converter timing, trigger mode, DMA control, sequence programming,
 * and watchdog settings into a single structure used by
 * `ADC_DRV_ConfigConverter()`.
 *
 * Implements : adc_converter_config_t_Class
 */
typedef struct
{
    adc_clk_divide_t clockDivider;          /*!< Divider applied to the ADC input clock. */
    uint8_t startTime;                      /*!< ADC startup delay in ADC clock cycles. */
    uint8_t sampleTime;                     /*!< Primary sample time in ADC clock cycles. */
#if defined(FEATURE_ADC_SUPPORT_SECOND_SMP) && (FEATURE_ADC_SUPPORT_SECOND_SMP > 0)
    uint8_t sampleTime1;                    /*!< Secondary sample time in ADC clock cycles. */
#endif

    bool overrunMode;                       /*!< Enable overwrite-on-overrun behavior. */
    bool autoOffEnable;                     /*!< Enable automatic converter power-down. */
    bool waitEnable;                        /*!< Stall conversion while the FIFO is full. */
    uint8_t triggerSource;                  /*!< Hardware trigger source selector. */
    adc_trigger_t trigger;                  /*!< Normal/injected trigger mode selection. */
    adc_align_t align;                      /*!< Result alignment setting. */
    adc_resolution_t resolution;            /*!< Conversion result width. */
    uint8_t dmaWaterMark;                   /*!< Result FIFO watermark for DMA requests. */
    bool dmaEnable;                         /*!< Enable DMA request generation. */
    adc_sequence_config_t sequenceConfig;   /*!< Normal sequence configuration. */
#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
    adc_inject_config_t injectConfig;       /*!< Injected sequence configuration. */
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */
    bool compIntEnable;                     /*!< Enable watchdog interrupt generation. */
#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
    uint8_t adcCompareCnt;                  /*!< Number of watchdog configurations in the group. */
    adc_compare_config_t* compareConfig;    /*!< Pointer to watchdog configuration array. */
#else
    adc_compare_config_t compareConfig;     /*!< Single watchdog configuration. */
#endif /* FEATURE_ADC_WDG_CHANNEL_COUNT */
} adc_converter_config_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined (__cplusplus)
extern "C" {
#endif

/*!
 * @name Converter Configuration
 * @brief Functions for preparing and applying the top-level converter setup.
 */
/*! @{*/

/*!
 * @brief Fill a converter configuration structure with safe default values.
 *
 * @param[out] config Pointer to the configuration structure to initialize.
 *
 * @post `config` contains reset-style defaults that can be customized before
 *       calling `ADC_DRV_ConfigConverter()`.
 */
void ADC_DRV_InitConverterStruct(adc_converter_config_t *const config);

/*!
 * @brief Apply a top-level converter configuration to an ADC instance.
 *
 * @param[in] instance ADC instance index.
 * @param[in] config Pointer to the caller-provided configuration structure.
 *
 * @return STATUS_SUCCESS if ADC parameter correct.
 *
 * @note This API also applies embedded sequence, DMA, injected-conversion, and
 *       watchdog settings.
 */
status_t ADC_DRV_ConfigConverter(const uint32_t instance,
                                 const adc_converter_config_t *const config);

/*!
 * @brief Read the active converter configuration from hardware.
 *
 * @param[in] instance ADC instance index.
 * @param[out] config Pointer to the structure that receives the current state.
 */
void ADC_DRV_GetConverterConfig(const uint32_t instance,
                                adc_converter_config_t *const config);

/*! @}*/

/*!
 * @name Hardware Compare Configuration
 * @brief Functions for configuring the analog watchdog / hardware compare path.
 */
/*! @{*/

/*!
 * @brief Fill a watchdog configuration structure with default values.
 *
 * @param[out] config Pointer to the configuration structure to initialize.
 */
void ADC_DRV_InitHwCompareStruct(adc_compare_config_t *const config);

/*!
 * @brief Apply a single hardware-compare configuration.
 *
 * @param[in] instance ADC instance index.
 * @param[in] config Pointer to the watchdog configuration.
 */
void ADC_DRV_ConfigHwCompare(const uint32_t instance,
                             const adc_compare_config_t *const config);

#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
/*!
 * @brief Apply a watchdog configuration array.
 *
 * @param[in] instance ADC instance index.
 * @param[in] config Pointer to the first element of the watchdog configuration array.
 * @param[in] count Number of watchdog configurations to apply.
 */
void ADC_DRV_ConfigHwCompareGroup(const uint32_t instance,
                                  adc_compare_config_t *const config, 
                                  uint8_t count);
#endif /* FEATURE_ADC_WDG_CHANNEL_COUNT */

/*!
 * @brief Read back the active hardware-compare configuration.
 *
 * @param[in] instance ADC instance index.
 * @param[out] config Pointer to the structure that receives the watchdog state.
 */
void ADC_DRV_GetHwCompareConfig(const uint32_t instance,
                                adc_compare_config_t *const config);

/*! @}*/

/*!
 * @name Sequence Configuration
 * @brief Functions for configuring normal and injected conversion sequences.
 */
/*! @{*/

/*!
 * @brief Fill a normal-sequence configuration structure with default values.
 *
 * @param[out] config Pointer to the sequence configuration structure.
 */
void ADC_DRV_InitSequenceStruct(adc_sequence_config_t *const config);

/*!
 * @brief Program the normal conversion sequence.
 *
 * @param[in] instance ADC instance index.
 * @param[in] config Pointer to the normal-sequence configuration.
 *
 * @note Reconfiguring the sequence aborts any active conversion in progress.
 */
void ADC_DRV_ConfigSequence(const uint32_t instance,
                            const adc_sequence_config_t *const config);

/*!
 * @brief Read back the normal-sequence configuration.
 *
 * @param[in] instance ADC instance index.
 * @param[out] config Pointer to the structure that receives the sequence state.
 */
void ADC_DRV_GetSequenceConfig(const uint32_t instance,
                               adc_sequence_config_t *const config);

#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
/*!
 * @brief Program the injected conversion sequence.
 *
 * @param[in] instance ADC instance index.
 * @param[in] config Pointer to the injected-sequence configuration.
 */
void ADC_DRV_ConfigInject(const uint32_t instance,
                          const adc_inject_config_t *const config);

/*!
 * @brief Read back the injected conversion sequence configuration.
 *
 * @param[in] instance ADC instance index.
 * @param[out] config Pointer to the structure that receives the injected sequence state.
 */
void ADC_DRV_GetInjectConfig(const uint32_t instance,
                             adc_inject_config_t *const config);
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */
/*! @}*/

/*!
 * @name Conversion Control & Status
 * @brief Functions for enabling, running, polling, and reading ADC conversions.
 */
/*! @{*/

/*!
 * @brief Reset the ADC instance to its runtime default state.
 *
 * @param[in] instance ADC instance index.
 */
void ADC_DRV_Reset(const uint32_t instance);

/*!
 * @brief Enable the ADC hardware.
 *
 * @param[in] instance ADC instance index.
 */
void ADC_DRV_Enable(const uint32_t instance);

/*!
 * @brief Disable the ADC hardware.
 *
 * @param[in] instance ADC instance index.
 *
 * @note Disabling the ADC aborts any active conversion.
 */
void ADC_DRV_Disable(const uint32_t instance);

/*!
 * @brief Start or arm ADC conversion processing.
 *
 * @param[in] instance ADC instance index.
 *
 * @note In hardware-trigger mode, this call enables the converter and arms it
 *       for the next trigger event.
 */
void ADC_DRV_Start(const uint32_t instance);

/*!
 * @brief Stop the current ADC conversion activity.
 *
 * @param[in] instance ADC instance index.
 */
void ADC_DRV_Stop(const uint32_t instance);

/*!
 * @brief Poll until the current conversion is no longer active.
 *
 * @param[in] instance ADC instance index.
 */
void ADC_DRV_WaitConvDone(const uint32_t instance);

/*!
 * @brief Poll until the configured sequence reports completion.
 *
 * @param[in] instance ADC instance index.
 */
void ADC_DRV_WaitSequenceDone(const uint32_t instance);

/*!
 * @brief Get the conversion-complete flag state.
 *
 * @param[in] instance ADC instance index.
 * @return `true` if conversion completion has been reported; `false` otherwise.
 */
bool ADC_DRV_GetConvCompleteFlag(const uint32_t instance);

/*!
 * @brief Get the end-of-conversion flag state.
 *
 * @param[in] instance ADC instance index.
 * @return `true` if the end-of-conversion flag is set; `false` otherwise.
 */
bool ADC_DRV_GetEndOfConversionFlag(const uint32_t instance);

/*!
 * @brief Get the result-FIFO full flag state.
 *
 * @param[in] instance ADC instance index.
 * @return `true` if the result FIFO is full; `false` otherwise.
 */
bool ADC_DRV_GetFullOfConversionFlag(const uint32_t instance);

/*!
 * @brief Get the FIFO overrun flag state.
 *
 * @param[in] instance ADC instance index.
 * @return `true` if a FIFO overrun has occurred; `false` otherwise.
 */
bool ADC_DRV_GetOvrRunOfConversionFlag(const uint32_t instance);

#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
/*!
 * @brief Get the analog watchdog flag state.
 *
 * @param[in] instance ADC instance index.
 * @param[in] wdgChannel Watchdog channel index.
 * @return `true` if the watchdog event flag is set; `false` otherwise.
 */
bool ADC_DRV_GetWatchdogFlag(const uint32_t instance, const uint8_t wdgChannel);
#else /* FEATURE_ADC_WDG_CHANNEL_COUNT == 1 */
/*!
 * @brief Get the analog watchdog flag state.
 *
 * @param[in] instance ADC instance index.
 * @return `true` if the watchdog event flag is set; `false` otherwise.
 */
bool ADC_DRV_GetWatchdogFlag(const uint32_t instance);
#endif /* FEATURE_ADC_WDG_CHANNEL_COUNT */

/*!
 * @brief Get the end-of-sequence flag state.
 *
 * @param[in] instance ADC instance index.
 * @return `true` if the sequence-complete flag is set; `false` otherwise.
 */
bool ADC_DRV_GetEndOfSequenceFlag(const uint32_t instance);

/*!
 * @brief Get the ADC-ready flag state.
 *
 * @param[in] instance ADC instance index.
 * @return `true` if the ADC is ready; `false` otherwise.
 */
bool ADC_DRV_GetReadyFlag(const uint32_t instance);

/*!
 * @brief Get the sample-end flag state.
 *
 * @param[in] instance ADC instance index.
 * @return `true` if the sample-end flag is set; `false` otherwise.
 */
bool ADC_DRV_GetSampEndFlag(const uint32_t instance);

/*!
 * @brief Read one conversion result from the result FIFO.
 *
 * @param[in] instance ADC instance index.
 * @return Raw conversion data value from the FIFO.
 */
uint16_t ADC_DRV_ReadFIFO(const uint32_t instance);


/*!
 * @brief Read one tagged FIFO entry.
 *
 * @param[in] instance ADC instance index.
 * @return Raw FIFO word containing both sequence tag information and sample data.
 */
uint32_t ADC_DRV_ReadSeqtagAndData(const uint32_t instance);
/*! @}*/


/*!
 * @name Interrupt & Flag Control
 * @brief Functions for IRQ discovery and sticky-flag clearing.
 */
/*! @{*/

/*!
 * @brief Return the IRQ number associated with an ADC instance.
 *
 * @param[in] instance ADC instance index.
 * @return IRQ number for the selected ADC instance.
 */
IRQn_Type ADC_DRV_GetInterruptNumber(const uint32_t instance);

/*!
 * @brief Clear the watchdog event flag.
 *
 * @param[in] instance ADC instance index.
 */
void ADC_DRV_ClearWdFlagCmd(const uint32_t instance);

/*!
 * @brief Clear the ADC-ready flag.
 *
 * @param[in] instance ADC instance index.
 */
void ADC_DRV_ClearReadyFlagCmd(const uint32_t instance);

/*!
 * @brief Clear the sample-end flag.
 *
 * @param[in] instance ADC instance index.
 */
void ADC_DRV_ClearSampEndFlagCmd(const uint32_t instance);

/*!
 * @brief Clear the FIFO overrun flag.
 *
 * @param[in] instance ADC instance index.
 */
void ADC_DRV_ClearOvrFlagCmd(const uint32_t instance);

/*!
 * @brief Clear the end-of-sequence flag.
 *
 * @param[in] instance ADC instance index.
 */
void ADC_DRV_ClearEoseqFlagCmd(const uint32_t instance);

/*!
 * @brief Clear the end-of-conversion flag.
 *
 * @param[in] instance ADC instance index.
 */
void ADC_DRV_ClearEocFlagCmd(const uint32_t instance);

#if defined (__cplusplus)
}
#endif

/*! @}*/

#endif /* ADC_DRIVER_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
