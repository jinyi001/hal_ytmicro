/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file hcu_driver.c
 * @version 1.4.1
 *
 * @brief HCU V1 driver implementation.
 *
 * This file implements the public `HCU_DRV_*` APIs together with the internal
 * helpers that configure the active engine, service FIFO transfers, coordinate
 * DMA callbacks, and complete HCU operations through polling or interrupts.
 */

 /*
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 4394 Rule 10.8: A composite expression of 'essentially unsigned' type (unsigned short) 
 *                       is being cast to a different type category, 'enum'.
 * PRQA S 0316 Rule 11.5: Cast from a pointer to void to a pointer to object type.
 * PRQA S 0326 Rule 11.6: Cast between a pointer to void and an integral type.
 * PRQA S 3101 Rule 10.1: Unary '-' applied to an operand of type unsigned int or unsigned long gives an unsigned result.
 * PRQA S 4446 Rule 10.3: A non-constant expression of 'essentially unsigned' type (unsigned long long) 
 *                        is being converted to signed type, 'long' on assignment.
 */

#include "hcu_hw_access.h"
#include "interrupt_manager.h"
#include "core_common.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Runtime state pointer installed by HCU_DRV_Init(). */
static hcu_state_t *s_hcuStatePtr = NULL;

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/* Configure the engine, message lengths, and runtime pointers for one command. */
static status_t HCU_ConfigAlgorithm(const uint32_t *dataIn,
                                    uint16_t msgLen,
                                    uint16_t exMsgLen,
                                    uint32_t *dataOut,
                                    hcu_engine_sel_t eng,
                                    hcu_alg_aes_mode_t alg,
                                    hcu_mode_sel_t mode);

static status_t HCU_RunOneLoop(void);

/* Handle ingress or egress DMA completion. */
static void HCU_DRV_CompleteDMA(void *parameter, dma_chn_status_t status);

/* Configure the DMA channels used by the active command. */
static status_t HCU_DRV_ConfigDMA(uint8_t ingressDMAChannel, uint8_t egressDMAChannel);

/* Finalize MAC handling after the last CMAC or CCM block. */
static status_t HCU_DRV_DoneMAC(void);

#if 0
static void HCU_DRV_WaitCommandCompletion(uint32_t timeout);
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Initialize the HCU V1 driver state.
 */
status_t HCU_DRV_Init(const hcu_user_config_t *userConfig, hcu_state_t *state)
{
    /* Validate that the driver state pointer is available. */
    DEV_ASSERT(state != NULL);
    DEV_ASSERT(userConfig != NULL);

    status_t semaStatus;
    status_t retVal = STATUS_SUCCESS;

    /* Store the caller-provided runtime state pointer. */
    s_hcuStatePtr = state;

    /* Clear the runtime state fields tracked by the driver. */
    s_hcuStatePtr->cmdInProgress = false;
    s_hcuStatePtr->blockingCmd = false;
    s_hcuStatePtr->isLastBlock = false;
    s_hcuStatePtr->callback = NULL;
    s_hcuStatePtr->callbackParam = NULL;
    s_hcuStatePtr->dataInputPtr = NULL;
    s_hcuStatePtr->dataOutputPtr = NULL;
    s_hcuStatePtr->msgLen = 0U;
    s_hcuStatePtr->inputCount = 0U;
    s_hcuStatePtr->outputCount = 0U;
    s_hcuStatePtr->ccmConfig = NULL;
    s_hcuStatePtr->cmacConfig = NULL;
    s_hcuStatePtr->status = STATUS_SUCCESS;

    /* Copy the carry-mode selection and DMA channel assignments. */
    s_hcuStatePtr->carryType = userConfig->carryType;
    s_hcuStatePtr->ingressDMAChannel = userConfig->ingressDMAChannel;
    s_hcuStatePtr->egressDMAChannel = userConfig->egressDMAChannel;

    HCU_DRV_CfgSwapping(userConfig->swap);

    /* Create the semaphore used by the runtime state. */
    semaStatus = OSIF_SemaCreate(&s_hcuStatePtr->cmdComplete, 0U);
    if (semaStatus == STATUS_ERROR)
    {
        retVal = STATUS_ERROR;
    }

    return retVal;
}

/*!
 * @brief De-initialize the HCU V1 driver state.
 */
status_t HCU_DRV_DeInit(hcu_state_t *state)
{
    status_t errorCode = STATUS_SUCCESS;
    /* Clear the runtime state fields tracked by the driver. */
    s_hcuStatePtr->cmdInProgress = false;
    s_hcuStatePtr->blockingCmd = false;
    s_hcuStatePtr->isLastBlock = false;
    s_hcuStatePtr->callback = NULL;
    s_hcuStatePtr->callbackParam = NULL;
    s_hcuStatePtr->dataInputPtr = NULL;
    s_hcuStatePtr->dataOutputPtr = NULL;
    s_hcuStatePtr->msgLen = 0U;
    s_hcuStatePtr->inputCount = 0U;
    s_hcuStatePtr->outputCount = 0U;
    s_hcuStatePtr->ccmConfig = NULL;
    s_hcuStatePtr->cmacConfig = NULL;
    s_hcuStatePtr->status = STATUS_SUCCESS;
    /* Disable the shared HCU interrupt line. */
    INT_SYS_DisableIRQ(HCU_IRQn);
    /* Release the module-level runtime state pointer. */
    s_hcuStatePtr = NULL;
    HCU->CR = 0;
    HCU->INTE = 0;

    /* Destroy the semaphore owned by the caller state. */
    errorCode = OSIF_SemaDestroy(&(state->cmdComplete));
    DEV_ASSERT(errorCode == STATUS_SUCCESS);
    return errorCode;
}

/*!
 * @brief Program the HCU data-swapping mode.
 */
void HCU_DRV_CfgSwapping(hcu_swapping_t cfg)
{
    /* The swap-mode field is encoded on two bits. */
    DEV_ASSERT(cfg < 4);

    uint32_t temp;
    temp = HCU->CR;
    temp &= ~HCU_CR_DATSWP_MASK;
    temp |= HCU_CR_DATSWP(cfg);
    HCU->CR = temp;
}

/*!
 * @brief Clear the operation-done status flag.
 */
void HCU_DRV_ClearODFlag(void)
{
    /* Clear the operation-done flag through the W1C status register path. */
    HCU->SR = HCU_SR_OD_MASK;
}

/*!
 * @brief Install a completion callback for asynchronous command paths.
 */
security_callback_t HCU_DRV_InstallCallback(security_callback_t callbackFunction, void *callbackParam)
{
    /* Validate that the driver state pointer is available. */
    DEV_ASSERT(s_hcuStatePtr != NULL);

    security_callback_t currentCallback = s_hcuStatePtr->callback;
    s_hcuStatePtr->callback = callbackFunction;
    s_hcuStatePtr->callbackParam = callbackParam;

    return currentCallback;
}

/*!
 * @brief Load a software key into the HCU key registers.
 */
status_t HCU_DRV_LoadUserKey(const void *key, hcu_key_size_t keySize)
{
    /* The hardware expects 32-bit aligned key buffers. */
    DEV_ASSERT((((uint32_t)key) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    /* Validate that the driver state pointer is available. */
    DEV_ASSERT(s_hcuStatePtr != NULL);
    uint8_t keySizeInWords;
    const uint32_t *keyPtr = (const uint32_t *)key; /* PRQA S 0316 */
    uint8_t keyLoopIndex;
    status_t retVal = STATUS_SUCCESS;
    switch (keySize)
    {
        case KEY_SIZE_128_BITS:
            keySizeInWords = 4U;
            break;
        case KEY_SIZE_192_BITS:
            keySizeInWords = 6U;
            break;
        case KEY_SIZE_256_BITS:
            keySizeInWords = 8U;
            break;
        default:
            retVal = STATUS_HCU_KEY_SIZE_NOT_SUPPORTED;
            break;
    }
    if(retVal == STATUS_SUCCESS)
    {
        /* Refuse key updates while another command is active. */
        if (s_hcuStatePtr->cmdInProgress)
        {
            retVal = STATUS_HCU_LOAD_KEY_WHEN_BUSY;
        }else
        {
            /* Stream the software key words into the HCU key registers. */
            for (keyLoopIndex = 0; keyLoopIndex < keySizeInWords; keyLoopIndex++)
            {
                HCU_SetUserKey(keyPtr[keyLoopIndex], keyLoopIndex);
            }
            /* Program the matching key-size field. */
            HCU_SetKeySize(keySize);
        }
    }
    return retVal;
}

#if FEATURE_HCU_HMAC_ENGINE
/*!
 * @brief Load an HMAC key into the optional HMAC engine.
 */
status_t HCU_DRV_LoadHMACKey(const void *key, hcu_hmac_key_size_t keySize)
{
    /* The hardware expects 32-bit aligned key buffers. */
    DEV_ASSERT((((uint32_t)key) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    /* Validate that the driver state pointer is available. */
    DEV_ASSERT(s_hcuStatePtr != NULL);
    uint8_t keySizeInWords;
    uint32_t *keyPtr = (uint32_t *)key;
    uint8_t keyLoopIndex;

    keySizeInWords = ((2 << keySize) >> 2);
    keySizeInWords = (keySizeInWords == 0) ? 1 : keySizeInWords;

    /* Refuse key updates while another command is active. */
    if (s_hcuStatePtr->cmdInProgress)
    {
        return STATUS_HCU_LOAD_KEY_WHEN_BUSY;
    }
    /* Enable the optional HMAC extension before loading its key. */
    HCU_EnableHMAC(true);
    /* Stream the software key words into the HCU key registers. */
    for (keyLoopIndex = 0; keyLoopIndex < keySizeInWords; keyLoopIndex++)
    {
        HCU_SetUserKey(keyPtr[keyLoopIndex], keyLoopIndex);
    }
    /* Program the matching key-size field. */
    HCU_SetHMACKeySize(keySize);
    return STATUS_SUCCESS;
}
#endif /* FEATURE_HCU_HMAC_ENGINE */

/*!
 * @brief Configure the active engine context for one command.
 */
static status_t HCU_ConfigAlgorithm(const uint32_t *dataIn,
                                    uint16_t msgLen,
                                    uint16_t exMsgLen,
                                    uint32_t *dataOut,
                                    hcu_engine_sel_t eng,
                                    hcu_alg_aes_mode_t alg,
                                    hcu_mode_sel_t mode)
{
    /* Validate that both payload buffers are present. */
    DEV_ASSERT(dataIn != NULL);
    /* The HCU data path requires 32-bit aligned buffers. */
    DEV_ASSERT((((uint32_t)dataIn) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    DEV_ASSERT((((uint32_t)dataOut) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    status_t retVal = STATUS_SUCCESS;

    /* Reject the request if the hardware or driver is already busy. */
    if (HCU_IsBusy() || s_hcuStatePtr->cmdInProgress)
    {
        retVal = STATUS_BUSY;
    }else
    {
        /* Update the runtime state for the new command. */
        s_hcuStatePtr->cmdInProgress = true;
        /* Program the engine, algorithm, and direction fields. */
        HCU_SetEngineAlgorithm(eng, alg, mode);
        /* Program the message-length fields for this transfer. */
        HCU_SetMsgLength(msgLen, exMsgLen);
        /* Save the FIFO source, destination, and byte counters. */
        s_hcuStatePtr->mode = mode;
        s_hcuStatePtr->msgLen = msgLen;
        s_hcuStatePtr->inputCount = msgLen;
        s_hcuStatePtr->outputCount = msgLen;
        s_hcuStatePtr->dataInputPtr = dataIn;
        s_hcuStatePtr->dataOutputPtr = dataOut;
        s_hcuStatePtr->algorithm = (hcu_alg_mode_t)(((uint16_t)alg + 1u) << (((uint16_t)eng - 1u) << 2u)); /* PRQA S 4394 */
#if FEATURE_HCU_SHA_ENGINE
        /* SHA commands always produce a fixed digest length. */
        if (ENG_SHA == eng)
        {
            if(alg == ALG_AES_CBC)
            {
                s_hcuStatePtr->outputCount = HCU_SHA_256_LENGTH;
            }else
            {
                s_hcuStatePtr->outputCount = HCU_SHA_384_LENGTH;
            }
        }
#endif /* FEATURE_HCU_SHA_ENGINE */
        /* Program the FIFO watermarks used by the runtime loop. */
        HCU_SetFIFOWatermark(FEATURE_HCU_ONE_LOOP_INPUT_WATERMARK, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
    }

    return retVal;
}

/*!
 * @brief Handle ingress or egress DMA completion.
 */
static void HCU_DRV_CompleteDMA(void *parameter, dma_chn_status_t status)
{
    DEV_ASSERT(s_hcuStatePtr != NULL);
    uint32_t channel = (uint32_t)parameter; /* PRQA S 0326 */
    if (status == DMA_CHN_ERROR)
    {
        HCU_SetInputDMA(false);
        HCU_SetOutputDMA(false);
    }
    else
    {
        if (channel == s_hcuStatePtr->ingressDMAChannel)
        {
            s_hcuStatePtr->inputCount = 0;
#if FEATURE_HCU_HAS_FIXED_DMA
            HCU_SetInputDMA(false);
#endif
        }
        else
        {
            HCU_SetInputDMA(false);
            HCU_SetOutputDMA(false);
            s_hcuStatePtr->outputCount = 0;
            s_hcuStatePtr->cmdInProgress = false;
            if ((SHA_256_MODE != s_hcuStatePtr->algorithm) && (SHA_384_MODE != s_hcuStatePtr->algorithm))
            {
                HCU_ClearStatusFlag(OPERATION_DONE_FLAG);
            }
            /* CCM completion may need to copy or verify the authentication tag. */
            if ((AES_CCM_MODE == s_hcuStatePtr->algorithm) && (s_hcuStatePtr->isLastBlock))
            {
                (void)HCU_DRV_DoneMAC();
            }
        }
    }
}

/*!
 * @brief Configure the DMA service path for the active command.
 */
static status_t HCU_DRV_ConfigDMA(uint8_t ingressDMAChannel, uint8_t egressDMAChannel)
{
    DEV_ASSERT(s_hcuStatePtr != NULL);
    dma_transfer_size_t dmaTransferSize = DMA_TRANSFER_SIZE_4B;
    s_hcuStatePtr->ingressDMAChannel = ingressDMAChannel;
    s_hcuStatePtr->egressDMAChannel = egressDMAChannel;

    if (s_hcuStatePtr->dataOutputPtr != NULL)
    {
        /* Configure one DMA block per FIFO watermark service burst. */
        (void)DMA_DRV_ConfigMultiBlockTransfer(s_hcuStatePtr->egressDMAChannel,
                                               DMA_TRANSFER_PERIPH2MEM,
                                               (uint32_t)(&(HCU->OFDAT)),
                                               (uint32_t)s_hcuStatePtr->dataOutputPtr,
                                               dmaTransferSize,
                                               16U,
                                               (uint32_t)s_hcuStatePtr->outputCount / 16U,
                                               true);

        /* Route DMA completion into the shared HCU DMA callback. */
        (void)DMA_DRV_InstallCallback(s_hcuStatePtr->egressDMAChannel,
                                      (HCU_DRV_CompleteDMA),
                                      (void *)(uint32_t)(s_hcuStatePtr->egressDMAChannel));/* PRQA S 0326 */
        (void)DMA_DRV_StartChannel(s_hcuStatePtr->egressDMAChannel);
        HCU_SetOutputDMA(true);
    }

    if (s_hcuStatePtr->dataInputPtr != NULL)
    {
#if FEATURE_HCU_HAS_FIXED_DMA
        {
            /* Configure one DMA block per FIFO watermark service burst. */
            (void)DMA_DRV_ConfigMultiBlockTransfer(s_hcuStatePtr->ingressDMAChannel,
                                                   DMA_TRANSFER_MEM2PERIPH,
                                                   (uint32_t)s_hcuStatePtr->dataInputPtr,
                                                   (uint32_t)(&(HCU->IFDAT)),
                                                   dmaTransferSize,
                                                   16U,
                                                   (uint32_t)s_hcuStatePtr->inputCount / 16U,
                                                   true);
        }
#else
        {
            /* Configure one DMA block per FIFO watermark service burst. */
            (void)DMA_DRV_ConfigMultiBlockTransfer(s_hcuStatePtr->ingressDMAChannel,
                                                   DMA_TRANSFER_MEM2PERIPH,
                                                   (uint32_t)s_hcuStatePtr->dataInputPtr,
                                                   (uint32_t)(&(HCU->IFDAT)),
                                                   dmaTransferSize,
                                                   16U,
                                                   (uint32_t)s_hcuStatePtr->inputCount / 16U,
                                                   false);
        }
        /* Clamp the source address adjustment on devices without fixed DMA. */
        DMA_DRV_SetSrcLastAddrAdjustment(s_hcuStatePtr->ingressDMAChannel, -s_hcuStatePtr->msgLen); /* PRQA S 3101, 4446 */
#endif
        /* Route DMA completion into the shared HCU DMA callback. */
        (void)DMA_DRV_InstallCallback(s_hcuStatePtr->ingressDMAChannel,
                                      (HCU_DRV_CompleteDMA),
                                      (void *)(uint32_t)(s_hcuStatePtr->ingressDMAChannel));/* PRQA S 0326 */
        (void)DMA_DRV_StartChannel(s_hcuStatePtr->ingressDMAChannel);
        HCU_SetInputDMA(true);
    }

    return STATUS_SUCCESS;
}

/*!
 * @brief Finalize CMAC or CCM tag handling for the active command.
 */
static status_t HCU_DRV_DoneMAC(void)
{
    DEV_ASSERT(s_hcuStatePtr != NULL);
    status_t status = STATUS_SUCCESS;
    if (MODE_ENC == s_hcuStatePtr->mode)
    {
        uint8_t i;
        uint8_t macPtr[16];
        HCU_ReadMac(macPtr);
        if (AES_CCM_MODE == s_hcuStatePtr->algorithm)
        {
            for (i = 0; (NULL != s_hcuStatePtr->ccmConfig) && (i < s_hcuStatePtr->ccmConfig->tagSize); i++)
            {
                s_hcuStatePtr->ccmConfig->tag[i] = macPtr[i];
            }
        }
        else if (AES_CMAC_MODE == s_hcuStatePtr->algorithm)
        {
            for (i = 0; (NULL != s_hcuStatePtr->cmacConfig) && (i < s_hcuStatePtr->cmacConfig->macLen); i++)
            {
                s_hcuStatePtr->cmacConfig->macPtr[i] = macPtr[i];
            }
        }else {
            /* No additional work is required for other carry modes. */
        }
    }
    else
    {
        if (false == HCU_GetStatusFlag(AES_MAC_VALID_FLAG))
        {
            status = STATUS_HCU_MAC_CHECK_ERROR;
            s_hcuStatePtr->status = STATUS_HCU_MAC_CHECK_ERROR;
        }
        else
        {
            s_hcuStatePtr->status = STATUS_SUCCESS;
        }
    }
    return status;
}

/*!
 * @brief Encrypt a payload with AES in ECB mode.
 */
status_t HCU_DRV_EncryptECB(const void *plainText, uint16_t length, void *cipherText)
{
    status_t status;
    /* Prepare the command-specific engine and runtime context. */
    status = HCU_ConfigAlgorithm((const uint32_t *)plainText, length, 0, cipherText, ENG_AES, ALG_AES_ECB, MODE_ENC); /* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable the FIFO watermark and operation-done interrupts used by cipher flows. */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* No additional work is required for other carry modes. */
        }
    
        /* Launch the configured HCU command. */
        HCU_StartEngine();
        /* Polling mode stays in the API until the command finishes. */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            s_hcuStatePtr->cmdInProgress = false;
        }
    }

    return status;
}

/*!
 * @brief Decrypt a payload with AES in ECB mode.
 */
status_t HCU_DRV_DecryptECB(const void *cipherText, uint16_t length, void *plainText)
{
    status_t status;
    /* Prepare the command-specific engine and runtime context. */
    status = HCU_ConfigAlgorithm((const uint32_t *)cipherText, length, 0, plainText, ENG_AES, ALG_AES_ECB, MODE_DEC);/* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable the FIFO watermark and operation-done interrupts used by cipher flows. */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* No additional work is required for other carry modes. */
        }
    
        /* Launch the configured HCU command. */
        HCU_StartEngine();
        /* Polling mode stays in the API until the command finishes. */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            s_hcuStatePtr->cmdInProgress = false;
        }
    }

    return status;
}

#if FEATURE_HCU_AES_CTR_ENGINE
/*!
 * @brief Encrypt a payload with AES in CTR mode.
 */
status_t HCU_DRV_EncryptCTR(const void *plainText, uint16_t length, const void *cv, void *cipherText)
{
    status_t status;
    /* Prepare the command-specific engine and runtime context. */
    status = HCU_ConfigAlgorithm((const uint32_t *)plainText, length, 0, cipherText, ENG_AES, ALG_AES_CTR, MODE_ENC);/* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        /* Update the counter-value registers when the caller supplies them. */
        if (NULL != cv)
        {
            /* Preserve the current counter value when no replacement is provided. */
            HCU_SetCV((const uint32_t *)cv);/* PRQA S 0316 */
        }
    
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable the FIFO watermark and operation-done interrupts used by cipher flows. */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* No additional work is required for other carry modes. */
        }
    
        /* Launch the configured HCU command. */
        HCU_StartEngine();
        /* Polling mode stays in the API until the command finishes. */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            s_hcuStatePtr->cmdInProgress = false;
        }
    }

    return status;
}

/*!
 * @brief Decrypt a payload with AES in CTR mode.
 */
status_t HCU_DRV_DecryptCTR(const void *cipherText, uint16_t length, const void *cv, void *plainText)
{
    status_t status;
    /* Prepare the command-specific engine and runtime context. */
    status = HCU_ConfigAlgorithm((const uint32_t *)cipherText, length, 0, plainText, ENG_AES, ALG_AES_CTR, MODE_DEC);/* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        /* Update the counter-value registers when the caller supplies them. */
        if (NULL != cv)
        {
            /* Preserve the current counter value when no replacement is provided. */
            HCU_SetCV((const uint32_t *)cv);/* PRQA S 0316 */
        }
    
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable the FIFO watermark and operation-done interrupts used by cipher flows. */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* No additional work is required for other carry modes. */
        }
    
        /* Launch the configured HCU command. */
        HCU_StartEngine();
        /* Polling mode stays in the API until the command finishes. */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            s_hcuStatePtr->cmdInProgress = false;
        }
    }

    return status;
}
#endif /* FEATURE_HCU_AES_CTR_ENGINE */

#if FEATURE_HCU_SM4_ENGINE
/*!
 * @brief Encrypt a payload with SM4 in ECB mode.
 */
status_t HCU_DRV_EncryptSM4ECB(const void *plainText, uint16_t length, void *cipherText)
{
    /* Validate that the driver state pointer is available. */
    DEV_ASSERT(s_hcuStatePtr != NULL);

    status_t status;
    /* Prepare the command-specific engine and runtime context. */
    status = HCU_ConfigAlgorithm((const uint32_t *)plainText, length, 0, cipherText, ENG_SM4, ALG_AES_ECB, MODE_ENC);/* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable the FIFO watermark and operation-done interrupts used by cipher flows. */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* No additional work is required for other carry modes. */
        }

        /* Launch the configured HCU command. */
        HCU_StartEngine();
        /* Polling mode stays in the API until the command finishes. */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            s_hcuStatePtr->cmdInProgress = false;
        }
    }

    return status;
}

/*!
 * @brief Decrypt a payload with SM4 in ECB mode.
 */
status_t HCU_DRV_DecryptSM4ECB(const void *cipherText, uint16_t length, void *plainText)
{
    status_t status;
    /* Prepare the command-specific engine and runtime context. */
    status = HCU_ConfigAlgorithm((const uint32_t *)cipherText, length, 0, plainText, ENG_SM4, ALG_AES_ECB, MODE_DEC);/* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable the FIFO watermark and operation-done interrupts used by cipher flows. */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* No additional work is required for other carry modes. */
        }
        /* Launch the configured HCU command. */
        HCU_StartEngine();
        /* Polling mode stays in the API until the command finishes. */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            s_hcuStatePtr->cmdInProgress = false;
        }
    }
    return status;
}
#endif /* FEATURE_HCU_SM4_ENGINE */

/*!
 * @brief Encrypt a payload with AES in CBC mode.
 */
status_t HCU_DRV_EncryptCBC(const void *plainText, uint16_t length, const void *iv, void *cipherText)
{
    status_t status;

    /* Prepare the command-specific engine and runtime context. */
    status = HCU_ConfigAlgorithm((const uint32_t *)plainText, length, 0, cipherText, ENG_AES, ALG_AES_CBC, MODE_ENC);/* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        /* Update the IV registers when the caller supplies them. */
        if (NULL != iv)
        {
            /* Preserve the current IV when no replacement is provided. */
            HCU_SetIV((const uint32_t *)iv);/* PRQA S 0316 */
        }
    
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable the FIFO watermark and operation-done interrupts used by cipher flows. */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else{
            /* No additional work is required for other carry modes. */
        }
    
        /* Launch the configured HCU command. */
        HCU_StartEngine();
        /* Polling mode stays in the API until the command finishes. */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            s_hcuStatePtr->cmdInProgress = false;
        }
    }

    return status;
}

/*!
 * @brief Decrypt a payload with AES in CBC mode.
 */
status_t HCU_DRV_DecryptCBC(const void *cipherText, uint16_t length, const void *iv, void *plainText)
{
    status_t status;

    /* Prepare the command-specific engine and runtime context. */
    status = HCU_ConfigAlgorithm((const uint32_t *)cipherText, length, 0, plainText, ENG_AES, ALG_AES_CBC, MODE_DEC);/* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        /* Update the IV registers when the caller supplies them. */
        if (NULL != iv)
        {
            /* Preserve the current IV when no replacement is provided. */
            HCU_SetIV((const uint32_t *)iv);/* PRQA S 0316 */
        }
    
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable the FIFO watermark and operation-done interrupts used by cipher flows. */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* No additional work is required for other carry modes. */
        }
    
        /* Launch the configured HCU command. */
        HCU_StartEngine();
        /* Polling mode stays in the API until the command finishes. */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            s_hcuStatePtr->cmdInProgress = false;
        }
    }

    return status;
}

/*!
 * @brief Generate a CMAC value with the AES engine.
 */
status_t HCU_DRV_GenerateMAC(const void *msg, uint16_t msgLen, hcu_msg_type_t msgType, hcu_cmac_config_t *cmacConfig)
{
    status_t status;
    uint8_t macPtr[16];
    uint8_t i;
    /* Prepare the command-specific engine and runtime context. */
    status = HCU_ConfigAlgorithm((const uint32_t *)msg, msgLen, 0, NULL, ENG_AES, ALG_AES_CMAC, MODE_ENC);/* PRQA S 0316 */
    if (STATUS_SUCCESS == status)
    {
        /* Keep the CMAC buffer description in the runtime state. */
        if (NULL != cmacConfig->macPtr)
        {
            s_hcuStatePtr->cmacConfig = cmacConfig;
        }
        /* Track whether this segment finishes the authenticated stream. */
        if((msgType == MSG_END) || (msgType == MSG_ALL))
        {
            s_hcuStatePtr->isLastBlock = true;
        }
        else
        {
            s_hcuStatePtr->isLastBlock = false;
        }
    
        /* Program the MAC length field before starting the operation. */
        HCU_SetMacLength(cmacConfig->macLen);
        /* Program the message-fragment type for this call. */
        HCU_SetMsgType(msgType);
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable the input-watermark and operation-done interrupts. */
            HCU_SetDoneInterrupt(true);
            HCU_SetInputInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            /* Keep the operation-done interrupt enabled for command completion. */
            HCU_SetDoneInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* No additional work is required for other carry modes. */
        }
        /* Launch the configured HCU command. */
        HCU_StartEngine();
        /* Polling mode stays in the API until the command finishes. */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            /* Read the generated CMAC bytes back into the caller buffer. */
            HCU_ReadMac(macPtr);
            for (i = 0; (NULL != cmacConfig->macPtr) && (i < cmacConfig->macLen); ++i)
            {
                cmacConfig->macPtr[i] = macPtr[i];
            }
            /* Finalize CMAC processing on the last segment. */
            if((msgType == MSG_END) || (msgType == MSG_ALL))
            {
                status = HCU_DRV_DoneMAC();
            }
            s_hcuStatePtr->cmdInProgress = false;
        }
    }

    return status;
}

/*!
 * @brief Authorize a CMAC value with the AES engine.
 */
status_t HCU_DRV_AuthorizeMAC(const void *msg, uint16_t msgLen, hcu_msg_type_t msgType, hcu_cmac_config_t *cmacConfig)
{
    status_t status;

    /* Prepare the command-specific engine and runtime context. */
    status = HCU_ConfigAlgorithm((const uint32_t *)msg, msgLen, 0, NULL, ENG_AES, ALG_AES_CMAC, MODE_DEC);/* PRQA S 0316 */
    if (STATUS_SUCCESS == status)
    {
        if (NULL != cmacConfig->macPtr)
        {
            /* Keep the CMAC buffer description in the runtime state. */
            s_hcuStatePtr->cmacConfig = cmacConfig;
            /* Set MAC value */
            HCU_SetMac(cmacConfig->macPtr, cmacConfig->macLen);
        }
        /* Track whether this segment finishes the authenticated stream. */
        if((msgType == MSG_END) || (msgType == MSG_ALL))
        {
            s_hcuStatePtr->isLastBlock = true;
        }
        else
        {
            s_hcuStatePtr->isLastBlock = false;
        }
        /* Program the MAC length field before starting the operation. */
        HCU_SetMacLength(cmacConfig->macLen);
        /* Program the message-fragment type for this call. */
        HCU_SetMsgType(msgType);
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable the input-watermark and operation-done interrupts. */
            HCU_SetDoneInterrupt(true);
            HCU_SetInputInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            /* Keep the operation-done interrupt enabled for command completion. */
            HCU_SetDoneInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* No additional work is required for other carry modes. */
        }
        /* Launch the configured HCU command. */
        HCU_StartEngine();
        /* Polling mode stays in the API until the command finishes. */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            /* Finalize CMAC authorization on the last segment. */
            if((msgType == MSG_END) || (msgType == MSG_ALL))
            {
                status = HCU_DRV_DoneMAC();
            }
            s_hcuStatePtr->cmdInProgress = false;
        }
    }

    return status;
}
#if FEATURE_HCU_SHA_ENGINE
/*!
 * @brief Generate a SHA digest.
 */
status_t HCU_DRV_GenerateSHA(const void *msg,
                             uint16_t msgLen,
                             uint32_t totalLen,
                             hcu_sha_type_t shaType,
                             hcu_msg_type_t msgType,
                             void *result)
{
    status_t status = STATUS_SUCCESS;
    if ((msgType == MSG_START) || (msgType == MSG_MIDDLE))
    {
        /* Segmented SHA-256 blocks must be non-zero multiples of 64 bytes. */
        if (HCU_SHA_256 == shaType)
        {
            if (((msgLen % HCU_SHA_256_BLOCK_SIZE) != 0u) || (msgLen == 0u))
            {
                status = STATUS_ERROR;
            }
        }
        /* Segmented SHA-384 blocks must be non-zero multiples of 128 bytes. */
        if (HCU_SHA_384 == shaType)
        {
            if (((msgLen % HCU_SHA_384_BLOCK_SIZE) != 0u) || (msgLen == 0u))
            {
                status = STATUS_ERROR;
            }
        }
    }
    if(STATUS_SUCCESS == status)
    {
        /* Prepare the command-specific engine and runtime context. */
        status = HCU_ConfigAlgorithm((const uint32_t *)msg, msgLen, msgLen, result, ENG_SHA, (hcu_alg_aes_mode_t)(uint8_t)shaType, MODE_ENC);/* PRQA S 0316, 0317 */
    }
    if (STATUS_SUCCESS == status)
    {
        /* Program the message-fragment type for this call. */
        HCU_SetMsgType(msgType);
        /* Program the total message length and clear SHA verification at the start. */
        if ((msgType == MSG_START) || (msgType == MSG_ALL))
        {
            HCU_SetSHAVerification(false);
            HCU_SetMsgTotalLength(totalLen);
            /* Updating SHA verification can assert the operation-done flag immediately. */
            HCU_ClearStatusFlag(OPERATION_DONE_FLAG);
        }
        /* Track whether this SHA segment closes the full message. */
        if ((msgType == MSG_END) || (msgType == MSG_ALL))
        {
            s_hcuStatePtr->isLastBlock = true;
        }else {
            s_hcuStatePtr->isLastBlock = false;
        }
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable the input-watermark and operation-done interrupts. */
            HCU_SetInputInterrupt(true);
            HCU_SetDoneInterrupt(true);
            if (NULL != result)
            {
                HCU_SetOutputInterrupt(true);
            }
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
    #if FEATURE_HCU_HAS_FIXED_DMA
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            /* Use the operation-done interrupt to complete the DMA-backed SHA flow. */
            HCU_SetDoneInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }
        else {
            /* No additional work is required for other carry modes. */
        }
    #endif
        /* Launch the configured HCU command. */
        HCU_StartEngine();
        /* Polling mode stays in the API until the command finishes. */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            s_hcuStatePtr->cmdInProgress = false;
        }
    }
    return status;
}

/*!
 * @brief Authorize a SHA digest.
 */
#if FEATURE_HCU_HAS_SHA_AUTHORIZE
status_t HCU_DRV_AuthorizeSHA(const void *msg,
                              uint16_t msgLen,
                              uint32_t totalLen,
                              hcu_sha_type_t shaType,
                              hcu_msg_type_t msgType,
                              void *result,
                              void *trueResult)
{
    status_t status = STATUS_SUCCESS;
    if ((msgType == MSG_START) || (msgType == MSG_MIDDLE))
    {
        /* Segmented SHA-256 blocks must be non-zero multiples of 64 bytes. */
        if (HCU_SHA_256 == shaType)
        {
            if (((msgLen % HCU_SHA_256_BLOCK_SIZE) != 0u) || (msgLen == 0u))
            {
                status = STATUS_ERROR;
            }
        }
        /* Segmented SHA-384 blocks must be non-zero multiples of 128 bytes. */
        if (HCU_SHA_384 == shaType)
        {
            if (((msgLen % HCU_SHA_384_BLOCK_SIZE) != 0u) || (msgLen == 0u))
            {
                status = STATUS_ERROR;
            }
        }
    }
    /* The authorize flow reloads the digest context before the first segment. */
    if((msgType == MSG_START) || (msgType == MSG_ALL))
    {
        /* Load the expected digest into the authorize context registers. */
        if (HCU_SHA_256 == shaType)
        {
            HCU_SetICV((const uint32_t *)result, HCU_SHA_256_LENGTH >> 2); /* PRQA S 0316 */
        }
        else
        {
            HCU_SetICV((const uint32_t *)result, HCU_SHA_384_LENGTH >> 2); /* PRQA S 0316 */
        }
    }
    if (STATUS_SUCCESS == status)
    {
        /* Prepare the command-specific engine and runtime context. */
        status = HCU_ConfigAlgorithm((const uint32_t *)msg, msgLen, 0u, trueResult, ENG_SHA, (hcu_alg_aes_mode_t)(uint8_t)shaType, MODE_ENC);/* PRQA S 0316, 0317 */
        s_hcuStatePtr->mode = MODE_DEC;
    }
    if (STATUS_SUCCESS == status)
    {
        /* Program the message-fragment type for this call. */
        HCU_SetMsgType(msgType);
        if(msgType == MSG_START)
        {
            /* Program the total message length on the first authorize segment. */
            HCU_SetMsgTotalLength(totalLen);
            /* Track whether this SHA segment closes the full message. */
            s_hcuStatePtr->isLastBlock = false;
        }else if(msgType == MSG_END)
        {
            /* Enable SHA verification when the final segment is reached. */
            HCU_SetSHAVerification(true);
            /* Track whether this SHA segment closes the full message. */
            s_hcuStatePtr->isLastBlock = true;
        }else if(msgType == MSG_ALL)
        {
            /* Program the total message length on the first authorize segment. */
            HCU_SetMsgTotalLength(totalLen);
            /* Enable SHA verification when the final segment is reached. */
            HCU_SetSHAVerification(true);
            /* Track whether this SHA segment closes the full message. */
            s_hcuStatePtr->isLastBlock = true;
        }else{
            /* Track whether this SHA segment closes the full message. */
            s_hcuStatePtr->isLastBlock = false;
        }
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable the input-watermark and operation-done interrupts. */
            HCU_SetInputInterrupt(true);
            HCU_SetDoneInterrupt(true);
            if (NULL != trueResult)
            {
                HCU_SetOutputInterrupt(true);
            }
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
    #if FEATURE_HCU_HAS_FIXED_DMA
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            /* Use the operation-done interrupt to complete the DMA-backed SHA flow. */
            HCU_SetDoneInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }
        else{
            /* No additional work is required for other carry modes. */
        }
    #endif
        /* Launch the configured HCU command. */
        HCU_StartEngine();
        /* Polling mode stays in the API until the command finishes. */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            if (true == HCU_GetStatusFlag(SHA_HASH_INVALID_FLAG))
            {
                status = STATUS_HCU_HASH_CHECK_ERROR;
                s_hcuStatePtr->status = STATUS_HCU_HASH_CHECK_ERROR;
            }
            else
            {
                s_hcuStatePtr->status = STATUS_SUCCESS;
            }
            s_hcuStatePtr->cmdInProgress = false;
        }
    }
    return status;
}
#endif /* FEATURE_HCU_HAS_SHA_AUTHORIZE */
#endif /* FEATURE_HCU_SHA_ENGINE */

#if FEATURE_HCU_HMAC_ENGINE
/*!
 * @brief Generate an HMAC digest.
 */
status_t HCU_DRV_GenerateHMAC(const void *msg,
                              uint16_t msgLen,
                              uint32_t totalLen,
                              hcu_sha_type_t shaType,
                              hcu_msg_type_t msgType,
                              void *result)
{
    return HCU_DRV_GenerateSHA(msg, msgLen, totalLen, shaType, msgType, result);
}

#if FEATURE_HCU_HAS_SHA_AUTHORIZE
/*!
 * @brief Authorize an HMAC digest.
 */
status_t HCU_DRV_AuthorizeHMAC(const void *msg,
                               uint16_t msgLen,
                               uint32_t totalLen,
                               hcu_sha_type_t shaType,
                               hcu_msg_type_t msgType,
                               void *result,
                               void *trueResult)
{
    return HCU_DRV_AuthorizeSHA(msg, msgLen, totalLen, shaType, msgType, result, trueResult);
}
#endif /* FEATURE_HCU_HAS_SHA_AUTHORIZE */
#endif /* FEATURE_HCU_HMAC_ENGINE */

#if FEATURE_HCU_AES_CCM_ENGINE
/*!
 * @brief Compute the CCM length-field width from the payload and nonce sizes.
 */
static uint8_t HCU_AESGetL(uint64_t PlainLen, uint8_t NonceLen)
{
    uint8_t L = 0;
    uint8_t nLen = NonceLen;
    uint64_t pLen = PlainLen;
    while (pLen != 0UL)
    {
        pLen >>= 8UL;
        ++L;
    }
    L = (L < 2u) ? 2u : L;
    /* Increase L as needed so the CCM nonce length and Q field remain valid. */
    nLen = (nLen > 13u) ? 13u : nLen;
    L = ((15u - nLen) > L) ? (15u - nLen) : L;
    return L;
}

/*!
 * @brief Format and push the CCM additional-authenticated-data header blocks.
 */
static status_t HCU_PushAdditionData(const uint8_t *Nonce,
                                     uint8_t NonceLen,
                                     const uint8_t *AddData,
                                     uint32_t AddDataLen,
                                     uint64_t PlainLen,
                                     uint8_t MacLen)
{
    uint32_t BWords[4];
    uint8_t * BBytesPtr = (uint8_t *)BWords;
    status_t status = STATUS_SUCCESS;

    uint8_t L;
    uint32_t i = 0; 
    uint32_t j = 0;
    uint16_t BLen;
    uint8_t x;
    uint64_t PlainLenTmp = PlainLen;
    /* Clear the temporary CCM formatting block. */
    for (i = 0; i < 4u; i++)
    {
        BWords[i] = 0;
    }
    /* Compute the number of bytes consumed by the encoded AAD payload. */
    BLen = (((uint16_t)NonceLen / 16u) + 1u) * 16u;
    if (AddDataLen > 0u)
    {
        if (AddDataLen < ((1UL << 16) - (1UL << 8)))
        {
            if (((AddDataLen + 2u) % 16u) == 0u)
            {
                BLen += ((uint16_t)AddDataLen + 2u);
            }
            else
            {
                BLen += (uint16_t)(((AddDataLen + 2u) / 16u) + 1u) * 16u;
            }
        }
        else
        {
            if (((AddDataLen + 6u) % 16u) == 0u)
            {
                BLen += ((uint16_t)AddDataLen + 6u);
            }
            else
            {
                BLen += (uint16_t)(((AddDataLen + 6u) / 16u) + 1u) * 16u;
            }
        }
    }
    HCU_SetMsgType(MSG_START);
    /* Program the CCM tag length. */
    HCU_SetMacLength(MacLen);
    /* Program the formatted CCM header length. */
    HCU_SetMsgLength(BLen, BLen);
    /* Build the CCM B0 block. */
    L = HCU_AESGetL(PlainLen, NonceLen);
    if ((L + NonceLen) != 15u)
    {
        status = STATUS_HCU_CCM_NONCE_DATA_SIZE_ERROR;
    }else 
    {
        /* Update the B0 size-encoding fields. */
        /* The first CCM counter block starts from one. */
        BBytesPtr[0] = L - 1u;
        /* Fill the nonce bytes and length field inside B0. */
        for (i = 0; i < NonceLen; i++)
        {
            BBytesPtr[i + 1u] = Nonce[i];
        }
        /* Push the initial CCM nonce block into the counter-value registers. */
        HCU_SetCV(BWords);
        /* Start the hardware so the formatted header can be absorbed. */
        HCU_StartEngine();
        /* Reflect whether additional authenticated data is present. */
        if(AddDataLen != 0u)
        {
            BBytesPtr[0] = 0x40u;
        }else {
            BBytesPtr[0] = 0x00u;
        }
        /* Encode the requested CCM tag size in the header flags. */
        if (MacLen == 0u)
        {
            BBytesPtr[0] |= (L - 1u);
        }
        else
        {
            BBytesPtr[0] |= (uint8_t)(((MacLen - 2u) / 2u) << 3u) | (L - 1u);
        }
        /* Emit the Q field using the computed CCM length width. */
        for (i = 0; i < L; i++)
        {
            BBytesPtr[15u - i] = (uint8_t)(PlainLenTmp & 0xffu);
            PlainLenTmp >>= 8u;
        }
        /* Write the prepared words into the HCU input FIFO. */
        HCU_WriteInputFifo(BWords, 4);
        /* Clear the temporary block before filling the next segment. */
        for (i = 0; i < 4u; i++)
        {
            BWords[i] = 0;
        }
        /* Emit the encoded AAD length field followed by the AAD payload. */
        if (AddDataLen > 0u)
        {
            /* Encode the AAD length prefix. */
            if (AddDataLen < ((1UL << 16) - (1UL << 8)))
            {
                BBytesPtr[0] = (uint8_t)(AddDataLen >> 8);
                BBytesPtr[1] = (uint8_t)(AddDataLen & 0xFFu);
                x = 2;
            }
            else
            {
                BBytesPtr[0] = 0xFF;
                BBytesPtr[1] = 0xFE;
                BBytesPtr[2] = (uint8_t)(AddDataLen >> 24);
                BBytesPtr[3] = (uint8_t)((AddDataLen >> 16) & 0xFFu);
                BBytesPtr[4] = (uint8_t)((AddDataLen >> 8) & 0xFFu);
                BBytesPtr[5] = (uint8_t)(AddDataLen & 0xFFu);
                x = 6;
            }
            /* Append the raw AAD bytes after the encoded length field. */
            for (i = x; i < (AddDataLen + x); i++)
            {
                BBytesPtr[i & 0xFu] = AddData[j];
                j++;
                /* Flush the block once one full 16-byte chunk has been assembled. */
                if ((i & 0xFu) == 0xFu)
                {
                    /* Write the prepared words into the HCU input FIFO. */
                    while (false == HCU_IsInputFifoEmpty())
                    {
                        /* Wait until the input FIFO can accept the next formatted block. */
                    }
                    /* Write the prepared words into the HCU input FIFO. */
                    HCU_WriteInputFifo(BWords, 4);
                    /* Clear the temporary block before filling the next segment. */
                    BWords[0] = 0;
                    BWords[1] = 0;
                    BWords[2] = 0;
                    BWords[3] = 0;
                }
            }
            /* Flush the final partial AAD block when bytes remain pending. */
            if ((i & 0xFu) != 0u)
            {
                /* Write the prepared words into the HCU input FIFO. */
                while (false == HCU_IsInputFifoEmpty())
                {
                    /* Wait until the input FIFO can accept the next formatted block. */
                }
                /* Write the prepared words into the HCU input FIFO. */
                HCU_WriteInputFifo((uint32_t *)BWords, 4);
            }
        }
        /* Wait until the hardware finishes absorbing the formatted CCM header. */
        while (HCU_IsBusy())
        {
            /* Wait until the hardware finishes absorbing the formatted CCM header. */
        }
        HCU_ClearStatusFlag(OPERATION_DONE_FLAG);
    }
    return status;
}

/*!
 * @brief Configure the CCM authenticated-data context.
 */
status_t HCU_DRV_CCMConfig(hcu_ccm_config_t *ccm, hcu_mode_sel_t mode)
{
    status_t status = STATUS_SUCCESS;

    /* Validate the CCM tag-size constraints before programming the context. */
    if ((ccm->tagSize > 16u) || ((ccm->tagSize & 1u) == 1u))
    {
        status = STATUS_HCU_CCM_TAG_SIZE_ERROR;
    }else 
    {    
        /* Update the runtime state for the new command. */
        s_hcuStatePtr->cmdInProgress = true;
        /* Program the engine, algorithm, and direction fields. */
        HCU_SetEngineAlgorithm(ENG_AES, ALG_AES_CCM, mode);
        /* Program the total CCM payload length. */
        HCU_SetMsgTotalLength((uint32_t)ccm->msgLen);
        /* Reset the FIFOs before applying the runtime watermarks. */
        HCU_ResetFifo();
        /* Program the FIFO watermarks used by the runtime loop. */
        HCU_SetFIFOWatermark(FEATURE_HCU_ONE_LOOP_INPUT_WATERMARK, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
        /* Load the expected authentication tag when preparing decrypt mode. */
        if (MODE_DEC == mode)
        {
            HCU_SetMac(ccm->tag, ccm->tagSize);
        }
        status = HCU_PushAdditionData(ccm->nonce, ccm->nonceSize, ccm->addData, ccm->addDataSize, ccm->msgLen, ccm->tagSize);
        if (STATUS_SUCCESS == status)
        {
            /* Keep the CCM configuration pointer for the payload stage. */
            s_hcuStatePtr->ccmConfig = ccm;
        }
    }
    return status;
}

/*!
 * @brief Encrypt a CCM payload segment.
 */
status_t HCU_DRV_EncryptCCM(const void *plainText, uint16_t length, void *cipherText, bool isLast)
{
    status_t status = STATUS_SUCCESS;
    /* Validate that both payload buffers are present. */
    DEV_ASSERT(plainText != NULL);
    DEV_ASSERT(cipherText != NULL);
    /* The HCU data path requires 32-bit aligned buffers. */
    DEV_ASSERT((((uint32_t)plainText) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    DEV_ASSERT((((uint32_t)cipherText) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    /* CCM payload segments must be multiples of 16 bytes. */
    DEV_ASSERT((length & HCU_BUFF_LEN_CHECK_MASK) == 0U);

    /* Reject the request if the hardware or driver is already busy. */
    if (HCU_IsBusy())
    {
        status = STATUS_BUSY;
    }
    else if (s_hcuStatePtr->ccmConfig == NULL)
    {
        status =  STATUS_HCU_CCM_NOT_CONFIGURED_ERROR;
    }else{
        /* Update the runtime state for the new command. */
        s_hcuStatePtr->cmdInProgress = true;
        /* Program the message-length fields for this transfer. */
        HCU_SetMsgLength(length, 0);
        /* Save the FIFO source, destination, and byte counters. */
        s_hcuStatePtr->mode = MODE_ENC;
        s_hcuStatePtr->msgLen = length;
        s_hcuStatePtr->inputCount = length;
        s_hcuStatePtr->outputCount = length;
        s_hcuStatePtr->dataInputPtr = (const uint32_t *)plainText;/* PRQA S 0316 */
        s_hcuStatePtr->dataOutputPtr = (uint32_t *)cipherText;/* PRQA S 0316 */
        s_hcuStatePtr->algorithm = AES_CCM_MODE;
        /* Reset the FIFOs before applying the runtime watermarks. */
        HCU_ResetFifo();
        /* Program the FIFO watermarks used by the runtime loop. */
        HCU_SetFIFOWatermark(FEATURE_HCU_ONE_LOOP_INPUT_WATERMARK, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
        /* Program the message-fragment type for this call. */
        if (isLast)
        {
            HCU_SetMsgType(MSG_END);
            s_hcuStatePtr->isLastBlock = true;
        }
        else
        {
            HCU_SetMsgType(MSG_MIDDLE);
            s_hcuStatePtr->isLastBlock = false;
        }
    
        /* Enable the runtime service path selected by the caller. */
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable the FIFO watermark and operation-done interrupts used by cipher flows. */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* No additional work is required for other carry modes. */
        }
    
        /* Start the configured CCM payload operation. */
        HCU_StartEngine();
        /* Polling mode stays in the API until the command finishes. */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            /* Keep servicing the FIFOs until the CCM payload segment completes. */
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            /* Update the runtime state for the new command. */
            s_hcuStatePtr->cmdInProgress = false;
            if (s_hcuStatePtr->isLastBlock)
            {
                status = HCU_DRV_DoneMAC();
            }
        }
    }

    return status;
}

/*!
 * @brief Decrypt a CCM payload segment.
 */
status_t HCU_DRV_DecryptCCM(const void *cipherText, uint16_t length, void *plainText, bool isLast)
{
    status_t status = STATUS_SUCCESS;
    /* Validate that both payload buffers are present. */
    DEV_ASSERT(plainText != NULL);
    DEV_ASSERT(cipherText != NULL);
    /* The HCU data path requires 32-bit aligned buffers. */
    DEV_ASSERT((((uint32_t)plainText) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    DEV_ASSERT((((uint32_t)cipherText) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    /* CCM payload segments must be multiples of 16 bytes. */
    DEV_ASSERT((length & HCU_BUFF_LEN_CHECK_MASK) == 0U);

    /* Reject the request if the hardware or driver is already busy. */
    if (HCU_IsBusy())
    {
        status = STATUS_BUSY;
    }
    else if (s_hcuStatePtr->ccmConfig == NULL)
    {
        status = STATUS_HCU_CCM_NOT_CONFIGURED_ERROR;
    }else{        
        /* Update the runtime state for the new command. */
        s_hcuStatePtr->cmdInProgress = true;
        /* Program the message-length fields for this transfer. */
        HCU_SetMsgLength(length, 0);
        /* Save the FIFO source, destination, and byte counters. */
        s_hcuStatePtr->mode = MODE_DEC;
        s_hcuStatePtr->msgLen = length;
        s_hcuStatePtr->inputCount = length;
        s_hcuStatePtr->outputCount = length;
        s_hcuStatePtr->dataInputPtr = (const uint32_t *)cipherText;/* PRQA S 0316 */
        s_hcuStatePtr->dataOutputPtr = (uint32_t *)plainText;/* PRQA S 0316 */
        s_hcuStatePtr->algorithm = AES_CCM_MODE;
        /* Reset the FIFOs before applying the runtime watermarks. */
        HCU_ResetFifo();
        /* Program the FIFO watermarks used by the runtime loop. */
        HCU_SetFIFOWatermark(FEATURE_HCU_ONE_LOOP_INPUT_WATERMARK, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
        /* Program the message-fragment type for this call. */
        if (isLast)
        {
            HCU_SetMsgType(MSG_END);
            s_hcuStatePtr->isLastBlock = true;
        }
        else
        {
            HCU_SetMsgType(MSG_MIDDLE);
            s_hcuStatePtr->isLastBlock = false;
        }
    
        /* Enable the runtime service path selected by the caller. */
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable the FIFO watermark and operation-done interrupts used by cipher flows. */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else{
            /* No additional work is required for other carry modes. */
        }
    
        /* Start the configured CCM payload operation. */
        HCU_StartEngine();
        /* Polling mode stays in the API until the command finishes. */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            /* Keep servicing the FIFOs until the CCM payload segment completes. */
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            /* Update the runtime state for the new command. */
            s_hcuStatePtr->cmdInProgress = false;
            if (s_hcuStatePtr->isLastBlock)
            {
                status = HCU_DRV_DoneMAC();
            }
        }
    }

    return status;
}
#endif /* FEATURE_HCU_AES_CCM_ENGINE */

/*!
 * @brief Service one polling iteration for the active HCU command.
 */
static status_t HCU_RunOneLoop(void)
{
    status_t status = STATUS_BUSY;
    /* Drain available output FIFO data before checking completion. */
    while (HCU_IsOutputFifoFull() && (NULL != s_hcuStatePtr->dataOutputPtr))
    {
        HCU_ReadOutputFifo(s_hcuStatePtr->dataOutputPtr, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
        s_hcuStatePtr->dataOutputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataOutputPtr + (FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2));
        s_hcuStatePtr->outputCount -= FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2;
    }
    /* Stop looping once the operation-done flag is asserted. */
    if (HCU_IsDone())
    {
        while (HCU_IsOutputFifoFull() && (NULL != s_hcuStatePtr->dataOutputPtr))
        {
            HCU_ReadOutputFifo(s_hcuStatePtr->dataOutputPtr, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
            s_hcuStatePtr->dataOutputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataOutputPtr + (FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2));
            s_hcuStatePtr->outputCount -= FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2;
        }
        HCU_ClearStatusFlag(OPERATION_DONE_FLAG);
        /* Report that the software loop finished the active command. */
        status = STATUS_SUCCESS;
    }else{
        /* Push more input data whenever the input watermark requests it. */
        if (HCU_IsInputFifoEmpty() && (s_hcuStatePtr->inputCount != 0U))
        {
            HCU_WriteInputFifo(s_hcuStatePtr->dataInputPtr, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
            s_hcuStatePtr->dataInputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataInputPtr + (FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2));
            /* Decrement the remaining byte counter by one FIFO burst. */
            s_hcuStatePtr->inputCount -= FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2;
        }
    }
    return status;
}

/*!
 * @brief Service HCU interrupt sources for asynchronous commands.
 */
void HCU_DRV_IRQHandler(void)
{
    bool statusFlag = false;
    bool intMode = false;
    /* Service the input FIFO watermark interrupt source. */
    statusFlag = HCU_GetStatusFlag(INPUT_FIFO_WATERMARK_FLAG);
    intMode = HCU_GetIntMode(INPUT_FIFO_WATERMARK_FLAG);
    if (statusFlag && intMode)
    {
        if (s_hcuStatePtr->inputCount != 0U)
        {
            HCU_WriteInputFifo(s_hcuStatePtr->dataInputPtr, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
            s_hcuStatePtr->dataInputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataInputPtr + (FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2));
            /* Decrement the remaining byte counter by one FIFO burst. */
            s_hcuStatePtr->inputCount -= FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2;
        }
        if (s_hcuStatePtr->inputCount == 0U)
        {
            /* Disable the watermark source only after the hardware accepts the update. */
            HCU_SetIntMode(INPUT_FIFO_WATERMARK_FLAG, false);
#if (FEATURE_HCU_HAS_FIXED_DMA < 1)
            /* Push padding words so the interrupt does not retrigger immediately. */
            HCU_WriteInputFifoPatch(FEATURE_HCU_ONE_LOOP_DATA_SIZE);
#endif
        }
    }

    /* Service the output FIFO watermark interrupt source. */
    statusFlag = HCU_GetStatusFlag(OUTPUT_FIFO_WATERMARK_FLAG);
    intMode = HCU_GetIntMode(OUTPUT_FIFO_WATERMARK_FLAG);
    if (statusFlag && intMode)
    {
        if ((s_hcuStatePtr->outputCount != 0U) && (NULL != s_hcuStatePtr->dataOutputPtr))
        {
            HCU_ReadOutputFifo(s_hcuStatePtr->dataOutputPtr, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
            s_hcuStatePtr->dataOutputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataOutputPtr + (FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2));
            /* Decrement the remaining byte counter by one FIFO burst. */
            s_hcuStatePtr->outputCount -= FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2;
        }
        if (s_hcuStatePtr->outputCount == 0U)
        {
            s_hcuStatePtr->cmdInProgress = false;
            HCU_SetIntMode(OUTPUT_FIFO_WATERMARK_FLAG, false);
            if (s_hcuStatePtr->callback != NULL)
            {
                s_hcuStatePtr->callback((uint32_t)STATUS_SUCCESS, s_hcuStatePtr->callbackParam);
            }
        }
    }

    /* Service the operation-done interrupt source. */
    statusFlag = HCU_GetStatusFlag(OPERATION_DONE_FLAG);
    intMode = HCU_GetIntMode(OPERATION_DONE_FLAG);
    if (statusFlag && intMode)
    {
        /* Clear the operation-done flag before leaving the handler. */
        HCU_ClearStatusFlag(OPERATION_DONE_FLAG);
        HCU_SetIntMode(OPERATION_DONE_FLAG, false);
        /* Stop further input-watermark interrupts for this command. */
        HCU_SetIntMode(INPUT_FIFO_WATERMARK_FLAG, false);
        /* CMAC completion is tracked through the done interrupt rather than output data. */
        if (AES_CMAC_MODE == s_hcuStatePtr->algorithm)
        {
            HCU_SetInputDMA(false);
            if (s_hcuStatePtr->isLastBlock)
            {
                (void)HCU_DRV_DoneMAC();
                if (s_hcuStatePtr->callback != NULL)
                {
                    s_hcuStatePtr->callback((uint32_t)s_hcuStatePtr->status, s_hcuStatePtr->callbackParam);
                }
            }
            s_hcuStatePtr->cmdInProgress = false;
        }
        /* CCM completion may need to copy or verify the authentication tag. */
        else if (AES_CCM_MODE == s_hcuStatePtr->algorithm)
        {
            if (s_hcuStatePtr->isLastBlock)
            {
                (void)HCU_DRV_DoneMAC();
            }
        }
        /* SHA completion can occur even when no output FIFO transfer is pending. */
        else if ((SHA_256_MODE == s_hcuStatePtr->algorithm) || (SHA_384_MODE == s_hcuStatePtr->algorithm))
        {
            HCU_SetInputDMA(false);
            if (s_hcuStatePtr->isLastBlock)
            {
                /* In authorize mode, inspect the hash-valid status before reporting success. */
                if (MODE_DEC == s_hcuStatePtr->mode)
                {
                    if (true == HCU_GetStatusFlag(SHA_HASH_INVALID_FLAG))
                    {
                        s_hcuStatePtr->status = STATUS_HCU_HASH_CHECK_ERROR;
                    }
                    else
                    {
                        s_hcuStatePtr->status = STATUS_SUCCESS;
                    }
                }
                if (s_hcuStatePtr->callback != NULL)
                {
                    s_hcuStatePtr->callback((uint32_t)s_hcuStatePtr->status, s_hcuStatePtr->callbackParam);
                }
            }
            /* When no output transfer is pending, the done interrupt can close the command. */
            if (NULL == s_hcuStatePtr->dataOutputPtr)
            {
                s_hcuStatePtr->cmdInProgress = false;
            }
        }else{
            /* No additional work is required for other carry modes. */
        }
    }
}

/******************************************************************************
 * EOF
 *****************************************************************************/
