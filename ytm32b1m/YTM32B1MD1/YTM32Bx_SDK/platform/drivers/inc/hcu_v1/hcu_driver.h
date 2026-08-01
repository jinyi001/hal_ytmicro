/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file hcu_driver.h
 * @version 1.4.1
 *
 * @brief HCU V1 Driver - Public API for the Hardware Cryptography Unit.
 *
 * This header defines the application-facing API for the HCU V1 security
 * engine. The driver manages runtime state, data-carry mode selection, user
 * key loading, and cryptographic processing flows for AES, optional SM4,
 * SHA, CMAC, CCM, and optional HMAC features.
 *
 * The APIs are organized into the following categories:
 *   - Initialization & Runtime Control
 *   - Callback & Key Management
 *   - Cipher Operations
 *   - MAC & Hash Operations
 *   - Authenticated Encryption
 *   - Command Status & Interrupt Handling
 *
 * @note Data-processing APIs document their alignment, block-size, and
 *       segmented-message requirements individually.
 */

#ifndef HCU_DRV_H
#define HCU_DRV_H

#include <stdint.h>
#include <stdbool.h>
#include "status.h"
#include "osif.h"
#include "dma_driver.h"
#include "callbacks.h"

/*!
 * @defgroup hcu_driver_v1 HCU V1 Driver
 * @ingroup hcu_v1
 * @brief Public driver APIs for the Hardware Cryptography Unit V1.
 * @details Provides runtime setup, key loading, cipher execution, MAC or hash
 *          orchestration, and asynchronous completion support for the HCU V1
 *          hardware block.
 * @{
 */

/*!
 * @brief User-key size selection.
 *
 * Selects the key length written through `HCU_DRV_LoadUserKey()`.
 *
 * | Value | Description |
 * |-------|-------------|
 * | `KEY_SIZE_128_BITS` | Load a 128-bit key. |
 * | `KEY_SIZE_192_BITS` | Load a 192-bit key. |
 * | `KEY_SIZE_256_BITS` | Load a 256-bit key. |
 */
typedef enum
{
    KEY_SIZE_128_BITS = 0x00U, /*!< Select a 128-bit key. */
    KEY_SIZE_192_BITS = 0x01U, /*!< Select a 192-bit key. */
    KEY_SIZE_256_BITS = 0x02U, /*!< Select a 256-bit key. */
} hcu_key_size_t;

/*!
 * @brief Message-fragment type for segmented operations.
 *
 * Identifies whether the current API call carries the first, middle, final, or
 * complete message fragment for CMAC, SHA, HMAC, or CCM processing.
 *
 * | Value | Description |
 * |-------|-------------|
 * | `MSG_START` | First fragment of a multi-call message. |
 * | `MSG_END` | Final fragment of a multi-call message. |
 * | `MSG_ALL` | Entire message provided in a single call. |
 * | `MSG_MIDDLE` | Intermediate fragment of a multi-call message. |
 */
typedef enum
{
    MSG_START = 0x02U,       /*!< Mark the first fragment of a segmented message. */
    MSG_END = 0x01U,         /*!< Mark the last fragment of a segmented message. */
    MSG_ALL = 0x03U,         /*!< Mark a complete message handled in one call. */
    MSG_MIDDLE = 0x00U,      /*!< Mark an intermediate fragment of a segmented message. */
} hcu_msg_type_t;

/*!
 * @brief SHA algorithm selector.
 *
 * Chooses which SHA engine profile is used by the SHA and optional HMAC APIs.
 */
typedef enum
{
    HCU_SHA_256 = 0x01U,       /*!< Select the SHA-256 profile. */
    HCU_SHA_384 = 0x02U,       /*!< Select the SHA-384 profile. */
} hcu_sha_type_t;

/*!
 * @brief HCU operation direction selector.
 *
 * Controls whether the active command runs in encrypt/generate mode or in
 * decrypt/authorize mode.
 */
typedef enum
{
    MODE_ENC = 1U,          /*!< Execute an encrypt or generate flow. */
    MODE_DEC = 0U           /*!< Execute a decrypt or authorize flow. */
} hcu_mode_sel_t;

/*!
 * @brief HCU data-swapping mode.
 *
 * Selects the byte or bit swapping applied by `HCU_DRV_CfgSwapping()`.
 */
typedef enum
{
    MODE_SWAPPING_NO        = 0U,          /*!< Keep the hardware data order unchanged. */
    MODE_SWAPPING_HALFWORD  = 1U,          /*!< Swap the two half-words in each word. */
    MODE_SWAPPING_BYTE      = 2U,          /*!< Swap the byte order in each word. */
    MODE_SWAPPING_BIT       = 3U,          /*!< Reverse the bit order in the data path. */
} hcu_swapping_t;

/*!
 * @brief Data-carry mode selection.
 *
 * Selects how the driver services the HCU FIFOs while a command is running.
 *
 * | Value | Description |
 * |-------|-------------|
 * | `HCU_USING_POLLING` | Service the FIFOs in the calling context until completion. |
 * | `HCU_USING_INTERRUPT` | Service the FIFOs from the HCU interrupt handler. |
 * | `HCU_USING_DMA` | Transfer FIFO data through DMA channels. |
 */
typedef enum
{
    HCU_USING_POLLING         = 0U,          /*!< Move data through polling loops. */
    HCU_USING_INTERRUPT       = 1U,          /*!< Move data through interrupt-driven FIFO service. */
    HCU_USING_DMA             = 2U,          /*!< Move data through DMA transfers. */
} hcu_carry_type_t;

/*!
 * @brief Driver-tracked algorithm identifier.
 *
 * This enumeration records which algorithm family is currently active in the
 * internal runtime state.
 */
typedef enum
{
    ALG_DISABLE   = 0x000U,             /*!< No command is currently associated with a tracked algorithm. */
    AES_ECB_MODE  = 0x001U,             /*!< AES ECB command context. */
    AES_CBC_MODE  = 0x002U,             /*!< AES CBC command context. */
    AES_CTR_MODE  = 0x003U,             /*!< AES CTR command context. */
    AES_CCM_MODE  = 0x004U,             /*!< AES CCM command context. */
    AES_CMAC_MODE = 0x005U,             /*!< AES CMAC command context. */
    SM4_ECB_MODE  = 0x010U,             /*!< SM4 ECB command context. */
    SHA_256_MODE  = 0x200U,             /*!< SHA-256 command context. */
    SHA_384_MODE  = 0x300U              /*!< SHA-384 command context. */
} hcu_alg_mode_t;

#if defined(FEATURE_HCU_HMAC_ENGINE) && (FEATURE_HCU_HMAC_ENGINE > 0)
/*!
 * @brief HMAC key-size encoding.
 *
 * Selects the key-size code written into the optional HMAC control field.
 */
typedef enum
{
    HMAC_KEY_16_BITS = 0x0U,            /*!< Select the HMAC key-size code named 16 bits. */
    HMAC_KEY_32_BITS = 0x1U,            /*!< Select the HMAC key-size code named 32 bits. */
    HMAC_KEY_64_BITS = 0x2U,            /*!< Select the HMAC key-size code named 64 bits. */
    HMAC_KEY_128_BITS = 0x3U,           /*!< Select the HMAC key-size code named 128 bits. */
    HMAC_KEY_256_BITS = 0x4U,           /*!< Select the HMAC key-size code named 256 bits. */
} hcu_hmac_key_size_t;
#endif /* FEATURE_HCU_HMAC_ENGINE */

/*!
 * @brief CCM context configuration.
 *
 * Holds the nonce, additional authenticated data, tag buffer, and message
 * length used by `HCU_DRV_CCMConfig()`.
 */
typedef struct
{
    uint8_t *nonce;         /*!< Nonce buffer used to build the CCM B0 and counter blocks. */
    uint8_t nonceSize;      /*!< Nonce length in bytes. */
    uint8_t *addData;       /*!< Additional authenticated data buffer. */
    uint8_t addDataSize;    /*!< Additional authenticated data length in bytes. */
    uint8_t *tag;           /*!< Tag buffer used for CCM output or verification input. */
    uint8_t tagSize;        /*!< Requested authentication tag length in bytes. */
    uint64_t msgLen;        /*!< Total plaintext or ciphertext length associated with the CCM context. */
} hcu_ccm_config_t;

/*!
 * @brief CMAC buffer configuration.
 *
 * Describes the MAC buffer used by CMAC generate and authorize flows.
 */
typedef struct
{
    uint8_t *macPtr;        /*!< MAC buffer used for CMAC output or authorize input. */
    uint8_t macLen;         /*!< Number of MAC bytes to read or compare. */
} hcu_cmac_config_t;

/*!
 * @brief User configuration for HCU V1 driver initialization.
 *
 * Selects the swap mode and FIFO carry path used after `HCU_DRV_Init()`.
 */
typedef struct
{
    hcu_swapping_t swap;           /*!< Data swap mode programmed into the HCU control register. */
    hcu_carry_type_t carryType;    /*!< FIFO carry mode used while a command is running. */
    uint8_t ingressDMAChannel;     /*!< DMA channel used for input FIFO transfers. */
    uint8_t egressDMAChannel;      /*!< DMA channel used for output FIFO transfers. */
} hcu_user_config_t;

/*!
 * @brief Internal driver state information.
 *
 * The driver uses this structure to track the active command, remaining FIFO
 * work, callback information, and authenticated-operation context.
 *
 * @note This structure is owned by the driver after `HCU_DRV_Init()` and must
 *       not be modified by application code while the driver is active.
 */
typedef struct
{
    volatile bool cmdInProgress;   /*!< True while the driver is servicing an active command. */
    bool isLastBlock;              /*!< True when the current segment closes the message stream. */
    bool blockingCmd;              /*!< True when the command is tracked as blocking by the driver. */
    security_callback_t callback;  /*!< Completion callback invoked by asynchronous paths. */
    void *callbackParam;           /*!< User parameter passed back through the completion callback. */
    semaphore_t cmdComplete;       /*!< Synchronization object created during initialization. */
    uint32_t const *dataInputPtr;  /*!< Current input FIFO source pointer. */
    uint32_t *dataOutputPtr;       /*!< Current output FIFO destination pointer. */
    uint64_t msgLen;               /*!< Current payload length programmed for the command. */
    uint64_t inputCount;           /*!< Remaining input bytes still waiting to enter the FIFO path. */
    uint64_t outputCount;          /*!< Remaining output bytes still waiting to be drained. */
    uint8_t ingressDMAChannel;     /*!< DMA channel reserved for input FIFO transfers. */
    uint8_t egressDMAChannel;      /*!< DMA channel reserved for output FIFO transfers. */
    hcu_ccm_config_t *ccmConfig;   /*!< Active CCM context, when CCM has been configured. */
    hcu_cmac_config_t *cmacConfig; /*!< Active CMAC buffer configuration. */
    status_t status;               /*!< Last completion status reported by the driver. */
    hcu_mode_sel_t mode;           /*!< Current encrypt/generate or decrypt/authorize direction. */
    hcu_carry_type_t carryType;    /*!< Active FIFO carry mode. */
    hcu_alg_mode_t algorithm;      /*!< Internal identifier for the currently active algorithm. */
} hcu_state_t;


/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Initialization & Runtime Control
 ******************************************************************************/
/*!
 * @name Initialization & Runtime Control
 * @brief Functions for installing the driver state, restoring runtime defaults,
 *        and controlling top-level HCU status bits.
 * @{
 */

/*!
 * @brief Initialize the HCU V1 driver state.
 *
 * Installs the caller-provided runtime state structure, clears its tracked
 * command context, copies the carry-mode configuration, applies the selected
 * swap mode, and creates the completion semaphore.
 *
 * @param[in] userConfig Pointer to the user configuration structure. Must not
 *                       be NULL.
 * @param[in] state Pointer to the runtime state structure that remains owned by
 *                  the driver after initialization. Must not be NULL.
 * @return Execution status.
 * @retval STATUS_SUCCESS Driver initialization completed successfully.
 * @retval STATUS_ERROR Semaphore creation failed.
 *
 * @post The driver is ready to accept key-loading and processing requests.
 */
status_t HCU_DRV_Init(const hcu_user_config_t * userConfig, hcu_state_t *state);

/*!
 * @brief De-initialize the HCU V1 driver state.
 *
 * Clears the tracked runtime state, disables the HCU interrupt path, resets the
 * main control and interrupt-enable registers, and destroys the semaphore owned
 * by @a state.
 *
 * @param[in] state Pointer to the runtime state structure previously passed to
 *                  `HCU_DRV_Init()`. Must not be NULL.
 * @return Execution status reported by the OSIF semaphore destroy helper.
 *
 * @post The global driver state pointer is cleared.
 */
status_t HCU_DRV_DeInit(hcu_state_t *state);

/*!
 * @brief Program the HCU data-swapping mode.
 *
 * @param[in] cfg Swap mode written into the HCU data-swap field.
 *
 * @note The value must be one of the `hcu_swapping_t` enumerators.
 */
void HCU_DRV_CfgSwapping(hcu_swapping_t cfg);

/*!
 * @brief Clear the operation-done status flag.
 *
 * Writes the HCU operation-done bit using the status register W1C mechanism.
 */
void HCU_DRV_ClearODFlag(void);

/*! @} */ /* End of Initialization & Runtime Control */

/*******************************************************************************
 * Callback & Key Management
 ******************************************************************************/
/*!
 * @name Callback & Key Management
 * @brief Functions for installing callbacks and loading software keys.
 * @{
 */

/*!
 * @brief Load a software key into the HCU key registers.
 *
 * @param[in] key Pointer to the key buffer. The address must be 32-bit aligned.
 * @param[in] keySize Key-size selector written to the HCU key-size field.
 * @return Execution status.
 * @retval STATUS_SUCCESS Key load completed successfully.
 * @retval STATUS_HCU_KEY_SIZE_NOT_SUPPORTED The requested key size is not handled.
 * @retval STATUS_HCU_LOAD_KEY_WHEN_BUSY A command is already in progress.
 *
 * @warning Do not call this function while the driver is servicing another
 *          command.
 */
status_t HCU_DRV_LoadUserKey(const void *key, hcu_key_size_t keySize);

/*!
 * @brief Install a completion callback for asynchronous command paths.
 *
 * @param[in] callbackFunction Callback invoked when an asynchronous command
 *                             completes. Pass NULL to clear the callback.
 * @param[in] callbackParam User parameter passed back to the callback.
 * @return Previously installed callback pointer.
 */
security_callback_t HCU_DRV_InstallCallback(security_callback_t callbackFunction, void *callbackParam);

/*! @} */ /* End of Callback & Key Management */

/*******************************************************************************
 * Cipher Operations
 ******************************************************************************/
/*!
 * @name Cipher Operations
 * @brief AES and optional SM4 block-cipher processing helpers.
 *
 * Unless stated otherwise, the input and output buffers used by these APIs
 * must be 32-bit aligned and the payload length must be a multiple of 16 bytes.
 * @{
 */

/*!
 * @brief Encrypt a payload with AES in ECB mode.
 *
 * @param[in] plainText Pointer to the plaintext buffer.
 * @param[in] length Plaintext length in bytes.
 * @param[out] cipherText Pointer to the ciphertext output buffer.
 * @return Execution status.
 *
 * @note The buffer addresses must be 32-bit aligned.
 * @warning `length` must be a multiple of 16 bytes.
 */
status_t HCU_DRV_EncryptECB(const void *plainText,
                            uint16_t length, void *cipherText);

/*!
 * @brief Decrypt a payload with AES in ECB mode.
 *
 * @param[in] cipherText Pointer to the ciphertext buffer.
 * @param[in] length Ciphertext length in bytes.
 * @param[out] plainText Pointer to the plaintext output buffer.
 * @return Execution status.
 *
 * @note The buffer addresses must be 32-bit aligned.
 * @warning `length` must be a multiple of 16 bytes.
 */
status_t HCU_DRV_DecryptECB(const void *cipherText,
                            uint16_t length, void *plainText);

#if FEATURE_HCU_SM4_ENGINE
/*!
 * @brief Encrypt a payload with SM4 in ECB mode.
 *
 * @param[in] plainText Pointer to the plaintext buffer.
 * @param[in] length Plaintext length in bytes.
 * @param[out] cipherText Pointer to the ciphertext output buffer.
 * @return Execution status.
 *
 * @note The buffer addresses must be 32-bit aligned.
 * @warning `length` must be a multiple of 16 bytes.
 */
status_t HCU_DRV_EncryptSM4ECB(const void *plainText,
                               uint16_t length, void *cipherText);

/*!
 * @brief Decrypt a payload with SM4 in ECB mode.
 *
 * @param[in] cipherText Pointer to the ciphertext buffer.
 * @param[in] length Ciphertext length in bytes.
 * @param[out] plainText Pointer to the plaintext output buffer.
 * @return Execution status.
 *
 * @note The buffer addresses must be 32-bit aligned.
 * @warning `length` must be a multiple of 16 bytes.
 */
status_t HCU_DRV_DecryptSM4ECB(const void *cipherText,
                               uint16_t length, void *plainText);
#endif /* FEATURE_HCU_SM4_ENGINE */

/*!
 * @brief Encrypt a payload with AES in CBC mode.
 *
 * @param[in] plainText Pointer to the plaintext buffer.
 * @param[in] length Plaintext length in bytes.
 * @param[in] iv Pointer to the IV buffer. Pass NULL to keep the current IV.
 * @param[out] cipherText Pointer to the ciphertext output buffer.
 * @return Execution status.
 *
 * @note The plaintext, IV, and ciphertext addresses must be 32-bit aligned
 *       when the corresponding pointer is non-NULL.
 * @warning `length` must be a multiple of 16 bytes.
 */
status_t HCU_DRV_EncryptCBC(const void *plainText, uint16_t length,
                            const void *iv, void *cipherText);

/*!
 * @brief Decrypt a payload with AES in CBC mode.
 *
 * @param[in] cipherText Pointer to the ciphertext buffer.
 * @param[in] length Ciphertext length in bytes.
 * @param[in] iv Pointer to the IV buffer. Pass NULL to keep the current IV.
 * @param[out] plainText Pointer to the plaintext output buffer.
 * @return Execution status.
 *
 * @note The ciphertext, IV, and plaintext addresses must be 32-bit aligned
 *       when the corresponding pointer is non-NULL.
 * @warning `length` must be a multiple of 16 bytes.
 */
status_t HCU_DRV_DecryptCBC(const void *cipherText, uint16_t length,
                            const void *iv, void *plainText);

#if FEATURE_HCU_AES_CTR_ENGINE
/*!
 * @brief Decrypt a payload with AES in CTR mode.
 *
 * @param[in] cipherText Pointer to the ciphertext buffer.
 * @param[in] length Ciphertext length in bytes.
 * @param[in] cv Pointer to the counter-value buffer. Pass NULL to keep the
 *               current counter value.
 * @param[out] plainText Pointer to the plaintext output buffer.
 * @return Execution status.
 *
 * @note The ciphertext, counter-value, and plaintext addresses must be 32-bit
 *       aligned when the corresponding pointer is non-NULL.
 * @warning `length` must be a multiple of 16 bytes.
 */
status_t HCU_DRV_DecryptCTR(const void *cipherText, uint16_t length, 
                            const void *cv, void *plainText);

/*!
 * @brief Encrypt a payload with AES in CTR mode.
 *
 * @param[in] plainText Pointer to the plaintext buffer.
 * @param[in] length Plaintext length in bytes.
 * @param[in] cv Pointer to the counter-value buffer. Pass NULL to keep the
 *               current counter value.
 * @param[out] cipherText Pointer to the ciphertext output buffer.
 * @return Execution status.
 *
 * @note The plaintext, counter-value, and ciphertext addresses must be 32-bit
 *       aligned when the corresponding pointer is non-NULL.
 * @warning `length` must be a multiple of 16 bytes.
 */
status_t HCU_DRV_EncryptCTR(const void *plainText, uint16_t length, 
                            const void *cv, void *cipherText);
#endif /* FEATURE_HCU_AES_CTR_ENGINE */

/*! @} */ /* End of Cipher Operations */

/*******************************************************************************
 * MAC & Hash Operations
 ******************************************************************************/
/*!
 * @name MAC & Hash Operations
 * @brief Functions for CMAC, SHA, and optional HMAC services.
 * @{
 */

/*!
 * @brief Generate a CMAC value with the AES engine.
 *
 * @param[in] msg Pointer to the message buffer. The address must be 32-bit aligned.
 * @param[in] msgLen Number of message bytes processed by this call.
 * @param[in] msgType Message-fragment type for segmented CMAC flows.
 * @param[in,out] cmacConfig CMAC buffer description used to return the MAC
 *                           bytes for generate flows.
 * @return Execution status.
 *
 * @note `MSG_END` and `MSG_ALL` cause the driver to finalize the MAC status.
 */
status_t HCU_DRV_GenerateMAC(const void *msg, uint16_t msgLen, hcu_msg_type_t msgType,
                             hcu_cmac_config_t *cmacConfig);

/*!
 * @brief Authorize a CMAC value with the AES engine.
 *
 * @param[in] msg Pointer to the message buffer. The address must be 32-bit aligned.
 * @param[in] msgLen Number of message bytes processed by this call.
 * @param[in] msgType Message-fragment type for segmented CMAC flows.
 * @param[in,out] cmacConfig CMAC buffer description that supplies the expected
 *                           MAC bytes.
 * @return Execution status.
 */
status_t HCU_DRV_AuthorizeMAC(const void *msg, uint16_t msgLen, hcu_msg_type_t msgType,
                              hcu_cmac_config_t *cmacConfig);

#if FEATURE_HCU_SHA_ENGINE
/*!
 * @brief Generate a SHA digest.
 *
 * @param[in] msg Pointer to the message buffer. The address must be 32-bit aligned.
 * @param[in] msgLen Number of bytes processed by this call.
 * @param[in] totalLen Total message length in bytes. Programmed on the first or
 *                     all-in-one SHA block.
 * @param[in] shaType SHA algorithm selector.
 * @param[in] msgType Message-fragment type for segmented SHA processing.
 * @param[out] result Pointer to the digest output buffer. The address must be
 *                    32-bit aligned when non-NULL.
 * @return Execution status.
 *
 * @warning For segmented SHA processing, `MSG_START` and `MSG_MIDDLE` blocks
 *          must be non-zero multiples of the hardware block size.
 */
status_t HCU_DRV_GenerateSHA(const void *msg, uint16_t msgLen, uint32_t totalLen,
                             hcu_sha_type_t shaType, hcu_msg_type_t msgType, void *result);

#if FEATURE_HCU_HAS_SHA_AUTHORIZE
/*!
 * @brief Authorize a SHA digest.
 *
 * @param[in] msg Pointer to the message buffer. The address must be 32-bit aligned.
 * @param[in] msgLen Number of bytes processed by this call.
 * @param[in] totalLen Total message length in bytes.
 * @param[in] shaType SHA algorithm selector.
 * @param[in] msgType Message-fragment type for segmented SHA authorization.
 * @param[in] result Pointer to the expected digest buffer. The address must be
 *                   32-bit aligned.
 * @param[out] trueResult Pointer to the computed digest buffer. The address
 *                        must be 32-bit aligned when non-NULL.
 * @return Execution status.
 */
status_t HCU_DRV_AuthorizeSHA(const void *msg, uint16_t msgLen, uint32_t totalLen,
                             hcu_sha_type_t shaType, hcu_msg_type_t msgType, void *result, void *trueResult);
#endif /* FEATURE_HCU_HAS_SHA_AUTHORIZE */
#endif /* FEATURE_HCU_SHA_ENGINE */

#if defined(FEATURE_HCU_HMAC_ENGINE) && (FEATURE_HCU_HMAC_ENGINE > 0)
/*!
 * @brief Load an HMAC key into the optional HMAC engine.
 *
 * @param[in] key Pointer to the HMAC key buffer. The address must be 32-bit aligned.
 * @param[in] keySize HMAC key-size encoding written to the control register.
 * @return Execution status.
 *
 * @note This API is available only when `FEATURE_HCU_HMAC_ENGINE` is enabled.
 */
status_t HCU_DRV_LoadHMACKey(const void *key, hcu_hmac_key_size_t keySize);

/*!
 * @brief Generate an HMAC digest.
 *
 * @param[in] msg Pointer to the message buffer. The address must be 32-bit aligned.
 * @param[in] msgLen Number of bytes processed by this call.
 * @param[in] totalLen Total message length in bytes.
 * @param[in] shaType SHA variant used by the HMAC engine.
 * @param[in] msgType Message-fragment type for segmented HMAC processing.
 * @param[out] result Pointer to the digest output buffer. The address must be
 *                    32-bit aligned when non-NULL.
 * @return Execution status.
 */
status_t HCU_DRV_GenerateHMAC(const void *msg, uint16_t msgLen, uint32_t totalLen,
                             hcu_sha_type_t shaType, hcu_msg_type_t msgType, void *result);

#if FEATURE_HCU_HAS_SHA_AUTHORIZE
/*!
 * @brief Authorize an HMAC digest.
 *
 * @param[in] msg Pointer to the message buffer. The address must be 32-bit aligned.
 * @param[in] msgLen Number of bytes processed by this call.
 * @param[in] totalLen Total message length in bytes.
 * @param[in] shaType SHA variant used by the HMAC engine.
 * @param[in] msgType Message-fragment type for segmented HMAC authorization.
 * @param[in] result Pointer to the expected digest buffer. The address must be
 *                   32-bit aligned.
 * @param[out] trueResult Pointer to the computed digest buffer. The address
 *                        must be 32-bit aligned when non-NULL.
 * @return Execution status.
 */
status_t HCU_DRV_AuthorizeHMAC(const void *msg, uint16_t msgLen, uint32_t totalLen,
                             hcu_sha_type_t shaType, hcu_msg_type_t msgType, void *result, void *trueResult);
#endif /* FEATURE_HCU_HAS_SHA_AUTHORIZE */
#endif /* FEATURE_HCU_HMAC_ENGINE */

/*! @} */ /* End of MAC & Hash Operations */

/*******************************************************************************
 * Authenticated Encryption
 ******************************************************************************/
/*!
 * @name Authenticated Encryption
 * @brief Functions for AES-CCM setup and payload processing.
 * @{
 */

#if FEATURE_HCU_AES_CCM_ENGINE
/*!
 * @brief Configure the CCM authenticated-data context.
 *
 * Programs the nonce, additional authenticated data, tag settings, and total
 * payload length required by subsequent CCM encrypt or decrypt calls.
 *
 * @param[in,out] ccm Pointer to the CCM configuration structure.
 * @param[in] mode Select encrypt or decrypt context setup.
 * @return Execution status.
 *
 * @warning `tagSize` must be even and must not exceed 16 bytes.
 */
status_t HCU_DRV_CCMConfig(hcu_ccm_config_t *ccm, hcu_mode_sel_t mode);

/*!
 * @brief Encrypt a CCM payload segment.
 *
 * @param[in] plainText Pointer to the plaintext buffer.
 * @param[in] length Payload length in bytes.
 * @param[out] cipherText Pointer to the ciphertext output buffer.
 * @param[in] isLast True when this segment is the last CCM payload block.
 * @return Execution status.
 *
 * @note `HCU_DRV_CCMConfig()` must succeed before this API is used.
 * @warning `length` must be a multiple of 16 bytes and the buffers must be
 *          32-bit aligned.
 */
status_t HCU_DRV_EncryptCCM(const void *plainText, uint16_t length,
                            void *cipherText, bool isLast);

/*!
 * @brief Decrypt a CCM payload segment.
 *
 * @param[in] cipherText Pointer to the ciphertext buffer.
 * @param[in] length Payload length in bytes.
 * @param[out] plainText Pointer to the plaintext output buffer.
 * @param[in] isLast True when this segment is the last CCM payload block.
 * @return Execution status.
 *
 * @note `HCU_DRV_CCMConfig()` must succeed before this API is used.
 * @warning `length` must be a multiple of 16 bytes and the buffers must be
 *          32-bit aligned.
 */
status_t HCU_DRV_DecryptCCM(const void *cipherText, uint16_t length,
                            void *plainText, bool isLast);
#endif /* FEATURE_HCU_AES_CCM_ENGINE */

/*! @} */ /* End of Authenticated Encryption */

/*******************************************************************************
 * Command Status & Interrupt Handling
 ******************************************************************************/
/*!
 * @name Command Status & Interrupt Handling
 * @brief Functions that expose command-cancel intent and the shared IRQ entry.
 * @{
 */

/*!
 * @brief Reserved hook for command cancellation support.
 *
 * @return Execution status reported by the cancel path.
 */
status_t HCU_DRV_CancelCommand(void);

/*!
 * @brief Shared HCU interrupt handler.
 *
 * Services input FIFO watermark, output FIFO watermark, and operation-done
 * interrupt events for interrupt-driven HCU commands.
 */
void HCU_DRV_IRQHandler(void);

/*! @} */ /* End of Command Status & Interrupt Handling */

#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* HCU_DRV_H */

/*******************************************************************************
 * EOF
 ******************************************************************************/
