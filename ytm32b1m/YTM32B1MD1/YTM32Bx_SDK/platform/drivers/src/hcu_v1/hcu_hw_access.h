/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file hcu_hw_access.h
 * @version 1.4.1
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
    return (((HCU->SR >> (uint32_t)statusFlag) & 1U) != 0U);
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
    return (((HCU->INTE >> (uint32_t) statusFlag) & 1U) != 0U);
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
        HCU->INTE |= HCU_INTE_IFWMIE_MASK;
    } else
    {
        HCU->INTE &= (~HCU_INTE_IFWMIE_MASK);
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
        HCU->INTE |= HCU_INTE_OFWMIE_MASK;
    } else
    {
        HCU->INTE &= (~HCU_INTE_OFWMIE_MASK);
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
        HCU->INTE |= HCU_INTE_ODIE_MASK | HCU_INTE_IFWMIE_MASK | HCU_INTE_OFWMIE_MASK;
    } else
    {
        HCU->INTE &= ~(HCU_INTE_ODIE_MASK | HCU_INTE_IFWMIE_MASK | HCU_INTE_OFWMIE_MASK);
    }
}

/*!
 * @brief Reset HCU input and output FIFOs.
 */
static inline void HCU_ResetFifo(void)
{
    /* Reset FIFO */
    HCU->CR |= (HCU_CR_OFSWR_MASK | HCU_CR_IFSWR_MASK);
    /* De-assert FIFO reset */
    HCU->CR &= ~(HCU_CR_OFSWR_MASK | HCU_CR_IFSWR_MASK);
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
    /* Reset FIFO */
    HCU_ResetFifo();
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
        HCU->AESIV[i] = iv[i];
    }
}

/*!
 * @brief Config HCU IV.
 */
static inline void HCU_SetCV(const uint32_t *cv)
{
#ifdef HCU_AESCV_CNT_MASK
    uint8_t i;
    for (i = 0; i < 4U; i++)
    {
        HCU->AESCV[i] = cv[i];
    }
#else
    (void) cv;
#endif
}

/*!
 * @brief Config HCU AESMAC.
 */
static inline void HCU_SetMac(const uint8_t *mac, uint8_t length)
{
    uint32_t word;
    for(uint8_t i = 0u; i < 4u; i++)
    {
        word = 0U;
        for(uint8_t j = 0u; j < 4u; j++)
        {
            if(((i * 4u) + j) < length)
            {
                word |= (uint32_t)mac[(i * 4u) + j] << (j * 8u);
            }
        }
        HCU->AESMAC[i] = word;
    }
}

/*!
 * @brief Readout HCU AESMAC.
 */
static inline void HCU_ReadMac(uint8_t *mac)
{
    for(uint8_t i = 0u; i < 4u; i++)
    {
        for(uint8_t j = 0u; j < 4u; j++)
        {
            mac[(i * 4u) + j] = (uint8_t)(HCU->AESMAC[i] >> (j * 8u)) & 0xFFU;
        }
    }
}

/*!
 * @brief Config HCU SHAICV.
 */
#if FEATURE_HCU_HAS_SHA_AUTHORIZE
static inline void HCU_SetICV(const uint32_t *icv, uint8_t length)
{
    uint8_t i;
#ifdef CPU_YTM32B1HA0
    for (i = 0; i < length; i++)
    {
        HCU->SHAICV[i] = icv[i];
    }
#else
    for (i = 0; i < 4u; i++)
    {
        HCU->AESCV[i] = icv[i];
    }

    for (i = 0; i < 4u; i++)
    {
        HCU->AESIV[i] = icv[i+4u];
    }

    if (length == 0xCu)
    {
        for (i = 0; i < 4u; i++)
        {
            HCU->AESMAC[i] = icv[i+8u];
        }
    }
#endif /* CPU_YTM32B1HA0 */
}
#endif

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
    HCU->MSGADL = HCU_MSGADL_ADLEN(addMsgLen) | HCU_MSGADL_MSGLEN(MsgLen);
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
    (void) MsgTotalLen;
#endif
}

/*!
 * @brief Config HCU FIFO watermark.
 */
static inline void HCU_SetFIFOWatermark(uint8_t in, uint8_t out)
{
    HCU->FIFOWM &= ~(HCU_FIFOWM_IFWM_MASK | HCU_FIFOWM_OFWM_MASK);
    HCU->FIFOWM |= HCU_FIFOWM_IFWM(in) | HCU_FIFOWM_OFWM(out);
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
 * @brief Returns true if HCU need to write in fifo data.
 */
static inline bool HCU_IsInputFifoEmpty(void)
{
    return ((HCU->SR & HCU_SR_IFWMF_MASK) != 0U);
}

/*!
 * @brief Returns true if HCU need to read out fifo data.
 */
static inline bool HCU_IsOutputFifoFull(void)
{
    return ((HCU->SR & HCU_SR_OFWMF_MASK) != 0U);
}

/*!
 * @brief Returns true if HCU is not empty.
 */
static inline bool HCU_OutputFifoHasData(void)
{
    return ((HCU->SR & HCU_SR_OFES_MASK) == 0U);
}

/*!
 * @brief HCU load data to input fifo.
 */
static inline void HCU_WriteInputFifo(const uint32_t *data, uint8_t length)
{
    uint8_t i;
    for (i = 0; i < length; i++)
    {
        HCU->IFDAT = data[i];
    }
}

/*!
 * @brief HCU write empty data to avoid entering interrupt.
 */
static inline void HCU_WriteInputFifoPatch(uint8_t length)
{
    uint8_t i;
    for (i = 0; i < (length + 1u); i++)
    {
        HCU->IFDAT = 0;
    }
}

/*!
 * @brief HCU read data from output fifo.
 */
static inline void HCU_ReadOutputFifo(uint32_t *data, uint8_t length)
{
    uint8_t i;
    for (i = 0; i < length; i++)
    {
        data[i] = HCU->OFDAT;
    }
}

/*!
 * @brief HCU set input fifo dma.
 */
static inline void HCU_SetInputDMA(bool enable)
{
    if (true == enable)
    {
        HCU->CR |= HCU_CR_IFDMAE_MASK;
    }
    else
    {
        HCU->CR &= ~HCU_CR_IFDMAE_MASK;
    }
}

/*!
 * @brief HCU set output fifo dma.
 */
static inline void HCU_SetOutputDMA(bool enable)
{
    if (true == enable)
    {
        HCU->CR |= HCU_CR_OFDMAE_MASK;
    }
    else
    {
        HCU->CR &= ~HCU_CR_OFDMAE_MASK;
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

#if defined(FEATURE_HCU_HMAC_ENGINE) && (FEATURE_HCU_HMAC_ENGINE > 0)
/*!
 * @brief Enable HMAC.
 */
static inline void HCU_EnableHMAC(bool enable)
{
    if(enable)
    {
        HCU->CR |= HCU_CR_HMACEN_MASK;
    }else {
        HCU->CR &= ~HCU_CR_HMACEN_MASK;
    }
}

/*!
 * @brief Set HCU HMAC key size.
 */
static inline void HCU_SetHMACKeySize(hcu_hmac_key_size_t keySize)
{
    HCU->CR &= ~HCU_CR_HMACKS_MASK;
    HCU->CR |= keySize << HCU_CR_HMACKS_SHIFT;
}
#endif /* FEATURE_HCU_HMAC_ENGINE */

#if defined(__cplusplus)
}
#endif

#endif /* HCU_HW_ACCESS_H */

/*******************************************************************************
 * EOF
 ******************************************************************************/
