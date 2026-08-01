/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file linflexd_lin_driver.c
 * @brief LINFlexD LIN driver implementation.
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 2986 Rule 2.2: This operation is redundant. The value of the result is 
 *                       always that of the right-hand operand.
 * PRQA S 4322 Rule 10.5:  An expression of 'essentially enum' type (linflexd_hw_state_t) 
 *                         is being cast to a different enum type, 'linflexd_node_state_t'.
 *
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "linflexd_lin_driver.h"
#include "linflexd_lin_irq.h"
#include "linflexd_hw_access.h"
#include <stdbool.h>

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Array of pointers to LINFlexD driver runtime state structures */
static linflexd_state_t * s_LINFlexDStatePtr[LINFlexD_INSTANCE_COUNT];

/* Table to save LINFlexD user config structure pointers */
static linflexd_user_config_t * s_LINFlexDUserconfigPtr[LINFlexD_INSTANCE_COUNT];

/*! @brief Table of base addresses for LINFlexD instances. */
static LINFlexD_Type * const s_LINFlexDBase[LINFlexD_INSTANCE_COUNT] = LINFlexD_BASE_PTRS;

static const clock_names_t s_LINFlexDClkName[LINFlexD_INSTANCE_COUNT] = LINFlexD_CLOCK_NAMES;

#if defined(FEATURE_LINFlexD_ORED_INT_LINES) && (FEATURE_LINFlexD_ORED_INT_LINES == 1U)
/*! @brief Table to save LINFlexD IRQ vectors defined in the header file */
static const IRQn_Type s_LINFlexDIntVec[LINFlexD_INSTANCE_COUNT] = LINFlexD_IRQS;
#endif

/*******************************************************************************
 * Internal Functions
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief This function initializes a LIN Hardware Interface for operation. This function will initialize the run-time state structure to keep track of the on-going transfers, ungate the clock to LIN Hardware Interface, initialize the module to user defined settings and default settings, configure the IRQ state structure and enable the module-level interrupt to the core, and enable the LIN Hardware Interface transmitter and receiver. The following is an example of how to set up the linflexd_state_t and the linflexd_user_config_t parameters and how to call the LINFlexD_DRV_Init function by passing in these parameters: linflexd_user_config_t linUserConfig linUserConfig.baudRate = 9600 linUserConfig.nodeFunction = SLAVE linUserConfig.autobaudEnable = true linflexd_state_t linState LINFlexD_DRV_Init(instance, (linflexd_user_config_t *) &linUserConfig, (linflexd_state_t *) &linState).
 */
status_t LINFlexD_DRV_Init(uint32_t instance,
                      linflexd_user_config_t * linUserConfig,
                      linflexd_state_t * linCurrentState)
{
    /* Assert parameters. */
    DEV_ASSERT(linUserConfig && linCurrentState);
    DEV_ASSERT(instance < LINFlexD_INSTANCE_COUNT);

    uint32_t linSourceClockFreq = 0U;
    status_t retVal = STATUS_SUCCESS;

    /* Get base address of the UART instance. */
    LINFlexD_Type * base = s_LINFlexDBase[instance];

    /* Get the LIN clock as configured in the clock manager */
    (void)CLOCK_SYS_GetFreq(s_LINFlexDClkName[instance], &linSourceClockFreq);

    /* Check if current instance is clock gated off. */
    DEV_ASSERT(linSourceClockFreq > 0U);

    /* Check if current instance is already initialized. */
    DEV_ASSERT(s_LINFlexDStatePtr[instance] == NULL);

    /* Save runtime structure pointer. */
    s_LINFlexDStatePtr[instance] = linCurrentState;

    /* Save LIN user config structure pointer. */
    s_LINFlexDUserconfigPtr[instance] = linUserConfig;

    /* Clear linSourceClockFreq value */
    linCurrentState->linSourceClockFreq = linSourceClockFreq;

    /* Create txCompleted and rxCompleted semaphores as counting, with init value is 0 */
    if (OSIF_SemaCreate(&linCurrentState->completed, 0) != STATUS_SUCCESS)
    {
        retVal = STATUS_ERROR;
    }
    else
    {
        /* Request init mode and wait until the mode entry is complete */
        LINFlexD_EnterInitMode(base);
        while (LINFlexD_GetLinState(base) != LINFlexD_STATE_INIT_MODE) {}

        /* Set linflexd to lin mode */
        LINFlexD_Init(base, (bool)(linUserConfig->nodeFunction), (linflexd_break_length_t)(linUserConfig->breakLength));
        /* Set break length */

        /* config ID filters for each */
        LINFlexD_ConfigIdFilter(base, linUserConfig->filterCount, linUserConfig->slaveFilterCfgPtr);
        /* configure timeout value */
        LINFlexD_EnableTimeout(base, (bool)(linUserConfig->timeoutEnable));
        LINFlexD_SetTimeoutValue(base, linUserConfig->responseTimeoutValue, linUserConfig->headerTimeoutValue);

        /* if autobaud is enabled */
        if ((linUserConfig->autobaudEnable) && (linUserConfig->nodeFunction == false))
        {
            /* Setting Slave's baudrate to 19200 will help Slave node */
            /* always detect LIN Break from Master */
            linUserConfig->baudRate = 19200U;
            LINFlexD_AutoSyncEnable(base, true);
        }

        /* Set baud rate to User's value */
        (void)LINFlexD_DRV_SetBaudRate(instance, linUserConfig->baudRate);

        /* Default interrupt enable setup */
        LINFlexD_IntInit(base);

        /* Disable Idle on Bit Error */
        LINFlexD_IdleBitError(base, false);

        /* Default disable call back */
        linCurrentState->Callback = NULL;
        /* Install handlers */
#if defined(FEATURE_LINFlexD_ORED_INT_LINES) && (FEATURE_LINFlexD_ORED_INT_LINES == 1U)
        INT_SYS_InstallHandler(s_LINFlexDIntVec[instance], g_LinLINFlexDIsr[instance], (isr_t*) 0);
#endif

        /* Enable LINFlexD interrupts */
#if defined(FEATURE_LINFlexD_ORED_INT_LINES) && (FEATURE_LINFlexD_ORED_INT_LINES == 1U)
        INT_SYS_EnableIRQ(s_LINFlexDIntVec[instance]);
#endif
        LINFlexD_EnterNormalMode(base);
    }

    return retVal;
}

/*!
 * @brief This function shuts down the LIN Hardware Interface by disabling interrupts and transmitter/receiver.
 */
void LINFlexD_DRV_Deinit(uint32_t instance)
{
    /* Assert parameters. */
    DEV_ASSERT(instance < LINFlexD_INSTANCE_COUNT);
    /* Get base address of the UART instance. */
    LINFlexD_Type * base = s_LINFlexDBase[instance];

    /* Get the current LIN state of this UART instance. */
    linflexd_state_t * linCurrentState = s_LINFlexDStatePtr[instance];

    /* Check if current instance is already de-initialized or is gated.*/
    DEV_ASSERT(linCurrentState != NULL);
    DEV_ASSERT(linCurrentState->linSourceClockFreq > 0U);

    /* Destroy semaphores. */
    (void)OSIF_SemaDestroy(&linCurrentState->completed);

    /* Disable LINFlexD interrupts */
#if defined(FEATURE_LINFlexD_ORED_INT_LINES) && (FEATURE_LINFlexD_ORED_INT_LINES == 1U)
    INT_SYS_DisableIRQ(s_LINFlexDIntVec[instance]);
#endif
    /* Enter init mode */
    LINFlexD_EnterInitMode(base);
    while (LINFlexD_GetLinState(base) != LINFlexD_STATE_INIT_MODE) {}
    /* Reset all control registers */
    LINFlexD_DeinitRegs(base);

    /* Clear our saved pointer to the LIN state structure */
    s_LINFlexDStatePtr[instance] = NULL;
}
/*!
 * @brief This function sets the baud rate for UART communication.
 */
status_t LINFlexD_DRV_SetBaudRate(uint32_t instance, uint32_t baudrate)
{
    DEV_ASSERT(instance < LINFlexD_INSTANCE_COUNT);

    float32_t prescaler;
    float32_t fraction;
    uint32_t mantissa;
    uint32_t fractionDenominator;
    uint8_t fractionNumerator;
    uint32_t uartSourceClock;
    clock_names_t instanceClkName;
    LINFlexD_Type * base;
    bool resetIdle = false;

    instanceClkName = s_LINFlexDClkName[instance];
    base = s_LINFlexDBase[instance];
    (void)CLOCK_SYS_GetFreq(instanceClkName, &uartSourceClock);

    /* Compute the values for baud rate divider mantissa and fraction */
    prescaler = (float32_t)uartSourceClock / ((float32_t)baudrate * (float32_t)DEFAULT_OSR);
    mantissa = (uint32_t)prescaler;
    fraction = prescaler - (float32_t)mantissa;
    fractionDenominator = ((uint32_t)1U << (uint32_t)BAUDRATE_FRACTION_WIDTH);
    float32_t float32_tNumerator = ((float32_t)fraction * (float32_t)fractionDenominator) + (float32_t)0.5F;
    fractionNumerator = (uint8_t)(float32_tNumerator);

    if (fractionNumerator == fractionDenominator)
    {
        fractionNumerator = 0;
        mantissa++;
    }

    if (LINFlexD_GetLinState(base) != LINFlexD_STATE_INIT_MODE)
    {
        /* Request init mode and wait until the mode entry is complete */
        LINFlexD_EnterInitMode(base);
        while (LINFlexD_GetLinState(base) != LINFlexD_STATE_INIT_MODE) {}

        resetIdle = true;
    }

    /* Write the computed values to registers */
    LINFlexD_SetIntegerBaudRate(base, mantissa);
    LINFlexD_SetFractionalBaudRate(base, fractionNumerator);

    if (resetIdle == true)
    {
        /* Enter normal mode */
        LINFlexD_EnterNormalMode(base);
    }

    return STATUS_SUCCESS;
}

/*!
 * @brief This function initializes a configuration structure received from the application with default values.
 */
void LINFlexD_DRV_GetDefaultConfig(bool isMaster,
                              linflexd_user_config_t * linUserConfig)
{
    linUserConfig->baudRate = 19200;            /*!< Default baud rate */
    linUserConfig->nodeFunction = isMaster;        /*!< default as slave node */
    linUserConfig->autobaudEnable = true;       /*!< Enable auto baudrate by default */

}

/*!
 * @brief This function installs the callback function that is used for LINFlexD_DRV_IRQHandler. Pass in Null pointer as callback will uninstall.
 */
linflexd_callback_t LINFlexD_DRV_InstallCallback(uint32_t instance,
                                       linflexd_callback_t function)
{
    /* Assert parameters. */
    DEV_ASSERT(instance < LINFlexD_INSTANCE_COUNT);

    /* Get the current LIN state of this UART instance. */
    linflexd_state_t * linCurrentState = s_LINFlexDStatePtr[instance];

    /* Get the current callback function. */
    linflexd_callback_t currentCallback = linCurrentState->Callback;

    /* Install new callback function. */
    linCurrentState->Callback = function;

    return currentCallback;
}
/*!
 * @brief This function sends frame through the LIN Hardware Interface using blocking method. This function will calculate the checksum byte and send it with the frame data. If data size is equal to 0 or greater than 8 then the function will return STATUS_ERROR. If isBusBusy is currently true then the function will return STATUS_BUSY.
 */
status_t LINFlexD_DRV_MasterTransferBlocking(uint32_t instance, linflexd_frame_t *frame, uint32_t timeoutMSec)
{
    linflexd_state_t * linState;
    status_t status;

    linState = (linflexd_state_t *)s_LINFlexDStatePtr[instance];
    linState->isBlocking = true;
    /* Reset signal before transfer */
    (void)OSIF_SemaWait(&(linState->completed), 0);
    status = LINFlexD_DRV_MasterTransfer(instance, frame);
    if (STATUS_SUCCESS == status)
    {
        if (OSIF_SemaWait(&linState->completed, timeoutMSec) == STATUS_TIMEOUT)
        {
            status = STATUS_TIMEOUT;
            (void)LINFlexD_DRV_AbortTransferData(instance);
        }
    }
    linState->isBlocking = false;
    return status;
}
/*!
 * @brief This function sends frame through the LIN Hardware Interface using non-blocking method. This function will calculate the checksum byte and send it with the frame data. The function will return immediately after calling this function. If data size is equal to 0 or greater than 8 then the function will return STATUS_ERROR. If isBusBusy is currently true then the function will return STATUS_BUSY.
 */
status_t LINFlexD_DRV_MasterTransfer(uint32_t instance, linflexd_frame_t *frame)
{
    /* Assert parameters. */
    DEV_ASSERT(instance < LINFlexD_INSTANCE_COUNT);

    status_t retVal = STATUS_SUCCESS;
    linflexd_direction_t bidr = LINFlexD_RX; 
    /* Get base address of the LINFlexD instance. */
    LINFlexD_Type * base = s_LINFlexDBase[instance];
    /* Get the current LIN state of this UART instance. */
    linflexd_state_t * linCurrentState = s_LINFlexDStatePtr[instance];
    
    /* Check if current instance is already de-initialized or is gated.*/
    DEV_ASSERT(linCurrentState != NULL);

    /* Check if txSize > 8 or equal to 0 */
    if ((8U < frame->dataLength ) || ((0U == frame->dataLength) || (0x3FU < frame->id)))
    {
        retVal = STATUS_ERROR;
    }else {

        /* Check if the LIN Bus is IDLE */
        if (LINFlexD_GetLinState(base) != LINFlexD_STATE_IDLE_MODE)
        {
            retVal = STATUS_BUSY;
        }else {

            /* Master Response */
            if (LIN_MASTER_RESPONSE == frame->responseType)
            {
                /* fill data to tx buffer */
                LINFlexD_SetTxDataArray(base, frame->data, frame->dataLength);
                bidr = LINFlexD_TX;
            }
            else if (LIN_SLAVE_TO_SLAVE == frame->responseType)
            {
                LINFlexD_DataDiscard(base, true);
            }
            else
            {
                /* Slave response, data from slave to master */
                LINFlexD_DataDiscard(base, false);
                /* Clear RDC */
                LINFlexD_ClearLinStatusFlag(base, LINFlexD_LIN_RX_DATA_START_FLAG);
                /* Update receive data buffer */
                linCurrentState->rxBuff = frame->data;
                linCurrentState->rxSize = frame->dataLength;
        
            }
            
            LINFlexD_SetLinBIDR(base, frame->id, frame->dataLength, (uint8_t)bidr, (uint8_t)frame->checksumType);
            /* Set header request to start transmission */
            LINFlexD_SetHeaderRequest(base);
        }
    }

    return retVal;
}

/*!
 * @brief This function sends out slave data through the LIN Hardware Interface non-blocking method. This function will calculate the checksum byte and send it with the frame data. The function will return immediately after calling this function. If data size is equal to 0 or greater than 8 then the function will return STATUS_ERROR. If isBusBusy is currently true then the function will return STATUS_BUSY.
 */
status_t LINFlexD_DRV_SlaveResponse(uint32_t instance, linflexd_frame_t *frame)
{
    /* Assert parameters. */
    DEV_ASSERT(instance < LINFlexD_INSTANCE_COUNT);
    DEV_ASSERT(frame->responseType < LIN_SLAVE_TO_SLAVE);

    status_t retVal = STATUS_SUCCESS;
    /* Get base address of the LINFlexD instance. */
    LINFlexD_Type * base = s_LINFlexDBase[instance];
    /* Get the current LIN state of this UART instance. */
    linflexd_state_t * linCurrentState = s_LINFlexDStatePtr[instance];

    /* Check if header is received. */
    if (0U == (base->LINSR & LINFlexD_LINSR_HRF_MASK))
    {
        /* Function called when no header received */
        retVal = STATUS_LINFlexD_NO_HEADER_RECEIVED;
    }else {
        
        if (LIN_SLAVE_RESPONSE == frame->responseType)
        {
            /* Slave send response data. */
            /* fill data to tx buffer */
            LINFlexD_SetTxDataArray(base, frame->data, frame->dataLength);
        }
        else
        {
            /* LIN slave receive response data. */
            /* Update receive data buffer */
            linCurrentState->rxBuff = frame->data;
            linCurrentState->rxSize = frame->dataLength;
        }
        /* Update direction, data length and checksum type. */
        LINFlexD_SetLinBIDR(base, frame->id, frame->dataLength, (uint8_t)frame->responseType, (uint8_t)frame->checksumType);
        if (LIN_SLAVE_RESPONSE == frame->responseType)
        {
            /* Set data request to start transmission */
            LINFlexD_SetDataRequest(base);
        }
    }

    return retVal;
}

/*!
 * @brief Stop data reception if the frame does not concern the node. Call it after receive header, used in LIN slave mode with id software-filtered.
 */
status_t LINFlexD_DRV_DataDiscard(uint32_t instance)
{
    /* Assert parameters. */
    DEV_ASSERT(instance < LINFlexD_INSTANCE_COUNT);

    /* Get base address of the LINFlexD instance. */
    LINFlexD_Type * base = s_LINFlexDBase[instance];

    LINFlexD_DataDiscard(base, true);

    return STATUS_SUCCESS;
}

/*!
 * @brief Aborts an ongoing non-blocking transmission/reception. While performing a non-blocking transferring data, users can call this function to terminate immediately the transferring.
 */
status_t LINFlexD_DRV_AbortTransferData(uint32_t instance)
{
    /* Assert parameters. */
    DEV_ASSERT(instance < LINFlexD_INSTANCE_COUNT);

    status_t retVal = STATUS_LINFlexD_ABORT_TIMEOUT;
    /* Get base address of the LINFlexD instance. */
    LINFlexD_Type * base = s_LINFlexDBase[instance];

    LINFlexD_SetTransferAbort(base, true);
    for (uint32_t i = 0; i < FEATURE_LINFlexD_TIMEOUT_LOOPS; i++) 
    {
        if (false == LINFlexD_GetTransferAbort(base))
        {
            retVal = STATUS_SUCCESS;
            break;
        }
    }
    return retVal;
}


/*!
 * @brief This function puts current LIN node to sleep mode. This function changes current node state to LINFlexD_NODE_STATE_SLEEP_MODE.
 */
status_t LINFlexD_DRV_GoToSleepMode(uint32_t instance)
{
    /* Assert parameters. */
    DEV_ASSERT(instance < LINFlexD_INSTANCE_COUNT);

    /* Get base address of the LINFlexD instance. */
    LINFlexD_Type * base = s_LINFlexDBase[instance];

    DEV_ASSERT (false == LINFlexD_GetSleepMode(base));

    /* Clear previous flags. */
    LINFlexD_ClearLinStatusFlag(base, LINFlexD_LIN_WAKEUP_FLAG);
    /* Enable wakeup interrupt */
    LINFlexD_SetInterruptMode(base, LINFlexD_WAKEUP_INT, true);
#if (FEATURE_LINFlexD_HAS_RED_IRQ)
    /* Clear previous flags. */
    LINFlexD_ClearLinStatusFlag(base, LINFlexD_LIN_RECEIVE_EDGE_DETECTION_FLAG);
    /* Enable wakeup interrupt */
    LINFlexD_SetInterruptMode(base, LINFlexD_RECEIVE_EDGE_DETECTION_INT, true);
#endif /* FEATURE_LINFlexD_HAS_RED_IRQ */
    /* Request to sleep mode. */
    LINFlexD_SetSleepMode(base, true);
    return STATUS_SUCCESS;
}

/*!
 * @brief This function puts current node to Idle state.
 */
status_t LINFlexD_DRV_GotoIdleState(uint32_t instance)
{
    /* Assert parameters. */
    DEV_ASSERT(instance < LINFlexD_INSTANCE_COUNT);

    /* Get base address of the LINFlexD instance. */
    LINFlexD_Type * base = s_LINFlexDBase[instance];
    /* Clear previous flags. */
    LINFlexD_ClearLinStatusFlag(base, LINFlexD_LIN_WAKEUP_FLAG);
    /* Exit sleep mode. */
    LINFlexD_SetSleepMode(base, false);
    return STATUS_SUCCESS;
}

/*!
 * @brief This function sends a wakeup signal through the LIN interface.
 */
status_t LINFlexD_DRV_SendWakeupSignal(uint32_t instance)
{
    /* Assert parameters. */
    DEV_ASSERT(instance < LINFlexD_INSTANCE_COUNT);

    /* Get base address of the LINFlexD instance. */
    LINFlexD_Type * base = s_LINFlexDBase[instance];
    DEV_ASSERT (false == LINFlexD_GetSleepMode(base));
    LINFlexD_SetWakeupRequest(base);
    return STATUS_SUCCESS;
}

/*!
 * @brief This function gets the current LIN node state.
 */
linflexd_node_state_t LINFlexD_DRV_GetCurrentNodeState(uint32_t instance)
{
    /* Assert parameters. */
    DEV_ASSERT(instance < LINFlexD_INSTANCE_COUNT);

    /* Get base address of the LINFlexD instance. */
    LINFlexD_Type * base = s_LINFlexDBase[instance];
    return (linflexd_node_state_t)LINFlexD_GetLinState(base); /*PRQA S 4322*/
}

/*!
 * @brief This function enables LIN hardware interrupts.
 */
status_t LINFlexD_DRV_EnableIRQ(uint32_t instance)
{
    /* Assert parameters. */
    DEV_ASSERT(instance < LINFlexD_INSTANCE_COUNT);
    /* Enable LINFlexD interrupts */
#if defined(FEATURE_LINFlexD_ORED_INT_LINES) && (FEATURE_LINFlexD_ORED_INT_LINES == 1U)
    INT_SYS_EnableIRQ(s_LINFlexDIntVec[instance]);
#endif

    return STATUS_SUCCESS;
}

/*!
 * @brief This function disables LIN hardware interrupts.
 */
status_t LINFlexD_DRV_DisableIRQ(uint32_t instance)
{
    /* Assert parameters. */
    DEV_ASSERT(instance < LINFlexD_INSTANCE_COUNT);
    /* Disable LINFlexD interrupts */
#if defined(FEATURE_LINFlexD_ORED_INT_LINES) && (FEATURE_LINFlexD_ORED_INT_LINES == 1U)
    INT_SYS_DisableIRQ(s_LINFlexDIntVec[instance]);
#endif
    return STATUS_SUCCESS;
}

/*!
 * @brief Interrupt handler for LIN Hardware Interface.
 */
void LINFlexD_LIN_DRV_IRQHandler(uint32_t instance)
{
    const LINFlexD_Type * base;

    base = s_LINFlexDBase[instance];

    /* Handle receive interrupt */
    /* header received(HRIE, Header transmit with RX dir), data received(DRIE), 
     * LIN State switch, wakeup interrupt (WUIE)*/
    if (LINFlexD_IsInterruptEnabled(base, 
                            (LINFlexD_LINIER_DRIE_MASK | 
                            LINFlexD_LINIER_HRIE_MASK | 
                            LINFlexD_LINIER_WUIE_MASK
#if (FEATURE_LINFlexD_HAS_RED_IRQ)
                            | LINFlexD_LINIER_REDIE_MASK
#endif /* FEATURE_LINFlexD_HAS_RED_IRQ */
                            )))
    {
        LINFlexD_LIN_DRV_RxIRQHandler(instance);
    }

    /* Handle transmitter data register empty interrupt */
    /* Data transition(DTIE), Header transmit with TX dir and matching */
    if (LINFlexD_IsInterruptEnabled(base, (uint32_t)LINFlexD_DATA_TRANSMITTED_INT))
    {
        LINFlexD_LIN_DRV_TxIRQHandler(instance);
    }

    /* Handle the error interrupts if no rx/tx interrupt was triggered */
    LINFlexD_LIN_DRV_ErrIRQHandler(instance);
}

/*!
 * @brief Rx interrupt handler for LIN. This handler uses the rx buffer stored in the state structure to receive data. This is not a public API as it is called by IRQ whenever an interrupt occurs.
 */
void LINFlexD_LIN_DRV_RxIRQHandler(uint32_t instance)
{
    linflexd_state_t * linState;
    LINFlexD_Type * base;

    base = s_LINFlexDBase[instance];
    linState = (linflexd_state_t *)s_LINFlexDStatePtr[instance];

    /* Handle receive data interrupt */
    if (LINFlexD_GetLinStatusFlag(base, LINFlexD_LIN_DATA_RECEPTION_COMPLETE_FLAG))
    {
        /* Handle ID filter for receiving. */
        LINFlexD_LIN_DRV_FilterResponse(instance, LINFlexD_RX);
        /* Retrieve the data */
        LINFlexD_GetRxDataArray(base, linState->rxBuff, linState->rxSize);
        /* Release receive data flag. */
        LINFlexD_ClearLinStatusFlag(base, LINFlexD_LIN_DATA_RECEPTION_COMPLETE_FLAG);
        /* Release receive data message buffer. */
        LINFlexD_ClearLinStatusFlag(base, LINFlexD_LIN_RELEASE_MESSAGE_BUFFER_FLAG);
        /* Release receive header flag. */
        LINFlexD_ClearLinStatusFlag(base, LINFlexD_LIN_HEADER_RECEIVED_FLAG);

        /* Signal the synchronous completion object. */
        if (linState->isBlocking)
        {
            (void)OSIF_SemaPost(&linState->completed);
        }
        linState->currentId = LINFlexD_GetLinBidrID(base);
        linState->currentEventId = LINFlexD_DATA_RECEIVED_EVENT;
        /* Invoke the callback when the buffer is finished; */
        if (NULL != linState->Callback)
        {
            linState->Callback(instance, linState);
        }
    }
    /* Handle receive header interrupt */
    if (LINFlexD_GetLinStatusFlag(base, LINFlexD_LIN_HEADER_RECEIVED_FLAG))
    {
        linState->currentEventId = LINFlexD_HEADER_RECEIVED_EVENT;
        linState->currentId = LINFlexD_GetLinBidrID(base);
        /* Handle ID filter for sending. */
        LINFlexD_LIN_DRV_FilterResponse(instance, LINFlexD_TX);
        /* Invoke the callback when the header received; */
        if (NULL != linState->Callback)
        {
            linState->Callback(instance, linState);
        }
        /* Clear the flag */
        LINFlexD_ClearLinStatusFlag(base, LINFlexD_LIN_HEADER_RECEIVED_FLAG);
    }
    /* Handle wakeup interrupt */
    if (LINFlexD_GetLinStatusFlag(base, LINFlexD_LIN_WAKEUP_FLAG))
    {
        linState->currentEventId = LINFlexD_WAKEUP_EVENT;
        /* Clear previous flags. */
        LINFlexD_ClearLinStatusFlag(base, LINFlexD_LIN_WAKEUP_FLAG);
        /* Disable wake up interrupt */
        LINFlexD_SetInterruptMode(base, LINFlexD_WAKEUP_INT, false);
        /* Exit sleep mode. */
        LINFlexD_SetSleepMode(base, false);
        /* Invoke the callback when the wakeup detected */
        if (NULL != linState->Callback)
        {
            linState->Callback(instance, linState);
        }
    }
#if (FEATURE_LINFlexD_HAS_RED_IRQ)
    /* Handle receive edge detection interrupt */
    if (LINFlexD_GetLinStatusFlag(base, LINFlexD_LIN_RECEIVE_EDGE_DETECTION_FLAG))
    {
        linState->currentEventId = LINFlexD_RECEIVE_EDGE_DETECTION_EVENT;
        /* Clear previous flags. */
        LINFlexD_ClearLinStatusFlag(base, LINFlexD_LIN_RECEIVE_EDGE_DETECTION_FLAG);
        /* Disable RDF interrupt */
        LINFlexD_SetInterruptMode(base, LINFlexD_RECEIVE_EDGE_DETECTION_INT, false);
        /* Exit sleep mode. */
        LINFlexD_SetSleepMode(base, false);
        /* Invoke the callback when the receive edge detected */
        if (NULL != linState->Callback)
        {
            linState->Callback(instance, linState);
        }
    }
#endif /* FEATURE_LINFlexD_HAS_RED_IRQ */
}
/*!
 * @brief Tx interrupt handler for LIN. This handler uses the tx buffer stored in the state structure to transmit data. This is not a public API as it is called by IRQ whenever an interrupt occurs.
 */
void LINFlexD_LIN_DRV_TxIRQHandler(uint32_t instance)
{
    linflexd_state_t * linState;
    LINFlexD_Type * base;

    base = s_LINFlexDBase[instance];
    linState = (linflexd_state_t *)s_LINFlexDStatePtr[instance];

    /* Handle transmit data interrupt */
    if (LINFlexD_GetLinStatusFlag(base, LINFlexD_LIN_DATA_TRANSMITTED_FLAG))
    {
        /* Signal the synchronous completion object. */
        if (linState->isBlocking)
        {
            (void)OSIF_SemaPost(&linState->completed);
        }
        /* Invoke the callback when the buffer is finished; */
        linState->currentEventId = LINFlexD_DATA_SENT_EVENT;
        if (NULL != linState->Callback)
        {
            linState->Callback(instance, linState);
        }
    }
    /* Clear the flag */
    LINFlexD_ClearLinStatusFlag(base, LINFlexD_LIN_DATA_TRANSMITTED_FLAG);
}

/*!
 * @brief Rx interrupt handler for LIN. This handler uses the rx buffer stored in the state structure to receive data. This is not a public API as it is called by IRQ whenever an interrupt occurs.
 */
void LINFlexD_LIN_DRV_ErrIRQHandler(uint32_t instance)
{
    linflexd_state_t * linState;
    LINFlexD_Type * base;
    uint32_t tempReg = 0;

    base = s_LINFlexDBase[instance];
    linState = (linflexd_state_t *)s_LINFlexDStatePtr[instance];
    if (base->LINESR != 0U)
    {
        linState->errorStatus = base->LINESR;
        /* get the received size for LIN event frame*/
        tempReg = ((base->LINSR&LINFlexD_LINSR_RDC_MASK) >> LINFlexD_LINSR_RDC_SHIFT);
        linState->rxSize = (tempReg < linState->rxSize) ? (linState->rxSize - (uint8_t)tempReg) : (uint8_t)0U;
        /* Check output compare event  */
        if (LINFlexD_GetLinErrorStatusFlag(base, LINFlexD_LIN_OUTPUT_COMPARE_FLAG))
        {
            linState->currentEventId = LINFlexD_OUTPUT_COMPARE_EVENT;
            base->LINESR = (uint32_t)LINFlexD_LIN_OUTPUT_COMPARE_FLAG;
        }
        if (LINFlexD_GetLinErrorStatusFlag(base, LINFlexD_LIN_BUFFER_OVERRUN_FLAG))
        {
            linState->currentEventId = LINFlexD_RX_OVERRUN_ERROR_EVENT;
            base->LINESR = (uint32_t)LINFlexD_LIN_BUFFER_OVERRUN_FLAG;
        }
        if (LINFlexD_GetLinErrorStatusFlag(base, LINFlexD_LIN_FRAMING_ERROR_FLAG))
        {
            linState->currentEventId = LINFlexD_FRAME_ERROR_EVENT;
            base->LINESR = (uint32_t)LINFlexD_LIN_FRAMING_ERROR_FLAG;
        }
        if (LINFlexD_GetLinErrorStatusFlag(base, LINFlexD_LIN_ID_PARITY_ERROR_FLAG))
        {
            linState->currentEventId = LINFlexD_PID_PARITY_ERROR_EVENT;
            base->LINESR = (uint32_t)LINFlexD_LIN_ID_PARITY_ERROR_FLAG;
        }
        if (LINFlexD_GetLinErrorStatusFlag(base, LINFlexD_LIN_BREAK_DELIMITER_ERROR_FLAG))
        {
            linState->currentEventId = LINFlexD_BREAK_DELIMITER_ERROR_EVENT;
            base->LINESR = (uint32_t)LINFlexD_LIN_BREAK_DELIMITER_ERROR_FLAG;
        }
        if (LINFlexD_GetLinErrorStatusFlag(base, LINFlexD_LIN_SYNC_FIELD_ERROR_FLAG))
        {
            linState->currentEventId = LINFlexD_SYNC_ERROR_EVENT;
            base->LINESR = (uint32_t)LINFlexD_LIN_SYNC_FIELD_ERROR_FLAG;
        }
        if (LINFlexD_GetLinErrorStatusFlag(base, LINFlexD_LIN_CHECKSUM_ERROR_FLAG))
        {
            linState->currentEventId = LINFlexD_CHECKSUM_ERROR_EVENT;
            base->LINESR = (uint32_t)LINFlexD_LIN_CHECKSUM_ERROR_FLAG;
        }
        if (LINFlexD_GetLinErrorStatusFlag(base, LINFlexD_LIN_BIT_ERROR_FLAG))
        {
            linState->currentEventId = LINFlexD_BIT_ERROR_EVENT;
            base->LINESR = (uint32_t)LINFlexD_LIN_BIT_ERROR_FLAG;
        }
        if (LINFlexD_GetLinErrorStatusFlag(base, LINFlexD_LIN_STUCK_AT_ZERO_FLAG))
        {
            linState->currentEventId = LINFlexD_STUCK_ZERO_ERROR_EVENT;
            base->LINESR = (uint32_t)LINFlexD_LIN_STUCK_AT_ZERO_FLAG;
        }
        if (LINFlexD_GetLinErrorStatusFlag(base, LINFlexD_LIN_NOISE_FLAG))
        {
            linState->currentEventId = LINFlexD_NOISE_ERROR_EVENT;
            base->LINESR = (uint32_t)LINFlexD_LIN_NOISE_FLAG;
        }

        if (NULL != linState->Callback)
        {
            linState->Callback(instance, linState);
        }
    }
}

/*!
 * @brief Handle ID filter for LIN. This function is used when ID Filter enable and auto sending or receving.
 */
void LINFlexD_LIN_DRV_FilterResponse(uint32_t instance, linflexd_direction_t direction)
{
    uint8_t index;
    linflexd_user_config_t * linUserConfig;
    linflexd_id_filter_config_t * linIdFilterConfig;
    linflexd_state_t * linState;
    LINFlexD_Type * base;

    base = s_LINFlexDBase[instance];
    linUserConfig = (linflexd_user_config_t *)s_LINFlexDUserconfigPtr[instance];
    linState = (linflexd_state_t *)s_LINFlexDStatePtr[instance];

    /* id filter used in slave mode */
    if (linUserConfig->nodeFunction == false)
    {
        index = LINFlexD_GetFilterMatchIndex(base);
        /* if no ID match, index equal 0. */
        if (index != 0U)
        {
            linIdFilterConfig = &(linUserConfig->slaveFilterCfgPtr[index-1U]);
            /* exclude slave rx in header interrupt */
            if (linIdFilterConfig->direction == direction)
            {
                if (direction == LINFlexD_RX)
                {
                    /* set pointer to linIdFilterConfig items */
                    linState->rxBuff = linIdFilterConfig->data;
                    linState->rxSize = linIdFilterConfig->dataFieldLength;
                }
                else
                {
                    /* Slave send response data. */
                    /* fill data to tx buffer */
                    LINFlexD_SetTxDataArray(base, linIdFilterConfig->data, linIdFilterConfig->dataFieldLength);
                    /* Set data request to start transmission */
                    LINFlexD_SetDataRequest(base);
                }
            }
        }
    }
}

/*!
 * @brief Get current match ID. This function is used when ID Filter enable and ID getting.
 */
uint8_t LINFlexD_GetFilterMatchId(uint32_t instance)
{
    uint8_t index;
    LINFlexD_Type * base;
    linflexd_user_config_t * linUserConfig;
    base = s_LINFlexDBase[instance];
    linUserConfig = (linflexd_user_config_t *)s_LINFlexDUserconfigPtr[instance];

    index = LINFlexD_GetFilterMatchIndex(base);
    DEV_ASSERT(index != 0);
    return (linUserConfig->slaveFilterCfgPtr[index-1U].id);
}

/*!
 * @brief Check if frame use classic checksum.
 */
linflexd_cs_t LINFlexD_DRV_GetChecksumType(uint32_t instance, uint8_t ID)
{
    /* Get list of PIDs use classic checksum. */
    const uint8_t *classicID = s_LINFlexDUserconfigPtr[instance]->classicID;
    const uint8_t numOfClassicID = s_LINFlexDUserconfigPtr[instance]->numOfClassicID;
    uint8_t i = 0;
    linflexd_cs_t type = LIN_ENHANCED_CHECKSUM;

    if(numOfClassicID == 255U)
    {
        /* All frame use classic checksum */
        type = LIN_CLASSIC_CHECKSUM;
    }
    else
    {
        if(classicID != NULL)
        {
            for (i = 0U; i < numOfClassicID; i++)
            {
                if(ID == classicID[i])
                {
                    type = LIN_CLASSIC_CHECKSUM;
                    break;
                }
            }
        }
    }
    
    /* For PID is 0x3C (ID 0x3C) or 0x7D (ID 0x3D) or 0xFE (ID 0x3E) or 0xBF (ID 0x3F)
     * apply classic checksum and apply enhanced checksum for other PID */
    if ((0x3CU == ID) || (0x3DU == ID) || (0x3EU == ID) || (0x3FU == ID))
    {
        type = LIN_CLASSIC_CHECKSUM;
    }
    return type;
}

/*!
 * @brief Return bit B in byte A.
 */
static inline uint8_t BIT(uint8_t A,
                          uint8_t B)
{
    return (uint8_t) ((A >> B) & 0x01U);
}

/*!
 * @brief Makes or checks parity bits. If action is checking parity, the function returns ID value if parity bits are correct or 0xFF if parity bits are incorrect. If action is making parity bits, then from input value of ID, the function returns PID.
 */
uint8_t LINFlexD_DRV_ProcessParity(uint8_t PID,
                              uint8_t typeAction)
{
    uint8_t parity;
    uint8_t retVal;

    parity = (uint8_t) (((0xFFU & (BIT(PID, 0U) ^ BIT(PID, 1U) ^ BIT(PID, 2U) ^ BIT(PID, 4U))) << 6U) | /*PRQA S 2986*/
                        ((0xFFU ^ (BIT(PID, 1U) ^ BIT(PID, 3U) ^ BIT(PID, 4U) ^ BIT(PID, 5U))) << 7U));

    /* Check if action is checking parity bits */
    if (CHECK_PARITY == typeAction)
    {
        /* If parity bits are incorrect */
        if ((PID & 0xC0U) != parity)
        {
            /* Return 0xFF if parity bits are incorrect */
            retVal = 0xFFU;
        }
            /* If parity bits are correct */
        else
        {
            /* Return ID if parity bits are correct */
            retVal = (uint8_t) (PID & 0x3FU);
        }
    }
    /* If action is making parity bits */
    else
    {
        /* Return PID in case of making parity bits */
        retVal = (uint8_t) (PID | parity);
    }

    return retVal;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
