/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file sent_irq.h
 * @version 1.4.0
 */

#ifndef SENT_IRQ_H
#define SENT_IRQ_H

#include <stddef.h>
#include "device_registers.h"

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * IRQ IDs
 */
typedef enum {
    SENT_IRQ_FAST = 0,
    SENT_IRQ_SLOW = 1,
    SENT_IRQ_ERROR = 2
} sent_interrupt_id_t;

/**
 * @brief Enable interrupt by mapping
 *
 * Platform specific
 *
 * @param[in] instance Peripheral instance
 * @param[in] channel Communication channel
 * @param[in] irqMap Interrupt map
 */
void SENT_DRV_IRQ_EnableIRQ(const uint32_t instance, const uint32_t channel, const sent_interrupt_id_t id);

/**
 * @brief Disable interrupt by mapping
 *
 * Platform specific
 *
 * @param[in] instance Peripheral instance
 * @param[in] channel Communication channel
 * @param[in] irqMap Interrupt map
 */
void SENT_DRV_IRQ_DisableIRQ(const uint32_t instance, const uint32_t channel, const sent_interrupt_id_t id);


/**
 * @brief Interrupt dispatcher
 *
 * Interrupt dispatcher
 *
 * @param[in] instance Peripheral instance
 * @param[in] channel Communication channel
 */
void SENT_DRV_IRQ_Dispatch(const uint32_t instance, const uint32_t channel);

/**
 * @brief Driver side interrupt
 *
 * Fast Rx Event.
 * Gets called from the low level handler
 * with instance and channel as parameter
 *
 * @param[in] instance Peripheral instance
 * @param[in] channel Communication channel
 */
void SENT_DRV_IRQ_FastHandler(const uint32_t instance, const uint32_t channel);

/**
 * @brief Driver side interrupt
 *
 * Slow Rx Event.
 * Gets called from the low level handler
 * with instance and channel as parameter
 *
 * @param[in] instance Peripheral instance
 * @param[in] channel Communication channel
 */
void SENT_DRV_IRQ_SlowHandler(const uint32_t instance, const uint32_t channel);

/**
 * @brief Driver side interrupt
 *
 * Rx Error Event.
 * Gets called from the low level handler
 * with instance and channel as parameter
 *
 * @param[in] instance Peripheral instance
 * @param[in] channel Communication channel
 */
void SENT_DRV_IRQ_WakeupHandler(const uint32_t instance, const uint32_t channel);

/**
 * @brief Driver side interrupt
 *
 * Rx Error Event.
 * Gets called from the low level handler
 * with instance and channel as parameter
 *
 * @param[in] instance Peripheral instance
 * @param[in] channel Communication channel
 */
void SENT_DRV_IRQ_RxErrHandler(const uint32_t instance, const uint32_t channel);

#if defined(__cplusplus)
}
#endif

#endif /* SENT_IRQ_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
