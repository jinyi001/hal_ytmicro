/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file hcu_driver.h
 * @version 1.4.1
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
 * @addtogroup hcu_driver_v1
 * @{
 */

/*!
 * @brief Specify the key size to be used to implement the requested cryptographic
 * operation.
 *
 * Implements : hcu_key_size_t_Class
 */
typedef enum
{
    KEY_SIZE_128_BITS = 0x00U, /*!< 128-bit key size */
    KEY_SIZE_192_BITS = 0x01U, /*!< 192-bit key size */
    KEY_SIZE_256_BITS = 0x02U, /*!< 256-bit key size */
} hcu_key_size_t;

/*!
 * @brief Specify the block type used to implement the requested cryptographic
 * operation.
 *
 * Implements : hcu_block_type_t_Class
 */
typedef enum
{
    MSG_START = 0x02U,       /*!< Start of a message block */
    MSG_END = 0x01U,         /*!< End of a message block */
    MSG_ALL = 0x03U,         /*!< All message in one block */
    MSG_MIDDLE = 0x00U,      /*!< All message in one block */
} hcu_msg_type_t;

/*!
 * @brief Specify the SHA type used to implement the requested cryptographic
 * operation.
 *
 * Implements : hcu_sha_type_t_Class
 */
typedef enum
{
    HCU_SHA_256 = 0x01U,       /*!< SHA 256 algorithm */
    HCU_SHA_384 = 0x02U,       /*!< SHA 384 algorithm */
} hcu_sha_type_t;

/*! 
 * @brief HCU mode selection 
 * 
 * Implements : hcu_mode_sel_t_Class
 */
typedef enum
{
    MODE_ENC = 1U,          /*!< Encryption mode */
    MODE_DEC = 0U           /*!< Decryption mode */
} hcu_mode_sel_t;

/*! 
 * @brief HCU swapping mode 
 * 
 * Implements : hcu_swapping_t_Class
 */
typedef enum
{
    MODE_SWAPPING_NO        = 0U,          /*!< No swapping */
    MODE_SWAPPING_HALFWORD  = 1U,          /*!< Half-word swapping */
    MODE_SWAPPING_BYTE      = 2U,          /*!< Byte swapping */
    MODE_SWAPPING_BIT       = 3U,          /*!< Bit swapping */
} hcu_swapping_t;

/*! 
 * @brief Define input data and output data carried type 
 *
 * Implements : hcu_carry_type_t_Class
 */
typedef enum
{
    HCU_USING_POLLING         = 0U,          /*!< Carry data by polling */
    HCU_USING_INTERRUPT       = 1U,          /*!< Carry data by interrupt */
    HCU_USING_DMA             = 2U,          /*!< Carry data by DMA */
} hcu_carry_type_t;

/*! 
 * @brief HCU algorithm selection
 *
 * Implements : hcu_alg_mode_t_Class
 */
typedef enum
{
    ALG_DISABLE   = 0x000U,             /*!< No algorithm selected */
    AES_ECB_MODE  = 0x001U,             /*!< AES-ECB selected */
    AES_CBC_MODE  = 0x002U,             /*!< AES-CBC selected */
    AES_CTR_MODE  = 0x003U,             /*!< AES-CTR selected */
    AES_CCM_MODE  = 0x004U,             /*!< AES-CCM selected */
    AES_CMAC_MODE = 0x005U,             /*!< AES-CMAC selected */
    SM4_ECB_MODE  = 0x010U,             /*!< SM4-ECB selected */
    SHA_256_MODE  = 0x200U,             /*!< SHA-256 selected */
    SHA_384_MODE  = 0x300U              /*!< SHA-384 selected */
} hcu_alg_mode_t;

#if defined(FEATURE_HCU_HMAC_ENGINE) && (FEATURE_HCU_HMAC_ENGINE > 0)
/*!
 * @brief Specify the key size to be used to implement the requested cryptographic
 * operation for HMAC.
 *
 * Implements :hcu_hmac_key_size_t_Class
 */
typedef enum
{
    HMAC_KEY_16_BITS = 0x0U,            /*!< 16-bit HMAC key size*/
    HMAC_KEY_32_BITS = 0x1U,            /*!< 32-bit HMAC key size*/
    HMAC_KEY_64_BITS = 0x2U,            /*!< 64-bit HMAC key size*/
    HMAC_KEY_128_BITS = 0x3U,           /*!< 128-bit HMAC key size*/
    HMAC_KEY_256_BITS = 0x4U,           /*!< 256-bit HMAC key size*/
} hcu_hmac_key_size_t;
#endif /* FEATURE_HCU_HMAC_ENGINE */

/*!
 * @brief Specifies the CCM configuration.
 *
 * Implements : hcu_ccm_config_t_Class
 */
typedef struct
{
    uint8_t *nonce;         /*!< Specifies the nonce used for the CCM operation */
    uint8_t nonceSize;      /*!< Specifies the size of the nonce used for the CCM operation */
    uint8_t *addData;       /*!< Specifies the additional data used for the CCM operation */
    uint8_t addDataSize;    /*!< Specifies the size of the additional data used for the CCM operation */
    uint8_t *tag;           /*!< Specifies the tag used for the CCM operation */
    uint8_t tagSize;        /*!< Specifies the size of the tag used for the CCM operation */
    uint64_t msgLen;        /*!< Specifies the length of the plain text or cipher text buffer */
} hcu_ccm_config_t;

/*!
 * @brief Specifies the CMAC configuration.
 *
 * Implements : hcu_cmac_config_t_Class
 */
typedef struct
{
    uint8_t *macPtr;        /*!< Specifies the mac used for the CMAC operation */
    uint8_t macLen;         /*!< Specifies the length of the mac used for the CMAC operation */
} hcu_cmac_config_t;

/*!
 * @brief Specifies the user configuration of HCU.
 *
 * Implements : hcu_user_config_t_Class
 */
typedef struct
{
    hcu_swapping_t swap;           /*!< Specifies the type of swapping */
    hcu_carry_type_t carryType;    /*!< Specifies the type of data carried type */
    uint8_t ingressDMAChannel;     /*!< Channel number for DMA ingress channel */
    uint8_t egressDMAChannel;      /*!< Channel number for DMA egress channel */
} hcu_user_config_t;

/*!
 * @brief Internal driver state information.
 *
 * @note The contents of this structure are internal to the driver and should not be
 *       modified by users. Also, contents of the structure are subject to change in
 *       future releases.
 *
 * Implements : hcu_state_t_Class
 */
typedef struct
{
    volatile bool cmdInProgress;  /*!< Specifies if a command is in progress */
    bool isLastBlock;             /*!< Specifies if the last block */
    bool blockingCmd;             /*!< Specifies if a command is blocking or asynchronous */
    security_callback_t callback; /*!< The callback invoked when a command is complete */
    void *callbackParam;          /*!< User parameter for the command completion callback */
    semaphore_t cmdComplete;      /*!< Synchronization object for synchronous operation */
    uint32_t const *dataInputPtr;  /*!< Specifies current processing data input pointer */
    uint32_t *dataOutputPtr;       /*!< Specifies current processing data output pointer */
    uint64_t msgLen;              /*!< Specifies the length of the plain text or cipher text buffer */
    uint64_t inputCount;          /*!< Specifies the input fifo count of the plain text or cipher text buffer */
    uint64_t outputCount;         /*!< Specifies the output fifo count of the plain text or cipher text buffer */
    uint8_t ingressDMAChannel;    /*!< Channel number for DMA ingress channel */
    uint8_t egressDMAChannel;     /*!< Channel number for DMA egress channel */
    hcu_ccm_config_t *ccmConfig;  /*!< Specifies the CCM configuration */
    hcu_cmac_config_t *cmacConfig; /*!< Specifies the CMAC configuration */
    status_t status;              /*!< Specifies the current status */
    hcu_mode_sel_t mode;           /*!< Encryption or decryption */
    hcu_carry_type_t carryType;    /*!< Specifies the type of data carried type */
    hcu_alg_mode_t algorithm;      /*!< Specifies the current algorithm */
} hcu_state_t;


/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initializes the internal state of the driver and enables the HCU interrupt.
 *
 * @param[in] userConfig Pointer to the configuration of hcu module.
 * @param[in] state Pointer to the state structure which will be used for holding
 * the internal state of the driver.
 * @return Error Code after command execution.
 */
status_t HCU_DRV_Init(const hcu_user_config_t * userConfig, hcu_state_t *state);

/*!
 * @brief De-initializes the internal state of the driver and disables the HCU interrupt.
 *
 * @param[in] state Pointer to the state structure which will be used for holding
 * the internal state of the driver.
 * @return Error Code after command execution.
 */
status_t HCU_DRV_DeInit(hcu_state_t *state);

/*!
 * @brief Configure Data Swapping.
 *
 * @param[in] cfg HCU swapping type.
 *
 * @return None.
 */
void HCU_DRV_CfgSwapping(hcu_swapping_t cfg);

/*!
 * @brief Clear operate done flag.
 *
 * @return None.
 */
void HCU_DRV_ClearODFlag(void);

/*!
 * @brief Load user defined key into the key slot.
 *
 * @param[in] key Pointer to the user defined key.
 * @param[in] keySize Specifies the size of the key.
 * @return Key load result.
 */
status_t HCU_DRV_LoadUserKey(const void *key, hcu_key_size_t keySize);

/*!
 * @brief Installs a user callback for the command complete event.
 *
 * This function installs a user callback for the command complete event.
 *
 * @param[in] callbackFunction The pointer to the callback function.
 * @param[in] callbackParam The pointer to the callback function's parameter.
 * @return Pointer to the previous callback.
 */
security_callback_t HCU_DRV_InstallCallback(security_callback_t callbackFunction, void *callbackParam);

/*!
 * @brief Performs the AES encryption in ECB mode.
 *
 * This function performs the AES encryption in ECB mode of the input
 * plain text buffer
 *
 * @param[in] plainText Pointer to the plain text buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] length Number of bytes of plain text message to be encrypted.
 *            @note Should be multiple of 16 bytes.
 * @param[out] cipherText Pointer to the cipher text buffer. The buffer shall
 * have the same size as the plain text buffer.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_EncryptECB(const void *plainText,
                            uint16_t length, void *cipherText);

/*!
 * @brief Performs the AES decryption in ECB mode.
 *
 * This function performs the AES decryption in ECB mode of the input
 * cipher text buffer.
 *
 * @param[in] cipherText Pointer to the cipher text buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] length Number of bytes of cipher text message to be decrypted.
 *            @note Should be multiple of 16 bytes.
 * @param[out] plainText Pointer to the plain text buffer. The buffer shall
 * have the same size as the cipher text buffer.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_DecryptECB(const void *cipherText,
                            uint16_t length, void *plainText);

#if FEATURE_HCU_SM4_ENGINE
/*!
 * @brief Performs the SM4-128 encryption in ECB mode.
 *
 * This function performs the SM4-128 encryption in ECB mode of the input
 * plain text buffer
 *
 * @param[in] plainText Pointer to the plain text buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] length Number of bytes of plain text message to be encrypted.
 *            @note Should be multiple of 16 bytes.
 * @param[out] cipherText Pointer to the cipher text buffer. The buffer shall
 * have the same size as the plain text buffer.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_EncryptSM4ECB(const void *plainText,
                               uint16_t length, void *cipherText);

/*!
 * @brief Performs the SM4-128 decryption in ECB mode.
 *
 * This function performs the SM4-128 decryption in ECB mode of the input
 * cipher text buffer.
 *
 * @param[in] cipherText Pointer to the cipher text buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] length Number of bytes of cipher text message to be decrypted.
 *            @note Should be multiple of 16 bytes.
 * @param[out] plainText Pointer to the plain text buffer. The buffer shall
 * have the same size as the cipher text buffer.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_DecryptSM4ECB(const void *cipherText,
                               uint16_t length, void *plainText);
#endif /* FEATURE_HCU_SM4_ENGINE */

/*!
 * @brief Performs the AES encryption in CBC mode.
 *
 * This function performs the AES encryption in CBC mode of the input
 * plaintext buffer.
 *
 * @param[in] plainText Pointer to the plain text buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] length Number of bytes of plain text message to be encrypted.
 *            @note Should be multiple of 16 bytes.
 * @param[in] iv Pointer to the initialization vector buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[out] cipherText Pointer to the cipher text buffer. The buffer shall
 * have the same size as the plain text buffer.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_EncryptCBC(const void *plainText, uint16_t length,
                            const void *iv, void *cipherText);

/*!
 * @brief Performs the AES decryption in CBC mode.
 *
 * This function performs the AES decryption in CBC mode of the input
 * cipher text buffer.
 *
 * @param[in] cipherText Pointer to the cipher text buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] length Number of bytes of cipher text message to be decrypted.
 * It should be multiple of 16 bytes.
 * @param[in] iv Pointer to the initialization vector buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[out] plainText Pointer to the plain text buffer. The buffer shall
 * have the same size as the cipher text buffer.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_DecryptCBC(const void *cipherText, uint16_t length,
                            const void *iv, void *plainText);

#if FEATURE_HCU_AES_CTR_ENGINE
/*!
 * @brief Performs the AES decryption in CTR mode.
 *
 * This function performs the AES decryption in CTR mode of the input
 * cipher text buffer.
 *
 * @param[in] cipherText Pointer to the cipher text buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] length Number of bytes of cipher text message to be decrypted.
 * It should be multiple of 16 bytes.
 * @param[in] cv Pointer to the count value buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[out] plainText Pointer to the plain text buffer. The buffer shall
 * have the same size as the cipher text buffer.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_DecryptCTR(const void *cipherText, uint16_t length, 
                            const void *cv, void *plainText);

/*!
 * @brief Performs the AES encryption in CTR mode.
 *
 * This function performs the AES encryption in CTR mode of the input
 * plaintext buffer.
 *
 * @param[in] plainText Pointer to the plain text buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] length Number of bytes of plain text message to be encrypted.
 *            @note Should be multiple of 16 bytes.
 * @param[in] cv Pointer to the count value buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[out] cipherText Pointer to the cipher text buffer. The buffer shall
 * have the same size as the plain text buffer.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_EncryptCTR(const void *plainText, uint16_t length, 
                            const void *cv, void *cipherText);
#endif

/*!
 * @brief Calculates the MAC of a given message using CMAC with AES.
 *
 * This function calculates the MAC of a given message using CMAC with AES.
 *
 * @param[in] msg Pointer to the message buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] msgLen Number of bytes of message on which CMAC will be computed.
 * @param[in] msgType Indicates whether the message is header or data.
 * @param[out] cmacConfig Pointer to the cmac configuration.
 * computation.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_GenerateMAC(const void *msg, uint16_t msgLen, hcu_msg_type_t msgType,
                             hcu_cmac_config_t *cmacConfig);

/*!
 * @brief Authorize the MAC of a given message using CMAC with AES.
 *
 * This function authorizes the MAC of a given message and mac using CMAC with AES.
 *
 * @param[in] msg Pointer to the message buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] msgLen Number of bytes of message on which CMAC will be computed.
 * @param[in] msgType Indicates whether the message is header or data.
 * @param[in] cmacConfig Pointer to the cmac configuration.
 * computation.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_AuthorizeMAC(const void *msg, uint16_t msgLen, hcu_msg_type_t msgType,
                              hcu_cmac_config_t *cmacConfig);

#if FEATURE_HCU_SHA_ENGINE
/*!
 * @brief Calculates the result of a given message using SHA.
 *
 * This function calculates the result of a given message using SHA.
 *
 * @param[in] msg Pointer to the message buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] msgLen Number of bytes of message on which SHA will be computed.
 * @param[in] totalLen Number of bytes of total message on which SHA will be computed.
 * @param[in] shaType SHA algorithm type.
 * @param[in] msgType Indicates whether the message is header or data.
 * @param[out] result Pointer to the buffer containing the result of the CMAC
 * computation.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_GenerateSHA(const void *msg, uint16_t msgLen, uint32_t totalLen,
                             hcu_sha_type_t shaType, hcu_msg_type_t msgType, void *result);

#if FEATURE_HCU_HAS_SHA_AUTHORIZE
/*!
 * @brief Authorize the result of a given message using SHA.
 *
 * This function authorizes the result of a given message using SHA.
 *
 * @param[in] msg Pointer to the message buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] msgLen Number of bytes of message on which SHA will be computed.
 * @param[in] totalLen Number of bytes of total message on which SHA will be computed.
 * @param[in] shaType SHA algorithm type.
 * @param[in] msgType Indicates whether the message is header or data.
 * @param[in] result Pointer to the buffer containing the given result of the SHA
 * computation.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @param[out] trueResult Pointer to the buffer containing the result of the SHA
 * computation.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_AuthorizeSHA(const void *msg, uint16_t msgLen, uint32_t totalLen,
                             hcu_sha_type_t shaType, hcu_msg_type_t msgType, void *result, void *trueResult);
#endif /* FEATURE_HCU_HAS_SHA_AUTHORIZE */
#endif /* FEATURE_HCU_SHA_ENGINE */

#if FEATURE_HCU_AES_CCM_ENGINE
/*!
 * @brief CCM parameters configuration.
 *
 * Config ccm parameters include nonce, addition data and tag.
 *
 * @param[in] ccm Pointer to the configuration of CCM.
 * @param[in] mode Indicate if encrypt or decrypt.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_CCMConfig(hcu_ccm_config_t *ccm, hcu_mode_sel_t mode);

/*!
 * @brief Performs the AES encryption in CCM mode.
 *
 * This function performs the AES encryption in CCM mode of the input
 * cipher text buffer.
 *
 * @param[in] plainText Pointer to the plain text buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] length Number of bytes of cipher text message to be decrypted.
 * It should be multiple of 16 bytes.
 * @param[out] cipherText Pointer to the cipher text buffer. The buffer shall
 * have the same size as the plain text buffer.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] isLast Indicate if is the last block.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_EncryptCCM(const void *plainText, uint16_t length,
                            void *cipherText, bool isLast);

/*!
 * @brief Performs the AES decryption in CCM mode.
 *
 * This function performs the AES decryption in CCM mode of the input
 * cipher text buffer.
 *
 * @param[in] cipherText Pointer to the cipher text buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] length Number of bytes of cipher text message to be decrypted.
 * It should be multiple of 16 bytes.
 * @param[out] plainText Pointer to the plain text buffer. The buffer shall
 * have the same size as the cipher text buffer.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] isLast Indicate if is the last block.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_DecryptCCM(const void *cipherText, uint16_t length,
                            void *plainText, bool isLast);
#endif /* FEATURE_HCU_AES_CCM_ENGINE */

#if defined(FEATURE_HCU_HMAC_ENGINE) && (FEATURE_HCU_HMAC_ENGINE > 0)
/*!
 * @brief Performs the AES load key for HMAC.
 *
 * This function loads the user key into the HCU for HMAC.
 *
 * @param[in] key Pointer to the user defined key.
 * @param[in] keySize Specifies the size of the key.
 * @return Key load result.
 */
status_t HCU_DRV_LoadHMACKey(const void *key, hcu_hmac_key_size_t keySize);

/*!
 * @brief Calculates the result of a given message using HMAC.
 *
 * This function calculates the result of a given message using HMAC.
 *
 * @param[in] msg Pointer to the message buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] msgLen Number of bytes of message on which HMAC will be computed.
 * @param[in] totalLen Number of bytes of total message on which HMAC will be computed.
 * @param[in] shaType SHA algorithm type.
 * @param[in] msgType Indicates whether the message is header or data.
 * @param[out] result Pointer to the buffer containing the result of the CMAC
 * computation.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_GenerateHMAC(const void *msg, uint16_t msgLen, uint32_t totalLen,
                             hcu_sha_type_t shaType, hcu_msg_type_t msgType, void *result);

#if FEATURE_HCU_HAS_SHA_AUTHORIZE
/*!
 * @brief Authorize the result of a given message using HMAC.
 *
 * This function authorizes the result of a given message using HMAC.
 *
 * @param[in] msg Pointer to the message buffer.
 *            @note Address passed in this parameter must be 32 bit aligned.
 * @param[in] msgLen Number of bytes of message on which HMAC will be computed.
 * @param[in] totalLen Number of bytes of total message on which HMAC will be computed.
 * @param[in] shaType SHA algorithm type.
 * @param[in] msgType Indicates whether the message is header or data.
 * @param[in] result Pointer to the buffer containing the given result of the HMAC
 * computation.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @param[out] trueResult Pointer to the buffer containing the result of the HMAC
 * computation.
 *             @note Address passed in this parameter must be 32 bit aligned.
 * @return Error Code after command execution. Output parameters are valid if
 * the error code is STATUS_SUCCESS.
 */
status_t HCU_DRV_AuthorizeHMAC(const void *msg, uint16_t msgLen, uint32_t totalLen,
                             hcu_sha_type_t shaType, hcu_msg_type_t msgType, void *result, void *trueResult);
#endif /* FEATURE_HCU_HAS_SHA_AUTHORIZE */
#endif /* FEATURE_HCU_HMAC_ENGINE */

/*!
 * @brief Cancels a previously initiated command.
 *
 * This function cancels any on-going HCU command.
 *
 * @return STATUS_SUCCESS.
 */
status_t HCU_DRV_CancelCommand(void);

/*!
 * @brief Interrupt handler for HCU hardware Interface.
 */
void HCU_DRV_IRQHandler(void);

#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* HCU_DRV_H */

/*******************************************************************************
 * EOF
 ******************************************************************************/
