/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file linflexd_uart_irq.h
 * @brief LINFlexD UART interrupt handler declarations.
 */

#ifndef LINFlexD_UART_IRQ_H
#define LINFlexD_UART_IRQ_H

#include "linflexd_hw_access.h"

/*!
 * @brief Or'ed interrupt handler, implemented in driver c file.
 */
void LINFlexD_UART_DRV_IRQHandler(uint32_t instance);

/*!
 * @brief Rx interrupt handler, implemented in driver c file.
 */
void LINFlexD_UART_DRV_RxIRQHandler(uint32_t instance);

/*!
 * @brief Tx interrupt handler, implemented in driver c file.
 */
void LINFlexD_UART_DRV_TxIRQHandler(uint32_t instance);

/*!
 * @brief Error interrupt handler, implemented in driver c file.
 */
void LINFlexD_UART_DRV_ErrIRQHandler(uint32_t instance);

/*!
 * @brief ISR declarations - implemented in linflexd_uart_irq.c.
 */
#if defined(FEATURE_LINFlexD_ORED_INT_LINES) && (FEATURE_LINFlexD_ORED_INT_LINES == 1U)

    /* ISR declarations for rx/tx/err interrupts or'ed together */
    #if (LINFlexD_INSTANCE_COUNT > 0U)
    void LINFlexD0_UART_IRQHandler(void);
    #endif

    #if (LINFlexD_INSTANCE_COUNT > 1U)
    void LINFlexD1_UART_IRQHandler(void);
    #endif

    #if (LINFlexD_INSTANCE_COUNT > 2U)
    void LINFlexD2_UART_IRQHandler(void);
    #endif

    #if (LINFlexD_INSTANCE_COUNT > 3U)
    void LINFlexD3_UART_IRQHandler(void);
    #endif

    #if (LINFlexD_INSTANCE_COUNT > 4U)
    void LINFlexD4_UART_IRQHandler(void);
    #endif

    #if (LINFlexD_INSTANCE_COUNT > 5U)
    void LINFlexD5_UART_IRQHandler(void);
    #endif

    #if (LINFlexD_INSTANCE_COUNT > 6U)
    void LINFlexD6_UART_IRQHandler(void);
    #endif

    #if (LINFlexD_INSTANCE_COUNT > 7U)
    void LINFlexD7_UART_IRQHandler(void);
    #endif

    #if (LINFlexD_INSTANCE_COUNT > 8U)
    void LINFlexD8_UART_IRQHandler(void);
    #endif

    #if (LINFlexD_INSTANCE_COUNT > 9U)
    void LINFlexD9_UART_IRQHandler(void);
    #endif

    /* ISR array for LINFlexD UART driver */
    extern const isr_t g_uartLINFlexDIsr[LINFlexD_INSTANCE_COUNT];

#endif

#endif
