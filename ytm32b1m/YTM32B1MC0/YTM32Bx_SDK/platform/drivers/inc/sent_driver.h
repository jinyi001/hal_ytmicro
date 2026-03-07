/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file sent_driver.h
 * @version 1.4.0
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

/*
 * Masks for various events
 * These can be combined such as: events |= (EV_1 | EV_2);
 * Also used inside the configuration structure for enabling/disabling certain events
 */
#define SENT_EV_BUS_IDLE          ((uint32_t)1u << 0) /*!< Channel has been idle for more than the allowed value */
#define SENT_EV_SPC_END           ((uint32_t)1u << 1) /*!< Channel SPC end event happens */
#define SENT_EV_CAL_RESYNC        ((uint32_t)1u << 4) /*!< Successive Calibration Check has failed three times */
#define SENT_EV_CAL_20_25         ((uint32_t)1u << 5) /*!< Calibration pulse received on this channel has variation in between 20% to 25% from 56 ticks */
#define SENT_EV_SMSG_OFLW         ((uint32_t)1u << 6) /*!< Slow message overflow */
#define SENT_EV_FMSG_OFLW         ((uint32_t)1u << 7) /*!< Fast message overflow */
#define SENT_EV_PP_DIAG_ERR       ((uint32_t)1u << 8) /*!< The ratio of calibration pulse length to overall message length (with pause pulse) is more than 1.5625% between two messages */
#define SENT_EV_CAL_LEN_ERR       ((uint32_t)1u << 9) /*!< Calibration pulse is more than 56 ticks 25% */
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
                    | SENT_EV_FIFO_OVERFLOW | SENT_EV_FDMA_UNDERFLOW | SENT_EV_SDMA_UNDERFLOW)) /*!< All flags */
#define SENT_EV_NONE ((uint32_t)(~(SENT_EV_ALL))) /*!< No events */
/**
 * @brief Fast/Slow CRC type
 *
 * Contains the possible values for the configuration
 * of the CRC type.
 *
 * Implements : sent_msg_crc_t_Class
 */
typedef enum
{
    SENT_CRC_RECOMMENDED,   /*!< Additional 0 data nibble XORed with the rest of the nibbles */
    SENT_CRC_LEGACY         /*!< No additional 0 data nibble is XORed */
} sent_msg_crc_t;

/**
 * @brief Idle check configuration
 *
 * Contains the possible values use in setting
 * up the IDLE pulse diagnostics.
 *
 * Implements : sent_diag_idle_cnt_cfg_t_Class
 */
typedef enum
{
    SENT_BUS_IDLE_DISABLED,         /*!< Disable IDLE pulse */
    SENT_BUS_IDLE_245_CLK_TICKS,    /*!< IDLE pulse diagnostics with 245 ticks */
    SENT_BUS_IDLE_508_CLK_TICKS,    /*!< IDLE pulse diagnostics with 508 ticks */
    SENT_BUS_IDLE_1016_CLK_TICKS,   /*!< IDLE pulse diagnostics with 1016 ticks */
    SENT_BUS_IDLE_2032_CLK_TICKS    /*!< IDLE pulse diagnostics with 2032 ticks */
} sent_diag_idle_cnt_cfg_t;

/**
 * @brief Calibration configuration
 *
 * Contains the allowed values for configuring
 * the variance in the calibration pulse.
 *
 * Implements : sent_diag_calib_pulse_var_cfg_t_Class
 */
typedef enum
{
    SENT_CALIB_VAR_20_PERCENT,   /*!< Calibration pulse with 20% */
    SENT_CALIB_VAR_25_PERCENT    /*!< Calibration pulse with 25% */
} sent_diag_calib_pulse_var_cfg_t;

/**
 * @brief Diagnostics pulse configuration
 *
 * Contains the possible value for configuring
 * the diagnostics pulse checks.
 *
 * Implements : sent_diag_pulse_cfg_t_Class
 */
typedef enum
{
    SENT_PULSE_CHECK_BOTH,      /*!< Both successive calibration pulse check and pause pulse diagnostic are run */
    SENT_PULSE_CHECK_PAUSE      /*!< Only pause pulse diagnostic is run */
} sent_diag_pulse_cfg_t;

/**
 * @brief Pause pulse configuration
 *
 * Contains the possible value of the PAUSE pulse
 * configuration.
 *
 * Implements : sent_diag_pause_pulse_cfg_t_Class
 */
typedef enum
{
    SENT_PAUSE_PULSE_DISABLED,  /*!< Disable SENT pause pulse */
    SENT_PAUSE_PULSE_ENABLED    /*!< Enable SENT pause pulse */
} sent_diag_pause_pulse_cfg_t;

/**
 * @brief Successive calibration check method
 *
 * This contains the values use in the configuration
 * for the successive calibration pulse check field.
 *
 * Implements : sent_diag_succ_cal_check_cfg_t_Class
 */
typedef enum
{
    SENT_SUCC_CAL_CHK_LOW_LATENCY,  /*!< Select Low Latency Option as SAE Specification */
    SENT_SUCC_CAL_CHK_PREFERRED     /*!< Select Preferred but High Latency Option as SAE Specification. */
} sent_diag_succ_cal_check_cfg_t;

#if FEATURE_SENT_HAS_INPUT_FILTER
/**
 * @brief Input filter settings
 *
 * Contains all the possible values for configuring
 * the channel hardware input filter (glitch filter).
 * It is expressed in a number of protocol clocks
 * during which the signal should maintain it's value
 * to be considered stable.
 *
 * Implements : sent_channel_input_filter_t_Class
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
/**
 * @brief SENT receive polarity setting
 *
 * Implements : sent_channel_recv_polarity_t_Class
 */
typedef enum
{
    SENT_DETECT_NEGATIVE_EDGE = 0,     /*!< High level is the bus idle state, SENT detects negative edge */
    SENT_DETECT_POSITIVE_EDGE = 1,     /*!< Low level is the bus idle state, SENT detects positive edge */
} sent_channel_recv_polarity_t;
#endif /* FEATURE_SENT_HAS_RECV_POLARITY */

/**
 * @brief Slow message type
 *
 * This is determined by the peripheral, at reception.
 *
 * Implements : sent_slow_msg_type_t_Class
 */
typedef enum
{
    SENT_SLOW_TYPE_SHORT,           /*!< Standard 4 bit ID, 8 bit Data */
    SENT_SLOW_TYPE_ENHANCED_4_BIT,  /*!< Enhanced 4 bit ID, 16 bit Data */
    SENT_SLOW_TYPE_ENHANCED_8_BIT   /*!< Enhanced 8 bit ID, 12 bit Data */
} sent_slow_msg_type_t;

/**
 * @brief Type of a SENT event
 *
 * Implements : sent_event_t_Class
 */
typedef uint32_t sent_event_t;

/**
 * @brief Type of issued callback
 *
 * Implements : sent_callback_type_t_Class
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

/**
 * @brief Type of callback function
 *
 * Implements : sent_callback_func_t_Class
 */
typedef void(* sent_callback_func_t)(uint32_t instance, uint32_t channel, sent_callback_type_t type, void * param);

/**
 * @brief Type of callback structure
 *
 * Implements : sent_callback_t_Class
 */
typedef struct
{
    sent_callback_func_t function;  /*!< Callback function */
    void * param;                   /*!< User parameter */
} sent_callback_t;

/**
 * @brief Fast messages configuration
 *
 * Contains all the necessary fields used in the
 * configuration of the fast message channel.
 *
 * Implements : sent_fast_msg_config_t_Class
 */
typedef struct
{
    /* Message length */
    uint8_t numberOfNibbles;    /*!< Number of nibbles for the message. Valid values 1 >= x <= 6 */

    /* DMA enabler for the specific channel */
    bool dmaEnable;             /*!< Enable DMA transfers */

    /* CRC related */
    bool crcIncludeStatus;      /*!< Include the STATUS nibble in the CRC calculation */
    bool disableCrcCheck;       /*!< Disable CRC checks for the channel */
    sent_msg_crc_t crcType;     /*!< CRC type */
} sent_fast_msg_config_t;

/**
 * @brief Slow messages configuration
 *
 * Contains all the necessary fields used in the
 * configuration of the slow message channel.
 *
 * Implements : sent_slow_msg_config_t_Class
 */
typedef struct
{
    /* DMA enabler for the specific channel */
    bool dmaEnable;         /*!< Enable DMA transfers */

    /* CRC related */
    sent_msg_crc_t crcType; /*!< CRC type */
} sent_slow_msg_config_t;

/**
 * @brief Diagnostics configuration
 *
 * Contains all the fields used in the configuration of the
 * diagnostics side of the driver. For a better understanding
 * of these settings, the SAE J2716 specification and
 * the user manual for the current part should be read.
 *
 * Implements : sent_diag_config_t_Class
 */
typedef struct
{
    sent_event_t diagEvents;                     /*!< Flags for active diagnostics events */
    sent_diag_idle_cnt_cfg_t idleCount;          /*!< Maximum allowed IDLE time */
    sent_diag_calib_pulse_var_cfg_t calibVar;    /*!< Valid calibration pulse range */
    sent_diag_pulse_cfg_t diagPulse;             /*!< Selection for which diagnostics to run for pulses*/
    sent_diag_pause_pulse_cfg_t pausePulse;      /*!< Selection of PAUSE pulse*/
    sent_diag_succ_cal_check_cfg_t successiveCal; /*!< Successive calibration check method */
} sent_diag_config_t;

#if FEATURE_SENT_HAS_SPC
/**
 * @brief SPC pulse polarity enum
 *
 * Implements : spc_pulse_polarity_t_Class
 */
typedef enum
{
    SPC_LOW_PULSE  = 0U,            /*!< SPC pulse is a low level pulse */
    SPC_HIGH_PULSE = 1U             /*!< SPC pulse is a high level pulse */
} spc_pulse_polarity_t;

/**
 * @brief SPC work mode selection
 *
 * Implements : spc_work_mode_t_Class
 */
typedef enum
{
    SPC_SENT_ON_DIFFERENT_PAD = 0U,         /*!< SPC pulse and SENT message on different pad */
    SPC_SENT_ON_SAME_PAD      = 1U,         /*!< SPC pulse and SENT message on different pad */
} spc_work_mode_t;

/**
 * @brief SPC pulse trigger mode selection
 *
 * Implements : spc_trigger_t_Class
 */
typedef enum
{
    SPC_GEN_BY_SW_TRIGGER = 0U,             /*!< SPC pulse is only generated by software trigger */
    SPC_GEN_BY_HW_TRIGGER = 1U,             /*!< SPC pulse is only generated by hardware trigger */
} spc_trigger_t;

/**
 * @brief SENT channel SPC configuration structure
 *
 * Contains all the required fields for the SPC configuration
 * of a single channel inside the peripheral.
 *
 * Implements : sent_spc_config_t_Class
 */
typedef struct
{
    bool enable;                            /*!< Enable SPC mode. */
    uint8_t pulseLength;                    /*!< SPC pulse length is this field multiply ticks. */
    spc_pulse_polarity_t pulsePolarity;     /*!< SPC pulse polarity selection. */
    spc_work_mode_t workMode;               /*!< SPC work mode selection. */
    spc_trigger_t triggerMode;              /*!< SPC trigger mode selection. */
} sent_spc_config_t;

#endif /* FEATURE_SENT_HAS_SPC */

/**
 * @brief Channel configuration structure
 *
 * Contains all the required fields for the configuration
 * of a single channel inside the peripheral.
 *
 * Implements : sent_channel_config_t_Class
 */
typedef struct
{
    uint8_t channelId;                       /*!< Designated channel */
    uint8_t tickDuration;                    /*!< Tick duration in microseconds */
#if FEATURE_SENT_HAS_RECV_POLARITY
    sent_channel_recv_polarity_t recvPol;    /*!< SENT receive polarity setting */
#endif /* FEATURE_SENT_HAS_RECV_POLARITY */
#if FEATURE_SENT_HAS_INPUT_FILTER
    sent_channel_input_filter_t inputFilter; /*!< Channel input filter configuration */
#endif /* FEATURE_SENT_HAS_INPUT_FILTER */
    sent_diag_config_t diagConfig;           /*!< Diagnostics configuration structure*/
    sent_fast_msg_config_t fastMsgConfig;    /*!< Fast messages configuration structure */
    sent_slow_msg_config_t slowMsgConfig;    /*!< Slow messages configuration structure */
#if FEATURE_SENT_HAS_SPC
    sent_spc_config_t spcConfig;             /*!< SPC configuration structure */
#endif /* FEATURE_SENT_HAS_SPC */
} sent_channel_config_t;

/**
 * @brief Raw message data type
 *
 * Raw message type. It is the actual format transferred by the
 * DMA peripheral and contained inside the Rx channel registers.
 *
 * Implements : sent_raw_msg_t_Class
 */
typedef struct
{
    uint32_t dataField0; /*!< Raw data field [0] */
    uint32_t dataField1; /*!< Raw data field [1] */
    uint32_t dataField2; /*!< Raw data field [2] */
} sent_raw_msg_t;

/**
 * @brief Actual format for a Fast message
 *
 * Contains the actual fields of a complete FAST message.
 *
 * Implements : sent_fast_msg_t_Class
 */
typedef struct
{
    uint32_t data;          /*!< Data field formed from all possible nibbles */
    uint32_t timeStamp;     /*!< Message timestamp */
    uint8_t channelNumber;  /*!< Channel number, 4 bits */
    uint8_t statusField;    /*!< Status field for the message, 2 bits */
    uint8_t crc;            /*!< CRC for the fast message, 4 bits */
} sent_fast_msg_t;

/**
 * @brief Actual format for a Slow message
 *
 * Contains the actual fields of a complete SLOW message
 * Lengths for the actual fields depend on the actual message type
 * Please see the SAE J2716 specification.
 *
 * Implements : sent_slow_msg_t_Class
 */
typedef struct
{
    uint32_t timeStamp;          /*!< Message timestamp */
    uint16_t data;               /*!< Data field */
    sent_slow_msg_type_t type;   /*!< Message type */
    uint8_t id;                  /*!< Message ID */
    uint8_t channelNumber;       /*!< Channel number, 4 bits */
    uint8_t crc;                 /*!< CRC for the slow message, max 6 bits */
} sent_slow_msg_t;

/**
 * @brief Configuration structure
 *
 * Contains all the fields necessary for a complete
 * configuration of the peripheral
 *
 * Implements : sent_driver_user_config_t_Class
 */
typedef struct
{
    /* Dma related transfers are common to all channels */
    sent_raw_msg_t * fastMsgDmaPtr; /*!< Initial DMA target buffer */
    sent_raw_msg_t * slowMsgDmaPtr; /*!< Initial DMA target buffer */
    sent_callback_t callbackFunc;   /*!< Callback function */
    uint8_t slowDmaChannel;         /*!< Assigned DMA channel for slow messages */
    uint8_t fastDmaChannel;         /*!< Assigned DMA channel for fast messages */
    bool fastDmaFIFOEnable;         /*!< Enable FIFO for Fast DMA transfers */
    uint8_t fastDmaFIFOSize;        /*!< Minimum FIFO size to trigger a Fast DMA transfer */

    /* Unique instances for each channel */
    const sent_channel_config_t * channelConfig; /*!< Pointer to a channel configuration list */
    uint8_t numOfConfigs;           /*!< Number of configurations in the channel configuration list */
} sent_driver_user_config_t;

/**
 * @brief State structure
 *
 * Contains internal driver state data.
 *
 * Implements : sent_state_t_Class
 */
typedef struct
{
/*! @cond DRIVER_INTERNAL_USE_ONLY */
    /* Configuration data */
    sent_raw_msg_t * fastMsgDmaPtr;            /*!< DMA transfer target pointer for fast messages */
    sent_raw_msg_t * slowMsgDmaPtr;            /*!< DMA transfer target pointer for slow messages */
    uint8_t fastDmaChannel;                    /*!< DMA channel on which fast message transfers are performed */
    uint8_t slowDmaChannel;                    /*!< DMA channel on which slow message transfers are performed */
    sent_callback_t callbackFunc;              /*!< Callback function */
    sent_event_t channelEvents[SENT_CH_COUNT]; /*!< Event configuration for each channel */
    bool activeChannels[SENT_CH_COUNT];        /*!< Activation state for the specific channel */
    bool fastDmaEnabled[SENT_CH_COUNT];        /*!< Holds the state for the Fast DMA channel */
    bool slowDmaEnabled[SENT_CH_COUNT];        /*!< Holds the state for the Slow DMA channel */
    uint8_t instanceId; /*!< Instance Id number */
/*! @endcond */
} sent_state_t;

/**
 * @brief Main initializer for the driver
 *
 * Initializes the driver for a given peripheral
 * according to the given configuration structure.
 *
 * @param[in] instance Instance of the SENT peripheral
 * @param[in] configPtr Pointer to the configuration structure
 * @param[out] state Pointer to the state structure to populate
 * @return The status of the operation
 */
status_t SENT_DRV_Init(const uint32_t instance, const sent_driver_user_config_t * configPtr, sent_state_t * state);

/**
 * @brief Event list getter
 *
 * Returns a list containing masks for the current active events.
 * Also clears the active events in the process.
 *
 * @param[in] instance Peripheral instance number
 * @param[in] channel Channel for which the mask shall be read
 * @param[out] events Pointer to a sent_event_t type in which to put the mask
 * @return The status of the operation
 */
status_t SENT_DRV_GetEvents(const uint32_t instance, const uint32_t channel, sent_event_t * events);

/**
 * @brief Gets RX status for the Fast channel
 *
 * Returns the buffer status for any incoming FAST message.
 *
 * @param[in] instance Peripheral instance number
 * @param[in] channel Channel for which the request is made
 * @return TRUE if there is a message in the RX buffer
 */
bool SENT_DRV_GetFastRxStatus(const uint32_t instance, const uint32_t channel);

/**
 * @brief Returns last received Fast message
 *
 * Returns last received fast message and clears the
 * Rx complete flag.
 *
 * @param[in] instance Peripheral instance number
 * @param[in] channel Target channel
 * @param[out] message Pointer to the target structure
 * @return The status of the operation
 */
status_t SENT_DRV_GetFastMsg(const uint32_t instance, const uint32_t channel, sent_fast_msg_t * message);

/**
 * @brief Sets the target buffer for reception
 *
 * Sets (modifies) the buffer in which the DMA driven
 * reception for Fast messages is made. Length of the
 * buffer must be (fastDmaFIFOSize)
 * bytes in case fastDmaFIFOEnable is TRUE.
 *
 * @param[in] instance Peripheral instance number
 * @param[out] buffer Pointer to the target raw buffer
 * @return The status of the operation
 */
status_t SENT_DRV_SetFastMsgDmaBuffer(const uint32_t instance, sent_raw_msg_t * buffer);

/**
 * @brief Raw to normal fast message transformation
 *
 * Transforms a RAW fast message into a normal fast message.
 *
 * @param[in] msg Pointer to a structure which will contain the fast message
 * @param[in] rawMsg pointer to a raw message
 */
void SENT_DRV_GetFastMsgFromRaw(sent_fast_msg_t * msg, const sent_raw_msg_t * rawMsg);

/**
 * @brief Gets RX status for the Slow channel
 *
 * Returns the buffer status for any incoming SLOW message.
 *
 * @param[in] instance Peripheral instance number
 * @param[in] channel Channel for which the request is made
 * @return TRUE if there is a message in the RX buffer
 */
bool SENT_DRV_GetSlowRxStatus(const uint32_t instance, const uint32_t channel);

/**
 * @brief Returns last received slow message
 *
 * Returns last received slow message and clears the
 * Rx complete flag.
 *
 * @param[in] instance Peripheral instance number
 * @param[in] channel Target channel
 * @param[out] message Pointer to the target structure
 * @return The status of the operation
 */
status_t SENT_DRV_GetSlowMsg(const uint32_t instance, const uint32_t channel, sent_slow_msg_t * message);

/**
 * @brief Sets the target buffer for reception
 *
 * Sets (modifies) the buffer in which the DMA driven
 * reception for slow messages is made.
 *
 * @param[in] instance Peripheral instance number
 * @param[out] buffer Pointer to the target raw buffer
 * @return The status of the operation
 */
status_t SENT_DRV_SetSlowMsgDmaBuffer(const uint32_t instance, sent_raw_msg_t * buffer);

/**
 * @brief Raw to normal slow message transformation
 *
 * Transforms a RAW slow message into a normal slow message.
 *
 * @param[in] msg Pointer to a structure which will contain the slow message
 * @param[in] rawMsg pointer to a raw message
 */
void SENT_DRV_GetSlowMsgFromRaw(sent_slow_msg_t * msg, const sent_raw_msg_t * rawMsg);

/**
 * @brief Set peripheral Rx notification
 *
 * Sets a notification function.
 * If the given function is NULL, it disables interrupt mode.
 *
 * @param[in] instance Peripheral instance number
 * @param[in] function Callback function
 * @param[in] param Callback parameter
 * @return The status of the operation
 */
status_t SENT_DRV_SetRxCallbackFunction(const uint32_t instance, sent_callback_func_t function, void * param);

/**
 * @brief Reset the peripheral.
 *
 * De-Initializes the peripheral and brings it's registers into a reset state.
 *
 * @param[in] instance Peripheral instance number
 * @return The status of the operation
 */
status_t SENT_DRV_Deinit(const uint32_t instance);

/**
 * @brief Default configuration structure.
 *
 * Returns a default configuration for the TLE4998 sensor
 *
 * @param[out] config Default configuration structure
 */
void SENT_DRV_GetDefaultConfig(sent_driver_user_config_t * config);

#if FEATURE_SENT_HAS_SPC
/**
 * @brief Generate a software trigger for SPC pulse
 *
 * Generate a software trigger for SPC pulse.
 *
 * @param[in] instance The SENT peripheral instance
 * @param[in] channel The requested channel
 */
void SENT_DRV_SwTrigger(uint32_t instance, uint8_t channel);
#endif /* FEATURE_SENT_HAS_SPC */

#if defined(__cplusplus)
}
#endif

/*! @}*/

/*! @}*/ /* End of addtogroup sent */

#endif /* SENT_DRIVER_H */
