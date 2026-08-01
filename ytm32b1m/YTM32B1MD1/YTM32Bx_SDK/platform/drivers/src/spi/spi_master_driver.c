/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file spi_master_driver.c
 * @brief SPI master mode driver implementation.
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 3387 Rule 13.3: A full expression containing an increment (++) or 
 *                        decrement (--) operator should have no potential side effects 
 *                        other than that caused by the increment or decrement operator.
 *
 * PRQA S 0326 Rule 11.6: Cast between a pointer to void and an integral type.
 */

#include "spi_master_driver.h"
#include "interrupt_manager.h"
#include "spi_hw_access.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*! @brief Configure and start a new SPI master transfer (internal). */
static status_t SPI_DRV_MasterStartTransfer(uint32_t instance,
                                            const uint8_t *sendBuffer,
                                            uint8_t *receiveBuffer,
                                            uint16_t transferByteCount);

/*! @brief Clean up state and hardware after transfer completion (internal). */
static void SPI_DRV_MasterCompleteTransfer(uint32_t instance);

#if defined(FEATURE_SPI_HAS_DMA_ENABLE) && (FEATURE_SPI_HAS_DMA_ENABLE > 0U)

/*! @brief DMA TX completion callback (internal). */
static void SPI_DRV_MasterCompleteDMATransfer(void *parameter, dma_chn_status_t status);

/*! @brief DMA RX completion callback (internal). */
static void SPI_DRV_MasterCompleteRX(void *parameter, dma_chn_status_t status);

#endif /* FEATURE_SPI_HAS_DMA_ENABLE */

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Fill the master configuration structure with default values.
 */
void SPI_DRV_MasterGetDefaultConfig(spi_master_config_t *spiConfig)
{
    DEV_ASSERT(spiConfig != NULL);

    spiConfig->bitsPerSec = 50000U;
    spiConfig->whichPcs = SPI_PCS0;
    spiConfig->pcsPolarity = SPI_ACTIVE_LOW;
    spiConfig->isPcsContinuous = false;
    spiConfig->bitcount = 8U;
    spiConfig->clkPhase = SPI_CLOCK_PHASE_1ST_EDGE;
    spiConfig->clkPolarity = SPI_SCK_ACTIVE_HIGH;
    spiConfig->lsbFirst = false;
    spiConfig->transferType = SPI_USING_INTERRUPTS;
#if defined(FEATURE_SPI_HAS_DMA_ENABLE) && (FEATURE_SPI_HAS_DMA_ENABLE > 0U)
    spiConfig->rxDMAChannel = 0;
    spiConfig->txDMAChannel = 0;
#endif
    spiConfig->callback = NULL;
    spiConfig->callbackParam = NULL;
}

/*!
 * @brief Initialize the SPI instance in master mode.
 */
status_t SPI_DRV_MasterInit(uint32_t instance, spi_state_t *spiState, const spi_master_config_t *spiConfig)
{
    DEV_ASSERT(instance < SPI_INSTANCE_COUNT);
    DEV_ASSERT(spiState != NULL);
    DEV_ASSERT(spiConfig != NULL);
    SPI_Type *base = g_spiBase[instance];
    status_t errorCode = STATUS_SUCCESS;

    /* Save runtime structure pointers so irq handler can point to the correct state structure */
    g_spiStatePtr[instance] = spiState;
    /* Reset the SPI registers to their default state */
    SPI_Init(base);
    /* Set for master mode */
    (void)SPI_SetMasterSlaveMode(base, SPI_MASTER);
#if !defined(FEATURE_SPI_LITE_VERSION)
    /* Set Pin configuration such that SDO=out and SDI=in */
    (void)SPI_SetPinConfigMode(base, SPI_SDI_IN_SDO_OUT, SPI_DATA_OUT_TRISTATE, true);
#else
    /* Set Pin configuration such that SDO=out and SDI=in */
    (void)SPI_SetPinConfigMode(base, SPI_SDI_IN_SDO_OUT, SPI_DATA_OUT_TRISTATE, false);
#endif /* FEATURE_SPI_LITE_VERSION */
#if !defined(FEATURE_SPI_LITE_VERSION)
    /* Calculate the FIFO size for the SPI */
    SPI_GetFifoSizes(base, &(spiState->fifoSize));
#endif /* FEATURE_SPI_LITE_VERSION */

    /* Configure bus for this device. If NULL is passed, we assume the caller has
     * preconfigured the bus and doesn't wish to re-configure it again for this transfer.
     * Do nothing for calculatedBaudRate. If the user wants to know the calculatedBaudRate
     * then they can call this function separately.
     */
    errorCode = SPI_DRV_MasterConfigureBus(instance, spiConfig, NULL);
    if (errorCode == STATUS_SUCCESS)
    {
        /* When TX is null the value sent on the bus will be 0 */
        spiState->dummy = 0;
        /* Initialize the semaphore */
        errorCode = OSIF_SemaCreate(&(spiState->spiSemaphore), 0);
        DEV_ASSERT(errorCode == STATUS_SUCCESS);
        /* Enable the interrupt */
        INT_SYS_EnableIRQ(g_spiIrqId[instance]);
        /* Finally, enable SPI */
        SPI_Enable(base);
    }
    return errorCode;
}

/*!
 * @brief De-initialize the SPI master instance.
 */
status_t SPI_DRV_MasterDeinit(uint32_t instance)
{
    DEV_ASSERT(instance < SPI_INSTANCE_COUNT);
    DEV_ASSERT(g_spiStatePtr[instance] != NULL);
    /* Instantiate local variable of type spi_state_t and point to global state */
    const spi_state_t *spiState = g_spiStatePtr[instance];
    SPI_Type *base = g_spiBase[instance];
    status_t errorCode = STATUS_SUCCESS;

    /* Check if a transfer is still in progress */
    DEV_ASSERT(spiState->isTransferInProgress == false);

    /* Reset the SPI registers to their default state, inlcuding disabling the SPI */
    SPI_Init(base);
    /* Disable the interrupt */
    INT_SYS_DisableIRQ(g_spiIrqId[instance]);
    /* Clear the state pointer. */
    g_spiStatePtr[instance] = NULL;

    /* Destroy the semaphore */
    errorCode = OSIF_SemaDestroy(&(spiState->spiSemaphore));
    DEV_ASSERT(errorCode == STATUS_SUCCESS);
    return errorCode;
}

/*!
 * @brief Configure SPI master timing delays.
 */
status_t
SPI_DRV_MasterSetDelay(uint32_t instance, uint32_t delayBetwenTransfers, uint32_t delaySCKtoPCS, uint32_t delayPCStoSCK)
{
    DEV_ASSERT(instance < SPI_INSTANCE_COUNT);

    /* Instantiate local variable of type SPI_Type and point to global state */
    SPI_Type *base = g_spiBase[instance];
    status_t errorCode = STATUS_SUCCESS;

    /* Disable module */
    errorCode = SPI_Disable(base);
    if (errorCode == STATUS_SUCCESS)
    {
        (void)SPI_SetDelay(base, SPI_SCK_TO_PCS, delaySCKtoPCS);
        (void)SPI_SetDelay(base, SPI_PCS_TO_SCK, delayPCStoSCK);
        (void)SPI_SetDelay(base, SPI_BETWEEN_TRANSFER, delayBetwenTransfers);
        /* Enable module */
        SPI_Enable(base);
    }

    return errorCode;
}

/*!
 * @brief Reconfigure the SPI bus parameters at runtime.
 */
status_t
SPI_DRV_MasterConfigureBus(uint32_t instance, const spi_master_config_t *spiConfig, uint32_t *calculatedBaudRate)
{
    DEV_ASSERT(instance < SPI_INSTANCE_COUNT);
    DEV_ASSERT(spiConfig != NULL);
    DEV_ASSERT(g_spiStatePtr[instance] != NULL);
    status_t status = STATUS_SUCCESS;
    /* Instantiate local variable of type spi_state_t and point to global state */
    spi_state_t *spiState = g_spiStatePtr[instance];
    SPI_Type *base = g_spiBase[instance];
    clock_names_t instanceClkName = s_spiClkNames[instance];
    uint32_t baudRate;

    /* The Transmit Command Register (TCR) Prescale value is calculated as part of the baud rate
       calculation. The value is stored in the run-time state structure for later programming
       in the TCR. */
    uint32_t tcrPrescaleValue;

    /* First, per the spec, we need to disable the SPI module before setting the delay */
    status = SPI_Disable(base);
    if(status == STATUS_SUCCESS)
    {
        /* Check the bitcount to make sure it falls within the boundary conditions */
        if (spiConfig->bitcount > 4096U)
        {
            status = STATUS_ERROR;
        }else{

            /* Get the SPI clock as configured in the clock manager */
            (void)CLOCK_SYS_GetFreq(instanceClkName, &(spiState->spiSrcClk));
        
            /* Configure internal state structure for SPI */
            spiState->bitsPerFrame = spiConfig->bitcount;
            spiState->isPcsContinuous = spiConfig->isPcsContinuous;
            spiState->lsb = spiConfig->lsbFirst;
            /* Save transfer type DMA/Interrupt */
            spiState->transferType = spiConfig->transferType;
            /* Update transfer status */
            spiState->isTransferInProgress = false;
            spiState->isBlocking = false;
            /* Calculate the bytes/frame for spiState->bytesPerFrame. */
            spiState->bytesPerFrame = (uint16_t)((spiState->bitsPerFrame + 7U) / 8U);
            /* Due to DMA limitations frames of 3 bytes/frame will be internally handled as 4 bytes/frame. */
            if (spiState->bytesPerFrame == 3U)
            {
                spiState->bytesPerFrame = 4U;
            }
            /* Due to some limitations all frames bigger than 4 bytes/frame must be composed only from 4 bytes chunks. */
            if (spiState->bytesPerFrame > 4U)
            {
                spiState->bytesPerFrame = (((spiState->bytesPerFrame - 1U) / 4U) + 1U) * 4U;
            }
#if defined(FEATURE_SPI_HAS_DMA_ENABLE) && (FEATURE_SPI_HAS_DMA_ENABLE > 0U)
            /* Store DMA channel number used in transfer */
            spiState->rxDMAChannel = spiConfig->rxDMAChannel;
            spiState->txDMAChannel = spiConfig->txDMAChannel;
#endif
            /* Store callback */
            spiState->callback = spiConfig->callback;
            spiState->callbackParam = spiConfig->callbackParam;
            /* Configure the desired PCS polarity */
            (void)SPI_SetPcsPolarityMode(base, spiConfig->whichPcs, spiConfig->pcsPolarity);
            /* Set up the baud rate */
            baudRate = SPI_SetBaudRate(base, spiConfig->bitsPerSec, spiState->spiSrcClk, &tcrPrescaleValue);
            /* Enable sampling point delay */
            SPI_SetSamplingPoint(base, true);
            /* Now, re-enable the SPI module */
            SPI_Enable(base);
            /* If the baud rate return is "0", it means there was an error */
            if (baudRate == (uint32_t)0)
            { 
                status = STATUS_ERROR;
            }else{

                /* If the user wishes to know the calculated baud rate, then pass it back */
                if (calculatedBaudRate != NULL)
                {
                    *calculatedBaudRate = baudRate;
                }
                /* Write the TCR for this transfer. */
                spi_tx_cmd_config_t txCmdCfg = { .frameSize = spiState->bitsPerFrame,
                                                    .width = spiConfig->width,
                                                    .txMask = false,
                                                    .rxMask = false,
                                                    .contCmd = false,
                                                    .contTransfer = spiConfig->isPcsContinuous,
                                                    .byteSwap = false,
                                                    .lsbFirst = spiConfig->lsbFirst,
                                                    .whichPcs = spiConfig->whichPcs,
                                                    .preDiv = tcrPrescaleValue,
                                                    .clkPhase = spiConfig->clkPhase,
                                                    .clkPolarity = spiConfig->clkPolarity };
                SPI_SetTxCommandReg(base, &txCmdCfg);
            }
        }
    }

    return status;
}

/*!
 * @brief Perform a polling (busy-wait) full-duplex master transfer.
 */
status_t SPI_DRV_MasterTransferPolling(uint32_t instance,
                                       const uint8_t *sendBuffer,
                                       uint8_t *receiveBuffer,
                                       uint16_t transferByteCount)
{
    DEV_ASSERT(instance < SPI_INSTANCE_COUNT);
    DEV_ASSERT(g_spiStatePtr[instance] != NULL);
    /* Instantiate local variable of type spi_state_t and point to global state */
    spi_state_t *spiState = g_spiStatePtr[instance];
    SPI_Type *base = g_spiBase[instance];
    status_t status = STATUS_SUCCESS;


    /* If the transfer count is zero, then return immediately.*/
    if (transferByteCount == (uint16_t)0)
    {
        status = STATUS_SUCCESS;
    }else{
        
        /* Check if another transfer is in progress */
        if (SPI_GetStatusFlag(base, SPI_MODULE_BUSY))
        {
            status = STATUS_BUSY;
        }else{
            
#if !defined(FEATURE_SPI_LITE_VERSION)
            /* Clean RX and TX buffers */
            SPI_SetFlushFifoCmd(base, true, true);
            /* The second flush command is used to avoid the case when one word is still in shifter. */
            SPI_SetFlushFifoCmd(base, true, true);
            if (spiState->isPcsContinuous == true)
            {
                SPI_SetContBit(base);
            }
#endif /* FEATURE_SPI_LITE_VERSION */
        
            /* Configure rxCount depending on transfer type.*/
            if (receiveBuffer == NULL)
            {
                spiState->rxCount = 0;
                SPI_SetRxmskBit(base);
            }
            else
            {
                spiState->rxCount = transferByteCount;
                SPI_ClearRxmaskBit(base);
            }
        
#if !defined(FEATURE_SPI_LITE_VERSION)
            /* Configure watermarks */
            SPI_SetRxWatermarks(base, 0U);
            SPI_SetTxWatermarks(base, 2U);
#endif /* FEATURE_SPI_LITE_VERSION */
            /* Fill out the other members of the run-time state structure. */
            spiState->txBuff = (const uint8_t *)sendBuffer;
            spiState->rxBuff = (uint8_t *)receiveBuffer;
            spiState->txFrameCnt = 0;
            spiState->rxFrameCnt = 0;
            spiState->txCount = transferByteCount;
            /*For continuous mode an extra word must be written to negate the PCS */
            if (spiState->isPcsContinuous == true)
            {
                spiState->txCount++; /* PRQA S 3387 */
            }
            while (spiState->txCount != (uint16_t)0)
            {
                /* Receive data */
                if (SPI_GetStatusFlag(base, SPI_RX_DATA_FLAG))
                {
                    if (spiState->rxCount != (uint16_t)0)
                    {
                        SPI_DRV_ReadRXBuffer(instance);
                    }
                }
                /* Transmit data */
                if (SPI_GetStatusFlag(base, SPI_TX_DATA_FLAG))
                {
                    if ((spiState->txCount != (uint16_t)0))
                    {
                        SPI_DRV_FillupTxBuffer(instance);
                    }
                }
            }
            while (spiState->rxCount != (uint16_t)0)
            {
                /* Read the last word from the RX FIFO */
                if (SPI_GetStatusFlag(base, SPI_RX_DATA_FLAG))
                {
                    SPI_DRV_ReadRXBuffer(instance);
                }
            }
            /* Wait all transfer ends */
            while (true == SPI_GetStatusFlag(base, SPI_MODULE_BUSY))
            {
            }
        }
    }

    return status;
}

/*!
 * @brief Perform an interrupt-driven blocking full-duplex master transfer.
 */
status_t SPI_DRV_MasterTransferBlocking(uint32_t instance,
                                        const uint8_t *sendBuffer,
                                        uint8_t *receiveBuffer,
                                        uint16_t transferByteCount,
                                        uint32_t timeout)
{
    DEV_ASSERT(instance < SPI_INSTANCE_COUNT);
    DEV_ASSERT(g_spiStatePtr[instance] != NULL);
    /* Instantiate local variable of type spi_state_t and point to global state */
    spi_state_t *spiState = g_spiStatePtr[instance];
    SPI_Type *base = g_spiBase[instance];
    status_t status = STATUS_SUCCESS;
    /* If the transfer count is zero, then return immediately.*/
    if (transferByteCount == (uint16_t)0)
    {
        status = STATUS_SUCCESS;
    }else{
        
        /* Check if another transfer is in progress */
        if (SPI_GetStatusFlag(base, SPI_MODULE_BUSY))
        {
            status = STATUS_BUSY;
        }else{
            
            /* Dummy wait to ensure the semaphore is 0, no need to check result */
            (void)OSIF_SemaWait(&(spiState->spiSemaphore), 0);
            spiState->isBlocking = true;
        
            status = SPI_DRV_MasterStartTransfer(instance, sendBuffer, receiveBuffer, transferByteCount);
            /* Start the transfer process, if it returns an error code, return this back to user */
            if (status != STATUS_SUCCESS)
            {
                /* Disable interrupt requests*/
                SPI_SetIntMode(base, SPI_TX_DATA_FLAG, false);
                SPI_SetIntMode(base, SPI_RX_DATA_FLAG, false);
        
                SPI_DRV_DisableTEIEInterrupts(instance);
                SPI_SetIntMode(base, SPI_TRANSFER_COMPLETE, false);
                (void)SPI_ClearStatusFlag(base, SPI_TRANSFER_COMPLETE);
        
                spiState->isBlocking = false;
            }else{
                
                /* As this is a synchronous transfer, wait until the transfer is complete.*/
                status = OSIF_SemaWait(&(spiState->spiSemaphore), timeout);
            
                /* If a timeout occurs, stop the transfer by setting the isTransferInProgress to false and
                 * disabling interrupts, then return the timeout error status.
                 */
                if (status == STATUS_TIMEOUT)
                {
                    /* Set isBlocking variable to false to avoid dummy semaphore post. */
                    spiState->isBlocking = false;
                    /* Complete transfer. */
                    SPI_DRV_MasterCompleteTransfer(instance);
                    status = STATUS_TIMEOUT;
                }else{
                    
                    SPI_DRV_DisableTEIEInterrupts(instance);
                    SPI_SetIntMode(base, SPI_TRANSFER_COMPLETE, false);
                    (void)SPI_ClearStatusFlag(base, SPI_TRANSFER_COMPLETE);          
                }
            }
        }
    }

    return status;
}

/*!
 * @brief Start a non-blocking full-duplex master transfer.
 */
status_t
SPI_DRV_MasterTransfer(uint32_t instance, const uint8_t *sendBuffer, uint8_t *receiveBuffer, uint16_t transferByteCount)
{
    DEV_ASSERT(instance < SPI_INSTANCE_COUNT);
    DEV_ASSERT(g_spiStatePtr[instance] != NULL);
    status_t status = STATUS_SUCCESS;
    /* If the transfer count is zero, then return immediately.*/
    if (transferByteCount == (uint16_t)0)
    {
        status = STATUS_SUCCESS;
    }else{
        
        /* Start the transfer process, if it returns an error code, return this back to user */
        status = SPI_DRV_MasterStartTransfer(instance, sendBuffer, receiveBuffer, transferByteCount);
    }

    /* Else, return immediately as this is an async transfer */
    return status;
}

/*!
 * @brief Query the status of an ongoing non-blocking master transfer.
 */
status_t SPI_DRV_MasterGetTransferStatus(uint32_t instance, uint32_t *bytesRemained)
{
    DEV_ASSERT(instance < SPI_INSTANCE_COUNT);
    DEV_ASSERT(g_spiStatePtr[instance] != NULL);
    status_t status = STATUS_SUCCESS;
    /* Instantiate local variable of type spi_state_t and point to global state */
    const spi_state_t *spiState = g_spiStatePtr[instance];
    /* Fill in the bytes transferred.*/
    if (bytesRemained != NULL)
    {
        *bytesRemained = spiState->rxCount;
    }
    if (spiState->status == SPI_TRANSFER_OK)
    {
        status = (status_t)(spiState->isTransferInProgress ? STATUS_BUSY : STATUS_SUCCESS);
    }
    else
    {
        status = STATUS_ERROR;
    }
    
    return status;
}

/*!
 * @brief Abort an in-progress non-blocking master transfer.
 */
status_t SPI_DRV_MasterAbortTransfer(uint32_t instance)
{
    DEV_ASSERT(instance < SPI_INSTANCE_COUNT);
    DEV_ASSERT(g_spiStatePtr[instance] != NULL);
    /* Stop the running transfer. */
    SPI_DRV_MasterCompleteTransfer(instance);
#if !defined(FEATURE_SPI_LITE_VERSION)
    SPI_Type *base = g_spiBase[instance];
    SPI_SetFlushFifoCmd(base, true, true);
    /* The second flush command is used to avoid the case when one word is still in shifter. */
    SPI_SetFlushFifoCmd(base, true, true);
#endif /* FEATURE_SPI_LITE_VERSION */
    return STATUS_SUCCESS;
}

/*!
 * @brief Select the active chip select signal and its polarity.
 */
status_t SPI_DRV_SetPcs(uint32_t instance, spi_which_pcs_t whichPcs, spi_signal_polarity_t polarity)
{
    DEV_ASSERT(instance < SPI_INSTANCE_COUNT);
    DEV_ASSERT(g_spiStatePtr[instance] != NULL);
#if defined(FEATURE_SPI_HAS_EXTERNAL_DEVICES_SELECTION) && (FEATURE_SPI_HAS_EXTERNAL_DEVICES_SELECTION > 4)
    DEV_ASSERT((uint32_t)whichPcs < FEATURE_SPI_HAS_EXTERNAL_DEVICES_SELECTION);
#else
    DEV_ASSERT((uint32_t)whichPcs < 4U);
#endif

    SPI_Type *base = g_spiBase[instance];
    status_t status = STATUS_SUCCESS;

    if (SPI_Disable(base) != STATUS_SUCCESS)
    {
        status = STATUS_ERROR;
    }else{

        status = SPI_SetPcsPolarityMode(base, whichPcs, polarity);
        if (status == STATUS_SUCCESS)
        {
            SPI_Enable(base);
            SPI_SetPcs(base, whichPcs);
        }
    }

    return status;
}

/*!
 * @brief Configure and start a new SPI master transfer (internal).
 */
static status_t SPI_DRV_MasterStartTransfer(uint32_t instance,
                                            const uint8_t *sendBuffer,
                                            uint8_t *receiveBuffer,
                                            uint16_t transferByteCount)
{
    DEV_ASSERT(instance < SPI_INSTANCE_COUNT);
    DEV_ASSERT(g_spiStatePtr[instance] != NULL);
    status_t status = STATUS_SUCCESS;
    /* Instantiate local variable of type spi_state_t and point to global state */
    spi_state_t *spiState = g_spiStatePtr[instance];
    SPI_Type *base = g_spiBase[instance];
#if defined(FEATURE_SPI_HAS_DMA_ENABLE) && (FEATURE_SPI_HAS_DMA_ENABLE > 0U)
    dma_transfer_size_t dmaTransferSize = DMA_TRANSFER_SIZE_1B;
#endif
    /* Check if TX mask and continuous mode are enabled simultaneously. */
    bool isTxMaskContinuousMode = (sendBuffer == NULL) && (spiState->isPcsContinuous == true);

    /* Check that we're not busy. */
    if (SPI_GetStatusFlag(base, SPI_MODULE_BUSY)
#ifdef SPI_CTRL_I2SMOD_MASK
    && (!SPI_IsI2SMOD(base))
#endif
    )
    {
        status = STATUS_BUSY;
    }else {

        /* Verify if the number of bytes is divided by number of bytes/frame. */
        if ((transferByteCount % spiState->bytesPerFrame) != (uint16_t)0)
        {
            status = STATUS_ERROR;
        }else{
            
#if !defined(FEATURE_SPI_LITE_VERSION)
                /* Clean RX and TX buffers */
                SPI_SetFlushFifoCmd(base, true, true);
                /* The second flush command is used to avoid the case when one word is still in shifter. */
                SPI_SetFlushFifoCmd(base, true, true);
                if (spiState->isPcsContinuous == true)
                {
                    SPI_SetContBit(base);
                }
#endif /* FEATURE_SPI_LITE_VERSION */
            
                spiState->status = SPI_TRANSFER_OK;
                /* Clear all interrupts sources */
                (void)SPI_ClearStatusFlag(base, SPI_ALL_STATUS);
                if (!isTxMaskContinuousMode)
                {
                    /* Enable fault interrupts sources */
                    SPI_SetIntMode(base, SPI_TRANSMIT_ERROR, true);
                }
                if (receiveBuffer != NULL)
                {
                    SPI_SetIntMode(base, SPI_RECEIVE_ERROR, true);
                }
            
                /* Configure rxCount depending on transfer type.*/
                if (receiveBuffer == NULL)
                {
                    spiState->rxCount = 0;
                    SPI_SetRxmskBit(base);
                }
                else
                {
                    spiState->rxCount = transferByteCount;
                    SPI_ClearRxmaskBit(base);
                }
            
#if !defined(FEATURE_SPI_LITE_VERSION)
                /* Configure watermarks */
                SPI_SetRxWatermarks(base, 0U);
                SPI_SetTxWatermarks(base, 2U);
#endif /* FEATURE_SPI_LITE_VERSION */
            
                if (spiState->transferType == SPI_USING_INTERRUPTS)
                {
                    /* Fill out the other members of the run-time state structure. */
                    spiState->txBuff = (const uint8_t *)sendBuffer;
                    spiState->rxBuff = (uint8_t *)receiveBuffer;
                    spiState->txFrameCnt = 0;
                    spiState->rxFrameCnt = 0;
                    spiState->txCount = isTxMaskContinuousMode ? 0 : transferByteCount;
                    /*For continuous mode an extra word must be written to negate the PCS */
                    if ((spiState->isPcsContinuous == true) && (!isTxMaskContinuousMode))
                    {
                        spiState->txCount++; /* PRQA S 3387 */
                    }
            
                    /* Update transfer status */
                    spiState->isTransferInProgress = true;
                    /* Enable RDF interrupt if RX buffer is not NULL. */
                    if (spiState->rxBuff != NULL)
                    {
                        SPI_SetIntMode(base, SPI_RX_DATA_FLAG, true);
                    }

                    if(isTxMaskContinuousMode)
                    {
                        SPI_SetTxmskBit(base); 
                    }
                    else 
                    {
                        /* Enable the TDF and RDF interrupt. */
                        SPI_SetIntMode(base, SPI_TX_DATA_FLAG, true);
                    }
                }
#if defined(FEATURE_SPI_HAS_DMA_ENABLE) && (FEATURE_SPI_HAS_DMA_ENABLE > 0U)
                else
                {
                    /* When SPI use DMA frames with 3 bytes size are not accepted. */
                    switch (spiState->bytesPerFrame)
                    {
                        case 1:
                            dmaTransferSize = DMA_TRANSFER_SIZE_1B;
                            break;
                        case 2:
                            dmaTransferSize = DMA_TRANSFER_SIZE_2B;
                            break;
                        case 4:
                            dmaTransferSize = DMA_TRANSFER_SIZE_4B;
                            break;
                        default:
                            dmaTransferSize = DMA_TRANSFER_SIZE_4B;
                            break;
                    }
                    /* Configure TX DMA channel */
                    if (sendBuffer != NULL)
                    {
                        (void)DMA_DRV_ConfigMultiBlockTransfer(spiState->txDMAChannel,
                                                               DMA_TRANSFER_MEM2PERIPH,
                                                               (uint32_t)sendBuffer,
                                                               (uint32_t)(&(base->DATA)),
                                                               dmaTransferSize,
                                                               (uint32_t)1U << (uint8_t)(dmaTransferSize),
                                                               (uint32_t)transferByteCount /
                                                                   (uint32_t)((uint32_t)1U << (uint8_t)(dmaTransferSize)),
                                                               true);
                    }
                    else
                    {
                        if(isTxMaskContinuousMode)
                        {
                            SPI_SetTxmskBit(base); 
                        }
                        else
                        {
                            spiState->dummy = base->TXCFG | SPI_TXCFG_MSKTX_MASK;
                            (void)DMA_DRV_ConfigMultiBlockTransfer(spiState->txDMAChannel,
                                                                DMA_TRANSFER_PERIPH2PERIPH,
                                                                (uint32_t)(&(spiState->dummy)),
                                                                (uint32_t)(&(base->TXCFG)),
                                                                DMA_TRANSFER_SIZE_4B,
                                                                (uint32_t)1U << (uint8_t)(DMA_TRANSFER_SIZE_4B),
                                                                (uint32_t)transferByteCount /
                                                                    (uint32_t)((uint32_t)1U << (uint8_t)(dmaTransferSize)),
                                                                true);
                        }
                    }
                    /* Configure RX DMA channel if is used in current transfer. */
                    if (receiveBuffer != NULL)
                    {
                        (void)DMA_DRV_ConfigMultiBlockTransfer(spiState->rxDMAChannel,
                                                               DMA_TRANSFER_PERIPH2MEM,
                                                               (uint32_t)(&(base->DATA)),
                                                               (uint32_t)receiveBuffer,
                                                               dmaTransferSize,
                                                               (uint32_t)1U << (uint8_t)(dmaTransferSize),
                                                               (uint32_t)transferByteCount /
                                                                   (uint32_t)((uint32_t)1U << (uint8_t)(dmaTransferSize)),
                                                               true);
                        (void)DMA_DRV_InstallCallback(spiState->rxDMAChannel, (SPI_DRV_MasterCompleteRX), (void *)(instance)); /* PRQA S 0326 */
                        /* Start RX channel */
                        (void)DMA_DRV_StartChannel(spiState->rxDMAChannel);
                    }
                    if(!isTxMaskContinuousMode)
                    {            
                        /* If RX buffer is null the transfer is done when all bytes were sent. */
                        (void)DMA_DRV_InstallCallback(spiState->txDMAChannel, (SPI_DRV_MasterCompleteDMATransfer), (void *)(instance)); /* PRQA S 0326 */
                    }
                    /* Start TX channel */
                    (void)DMA_DRV_StartChannel(spiState->txDMAChannel);
                    /* Update transfer status */
                    spiState->isTransferInProgress = true;
                    /* Enable SPI DMA request */
                    if (receiveBuffer != NULL)
                    {
                        SPI_SetRxDmaCmd(base, true);
                    }
                    if (!isTxMaskContinuousMode)
                    {
                        SPI_SetTxDmaCmd(base, true);
                    }
                }
#endif
        }
    }

    return status;
}

/*!
 * @brief Clean up state and hardware after transfer completion (internal).
 */
static void SPI_DRV_MasterCompleteTransfer(uint32_t instance)
{
    /* Instantiate local variable of type spi_state_t and point to global state */
    spi_state_t *spiState = g_spiStatePtr[instance];
    SPI_Type *base = g_spiBase[instance];
    /* The transfer is complete.*/
    spiState->isTransferInProgress = false;
    if (spiState->transferType == SPI_USING_DMA)
    {
#if !defined(FEATURE_SPI_LITE_VERSION)
        /* Disable SPI DMA request */
        SPI_SetRxDmaCmd(base, false);
        SPI_SetTxDmaCmd(base, false);
#endif /* FEATURE_SPI_LITE_VERSION */
    }
    else
    {
        /* Disable (clear) interrupt requests */
        SPI_SetIntMode(base, SPI_RX_DATA_FLAG, false);
        SPI_SetIntMode(base, SPI_TX_DATA_FLAG, false);
    }

    SPI_DRV_DisableTEIEInterrupts(instance);
    SPI_SetIntMode(base, SPI_TRANSFER_COMPLETE, false);
    (void)SPI_ClearStatusFlag(base, SPI_TRANSFER_COMPLETE);
    if (spiState->isBlocking == true)
    {
        (void)OSIF_SemaPost(&(spiState->spiSemaphore));
        spiState->isBlocking = false;
    }
    if (spiState->callback != NULL)
    {
        spiState->callback(spiState, SPI_EVENT_END_TRANSFER, spiState->callbackParam);
    }
}

#if defined(FEATURE_SPI_HAS_DMA_ENABLE) && (FEATURE_SPI_HAS_DMA_ENABLE > 0U)

/*!
 * @brief DMA TX completion callback (internal).
 */
static void SPI_DRV_MasterCompleteDMATransfer(void *parameter, dma_chn_status_t status)
{
    uint32_t instance = (uint32_t)parameter; /* PRQA S 0326 */
    spi_state_t *spiState = g_spiStatePtr[instance];
    SPI_Type *base = g_spiBase[instance];

    if (status == DMA_CHN_ERROR)
    {
        (void)SPI_DRV_MasterAbortTransfer(instance);
        spiState->status = SPI_TRANSMIT_FAIL;
    }
    else
    {
        if (spiState->isPcsContinuous == true)
        {
            SPI_ClearContBit(base);
        }

        /* Enable transfer complete flag interrupt to catch the end of the transfer. */
        spiState->txCount = 0;
        spiState->rxCount = 0;
        SPI_SetIntMode(base, SPI_TRANSFER_COMPLETE, true);
    }
}

#endif /* FEATURE_SPI_HAS_DMA_ENABLE */

#if defined(FEATURE_SPI_HAS_DMA_ENABLE) && (FEATURE_SPI_HAS_DMA_ENABLE > 0U)

/*!
 * @brief DMA RX completion callback (internal).
 */
static void SPI_DRV_MasterCompleteRX(void *parameter, dma_chn_status_t status)
{
    uint32_t instance = (uint32_t)parameter; /* PRQA S 0326 */
    spi_state_t *spiState = g_spiStatePtr[instance];
    SPI_Type *base = g_spiBase[instance];
    bool isTxMaskContinuousMode = (spiState->txBuff == NULL) && (spiState->isPcsContinuous == true);
    if(isTxMaskContinuousMode)
    {
        SPI_SetFlushFifoCmd(base, true, false);
        SPI_ClearTxmaskBit(base);
        SPI_DRV_MasterCompleteTransfer(instance);
    }
    if (status == DMA_CHN_ERROR)
    {
        (void)SPI_DRV_MasterAbortTransfer(instance);
        spiState->status = SPI_TRANSMIT_FAIL;
    }
}

#endif /* FEATURE_SPI_HAS_DMA_ENABLE */

/*!
 * @brief Interrupt handler for SPI master mode.
 */
void SPI_DRV_MasterIRQHandler(uint32_t instance)
{
    /* Instantiate local variable of type spi_state_t and point to global state */
    spi_state_t *spiState = g_spiStatePtr[instance];
    SPI_Type *base = g_spiBase[instance];
     /* Check if TX mask and continuous mode are enabled simultaneously. */
     bool isTxMaskContinuousMode = (spiState->txBuff == NULL) && (spiState->isPcsContinuous == true);

    /* If an error is detected the transfer will be aborted */
    if ((bool)SPI_GetStatusFlag(base, SPI_TRANSMIT_ERROR) && (spiState->txBuff != NULL))
    {
        (void)SPI_DRV_MasterAbortTransfer(instance);
        (void)SPI_ClearStatusFlag(base, SPI_TRANSMIT_ERROR);
        spiState->status = SPI_TRANSMIT_FAIL;
    }else {

        if (SPI_GetStatusFlag(base, SPI_RECEIVE_ERROR) && (spiState->rxBuff != NULL))
        {
            (void)SPI_DRV_MasterAbortTransfer(instance);
            (void)SPI_ClearStatusFlag(base, SPI_RECEIVE_ERROR);
            spiState->status = SPI_RECEIVE_FAIL;
        }else {

            /* RECEIVE IRQ handler: Check read buffer only if there are remaining bytes to read. */
            if (SPI_GetStatusFlag(base, SPI_RX_DATA_FLAG))
            {
                if (spiState->rxCount != (uint16_t)0)
                {
                    SPI_DRV_ReadRXBuffer(instance);
                }
            }
            /* Transmit data */
            if (SPI_GetStatusFlag(base, SPI_TX_DATA_FLAG))
            {
                if ((spiState->txCount != (uint16_t)0))
                {
                    SPI_DRV_FillupTxBuffer(instance);
                }
            }
            if (spiState->txCount == (uint16_t)0)
            {
                if (isTxMaskContinuousMode)
                {
                    /* In TX mask continuous mode, complete transfer when all data is received */
                    if (spiState->rxCount == (uint16_t)0)
                    {
                        SPI_ClearTxmaskBit(base);
                        SPI_DRV_MasterCompleteTransfer(instance);
                        NVIC_ClearPendingIRQ(g_spiIrqId[instance]);
                    }
                }
                else
                {
                    /* Disable TX flag. Software buffer is empty.*/
                    SPI_SetIntMode(base, SPI_TX_DATA_FLAG, false);
                    SPI_SetIntMode(base, SPI_TRANSFER_COMPLETE, true);
            
                    /* Check if we're done with this transfer.*/
                    if ((spiState->rxCount == (uint16_t)0) &&
                        (SPI_GetStatusFlag(base, SPI_TRANSFER_COMPLETE) == true))
                    {
                        SPI_DRV_MasterCompleteTransfer(instance);
                    }
                }
            }
        }
    }
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
