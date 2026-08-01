/*
 * Copyright 2020-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file etmr_common.h
 * @version 1.4.1
 *
 * @brief eTMR Common Driver — public API declarations and shared data types.
 *
 * This header defines the shared enumerations, structures, and driver
 * state used by all eTMR sub-modules (PWM, IC, OC, MC, QD). It also
 * declares the common API functions for initialization, counter control,
 * synchronization, interrupt management, and status flag access.
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 0779 Rule 5.2: Identifier does not differ from other identifier(s) (e.g. '%1s') 
 *                       within the specified number of significant characters.
 */

#ifndef eTMR_COMMON_H
#define eTMR_COMMON_H

#include <stddef.h>
#include "status.h"
#include "device_registers.h"
#include "callbacks.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*! @brief Table of base addresses for eTMR instances. */
extern eTMR_Type *const g_etmrBase[eTMR_INSTANCE_COUNT];
extern const uint8_t g_etmrChannelNum[eTMR_INSTANCE_COUNT];

/*! @brief Interrupt vectors for the eTMR peripheral. */
extern IRQn_Type g_etmrIrqId[eTMR_INSTANCE_COUNT][FEATURE_eTMR_CHANNEL_INTERRUPT_COUNTS];

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!< @brief Channel number for CHAN0.*/
#define CHAN0_IDX (0U)
/*!< @brief Channel number for CHAN1.*/
#define CHAN1_IDX (1U)
/*!< @brief Channel number for CHAN2.*/
#define CHAN2_IDX (2U)
/*!< @brief Channel number for CHAN3.*/
#define CHAN3_IDX (3U)
/*!< @brief Channel number for CHAN4.*/
#define CHAN4_IDX (4U)
/*!< @brief Channel number for CHAN5.*/
#define CHAN5_IDX (5U)
/*!< @brief Channel number for CHAN6.*/
#define CHAN6_IDX (6U)
/*!< @brief Channel number for CHAN7.*/
#define CHAN7_IDX (7U)

#define TOTAL_CHAN (8U)
#define CAPTURE_COUNT (3U)
/*******************************************************************************
 * Enumerations
 ******************************************************************************/
/**
 * @brief eTMR Channel Mode
 *
 */
typedef enum
{
    eTMR_CHANNEL_DISABLE = 0x0U,    /*!< Channel disable */
    eTMR_PWM_MODE = 0x1U,           /*!< PWM mode */
    eTMR_COMPARE_MODE = 0x2U,       /*!< Compare mode */
    eTMR_INPUT_CAPTURE_MODE = 0x3U, /*!< Input capture mode */
} etmr_channel_mode_t;

/**
 * @brief eTMR PWM mode
 *
 */
typedef enum
{
    PWM_INDEPENDENT_MODE = 0,   /*!< Independent mode */
    PWM_COMPLEMENTARY_MODE = 1, /*!< Complementary mode */
} etmr_pwm_mode_t;

/*!
 * @brief eTMR Configure type of PWM update in the duty cycle or in ticks
 *
 */
typedef enum
{
    eTMR_PWM_UPDATE_IN_DUTY_CYCLE = 0x00U, /*!< The type of PWM update in the duty cycle/pulse */
    eTMR_PWM_UPDATE_IN_TICKS = 0x01U,      /*!< The type of PWM update in ticks which is filled into registers */
} etmr_pwm_update_option_t;

/*!
 * @brief eTMR PWM period unit
 *
 */
typedef enum
{
    eTMR_PWM_PERIOD_IN_HZ = 0U,    /*!< The type of PWM period in Hz */
    eTMR_PWM_PERIOD_IN_TICKS = 1U, /*!< The type of PWM period in ticks */
} etmr_pwm_period_unit_t;

/*!
 * @brief eTMR PWM align mode
 *
 */
typedef enum
{
    eTMR_PWM_RIGHT_EDGE_ALIGN = 0U, /*!< Rising edge aligned pwm */
    eTMR_PWM_LEFT_EDGE_ALIGN = 1U,  /*!< Falling edge aligned pwm */
    eTMR_PWM_CENTER_ALIGN = 2U,     /*!< Center aligned pwm */
    eTMR_PWM_ASYMMETRICAL = 3U,     /*!< Asymmetrical pwm*/
} etmr_pwm_align_mode_t;

/*!
 * @brief eTMR clock source selection
 *
 */
typedef enum
{
    eTMR_CLOCK_SOURCE_INTERNALCLK = 0x00U, /*!< Internal clock */
    eTMR_CLOCK_SOURCE_EXTERNALCLK = 0x01U, /*!< External clock */
    eTMR_CLOCK_SOURCE_EXT_IPC_CLK = 0x02U, /*!< IPC clock */
} etmr_clock_source_t;

/*!
 * @brief eTMR quadrature decoder modes, phase encode or count and direction mode
 *
 */
typedef enum
{
    eTMR_QUAD_PHASE_B_FOLLOW_A = 0U,         /*!< Phase A and B, B follow A, counter increase, A follow B, counter decrease */
    eTMR_QUAD_PHASE_A_FOLLOW_B = 1U,         /*!< Phase A and B, A follow B, counter increase, B follow A, counter decrease */
    eTMR_QUAD_COUNT_AND_DIR_BY_PHASE_B = 2U, /*!< Counting and direction mode, phase B controls direction */
    eTMR_QUAD_COUNT_AND_DIR_BY_PHASE_A = 3U, /*!< Counting and direction mode, phase A controls direction */ /* PRQA S 0779 */
} etmr_qd_mode_t;

/*!
 * @brief eTMR quadrature decoder clock prescaler
 *
 */
typedef enum
{
    eTMR_QD_CLOCK_DIV_BY_1 = 0x00U,  /*!< Divide by 1   */
    eTMR_QD_CLOCK_DIV_BY_2 = 0x01U,  /*!< Divide by 2   */
    eTMR_QD_CLOCK_DIV_BY_4 = 0x02U,  /*!< Divide by 4   */
    eTMR_QD_CLOCK_DIV_BY_8 = 0x03U,  /*!< Divide by 8   */
    eTMR_QD_CLOCK_DIV_BY_16 = 0x04U, /*!< Divide by 16  */
    eTMR_QD_CLOCK_DIV_BY_32 = 0x05U, /*!< Divide by 32  */
    eTMR_QD_CLOCK_DIV_BY_64 = 0x06U, /*!< Divide by 64  */
    eTMR_QD_CLOCK_DIV_BY_128 = 0x07U /*!< Divide by 128 */
} etmr_qd_clock_prs_t;

/*!
 * @brief eTMR CHMASK register sync selection
 *
 */
typedef enum
{
    CHMASK_SYNC_WITH_REG = 0U,  /*!< Synchronized with register loading */
    CHMASK_SYNC_WITH_MOD = 1U,  /*!< Synchronized with counter matching MOD */
#if defined(FEATURE_eTMR_HAS_MID) && (FEATURE_eTMR_HAS_MID == 1)
    CHMASK_SYNC_WITH_MID = 2U,  /*!< Synchronized with counter matching MID */
    CHMASK_SYNC_WITH_TRIG = 3U, /*!< Synchronized with trigger source 2 */
#else
    CHMASK_SYNC_WITH_TRIG = 2U, /*!< Synchronized with trigger source 2 */
    CHMASK_SYNC_DISABLED = 3U,  /*!< Synchronized disabled */
#endif
} etmr_mask_sync_sel_t;

/*!
 * @brief eTMR counter sync selection
 *
 */
typedef enum
{
    CNT_SYNC_WITH_REG = 0,  /*!< Synchronized with register loading */
#if defined(FEATURE_eTMR_HAS_MID) && (FEATURE_eTMR_HAS_MID == 1)
    CNT_SYNC_WITH_MID = 1,  /*!< Synchronized with counter matching MID */
    CNT_SYNC_WITH_TRIG = 2, /*!< Synchronized with trigger source 1 */
#else
    CNT_SYNC_WITH_TRIG = 1, /*!< Synchronized with trigger source 1 */
#endif
    CNT_SYNC_DISABLED = 3,  /*!< Synchronized disabled */
} etmr_cnt_sync_sel_t;

/*!
 * @brief eTMR register sync selection
 *
 */
typedef enum
{
    REG_SYNC_DISABLED = 0,  /*!< Synchronized disabled */
    REG_SYNC_WITH_MOD = 1,  /*!< Synchronized with counter matching MOD */
#if defined(FEATURE_eTMR_HAS_MID) && (FEATURE_eTMR_HAS_MID == 1)
    REG_SYNC_WITH_MID = 2,  /*!< Synchronized with counter matching MID */
    REG_SYNC_WITH_TRIG = 3, /*!< Synchronized with trigger source 0 */
#else
    REG_SYNC_WITH_TRIG = 2, /*!< Synchronized with trigger source 0 */
#endif
} etmr_reg_sync_sel_t;

/*!
 * @brief eTMR register loading trigger source
 *
 */
typedef enum
{
    DISABLE_TRIGGER = 0U,   /*!< Disable trigger, with register loading or matching with MOD/MID */
    SW_TRIGGER = 1U,        /*!< Select software trigger to load */
    HW_TRIGGER = 2U,        /*!< Select hardware trigger to load */
    SW_AND_HW_TRIGGER = 3U, /*!< Select software and hardware trigger to load */
} etmr_sync_trig_src_t;

typedef enum
{
    eTMR_CHANNEL_0 = 0x00U, /*!< channel 0 */
    eTMR_CHANNEL_1 = 0x01U, /*!< channel 1 */
    eTMR_CHANNEL_2 = 0x02U, /*!< channel 2 */
    eTMR_CHANNEL_3 = 0x03U, /*!< channel 3 */
    eTMR_CHANNEL_4 = 0x04U, /*!< channel 4 */
    eTMR_CHANNEL_5 = 0x05U, /*!< channel 5 */
    eTMR_CHANNEL_6 = 0x06U, /*!< channel 6 */
    eTMR_CHANNEL_7 = 0x07U, /*!< channel 7 */
} etmr_channel_index_t;

typedef enum
{
    TRIGGER_FROM_MATCHING_EVENT = 0U,
    TRIGGER_FROM_PWM = 1U,
} etmr_trig_src_t;

/*!
 * @brief List of eTMR interrupts
 *
 */
typedef enum
{
    eTMR_CHANNEL0_INT_ENABLE = 0x00000001U,       /*!< Channel 0 interrupt */
    eTMR_CHANNEL1_INT_ENABLE = 0x00000002U,       /*!< Channel 1 interrupt */
    eTMR_CHANNEL2_INT_ENABLE = 0x00000004U,       /*!< Channel 2 interrupt */
    eTMR_CHANNEL3_INT_ENABLE = 0x00000008U,       /*!< Channel 3 interrupt */
    eTMR_CHANNEL4_INT_ENABLE = 0x00000010U,       /*!< Channel 4 interrupt */
    eTMR_CHANNEL5_INT_ENABLE = 0x00000020U,       /*!< Channel 5 interrupt */
    eTMR_CHANNEL6_INT_ENABLE = 0x00000040U,       /*!< Channel 6 interrupt */
    eTMR_CHANNEL7_INT_ENABLE = 0x00000080U,       /*!< Channel 7 interrupt */
    eTMR_FAULT_INT_ENABLE = 0x00000100U,          /*!< Fault interrupt */
    eTMR_TIME_OVER_FLOW_INT_ENABLE = 0x00000200U, /*!< Time overflow interrupt */
} etmr_interrupt_option_t;

/*!
 * @brief List of eTMR flags
 *
 */
typedef enum
{
    eTMR_CHANNEL0_FLAG = 0x00000001U,       /*!< Channel 0 Flag */
    eTMR_CHANNEL1_FLAG = 0x00000002U,       /*!< Channel 1 Flag */
    eTMR_CHANNEL2_FLAG = 0x00000004U,       /*!< Channel 2 Flag */
    eTMR_CHANNEL3_FLAG = 0x00000008U,       /*!< Channel 3 Flag */
    eTMR_CHANNEL4_FLAG = 0x00000010U,       /*!< Channel 4 Flag */
    eTMR_CHANNEL5_FLAG = 0x00000020U,       /*!< Channel 5 Flag */
    eTMR_CHANNEL6_FLAG = 0x00000040U,       /*!< Channel 6 Flag */
    eTMR_CHANNEL7_FLAG = 0x00000080U,       /*!< Channel 7 Flag */
    eTMR_FAULT0_FLAG = 0x00000100U,         /*!< Fault Flag */
    eTMR_FAULT1_FLAG = 0x00000200U,         /*!< Fault Flag */
    eTMR_FAULT2_FLAG = 0x00000400U,         /*!< Fault Flag */
    eTMR_FAULT3_FLAG = 0x00000800U,         /*!< Fault Flag */
    eTMR_TIME_OVER_FLOW_FLAG = 0x00001000U, /*!< Time overflow Flag */
    eTMR_RELOAD_FLAG = 0x00002000U,         /*!< Reload Flag; Available only on certain SoC's */
} etmr_status_flag_t;

#if FEATURE_eTMR_HAS_CNT_INIT_SRC
/*!
 * @brief eTMR Initial counter value source
 *
 */
typedef enum
{
    COUNTER_VAL_FROM_CNT = 0U,  /*!< Counter initial value from CNT register */
    COUNTER_VAL_FROM_INIT = 1U, /*!< Counter initial value from INIT register */
} etmr_counter_init_src_t;
#endif

/*!
 * @brief eTMR channel val0 and val1 trigger enablement
 *
 */
typedef struct
{
    uint8_t channelId;           /*!< channel id */
    bool channelVal0MatchTrigEn; /*!< channel val0 match trigger enablement */
    bool channelVal1MatchTrigEn; /*!< channel val1 match trigger enablement */
} etmr_trig_ch_param_t;

/*!
 * @brief eTMR trigger configurations
 *
 */
typedef struct
{
    etmr_trig_src_t trigSrc;                      /*!< trigger source */
    uint8_t pwmOutputChannel;                     /*!< pwm output channel as trigger source */
    uint8_t outputTrigWidth;                      /*!< output trigger width */
    uint8_t outputTrigFreq;                       /*!< output trigger per x times */
    bool modMatchTrigEnable;                      /*!< mod match trigger enablement */
    bool midMatchTrigEnable;                      /*!< mid match trigger enablement */
    bool initMatchTrigEnable;                     /*!< init match trigger enablement */
    uint8_t numOfChannels;                        /*!< number of channels */
    etmr_trig_ch_param_t *channelTrigParamConfig; /*!< trigger channel configurations */
} etmr_trig_config_t;

/*!
 * @brief eTMR Registers sync configurations
 *
 */
typedef struct
{
    uint8_t regSyncFreq;                        /*!< register loading frequency */
    etmr_reg_sync_sel_t regSyncSel;             /*!< register loading opportunity */
    etmr_cnt_sync_sel_t cntInitSyncSel;         /*!< counter initial loading opportunity */
    etmr_mask_sync_sel_t maskOutputSyncSel;     /*!< mask output loading opportunity */
    etmr_sync_trig_src_t regSyncTrigSrc;        /*!< register loading trigger source selection */
    etmr_sync_trig_src_t cntInitSyncTrigSrc;    /*!< counter initial loading trigger source selection */
    etmr_sync_trig_src_t maskOutputSyncTrigSrc; /*!< mask output loading trigger source selection */
    bool hwTrigFromTmuEnable;                   /*!< hardware trigger from TMU enable or not */
    bool hwTrigFromCimEnable;                   /*!< hardware trigger from CIM enable or not */
    bool hwTrigFromPadEnable;                   /*!< hardware trigger from pad enable or not */
} etmr_pwm_sync_t;

/*!
 * @brief Configuration structure that the user needs to set
 *
 */
typedef struct
{
    etmr_clock_source_t etmrClockSource;  /*!< Select clock source for eTMR */
    uint8_t etmrPrescaler;                /*!< eTMR clock prescaler */
    bool debugMode;                       /*!< true  -- debug mode enabled, counter will stop in debug mode
                                           *   false -- debug mode disabled, counter will continue to run in debug mode */
    etmr_pwm_sync_t *syncMethod;          /*!< etmr sync method */
    etmr_trig_config_t *outputTrigConfig; /*!< etmr output trigger configure */
    bool isTofIntEnabled;                 /*!< true: enable interrupt, false: disable interrupt */
} etmr_user_config_t;

/*!
 * @brief eTMR channel mode state
 *
 */
typedef enum
{
    eTMR_STATE_NO_MODE = 0U,           /*!< No mode */
    eTMR_STATE_PWM_INDEPENDENT = 1U,   /*!< PWM independent mode */
    eTMR_STATE_PWM_COMPLEMENTARY = 2U, /*!< PWM complementary mode */
    eTMR_STATE_OUTPUT_COMPARE = 3U,    /*!< Output compare mode */
    eTMR_STATE_INPUT_CAPTURE = 4U,     /*!< Input Capture mode */
} etmr_channel_state_t;

/*!
 * @brief eTMR input capture edge mode as rising edge or falling edge
 *
 */
typedef enum
{
    eTMR_NOT_SEL_EDGE = 0x00U, /*!< No select edge */
    eTMR_POS_EDGE = 0x01U,     /*!< Positive edge */
    eTMR_NEG_EDGE = 0x02U,     /*!< Negative edge */
    eTMR_DUAL_EDGES = 0x03U    /*!< Dual edges */
} etmr_ic_capture_edge_t;

/*!
 * @brief eTMR input capture measurement type
 *
 */
typedef enum
{
    eTMR_POS_PULSE_MEASUREMENT = 0x00U, /*!< Positive pulse measurement */
    eTMR_NEG_PULSE_MEASUREMENT = 0x01U, /*!< Negative pulse measurement */
    eTMR_PERIOD_MEASUREMENT = 0x02U,    /*!< Period measurement */
} etmr_ic_measurement_type_t;

/*!
 * @brief eTMR state structure of the driver
 *
 */
typedef struct
{
    etmr_clock_source_t etmrClockSource;                                   /*!< Clock source used by eTMR counter */
    uint32_t etmrSourceClockFrequency;                                     /*!< The clock frequency is used for counting */
    etmr_channel_state_t etmrChnMode[FEATURE_eTMR_CHANNEL_MAX_COUNT];      /*!< Mode of operation for eTMR */
    uint32_t etmrPeriod;                                                   /*!< This field is used only in PWM mode to store signal period */
    uint32_t etmrModValue;                                                 /*!< This field is used only in input capture mode to store MOD value */
    etmr_pwm_update_option_t typeOfUpdate[FEATURE_eTMR_CHANNEL_MAX_COUNT]; /*!< Type of update for PWM update */
    etmr_pwm_align_mode_t alignMode[FEATURE_eTMR_CHANNEL_MAX_COUNT];       /*!< Align mode of PWM update */
    uint32_t dutyCycle[FEATURE_eTMR_CHANNEL_MAX_COUNT];                    /*!< Duty cycle of PWM update */
    uint32_t offset[FEATURE_eTMR_CHANNEL_MAX_COUNT];                       /*!< Offset of pulse for asymmetric mode */
    etmr_ic_measurement_type_t measurementType[FEATURE_eTMR_CHANNEL_MAX_COUNT]; /*!< Measurement type */
    etmr_ic_capture_edge_t captureEdge[FEATURE_eTMR_CHANNEL_MAX_COUNT];         /*!< Input capture edge */
    uint8_t icOvfCnt[FEATURE_eTMR_CHANNEL_MAX_COUNT][CAPTURE_COUNT];       /*!< eTMR channel capture overflow count */
    uint8_t icOvfCntTemp[FEATURE_eTMR_CHANNEL_MAX_COUNT];                  /*!< eTMR overflow count */
    uint8_t icCnt[FEATURE_eTMR_CHANNEL_MAX_COUNT];                         /*!< Count of enter into input capture interrupt */
    uint8_t icCapPol[FEATURE_eTMR_CHANNEL_MAX_COUNT][CAPTURE_COUNT];       /*!< eTMR channel capture polarity */
    uint32_t icCntVal[FEATURE_eTMR_CHANNEL_MAX_COUNT][CAPTURE_COUNT];      /*!< eTMR channel capture value */
    uint32_t measurementPeriod[FEATURE_eTMR_CHANNEL_MAX_COUNT];            /*!< Store period */
    uint32_t measurementPosPulseCnt[FEATURE_eTMR_CHANNEL_MAX_COUNT];       /*!< Positive pulse count value */
    uint32_t measurementNegPulseCnt[FEATURE_eTMR_CHANNEL_MAX_COUNT];       /*!< Negative pulse count value */
    uint32_t measurementCapCnt[FEATURE_eTMR_CHANNEL_MAX_COUNT];            /*!< Store capture value */
    bool measurementComplete[FEATURE_eTMR_CHANNEL_MAX_COUNT];              /*!< eTMR input capture measurement complete signal */
    void *channelsCallbacksParams[FEATURE_eTMR_CHANNEL_MAX_COUNT];         /*!< The parameters of callback function */
    ic_callback_t channelsCallbacks[FEATURE_eTMR_CHANNEL_MAX_COUNT];       /*!< The callback function */
    bool enableNotification[FEATURE_eTMR_CHANNEL_MAX_COUNT];               /*!< The notification on the callback application */
} etmr_state_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*! @brief Pointer to runtime state structure. */
extern etmr_state_t *etmrStatePtr[eTMR_INSTANCE_COUNT];

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @name Initialization & De-initialization
 * @{
 */

/*!
 * @brief Disable eTMR counter
 *
 * @param[in] instance  The eTMR peripheral instance number.
 */
void eTMR_DRV_Disable(uint32_t instance);

/*!
 * @brief Enable eTMR counter
 *
 * @param[in] instance  The eTMR peripheral instance number.
 */
void eTMR_DRV_Enable(uint32_t instance);

/*!
 * @brief Initialize the eTMR driver.
 *
 * Configures clock source, prescaler, debug mode, synchronization,
 * output trigger, and overflow interrupt based on the user config.
 *
 * @param[in]  instance The eTMR peripheral instance number.
 * @param[in]  info     Pointer to the user configuration structure.
 * @param[out] state    Pointer to the driver state structure.
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_Init(uint32_t instance, const etmr_user_config_t *info, etmr_state_t *state);

/*!
 * @brief Shut down the eTMR driver.
 *
 * Resets all registers to default, clears the driver state, and
 * disables the eTMR counter.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_Deinit(uint32_t instance);

/*!
 * @brief Populate a user configuration structure with default values.
 *
 * Fills the structure with safe defaults: internal clock, prescaler 0,
 * no sync, no output trigger, TOF interrupt disabled.
 *
 * @param[out] config Pointer to the structure to be filled.
 */
void eTMR_DRV_GetDefaultConfig(etmr_user_config_t *const config);

/*!
 * @brief Reset all eTMR registers to their default values.
 *
 * @param[in] instance The eTMR peripheral instance number.
 */
void eTMR_DRV_Reset(uint32_t instance);

/*! @} */

/*!
 * @name Counter Control
 * @{
 */

/*!
 * @brief Set eTMR global time base feature.
 *
 * @param[in] instance  The eTMR peripheral instance number.
 * @param[in] enable    true or false
 */
void eTMR_DRV_SetGlobalTimeBase(uint32_t instance, bool enable);

/*!
 * @brief Generate a trigger signal to enable global time base feature.
 *
 * @param[in] instance  The eTMR peripheral instance number.
 * @param[in] enable    true or false
 */
void eTMR_DRV_GenGlobalSignal(uint32_t instance, bool enable);

/*!
 * @brief Retrieve the frequency of the clock source feeding the counter.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return Clock frequency in Hz (0 if counter is disabled).
 */
uint32_t eTMR_DRV_GetFrequency(uint32_t instance);

/*!
 * @brief Convert a frequency in Hz to a period value in timer ticks.
 *
 * @param[in] instance   The eTMR peripheral instance number.
 * @param[in] freqencyHz Frequency value in Hz.
 * @return Period value in ticks.
 */
uint32_t eTMR_DRV_ConvertFreqToPeriodTicks(uint32_t instance, uint32_t freqencyHz);

/*!
 * @brief Get the current eTMR counter value.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return Current counter value.
 */
uint32_t eTMR_DRV_GetCntVal(uint32_t instance);

/*!
 * @brief Set the counter initial value (INIT register).
 *
 * @param[in] instance       The eTMR peripheral instance number.
 * @param[in] counterValue   Initial counter value.
 * @param[in] softwareTrigger If true, generate a software trigger to update.
 * @return STATUS_SUCCESS always.
 */
status_t eTMR_DRV_SetCounterInit(uint32_t instance, uint32_t counterValue, bool softwareTrigger);

/*!
 * @brief Set the counter modulus value (MOD register).
 *
 * @param[in] instance       The eTMR peripheral instance number.
 * @param[in] counterValue   Maximum counter value.
 * @param[in] softwareTrigger If true, generate a software trigger to update.
 * @return STATUS_SUCCESS always.
 */
status_t eTMR_DRV_SetCounterMod(uint32_t instance, uint32_t counterValue, bool softwareTrigger);

#if FEATURE_eTMR_HAS_MID
/*!
 * @brief Set the counter middle value (MID register).
 *
 * @param[in] instance     The eTMR peripheral instance number.
 * @param[in] counterValue Middle counter value.
 * @param[in] softwareTrigger If true, generate a software trigger to update.
 * @return STATUS_SUCCESS always.
 */
status_t eTMR_DRV_SetCounterMid(uint32_t instance, uint32_t counterValue, bool softwareTrigger);

/*!
 * @brief Get the counter middle value.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return Middle counter value.
 */
uint32_t eTMR_DRV_GetMid(uint32_t instance);
#endif

/*! @} */

/*!
 * @name Synchronization
 * @{
 */

/*!
 * @brief Set LDOK to start register synchronization.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return STATUS_SUCCESS always.
 */
status_t eTMR_DRV_SetLdok(uint32_t instance);

/*!
 * @brief Clear LDOK bit.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return STATUS_SUCCESS always.
 */
status_t eTMR_DRV_ClearLdok(uint32_t instance);

/*!
 * @brief Configure synchronization for buffered registers.
 *
 * Sets up loading opportunities, trigger sources, and hardware trigger
 * enables for MOD, MID, INIT, CHMASK, CHxVAL0, and CHxVAL1.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] param    Pointer to sync configuration.
 * @return Operation status.
 * @retval STATUS_SUCCESS Completed successfully.
 * @retval STATUS_ERROR   Error occurred.
 */
status_t eTMR_DRV_SetSync(uint32_t instance, const etmr_pwm_sync_t *param);

/*! @} */

/*!
 * @name Channel Output Mask & Safe State
 * @{
 */

/*!
 * @brief Mask channel outputs.
 *
 * @param[in] instance       The eTMR peripheral instance number.
 * @param[in] maskEn         Channel mask enable bitmask.
 * @param[in] maskVal        Channel mask values.
 * @param[in] softwareTrigger If true, generate a software trigger.
 * @return STATUS_SUCCESS always.
 */
status_t eTMR_DRV_SetChnOutMask(uint32_t instance, uint8_t maskEn, uint16_t maskVal, bool softwareTrigger);

/*!
 * @brief Set channel safe-state output values on fault.
 *
 * @param[in] instance  The eTMR peripheral instance number.
 * @param[in] safeState Combined safe-state value for all channels.
 * @return Operation status.
 */
status_t eTMR_DRV_SetSafeState(uint32_t instance, uint32_t safeState);

/*! @} */

/*!
 * @name Output Trigger
 * @{
 */

/*!
 * @brief Configure the output trigger.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] param    Pointer to trigger configuration.
 * @return Operation status.
 */
status_t eTMR_DRV_SetOutputTrigger(uint32_t instance, const etmr_trig_config_t *param);

/*! @} */

/*!
 * @name Interrupt Management
 * @{
 */

/*!
 * @brief Enable a set of interrupts by bitmask.
 *
 * @param[in] instance      The eTMR peripheral instance number.
 * @param[in] interruptMask Logical OR of ::etmr_interrupt_option_t values.
 * @return STATUS_SUCCESS always.
 */
status_t eTMR_DRV_EnableInterrupts(uint32_t instance, uint32_t interruptMask);

/*!
 * @brief Disable a set of interrupts by bitmask.
 *
 * @param[in] instance      The eTMR peripheral instance number.
 * @param[in] interruptMask Logical OR of ::etmr_interrupt_option_t values.
 */
void eTMR_DRV_DisableInterrupts(uint32_t instance, uint32_t interruptMask);

/*!
 * @brief Get the currently enabled interrupt mask.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return Logical OR of ::etmr_interrupt_option_t values.
 */
uint32_t eTMR_DRV_GetEnabledInterrupts(uint32_t instance);

/*! @} */

/*!
 * @name Status Flags
 * @{
 */

/*!
 * @brief Get status flags for all sources.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return Logical OR of ::etmr_status_flag_t values.
 */
uint32_t eTMR_DRV_GetStatusFlags(uint32_t instance);

/*!
 * @brief Clear all status flags.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return STATUS_SUCCESS always.
 */
status_t eTMR_DRV_ClearAllStatusFlag(uint32_t instance);
/*!
 * @brief Get channel negative pulse ready flag.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel index.
 * @return true if negative pulse ready, false otherwise.
 */
bool eTMR_DRV_GetChnNegPulseRdyFlag(uint32_t instance, uint8_t channel);

/*!
 * @brief Get channel positive pulse ready flag.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel index.
 * @return true if positive pulse ready, false otherwise.
 */
bool eTMR_DRV_GetChnPosPulseRdyFlag(uint32_t instance, uint8_t channel);

/*!
 * @brief Get quadrature decoder counting direction.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return true if counting up, false if counting down.
 */
bool eTMR_DRV_GetQuadDecodeDir(uint32_t instance);

/*!
 * @brief Get quadrature decoder counter overflow flag.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return true if overflow occurred.
 */
bool eTMR_DRV_GetQuadDecodeTofFlag(uint32_t instance);

/*!
 * @brief Get counter overflow flag.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return true if overflow occurred.
 */
bool eTMR_DRV_GetTofFlag(uint32_t instance);

/*!
 * @brief Get reload flag.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return true if reload event occurred.
 */
bool eTMR_DRV_GetReloadFlag(uint32_t instance);

/*!
 * @brief Get fault flag for a specific fault channel.
 *
 * @param[in] instance   The eTMR peripheral instance number.
 * @param[in] fltChannel The fault channel index.
 * @return true if fault occurred.
 */
bool eTMR_DRV_GetFaultFlag(uint32_t instance, uint8_t fltChannel);

/*!
 * @brief Get channel interrupt flag.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel index.
 * @return true if the channel flag is set.
 */
bool eTMR_DRV_GetChnFlag(uint32_t instance, uint8_t channel);

/*!
 * @brief Clear channel negative pulse ready flag.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel index.
 */
void eTMR_DRV_ClearChnNegPulseRdyFlag(uint32_t instance, uint8_t channel);

/*!
 * @brief Clear channel positive pulse ready flag.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel index.
 */
void eTMR_DRV_ClearChnPosPulseRdyFlag(uint32_t instance, uint8_t channel);

/*!
 * @brief Clear quadrature decoder counter overflow flag.
 *
 * @param[in] instance The eTMR peripheral instance number.
 */
void eTMR_DRV_ClearQuadDecodeTofFlag(uint32_t instance);

/*!
 * @brief Clear counter overflow flag.
 *
 * @param[in] instance The eTMR peripheral instance number.
 */
void eTMR_DRV_ClearTofFlag(uint32_t instance);

/*!
 * @brief Clear counter reload flag.
 *
 * @param[in] instance The eTMR peripheral instance number.
 */
void eTMR_DRV_ClearReloadFlag(uint32_t instance);

/*!
 * @brief Clear fault channel flag.
 *
 * @param[in] instance   The eTMR peripheral instance number.
 * @param[in] fltChannel The fault channel index.
 */
void eTMR_DRV_ClearFaultFlag(uint32_t instance, uint8_t fltChannel);

/*!
 * @brief Clear channel interrupt flag.
 *
 * @param[in] instance   The eTMR peripheral instance number.
 * @param[in] channelSet The eTMR channel set bitmask.
 */
void eTMR_DRV_ClearChnFlag(uint32_t instance, uint8_t channelSet);

/*! @} */

/*!
 * @name IO Status
 * @{
 */
/*!
 * @brief Get phase B input status.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return Phase B input level (0 or 1).
 */
uint8_t eTMR_DRV_GetPhaseBStatus(uint32_t instance);

/*!
 * @brief Get phase A input status.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @return Phase A input level (0 or 1).
 */
uint8_t eTMR_DRV_GetPhaseAStatus(uint32_t instance);

/*!
 * @brief Get fault channel input status (polarity).
 *
 * @param[in] instance   The eTMR peripheral instance number.
 * @param[in] fltChannel The fault channel index.
 * @return Fault input status.
 */
uint8_t eTMR_DRV_GetFaultInputStatus(uint32_t instance, uint8_t fltChannel);

/*!
 * @brief Get channel IO status.
 *
 * @param[in] instance The eTMR peripheral instance number.
 * @param[in] channel  The eTMR channel index.
 * @return Channel IO level.
 */
uint8_t eTMR_DRV_GetChnIoStatus(uint32_t instance, uint8_t channel);

/*! @} */

/*!
 * @name Channel Mode Control
 * @{
 */

/*!
 * @brief Set complementary mode for channels
 *
 * @param[in] instance    the eTMR peripheral instance
 * @param[in] channelPair the eTMR channel pair
 * @param[in] mode        pwm mode, see #etmr_pwm_mode_t struct
 */
void eTMR_DRV_SetChnCompMode(uint32_t instance, uint8_t channelPair, etmr_pwm_mode_t mode);

#if FEATURE_eTMR_HAS_DOUBLE_SWITCH
/*!
 * @brief Set double switch for channels
 *
 * @param[in] instance    the eTMR peripheral instance
 * @param[in] channelPair the eTMR channel pair
 * @param[in] en          enable/disable double switch
 */
void eTMR_DRV_SetChnDoubleSwitch(uint32_t instance, uint8_t channelPair, bool en);
#endif

#if FEATURE_eTMR_HAS_COMBINATION_CAPTURE
/*!
 * @brief Set combine mode for channels
 *
 * @param[in] instance    the eTMR peripheral instance
 * @param[in] channelPair the eTMR channel pair
 * @param[in] en          enable/disable combine mode
 */
void eTMR_DRV_SetChnCombMode(uint32_t instance, uint8_t channelPair, bool en);

/*!
 * @brief Set channel combine source
 *
 * @param[in] instance    the eTMR peripheral instance
 * @param[in] channelPair the eTMR channel pair
 * @param[in] src         the combine source
 */
void eTMR_DRV_SetChnCombSrc(uint32_t instance, uint8_t channelPair, uint8_t src);
#endif

/*! @} */

/*!
 * @name Fault Configuration
 * @{
 */

/*!
 * @brief Set channel fault input polarity
 *
 * @param[in] instance the eTMR peripheral instance
 * @param[in] channel the eTMR channel
 * @param[in] pol the fault input polarity
 */
void eTMR_DRV_SetFaultChnPol(uint32_t instance, uint8_t fltChannel, uint32_t pol);

/*!
 * @brief Set channel fault interrupt enable or disable
 *
 * @param[in] instance the eTMR peripheral instance
 * @param[in] channel the eTMR channel
 */
void eTMR_DRV_SetFaultChnEnable(uint32_t instance, uint8_t fltChannel, bool enable);

/*! @} */

/*!
 * @name Interrupt Enable/Disable (Fine-grained)
 * @{
 */

/*!
 * @brief Enable quadrature decoder counter overflow interrupt
 *
 * @param[in] instance the eTMR peripheral instance
 */
void eTMR_DRV_EnableQuadDecodeTofInt(uint32_t instance);

/*!
 * @brief Disable quadrature decoder counter overflow interrupt
 *
 * @param[in] instance the eTMR peripheral instance
 */
void eTMR_DRV_DisableQuadDecodeTofInt(uint32_t instance);

/*!
 * @brief Enable eTMR overflow interrupt
 *
 * @param[in] instance the eTMR peripheral instance
 */
void eTMR_DRV_EnableTofInt(uint32_t instance);

/*!
 * @brief Disable eTMR overflow interrupt
 *
 * @param[in] instance the eTMR peripheral instance
 */
void eTMR_DRV_DisableTofInt(uint32_t instance);

/*!
 * @brief Enable specified channel interrupt.
 *
 * @param[in] instance the eTMR peripheral instance
 * @param[in] channel the eTMR channel
 */
void eTMR_DRV_EnableChnInt(uint32_t instance, uint8_t channel);

/*!
 * @brief Enable channel set interrupt.
 *
 * @param[in] instance the eTMR peripheral instance
 * @param[in] channelSet the eTMR channel set (1 << channel0 | 1 << channeln)
 */
void eTMR_DRV_SetChnsetIntEnable(uint32_t instance, uint8_t channelSet);

/*!
 * @brief Disable specified channel interrupt.
 *
 * @param[in] instance the eTMR peripheral instance
 * @param[in] channel the eTMR channel
 */
void eTMR_DRV_DisableChnInt(uint32_t instance, uint8_t channel);

/*!
 * @brief Disable channel set interrupt.
 *
 * @param[in] instance the eTMR peripheral instance
 * @param[in] channelSet the eTMR channel set (1 << channel0 | 1 << channeln)
 */
void eTMR_DRV_SetChnsetIntDisable(uint32_t instance, uint8_t channelSet);

/*!
 * @brief Get if counter overflow interrupt is enabled or not
 *
 * @param[in] instance the eTMR peripheral instance
 */
bool eTMR_DRV_IsTofIntEnabled(uint32_t instance);

/*!
 * @brief Get if quadrature decoder overflow interrupt is enabled or not
 *
 * @param[in] instance the eTMR peripheral instance
 *
 * @return    true/false
 */
bool eTMR_DRV_IsQuadDecodeTofIntEnabled(uint32_t instance);

/*!
 * @brief Get if fault interrupt is enabled or not
 *
 * @param[in] instance the eTMR peripheral instance
 *
 * @return    true/false
 */
bool eTMR_DRV_IsFaultIntEnabled(uint32_t instance);

/*!
 * @brief Get if the channel interrupt is enabled or not.
 *
 * @param[in] instance the eTMR peripheral instance
 * @param[in] channel the eTMR channel that needs to get
 *
 * @return    true/false
 */
bool eTMR_DRV_IsChnIntEnabled(uint32_t instance, uint8_t channel);

/*! @} */

/*!
 * @brief Get eTMR current counter value.
 *
 * @param[in] instance the eTMR peripheral instance
 *
 * @return    current counter value
 */
uint32_t eTMR_DRV_GetCntVal(uint32_t instance);

#if defined(__cplusplus)
}
#endif

#endif /* eTMR_COMMON_H */

/*! @}*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
