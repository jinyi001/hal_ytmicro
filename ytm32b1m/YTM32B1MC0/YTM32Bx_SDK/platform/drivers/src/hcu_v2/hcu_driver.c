/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*!
 * @file hcu_driver.c
 * @version 1.4.0
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 4394 Rule 10.8: A composite expression of 'essentially unsigned' type (unsigned short) 
 *                       is being cast to a different type category, 'enum'.
 * PRQA S 0316 Rule 11.5: Cast from a pointer to void to a pointer to object type.
 * PRQA S 0326 Rule 11.6: Cast between a pointer to void and an integral type.
 */

#include "hcu_driver.h"
#include "hcu_hw_access.h"
#include "flash_driver.h"
#include "interrupt_manager.h"
#include "core_common.h"
#include "status.h" 

/*******************************************************************************
 * Variables
 ******************************************************************************/
#define HCU_WAIT_TIMEOUT    (1000u)
/* Pointer to runtime state structure.*/
static hcu_state_t *s_hcuStatePtr = NULL;

/* Save IV ,CV and MAC temp value */
static uint32_t s_FutileData[8];

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/* Waits on the synchronization object and updates the internal flags */
static status_t HCU_ConfigAlgorithm(const uint32_t *dataIn, uint16_t msgLen, uint16_t exMsgLen,
                                    uint32_t *dataOut, hcu_engine_sel_t eng,
                                    hcu_alg_aes_mode_t alg, hcu_mode_sel_t mode);

static status_t HCU_RunOneLoop(void);

/* Callback for DMA complete */
static void HCU_DRV_CompleteDMA(void *parameter, dma_chn_status_t status);

/* DMA configuration */
static status_t HCU_DRV_ConfigDMA(uint8_t ingressDMAChannel, uint8_t egressDMAChannel);

/* When AES-CCM done in last block, call it */
static status_t HCU_DRV_DoneMAC(void);

#if FEATURE_HCU_AES_CCM_ENGINE
/* Push addition data for AES-CCM */
static status_t HCU_DRV_PushAdditionData(aes_algorithm_config_t * aesAlgCfg, hcu_ccm_config_t *ccmConfig, hcu_mode_sel_t mode);
#endif /* FEATURE_HCU_AES_CCM_ENGINE */

/*  */

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
status_t HCU_DRV_Init(const hcu_user_config_t * userConfig, hcu_state_t *state)
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
    s_hcuStatePtr->futileData = s_FutileData;

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
    s_hcuStatePtr->futileData = NULL;
    /* Disable the interrupt */
    INT_SYS_DisableIRQ(HCU_IRQn);
    /* Clear the state pointer. */
    s_hcuStatePtr = NULL;
    HCU->CR = 0;
    HCU->INTE = 0;
    HCU_ClearStatusFlag(OPERATION_DONE_FLAG);

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
    temp &=~ HCU_CR_DATSWP_MASK;
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

static status_t HCU_DRV_LoadUserKey(const void *key, hcu_key_size_t keySize)
{
    /* Check the key addresses are 32 bit aligned */
    DEV_ASSERT((((uint32_t) key) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    /* Check the driver is initialized */
    DEV_ASSERT(s_hcuStatePtr != NULL);
    uint8_t keySizeInWords;
    const uint32_t *keyPtr = (const uint32_t *) key;    /* PRQA S 0316 */
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
        /* Write user key to HCU */
        for (keyLoopIndex = 0; keyLoopIndex < keySizeInWords; keyLoopIndex++)
        {
            HCU_SetUserKey(keyPtr[keyLoopIndex], keyLoopIndex);
        }
    }
    return retVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_ConfigAlgorithm
 * Description   : This function configures algorithm with given information
 *
 * Implements    : HCU_ConfigAlgorithm_Activity
 * END**************************************************************************/
static status_t HCU_ConfigAlgorithm(const uint32_t *dataIn, uint16_t msgLen, uint16_t exMsgLen,
                                    uint32_t *dataOut, hcu_engine_sel_t eng,
                                    hcu_alg_aes_mode_t alg, hcu_mode_sel_t mode)
{
    /* Check the buffer addresses are valid */
    DEV_ASSERT(dataIn != NULL);
    /* Check the buffers addresses are 32 bit aligned */
    DEV_ASSERT((((uint32_t) dataIn) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    DEV_ASSERT((((uint32_t) dataOut) & HCU_BUFF_ADDR_CHECK_MASK) == 0U);
    status_t retVal = STATUS_SUCCESS;

    /* Check there is no other command in execution */
    if (HCU_IsBusy() || s_hcuStatePtr->cmdInProgress)
    {
        retVal = STATUS_BUSY;
    }else{
        /* Update the internal flags */
        s_hcuStatePtr->cmdInProgress = true;
        /* Update message length */
        HCU_SetMsgLength(msgLen, exMsgLen);
        /* Update engine and algorithm settings */
        HCU_SetEngineAlgorithm(eng, alg, mode);
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
#if FEATURE_HCU_HAS_SHA_AUTHORIZE
            HCU_SetSHAVerification(false);
#endif /* FEATURE_HCU_HAS_SHA_AUTHORIZE */
        }
#endif /* FEATURE_HCU_SHA_ENGINE */
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
    uint32_t channel = (uint32_t) parameter;    /* PRQA S 0326 */
    uint8_t srCount = 0;
    if (status == DMA_CHN_ERROR)
    {
        HCU_SetInputDMA(false);
        HCU_SetInputDMA(false);
    } else
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
            switch (s_hcuStatePtr->algorithm)
            {    
                /* If AES-CBC, need save IV */
                case AES_CBC_MODE:
                    HCU_ReadIV(s_hcuStatePtr->futileData);
                    break;
#if FEATURE_HCU_AES_CTR_ENGINE
                /* If AES-CTR, need save CV */
                case AES_CTR_MODE:
                    HCU_ReadCV(&s_hcuStatePtr->futileData[4]);
                    break;
#endif /* FEATURE_HCU_AES_CTR_ENGINE */
#if FEATURE_HCU_AES_CCM_ENGINE
                /* if AES-CCM, need save IV, CV and MAC */
                case AES_CCM_MODE:
                    while(srCount < 2u)
                    {
                        if(HCU_GetCVReady())
                        {
                            HCU_ReadCV(&s_hcuStatePtr->futileData[4]);
                            srCount++;
                        }
                        if(HCU_GetMacReady())
                        {
                            HCU_ReadMAC(s_hcuStatePtr->futileData);
                            (void) HCU_DRV_DoneMAC();
                            srCount++;
                        }
                        if(HCU_GetMacWait())
                        {
                            HCU_SetMAC(s_hcuStatePtr->ccmConfig->tag, s_hcuStatePtr->ccmConfig->tagSize);
                            (void) HCU_DRV_DoneMAC();
                            srCount++;
                        }
                        if(HCU_GetIVReady())
                        {
                            HCU_ReadIV(s_hcuStatePtr->futileData);
                            srCount++;
                        }
                    }
                    break;
#endif /* FEATURE_HCU_AES_CCM_ENGINE */
                /* if AES-CMAC, need save IV and MAC */
                case AES_CMAC_MODE:
                    while (srCount < 1u) {
                        if(HCU_GetMacReady())
                        {
                            HCU_ReadMAC(s_hcuStatePtr->futileData);
                            (void) HCU_DRV_DoneMAC();
                            srCount++;
                        }
                        if(HCU_GetMacWait())
                        {
                            HCU_SetMAC(s_hcuStatePtr->cmacConfig->macPtr, s_hcuStatePtr->cmacConfig->macLen);
                            (void) HCU_DRV_DoneMAC();
                            srCount++;
                        }
                        if(HCU_GetIVReady())
                        {
                            HCU_ReadIV(s_hcuStatePtr->futileData);
                            srCount++;
                        }
                    }
                    break;
                default:
                    ; /* Empty statement */
                    break;
            }
            HCU_ClearStatusFlag(OPERATION_DONE_FLAG);
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

    /* SHA AUTH don't need DMA for data carrying */
    if ((s_hcuStatePtr->dataOutputPtr != NULL) && 
        ((s_hcuStatePtr->algorithm != SHA_256_MODE) || (s_hcuStatePtr->mode == MODE_ENC)))
    {
        /* DMA block size config by watermark, now is 4 words, 16 bytes */
        (void) DMA_DRV_ConfigMultiBlockTransfer(s_hcuStatePtr->egressDMAChannel, DMA_TRANSFER_PERIPH2MEM,
                                                (uint32_t) (&(HCU->OUTMSG)), 
                                                (uint32_t) s_hcuStatePtr->dataOutputPtr, dmaTransferSize,
                                                16U, (uint32_t) s_hcuStatePtr->outputCount / 16u, true);

        /* Combine ingress to completed event */
        (void) DMA_DRV_InstallCallback(s_hcuStatePtr->egressDMAChannel, (HCU_DRV_CompleteDMA), (void *) (uint32_t) (s_hcuStatePtr->egressDMAChannel));  /* PRQA S 0326 */
        (void) DMA_DRV_StartChannel(s_hcuStatePtr->egressDMAChannel);
        HCU_SetOutputDMA(true);
    }

    if (s_hcuStatePtr->dataInputPtr != NULL)
    {
#if FEATURE_HCU_SHA_ENGINE
        /* In SHA mode, HCU should write 1 or 2 words one time */
        if(s_hcuStatePtr->algorithm == SHA_256_MODE)
        {
            /* DMA block size config by watermark, now is 2 words, 8 bytes */
            (void) DMA_DRV_ConfigMultiBlockTransfer(s_hcuStatePtr->ingressDMAChannel, DMA_TRANSFER_MEM2PERIPH, (uint32_t) s_hcuStatePtr->dataInputPtr, 
                                                    (uint32_t) (&(HCU->INMSG)),
                                                    dmaTransferSize, 8U, (uint32_t) s_hcuStatePtr->inputCount / 8u, false);
        }else
#endif /* FEATURE_HCU_SHA_ENGINE */
        {
            /* DMA block size config by watermark, now is 4 words, 16 bytes */
            (void) DMA_DRV_ConfigMultiBlockTransfer(s_hcuStatePtr->ingressDMAChannel, DMA_TRANSFER_MEM2PERIPH, (uint32_t) s_hcuStatePtr->dataInputPtr, 
                                                    (uint32_t) (&(HCU->INMSG)),
                                                    dmaTransferSize, 16U, (uint32_t) s_hcuStatePtr->inputCount / 16u, false);
        }
        /* Combine ingress to completed event */
        (void) DMA_DRV_InstallCallback(s_hcuStatePtr->ingressDMAChannel, (HCU_DRV_CompleteDMA), (void *) (uint32_t) (s_hcuStatePtr->ingressDMAChannel));    /* PRQA S 0326 */
        (void) DMA_DRV_StartChannel(s_hcuStatePtr->ingressDMAChannel);
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
        uint8_t *dataPtr = (uint8_t *)s_hcuStatePtr->futileData;
        if (AES_CCM_MODE == s_hcuStatePtr->algorithm)
        {
            for (i = 0; (NULL != s_hcuStatePtr->ccmConfig) && (i < s_hcuStatePtr->ccmConfig->tagSize); i++)
            {
                s_hcuStatePtr->ccmConfig->tag[i] = dataPtr[i];
            }
        }
        else if (AES_CMAC_MODE == s_hcuStatePtr->algorithm)
        {
            for (i = 0; (NULL != s_hcuStatePtr->cmacConfig) && (i < s_hcuStatePtr->cmacConfig->macLen); i++)
            {
                s_hcuStatePtr->cmacConfig->macPtr[i] = dataPtr[i];
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

static status_t HCU_DRV_LoadKey(bool hwKeySelected, uint32_t hwKeySlot,
                                uint32_t const * swKeyPtr, hcu_key_size_t keySize)
{
    status_t status = STATUS_SUCCESS;
    uint8_t cnt = 0;
    while(!HCU_GetKeyWait())
    {
        if(cnt > 100u)
        {
            status = STATUS_BUSY;
            break;
        }
        cnt++;
    }
    if(status == STATUS_SUCCESS)
    {
        if(hwKeySelected)
        {
            status = FLASH_DRV_LoadAESKey(0, FEATURE_HCU_GET_KEY_ADDR(hwKeySlot));
        }else
        {
            status = HCU_DRV_LoadUserKey(swKeyPtr, keySize);
        }
#if FEATURE_HCU_KEY_SIZE_UNFIXED
        HCU_SetKeySize(keySize);
#endif
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
status_t HCU_DRV_EncryptECB(aes_algorithm_config_t * aesAlgCfg)
{
    status_t status;
    DEV_ASSERT(aesAlgCfg != NULL);

    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm(aesAlgCfg->dataInputPtr, aesAlgCfg->msgLen, 0, 
                                 aesAlgCfg->dataOutputPtr, ENG_AES, ALG_AES_ECB, MODE_ENC);
    if (STATUS_SUCCESS == status)
    {
        if ( HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if ( HCU_USING_DMA == s_hcuStatePtr->carryType )
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
        
        /* Start the command */
        HCU_StartEngine();
    
        /* Load key after setting go */
        status = HCU_DRV_LoadKey(aesAlgCfg->hwKeySelected, aesAlgCfg->hwKeySlot, 
                                    aesAlgCfg->swKeyPtr, aesAlgCfg->keySize);
        if (STATUS_SUCCESS == status)
        {
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
status_t HCU_DRV_DecryptECB(aes_algorithm_config_t * aesAlgCfg)
{
    status_t status;
    DEV_ASSERT(aesAlgCfg != NULL);

    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm(aesAlgCfg->dataInputPtr, aesAlgCfg->msgLen, 0, 
                                 aesAlgCfg->dataOutputPtr, ENG_AES, ALG_AES_ECB, MODE_DEC);
    if (STATUS_SUCCESS == status)
    {
        if ( HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if ( HCU_USING_DMA == s_hcuStatePtr->carryType )
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
        
        /* Start the command */
        HCU_StartEngine();
    
        /* Load key after setting go */
        status = HCU_DRV_LoadKey(aesAlgCfg->hwKeySelected, aesAlgCfg->hwKeySlot, 
                                 aesAlgCfg->swKeyPtr, aesAlgCfg->keySize);
        if (STATUS_SUCCESS == status)
        {
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
status_t HCU_DRV_EncryptCTR(aes_algorithm_config_t * aesAlgCfg, uint32_t * cv)
{
    status_t status;
    DEV_ASSERT(aesAlgCfg != NULL);

    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm(aesAlgCfg->dataInputPtr, aesAlgCfg->msgLen, 0,
                                 aesAlgCfg->dataOutputPtr, ENG_AES, ALG_AES_CTR, MODE_ENC);
    if (STATUS_SUCCESS == status)
    {
        if ( HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            /* Enable CV ready flag */
            HCU_SetIntMode(AES_CV_READY_FLAG, true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if ( HCU_USING_DMA == s_hcuStatePtr->carryType )
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
        
        /* Start the command */
        HCU_StartEngine();
    
        /* Load key after setting go */
        status = HCU_DRV_LoadKey(aesAlgCfg->hwKeySelected, aesAlgCfg->hwKeySlot, 
                                 aesAlgCfg->swKeyPtr, aesAlgCfg->keySize);
        if (STATUS_SUCCESS == status)
        {
            /* Wait until CV wait set */
            while(!HCU_GetCVWait());
            /* Only first block use CV from user */
            if(aesAlgCfg->msgType & MSG_START)
            {
                /* Configure the CV */
                if (NULL != cv)
                {
                    /* Only update when the CV is not NULL */
                    HCU_SetCV(cv);
                }
            }else
            {
                HCU_SetCV(&s_hcuStatePtr->futileData[4]);
            }
        
            /* Polling is blocking */
            if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
            {
                do
                {
                    status = HCU_RunOneLoop();
                    if(HCU_GetCVReady())
                    {
                        HCU_ReadCV(&s_hcuStatePtr->futileData[4]);
                    }
                } while (STATUS_BUSY == status);
                s_hcuStatePtr->cmdInProgress = false;
            }
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
status_t HCU_DRV_DecryptCTR(aes_algorithm_config_t * aesAlgCfg, uint32_t * cv)
{
    status_t status;
    DEV_ASSERT(aesAlgCfg != NULL);

    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm(aesAlgCfg->dataInputPtr, aesAlgCfg->msgLen, 0, 
                                 aesAlgCfg->dataOutputPtr,ENG_AES, ALG_AES_CTR, MODE_DEC);
    if (STATUS_SUCCESS == status)
    {
        if ( HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            /* Enable CV read flag */
            HCU_SetIntMode(AES_CV_READY_FLAG, true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if ( HCU_USING_DMA == s_hcuStatePtr->carryType )
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
        
        /* Start the command */
        HCU_StartEngine();
    
        /* Load key after setting go */
        status = HCU_DRV_LoadKey(aesAlgCfg->hwKeySelected, aesAlgCfg->hwKeySlot, 
                                 aesAlgCfg->swKeyPtr, aesAlgCfg->keySize);
        if (STATUS_SUCCESS == status)
        {
            /* Wait until CV wait set */
            while(!HCU_GetCVWait());
            /* Only first block use CV from user */
            if(aesAlgCfg->msgType & MSG_START)
            {
                /* Configure the CV */
                if (NULL != cv)
                {
                    /* Only update when the CV is not NULL */
                    HCU_SetCV(cv);
                }
            }else
            {
                HCU_SetCV(&s_hcuStatePtr->futileData[4]);
            }
        
            /* Polling is blocking */
            if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
            {
                do
                {
                    status = HCU_RunOneLoop();
                    if(HCU_GetCVReady())
                    {
                        HCU_ReadCV(&s_hcuStatePtr->futileData[4]);
                    }
                } while (STATUS_BUSY == status);
                s_hcuStatePtr->cmdInProgress = false;
            }
        }
    }

    return status;
}
#endif /* FEATURE_HCU_AES_CTR_ENGINE */

#if FEATURE_HCU_SM4_ENGINE
/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_EncryptSM4ECB
 * Description   : This function performs the AES-128 encryption in ECB mode of
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
    status = HCU_ConfigAlgorithm(plainText, length, 0, cipherText,
                                 ENG_SM4, ALG_AES_ECB, MODE_ENC);
    if (STATUS_SUCCESS == status)
    {
        if ( HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if ( HCU_USING_DMA == s_hcuStatePtr->carryType )
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
status_t HCU_DRV_DecryptSM4ECB(const void *cipherText,
                               uint32_t length, void *plainText)
{
    status_t status;
    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm(cipherText, length, 0, plainText,
                                 ENG_SM4, ALG_AES_ECB, MODE_DEC);
    if (STATUS_SUCCESS == status)
    {
        if ( HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if ( HCU_USING_DMA == s_hcuStatePtr->carryType )
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
status_t HCU_DRV_EncryptCBC(aes_algorithm_config_t * aesAlgCfg, uint32_t * iv)
{
    status_t status;
    DEV_ASSERT(aesAlgCfg != NULL);
    uint32_t timeout = 0;

    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm(aesAlgCfg->dataInputPtr, aesAlgCfg->msgLen, 0,
                                 aesAlgCfg->dataOutputPtr, ENG_AES, ALG_AES_CBC, MODE_ENC);
    if (STATUS_SUCCESS == status)
    {
        if ( HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            /* Enable IV read flag */
            HCU_SetIntMode(AES_IV_READY_FLAG, true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if ( HCU_USING_DMA == s_hcuStatePtr->carryType )
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
        
        /* Start the command */
        HCU_StartEngine();
    
        /* Load key after setting go */
        status = HCU_DRV_LoadKey(aesAlgCfg->hwKeySelected, aesAlgCfg->hwKeySlot, 
                                 aesAlgCfg->swKeyPtr, aesAlgCfg->keySize);
        if (STATUS_SUCCESS == status)
        {
            /* Wait until IV wait set */
            while(!HCU_GetIVWait())
            {
                if(timeout > HCU_WAIT_TIMEOUT)
                {
                    status = STATUS_TIMEOUT;
                    break;
                }
                timeout++;
            }
            if (STATUS_SUCCESS == status)
            {
                /* Only first block use IV from user */
                if((aesAlgCfg->msgType == MSG_START) || (aesAlgCfg->msgType == MSG_ALL))
                {
                    /* Configure the IV */
                    if (NULL != iv)
                    {
                        /* Only update when the IV is not NULL */
                        HCU_SetIV(iv);
                    }
                }else
                {
                    HCU_SetIV(s_hcuStatePtr->futileData);
                }
            
                /* Polling is blocking */
                if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
                {
                    do
                    {
                        status = HCU_RunOneLoop();
                        if(HCU_GetIVReady())
                        {
                            HCU_ReadIV(s_hcuStatePtr->futileData);
                        }
                    } while (STATUS_BUSY == status);
                    s_hcuStatePtr->cmdInProgress = false;
                }
            }
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
status_t HCU_DRV_DecryptCBC(aes_algorithm_config_t * aesAlgCfg, uint32_t * iv)
{
    status_t status;
    uint32_t timeout = 0;
    DEV_ASSERT(aesAlgCfg != NULL);

    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm(aesAlgCfg->dataInputPtr, aesAlgCfg->msgLen, 0, 
                                 aesAlgCfg->dataOutputPtr,ENG_AES, ALG_AES_CBC, MODE_DEC);
    if (STATUS_SUCCESS == status)
    {
        if ( HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark, output fifo watermark and operate done */
            HCU_SetDefaultInterrupt(true);
            /* Enable IV read flag */
            HCU_SetIntMode(AES_IV_READY_FLAG, true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if ( HCU_USING_DMA == s_hcuStatePtr->carryType )
        {
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
        
        /* Start the command */
        HCU_StartEngine();
    
        /* Load key after setting go */
        status = HCU_DRV_LoadKey(aesAlgCfg->hwKeySelected, aesAlgCfg->hwKeySlot, 
                                 aesAlgCfg->swKeyPtr, aesAlgCfg->keySize);
        if (STATUS_SUCCESS == status)
        {
            /* Wait until IV wait set */
            while(!HCU_GetIVWait())
            {
                if(timeout > HCU_WAIT_TIMEOUT)
                {
                    status = STATUS_TIMEOUT;
                    break;
                }
                timeout++;
            }
            if (STATUS_SUCCESS == status)
            {
                /* Only first block use IV from user */
                if((aesAlgCfg->msgType == MSG_START) || (aesAlgCfg->msgType == MSG_ALL))
                {
                    /* Configure the IV */
                    if (NULL != iv)
                    {
                        /* Only update when the IV is not NULL */
                        HCU_SetIV(iv);
                    }
                }else
                {
                    HCU_SetIV(s_hcuStatePtr->futileData);
                }
            
                /* Polling is blocking */
                if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
                {
                    do
                    {
                        status = HCU_RunOneLoop();
                        if(HCU_GetIVReady())
                        {
                            HCU_ReadIV(s_hcuStatePtr->futileData);
                        }
                    } while (STATUS_BUSY == status);
                    s_hcuStatePtr->cmdInProgress = false;
                }
            }
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
status_t HCU_DRV_GenerateMAC(aes_algorithm_config_t * aesAlgCfg, hcu_cmac_config_t *cmacConfig)
{
    status_t status;
    DEV_ASSERT(aesAlgCfg != NULL);
    DEV_ASSERT(cmacConfig != NULL);
    uint8_t i;
    uint32_t timeout = 0;

    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm(aesAlgCfg->dataInputPtr, aesAlgCfg->msgLen, 0, NULL,
                                 ENG_AES, ALG_AES_CMAC, MODE_ENC);
    if (STATUS_SUCCESS == status)
    {
        /* Copy mac pointer and length */
        if (NULL != cmacConfig->macPtr)
        {
            s_hcuStatePtr->cmacConfig = cmacConfig;        
        }
        /* Set isLastBlock at the end block */
        if((aesAlgCfg->msgType == MSG_END) || (aesAlgCfg->msgType == MSG_ALL))
        {
            s_hcuStatePtr->isLastBlock = true;
        }else
        {
            s_hcuStatePtr->isLastBlock = false;
        }

    #if defined(FEATURE_HCU_MAC_LEN_IN_BIT) && (FEATURE_HCU_MAC_LEN_IN_BIT > 0)
        /* Configure the MAC length */
        HCU_SetMacLength(cmacConfig->macLen << 3);
    #else
        /* Configure the MAC length */
        HCU_SetMacLength(cmacConfig->macLen);
    #endif /* FEATURE_HCU_MAC_LEN_IN_BIT */
        /* Configure the message type */
        HCU_SetMsgType(aesAlgCfg->msgType);
        if ( HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark and operate done */
            HCU_SetDoneInterrupt(true);
            HCU_SetInputInterrupt(true);
            /* Enable mac ready and iv ready */
            HCU_SetIntMode(AES_MAC_READY_FLAG, true);
            if((aesAlgCfg->msgType == MSG_START) || (aesAlgCfg->msgType == MSG_MIDDLE))
            {
                HCU_SetIntMode(AES_IV_READY_FLAG, true);
            }
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if ( HCU_USING_DMA == s_hcuStatePtr->carryType )
        {
            /* Enable mac ready and iv ready */
            HCU_SetIntMode(AES_MAC_READY_FLAG, true);
            if((aesAlgCfg->msgType == MSG_START) || (aesAlgCfg->msgType == MSG_MIDDLE))
            {
                HCU_SetIntMode(AES_IV_READY_FLAG, true);
            }
            /* Enable operate done */
            HCU_SetDoneInterrupt(true);
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
        /* Start the command */
        HCU_StartEngine();
        /* Load key after setting go */
        status = HCU_DRV_LoadKey(aesAlgCfg->hwKeySelected, aesAlgCfg->hwKeySlot, 
                                 aesAlgCfg->swKeyPtr, aesAlgCfg->keySize);
        if (STATUS_SUCCESS == status)
        {
            /* Only first block don't need write MAC */
            if((aesAlgCfg->msgType == MSG_END) || (aesAlgCfg->msgType == MSG_MIDDLE))
            {
                /* Wait MAC to IV */
                while(!HCU_GetIVWait())
                {
                    if(timeout > HCU_WAIT_TIMEOUT)
                    {
                        status = STATUS_TIMEOUT;
                        break;
                    }
                    timeout++;
                }
                if (STATUS_SUCCESS == status)
                {
                    HCU_SetIV(s_hcuStatePtr->futileData);
                }
            }
            if (STATUS_SUCCESS == status)
            {
                /* Polling is blocking */
                if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
                {
                    do
                    {
                        status = HCU_RunOneLoop();
                        /* Only last block, rise MAC ready */
                        if(HCU_GetMacReady())
                        {
                            /* Get MAC from register by 32-bit */
                            HCU_ReadMAC(s_hcuStatePtr->futileData);
                            uint8_t *dataPtr = (uint8_t *)s_hcuStatePtr->futileData;
                            for (i = 0; (NULL != cmacConfig->macPtr) && (i < cmacConfig->macLen); ++i)
                            {
                                cmacConfig->macPtr[i] = dataPtr[i];
                            }
                        }
                        /* If not last block, rise IV ready */
                        if(HCU_GetIVReady())
                        {
                            HCU_ReadIV(s_hcuStatePtr->futileData);
                        }
                    } while (STATUS_BUSY == status);
                    s_hcuStatePtr->cmdInProgress = false;
                }
            }
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
status_t HCU_DRV_AuthorizeMAC(aes_algorithm_config_t * aesAlgCfg, hcu_cmac_config_t *cmacConfig)
{
    status_t status;
    DEV_ASSERT(aesAlgCfg != NULL);
    DEV_ASSERT(cmacConfig != NULL);
    uint8_t i;
    uint8_t tagBuffer[16] = {0};
    uint32_t timeout = 0;

    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm(aesAlgCfg->dataInputPtr, aesAlgCfg->msgLen, 0, NULL,
                                 ENG_AES, ALG_AES_CMAC, MODE_DEC);
    if (STATUS_SUCCESS == status)
    {
        if (NULL != cmacConfig->macPtr)
        {
            /* Copy mac pointer and length */
            s_hcuStatePtr->cmacConfig = cmacConfig;
            /* Set MAC value */
            for (i = 0; i < cmacConfig->macLen; i++)
            {
                tagBuffer[i] = cmacConfig->macPtr[i];
            }
        }
        /* Set isLastBlock at the end block */
        if((aesAlgCfg->msgType == MSG_END) || (aesAlgCfg->msgType == MSG_ALL))
        {
            s_hcuStatePtr->isLastBlock = true;
        }else
        {
            s_hcuStatePtr->isLastBlock = false;
        }
    #if defined(FEATURE_HCU_MAC_LEN_IN_BIT) && (FEATURE_HCU_MAC_LEN_IN_BIT > 0)
        /* Configure the MAC length */
        HCU_SetMacLength(cmacConfig->macLen << 3);
    #else
        /* Configure the MAC length */
        HCU_SetMacLength(cmacConfig->macLen);
    #endif /* FEATURE_HCU_MAC_LEN_IN_BIT */
        /* Configure the message type */
        HCU_SetMsgType(aesAlgCfg->msgType);
        if ( HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
        {
            /* Enable input fifo watermark and operate done */
            HCU_SetDoneInterrupt(true);
            HCU_SetInputInterrupt(true);
            /* Enable mac ready and iv ready */
            HCU_SetIntMode(AES_MAC_WAIT_FLAG, true);
            if((aesAlgCfg->msgType == MSG_START) || (aesAlgCfg->msgType == MSG_MIDDLE))
            {
                HCU_SetIntMode(AES_IV_READY_FLAG, true);
            }
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        }
        else if ( HCU_USING_DMA == s_hcuStatePtr->carryType )
        {
            /* Enable operate done */
            HCU_SetDoneInterrupt(true);
            /* Enable mac ready and iv ready */
            HCU_SetIntMode(AES_MAC_WAIT_FLAG, true);
            if((aesAlgCfg->msgType == MSG_START) || (aesAlgCfg->msgType == MSG_MIDDLE))
            {
                HCU_SetIntMode(AES_IV_READY_FLAG, true);
            }
            INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
            (void)HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
        }else {
            /* Not Enter this statement */
        }
        /* Start the command */
        HCU_StartEngine();
        /* Load key after setting go */
        status = HCU_DRV_LoadKey(aesAlgCfg->hwKeySelected, aesAlgCfg->hwKeySlot, 
                                 aesAlgCfg->swKeyPtr, aesAlgCfg->keySize);
        if (STATUS_SUCCESS == status)
        {
            /* Only first block don't need write MAC */
            if((aesAlgCfg->msgType == MSG_END) || (aesAlgCfg->msgType == MSG_MIDDLE))
            {
                /* Wait MAC to IV */
                while(!HCU_GetIVWait())
                {
                    if(timeout > HCU_WAIT_TIMEOUT)
                    {
                        status = STATUS_TIMEOUT;
                        break;
                    }
                    timeout++;
                }
                if (STATUS_SUCCESS == status)
                {
                    HCU_SetIV(s_hcuStatePtr->futileData);
                }
            }
            if (STATUS_SUCCESS == status)
            {
                /* Polling is blocking */
                if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
                {
                    do
                    {
                        status = HCU_RunOneLoop();
                        if(HCU_GetMacWait())
                        {
                            /* Copy CMAC result to output buffer */
                            HCU_SetMAC(tagBuffer, cmacConfig->macLen);
                        }
                        /* If not last block, rise IV ready */
                        if(HCU_GetIVReady())
                        {
                            HCU_ReadIV(s_hcuStatePtr->futileData);
                        }
                    } while (STATUS_BUSY == status);
                    /* Check CMAC result at the end */
                    if((aesAlgCfg->msgType == MSG_END) || (aesAlgCfg->msgType == MSG_ALL))
                    {
                        status = HCU_DRV_DoneMAC();
                    }
                    s_hcuStatePtr->cmdInProgress = false;
                }
            }
        }
    }

    return status;
}

#if FEATURE_HCU_AES_CCM_ENGINE
/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_EncryptCCM
 * Description   : This function performs the AES-128 encryption in CCM mode of
 * the input cipher text buffer.
 *
 * Implements    : HCU_DRV_EncryptCCM_Activity
 * END**************************************************************************/
status_t HCU_DRV_EncryptCCM(aes_algorithm_config_t * aesAlgCfg, hcu_ccm_config_t *ccmConfig)
{
    DEV_ASSERT(aesAlgCfg != NULL);
    DEV_ASSERT(ccmConfig != NULL);
    status_t status;
    uint8_t i;

    /* Push addition data and nonce at first block */
    if(aesAlgCfg->msgType & MSG_START)
    {
        status = HCU_DRV_PushAdditionData(aesAlgCfg, ccmConfig, MODE_ENC);
        if(STATUS_SUCCESS != status)
        {
            return status;
        }
    }

    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm(aesAlgCfg->dataInputPtr, aesAlgCfg->msgLen, 0, aesAlgCfg->dataOutputPtr,
                                 ENG_AES, ALG_AES_CCM, MODE_ENC);
    if (STATUS_SUCCESS != status)
    {
        return status;
    }
    /* Set isLastBlock at the end block */
    if (aesAlgCfg->msgType & MSG_END )
    {
        HCU_SetMsgType(MSG_END);
        s_hcuStatePtr->isLastBlock = true;
    }else
    {
        HCU_SetMsgType(MSG_MIDDLE);
        s_hcuStatePtr->isLastBlock = false;
    }

    /* Check carry type */
    if ( HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
    {
        /* Enable input fifo watermark, output fifo watermark and operate done */
        HCU_SetDefaultInterrupt(true);
        /* Enable mac ready and iv ready */
        if((aesAlgCfg->msgType & MSG_END) == 0)
        {
            HCU_SetIntMode(AES_IV_READY_FLAG, true);
        }else {
            HCU_SetIntMode(AES_MAC_READY_FLAG, true);
        }
        /* Enable cv ready */
        HCU_SetIntMode(AES_CV_READY_FLAG, true);
        INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
    }
    else if ( HCU_USING_DMA == s_hcuStatePtr->carryType )
    {
        HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
    }

    /* Start the command */
    HCU_StartEngine();
    /* Load key after setting go */
    status = HCU_DRV_LoadKey(aesAlgCfg->hwKeySelected, aesAlgCfg->hwKeySlot, 
                             aesAlgCfg->swKeyPtr, aesAlgCfg->keySize);
    if (STATUS_SUCCESS != status)
    {
        return status;
    }

    /* Wait CV ready */
    while(!HCU_GetCVWait());
    /* 0~3 is IV, 4~7 is CV */
    HCU_SetCV(&s_hcuStatePtr->futileData[4]);
    /* Wait MAC to IV */
    while(!HCU_GetIVWait());
    /* 0~3 is IV, 4~7 is CV */
    HCU_SetIV(s_hcuStatePtr->futileData);

    /* Polling is blocking */
    if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
    {
        do
        {
            status = HCU_RunOneLoop();
            /* Only last block, rise MAC ready */
            if(HCU_GetMacReady())
            {
                /* Get MAC from register by 32-bit */
                HCU_ReadMAC(s_hcuStatePtr->futileData);
                for (i = 0; (NULL != ccmConfig->tag) && (i < ccmConfig->tagSize); ++i)
                {
                    ccmConfig->tag[i] = *((uint8_t *)(s_hcuStatePtr->futileData) + i);
                }
            }
            /* If not last block, rise CV ready and IV ready */
            if(HCU_GetCVReady())
            {
                HCU_ReadCV(&s_hcuStatePtr->futileData[4]);
            }
            if(HCU_GetIVReady())
            {
                HCU_ReadIV(s_hcuStatePtr->futileData);
            }
        } while (STATUS_BUSY == status);
        s_hcuStatePtr->cmdInProgress = false;
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
status_t HCU_DRV_DecryptCCM(aes_algorithm_config_t * aesAlgCfg, hcu_ccm_config_t *ccmConfig)
{
    status_t status = STATUS_SUCCESS;
    DEV_ASSERT(aesAlgCfg != NULL);
    DEV_ASSERT(ccmConfig != NULL);

    /* Push addition data and nonce at first block */
    if(aesAlgCfg->msgType & MSG_START)
    {
        status = HCU_DRV_PushAdditionData(aesAlgCfg, ccmConfig, MODE_DEC);
        if(STATUS_SUCCESS != status)
        {
            return status;
        }
    }

    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm(aesAlgCfg->dataInputPtr, aesAlgCfg->msgLen, 0, aesAlgCfg->dataOutputPtr,
                                 ENG_AES, ALG_AES_CCM, MODE_DEC);
    if (STATUS_SUCCESS != status)
    {
        return status;
    }
    /* Set isLastBlock at the end block */
    if (aesAlgCfg->msgType & MSG_END )
    {
        HCU_SetMsgType(MSG_END);
        s_hcuStatePtr->isLastBlock = true;
    }else
    {
        HCU_SetMsgType(MSG_MIDDLE);
        s_hcuStatePtr->isLastBlock = false;
    }

    /* Check carry type */
    if ( HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
    {
        /* Enable input fifo watermark, output fifo watermark and operate done */
        HCU_SetDefaultInterrupt(true);
        /* Enable mac ready and iv ready */
        if((aesAlgCfg->msgType & MSG_END) == 0)
        {
            HCU_SetIntMode(AES_IV_READY_FLAG, true);
        }else {
            HCU_SetIntMode(AES_MAC_WAIT_FLAG, true);
        }
        /* Enable cv ready */
        HCU_SetIntMode(AES_CV_READY_FLAG, true);
        INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
    }
    else if ( HCU_USING_DMA == s_hcuStatePtr->carryType )
    {
        HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
    }

    /* Start the command */
    HCU_StartEngine();
    /* Load key after setting go */
    status = HCU_DRV_LoadKey(aesAlgCfg->hwKeySelected, aesAlgCfg->hwKeySlot, 
                             aesAlgCfg->swKeyPtr, aesAlgCfg->keySize);
    if (STATUS_SUCCESS != status)
    {
        return status;
    }

    /* Wait CV ready */
    while(!HCU_GetCVWait());
    /* 0~3 is IV, 4~7 is CV */
    HCU_SetCV(&s_hcuStatePtr->futileData[4]);
    /* Wait MAC to IV */
    while(!HCU_GetIVWait());
    /* 0~3 is IV, 4~7 is CV */
    HCU_SetIV(s_hcuStatePtr->futileData);

    /* Polling is blocking */
    if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
    {
        do
        {
            status = HCU_RunOneLoop();
            /* Only last block, rise MAC ready */
            if(HCU_GetMacWait())
            {
                /* Get MAC from register by 32-bit */
                HCU_SetMAC(ccmConfig->tag, ccmConfig->tagSize);
            }
            /* If not last block, rise CV ready and IV ready */
            if(HCU_GetCVReady())
            {
                HCU_ReadCV(&s_hcuStatePtr->futileData[4]);
            }
            if(HCU_GetIVReady())
            {
                HCU_ReadIV(s_hcuStatePtr->futileData);
            }
        } while (STATUS_BUSY == status);
        /* Check CMAC result at the end */
        if (aesAlgCfg->msgType & MSG_END )
        {
            status = HCU_DRV_DoneMAC();
        }
        s_hcuStatePtr->cmdInProgress = false;
        s_hcuStatePtr->cmdInProgress = false;
    }

    return status;
}
#endif /* FEATURE_HCU_AES_CCM_ENGINE */

#if FEATURE_HCU_SHA_ENGINE
/*FUNCTION**********************************************************************
 *
 * Function Name : HCU_DRV_GenerateSHA
 * Description   : This function calculates the MAC of a given message using SHA
 *
 * Implements    : HCU_DRV_GenerateSHA_Activity
 * END**************************************************************************/
status_t HCU_DRV_GenerateSHA(sha_algorithm_config_t * shaAlgCfg)
{
    status_t status;
    DEV_ASSERT(shaAlgCfg != NULL);
    if (!(shaAlgCfg->msgType & MSG_END))
    {
        /* If divide in several blocks, each block must at least 64 bytes */
        if (HCU_SHA_256 == shaAlgCfg->shaType)
        {
            if ((shaAlgCfg->msgLen % HCU_SHA_256_BLOCK_SIZE != 0) || (shaAlgCfg->msgLen == 0))
            {
                return STATUS_ERROR;
            }
        }
        /* If divide in several blocks, each block must at least 128 bytes */
        if (HCU_SHA_384 == shaAlgCfg->shaType)
        {
            if ((shaAlgCfg->msgLen % HCU_SHA_384_BLOCK_SIZE != 0) || (shaAlgCfg->msgLen == 0))
            {
                return STATUS_ERROR;
            }
        }        
    }
    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm(shaAlgCfg->dataInputPtr, shaAlgCfg->msgLen, 0, shaAlgCfg->dataOutputPtr,
                                 ENG_SHA, (hcu_alg_aes_mode_t)shaAlgCfg->shaType, MODE_ENC);
    if (STATUS_SUCCESS != status)
    {
        return status;
    }
    /* Configure the message type */
    HCU_SetMsgType(shaAlgCfg->msgType);
    /* Configure the total length of message at the beginning */
    if (shaAlgCfg->msgType & MSG_START)
    {
        HCU_SetMsgTotalLength(shaAlgCfg->totalLen);
        /* When change sha-verify, OD will immediately set */
        HCU_ClearStatusFlag(OPERATION_DONE_FLAG);
    }
    /* Set if last block */
    s_hcuStatePtr->isLastBlock = (shaAlgCfg->msgType & MSG_END) ? true : false;
    if ( HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
    {
        /* Enable input fifo watermark and operate done */
        HCU_SetInputInterrupt(true);
        HCU_SetDoneInterrupt(true);
        /* Enable context data ready */
        HCU_SetShaCtxReadyInt(true);
        /* Enable output ready in last block */
        if (NULL != shaAlgCfg->dataOutputPtr)
        {
            HCU_SetOutputInterrupt(true);
        }
        INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
    }
#if FEATURE_HCU_HAS_FIXED_DMA
    else if ( HCU_USING_DMA == s_hcuStatePtr->carryType )
    {
        /* Enable context data ready */
        HCU_SetShaCtxReadyInt(true);
        /* Check by operate done flag */
        HCU_SetDoneInterrupt(true);
        INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
    }
#endif
    /* Start the command */
    HCU_StartEngine();
    /* Only first block don't need write context data */
    if((shaAlgCfg->msgType & MSG_START) == 0)
    {
        /* Wait context data to CTX */
        while(!HCU_GetShaCtxWait());
        HCU_WriteShaCtxData(s_hcuStatePtr->futileData);
    }
    /* Polling is blocking */
    if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
    {
        do
        {
            status = HCU_RunOneLoop();
            /* Only last block no context data ready */
            if(HCU_GetShaCtxReady())
            {
                HCU_ReadShaCtxData(s_hcuStatePtr->futileData);
            }
        } while (STATUS_BUSY == status);
        s_hcuStatePtr->cmdInProgress = false;
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
status_t HCU_DRV_AuthorizeSHA(sha_algorithm_config_t * shaAlgCfg)
{
    status_t status;
    DEV_ASSERT(shaAlgCfg != NULL);
    if (!(shaAlgCfg->msgType & MSG_END))
    {
        /* If divide in several blocks, each block must at least 64 bytes */
        if (HCU_SHA_256 == shaAlgCfg->shaType)
        {
            if ((shaAlgCfg->msgLen % HCU_SHA_256_BLOCK_SIZE != 0) || (shaAlgCfg->msgLen == 0))
            {
                return STATUS_ERROR;
            }
        }
        /* If divide in several blocks, each block must at least 128 bytes */
        if (HCU_SHA_384 == shaAlgCfg->shaType)
        {
            if ((shaAlgCfg->msgLen % HCU_SHA_384_BLOCK_SIZE != 0) || (shaAlgCfg->msgLen == 0))
            {
                return STATUS_ERROR;
            }
        }        
    }
    /* Configure the algorithm */
    status = HCU_ConfigAlgorithm(shaAlgCfg->dataInputPtr, shaAlgCfg->msgLen, 0, shaAlgCfg->dataOutputPtr,
                                 ENG_SHA, (hcu_alg_aes_mode_t)shaAlgCfg->shaType, MODE_DEC);
    if (STATUS_SUCCESS != status)
    {
        return status;
    }
    /* Configure the message type */
    HCU_SetMsgType(shaAlgCfg->msgType);
    /* Configure the total length of message at the beginning*/
    if (shaAlgCfg->msgType & MSG_START)
    {
        HCU_SetMsgTotalLength(shaAlgCfg->totalLen);
        /* When change sha-verify, OD will immediately set */
        HCU_ClearStatusFlag(OPERATION_DONE_FLAG);
    }
    /* Set if last block */
    s_hcuStatePtr->isLastBlock = (shaAlgCfg->msgType & MSG_END) ? true : false;
    /* Start SHA at ending block */
    if (shaAlgCfg->msgType & MSG_END)
    {
        HCU_SetSHAVerification(true);
    }
    if ( HCU_USING_INTERRUPT == s_hcuStatePtr->carryType)
    {
        /* Enable input fifo watermark and operate done */
        HCU_SetInputInterrupt(true);
        HCU_SetDoneInterrupt(true);
        /* Enable SHA context data ready */
        HCU_SetShaCtxReadyInt(true);
        /* Enable ICV wait at last block */
        if(shaAlgCfg->msgType & MSG_END)
        {
            HCU_SetIntMode(SHA_ICV_WAIT_FLAG, true);
        }
        INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
    }
#if FEATURE_HCU_HAS_FIXED_DMA
    else if ( HCU_USING_DMA == s_hcuStatePtr->carryType )
    {
        /* Check by operate done flag */
        HCU_SetDoneInterrupt(true);
        /* Enable SHA context data ready */
        HCU_SetShaCtxReadyInt(true);
        /* Enable ICV wait at last block */
        if(shaAlgCfg->msgType & MSG_END)
        {
            HCU_SetIntMode(SHA_ICV_WAIT_FLAG, true);
        }
        INT_SYS_EnableIRQ(HCU_IRQ_NUMBER);
        HCU_DRV_ConfigDMA(s_hcuStatePtr->ingressDMAChannel, s_hcuStatePtr->egressDMAChannel);
    }
#endif
    /* Start the command */
    HCU_StartEngine();
    /* Only first block don't need write context data */
    if((shaAlgCfg->msgType & MSG_START) == 0)
    {
        /* Wait context data to CTX */
        while(!HCU_GetShaCtxWait());
        HCU_WriteShaCtxData(s_hcuStatePtr->futileData);
    }
    /* Polling is blocking */
    if (HCU_USING_POLLING == s_hcuStatePtr->carryType)
    {
        do
        {
            status = HCU_RunOneLoop();
            /* Only last block no context data ready */
            if(HCU_GetShaCtxReady())
            {
                HCU_ReadShaCtxData(s_hcuStatePtr->futileData);
            }
            /* Set SHA ICV */
            if(HCU_GetShaIcvWait())
            {
                HCU_WriteShaIcvData(shaAlgCfg->dataOutputPtr);
            }
        } while (STATUS_BUSY == status);
        /* Check SHA result at the end */
        if (shaAlgCfg->msgType & MSG_END )
        {
            if(HCU_GetStatusFlag(SHA_HASH_INVALID_FLAG))
            {
                status = STATUS_HCU_HASH_CHECK_ERROR;
                s_hcuStatePtr->status = STATUS_HCU_HASH_CHECK_ERROR;
            }
        }
        s_hcuStatePtr->cmdInProgress = false;
    }
    return status;
}
#endif /* FEATURE_HCU_HAS_SHA_AUTHORIZE */
#endif /* FEATURE_HCU_SHA_ENGINE */

#if FEATURE_HCU_AES_CCM_ENGINE
static uint8_t HCU_AESGetL(uint64_t PlainLen, uint8_t NonceLen)
{
    uint8_t L;
    L = 0;
    while (PlainLen)
    {
        PlainLen >>= 8;
        ++L;
    }
    L = (L < 2) ? 2 : L;
    /* Increase L to match the nonce len */
    NonceLen = (NonceLen > 13) ? 13 : NonceLen;
    L = ((15 - NonceLen) > L) ? (15 - NonceLen) : L;
    return L;

}

static status_t HCU_DRV_PushAdditionData(aes_algorithm_config_t * aesAlgCfg, hcu_ccm_config_t *ccmConfig, hcu_mode_sel_t mode)
{
    union
    {
        uint32_t words[4];
        uint8_t bytes[16];
    } B;
    status_t status;
    uint8_t L;
    uint32_t i, PlainLen = ccmConfig->msgLen;
    uint16_t BLen;
    uint8_t x;
    uint8_t * AddData;
    DEV_ASSERT(aesAlgCfg != NULL);
    DEV_ASSERT(ccmConfig != NULL);
    /* initialize B */
    for (i = 0; i < 4; i++)
    {
        B.words[i] = 0;
    }
    AddData = ccmConfig->addData;
    /* Check the input parameters */
    if (ccmConfig->tagSize > 16 || (ccmConfig->tagSize & 0x01))
    {
        return STATUS_HCU_CCM_TAG_SIZE_ERROR;
    }

    /* Update the internal flags */
    s_hcuStatePtr->cmdInProgress = true;
    /* Update engine and algorithm settings */
    HCU_SetEngineAlgorithm(ENG_AES, ALG_AES_CCM, mode);
    /* Set total message length */
    HCU_SetMsgTotalLength(PlainLen);
    /* Calculate Add data length */
    BLen = (ccmConfig->nonceSize / 16 + 1) * 16;
    if (ccmConfig->addDataSize > 0)
    {
        if (ccmConfig->addDataSize < ((1UL << 16) - (1UL << 8)))
        {
            if ((ccmConfig->addDataSize + 2) % 16 == 0)
                BLen += (ccmConfig->addDataSize + 2);
            else
                BLen += ((ccmConfig->addDataSize + 2) / 16 + 1) * 16;
        } else
        {
            if ((ccmConfig->addDataSize + 6) % 16 == 0)
                BLen += (ccmConfig->addDataSize + 6);
            else
                BLen += ((ccmConfig->addDataSize + 6) / 16 + 1) * 16;
        }
    }
    /* Set nonce and addition data in first block */
    HCU_SetMsgType(MSG_START);
    /* Set the MAC length */
    HCU_SetMacLength(ccmConfig->tagSize);
    /* Set message length */
    HCU_SetMsgLength(BLen, BLen);
    /*B0*/
    L = HCU_AESGetL(PlainLen, ccmConfig->nonceSize);
    if (L + ccmConfig->nonceSize != 15)
        return STATUS_HCU_CCM_NONCE_DATA_SIZE_ERROR;
    /* Update Data size configuration */
    /* Count value start at 1 */
    B.bytes[0] = L - 1;
    /*B1-B15: Nonce & Q*/
    for (i = 0; i < ccmConfig->nonceSize; i++)
    {
        B.bytes[i + 1] = ccmConfig->nonce[i];
    }

    /* Start the command */
    HCU_StartEngine();
    /* Load key after setting go */
    status = HCU_DRV_LoadKey(aesAlgCfg->hwKeySelected, aesAlgCfg->hwKeySlot, 
                             aesAlgCfg->swKeyPtr, aesAlgCfg->keySize);
    if (STATUS_SUCCESS != status)
    {
        return status;
    }
    /* Update nonce in CV */
    while(!HCU_GetCVWait());
    HCU_SetCV(B.words);

    /* Update additional data flag */
    B.bytes[0] = ccmConfig->addDataSize ? 0x40 : 0x00;
    /* Update Mac length */
    if (ccmConfig->tagSize == 0)
    {
        B.bytes[0] |= (0 << 3) | (L - 1);
    } else
    {
        B.bytes[0] |= (((ccmConfig->tagSize - 2) / 2) << 3) | (L - 1);
    }
    for (i = 0; i < L; i++)
    {

        B.bytes[15 - i] = PlainLen & 0xff;
        PlainLen >>= 8;
    }
    /* Push data to Input */
    while(!HCU_GetInputMsgWait());
    HCU_WriteInputMsg((uint32_t *) B.words, 4);
    /* reinitialize B */
    for (i = 0; i < 4; i++)
    {
        B.words[i] = 0;
    }
    /*aad length and data*/
    if (ccmConfig->addDataSize > 0)
    {
        /* store length */
        if (ccmConfig->addDataSize < ((1UL << 16) - (1UL << 8)))
        {
            B.bytes[0] = (ccmConfig->addDataSize >> 8) & 255;
            B.bytes[1] = ccmConfig->addDataSize & 255;
            x = 2;
        } else
        {
            B.bytes[0] = 0xFF;
            B.bytes[1] = 0xFE;
            B.bytes[2] = (ccmConfig->addDataSize >> 24) & 255;
            B.bytes[3] = (ccmConfig->addDataSize >> 16) & 255;
            B.bytes[4] = (ccmConfig->addDataSize >> 8) & 255;
            B.bytes[5] = ccmConfig->addDataSize & 255;
            x = 6;
        }
        /* x bytes length of addData_ori */
        for (i = x; i < ccmConfig->addDataSize + x; i++)
        {
            B.bytes[i & 0xF] = *AddData++;
            /* Check if reaches one block */
            if ((i & 0xF) == 0xF)
            {
                /* Push data to FIFO */
                while(!HCU_GetInputMsgWait());
                HCU_WriteInputMsg((uint32_t *) B.words, 4);
                /* reinitialize B */
                B.words[0] = 0;
                B.words[1] = 0;
                B.words[2] = 0;
                B.words[3] = 0;
            }
        }
        /* Check if we need to push the last block */
        if ((i & 0xF) != 0)
        {
            /* Push data to FIFO */
            while(!HCU_GetInputMsgWait());
            HCU_WriteInputMsg((uint32_t *) B.words, 4);
        }
    }
    /* Wait CV ready */
    while(!HCU_GetCVReady());
    HCU_ReadCV(&s_hcuStatePtr->futileData[4]);
    /* Wait IV ready */
    while(!HCU_GetIVReady());
    HCU_ReadIV(s_hcuStatePtr->futileData);
    /* Wait for engine to finish */
    while (HCU_IsBusy());
    HCU_ClearStatusFlag(OPERATION_DONE_FLAG);
    /* Copy CCM config */
    s_hcuStatePtr->ccmConfig = ccmConfig;
    /* Update the internal flags */
    s_hcuStatePtr->cmdInProgress = false;
    return STATUS_SUCCESS;
}
#endif /* FEATURE_HCU_AES_CCM_ENGINE */

static status_t HCU_RunOneLoop(void)
{
    status_t status;
    DEV_ASSERT(NULL != s_hcuStatePtr->dataInputPtr);
    if((AES_CMAC_MODE != s_hcuStatePtr->algorithm) && 
       (SHA_256_MODE != s_hcuStatePtr->algorithm) &&
       (SHA_384_MODE != s_hcuStatePtr->algorithm))
    {
        DEV_ASSERT(NULL != s_hcuStatePtr->dataOutputPtr);
    }

    /* Write input until input wait clear */
    while(HCU_GetInputMsgWait())
    {
#if FEATURE_HCU_SHA_ENGINE
        /* In SHA mode, HCU should write 1 or 2 words one time */
        if(s_hcuStatePtr->algorithm == SHA_256_MODE)
        {
            HCU_WriteInputMsg((uint32_t*)s_hcuStatePtr->dataInputPtr, 2u);            
            s_hcuStatePtr->dataInputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataInputPtr + 8u);
            /* Convert the message length to be processed */
            s_hcuStatePtr->inputCount -= 8u;
        }else
#endif /* FEATURE_HCU_SHA_ENGINE */
        {
            HCU_WriteInputMsg(s_hcuStatePtr->dataInputPtr, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
            s_hcuStatePtr->dataInputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataInputPtr + (FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2));
            /* Convert the message length to be processed */
            s_hcuStatePtr->inputCount -= FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2;
        }
    }

    /* Read out output until output ready clear */
    while(HCU_GetOutputMsgReady())
    {
        HCU_ReadOutputMsg(s_hcuStatePtr->dataOutputPtr, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
        s_hcuStatePtr->dataOutputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataOutputPtr + (FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2));
        s_hcuStatePtr->outputCount -= FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2;
    }

    /* End input and output */
    if(HCU_IsDone())
    {
        HCU_ClearStatusFlag(OPERATION_DONE_FLAG);
        status = STATUS_SUCCESS;
    }else
    {
        status = STATUS_BUSY;
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
    /* Input watermark handler */
    statusFlag = HCU_GetStatusFlag(INPUT_MESSAGE_WAIT_FLAG);
    intMode = HCU_GetIntMode(INPUT_MESSAGE_WAIT_FLAG);
    if (statusFlag && intMode)
    {
        if (s_hcuStatePtr->inputCount != 0U)
        {
#if FEATURE_HCU_SHA_ENGINE
            /* In SHA mode, HCU should write 1 or 2 words one time */
            if(s_hcuStatePtr->algorithm == SHA_256_MODE)
            {
                HCU_WriteInputMsg((uint32_t*)s_hcuStatePtr->dataInputPtr, 2u);
                s_hcuStatePtr->dataInputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataInputPtr + 8u);
                /* Convert the message length to be processed */
                s_hcuStatePtr->inputCount -= 8u;
            }else
#endif /* FEATURE_HCU_SHA_ENGINE */
            {
                HCU_WriteInputMsg(s_hcuStatePtr->dataInputPtr, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
                s_hcuStatePtr->dataInputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataInputPtr + (FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2));
                /* Convert the message length to be processed */
                s_hcuStatePtr->inputCount -= FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2;
            }
        }
        if (s_hcuStatePtr->inputCount == 0U)
        {
            /* When HCU busy, INTE can not be changed */
            HCU_SetIntMode(INPUT_MESSAGE_WAIT_FLAG, false);
        }
    }

    /* Output watermark handler */
    statusFlag = HCU_GetStatusFlag(OUTPUT_MESSAGE_READY_FLAG);
    intMode = HCU_GetIntMode(OUTPUT_MESSAGE_READY_FLAG);
    if (statusFlag && intMode)
    {
        if ((s_hcuStatePtr->outputCount != 0U) && (NULL != s_hcuStatePtr->dataOutputPtr))
        {
            HCU_ReadOutputMsg(s_hcuStatePtr->dataOutputPtr, FEATURE_HCU_ONE_LOOP_DATA_SIZE);
            s_hcuStatePtr->dataOutputPtr = (uint32_t *)((uint32_t)s_hcuStatePtr->dataOutputPtr + (FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2));
            /* Convert the message length to be processed */
            s_hcuStatePtr->outputCount -= FEATURE_HCU_ONE_LOOP_DATA_SIZE << 2;
        }
    }

#if FEATURE_HCU_AES_CTR_ENGINE
    /* CV Ready handler */
    statusFlag = HCU_GetStatusFlag(AES_CV_READY_FLAG);
    intMode = HCU_GetIntMode(AES_CV_READY_FLAG);
    if (statusFlag && intMode)
    {
        HCU_ReadCV(&s_hcuStatePtr->futileData[4]);
        HCU_SetIntMode(AES_CV_READY_FLAG, false);
    }
#endif /* FEATURE_HCU_AES_CTR_ENGINE */

    /* IV Ready handler */
    statusFlag = HCU_GetStatusFlag(AES_IV_READY_FLAG);
    intMode = HCU_GetIntMode(AES_IV_READY_FLAG);
    if (statusFlag && intMode)
    {
        HCU_ReadIV(s_hcuStatePtr->futileData);
        HCU_SetIntMode(AES_IV_READY_FLAG, false);
    }

    /* MAC ready handler */
    statusFlag = HCU_GetStatusFlag(AES_MAC_READY_FLAG);
    intMode = HCU_GetIntMode(AES_MAC_READY_FLAG);
    if (statusFlag && intMode)
    {
        /* Get MAC from register by 32-bit */
        HCU_ReadMAC(s_hcuStatePtr->futileData);
        HCU_SetIntMode(AES_MAC_READY_FLAG, false);
        (void) HCU_DRV_DoneMAC();
    }

#if FEATURE_HCU_SHA_ENGINE
    /* SHA context data ready handler */
    statusFlag = HCU_GetStatusFlag(SHA_CTX_READY_FLAG);
    intMode = HCU_GetIntMode(SHA_CTX_READY_FLAG);
    if (statusFlag && intMode)
    {
        /* Get SHA context data */
        HCU_ReadShaCtxData(s_hcuStatePtr->futileData);
        HCU_SetIntMode(SHA_CTX_READY_FLAG, false);
    }
#if FEATURE_HCU_HAS_SHA_AUTHORIZE
    /* SHA ICV wait handler */
    statusFlag = HCU_GetStatusFlag(SHA_ICV_WAIT_FLAG);
    intMode = HCU_GetIntMode(SHA_ICV_WAIT_FLAG);
    if (statusFlag && intMode)
    {
        /* Write SHA ICV */
        HCU_WriteShaIcvData(s_hcuStatePtr->dataOutputPtr);
        HCU_SetIntMode(SHA_ICV_WAIT_FLAG, false);
        if(HCU_GetStatusFlag(SHA_HASH_INVALID_FLAG))
        {
            s_hcuStatePtr->status = STATUS_HCU_HASH_CHECK_ERROR;
        }
    }
#endif /* FEATURE_HCU_HAS_SHA_AUTHORIZE */
#endif /* FEATURE_HCU_SHA_ENGINE */

    /* MAC wait handler */
    statusFlag = HCU_GetStatusFlag(AES_MAC_WAIT_FLAG);
    intMode = HCU_GetIntMode(AES_MAC_WAIT_FLAG);
    if (statusFlag && intMode)
    {
        /* Get MAC from register by 32-bit */
        if(AES_CMAC_MODE == s_hcuStatePtr->algorithm)
        {
            HCU_SetMAC(s_hcuStatePtr->cmacConfig->macPtr, s_hcuStatePtr->cmacConfig->macLen);
        }
        else if (AES_CCM_MODE == s_hcuStatePtr->algorithm) 
        {
            HCU_SetMAC(s_hcuStatePtr->ccmConfig->tag, s_hcuStatePtr->ccmConfig->tagSize);
        }
        else
        {
            /* Not Enter this statement */
        }
        HCU_SetIntMode(AES_MAC_WAIT_FLAG, false);
        (void) HCU_DRV_DoneMAC();
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
        HCU_SetIntMode(INPUT_MESSAGE_WAIT_FLAG, false);
        /* Disable output watermark interrupt */
        HCU_SetIntMode(OUTPUT_MESSAGE_READY_FLAG, false);
        s_hcuStatePtr->cmdInProgress = false;
    }
}

/******************************************************************************
 * EOF
 *****************************************************************************/
