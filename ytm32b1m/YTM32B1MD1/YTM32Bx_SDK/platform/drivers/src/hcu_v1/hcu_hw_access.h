/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file hcu_hw_access.h
 * @version 1.4.1
 *
 * @brief HCU V1 Hardware Access Layer.
 *
 * This header provides low-level inline helpers for the HCU V1 registers. The
 * driver implementation uses these helpers to control status flags, interrupt
 * enables, engine selection, message framing, FIFO movement, DMA toggles, and
 * optional HMAC support.
 *
 * The helpers are organized into the following categories:
 *   - Status & Interrupt Control
 *   - Engine & Context Configuration
 *   - FIFO Runtime Access
 *   - DMA, Verification & Reset Control
 *   - Optional HMAC Control
 *
 * @note This is an internal layer used by `hcu_driver.c`. Application code
 *       should use the public `HCU_DRV_*` APIs from `hcu_driver.h`.
 */

#ifndef HCU_HW_ACCESS_H
#define HCU_HW_ACCESS_H

#include "hcu_driver.h"
#include "device_registers.h"

/*!
 * @defgroup hcu_hw_access_v1 HCU V1 Hardware Access
 * @ingroup hcu_v1
 * @brief Low-level register helpers for the HCU V1 peripheral.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Interrupt line routed to the HCU driver. */
#define HCU_IRQ_NUMBER            HCU_IRQn

/*! @brief Mask used to validate 16-byte aligned payload lengths. */
#define HCU_BUFF_LEN_CHECK_MASK   0x0FU

/*! @brief Mask used to validate 32-bit aligned buffer addresses. */
#define HCU_BUFF_ADDR_CHECK_MASK   0x03U

/*! @brief Convert a byte count into the corresponding 128-bit block count. */
#define HCU_BUFF_BLOCK_COUNT(x)   ((x) >> 4U)

/*! @brief SHA-256 digest length in bytes. */
#define HCU_SHA_256_LENGTH        0x20U

/*! @brief SHA-384 digest length in bytes. */
#define HCU_SHA_384_LENGTH        0x30U

/*! @brief SHA-256 message-block size in bytes. */
#define HCU_SHA_256_BLOCK_SIZE    0x40U

/*! @brief SHA-384 message-block size in bytes. */
#define HCU_SHA_384_BLOCK_SIZE    0x80U

/*!
 * @brief HCU engine selector.
 *
 * Identifies which hardware sub-engine is programmed into the HCU control
 * register.
 */
typedef enum
{
    ENG_AES = 1U, /*!< AES engine selection. */
    ENG_SM4 = 2U, /*!< SM4 engine selection. */
    ENG_SHA = 3U  /*!< SHA engine selection. */
} hcu_engine_sel_t;

/*!
 * @brief AES/SM4 algorithm selector written into the hardware control field.
 */
typedef enum
{
    ALG_AES_ECB = 0U,  /*!< ECB mode selection. */
    ALG_AES_CBC = 1U,  /*!< CBC mode selection. */
    ALG_AES_CTR = 2U,  /*!< CTR mode selection. */
    ALG_AES_CCM = 3U,  /*!< CCM mode selection. */
    ALG_AES_CMAC = 4U, /*!< CMAC mode selection. */
    ALG_AES_GCM = 5U   /*!< GCM mode selection. */
} hcu_alg_aes_mode_t;

/*!
 * @brief HCU status and interrupt flag positions.
 */
typedef enum
{
    OPERATION_DONE_FLAG = 0U,      /*!< Operation-done flag position. */
    AES_MAC_VALID_FLAG = 1U,       /*!< AES MAC valid flag position. */
    SHA_HASH_INVALID_FLAG = 2U,    /*!< SHA authorize mismatch flag position. */
    OUTPUT_FIFO_EMPTY_FLAG = 8U,   /*!< Output FIFO empty flag position. */
    OUTPUT_FIFO_FULL_FLAG = 9U,    /*!< Output FIFO full flag position. */
    INPUT_FIFO_EMPTY_FLAG = 10U,   /*!< Input FIFO empty flag position. */
    INPUT_FIFO_FULL_FLAG = 11U,    /*!< Input FIFO full flag position. */
    OUTPUT_FIFO_OVERFLOW_FLAG = 12U, /*!< Output FIFO overflow flag position. */
    INPUT_FIFO_OVERFLOW_FLAG = 13U,  /*!< Input FIFO overflow flag position. */
    OUTPUT_FIFO_WATERMARK_FLAG = 14U, /*!< Output FIFO watermark flag position. */
    INPUT_FIFO_WATERMARK_FLAG = 15U,  /*!< Input FIFO watermark flag position. */
} hcu_status_flag_t;

/*******************************************************************************
 * CODE
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Status & Interrupt Control
 ******************************************************************************/
/*!
 * @name Status & Interrupt Control
 * @brief Helpers for reading HCU status flags and controlling interrupt enables.
 * @{
 */

/*!
 * @brief Read one HCU status flag.
 *
 * @param[in] statusFlag Status-bit selector.
 * @return True when the selected flag is asserted.
 */
static inline bool HCU_GetStatusFlag(hcu_status_flag_t statusFlag)
{
    return (((HCU->SR >> (uint32_t)statusFlag) & 1U) != 0U);
}

/*!
 * @brief Clear one HCU status flag.
 *
 * @param[in] statusFlag Status-bit selector written through the W1C path.
 */
static inline void HCU_ClearStatusFlag(hcu_status_flag_t statusFlag)
{
    HCU->SR = (uint32_t) 1 << (uint8_t) statusFlag;
}

/*!
 * @brief Read the enable state of one HCU interrupt source.
 *
 * @param[in] statusFlag Interrupt-bit selector.
 * @return True when the corresponding interrupt is enabled.
 */
static inline bool HCU_GetIntMode(hcu_status_flag_t statusFlag)
{
    return (((HCU->INTE >> (uint32_t) statusFlag) & 1U) != 0U);
}

/*!
 * @brief Enable or disable one HCU interrupt source.
 *
 * @param[in] statusFlag Interrupt-bit selector.
 * @param[in] enable True to enable the interrupt, false to disable it.
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
 * @brief Enable or disable the operation-done interrupt.
 *
 * @param[in] enable True to enable the interrupt, false to disable it.
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
 * @brief Enable or disable the input FIFO watermark interrupt.
 *
 * @param[in] enable True to enable the interrupt, false to disable it.
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
 * @brief Enable or disable the output FIFO watermark interrupt.
 *
 * @param[in] enable True to enable the interrupt, false to disable it.
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
 * @brief Enable or disable the default interrupt set used by the driver.
 *
 * Controls the operation-done, input-watermark, and output-watermark interrupt
 * enables as one combined helper.
 *
 * @param[in] enable True to enable the default interrupt set, false to disable it.
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

/*! @} */ /* End of Status & Interrupt Control */

/*******************************************************************************
 * Engine & Context Configuration
 ******************************************************************************/
/*!
 * @name Engine & Context Configuration
 * @brief Helpers for configuring engine selection, key material, message
 *        metadata, and authenticated-operation context.
 * @{
 */

/*!
 * @brief Reset both HCU FIFOs.
 */
static inline void HCU_ResetFifo(void)
{
    /* Assert both FIFO reset bits. */
    HCU->CR |= (HCU_CR_OFSWR_MASK | HCU_CR_IFSWR_MASK);
    /* Release both FIFO reset bits. */
    HCU->CR &= ~(HCU_CR_OFSWR_MASK | HCU_CR_IFSWR_MASK);
}

/*!
 * @brief Program the active engine, algorithm, and direction.
 */
static inline void HCU_SetEngineAlgorithm(hcu_engine_sel_t engine, hcu_alg_aes_mode_t algorithm, hcu_mode_sel_t encrypt)
{
    /* Clear the engine, algorithm, and direction fields before updating them. */
    HCU->CR &= ~(HCU_CR_ENGSEL_MASK | HCU_CR_ALGSEL_MASK | HCU_CR_ENC_MASK);
    /* Apply the caller-selected engine, algorithm, and direction. */
    HCU->CR |= HCU_CR_ENGSEL(engine) | HCU_CR_ALGSEL(algorithm) | HCU_CR_ENC(encrypt);
    /* Keep context-store support enabled for the next operation. */
    HCU->CR |= HCU_CR_CS_MASK;
    /* Start the new operation with empty FIFOs. */
    HCU_ResetFifo();
}

/*!
 * @brief Write one software-key word.
 *
 * @param[in] key Key word written into the indexed key register.
 * @param[in] index Key register index.
 */
static inline void HCU_SetUserKey(uint32_t key, uint8_t index)
{
    HCU->KEY[index] = key;
}

/*!
 * @brief Write the AES IV registers.
 *
 * @param[in] iv Pointer to four IV words.
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
 * @brief Write the AES counter-value registers.
 *
 * @param[in] cv Pointer to four counter-value words.
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
 * @brief Write the AES MAC registers from a byte buffer.
 *
 * @param[in] mac Pointer to the MAC byte buffer.
 * @param[in] length Number of MAC bytes to copy into the registers.
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
 * @brief Read the AES MAC registers into a byte buffer.
 *
 * @param[out] mac Pointer to the destination MAC byte buffer.
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
 * @brief Program the initial SHA authorize digest context.
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
#endif /* FEATURE_HCU_HAS_SHA_AUTHORIZE */

/*!
 * @brief Program the software-key size field.
 *
 * @param[in] size Key-size selector.
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
 * @brief Program the primary and additional message lengths.
 *
 * @param[in] MsgLen Main message length field.
 * @param[in] addMsgLen Additional-data length field.
 */
static inline void HCU_SetMsgLength(uint16_t MsgLen, uint16_t addMsgLen)
{
    HCU->MSGADL = HCU_MSGADL_ADLEN(addMsgLen) | HCU_MSGADL_MSGLEN(MsgLen);
}

/*!
 * @brief Program the MAC length field.
 *
 * @param[in] length MAC length encoding.
 */
static inline void HCU_SetMacLength(uint8_t length)
{
    HCU->CR &= ~(HCU_CR_MACLEN_MASK);
    HCU->CR |= HCU_CR_MACLEN(length);
}

/*!
 * @brief Program the message-fragment type field.
 *
 * @param[in] type Message-fragment selector.
 */
static inline void HCU_SetMsgType(hcu_msg_type_t type)
{
    HCU->CR &= ~(HCU_CR_MSGE_MASK | HCU_CR_MSGB_MASK);
    HCU->CR |= (uint32_t)type << HCU_CR_MSGE_SHIFT;
}

/*!
 * @brief Program the total-message-length field.
 *
 * @param[in] MsgTotalLen Total message length in bytes.
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
 * @brief Program the input and output FIFO watermark levels.
 *
 * @param[in] in Input FIFO watermark value.
 * @param[in] out Output FIFO watermark value.
 */
static inline void HCU_SetFIFOWatermark(uint8_t in, uint8_t out)
{
    HCU->FIFOWM &= ~(HCU_FIFOWM_IFWM_MASK | HCU_FIFOWM_OFWM_MASK);
    HCU->FIFOWM |= HCU_FIFOWM_IFWM(in) | HCU_FIFOWM_OFWM(out);
}

/*! @} */ /* End of Engine & Context Configuration */

/*******************************************************************************
 * FIFO Runtime Access
 ******************************************************************************/
/*!
 * @name FIFO Runtime Access
 * @brief Helpers for starting the engine, polling FIFO state, and moving FIFO
 *        data in software.
 * @{
 */

/*!
 * @brief Start the HCU engine.
 */
static inline void HCU_StartEngine(void)
{
    /* Writing GO launches the configured HCU operation. */
    HCU->GO = HCU_GO_GO_MASK;
}

/*!
 * @brief Check whether the HCU engine is still busy.
 *
 * @return True while the GO bit remains asserted.
 */
static inline bool HCU_IsBusy(void)
{
    return ((HCU->GO & HCU_GO_GO_MASK) != 0U);
}

/*!
 * @brief Check whether the current HCU operation is done.
 *
 * @return True when the operation-done status flag is asserted.
 */
static inline bool HCU_IsDone(void)
{
    return ((HCU->SR & HCU_SR_OD_MASK) != 0U);
}

/*!
 * @brief Check whether the input FIFO watermark requests more data.
 *
 * @return True when the input FIFO watermark flag is asserted.
 */
static inline bool HCU_IsInputFifoEmpty(void)
{
    return ((HCU->SR & HCU_SR_IFWMF_MASK) != 0U);
}

/*!
 * @brief Check whether the output FIFO watermark requests a read.
 *
 * @return True when the output FIFO watermark flag is asserted.
 */
static inline bool HCU_IsOutputFifoFull(void)
{
    return ((HCU->SR & HCU_SR_OFWMF_MASK) != 0U);
}

/*!
 * @brief Check whether the output FIFO still contains unread data.
 *
 * @return True when the output FIFO is not empty.
 */
static inline bool HCU_OutputFifoHasData(void)
{
    return ((HCU->SR & HCU_SR_OFES_MASK) == 0U);
}

/*!
 * @brief Push one software-managed burst into the input FIFO.
 *
 * @param[in] data Pointer to the source word buffer.
 * @param[in] length Number of 32-bit words to write.
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
 * @brief Push padding words into the input FIFO.
 *
 * This helper is used by the interrupt path on devices without fixed DMA to
 * avoid repeated re-entry after the real payload has been exhausted.
 *
 * @param[in] length Number of zero words to emit, minus the helper's final
 *                   extra word.
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
 * @brief Pull one software-managed burst from the output FIFO.
 *
 * @param[out] data Pointer to the destination word buffer.
 * @param[in] length Number of 32-bit words to read.
 */
static inline void HCU_ReadOutputFifo(uint32_t *data, uint8_t length)
{
    uint8_t i;
    for (i = 0; i < length; i++)
    {
        data[i] = HCU->OFDAT;
    }
}

/*! @} */ /* End of FIFO Runtime Access */

/*******************************************************************************
 * DMA, Verification & Reset Control
 ******************************************************************************/
/*!
 * @name DMA, Verification & Reset Control
 * @brief Helpers for DMA toggles, SHA verification control, and APB reset.
 * @{
 */

/*!
 * @brief Enable or disable input FIFO DMA service.
 *
 * @param[in] enable True to enable input DMA, false to disable it.
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
 * @brief Enable or disable output FIFO DMA service.
 *
 * @param[in] enable True to enable output DMA, false to disable it.
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
 * @brief Enable or disable SHA authorization mode.
 *
 * @param[in] enable True to enable SHA verification, false to disable it.
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
 * @brief Reset the HCU through the APB software-reset path.
 */
static inline void HCU_APBReset(void)
{
    IPC->CTRL[IPC_HCU_INDEX] |=  IPC_CTRL_SWREN_MASK;
    IPC->CTRL[IPC_HCU_INDEX] &= ~IPC_CTRL_SWREN_MASK;
}

/*! @} */ /* End of DMA, Verification & Reset Control */

#if defined(FEATURE_HCU_HMAC_ENGINE) && (FEATURE_HCU_HMAC_ENGINE > 0)
/*******************************************************************************
 * Optional HMAC Control
 ******************************************************************************/
/*!
 * @name Optional HMAC Control
 * @brief Helpers for the optional HMAC extension.
 * @{
 */

/*!
 * @brief Enable or disable the HMAC extension.
 *
 * @param[in] enable True to enable HMAC mode, false to disable it.
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
 * @brief Program the HMAC key-size encoding.
 *
 * @param[in] keySize HMAC key-size selector.
 */
static inline void HCU_SetHMACKeySize(hcu_hmac_key_size_t keySize)
{
    HCU->CR &= ~HCU_CR_HMACKS_MASK;
    HCU->CR |= keySize << HCU_CR_HMACKS_SHIFT;
}

/*! @} */ /* End of Optional HMAC Control */
#endif /* FEATURE_HCU_HMAC_ENGINE */

#if defined(__cplusplus)
}
#endif

/*! @} */ /* End of hcu_hw_access_v1 */

#endif /* HCU_HW_ACCESS_H */

/*******************************************************************************
 * EOF
 ******************************************************************************/
