/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file sent_irq.c
 * @version 1.4.0
 */

#include <stddef.h>
#include "sent_irq.h"
#include "device_registers.h"
#include "interrupt_manager.h"
#include "sent_hw_access.h"

/*******************************************************************************
 * Declare
 ******************************************************************************/
void SENT_IRQHandler(void);

void SENT0_IRQHandler(void);

#if (SENT_INSTANCE_COUNT > 1u)
void SENT1_IRQHandler(void);
#endif /* (SENT_INSTANCE_COUNT > 1u) */

/*******************************************************************************
 * Code
 ******************************************************************************/
/**
 * Array containing interrupt mappings.
 */
static const IRQn_Type s_sentInterruptMappings[SENT_INSTANCE_COUNT][SENT_CH_COUNT] = FEATURE_SENT_IRQS;

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_IRQ_EnableIRQ
 * Description   : Enabler for SENT_DRV_IRQ_EnableIRQ interrupt
 *
 *END**************************************************************************/
void SENT_DRV_IRQ_EnableIRQ(const uint32_t instance, const uint32_t channel, const sent_interrupt_id_t id)
{
    (void)id; /* Shared */
    INT_SYS_EnableIRQ(s_sentInterruptMappings[instance][channel]);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_IRQ_DisableIRQ
 * Description   : Disabler for SENT_DRV_IRQ_DisableIRQ interrupt
 *
 *END**************************************************************************/
void SENT_DRV_IRQ_DisableIRQ(const uint32_t instance, const uint32_t channel, const sent_interrupt_id_t id)
{
    (void)id; /* Shared */
    INT_SYS_DisableIRQ(s_sentInterruptMappings[instance][channel]);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_IRQ_Dispatch
 * Description   : Interrupt dispatcher
 *
 *END**************************************************************************/
void SENT_DRV_IRQ_Dispatch(const uint32_t instance, const uint32_t channel)
{
    /* Dispatch according to event type */
    if (SENT_DRV_HW_GetFastRxStatus(instance, channel))
    {
        SENT_DRV_IRQ_FastHandler(instance, channel);
    }
    else if(SENT_DRV_HW_GetSlowRxStatus(instance, channel))
    {
        SENT_DRV_IRQ_SlowHandler(instance, channel);
    }
    else if(SENT_DRV_HW_GetWakeupEvent(instance))
    {
        SENT_DRV_IRQ_WakeupHandler(instance, channel);
    }
    else
    {
        /* Fall through, error */
        SENT_DRV_IRQ_RxErrHandler(instance, channel);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_IRQHandler
 * Description   : Handler for SENT_IRQn interrupt
 *
 *END**************************************************************************/
void SENT_IRQHandler(void)
{
    SENT_DRV_IRQ_Dispatch(0u, 0u);
    SENT_DRV_IRQ_Dispatch(0u, 1u);
}

void SENT0_IRQHandler(void)
{
    for(uint8_t i = 0; i < SENT_CH_COUNT; i++)
    {
        SENT_DRV_IRQ_Dispatch(0, i);
    }
}

#if (SENT_INSTANCE_COUNT > 1u)
void SENT1_IRQHandler(void)
{
    for(uint8_t i = 0; i < SENT_CH_COUNT; i++)
    {
        SENT_DRV_IRQ_Dispatch(1, i);
    }
}
#endif /* (SENT_INSTANCE_COUNT > 1u) */
/*******************************************************************************
* EOF
*******************************************************************************/




