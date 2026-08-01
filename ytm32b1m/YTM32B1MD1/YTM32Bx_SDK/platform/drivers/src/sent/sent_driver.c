/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file sent_driver.c
 * @version 1.4.1
 * @brief SENT driver implementation — SAE J2716 receiver for the YTMicro SDK.
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 * 
 * PRQA S 0316 Rule 11.5: Cast from a pointer to void to a pointer to object type.
 *
 */

#include <stddef.h>
#include "device_registers.h"
#include "sent_driver.h"
#include "status.h"
#include "clock_manager.h"
#include "sent_hw_access.h"
#include "dma_driver.h"
#include "interrupt_manager.h"
#include "sent_irq.h"

/*! @cond DRIVER_INTERNAL_USE_ONLY */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! Number of input-clock cycles in one microsecond. */
#define SENT_ONE_MICROSECOND_CYCLES (1000000u)

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Private data
 ******************************************************************************/

/*! Clock name array indexed by SENT peripheral instance number. */
static const clock_names_t s_sentClockNames[SENT_INSTANCE_COUNT] = FEATURE_SENT_CLOCK_NAMES;

/*! DMA request source for the Fast channel, indexed by peripheral instance. */
static const dma_request_source_t s_sentFastDMASrc[SENT_INSTANCE_COUNT] = FEATURE_SENT_FAST_DMA_REQS;

/*! DMA request source for the Slow channel, indexed by peripheral instance. */
static const dma_request_source_t s_sentSlowDMASrc[SENT_INSTANCE_COUNT] = FEATURE_SENT_SLOW_DMA_REQS;

/*! Runtime state pointers, one entry per SENT peripheral instance. */
static sent_state_t * s_sentStatePtr[SENT_INSTANCE_COUNT];

/*!
 * @brief Base DMA transfer configuration template for SENT channels.
 *
 * Fields annotated with "Modified at runtime" are overwritten before each
 * DMA_DRV_ConfigLoopTransfer() call and must not be treated as fixed values.
 */
static const dma_transfer_config_t sentDmaBaseConfig =
{
    .srcAddr = 0u,                              /*!< Modified at runtime */
    .destAddr = 0u,                             /*!< Modified at runtime */
    .srcTransferSize = DMA_TRANSFER_SIZE_4B,
    .destTransferSize = DMA_TRANSFER_SIZE_4B,
    .srcOffset = 0,                             /*!< Modified at runtime */
    .destOffset = (int16_t)sizeof(uint32_t),
    .srcLastAddrAdjust = 0,                     /*!< Modified at runtime */
    .destLastAddrAdjust = 0,                    /*!< Modified at runtime */
    .srcModulo = DMA_MODULO_OFF,
    .destModulo = DMA_MODULO_OFF,
    .transferLoopByteCount = 0,                 /*!< Modified at runtime */
    .ramReloadEnable = false,
    .ramReloadNextDescAddr = 0u,
    .interruptEnable = true,
    .loopTransferConfig = &(dma_loop_transfer_config_t)
    {
        .triggerLoopIterationCount = 1u,
        .srcOffsetEnable = true,
        .dstOffsetEnable = true,
        .triggerLoopOffset = 0,
        .transferLoopChnLinkEnable = false,
        .transferLoopChnLinkNumber = 0u,
        .triggerLoopChnLinkEnable = false,
        .triggerLoopChnLinkNumber = 0u
    }
};

/*******************************************************************************
 * Private functions
 ******************************************************************************/

/*!
 * @brief Configure the diagnostics hardware for one channel.
 */
static void SENT_DRV_InitDiag(const uint32_t instance, const uint8_t channel, const sent_diag_config_t * config)
{
    /* Set bus IDLE count */
    SENT_DRV_HW_SetBusIdleCnt(instance, channel, config->idleCount);

    /* Set calibration range */
    SENT_DRV_HW_SetCalRng(instance, channel, config->calibVar);

    /* Set pause pulse diag check */
    SENT_DRV_HW_SetPpChkSel(instance, channel, config->diagPulse);

    /* Set pause pulse enable status */
    SENT_DRV_HW_SetPausePulseEnable(instance, channel, config->pausePulse);

    /* Successive calibration check */
    SENT_DRV_HW_SetSuccCalChk(instance, channel, config->successiveCal);
}

/*!
 * @brief Configure the Slow message reception hardware for one channel.
 */
static void SENT_DRV_InitSlowMsg(const uint32_t instance, const uint8_t channel, const sent_slow_msg_config_t * config)
{
    /* CRC related */
    SENT_DRV_HW_SetSlowCrcType(instance, channel, config->crcType);
}

#if FEATURE_SENT_HAS_SPC
/*!
 * @brief Configure the SPC hardware for one channel.
 */
static void SENT_DRV_InitSPC(const uint32_t instance, const uint8_t channel, const sent_spc_config_t * config)
{
    /* SPC configuration */
    SENT_DRV_HW_ConfigSPC(instance, channel, config);
}

/*!
 * @brief Generate a software trigger for SPC pulse.
 */
void SENT_DRV_SwTrigger(uint32_t instance, uint8_t channel)
{
    SENT_DRV_HW_SPCSwTrigger(instance, channel);
}
#endif /* FEATURE_SENT_HAS_SPC */

/*!
 * @brief Configure the Fast message reception hardware for one channel.
 */
static void SENT_DRV_InitFastMsg(const uint32_t instance, const uint8_t channel, const sent_fast_msg_config_t * config)
{
    /* Number of nibbles */
    SENT_DRV_HW_SetFastNumNibbles(instance, channel, config->numberOfNibbles);

    /* CRC related */
    SENT_DRV_HW_SetFastCrcType(instance, channel, config->crcType);
    SENT_DRV_HW_SetFastCrcIncStatus(instance, channel, config->crcIncludeStatus);
    SENT_DRV_HW_SetFastDisableCrc(instance, channel, config->disableCrcCheck);
}

/*!
 * @brief Compute the global timestamp prescaler from the input clock frequency.
 */
static uint8_t SENT_DRV_ComputeTimestampPrescaler(const uint32_t inputClock)
{
    uint32_t tsPre;

    /* 0 in worst case */
    tsPre = (uint32_t)(inputClock / SENT_ONE_MICROSECOND_CYCLES) - 1u;
    /* Saturate at upper interval */
    tsPre = (tsPre > 255u) ? 255u : tsPre;

    return (uint8_t)tsPre;
}

/*!
 * @brief Compute the per-channel clock prescaler for the requested tick duration.
 */
static uint32_t SENT_DRV_ComputeChannelPrescaler(const uint8_t reqTick, const uint32_t inputClock)
{
    uint32_t chPre;

    /* Compute and set the channel prescaler */
    chPre = inputClock / 1000u; /* Keep info on fractional frequencies */

    chPre = (reqTick * chPre) / 1000u;

    return chPre;
}

/*!
 * @brief DMA completion callback for the Fast message channel.
 */
static void SENT_DRV_FastRxCompleteDma(void * parameter, dma_chn_status_t status)
{
    uint8_t instance = *(const uint8_t *)parameter; /*PRQA S 0316*/

    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);

    if (status == DMA_CHN_NORMAL)
    {
        DEV_ASSERT(s_sentStatePtr[instance] != NULL);

        /* Call the notification */
        if(s_sentStatePtr[instance]->callbackFunc.function != NULL)
        {
            s_sentStatePtr[instance]->callbackFunc.function(instance, 0u, /* Channel always passed as 0 in DMA mode */
                    SENT_CALLBACK_FAST_DMA_RX_COMPLETE, s_sentStatePtr[instance]->callbackFunc.param);
        }
    }
}

/*!
 * @brief DMA completion callback for the Slow message channel.
 */
static void SENT_DRV_SlowRxCompleteDma(void * parameter, dma_chn_status_t status)
{
    uint8_t instance = *(const uint8_t *)parameter; /*PRQA S 0316*/

    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);

    if (status == DMA_CHN_NORMAL)
    {
        DEV_ASSERT(s_sentStatePtr[instance] != NULL);

        /* Call the notification */
        if(s_sentStatePtr[instance]->callbackFunc.function != NULL)
        {
            s_sentStatePtr[instance]->callbackFunc.function(instance, 0u, /* Channel always passed as 0 in DMA mode */
                    SENT_CALLBACK_SLOW_DMA_RX_COMPLETE, s_sentStatePtr[instance]->callbackFunc.param);
        }
    }
}

/*!
 * @brief Configure a looping DMA transfer for the given channel.
 */
static void SENT_DRV_ConfigureDmaTransfer(const uint8_t channel, const uint32_t srcAddr,
                                         const uint32_t destAddr, const bool hasFifo,
                                         const uint8_t lenFifo)
{
    /* Generate the structure. Passing it on the stack is OK
     * since the values are copied to peripheral registers */
    dma_transfer_config_t transferConfig = sentDmaBaseConfig;

    /* Modify source and destination */
    transferConfig.srcAddr = srcAddr;
    transferConfig.destAddr = destAddr;

#ifdef FEATURE_SENT_DMA_HAS_FIFO
    /* Depending on case, configure the parameters */
    if(hasFifo)
    {
        /* Valid values are between 1 and FEATURE_SENT_MAX_FIFO_SIZE */
        uint8_t locFifoLen = (lenFifo < 1u) ? 1u : ((lenFifo > FEATURE_SENT_MAX_FIFO_SIZE) ? FEATURE_SENT_MAX_FIFO_SIZE : lenFifo);

        transferConfig.transferLoopByteCount = locFifoLen * sizeof(sent_raw_msg_t);
        transferConfig.destLastAddrAdjust = -((int32_t)locFifoLen * (int32_t)sizeof(sent_raw_msg_t));
        transferConfig.srcOffset = 0;
        transferConfig.srcLastAddrAdjust = 0;
    }
    else
    {
        transferConfig.transferLoopByteCount = sizeof(sent_raw_msg_t);
        transferConfig.destLastAddrAdjust = -((int32_t)sizeof(sent_raw_msg_t));
        transferConfig.srcOffset = (int16_t)(sizeof(uint32_t));
        transferConfig.srcLastAddrAdjust = -((int32_t)sizeof(sent_raw_msg_t));
    }
#else /* Parts with no FIFO feature */
    (void)hasFifo;
    (void)lenFifo;

    transferConfig.transferLoopByteCount = sizeof(sent_raw_msg_t);
    transferConfig.destLastAddrAdjust = -((int32_t)sizeof(sent_raw_msg_t));
    transferConfig.srcOffset = (int16_t)(sizeof(uint32_t));
    transferConfig.srcLastAddrAdjust = -((int32_t)sizeof(sent_raw_msg_t));
#endif /* FEATURE_SENT_DMA_HAS_FIFO */

    /* Configure a loop transfer */
    (void)DMA_DRV_ConfigLoopTransfer(channel, &transferConfig);
}

/*!
 * @brief Install or remove interrupt and DMA callbacks for all active channels.
 */
static void SENT_DRV_InstallCallbacks(const uint32_t instance)
{
    uint32_t chInd;
    bool fastDmaSet = false;
    bool slowDmaSet = false;
    sent_state_t * lState;

    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);
    DEV_ASSERT(s_sentStatePtr[instance] != NULL);

    lState = s_sentStatePtr[instance];

    /* Check for valid function first */
    if(lState->callbackFunc.function != NULL)
    {
        for(chInd = 0u; chInd < SENT_CH_COUNT; chInd++)
        {
            /* Only on active channel */
            if(lState->activeChannels[chInd])
            {
                /* Rx events only if no bits are set in mask */
                if((lState->channelEvents[chInd] & SENT_EV_ALL) != 0u)
                {
                    SENT_DRV_HW_SetEventConfig(lState->instanceId, (uint8_t)chInd, lState->channelEvents[chInd]);
                    SENT_DRV_IRQ_EnableIRQ(lState->instanceId, chInd, SENT_IRQ_ERROR);
                }
                else
                {
                    SENT_DRV_HW_SetEventConfig(lState->instanceId, (uint8_t)chInd, SENT_EV_NONE);
                    SENT_DRV_IRQ_DisableIRQ(lState->instanceId, chInd, SENT_IRQ_ERROR);
                }

                /* Fast events */
                if(lState->fastDmaEnabled[chInd])
                {
                    /* Mark to be set later */
                    fastDmaSet = true;

                    /* This means that there will be no more fast Channel Rx interrupts since DMA is enabled. */
                    SENT_DRV_HW_SetFastRxInterrupt(lState->instanceId, (uint8_t)chInd, false);
                    SENT_DRV_IRQ_DisableIRQ(lState->instanceId, chInd, SENT_IRQ_FAST);
                }
                else
                {
                    /* Enable channel interrupt, since DMA is not enabled */
                    SENT_DRV_HW_SetFastRxInterrupt(lState->instanceId, (uint8_t)chInd, true);
                    SENT_DRV_IRQ_EnableIRQ(lState->instanceId, chInd, SENT_IRQ_FAST);
                }

                /* Slow events */
                if(lState->slowDmaEnabled[chInd])
                {
                    /* Mark to be set later */
                    slowDmaSet = true;

                    /* This means that there will be no more fast Channel Rx interrupts since DMA is enabled. */
                    SENT_DRV_HW_SetSlowRxInterruptStatus(lState->instanceId, (uint8_t)chInd, false);
                    SENT_DRV_IRQ_DisableIRQ(lState->instanceId, chInd, SENT_IRQ_SLOW);
                }
                else
                {
                    /* Enable channel interrupt, since DMA is not enabled */
                    SENT_DRV_HW_SetSlowRxInterruptStatus(lState->instanceId, (uint8_t)chInd, true);
                    SENT_DRV_IRQ_EnableIRQ(lState->instanceId, chInd, SENT_IRQ_SLOW);
                }
            }
        }

        /* Fast DMA callbacks (global for all channels) */
        if(fastDmaSet)
        {
            (void)DMA_DRV_InstallCallback(lState->fastDmaChannel,
                                           &SENT_DRV_FastRxCompleteDma,
                                           &lState->instanceId);
        }

        /* Slow DMA callbacks (global for all channels) */
        if(slowDmaSet)
        {
            (void)DMA_DRV_InstallCallback(lState->slowDmaChannel,
                                           &SENT_DRV_SlowRxCompleteDma,
                                           &lState->instanceId);
        }
    }
    else /* if(lState->callbackFunc.function != NULL) Disable all callbacks in case the function was removed */
    {
        for(chInd = 0u; chInd < SENT_CH_COUNT; chInd++)
        {
            /* Only on active channel */
            if(lState->activeChannels[chInd])
            {
                /* Fast events */
                if(lState->fastDmaEnabled[chInd])
                {
                    /* Mark to be set later */
                    fastDmaSet = true;
                }

                /* Slow events */
                if(lState->slowDmaEnabled[chInd])
                {
                    /* Mark to be set later */
                    slowDmaSet = true;
                }

                /* Disable Diagnostics interrupts */
                SENT_DRV_HW_SetEventConfig(lState->instanceId, (uint8_t)chInd, SENT_EV_NONE);
                SENT_DRV_IRQ_DisableIRQ(lState->instanceId, chInd, SENT_IRQ_ERROR);

                /* Disable Fast interrupts */
                SENT_DRV_HW_SetFastRxInterrupt(lState->instanceId, (uint8_t)chInd, false);
                SENT_DRV_IRQ_DisableIRQ(lState->instanceId, chInd, SENT_IRQ_FAST);

                /* Disable slow interrupts */
                SENT_DRV_HW_SetSlowRxInterruptStatus(lState->instanceId, (uint8_t)chInd, false);
                SENT_DRV_IRQ_DisableIRQ(lState->instanceId, chInd, SENT_IRQ_SLOW);
            }
        }

        /* Only if active */
        if(fastDmaSet)
        {
            (void)DMA_DRV_InstallCallback(lState->fastDmaChannel,
                                           ((void *)0u),
                                           &lState->instanceId);
        }

        /* Only if active */
        if(slowDmaSet)
        {
            (void)DMA_DRV_InstallCallback(lState->slowDmaChannel,
                                           ((void *)0u),
                                           &lState->instanceId);
        }
    }
}

/*******************************************************************************
 * IRQ handlers
 ******************************************************************************/

/*!
 * @brief Fast channel IRQ handler — dispatches to the user callback.
 */
void SENT_DRV_IRQ_FastHandler(const uint32_t instance, const uint32_t channel)
{
    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);
    DEV_ASSERT(channel < SENT_CH_COUNT);

    if(s_sentStatePtr[instance] != NULL)
    {
        /* Check if channel is valid first */
        if(s_sentStatePtr[instance]->activeChannels[channel])
        {

#if defined(ERRATA_E7425) /* Enable only when we have this active */
            if (SENT_DRV_HW_ErrataE7425Workaroud(instance, channel))
            {
#endif /* defined(ERRATA_E7425) */

            if(s_sentStatePtr[instance]->callbackFunc.function != NULL)
            {
                s_sentStatePtr[instance]->callbackFunc.function(instance, channel,
                        SENT_CALLBACK_FAST_RX_COMPLETE, s_sentStatePtr[instance]->callbackFunc.param);
            }

#if defined(ERRATA_E7425)
            }
#endif /* defined(ERRATA_E7425) */
        }
    }
}

/*!
 * @brief Slow channel IRQ handler — dispatches to the user callback.
 */
void SENT_DRV_IRQ_SlowHandler(const uint32_t instance, const uint32_t channel)
{
    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);
    DEV_ASSERT(channel < SENT_CH_COUNT);

    if(s_sentStatePtr[instance] != NULL)
    {
        /* Check if channel is valid first */
        if(s_sentStatePtr[instance]->activeChannels[channel])
        {
            if(s_sentStatePtr[instance]->callbackFunc.function != NULL)
            {
                s_sentStatePtr[instance]->callbackFunc.function(instance, channel,
                        SENT_CALLBACK_SLOW_RX_COMPLETE, s_sentStatePtr[instance]->callbackFunc.param);
            }
        }
    }
}

/*!
 * @brief Wakeup event IRQ handler — dispatches to the user callback.
 */
void SENT_DRV_IRQ_WakeupHandler(const uint32_t instance, const uint32_t channel)
{
    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);
    DEV_ASSERT(channel < SENT_CH_COUNT);

    if(s_sentStatePtr[instance] != NULL)
    {
        /* Check if channel is valid first */
        if(s_sentStatePtr[instance]->activeChannels[channel])
        {
            if(s_sentStatePtr[instance]->callbackFunc.function != NULL)
            {
                s_sentStatePtr[instance]->callbackFunc.function(instance, channel,
                        SENT_CALLBACK_RX_WAKEUP_EVENT, s_sentStatePtr[instance]->callbackFunc.param);
            }
        }
    }
}

/*!
 * @brief Receive error IRQ handler — dispatches to the user callback when events are active.
 */
void SENT_DRV_IRQ_RxErrHandler(const uint32_t instance, const uint32_t channel)
{
    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);
    DEV_ASSERT(channel < SENT_CH_COUNT);

    if(s_sentStatePtr[instance] != NULL)
    {
        /* Check if channel is valid first */
        if(s_sentStatePtr[instance]->activeChannels[channel])
        {

#if defined(ERRATA_E7425) /* Enable only when we have this active */
            if (SENT_DRV_HW_ErrataE7425Workaroud(instance, channel))
            {
#endif /* defined(ERRATA_E7425) */
            if((SENT_DRV_HW_GetActiveEvents(instance, (uint8_t)channel) != 0u) &&
               (s_sentStatePtr[instance]->callbackFunc.function != NULL))
            {
                s_sentStatePtr[instance]->callbackFunc.function(instance, channel,
                        SENT_CALLBACK_RX_ERROR, s_sentStatePtr[instance]->callbackFunc.param);
            }

#if defined(ERRATA_E7425)
            }
#endif /* defined(ERRATA_E7425) */
        }
    }
}

/*! @endcond */

/*******************************************************************************
 * API implementation
 ******************************************************************************/

/*!
 * @brief Initialize the SENT driver for a peripheral instance.
 */
status_t SENT_DRV_Init(const uint32_t instance, const sent_driver_user_config_t * configPtr, sent_state_t * state)
{
    uint32_t inputClock;
    uint8_t chInd;
    bool fDmaEn = false;
    bool sDmaEn = false;
    status_t initErr;
    const sent_channel_config_t * chPtr;
    status_t retVal;

    /* Check for correct calling parameters */
    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);
    DEV_ASSERT(configPtr != NULL);
    DEV_ASSERT(configPtr->numOfConfigs <= SENT_CH_COUNT);

    /* Only if not already initialized */
    if(s_sentStatePtr[instance] != NULL)
    {
        retVal = STATUS_ERROR;
    }
    else
    {
        /* Copy the references */
        s_sentStatePtr[instance] = state;

        /* Populate the configuration structure */
        state->fastMsgDmaPtr = configPtr->fastMsgDmaPtr;
        state->slowMsgDmaPtr = configPtr->slowMsgDmaPtr;
        state->fastDmaChannel = configPtr->fastDmaChannel;
        state->slowDmaChannel = configPtr->slowDmaChannel;
        state->callbackFunc = configPtr->callbackFunc;
        state->instanceId = (uint8_t)instance;

        /* Get the protocol clock frequency */
        initErr = CLOCK_SYS_GetFreq(s_sentClockNames[instance], &inputClock);
        DEV_ASSERT(initErr == STATUS_SUCCESS);
        DEV_ASSERT(inputClock >= SENT_ONE_MICROSECOND_CYCLES);
        (void) initErr;

        /* Set register time base */
        SENT_DRV_HW_SetTimestampPrescaler(instance, SENT_DRV_ComputeTimestampPrescaler(inputClock));

        /* Setup the channels */
        for(chInd = 0u; chInd < configPtr->numOfConfigs; chInd++)
        {
            /* Only if we have a valid channel */
            DEV_ASSERT(configPtr->channelConfig[chInd].channelId < SENT_CH_COUNT);

            /* Obtain a pointer to channel config */
            chPtr = &configPtr->channelConfig[chInd];

            /* Mark channel as active */
            state->activeChannels[chPtr->channelId] = true;

            /* Copy DMA status */
            state->fastDmaEnabled[chPtr->channelId] = chPtr->fastMsgConfig.dmaEnable;
            state->slowDmaEnabled[chPtr->channelId] = chPtr->slowMsgConfig.dmaEnable;

            /* Store active events */
            state->channelEvents[chPtr->channelId] = chPtr->diagConfig.diagEvents;

            /* pre-scaler and compensation */
            SENT_DRV_HW_SetChannelPrescaler(instance, chPtr->channelId,
                    (uint16_t)SENT_DRV_ComputeChannelPrescaler(chPtr->tickDuration, inputClock));

            SENT_DRV_HW_SetChannelCompensation(instance, chPtr->channelId, true);

            /* DMA enablers */
            if(chPtr->fastMsgConfig.dmaEnable)
            {
                SENT_DRV_HW_SetFastDma(instance, chPtr->channelId, true);
                fDmaEn = true;
            }

            if(chPtr->slowMsgConfig.dmaEnable)
            {
                SENT_DRV_HW_SetSlowDma(instance, chPtr->channelId, true);
                sDmaEn = true;
            }
#if FEATURE_SENT_HAS_INPUT_FILTER
            /* Setup input filter */
            SENT_DRV_HW_SetBusFilterCnt(instance, chPtr->channelId, chPtr->inputFilter);
#endif /* FEATURE_SENT_HAS_INPUT_FILTER */
            /* Setup diagnostics */
            SENT_DRV_InitDiag(instance, chPtr->channelId, &chPtr->diagConfig);

            /* Setup fast messages */
            SENT_DRV_InitFastMsg(instance, chPtr->channelId, &chPtr->fastMsgConfig);

            /* Setup slow messages */
            SENT_DRV_InitSlowMsg(instance, chPtr->channelId, &chPtr->slowMsgConfig);
#if FEATURE_SENT_HAS_SPC
            /* Setup SPC configuration */
            SENT_DRV_InitSPC(instance, chPtr->channelId, &chPtr->spcConfig);
#endif /* FEATURE_SENT_HAS_SPC */
        }

        /* Enable DMA transfers */
        if(fDmaEn)
        {
            /* Assures correct designation, rather than configuring it from DMA */
            (void)DMA_DRV_SetChannelRequestAndTrigger(state->fastDmaChannel, (uint8_t)s_sentFastDMASrc[instance]);

#ifdef FEATURE_SENT_DMA_HAS_FIFO
            /* Configure the FIFO */
            if(configPtr->fastDmaFIFOEnable)
            {

                SENT_DRV_HW_SetFifoState(instance, true);
                SENT_DRV_HW_SetFifoWm(instance, configPtr->fastDmaFIFOSize);
            }
#endif /* FEATURE_SENT_DMA_HAS_FIFO */

            /* Configure DMA */
            SENT_DRV_ConfigureDmaTransfer(state->fastDmaChannel,
                                         (uint32_t)SENT_DRV_HW_GetFastDmaRegStartAddr(instance),
                                         (uint32_t)state->fastMsgDmaPtr,
                                         configPtr->fastDmaFIFOEnable,
                                         configPtr->fastDmaFIFOSize);

            /* Start channel */
            (void)DMA_DRV_StartChannel(state->fastDmaChannel);
        }

        if(sDmaEn)
        {
            /* Configure DMA */
            (void)DMA_DRV_SetChannelRequestAndTrigger(state->slowDmaChannel, (uint8_t)s_sentSlowDMASrc[instance]);

            /* Configure DMA */
            SENT_DRV_ConfigureDmaTransfer(state->slowDmaChannel,
                                         (uint32_t)SENT_DRV_HW_GetSlowDmaRegStartAddr(instance),
                                         (uint32_t)state->slowMsgDmaPtr,
                                         false, 0u);

            /* Start channel */
            (void)DMA_DRV_StartChannel(state->slowDmaChannel);
        }

        /* Setup notifications */
        SENT_DRV_InstallCallbacks(instance);

        /* Enable channels */
        for(chInd = 0u; chInd < SENT_CH_COUNT; chInd++)
        {
            if(state->activeChannels[chInd])
            {
                /* Enable channel */
                SENT_DRV_HW_SetChannelStatus(instance, chInd, true);
            }
        }

        /* Enable peripheral */
        SENT_DRV_HW_SetPeripheralStatus(instance, true);

        /* Success */
        retVal = STATUS_SUCCESS;
    }

    return retVal;
}

/*!
 * @brief De-initialize the SENT driver and reset peripheral registers.
 */
status_t SENT_DRV_Deinit(const uint32_t instance)
{
    uint8_t indChan;
    status_t retVal;

    /* Invalid instance or channel */
    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);

    if (s_sentStatePtr[instance] != NULL)
    {
        for(indChan = 0u; indChan < SENT_CH_COUNT; indChan++)
        {
            /* Mask interrupts */
            SENT_DRV_IRQ_DisableIRQ(instance, indChan, SENT_IRQ_FAST);
            SENT_DRV_IRQ_DisableIRQ(instance, indChan, SENT_IRQ_SLOW);
            SENT_DRV_IRQ_DisableIRQ(instance, indChan, SENT_IRQ_ERROR);
        }

        /* Reset peripheral*/
        SENT_DRV_HW_ResetPeripheral(instance);

        /* Clear internal data */
        s_sentStatePtr[instance] = NULL;

        retVal = STATUS_SUCCESS;
    }
    else
    {
        retVal = STATUS_ERROR;
    }

    return retVal;
}

/*!
 * @brief Populate a configuration structure with TLE4998 sensor defaults.
 */
void SENT_DRV_GetDefaultConfig(sent_driver_user_config_t * config)
{
    DEV_ASSERT(config->channelConfig != NULL);

    static sent_channel_config_t chConfig =
    {
        .channelId = 0u,
        .tickDuration = 3u,
#if FEATURE_SENT_HAS_INPUT_FILTER    
        .inputFilter = SENT_INPUT_FILTER_NONE,
#endif /* FEATURE_SENT_HAS_INPUT_FILTER */        
        .diagConfig =
        {
            .diagEvents = (
                            SENT_EV_CAL_RESYNC |
                            SENT_EV_CAL_20_25 |
                            SENT_EV_SMSG_OFLW |
                            SENT_EV_FMSG_OFLW |
                            SENT_EV_PP_DIAG_ERR |
                            SENT_EV_CAL_LEN_ERR |
                            SENT_EV_CAL_DIAG_ERR |
                            SENT_EV_NIB_VAL_ERR |
                            SENT_EV_SMSG_CRC_ERR |
                            SENT_EV_FMSG_CRC_ERR |
                            SENT_EV_NUM_EDGES_ERR |
                            SENT_EV_NONE
                        ),
            .idleCount = SENT_BUS_IDLE_DISABLED,
            .calibVar = SENT_CALIB_VAR_25_PERCENT,
            .diagPulse = SENT_PULSE_CHECK_BOTH,
            .pausePulse = SENT_PAUSE_PULSE_ENABLED,
            .successiveCal = SENT_SUCC_CAL_CHK_PREFERRED
        },
        .fastMsgConfig =
        {
            .numberOfNibbles = 6,
            .dmaEnable = false,
            .crcIncludeStatus = false,
            .disableCrcCheck = false,
            .crcType = SENT_CRC_RECOMMENDED
        },
        .slowMsgConfig =
        {
            .dmaEnable = false,
            .crcType = SENT_CRC_RECOMMENDED
        }
    };

    config->fastDmaFIFOSize = 1U;
    config->channelConfig = &chConfig;
    config->numOfConfigs = 1U;
}

/*!
 * @brief Check whether a new Fast message is available.
 */
bool SENT_DRV_GetFastRxStatus(const uint32_t instance, const uint32_t channel)
{
    bool retVal;

    /* Invalid instance or channel */
    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);
    DEV_ASSERT(channel < SENT_CH_COUNT);
    DEV_ASSERT(s_sentStatePtr[instance] != NULL);

    /* Only if channel is enabled */
    if(s_sentStatePtr[instance]->activeChannels[channel] != false)
    {
        retVal = SENT_DRV_HW_GetFastRxStatus(instance, (uint8_t)channel);
    }
    else
    {
        retVal = false;
    }

    return retVal;
}

/*!
 * @brief Read the most recently received Fast message.
 */
status_t SENT_DRV_GetFastMsg(const uint32_t instance, const uint32_t channel, sent_fast_msg_t * message)
{
    sent_raw_msg_t locMsg;
    status_t retVal;

    /* Invalid instance or channel */
    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);
    DEV_ASSERT(channel < SENT_CH_COUNT);
    DEV_ASSERT(s_sentStatePtr[instance] != NULL);

    /* Only if channel is enabled */
    if(s_sentStatePtr[instance]->activeChannels[channel] != false)
    {
        SENT_DRV_HW_GetFastRawMsg(instance, (uint8_t)channel, &locMsg);
        SENT_DRV_HW_ConvertFastRaw(message, &locMsg);
        retVal = STATUS_SUCCESS;
    }
    else
    {
        retVal = STATUS_ERROR;
    }

    return retVal;
}

/*!
 * @brief Update the DMA destination buffer for Fast messages.
 */
status_t SENT_DRV_SetFastMsgDmaBuffer(const uint32_t instance, sent_raw_msg_t * buffer)
{
    /* Invalid instance */
    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);
    DEV_ASSERT(s_sentStatePtr[instance] != NULL);

    /* Store and update */
    s_sentStatePtr[instance]->fastMsgDmaPtr = buffer;
    DMA_DRV_SetDestAddr(s_sentStatePtr[instance]->fastDmaChannel, (uint32_t)buffer);

    return STATUS_SUCCESS;
}

/*!
 * @brief Convert a raw DMA Fast message to a structured Fast message.
 */
void SENT_DRV_GetFastMsgFromRaw(sent_fast_msg_t * msg, const sent_raw_msg_t * rawMsg)
{
    /* Just call the conversion function */
    SENT_DRV_HW_ConvertFastRaw(msg, rawMsg);
}

/*!
 * @brief Check whether a new Slow message is available.
 */
bool SENT_DRV_GetSlowRxStatus(const uint32_t instance, const uint32_t channel)
{
    bool retVal;

    /* Invalid instance or channel */
    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);
    DEV_ASSERT(channel < SENT_CH_COUNT);
    DEV_ASSERT(s_sentStatePtr[instance] != NULL);

    /* Only if channel is enabled */
    if(s_sentStatePtr[instance]->activeChannels[channel] != false)
    {
        retVal = SENT_DRV_HW_GetSlowRxStatus(instance, (uint8_t)channel);
    }
    else
    {
        retVal = false;
    }

    return retVal;
}

/*!
 * @brief Read the most recently received Slow message.
 */
status_t SENT_DRV_GetSlowMsg(const uint32_t instance, const uint32_t channel, sent_slow_msg_t * message)
{
    sent_raw_msg_t locMsg;
    status_t retVal;

    /* Invalid instance or channel */
    DEV_ASSERT(channel < SENT_CH_COUNT);
    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);
    DEV_ASSERT(s_sentStatePtr[instance] != NULL);


    /* Only if channel is enabled */
    if(s_sentStatePtr[instance]->activeChannels[channel] != false)
    {
        SENT_DRV_HW_GetSlowRawMsg(instance, (uint8_t)channel, &locMsg);
        SENT_DRV_HW_ConvertSlowRaw(message, &locMsg);
        retVal = STATUS_SUCCESS;
    }
    else
    {
        retVal = STATUS_ERROR;
    }

    return retVal;
}

/*!
 * @brief Update the DMA destination buffer for Slow messages.
 */
status_t SENT_DRV_SetSlowMsgDmaBuffer(const uint32_t instance, sent_raw_msg_t * buffer)
{
    /* Invalid instance or channel */
    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);
    DEV_ASSERT(s_sentStatePtr[instance] != NULL);

    /* Store and update */
    s_sentStatePtr[instance]->slowMsgDmaPtr = buffer;
    DMA_DRV_SetDestAddr(s_sentStatePtr[instance]->slowDmaChannel, (uint32_t)buffer);

    return STATUS_SUCCESS;
}

/*!
 * @brief Convert a raw DMA Slow message to a structured Slow message.
 */
void SENT_DRV_GetSlowMsgFromRaw(sent_slow_msg_t * msg, const sent_raw_msg_t * rawMsg)
{
    /* Just call the conversion function */
    SENT_DRV_HW_ConvertSlowRaw(msg, rawMsg);
}

/*!
 * @brief Set or replace the event callback function.
 */
status_t SENT_DRV_SetRxCallbackFunction(const uint32_t instance, sent_callback_func_t function, void * param)
{
    /* Invalid instance */
    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);
    DEV_ASSERT(s_sentStatePtr[instance] != NULL);

    /* Copy to state structure */
    s_sentStatePtr[instance]->callbackFunc.function = function;
    s_sentStatePtr[instance]->callbackFunc.param = param;

    /* Notification setup */
    SENT_DRV_InstallCallbacks(instance);

    return STATUS_SUCCESS;
}

/*!
 * @brief Read and atomically clear the active diagnostic event flags.
 */
status_t SENT_DRV_GetEvents(const uint32_t instance, const uint32_t channel, sent_event_t * events)
{
    status_t retVal;
    sent_event_t locEv;

    /* Invalid instance or channel */
    DEV_ASSERT(instance < SENT_INSTANCE_COUNT);
    DEV_ASSERT(channel < SENT_CH_COUNT);
    DEV_ASSERT(s_sentStatePtr[instance] != NULL);

    /* Only if channel is enabled */
    if(s_sentStatePtr[instance]->activeChannels[channel] != false)
    {
        /* Get, clear and return */
        locEv = SENT_DRV_HW_GetActiveEvents(instance, (uint8_t)channel);
        SENT_DRV_HW_ClearActiveEvents(instance, (uint8_t)channel, locEv);
        *events = locEv;
        retVal = STATUS_SUCCESS;
    }
    else
    {
        retVal = STATUS_ERROR;
    }

    return retVal;
}

/*******************************************************************************
* EOF
*******************************************************************************/
