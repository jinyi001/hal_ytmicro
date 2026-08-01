/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file sent_driver.h
 * @version 1.4.1
 * @brief SENT (Single Edge Nibble Transmission) driver public API.
 *
 * This file declares the public API, configuration structures, and type
 * definitions for the SENT receiver peripheral driver.
 * Refer to @ref sent for the complete module documentation and usage
 * patterns.
 */

#ifndef SENT_DRIVER_H
#define SENT_DRIVER_H

/* Required headers */
#include <stddef.h>
#include "device_registers.h"
#include "status.h"

/*!
 * @addtogroup sent
 * @{
 */

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Macros — Diagnostic Event Masks
 ******************************************************************************/

/*
 * Masks for various diagnostic events.
 * These can be combined: events |= (SENT_EV_1 | SENT_EV_2);
 * Also used inside the configuration structure to enable/disable events.
 */
#define SENT_EV_BUS_IDLE          ((uint32_t)1u << 0)  /*!< Channel has been idle for more than the allowed value */
#define SENT_EV_SPC_END           ((uint32_t)1u << 1)  /*!< Channel SPC end event happens */
#define SENT_EV_CAL_RESYNC        ((uint32_t)1u << 4)  /*!< Successive Calibration Check has failed three times */
#define SENT_EV_CAL_20_25         ((uint32_t)1u << 5)  /*!< Calibration pulse received on this channel has variation in between 20% to 25% from 56 ticks */
#define SENT_EV_SMSG_OFLW         ((uint32_t)1u << 6)  /*!< Slow message overflow */
#define SENT_EV_FMSG_OFLW         ((uint32_t)1u << 7)  /*!< Fast message overflow */
#define SENT_EV_PP_DIAG_ERR       ((uint32_t)1u << 8)  /*!< The ratio of calibration pulse length to overall message length (with pause pulse) is more than 1.5625% between two messages */
#define SENT_EV_CAL_LEN_ERR       ((uint32_t)1u << 9)  /*!< Calibration pulse is more than 56 ticks 25% */
#define SENT_EV_CAL_DIAG_ERR      ((uint32_t)1u << 10) /*!< Successive Calibration pulses differ by 1.56% */
#define SENT_EV_NIB_VAL_ERR       ((uint32_t)1u << 11) /*!< Any nibble data value <0 or >15 */
#define SENT_EV_SMSG_CRC_ERR      ((uint32_t)1u << 12) /*!< Checksum error in Slow Serial Message */
#define SENT_EV_FMSG_CRC_ERR      ((uint32_t)1u << 13) /*!< Checksum error in Fast Serial Message */
#define SENT_EV_NUM_EDGES_ERR     ((uint32_t)1u << 14) /*!<  Not the expected number of negative edges between calibration pulse */
#define SENT_EV_FIFO_OVERFLOW     ((uint32_t)1u << 17) /*!< Overflow in FIFO queue */
#define SENT_EV_FDMA_UNDERFLOW    ((uint32_t)1u << 18) /*!< Underflow in Fast message DMA registers */
#define SENT_EV_SDMA_UNDERFLOW    ((uint32_t)1u << 19) /*!< Underflow in Slow message DMA registers */
#define SENT_EV_ALL ((uint32_t)(SENT_EV_BUS_IDLE | SENT_EV_SPC_END | SENT_EV_CAL_RESYNC | SENT_EV_CAL_20_25 | SENT_EV_SMSG_OFLW \
                    | SENT_EV_FMSG_OFLW | SENT_EV_PP_DIAG_ERR | SENT_EV_CAL_LEN_ERR | SENT_EV_CAL_DIAG_ERR \
                    | SENT_EV_NIB_VAL_ERR  | SENT_EV_SMSG_CRC_ERR | SENT_EV_FMSG_CRC_ERR | SENT_EV_NUM_EDGES_ERR \
                    | SENT_EV_FIFO_OVERFLOW | SENT_EV_FDMA_UNDERFLOW | SENT_EV_SDMA_UNDERFLOW)) /*!< All event flags combined */
#define SENT_EV_NONE ((uint32_t)(~(SENT_EV_ALL))) /*!< No events (all flags cleared) */

/*******************************************************************************
 * Types
 ******************************************************************************/

/*!
 * @brief CRC computation method for Fast and Slow messages.
 *
 * Selects whether the STATUS nibble contributes an extra zero nibble to
 * the CRC XOR chain (recommended method) or not (legacy method).
 *
 * | Value                | Description                                              |
 * |----------------------|----------------------------------------------------------|
 * | SENT_CRC_RECOMMENDED | Additional 0 nibble XORed with the rest (SAE preferred). |
 * | SENT_CRC_LEGACY      | No additional 0 nibble XORed (legacy sensors).           |
 */
typedef enum
{
    SENT_CRC_RECOMMENDED,   /*!< Additional 0 data nibble XORed with the rest of the nibbles */
    SENT_CRC_LEGACY         /*!< No additional 0 data nibble is XORed */
} sent_msg_crc_t;

/*!
 * @brief Bus IDLE detection threshold configuration.
 *
 * Sets the maximum number of clock ticks for which the channel may be
 * idle before a bus-idle event is reported. Use SENT_BUS_IDLE_DISABLED
 * to suppress idle detection entirely.
 *
 * | Value                        | Idle Threshold |
 * |------------------------------|----------------|
 * | SENT_BUS_IDLE_DISABLED       | Disabled        |
 * | SENT_BUS_IDLE_245_CLK_TICKS  | 245 ticks       |
 * | SENT_BUS_IDLE_508_CLK_TICKS  | 508 ticks       |
 * | SENT_BUS_IDLE_1016_CLK_TICKS | 1016 ticks      |
 * | SENT_BUS_IDLE_2032_CLK_TICKS | 2032 ticks      |
 */
typedef enum
{
    SENT_BUS_IDLE_DISABLED,         /*!< Disable IDLE pulse */
    SENT_BUS_IDLE_245_CLK_TICKS,    /*!< IDLE pulse diagnostics with 245 ticks */
    SENT_BUS_IDLE_508_CLK_TICKS,    /*!< IDLE pulse diagnostics with 508 ticks */
    SENT_BUS_IDLE_1016_CLK_TICKS,   /*!< IDLE pulse diagnostics with 1016 ticks */
    SENT_BUS_IDLE_2032_CLK_TICKS    /*!< IDLE pulse diagnostics with 2032 ticks */
} sent_diag_idle_cnt_cfg_t;

/*!
 * @brief Calibration pulse variance tolerance configuration.
 *
 * Controls how far the received calibration pulse length may deviate
 * from the nominal 56-tick period before triggering a calibration error.
 *
 * | Value                     | Allowed Deviation |
 * |---------------------------|-------------------|
 * | SENT_CALIB_VAR_20_PERCENT | Up to 20%         |
 * | SENT_CALIB_VAR_25_PERCENT | Up to 25%         |
 */
typedef enum
{
    SENT_CALIB_VAR_20_PERCENT,   /*!< Calibration pulse with 20% */
    SENT_CALIB_VAR_25_PERCENT    /*!< Calibration pulse with 25% */
} sent_diag_calib_pulse_var_cfg_t;

/*!
 * @brief Diagnostics pulse check selection.
 *
 * Selects which calibration/pause-pulse diagnostic checks are enabled.
 *
 * | Value                  | Description                                                        |
 * |------------------------|--------------------------------------------------------------------|
 * | SENT_PULSE_CHECK_BOTH  | Run both successive calibration and pause pulse diagnostic checks. |
 * | SENT_PULSE_CHECK_PAUSE | Run only the pause pulse diagnostic check.                         |
 */
typedef enum
{
    SENT_PULSE_CHECK_BOTH,      /*!< Both successive calibration pulse check and pause pulse diagnostic are run */
    SENT_PULSE_CHECK_PAUSE      /*!< Only pause pulse diagnostic is run */
} sent_diag_pulse_cfg_t;

/*!
 * @brief Pause pulse enable/disable configuration.
 *
 * Controls whether the hardware recognises and checks the optional
 * SENT pause pulse that follows each Fast message frame.
 *
 * | Value                     | Description                   |
 * |---------------------------|-------------------------------|
 * | SENT_PAUSE_PULSE_DISABLED | Ignore the pause pulse.       |
 * | SENT_PAUSE_PULSE_ENABLED  | Detect and check pause pulse. |
 */
typedef enum
{
    SENT_PAUSE_PULSE_DISABLED,  /*!< Disable SENT pause pulse */
    SENT_PAUSE_PULSE_ENABLED    /*!< Enable SENT pause pulse */
} sent_diag_pause_pulse_cfg_t;

/*!
 * @brief Successive calibration check method selection.
 *
 * Chooses between the SAE J2716 low-latency and the preferred
 * (higher-latency) successive calibration check algorithms.
 *
 * | Value                         | Description                                              |
 * |-------------------------------|----------------------------------------------------------|
 * | SENT_SUCC_CAL_CHK_LOW_LATENCY | Low-latency option as per SAE J2716 specification.       |
 * | SENT_SUCC_CAL_CHK_PREFERRED   | Preferred (high-latency) option as per SAE specification. |
 */
typedef enum
{
    SENT_SUCC_CAL_CHK_LOW_LATENCY,  /*!< Select Low Latency Option as SAE Specification */
    SENT_SUCC_CAL_CHK_PREFERRED     /*!< Select Preferred but High Latency Option as SAE Specification. */
} sent_diag_succ_cal_check_cfg_t;

#if FEATURE_SENT_HAS_INPUT_FILTER
/*!
 * @brief Hardware glitch-filter width selection.
 *
 * Sets the minimum pulse duration (in protocol clocks) that the input
 * signal must maintain to be considered a valid edge. A value of
 * SENT_INPUT_FILTER_NONE disables the filter entirely.
 *
 * | Value                  | Filter Width      |
 * |------------------------|-------------------|
 * | SENT_INPUT_FILTER_NONE | No filtering      |
 * | SENT_INPUT_FILTER_2    | 2 function clocks |
 * | SENT_INPUT_FILTER_4    | 4 function clocks |
 * | SENT_INPUT_FILTER_8    | 8 function clocks |
 * | SENT_INPUT_FILTER_16   | 16 function clocks|
 * | SENT_INPUT_FILTER_32   | 32 function clocks|
 * | SENT_INPUT_FILTER_64   | 64 function clocks|
 * | SENT_INPUT_FILTER_128  | 128 function clocks|
 * | SENT_INPUT_FILTER_256  | 256 function clocks|
 */
typedef enum
{
    SENT_INPUT_FILTER_NONE,     /*!< No filtering */
    SENT_INPUT_FILTER_2,        /*!< Set filter as 2 function clocks */
    SENT_INPUT_FILTER_4,        /*!< Set filter as 4 function clocks */
    SENT_INPUT_FILTER_8,        /*!< Set filter as 8 function clocks */
    SENT_INPUT_FILTER_16,       /*!< Set filter as 16 function clocks */
    SENT_INPUT_FILTER_32,       /*!< Set filter as 32 function clocks */
    SENT_INPUT_FILTER_64,       /*!< Set filter as 64 function clocks */
    SENT_INPUT_FILTER_128,      /*!< Set filter as 128 function clocks */
    SENT_INPUT_FILTER_256,      /*!< Set filter as 256 function clocks */
} sent_channel_input_filter_t;
#endif /* FEATURE_SENT_HAS_INPUT_FILTER */

#if FEATURE_SENT_HAS_RECV_POLARITY
/*!
 * @brief SENT receive polarity configuration.
 *
 * Selects the active edge polarity for the SENT input pin. Choose based
 * on the sensor's idle-state electrical level.
 *
 * | Value                     | Idle Level | Detected Edge |
 * |---------------------------|------------|---------------|
 * | SENT_DETECT_NEGATIVE_EDGE | High       | Falling edge  |
 * | SENT_DETECT_POSITIVE_EDGE | Low        | Rising edge   |
 */
typedef enum
{
    SENT_DETECT_NEGATIVE_EDGE = 0,     /*!< High level is the bus idle state, SENT detects negative edge */
    SENT_DETECT_POSITIVE_EDGE = 1,     /*!< Low level is the bus idle state, SENT detects positive edge */
} sent_channel_recv_polarity_t;
#endif /* FEATURE_SENT_HAS_RECV_POLARITY */

/*!
 * @brief Slow message type identifier.
 *
 * Determined by the hardware at reception time based on the message
 * format bit pattern defined in SAE J2716.
 *
 * | Value                        | Format                          |
 * |------------------------------|---------------------------------|
 * | SENT_SLOW_TYPE_SHORT         | Standard: 4-bit ID, 8-bit Data  |
 * | SENT_SLOW_TYPE_ENHANCED_4_BIT| Enhanced: 4-bit ID, 16-bit Data |
 * | SENT_SLOW_TYPE_ENHANCED_8_BIT| Enhanced: 8-bit ID, 12-bit Data |
 */
typedef enum
{
    SENT_SLOW_TYPE_SHORT,           /*!< Standard 4 bit ID, 8 bit Data */
    SENT_SLOW_TYPE_ENHANCED_4_BIT,  /*!< Enhanced 4 bit ID, 16 bit Data */
    SENT_SLOW_TYPE_ENHANCED_8_BIT   /*!< Enhanced 8 bit ID, 12 bit Data */
} sent_slow_msg_type_t;

/*!
 * @brief Diagnostic event flag type.
 *
 * A bitmask of active diagnostic events. Use the @c SENT_EV_* macros
 * to test and combine individual event flags.
 */
typedef uint32_t sent_event_t;

/*!
 * @brief Callback event type enumeration.
 *
 * Identifies the reason a user callback is invoked by the driver.
 *
 * | Value                             | Trigger                                       |
 * |-----------------------------------|-----------------------------------------------|
 * | SENT_CALLBACK_SLOW_DMA_RX_COMPLETE| Slow DMA transfer buffer complete.            |
 * | SENT_CALLBACK_FAST_DMA_RX_COMPLETE| Fast DMA transfer buffer complete.            |
 * | SENT_CALLBACK_SLOW_RX_COMPLETE    | Slow message received (interrupt mode).       |
 * | SENT_CALLBACK_FAST_RX_COMPLETE    | Fast message received (interrupt mode).       |
 * | SENT_CALLBACK_RX_ERROR            | One or more diagnostic error events active.   |
 * | SENT_CALLBACK_RX_WAKEUP_EVENT     | Wakeup event detected on the SENT bus.        |
 */
typedef enum
{
    SENT_CALLBACK_SLOW_DMA_RX_COMPLETE,     /*!< Slow SENT DMA Rx Complete callback */
    SENT_CALLBACK_FAST_DMA_RX_COMPLETE,     /*!< Fast SENT DMA Rx Complete callback */
    SENT_CALLBACK_SLOW_RX_COMPLETE,         /*!< Slow SENT Rx Complete callback */
    SENT_CALLBACK_FAST_RX_COMPLETE,         /*!< Fast SENT Rx Complete callback */
    SENT_CALLBACK_RX_ERROR,                 /*!< Rx Error callback */
    SENT_CALLBACK_RX_WAKEUP_EVENT           /*!< Rx wakeup callback */
} sent_callback_type_t;

/*!
 * @brief Prototype for the user-supplied event callback function.
 *
 * The driver calls this function from ISR or DMA contexts whenever a
 * notable event occurs. The callback must complete quickly; blocking
 * operations must not be performed inside.
 *
 * @param[in] instance  Peripheral instance index.
 * @param[in] channel   Channel index that triggered the event.
 * @param[in] type      Event type identifier.
 * @param[in] param     Opaque user context pointer supplied at registration.
 */
typedef void(* sent_callback_func_t)(uint32_t instance, uint32_t channel, sent_callback_type_t type, void * param);

/*!
 * @brief Callback registration structure.
 *
 * Bundles the callback function pointer with an opaque user parameter
 * that is forwarded unmodified to every invocation.
 */
typedef struct
{
    sent_callback_func_t function;  /*!< Callback function pointer; NULL disables notifications */
    void * param;                   /*!< Opaque user context pointer passed to every callback */
} sent_callback_t;

/*!
 * @brief Fast message reception configuration.
 *
 * Controls all aspects of Fast message decoding for a single channel,
 * including the nibble count, CRC settings, and DMA enablement.
 */
typedef struct
{
    uint8_t numberOfNibbles;    /*!< Number of data nibbles per frame (valid range: 1–6) */
    bool dmaEnable;             /*!< Enable DMA transfers for this channel */
    bool crcIncludeStatus;      /*!< Include the STATUS nibble in the CRC calculation */
    bool disableCrcCheck;       /*!< Disable CRC verification for this channel */
    sent_msg_crc_t crcType;     /*!< CRC computation method (recommended or legacy) */
} sent_fast_msg_config_t;

/*!
 * @brief Slow message reception configuration.
 *
 * Controls CRC method and DMA enablement for Slow serial message decoding.
 */
typedef struct
{
    bool dmaEnable;         /*!< Enable DMA transfers for this channel */
    sent_msg_crc_t crcType; /*!< CRC computation method (recommended or legacy) */
} sent_slow_msg_config_t;

/*!
 * @brief Channel diagnostics configuration.
 *
 * Controls all diagnostic and error-detection settings for one SENT
 * channel. For a complete description of each field, refer to the
 * SAE J2716 specification and the device user manual.
 */
typedef struct
{
    sent_event_t diagEvents;                     /*!< Bitmask of events that trigger an error callback */
    sent_diag_idle_cnt_cfg_t idleCount;          /*!< Maximum allowed bus idle time threshold */
    sent_diag_calib_pulse_var_cfg_t calibVar;    /*!< Calibration pulse variance tolerance */
    sent_diag_pulse_cfg_t diagPulse;             /*!< Diagnostic pulse check selection */
    sent_diag_pause_pulse_cfg_t pausePulse;      /*!< Pause pulse enable/disable selection */
    sent_diag_succ_cal_check_cfg_t successiveCal; /*!< Successive calibration check method */
} sent_diag_config_t;

#if FEATURE_SENT_HAS_SPC
/*!
 * @brief SPC pulse polarity selection.
 *
 * Selects the electrical polarity of the Short PWM Code (SPC) trigger
 * pulse output on the channel pin.
 *
 * | Value          | Description                       |
 * |----------------|-----------------------------------|
 * | SPC_LOW_PULSE  | SPC trigger is a low-level pulse. |
 * | SPC_HIGH_PULSE | SPC trigger is a high-level pulse.|
 */
typedef enum
{
    SPC_LOW_PULSE  = 0U,            /*!< SPC pulse is a low level pulse */
    SPC_HIGH_PULSE = 1U             /*!< SPC pulse is a high level pulse */
} spc_pulse_polarity_t;

/*!
 * @brief SPC operating mode selection.
 *
 * Determines whether the SPC trigger pulse and the SENT receive signal
 * share the same physical pad or use separate pads.
 *
 * | Value                     | Description                                        |
 * |---------------------------|----------------------------------------------------|
 * | SPC_SENT_ON_DIFFERENT_PAD | SPC pulse and SENT message use separate pads.     |
 * | SPC_SENT_ON_SAME_PAD      | SPC pulse and SENT message share the same pad.    |
 */
typedef enum
{
    SPC_SENT_ON_DIFFERENT_PAD = 0U,         /*!< SPC pulse and SENT message on different pad */
    SPC_SENT_ON_SAME_PAD      = 1U,         /*!< SPC pulse and SENT message on same pad */
} spc_work_mode_t;

/*!
 * @brief SPC trigger source selection.
 *
 * Controls whether the SPC pulse is driven by a software write or by
 * an external hardware trigger input.
 *
 * | Value                  | Description                                      |
 * |------------------------|--------------------------------------------------|
 * | SPC_GEN_BY_SW_TRIGGER  | SPC pulse generated only by software trigger.   |
 * | SPC_GEN_BY_HW_TRIGGER  | SPC pulse generated only by hardware trigger.   |
 */
typedef enum
{
    SPC_GEN_BY_SW_TRIGGER = 0U,             /*!< SPC pulse is only generated by software trigger */
    SPC_GEN_BY_HW_TRIGGER = 1U,             /*!< SPC pulse is only generated by hardware trigger */
} spc_trigger_t;

/*!
 * @brief SPC (Short PWM Code) pulse configuration for one channel.
 *
 * Configures the SPC trigger output used for sensor addressing in
 * bi-directional SENT topologies.
 */
typedef struct
{
    bool enable;                            /*!< Enable SPC mode on this channel */
    uint8_t pulseLength;                    /*!< SPC pulse length in tick units */
    spc_pulse_polarity_t pulsePolarity;     /*!< SPC pulse electrical polarity */
    spc_work_mode_t workMode;               /*!< SPC and SENT signal routing mode */
    spc_trigger_t triggerMode;              /*!< SPC trigger source selection */
} sent_spc_config_t;

#endif /* FEATURE_SENT_HAS_SPC */

/*!
 * @brief Per-channel configuration structure.
 *
 * Aggregates all settings required to configure one hardware SENT
 * receive channel within the peripheral.
 */
typedef struct
{
    uint8_t channelId;                       /*!< Hardware channel index (0..SENT_CH_COUNT-1) */
    uint8_t tickDuration;                    /*!< SENT tick duration in microseconds */
#if FEATURE_SENT_HAS_RECV_POLARITY
    sent_channel_recv_polarity_t recvPol;    /*!< Receive edge polarity selection */
#endif /* FEATURE_SENT_HAS_RECV_POLARITY */
#if FEATURE_SENT_HAS_INPUT_FILTER
    sent_channel_input_filter_t inputFilter; /*!< Hardware glitch-filter width */
#endif /* FEATURE_SENT_HAS_INPUT_FILTER */
    sent_diag_config_t diagConfig;           /*!< Diagnostics configuration sub-structure */
    sent_fast_msg_config_t fastMsgConfig;    /*!< Fast message channel configuration */
    sent_slow_msg_config_t slowMsgConfig;    /*!< Slow message channel configuration */
#if FEATURE_SENT_HAS_SPC
    sent_spc_config_t spcConfig;             /*!< SPC pulse configuration (feature-gated) */
#endif /* FEATURE_SENT_HAS_SPC */
} sent_channel_config_t;

/*!
 * @brief Raw DMA message data container.
 *
 * Represents the register-level binary format transferred by the DMA
 * engine from the peripheral Rx registers. Use SENT_DRV_GetFastMsgFromRaw()
 * or SENT_DRV_GetSlowMsgFromRaw() to convert to the structured message types.
 */
typedef struct
{
    uint32_t dataField0; /*!< Raw data register word [0] */
    uint32_t dataField1; /*!< Raw data register word [1] */
    uint32_t dataField2; /*!< Raw data register word [2] */
} sent_raw_msg_t;

/*!
 * @brief Decoded Fast message data structure.
 *
 * Contains the fully decoded fields of one received SENT Fast message,
 * as extracted from the peripheral receive registers.
 */
typedef struct
{
    uint32_t data;          /*!< Concatenated data nibble payload */
    uint32_t timeStamp;     /*!< Hardware reception timestamp */
    uint8_t channelNumber;  /*!< Source channel index (4-bit field) */
    uint8_t statusField;    /*!< STATUS nibble value (4-bit field) */
    uint8_t crc;            /*!< Received CRC nibble (4-bit field) */
} sent_fast_msg_t;

/*!
 * @brief Decoded Slow serial message data structure.
 *
 * Contains the fully decoded fields of one received SENT Slow message.
 * Field widths depend on the detected message type; refer to SAE J2716
 * for the exact bit-field mappings.
 */
typedef struct
{
    uint32_t timeStamp;          /*!< Hardware reception timestamp */
    uint16_t data;               /*!< Decoded data payload */
    sent_slow_msg_type_t type;   /*!< Detected slow message type */
    uint8_t id;                  /*!< Message ID field */
    uint8_t channelNumber;       /*!< Source channel index (4-bit field) */
    uint8_t crc;                 /*!< Received CRC value (up to 6 bits) */
} sent_slow_msg_t;

/*!
 * @brief Top-level driver configuration structure.
 *
 * Contains all global settings for one SENT peripheral instance,
 * including DMA buffer pointers, callback registration, and the
 * per-channel configuration array.
 *
 * @note DMA-related fields (fastMsgDmaPtr, slowMsgDmaPtr, etc.) are
 *       shared across all channels of a single peripheral instance.
 */
typedef struct
{
    sent_raw_msg_t * fastMsgDmaPtr; /*!< DMA destination buffer for Fast messages */
    sent_raw_msg_t * slowMsgDmaPtr; /*!< DMA destination buffer for Slow messages */
    sent_callback_t callbackFunc;   /*!< Event callback registration structure */
    uint8_t slowDmaChannel;         /*!< DMA channel number for Slow message transfers */
    uint8_t fastDmaChannel;         /*!< DMA channel number for Fast message transfers */
    bool fastDmaFIFOEnable;         /*!< Enable FIFO mode for Fast DMA transfers */
    uint8_t fastDmaFIFOSize;        /*!< FIFO watermark (messages before DMA trigger) */
    const sent_channel_config_t * channelConfig; /*!< Pointer to the channel configuration array */
    uint8_t numOfConfigs;           /*!< Number of entries in the channelConfig array */
} sent_driver_user_config_t;

/*!
 * @brief Driver internal state structure.
 *
 * Holds runtime state for one SENT peripheral instance. Must be
 * allocated by the application and passed to SENT_DRV_Init().
 * All fields inside are managed exclusively by the driver.
 */
typedef struct
{
/*! @cond DRIVER_INTERNAL_USE_ONLY */
    sent_raw_msg_t * fastMsgDmaPtr;            /*!< DMA transfer target pointer for fast messages */
    sent_raw_msg_t * slowMsgDmaPtr;            /*!< DMA transfer target pointer for slow messages */
    uint8_t fastDmaChannel;                    /*!< DMA channel used for Fast message transfers */
    uint8_t slowDmaChannel;                    /*!< DMA channel used for Slow message transfers */
    sent_callback_t callbackFunc;              /*!< Registered event callback */
    sent_event_t channelEvents[SENT_CH_COUNT]; /*!< Active event mask per channel */
    bool activeChannels[SENT_CH_COUNT];        /*!< TRUE if the channel has been configured */
    bool fastDmaEnabled[SENT_CH_COUNT];        /*!< TRUE if Fast DMA is enabled on this channel */
    bool slowDmaEnabled[SENT_CH_COUNT];        /*!< TRUE if Slow DMA is enabled on this channel */
    uint8_t instanceId;                        /*!< Peripheral instance index */
/*! @endcond */
} sent_state_t;

/*******************************************************************************
 * API Declarations
 ******************************************************************************/

/*!
 * @name Initialization and De-initialization
 * @{
 */

/*!
 * @brief Initialize the SENT driver for a peripheral instance.
 *
 * Configures the hardware timestamp prescaler, sets up all active channels
 * (prescaler, diagnostics, Fast/Slow message settings, optional DMA and SPC),
 * installs interrupt/DMA callbacks, and enables the peripheral.
 *
 * @param[in]  instance   SENT peripheral instance index (0..SENT_INSTANCE_COUNT-1).
 * @param[in]  configPtr  Pointer to the fully populated driver configuration
 *                        structure. Must not be NULL.
 * @param[out] state      Pointer to the driver state structure allocated by
 *                        the application. Must not be NULL.
 * @return STATUS_SUCCESS  Initialization succeeded.
 * @return STATUS_ERROR    The instance was already initialized.
 *
 * @pre  The SENT peripheral clock must be enabled via the clock manager
 *       and its frequency must be at least 1 MHz.
 * @pre  If DMA mode is used, the DMA driver must be initialized and channels
 *       allocated before calling this function.
 * @pre  Interrupt priorities must be configured before calling this function.
 * @post The peripheral is enabled and receiving data on all configured channels.
 * @note Call SENT_DRV_Deinit() before re-initializing an already-initialized instance.
 */
status_t SENT_DRV_Init(const uint32_t instance, const sent_driver_user_config_t * configPtr, sent_state_t * state);

/*!
 * @brief De-initialize the SENT driver and reset peripheral registers.
 *
 * Disables all channel interrupts, resets the peripheral to its power-on
 * state, and clears the internal state pointer so the instance can be
 * re-initialized.
 *
 * @param[in] instance  SENT peripheral instance index.
 * @return STATUS_SUCCESS  De-initialization succeeded.
 * @return STATUS_ERROR    The instance was not initialized.
 *
 * @post All channel interrupts are disabled and the peripheral is idle.
 * @post The instance may be re-initialized by calling SENT_DRV_Init().
 */
status_t SENT_DRV_Deinit(const uint32_t instance);

/*!
 * @brief Populate a configuration structure with TLE4998 sensor defaults.
 *
 * Fills @p config with a pre-defined default configuration suitable for
 * use with the Infineon TLE4998S SENT sensor (channel 0, 3 µs tick,
 * 6 data nibbles, recommended CRC, all diagnostics enabled).
 *
 * @param[out] config  Pointer to the configuration structure to populate.
 *                     The @c channelConfig pointer inside this structure
 *                     must be set to a valid channel configuration buffer
 *                     before calling this function.
 *
 * @pre  @c config->channelConfig must point to a valid writable buffer.
 * @note This function provides a reasonable starting configuration only.
 *       Modify the returned fields to match your sensor and application.
 */
void SENT_DRV_GetDefaultConfig(sent_driver_user_config_t * config);

/*! @} */

/*!
 * @name Fast Message Operations
 * @{
 */

/*!
 * @brief Check whether a new Fast message is available.
 *
 * Queries the hardware Rx-complete flag for the Fast message channel
 * without consuming the message data.
 *
 * @param[in] instance  SENT peripheral instance index.
 * @param[in] channel   Channel index to query (0..SENT_CH_COUNT-1).
 * @return true   A Fast message is waiting in the Rx buffer.
 * @return false  No message available, or the channel is not active.
 */
bool SENT_DRV_GetFastRxStatus(const uint32_t instance, const uint32_t channel);

/*!
 * @brief Read the most recently received Fast message.
 *
 * Reads the hardware receive registers, decodes them into the structured
 * @p message fields, and clears the Rx-complete flag.
 *
 * @param[in]  instance  SENT peripheral instance index.
 * @param[in]  channel   Channel index from which to read.
 * @param[out] message   Pointer to the structure that receives the decoded message.
 * @return STATUS_SUCCESS  Message read and decoded successfully.
 * @return STATUS_ERROR    The specified channel is not active.
 *
 * @pre  SENT_DRV_GetFastRxStatus() should return true before calling this function.
 * @note In DMA mode use SENT_DRV_GetFastMsgFromRaw() to decode from the DMA buffer.
 */
status_t SENT_DRV_GetFastMsg(const uint32_t instance, const uint32_t channel, sent_fast_msg_t * message);

/*!
 * @brief Update the DMA destination buffer for Fast messages.
 *
 * Changes the memory address to which the DMA controller writes incoming
 * Fast message raw data. The buffer must be large enough to hold
 * @c fastDmaFIFOSize elements of type @c sent_raw_msg_t when FIFO mode
 * is enabled.
 *
 * @param[in]  instance  SENT peripheral instance index.
 * @param[out] buffer    Pointer to the new DMA target buffer.
 * @return STATUS_SUCCESS  Buffer address updated successfully.
 *
 * @pre  The instance must be initialized with Fast DMA enabled.
 * @note This function may be called at runtime to implement a
 *       ping-pong buffering scheme.
 */
status_t SENT_DRV_SetFastMsgDmaBuffer(const uint32_t instance, sent_raw_msg_t * buffer);

/*!
 * @brief Convert a raw DMA Fast message to a structured Fast message.
 *
 * Performs bit-field extraction from the DMA raw register image and
 * populates the structured @p msg fields. Intended for use inside the
 * @c SENT_CALLBACK_FAST_DMA_RX_COMPLETE callback to decode DMA data.
 *
 * @param[out] msg     Pointer to the destination structured message.
 * @param[in]  rawMsg  Pointer to the source raw DMA message.
 */
void SENT_DRV_GetFastMsgFromRaw(sent_fast_msg_t * msg, const sent_raw_msg_t * rawMsg);

/*! @} */

/*!
 * @name Slow Message Operations
 * @{
 */

/*!
 * @brief Check whether a new Slow message is available.
 *
 * Queries the hardware Rx-complete flag for the Slow message channel
 * without consuming the message data.
 *
 * @param[in] instance  SENT peripheral instance index.
 * @param[in] channel   Channel index to query (0..SENT_CH_COUNT-1).
 * @return true   A Slow message is waiting in the Rx buffer.
 * @return false  No message available, or the channel is not active.
 */
bool SENT_DRV_GetSlowRxStatus(const uint32_t instance, const uint32_t channel);

/*!
 * @brief Read the most recently received Slow message.
 *
 * Reads the hardware receive registers, decodes them into the structured
 * @p message fields, and clears the Rx-complete flag.
 *
 * @param[in]  instance  SENT peripheral instance index.
 * @param[in]  channel   Channel index from which to read.
 * @param[out] message   Pointer to the structure that receives the decoded message.
 * @return STATUS_SUCCESS  Message read and decoded successfully.
 * @return STATUS_ERROR    The specified channel is not active.
 *
 * @pre  SENT_DRV_GetSlowRxStatus() should return true before calling this function.
 */
status_t SENT_DRV_GetSlowMsg(const uint32_t instance, const uint32_t channel, sent_slow_msg_t * message);

/*!
 * @brief Update the DMA destination buffer for Slow messages.
 *
 * Changes the memory address to which the DMA controller writes incoming
 * Slow message raw data.
 *
 * @param[in]  instance  SENT peripheral instance index.
 * @param[out] buffer    Pointer to the new DMA target buffer.
 * @return STATUS_SUCCESS  Buffer address updated successfully.
 *
 * @pre  The instance must be initialized with Slow DMA enabled.
 */
status_t SENT_DRV_SetSlowMsgDmaBuffer(const uint32_t instance, sent_raw_msg_t * buffer);

/*!
 * @brief Convert a raw DMA Slow message to a structured Slow message.
 *
 * Performs bit-field extraction from the DMA raw register image and
 * populates the structured @p msg fields. Intended for use inside the
 * @c SENT_CALLBACK_SLOW_DMA_RX_COMPLETE callback to decode DMA data.
 *
 * @param[out] msg     Pointer to the destination structured message.
 * @param[in]  rawMsg  Pointer to the source raw DMA message.
 */
void SENT_DRV_GetSlowMsgFromRaw(sent_slow_msg_t * msg, const sent_raw_msg_t * rawMsg);

/*! @} */

/*!
 * @name Callback and Notification
 * @{
 */

/*!
 * @brief Set or replace the event callback function.
 *
 * Installs @p function as the event notification callback for the specified
 * instance and reconfigures interrupt enable/disable states accordingly.
 * Passing NULL disables interrupt-driven notification and reverts the
 * instance to polled mode.
 *
 * @param[in] instance  SENT peripheral instance index.
 * @param[in] function  Callback function pointer; NULL to disable notifications.
 * @param[in] param     Opaque context pointer forwarded to every callback invocation.
 * @return STATUS_SUCCESS  Callback installed and interrupts reconfigured.
 *
 * @pre  The instance must be initialized.
 * @note May be called at runtime to switch between polled and interrupt modes.
 * @warning The callback executes in ISR context. Keep it short and do not
 *          call blocking operations from within.
 */
status_t SENT_DRV_SetRxCallbackFunction(const uint32_t instance, sent_callback_func_t function, void * param);

/*! @} */

/*!
 * @name Diagnostics and Events
 * @{
 */

/*!
 * @brief Read and atomically clear the active diagnostic event flags.
 *
 * Returns the current bitmask of active error/diagnostic events for the
 * given channel and clears the flags in hardware so that they do not
 * persist into the next read.
 *
 * @param[in]  instance  SENT peripheral instance index.
 * @param[in]  channel   Channel index to query.
 * @param[out] events    Pointer to the variable that receives the event bitmask.
 * @return STATUS_SUCCESS  Events read and cleared successfully.
 * @return STATUS_ERROR    The specified channel is not active.
 *
 * @note Use the @c SENT_EV_* macros to test individual event bits in @p events.
 */
status_t SENT_DRV_GetEvents(const uint32_t instance, const uint32_t channel, sent_event_t * events);

/*! @} */

#if FEATURE_SENT_HAS_SPC
/*!
 * @name SPC Control
 * @{
 */

/*!
 * @brief Generate a software SPC trigger pulse.
 *
 * Issues an immediate software trigger on the SPC output of the specified
 * channel. Only effective when the channel SPC trigger mode is configured
 * as @c SPC_GEN_BY_SW_TRIGGER.
 *
 * @param[in] instance  SENT peripheral instance index.
 * @param[in] channel   Channel on which to assert the SPC trigger pulse.
 *
 * @pre  The channel must be initialized with SPC mode enabled and
 *       @c triggerMode set to @c SPC_GEN_BY_SW_TRIGGER.
 * @warning Calling this function when the trigger mode is hardware-driven
 *          has no effect.
 */
void SENT_DRV_SwTrigger(uint32_t instance, uint8_t channel);

/*! @} */
#endif /* FEATURE_SENT_HAS_SPC */

#if defined(__cplusplus)
}
#endif

/*! @}*/

/*! @}*/ /* End of addtogroup sent */

#endif /* SENT_DRIVER_H */
