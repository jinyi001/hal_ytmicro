/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file hcu_driver.c
 * @version 1.4.1
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

/* Pointer to runtime state structure.*/
static hcu_state_t *s_hcuStatePtr = NULL;

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/* Waits on the synchronization object and updates the internal flags */
static status_t HCU_ConfigAlgorithm(const uint32_t *dataIn,
                                    uint16_t msgLen,
                                    uint16_t exMsgLen,
                                    uint32_t *dataOut,
                                    hcu_engine_sel_t eng,
                                    hcu_alg_aes_mode_t alg,
                                    hcu_mode_sel_t mode);

static status_t HCU_RunOneLoop(void);

/* Callback for DMA complete */
static void HCU_DRV_CompleteDMA(void *parameter, dma_chn_status_t status);

/* DMA configuration */
static status_t HCU_DRV_ConfigDMA(uint8_t ingressDMAChannel, uint8_t egressDMAChannel);

/* When AES-CCM done in last block, call it */
static status_t HCU_DRV_DoneMAC(void);

#if 0
static void HCU_DRV_WaitCommandCompletion(uint32_t timeout);
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_Init
 * Description   : This function initializes the internal state of the driver
 * and enables the HCU interrupt.
 *
 * Implements    : HCU_DRV_Init_Activity
 * END**************************************************************************/
status_t HCU_DRV_Init(const hcu_user_config_t *userConfig, hcu_state_t *state)
{
    /* Check the driver is initialized */
    DEV_ASSERT(state != NULL);
    DEV_ASSERT(userConfig != NULL);

    status_t semaStatus;
    status_t retVal = STATUS_SUCCESS;

    /* Save the driver state structure */
    s_hcuStatePtr = state;

    /* Clear the contents of the state structure */
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

    /* New attribution for hcu state */
    s_hcuStatePtr->carryType = userConfig->carryType;
    s_hcuStatePtr->ingressDMAChannel = userConfig->ingressDMAChannel;
    s_hcuStatePtr->egressDMAChannel = userConfig->egressDMAChannel;

    HCU_DRV_CfgSwapping(userConfig->swap);

    /* Create the synchronization semaphore */
    semaStatus = OSIF_SemaCreate(&s_hcuStatePtr->cmdComplete, 0U);
    if (semaStatus == STATUS_ERROR)
    {
        retVal = STATUS_ERROR;
    }

    return retVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_DeInit
 * Description   : This function clears the internal state of the driver and
 * disables the HCU interrupt.
 *
 * Implements    : HCU_DRV_DeInit_Activity
 * END**************************************************************************/
status_t HCU_DRV_DeInit(hcu_state_t *state)
{
    status_t errorCode = STATUS_SUCCESS;
    /* Clear the contents of the state structure */
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
    /* Disable the interrupt */
    INT_SYS_DisableIRQ(HCU_IRQn);
    /* Clear the state pointer. */
    s_hcuStatePtr = NULL;
    HCU->CR = 0;
    HCU->INTE = 0;

    /* Destroy the semaphore */
    errorCode = OSIF_SemaDestroy(&(state->cmdComplete));
    DEV_ASSERT(errorCode == STATUS_SUCCESS);
    return errorCode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_CfgSwapping
 * Description   : This function cfg data swapping.
 *
 * Implements    : HCU_DRV_CfgSwapping_Activity
 * END**************************************************************************/
void HCU_DRV_CfgSwapping(hcu_swapping_t cfg)
{
    /* 2-bit cfg value check*/
    DEV_ASSERT(cfg < 4);

    uint32_t temp;
    temp = HCU->CR;
    temp &= ~HCU_CR_DATSWP_MASK;
    temp |= HCU_CR_DATSWP(cfg);
    HCU->CR = temp;
}

void HCU_DRV_ClearODFlag(void)
{
    /* clear OD flag in W1C method */
    HCU->SR = HCU_SR_OD_MASK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_InstallCallback
 * Description   : This function installs a user callback for the command
 * complete event.
 *
 * Implements    : HCU_DRV_InstallCallback_Activity
 * END**************************************************************************/
security_callback_t HCU_DRV_InstallCallback(security_callback_t callbackFunction, void *callbackParam)
{
    /* Check the driver is initialized */
    DEV_ASSERT(s_hcuStatePtr != NULL);

    security_callback_t currentCallback = s_hcuStatePtr->callback;
    s_hcuStatePtr->callback = callbackFunction;
    s_hcuStatePtr->callbackParam = callbackParam;

    return currentCallback;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_LoadUserKey
 * Description   : This function loads the user key into the HCU.
 *
 * Implements    : HCU_DRV_LoadUserKey_Activity
 * END**************************************************************************/
status_t HCU_DRV_LoadUserKey(const void *key, hcu_key_size_t keySize)
{
    /* Check the key addresses are 32 bit aligned */
    DEV_ASSERT((((uint32_t)key) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    /* Check the driver is initialized */
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
        /* Set key should always update when no command is in progress */
        if (s_hcuStatePtr->cmdInProgress)
        {
            retVal = STATUS_HCU_LOAD_KEY_WHEN_BUSY;
        }else
        {
            /* Write user key to HCU */
            for (keyLoopIndex = 0; keyLoopIndex < keySizeInWords; keyLoopIndex++)
            {
                HCU_SetUserKey(keyPtr[keyLoopIndex], keyLoopIndex);
            }
            /* Update Key Size */
            HCU_SetKeySize(keySize);
        }
    }
    return retVal;
}

#if FEATURE_HCU_HMAC_ENGINE
/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_LoadHMACKey
 * Description   : This function loads the user key into the HCU for HMAC.
 *
 * Implements    : HCU_DRV_LoadHMACKey_Activity
 * END**************************************************************************/
status_t HCU_DRV_LoadHMACKey(const void *key, hcu_hmac_key_size_t keySize)
{
    /* Check the key addresses are 32 bit aligned */
    DEV_ASSERT((((uint32_t)key) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    /* Check the driver is initialized */
    DEV_ASSERT(s_hcuStatePtr != NULL);
    uint8_t keySizeInWords;
    uint32_t *keyPtr = (uint32_t *)key;
    uint8_t keyLoopIndex;

    keySizeInWords = ((2 << keySize) >> 2);
    keySizeInWords = (keySizeInWords == 0) ? 1 : keySizeInWords;

    /* Set key should always update when no command is in progress */
    if (s_hcuStatePtr->cmdInProgress)
    {
        return STATUS_HCU_LOAD_KEY_WHEN_BUSY;
    }
    /* Enable HMAC engine */
    HCU_EnableHMAC(true);
    /* Write user key to HCU */
    for (keyLoopIndex = 0; keyLoopIndex < keySizeInWords; keyLoopIndex++)
    {
        HCU_SetUserKey(keyPtr[keyLoopIndex], keyLoopIndex);
    }
    /* Update Key Size */
    HCU_SetHMACKeySize(keySize);
    return STATUS_SUCCESS;
}
#endif /* FEATURE_HCU_HMAC_ENGINE */

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_ConfigAlgorithm
 * Description   : This function configures algorithm with given information
 *
 * Implements    : HCU_ConfigAlgorithm_Activity
 * END**************************************************************************/
static status_t HCU_ConfigAlgorithm(const uint32_t *dataIn,
                                    uint16_t msgLen,
                                    uint16_t exMsgLen,
                                    uint32_t *dataOut,
                                    hcu_engine_sel_t eng,
                                    hcu_alg_aes_mode_t alg,
                                    hcu_mode_sel_t mode)
{
    /* Check the buffer addresses are valid */
    DEV_ASSERT(dataIn != NULL);
    /* Check the buffers addresses are 32 bit aligned */
    DEV_ASSERT((((uint32_t)dataIn) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    DEV_ASSERT((((uint32_t)dataOut) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    status_t retVal = STATUS_SUCCESS;

    /* Check there is no other command in execution */
    if (HCU_IsBusy() || s_hcuStatePtr->cmdInProgress)
    {
        retVal = STATUS_BUSY;
    }else
    {
        /* Update the internal flags */
        s_hcuStatePtr->cmdInProgress = true;
        /* Update engine and algorithm settings */
        HCU_SetEngineAlgorithm(eng, alg, mode);
        /* Update message length */
        HCU_SetMsgLength(msgLen, exMsgLen);
        /* Save data information */
        s_hcuStatePtr->mode = mode;
        s_hcuStatePtr->msgLen = msgLen;
        s_hcuStatePtr->inputCount = msgLen;
        s_hcuStatePtr->outputCount = msgLen;
        s_hcuStatePtr->dataInputPtr = dataIn;
        s_hcuStatePtr->dataOutputPtr = dataOut;
        s_hcuStatePtr->algorithm = (hcu_alg_mode_t)(((uint16_t)alg + 1u) << (((uint16_t)eng - 1u) << 2u)); /* PRQA S 4394 */
#if FEATURE_HCU_SHA_ENGINE
        /* SHA output count is fixed */
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
        /* Config input and output FIFOs */
        HCU_SetFIFOWatermark(FEATURE_HCU_ONE_LOOP_INPUT_WATERMARK, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
    }

    return retVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_CompleteDMA
 * Description   : This function is called back for DMA done
 *
 * Implements    : HCU_DRV_CompleteDMA_Activity
 * END**************************************************************************/
static void HCU_DRV_CompleteDMA(void *parameter, dma_chn_status_t status)
{
    DEV_ASSERT(s_hcuStatePtr != NULL);
    uint32_t channel = (uint32_t)parameter; /* PRQA S 0326 */
    if (status == DMA_CHN_ERROR)
    {
        HCU_SetInputDMA(false);
        HCU_SetInputDMA(false);
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
            /* AES-CCM should check status in decrypt and copy MAC in encrypt */
            if ((AES_CCM_MODE == s_hcuStatePtr->algorithm) && (s_hcuStatePtr->isLastBlock))
            {
                (void)HCU_DRV_DoneMAC();
            }
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_ConfigDMA
 * Description   : This function config DMA for data carrying.
 *
 * Implements    : HCU_DRV_ConfigDMA_Activity
 * END**************************************************************************/
static status_t HCU_DRV_ConfigDMA(uint8_t ingressDMAChannel, uint8_t egressDMAChannel)
{
    DEV_ASSERT(s_hcuStatePtr != NULL);
    dma_transfer_size_t dmaTransferSize = DMA_TRANSFER_SIZE_4B;
    s_hcuStatePtr->ingressDMAChannel = ingressDMAChannel;
    s_hcuStatePtr->egressDMAChannel = egressDMAChannel;

    if (s_hcuStatePtr->dataOutputPtr != NULL)
    {
        /* DMA block size config by watermark, now is 4 words, 16 bytes */
        (void)DMA_DRV_ConfigMultiBlockTransfer(s_hcuStatePtr->egressDMAChannel,
                                               DMA_TRANSFER_PERIPH2MEM,
                                               (uint32_t)(&(HCU->OFDAT)),
                                               (uint32_t)s_hcuStatePtr->dataOutputPtr,
                                               dmaTransferSize,
                                               16U,
                                               (uint32_t)s_hcuStatePtr->outputCount / 16U,
                                               true);

        /* Combine ingress to completed event */
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
            /* DMA block size config by watermark, now is 4 words, 16 bytes */
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
            /* DMA block size config by watermark, now is 4 words, 16 bytes */
            (void)DMA_DRV_ConfigMultiBlockTransfer(s_hcuStatePtr->ingressDMAChannel,
                                                   DMA_TRANSFER_MEM2PERIPH,
                                                   (uint32_t)s_hcuStatePtr->dataInputPtr,
                                                   (uint32_t)(&(HCU->IFDAT)),
                                                   dmaTransferSize,
                                                   16U,
                                                   (uint32_t)s_hcuStatePtr->inputCount / 16U,
                                                   false);
        }
        /* Control address for out of range */
        DMA_DRV_SetSrcLastAddrAdjustment(s_hcuStatePtr->ingressDMAChannel, -s_hcuStatePtr->msgLen); /* PRQA S 3101, 4446 */
#endif
        /* Combine ingress to completed event */
        (void)DMA_DRV_InstallCallback(s_hcuStatePtr->ingressDMAChannel,
                                      (HCU_DRV_CompleteDMA),
                                      (void *)(uint32_t)(s_hcuStatePtr->ingressDMAChannel));/* PRQA S 0326 */
        (void)DMA_DRV_StartChannel(s_hcuStatePtr->ingressDMAChannel);
        HCU_SetInputDMA(true);
    }

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_DoneMAC
 * Description   : This function is called back when need MAC at last block.
 *
 * Implements    : HCU_DRV_DoneMAC_Activity
 * END**************************************************************************/
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
            /* Not Enter this statement */
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

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_EncryptECB
 * Description   : This function performs the AES-128 encryption in ECB mode of
 * the input plain text buffer.
 *
 * Implements    : HCU_DRV_EncryptECB_Activity
 * END**************************************************************************/
status_t HCU_DRV_EncryptECB(const void *plainText, uint16_t length, void *cipherText)
{
    status_t status;
    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm((const uint32_t *)plainText, length, 0, cipherText, ENG_AES, ALG_AES_ECB, MODE_ENC); /* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
    
        /* Start the command */
        HCU_StartEngine();
        /* Polling is blocking */
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

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_DecryptECB
 * Description   : This function performs the AES-128 decryption in ECB mode of
 * the input cipher text buffer.
 *
 * Implements    : HCU_DRV_DecryptECB_Activity
 * END**************************************************************************/
status_t HCU_DRV_DecryptECB(const void *cipherText, uint16_t length, void *plainText)
{
    status_t status;
    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm((const uint32_t *)cipherText, length, 0, plainText, ENG_AES, ALG_AES_ECB, MODE_DEC);/* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
    
        /* Start the command */
        HCU_StartEngine();
        /* Polling is blocking */
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
/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_EncryptCTR
 * Description   : This function performs the AES-128 encryption in CTR mode of
 * the input cipher text buffer.
 *
 * Implements    : HCU_DRV_EncryptCTR_Activity
 * END**************************************************************************/
status_t HCU_DRV_EncryptCTR(const void *plainText, uint16_t length, const void *cv, void *cipherText)
{
    status_t status;
    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm((const uint32_t *)plainText, length, 0, cipherText, ENG_AES, ALG_AES_CTR, MODE_ENC);/* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        /* Configure the CV */
        if (NULL != cv)
        {
            /* Only update when the CV is not NULL */
            HCU_SetCV((const uint32_t *)cv);/* PRQA S 0316 */
        }
    
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
    
        /* Start the command */
        HCU_StartEngine();
        /* Polling is blocking */
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

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_DecryptCTR
 * Description   : This function performs the AES-128 decryption in CTR mode of
 * the input cipher text buffer.
 *
 * Implements    : HCU_DRV_DecryptCTR_Activity
 * END**************************************************************************/
status_t HCU_DRV_DecryptCTR(const void *cipherText, uint16_t length, const void *cv, void *plainText)
{
    status_t status;
    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm((const uint32_t *)cipherText, length, 0, plainText, ENG_AES, ALG_AES_CTR, MODE_DEC);/* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        /* Configure the CV */
        if (NULL != cv)
        {
            /* Only update when the CV is not NULL */
            HCU_SetCV((const uint32_t *)cv);/* PRQA S 0316 */
        }
    
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
    
        /* Start the command */
        HCU_StartEngine();
        /* Polling is blocking */
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
/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_EncryptSM4ECB
 * Description   : This function performs the SM4-128 encryption in ECB mode of
 * the input plain text buffer.
 *
 * Implements    : HCU_DRV_EncryptSM4ECB_Activity
 * END**************************************************************************/
status_t HCU_DRV_EncryptSM4ECB(const void *plainText, uint16_t length, void *cipherText)
{
    /* Check the driver is initialized */
    DEV_ASSERT(s_hcuStatePtr != NULL);

    status_t status;
    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm((const uint32_t *)plainText, length, 0, cipherText, ENG_SM4, ALG_AES_ECB, MODE_ENC);/* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }

        /* Start the command */
        HCU_StartEngine();
        /* Polling is blocking */
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

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_DecryptSM4ECB
 * Description   : This function performs the SM4-128 decryption in ECB mode of
 * the input cipher text buffer.
 *
 * Implements    : HCU_DRV_DecryptSM4ECB_Activity
 * END**************************************************************************/
status_t HCU_DRV_DecryptSM4ECB(const void *cipherText, uint16_t length, void *plainText)
{
    status_t status;
    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm((const uint32_t *)cipherText, length, 0, plainText, ENG_SM4, ALG_AES_ECB, MODE_DEC);/* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
        /* Start the command */
        HCU_StartEngine();
        /* Polling is blocking */
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

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_EncryptCBC
 * Description   : This function performs the AES-128 encryption in CBC mode of
 * the input plain text buffer.
 *
 * Implements    : HCU_DRV_EncryptCBC_Activity
 * END**************************************************************************/
status_t HCU_DRV_EncryptCBC(const void *plainText, uint16_t length, const void *iv, void *cipherText)
{
    status_t status;

    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm((const uint32_t *)plainText, length, 0, cipherText, ENG_AES, ALG_AES_CBC, MODE_ENC);/* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        /* Configure the IV */
        if (NULL != iv)
        {
            /* Only update when the IV is not NULL */
            HCU_SetIV((const uint32_t *)iv);/* PRQA S 0316 */
        }
    
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else{
            /* Not Enter this statement */
        }
    
        /* Start the command */
        HCU_StartEngine();
        /* Polling is blocking */
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

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_DecryptCBC
 * Description   : This function performs the AES-128 decryption in CBC mode of
 * the input cipher text buffer.
 *
 * Implements    : HCU_DRV_DecryptCBC_Activity
 * END**************************************************************************/
status_t HCU_DRV_DecryptCBC(const void *cipherText, uint16_t length, const void *iv, void *plainText)
{
    status_t status;

    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm((const uint32_t *)cipherText, length, 0, plainText, ENG_AES, ALG_AES_CBC, MODE_DEC);/* PRQA S 0316, 0317 */
    if (STATUS_SUCCESS == status)
    {
        /* Configure the IV */
        if (NULL != iv)
        {
            /* Only update when the IV is not NULL */
            HCU_SetIV((const uint32_t *)iv);/* PRQA S 0316 */
        }
    
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
    
        /* Start the command */
        HCU_StartEngine();
        /* Polling is blocking */
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

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_GenerateMAC
 * Description   : This function calculates the MAC of a given message using CMAC
 * with AES-128.
 *
 * Implements    : HCU_DRV_GenerateMAC_Activity
 * END**************************************************************************/
status_t HCU_DRV_GenerateMAC(const void *msg, uint16_t msgLen, hcu_msg_type_t msgType, hcu_cmac_config_t *cmacConfig)
{
    status_t status;
    uint8_t macPtr[16];
    uint8_t i;
    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm((const uint32_t *)msg, msgLen, 0, NULL, ENG_AES, ALG_AES_CMAC, MODE_ENC);/* PRQA S 0316 */
    if (STATUS_SUCCESS == status)
    {
        /* Copy mac pointer and length */
        if (NULL != cmacConfig->macPtr)
        {
            s_hcuStatePtr->cmacConfig = cmacConfig;
        }
        /* Set isLastBlock at the end block */
        if((msgType == MSG_END) || (msgType == MSG_ALL))
        {
            s_hcuStatePtr->isLastBlock = true;
        }
        else
        {
            s_hcuStatePtr->isLastBlock = false;
        }
    
        /* Configure the MAC length */
        HCU_SetMacLength(cmacConfig->macLen);
        /* Configure the message type */
        HCU_SetMsgType(msgType);
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark and operate done */
            HCU_SetDoneInterrupt(true);
            HCU_SetInputInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            /* Enable operate done */
            HCU_SetDoneInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
        /* Start the command */
        HCU_StartEngine();
        /* Polling is blocking */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            /* Copy CMAC result to output buffer */
            HCU_ReadMac(macPtr);
            for (i = 0; (NULL != cmacConfig->macPtr) && (i < cmacConfig->macLen); ++i)
            {
                cmacConfig->macPtr[i] = macPtr[i];
            }
            /* Copy CMAC result at the end */
            if((msgType == MSG_END) || (msgType == MSG_ALL))
            {
                status = HCU_DRV_DoneMAC();
            }
            s_hcuStatePtr->cmdInProgress = false;
        }
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_AuthorizeMAC
 * Description   : This function authorize the MAC of a given message using CMAC
 * with AES-128.
 *
 * Implements    : HCU_DRV_AuthorizeMAC_Activity
 * END**************************************************************************/
status_t HCU_DRV_AuthorizeMAC(const void *msg, uint16_t msgLen, hcu_msg_type_t msgType, hcu_cmac_config_t *cmacConfig)
{
    status_t status;

    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm((const uint32_t *)msg, msgLen, 0, NULL, ENG_AES, ALG_AES_CMAC, MODE_DEC);/* PRQA S 0316 */
    if (STATUS_SUCCESS == status)
    {
        if (NULL != cmacConfig->macPtr)
        {
            /* Copy mac pointer and length */
            s_hcuStatePtr->cmacConfig = cmacConfig;
            /* Set MAC value */
            HCU_SetMac(cmacConfig->macPtr, cmacConfig->macLen);
        }
        /* Set isLastBlock at the end block */
        if((msgType == MSG_END) || (msgType == MSG_ALL))
        {
            s_hcuStatePtr->isLastBlock = true;
        }
        else
        {
            s_hcuStatePtr->isLastBlock = false;
        }
        /* Configure the MAC length */
        HCU_SetMacLength(cmacConfig->macLen);
        /* Configure the message type */
        HCU_SetMsgType(msgType);
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark and operate done */
            HCU_SetDoneInterrupt(true);
            HCU_SetInputInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            /* Enable operate done */
            HCU_SetDoneInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
        /* Start the command */
        HCU_StartEngine();
        /* Polling is blocking */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            /* Check CMAC result at the end */
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
/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_GenerateSHA
 * Description   : This function calculates the MAC of a given message using SHA
 *
 * Implements    : HCU_DRV_GenerateSHA_Activity
 * END**************************************************************************/
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
        /* If divide in several blocks, each block must at least 64 bytes */
        if (HCU_SHA_256 == shaType)
        {
            if (((msgLen % HCU_SHA_256_BLOCK_SIZE) != 0u) || (msgLen == 0u))
            {
                status = STATUS_ERROR;
            }
        }
        /* If divide in several blocks, each block must at least 128 bytes */
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
        /* Configure the algorithm */
        status = HCU_ConfigAlgorithm((const uint32_t *)msg, msgLen, msgLen, result, ENG_SHA, (hcu_alg_aes_mode_t)(uint8_t)shaType, MODE_ENC);/* PRQA S 0316, 0317 */
    }
    if (STATUS_SUCCESS == status)
    {
        /* Configure the message type */
        HCU_SetMsgType(msgType);
        /* Configure the total length of message at the beginning and disable sha-verify*/
        if ((msgType == MSG_START) || (msgType == MSG_ALL))
        {
            HCU_SetSHAVerification(false);
            HCU_SetMsgTotalLength(totalLen);
            /* When change sha-verify, OD will immediately set */
            HCU_ClearStatusFlag(OPERATION_DONE_FLAG);
        }
        /* Set if last block */
        if ((msgType == MSG_END) || (msgType == MSG_ALL))
        {
            s_hcuStatePtr->isLastBlock = true;
        }else {
            s_hcuStatePtr->isLastBlock = false;
        }
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark and operate done */
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
            /* Check by operate done flag */
            HCU_SetDoneInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }
        else {
            /* Not Enter this statement */
        }
    #endif
        /* Start the command */
        HCU_StartEngine();
        /* Polling is blocking */
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

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_AuthorizeSHA
 * Description   : This function verify the MAC of a given message using SHA
 *
 * Implements    : HCU_DRV_AuthorizeSHA_Activity
 * END**************************************************************************/
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
        /* If divide in several blocks, each block must at least 64 bytes */
        if (HCU_SHA_256 == shaType)
        {
            if (((msgLen % HCU_SHA_256_BLOCK_SIZE) != 0u) || (msgLen == 0u))
            {
                status = STATUS_ERROR;
            }
        }
        /* If divide in several blocks, each block must at least 128 bytes */
        if (HCU_SHA_384 == shaType)
        {
            if (((msgLen % HCU_SHA_384_BLOCK_SIZE) != 0u) || (msgLen == 0u))
            {
                status = STATUS_ERROR;
            }
        }
    }
    /* SHA authorize need reset HCU at first */
    if((msgType == MSG_START) || (msgType == MSG_ALL))
    {
        /* Write hash value to SHAICV, load at first */
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
        /* Configure the algorithm */
        status = HCU_ConfigAlgorithm((const uint32_t *)msg, msgLen, 0u, trueResult, ENG_SHA, (hcu_alg_aes_mode_t)(uint8_t)shaType, MODE_ENC);/* PRQA S 0316, 0317 */
        s_hcuStatePtr->mode = MODE_DEC;
    }
    if (STATUS_SUCCESS == status)
    {
        /* Configure the message type */
        HCU_SetMsgType(msgType);
        if(msgType == MSG_START)
        {
            /* Configure the total length of message at the beginning*/    
            HCU_SetMsgTotalLength(totalLen);
            /* Set if last block */
            s_hcuStatePtr->isLastBlock = false;
        }else if(msgType == MSG_END)
        {
            /* Start SHA at ending block */
            HCU_SetSHAVerification(true);
            /* Set if last block */
            s_hcuStatePtr->isLastBlock = true;
        }else if(msgType == MSG_ALL)
        {
            /* Configure the total length of message at the beginning*/    
            HCU_SetMsgTotalLength(totalLen);
            /* Start SHA at ending block */
            HCU_SetSHAVerification(true);
            /* Set if last block */
            s_hcuStatePtr->isLastBlock = true;
        }else{
            /* Set if last block */
            s_hcuStatePtr->isLastBlock = false;
        }
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark and operate done */
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
            /* Check by operate done flag */
            HCU_SetDoneInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }
        else{
            /* Not Enter this statement */
        }
    #endif
        /* Start the command */
        HCU_StartEngine();
        /* Polling is blocking */
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
/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_GenerateHMAC
 * Description   : This function calculates the MAC of a given message using HMAC
 *
 * Implements    : HCU_DRV_GenerateHMAC_Activity
 * END**************************************************************************/
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
/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_AuthorizeHMAC
 * Description   : This function verify the MAC of a given message using HMAC
 *
 * Implements    : HCU_DRV_AuthorizeHMAC_Activity
 * END**************************************************************************/
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
    /* Increase L to match the nonce len */
    nLen = (nLen > 13u) ? 13u : nLen;
    L = ((15u - nLen) > L) ? (15u - nLen) : L;
    return L;
}

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
    /* initialize B */
    for (i = 0; i < 4u; i++)
    {
        BWords[i] = 0;
    }
    /* Calculate Add data length */
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
    /* Set the MAC length */
    HCU_SetMacLength(MacLen);
    /* Set message length */
    HCU_SetMsgLength(BLen, BLen);
    /*B0*/
    L = HCU_AESGetL(PlainLen, NonceLen);
    if ((L + NonceLen) != 15u)
    {
        status = STATUS_HCU_CCM_NONCE_DATA_SIZE_ERROR;
    }else 
    {
        /* Update Data size configuration */
        /* Count value start at 1 */
        BBytesPtr[0] = L - 1u;
        /*B1-B15: Nonce & Q*/
        for (i = 0; i < NonceLen; i++)
        {
            BBytesPtr[i + 1u] = Nonce[i];
        }
        /* Update Nonce */
        HCU_SetCV(BWords);
        /* Start engine */
        HCU_StartEngine();
        /* Update additional data flag */
        if(AddDataLen != 0u)
        {
            BBytesPtr[0] = 0x40u;
        }else {
            BBytesPtr[0] = 0x00u;
        }
        /* Update Mac length */
        if (MacLen == 0u)
        {
            BBytesPtr[0] |= (L - 1u);
        }
        else
        {
            BBytesPtr[0] |= (uint8_t)(((MacLen - 2u) / 2u) << 3u) | (L - 1u);
        }
        /* L = (L > 4) ? 4 : L */
        for (i = 0; i < L; i++)
        {
            BBytesPtr[15u - i] = (uint8_t)(PlainLenTmp & 0xffu);
            PlainLenTmp >>= 8u;
        }
        /* Push data to FIFO */
        HCU_WriteInputFifo(BWords, 4);
        /* reinitialize B */
        for (i = 0; i < 4u; i++)
        {
            BWords[i] = 0;
        }
        /*aad length and data*/
        if (AddDataLen > 0u)
        {
            /* store length */
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
            /* x bytes length of addData_ori */
            for (i = x; i < (AddDataLen + x); i++)
            {
                BBytesPtr[i & 0xFu] = AddData[j];
                j++;
                /* Check if reaches one block */
                if ((i & 0xFu) == 0xFu)
                {
                    /* Push data to FIFO */
                    while (false == HCU_IsInputFifoEmpty())
                    {
                        /* Wait for input fifo */
                    }
                    /* Push data to FIFO */
                    HCU_WriteInputFifo(BWords, 4);
                    /* reinitialize B */
                    BWords[0] = 0;
                    BWords[1] = 0;
                    BWords[2] = 0;
                    BWords[3] = 0;
                }
            }
            /* Check if we need to push the last block */
            if ((i & 0xFu) != 0u)
            {
                /* Push data to FIFO */
                while (false == HCU_IsInputFifoEmpty())
                {
                    /* Wait for input fifo */
                }
                /* Push data to FIFO */
                HCU_WriteInputFifo((uint32_t *)BWords, 4);
            }
        }
        /* Wait for engine to finish */
        while (HCU_IsBusy())
        {
            /* Wait for engine to finish */
        }
        HCU_ClearStatusFlag(OPERATION_DONE_FLAG);
    }
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_CCMConfig
 * Description   : This function prepares the AES-128 CCM configuration.
 * with AES-128.
 *
 * Implements    : HCU_DRV_CCMConfig_Activity
 * END**************************************************************************/
status_t HCU_DRV_CCMConfig(hcu_ccm_config_t *ccm, hcu_mode_sel_t mode)
{
    status_t status = STATUS_SUCCESS;

    /* Check the input parameters */
    if ((ccm->tagSize > 16u) || ((ccm->tagSize & 1u) == 1u))
    {
        status = STATUS_HCU_CCM_TAG_SIZE_ERROR;
    }else 
    {    
        /* Update the internal flags */
        s_hcuStatePtr->cmdInProgress = true;
        /* Update engine and algorithm settings */
        HCU_SetEngineAlgorithm(ENG_AES, ALG_AES_CCM, mode);
        /* Set total message length */
        HCU_SetMsgTotalLength((uint32_t)ccm->msgLen);
        /* Reset fifo before set watermark */
        HCU_ResetFifo();
        /* Config input and output FIFOs */
        HCU_SetFIFOWatermark(FEATURE_HCU_ONE_LOOP_INPUT_WATERMARK, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
        /* Update tag information if mode is decrypt */
        if (MODE_DEC == mode)
        {
            HCU_SetMac(ccm->tag, ccm->tagSize);
        }
        status = HCU_PushAdditionData(ccm->nonce, ccm->nonceSize, ccm->addData, ccm->addDataSize, ccm->msgLen, ccm->tagSize);
        if (STATUS_SUCCESS == status)
        {
            /* Update ccm configuration */
            s_hcuStatePtr->ccmConfig = ccm;
        }
    }
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_EncryptCCM
 * Description   : This function performs the AES-128 encryption in CCM mode of
 * the input cipher text buffer.
 *
 * Implements    : HCU_DRV_EncryptCCM_Activity
 * END**************************************************************************/
status_t HCU_DRV_EncryptCCM(const void *plainText, uint16_t length, void *cipherText, bool isLast)
{
    status_t status = STATUS_SUCCESS;
    /* Check the buffer addresses are valid */
    DEV_ASSERT(plainText != NULL);
    DEV_ASSERT(cipherText != NULL);
    /* Check the buffers addresses are 32 bit aligned */
    DEV_ASSERT((((uint32_t)plainText) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    DEV_ASSERT((((uint32_t)cipherText) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    /* Check the buffer length is multiple of 16 bytes */
    DEV_ASSERT((length & HCU_BUFF_LEN_CHECK_MASK) == 0U);

    /* Check there is no other command in execution */
    if (HCU_IsBusy())
    {
        status = STATUS_BUSY;
    }
    else if (s_hcuStatePtr->ccmConfig == NULL)
    {
        status =  STATUS_HCU_CCM_NOT_CONFIGURED_ERROR;
    }else{
        /* Update the internal flags */
        s_hcuStatePtr->cmdInProgress = true;
        /* Update message length */
        HCU_SetMsgLength(length, 0);
        /* Save data information */
        s_hcuStatePtr->mode = MODE_ENC;
        s_hcuStatePtr->msgLen = length;
        s_hcuStatePtr->inputCount = length;
        s_hcuStatePtr->outputCount = length;
        s_hcuStatePtr->dataInputPtr = (const uint32_t *)plainText;/* PRQA S 0316 */
        s_hcuStatePtr->dataOutputPtr = (uint32_t *)cipherText;/* PRQA S 0316 */
        s_hcuStatePtr->algorithm = AES_CCM_MODE;
        /* Reset fifo before set watermark */
        HCU_ResetFifo();
        /* Config input and output FIFOs */
        HCU_SetFIFOWatermark(FEATURE_HCU_ONE_LOOP_INPUT_WATERMARK, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
        /* Configure the message type */
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
    
        /* Check carry type */
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
    
        /* Start the engine */
        HCU_StartEngine();
        /* Polling is blocking */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            /* Start to run loops */
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            /* Update the internal flags */
            s_hcuStatePtr->cmdInProgress = false;
            if (s_hcuStatePtr->isLastBlock)
            {
                status = HCU_DRV_DoneMAC();
            }
        }
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_DecryptCCM
 * Description   : This function performs the AES-128 decryption in CCM mode of
 * the input cipher text buffer.
 *
 * Implements    : HCU_DRV_DecryptCCM_Activity
 * END**************************************************************************/
status_t HCU_DRV_DecryptCCM(const void *cipherText, uint16_t length, void *plainText, bool isLast)
{
    status_t status = STATUS_SUCCESS;
    /* Check the buffer addresses are valid */
    DEV_ASSERT(plainText != NULL);
    DEV_ASSERT(cipherText != NULL);
    /* Check the buffers addresses are 32 bit aligned */
    DEV_ASSERT((((uint32_t)plainText) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    DEV_ASSERT((((uint32_t)cipherText) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    /* Check the buffer length is multiple of 16 bytes */
    DEV_ASSERT((length & HCU_BUFF_LEN_CHECK_MASK) == 0U);

    /* Check there is no other command in execution */
    if (HCU_IsBusy())
    {
        status = STATUS_BUSY;
    }
    else if (s_hcuStatePtr->ccmConfig == NULL)
    {
        status = STATUS_HCU_CCM_NOT_CONFIGURED_ERROR;
    }else{        
        /* Update the internal flags */
        s_hcuStatePtr->cmdInProgress = true;
        /* Update message length */
        HCU_SetMsgLength(length, 0);
        /* Save data information */
        s_hcuStatePtr->mode = MODE_DEC;
        s_hcuStatePtr->msgLen = length;
        s_hcuStatePtr->inputCount = length;
        s_hcuStatePtr->outputCount = length;
        s_hcuStatePtr->dataInputPtr = (const uint32_t *)cipherText;/* PRQA S 0316 */
        s_hcuStatePtr->dataOutputPtr = (uint32_t *)plainText;/* PRQA S 0316 */
        s_hcuStatePtr->algorithm = AES_CCM_MODE;
        /* Reset fifo before set watermark */
        HCU_ResetFifo();
        /* Config input and output FIFOs */
        HCU_SetFIFOWatermark(FEATURE_HCU_ONE_LOOP_INPUT_WATERMARK, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
        /* Configure the message type */
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
    
        /* Check carry type */
        if (HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if (HCU_USING_DMA == s_hcuStatePtr->carryType)
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else{
            /* Not Enter this statement */
        }
    
        /* Start the engine */
        HCU_StartEngine();
        /* Polling is blocking */
        if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
        {
            /* Start to run loops */
            do
            {
                status = HCU_RunOneLoop();
            } while (STATUS_BUSY == status);
            /* Update the internal flags */
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

static status_t HCU_RunOneLoop(void)
{
    status_t status = STATUS_BUSY;
    /* Check if hcu need to read data */
    while (HCU_IsOutputFifoFull() && (NULL != s_hcuStatePtr->dataOutputPtr))
    {
        HCU_ReadOutputFifo(s_hcuStatePtr->dataOutputPtr, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
        s_hcuStatePtr->dataOutputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataOutputPtr + (FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2));
        s_hcuStatePtr->outputCount -= FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2;
    }
    /* Check if Operation is already done */
    if (HCU_IsDone())
    {
        while (HCU_IsOutputFifoFull() && (NULL != s_hcuStatePtr->dataOutputPtr))
        {
            HCU_ReadOutputFifo(s_hcuStatePtr->dataOutputPtr, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
            s_hcuStatePtr->dataOutputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataOutputPtr + (FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2));
            s_hcuStatePtr->outputCount -= FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2;
        }
        HCU_ClearStatusFlag(OPERATION_DONE_FLAG);
        /* No more data to process */
        status = STATUS_SUCCESS;
    }else{
        /* Check if hcu need to write data */
        if (HCU_IsInputFifoEmpty() && (s_hcuStatePtr->inputCount != 0U))
        {
            HCU_WriteInputFifo(s_hcuStatePtr->dataInputPtr, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
            s_hcuStatePtr->dataInputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataInputPtr + (FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2));
            /* Convert the message length to be processed */
            s_hcuStatePtr->inputCount -= FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2;
        }
    }
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_IRQHandler
 * Description   : Implementation of the HCU interrupt handler. Handles completed
 * command events.
 *
 * END**************************************************************************/
void HCU_DRV_IRQHandler(void)
{
    bool statusFlag = false;
    bool intMode = false;
    /* Input fifo watermark handler */
    statusFlag = HCU_GetStatusFlag(INPUT_FIFO_WATERMARK_FLAG);
    intMode = HCU_GetIntMode(INPUT_FIFO_WATERMARK_FLAG);
    if (statusFlag && intMode)
    {
        if (s_hcuStatePtr->inputCount != 0U)
        {
            HCU_WriteInputFifo(s_hcuStatePtr->dataInputPtr, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
            s_hcuStatePtr->dataInputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataInputPtr + (FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2));
            /* Convert the message length to be processed */
            s_hcuStatePtr->inputCount -= FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2;
        }
        if (s_hcuStatePtr->inputCount == 0U)
        {
            /* When HCU busy, INTE can not be changed */
            HCU_SetIntMode(INPUT_FIFO_WATERMARK_FLAG, false);
#if (FEATURE_HCU_HAS_FIXED_DMA < 1)
            /* Write empty data to avoid reentry interrupt */
            HCU_WriteInputFifoPatch(FEATURE_HCU_ONE_LOOP_DATA_SIZE);
#endif
        }
    }

    /* Output fifo watermark handler */
    statusFlag = HCU_GetStatusFlag(OUTPUT_FIFO_WATERMARK_FLAG);
    intMode = HCU_GetIntMode(OUTPUT_FIFO_WATERMARK_FLAG);
    if (statusFlag && intMode)
    {
        if ((s_hcuStatePtr->outputCount != 0U) && (NULL != s_hcuStatePtr->dataOutputPtr))
        {
            HCU_ReadOutputFifo(s_hcuStatePtr->dataOutputPtr, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
            s_hcuStatePtr->dataOutputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataOutputPtr + (FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2));
            /* Convert the message length to be processed */
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

    /* Operate done handler */
    statusFlag = HCU_GetStatusFlag(OPERATION_DONE_FLAG);
    intMode = HCU_GetIntMode(OPERATION_DONE_FLAG);
    if (statusFlag && intMode)
    {
        /* Clear operate done flag */
        HCU_ClearStatusFlag(OPERATION_DONE_FLAG);
        HCU_SetIntMode(OPERATION_DONE_FLAG, false);
        /* Disable input watermark interrupt */
        HCU_SetIntMode(INPUT_FIFO_WATERMARK_FLAG, false);
        /* CMAC has no output to indicate ending */
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
        /* AES-CCM should check status in decrypt and copy MAC in encrypt */
        else if (AES_CCM_MODE == s_hcuStatePtr->algorithm)
        {
            if (s_hcuStatePtr->isLastBlock)
            {
                (void)HCU_DRV_DoneMAC();
            }
        }
        /* SHA may have no output to indicate ending */
        else if ((SHA_256_MODE == s_hcuStatePtr->algorithm) || (SHA_384_MODE == s_hcuStatePtr->algorithm))
        {
            HCU_SetInputDMA(false);
            if (s_hcuStatePtr->isLastBlock)
            {
                /* When in authorize mode, check hash-valid */
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
            /* Only output enable, wait until data carry done */
            if (NULL == s_hcuStatePtr->dataOutputPtr)
            {
                s_hcuStatePtr->cmdInProgress = false;
            }
        }else{
            /* Not Enter this statement */
        }
    }
}

/******************************************************************************
 * EOF
 *****************************************************************************/
