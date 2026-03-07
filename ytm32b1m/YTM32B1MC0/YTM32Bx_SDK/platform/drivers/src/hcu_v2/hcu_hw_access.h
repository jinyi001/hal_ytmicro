/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*!
 * @file hcu_hw_access.h
 * @version 1.4.0
 */

#ifndef HCU_HW_ACCESS_H
#define HCU_HW_ACCESS_H

#include "hcu_driver.h"
#include "device_registers.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Macro used to enable/disable HCU interrupt request */
#define HCU_IRQ_NUMBER            HCU_IRQn

/*! @brief Macro used for checking buffer length */
#define HCU_BUFF_LEN_CHECK_MASK   0x0FU

/*! @brief Macro used for checking buffer address */
#define HCU_BUFF_ADDR_CHECK_MASK   0x03U

/*! @brief Macro used to convert buffer length in bytes to number of 128-bits blocks */
#define HCU_BUFF_BLOCK_COUNT(x)   ((x) >> 4U)

/*! @brief Macro used for setting output length */
#define HCU_SHA_256_LENGTH        0x20U

/*! @brief Macro used for setting output length */
#define HCU_SHA_384_LENGTH        0x30U

/*! @brief Macro used for setting SHA-256 block size */
#define HCU_SHA_256_BLOCK_SIZE    0x40U

/*! @brief Macro used for setting SHA-384 block size */
#define HCU_SHA_384_BLOCK_SIZE    0x80U

/*! @brief HCU engine selection */
typedef enum
{
    ENG_AES = 1U,
    ENG_SM4 = 2U,
    ENG_SHA = 3U
} hcu_engine_sel_t;
/*! @brief HCU algorithm selection */
typedef enum
{
    ALG_AES_ECB = 0U,
    ALG_AES_CBC = 1U,
    ALG_AES_CTR = 2U,
    ALG_AES_CCM = 3U,
    ALG_AES_CMAC = 4U,
    ALG_AES_GCM = 5U
} hcu_alg_aes_mode_t;
/*! @brief HCU status flag */
typedef enum
{
    OPERATION_DONE_FLAG = 0U,
    AES_MAC_VALID_FLAG = 1U,
    SHA_HASH_INVALID_FLAG = 2U,
    OUTPUT_FIFO_EMPTY_FLAG = 8U,
    OUTPUT_FIFO_FULL_FLAG = 9U,
    INPUT_FIFO_EMPTY_FLAG = 10U,
    INPUT_FIFO_FULL_FLAG = 11U,
    OUTPUT_FIFO_OVERFLOW_FLAG = 12U,    
    INPUT_FIFO_OVERFLOW_FLAG = 13U,    
    OUTPUT_FIFO_WATERMARK_FLAG = 14U,
    INPUT_FIFO_WATERMARK_FLAG = 15U,
    SHA_CTX_READY_FLAG = 20U,
    SHA_CTX_WAIT_FLAG = 21U,
    SHA_ICV_WAIT_FLAG = 22U,
    AES_IV_READY_FLAG = 23U,
    AES_CV_READY_FLAG = 24U,
    AES_CV_WAIT_FLAG = 25U,
    AES_MAC_WAIT_FLAG = 26U,
    AES_MAC_READY_FLAG = 27U,
    AES_IV_WAIT_FLAG = 28U,
    KEY_WAIT_FLAG = 29U,
    OUTPUT_MESSAGE_READY_FLAG = 30U,
    INPUT_MESSAGE_WAIT_FLAG = 31U
} hcu_status_flag_t;

/*******************************************************************************
 * CODE
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Get the HCU status flag.
 *
 * @param[in] statusFlag The status flag, of type hcu_status_flag_t
 * @return State of the status flag: asserted (true) or not-asserted (false)
 */
static inline bool HCU_GetStatusFlag(hcu_status_flag_t statusFlag)
{
    return ((((HCU->SR) >> (uint8_t) statusFlag) & 1U) != 0U);
}

/*!
 * @brief Clear the HCU status flag.
 *
 * @param[in] statusFlag The status flag, of type hcu_status_flag_t
 */
static inline void HCU_ClearStatusFlag(hcu_status_flag_t statusFlag)
{
    HCU->SR = (uint32_t) 1 << (uint8_t) statusFlag;
}

/*!
 * @brief Get the HCU interrupt enable.
 *
 * @param[in] statusFlag The status flag, of type hcu_status_flag_t
 * @return State of the status flag: asserted (true) or not-asserted (false)
 */
static inline bool HCU_GetIntMode(hcu_status_flag_t statusFlag)
{
    return ((((HCU->INTE) >> (uint8_t) statusFlag) & 1U) != 0U);
}

/*!
 * @brief Set the HCU interrupt enable.
 *
 * @param[in] statusFlag The status flag, of type hcu_status_flag_t
 */
static inline void HCU_SetIntMode(hcu_status_flag_t statusFlag, bool enable)
{
    if (true == enable)
    {
        HCU->INTE |= (uint32_t) 1 << (uint8_t) statusFlag;
    } else
    {
        HCU->INTE &= ~((uint32_t) 1U << (uint8_t) statusFlag);
    }
}

/*!
 * @brief Enables/Disables HCU done interrupt.
 *
 * @param[in] enable Enables/Disables HCU done interrupt.
 */
static inline void HCU_SetDoneInterrupt(bool enable)
{
    if (enable)
    {
        HCU->INTE |= HCU_INTE_ODIE_MASK;
    } else
    {
        HCU->INTE &= (~HCU_INTE_ODIE_MASK);
    }
}

/*!
 * @brief Enables/Disables HCU data input interrupt.
 *
 * @param[in] enable Enables/Disables HCU input interrupt.
 */
static inline void HCU_SetInputInterrupt(bool enable)
{
    if (enable)
    {
        HCU->INTE |= HCU_INTE_INMSGWIE_MASK;
    } else
    {
        HCU->INTE &= (~HCU_INTE_INMSGWIE_MASK);
    }
}

/*!
 * @brief Enables/Disables HCU data output interrupt.
 *
 * @param[in] enable Enables/Disables HCU output interrupt.
 */
static inline void HCU_SetOutputInterrupt(bool enable)
{
    if (enable)
    {
        HCU->INTE |= HCU_INTE_OUTMSGRIE_MASK;
    } else
    {
        HCU->INTE &= (~HCU_INTE_OUTMSGRIE_MASK);
    }
}

/*!
 * @brief Enables/Disables HCU default interrupt.
 *
 * @param[in] enable Enables/Disables HCU default interrupt.
 */
static inline void HCU_SetDefaultInterrupt(bool enable)
{
    if (enable)
    {
        HCU->INTE |= HCU_INTE_ODIE_MASK | HCU_INTE_INMSGWIE_MASK | HCU_INTE_OUTMSGRIE_MASK;
    } else
    {
        HCU->INTE &= ~(HCU_INTE_ODIE_MASK | HCU_INTE_INMSGWIE_MASK | HCU_INTE_OUTMSGRIE_MASK);
    }
}

/*!
 * @brief Config HCU engine and algorithm.
 */

static inline void HCU_SetEngineAlgorithm(hcu_engine_sel_t engine, hcu_alg_aes_mode_t algorithm, hcu_mode_sel_t encrypt)
{
    /* Reset register fields */
    HCU->CR &= ~(HCU_CR_ENGSEL_MASK | HCU_CR_ALGSEL_MASK | HCU_CR_ENC_MASK);
    /* Set register fields */
    HCU->CR |= HCU_CR_ENGSEL(engine) | HCU_CR_ALGSEL(algorithm) | HCU_CR_ENC(encrypt);
    /* Default enable store context data */
    HCU->CR |= HCU_CR_CS_MASK;
}

/*!
 * @brief Config HCU key.
 */
static inline void HCU_SetUserKey(uint32_t key, uint8_t index)
{
    HCU->KEY[index] = key;
}

/*!
 * @brief Config HCU IV.
 */
static inline void HCU_SetIV(const uint32_t *iv)
{
    uint8_t i;
    for (i = 0; i < 4U; i++)
    {
        HCU->CONTEXT_4_7[i] = iv[i];
    }
}

/*!
 * @brief Config HCU IV.
 */
static inline void HCU_SetMAC(const uint8_t *mac, uint8_t length)
{
    uint32_t word;
    for (uint8_t i = 0; i < 4U; i++)
    {
        word = 0u;
        for(uint8_t j = 0u; j < 4u; j++)
        {
            if(((i * 4u) + j) < length)
            {
                word |= (uint32_t)mac[(i * 4u) + j] << (j * 8u);
            }
        }
        HCU->CONTEXT_8_11[i] = word;
    }
}

/*!
 * @brief Config HCU IV.
 */
static inline void HCU_SetCV(const uint32_t *cv)
{
#if FEATURE_HCU_AES_CTR_ENGINE
    uint8_t i;
    for (i = 0; i < 4U; i++)
    {
        HCU->CONTEXT_0_3[i] = cv[i];
    }
#else
    (void) cv;
#endif
}

/*!
 * @brief Config HCU key size.
 */
static inline void HCU_SetKeySize(hcu_key_size_t size)
{
#ifdef HCU_CR_KEYLEN_MASK
    HCU->CR &= ~(HCU_CR_KEYLEN_MASK);
    HCU->CR |= HCU_CR_KEYLEN(size);
#else
    (void) size;
#endif
}

/*!
 * @brief Config HCU input data size.
 */
static inline void HCU_SetMsgLength(uint16_t MsgLen, uint16_t addMsgLen)
{
#ifdef HCU_MSGCUR_ADLEN_MASK
    HCU->MSGCUR = HCU_MSGCUR_ADLEN(addMsgLen) | HCU_MSGCUR_MSGLEN(MsgLen);
#else
    (void)addMsgLen;
    HCU->MSGCUR = HCU_MSGCUR_MSGLEN(MsgLen);
#endif /* HCU_MSGCUR_ADLEN_MASK */
}

/*!
 * @brief Config HCU CMAC length
 */
static inline void HCU_SetMacLength(uint8_t length)
{
    HCU->CR &= ~(HCU_CR_MACLEN_MASK);
    HCU->CR |= HCU_CR_MACLEN(length);
}

/*!
 * @brief Config HCU Message type
 */
static inline void HCU_SetMsgType(hcu_msg_type_t type)
{
    HCU->CR &= ~(HCU_CR_MSGE_MASK | HCU_CR_MSGB_MASK);
    HCU->CR |= (uint32_t)type << HCU_CR_MSGE_SHIFT;
}

/*!
 * @brief Config HCU message total bytes.
 */
static inline void HCU_SetMsgTotalLength(uint32_t MsgTotalLen)
{
#ifdef HCU_MSGINFO_MSGTOT_MASK
    HCU->MSGINFO = HCU_MSGINFO_MSGTOT(MsgTotalLen);
#else
#ifdef HCU_MSGTOT_MSGTOT_MASK
    HCU->MSGTOT = HCU_MSGTOT_MSGTOT(MsgTotalLen);
#else
    (void) MsgTotalLen;
#endif
#endif
}

/*!
 * @brief HCU start engine.
 */

static inline void HCU_StartEngine(void)
{
    // write HCU->GO is valid
    HCU->GO = HCU_GO_GO_MASK;
}

/*!
 * @brief Returns true if HCU is busy processing a command
 * and false if the command has completed.
 */
static inline bool HCU_IsBusy(void)
{
    return ((HCU->GO & HCU_GO_GO_MASK) != 0U);
}

/*!
 * @brief Returns true if HCU job finished current operation.
 */
static inline bool HCU_IsDone(void)
{
    return ((HCU->SR & HCU_SR_OD_MASK) != 0U);
}

/*!
 * @brief HCU set input fifo dma.
 */
static inline void HCU_SetInputDMA(bool enable)
{
    if (true == enable)
    {
        HCU->CR |= HCU_CR_INDMAE_MASK;
    }
    else
    {
        HCU->CR &= ~HCU_CR_INDMAE_MASK;
    }
}

/*!
 * @brief HCU set output fifo dma.
 */
static inline void HCU_SetOutputDMA(bool enable)
{
    if (true == enable)
    {
        HCU->CR |= HCU_CR_OUTDMAE_MASK;
    }
    else
    {
        HCU->CR &= ~HCU_CR_OUTDMAE_MASK;
    }
}

/*!
 * @brief HCU set SHA verification.
 */
static inline void HCU_SetSHAVerification(bool enable)
{
#ifdef HCU_CR_SHAVE_MASK
    if (true == enable)
    {
        HCU->CR |= HCU_CR_SHAVE_MASK;
    }
    else
    {
        HCU->CR &= ~HCU_CR_SHAVE_MASK;
    }
#else
    (void) enable;
#endif
}

/*!
 * @brief Reset HCU by APB.
 */
static inline void HCU_APBReset(void)
{
    IPC->CTRL[IPC_HCU_INDEX] |=  IPC_CTRL_SWREN_MASK;
    IPC->CTRL[IPC_HCU_INDEX] &= ~IPC_CTRL_SWREN_MASK;
}

/*!
 * @brief Get input message wait
 */
static inline bool HCU_GetInputMsgWait(void)
{
    return ((HCU->SR & HCU_SR_INMSGW_MASK) != 0U);
}

/*!
 * @brief Get output message ready
 */
static inline bool HCU_GetOutputMsgReady(void)
{
    return ((HCU->SR & HCU_SR_OUTMSGR_MASK) != 0U);
}

/*!
 * @brief Get key wait
 */
static inline bool HCU_GetKeyWait(void)
{
    return ((HCU->SR & HCU_SR_KEYW_MASK) != 0U);
}

/*!
 * @brief Get AES Initial Vector wait
 */
static inline bool HCU_GetIVWait(void)
{
    return ((HCU->SR & HCU_SR_AESIVW_MASK) != 0U);
}

/*!
 * @brief Get AES Initial Vector ready
 */
static inline bool HCU_GetIVReady(void)
{
    return ((HCU->SR & HCU_SR_AESIVR_MASK) != 0U);
}

/*!
 * @brief Get AES Counter Value wait
 */
static inline bool HCU_GetCVWait(void)
{
#if FEATURE_HCU_AES_CTR_ENGINE
    return ((HCU->SR & HCU_SR_AESCVW_MASK) != 0U);
#else
    return false;
#endif /* FEATURE_HCU_AES_CTR_ENGINE */
}

/*!
 * @brief Get AES Counter Value ready
 */
static inline bool HCU_GetCVReady(void)
{
#if FEATURE_HCU_AES_CTR_ENGINE
    return ((HCU->SR & HCU_SR_AESCVR_MASK) != 0U);
#else
    return false;
#endif /* FEATURE_HCU_AES_CTR_ENGINE */
}

/*!
 * @brief Get AES MAC wait
 */
static inline bool HCU_GetMacWait(void)
{
    return ((HCU->SR & HCU_SR_AESMACW_MASK) != 0U);
}

/*!
 * @brief Get AES MAC ready
 */
static inline bool HCU_GetMacReady(void)
{
    return ((HCU->SR & HCU_SR_AESMACR_MASK) != 0U);
}

#if FEATURE_HCU_SHA_ENGINE
/*!
 * @brief Get SHA ICV wait
 */
static inline bool HCU_GetShaIcvWait(void)
{
    return ((HCU->SR & HCU_SR_SHAICVW_MASK) != 0U);
}

/*!
 * @brief Get SHA context data wait
 */
static inline bool HCU_GetShaCtxWait(void)
{
    return ((HCU->SR & HCU_SR_SHACTXW_MASK) != 0U);
}

/*!
 * @brief Get SHA context data ready
 */
static inline bool HCU_GetShaCtxReady(void)
{
    return ((HCU->SR & HCU_SR_SHACTXR_MASK) != 0U);
}

/*!
 * @brief Set SHA context data ready interrupt
 */
static inline void HCU_SetShaCtxReadyInt(bool enable)
{
    if(enable)
    {
        HCU->INTE |= HCU_INTE_SHACTXRIE_MASK;
    } else {
        HCU->INTE &= ~HCU_INTE_SHACTXRIE_MASK;
    }
}

/*!
 * @brief Set SHA ICV Wait interrupt
 */
static inline void HCU_SetShaIcvWaitInt(bool enable)
{
    if(enable)
    {
        HCU->INTE |= HCU_INTE_SHAICVWIE_MASK;
    } else {
        HCU->INTE &= ~HCU_INTE_SHAICVWIE_MASK;
    }
}

/*!
 * @brief Write SHA ICV data
 */
static inline void HCU_WriteShaIcvData(const uint32_t * icv)
{
    for(uint8_t i = 0; i < 8U; i ++)
    {
        HCU->CONTEXT_12_19[i] = icv[i];
    }
}

/*!
 * @brief Write SHA context data
 */
static inline void HCU_WriteShaCtxData(const uint32_t * data)
{
    for(uint8_t i = 0; i < 8U; i ++)
    {
        HCU->CONTEXT_28_35[i] = data[i];
    }
}

/*!
 * @brief Read SHA context data
 */
static inline void HCU_ReadShaCtxData(uint32_t * data)
{
    for(uint8_t i = 0; i < 8U; i ++)
    {
        data[i] = HCU->CONTEXT_28_35[i];
    }
}

#endif /* FEATURE_HCU_SHA_ENGINE */

/*!
 * @brief Write input message
 */
static inline void HCU_WriteInputMsg(const uint32_t * data, uint8_t length)
{
    for(uint8_t i = 0; i < length; i++)
    {
        HCU->INMSG = data[i];
    }
}

/*!
 * @brief Readout output message
 */
static inline void HCU_ReadOutputMsg(uint32_t * data, uint8_t length)
{
    for(uint8_t i = 0; i < length; i++)
    {
        data[i] = HCU->OUTMSG;
    }
}

/*!
 * @brief Readout IV
 */
static inline void HCU_ReadIV(uint32_t * iv)
{
    for(uint8_t i = 0; i < 4U; i++)
    {
        iv[i] = HCU->CONTEXT_4_7[i];
    }
}

/*!
 * @brief Readout MAC
 */
static inline void HCU_ReadMAC(uint32_t * mac)
{
    for(uint8_t i = 0; i < 4U; i++)
    {
        mac[i] = HCU->CONTEXT_8_11[i];
    }
}

/*!
 * @brief Readout CV
 */
static inline void HCU_ReadCV(uint32_t * cv)
{
#if FEATURE_HCU_AES_CTR_ENGINE
    for(uint8_t i = 0; i < 4U; i++)
    {
        cv[i] = HCU->CONTEXT_0_3[i];
    }
#else
    (void) cv;
#endif /* FEATURE_HCU_AES_CTR_ENGINE */
}

#if defined(__cplusplus)
}
#endif

#endif /* HCU_HW_ACCESS_H */

/*******************************************************************************
 * EOF
 ******************************************************************************/
