/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file linflexd_uart_irq.c
 * @brief LINFlexD UART interrupt dispatch implementation.
 */

/*!
* @page misra_violations MISRA-C:2012 violations list
*
* PRQA S 1533 Rule 8.9: The object '%1s' is only referenced by function '%2s'.
*
*/

#include "linflexd_uart_irq.h"

#if defined(FEATURE_LINFlexD_ORED_INT_LINES) && (FEATURE_LINFlexD_ORED_INT_LINES == 1U)

#if (LINFlexD_INSTANCE_COUNT > 0U)
/* Implementation of LINFlexD0 handler named in startup code. */
void LINFlexD0_UART_IRQHandler(void)
{
    LINFlexD_UART_DRV_IRQHandler(0U);
}
#endif

#if (LINFlexD_INSTANCE_COUNT > 1U)
/* Implementation of LINFlexD1 handler named in startup code. */
void LINFlexD1_UART_IRQHandler(void)
{
    LINFlexD_UART_DRV_IRQHandler(1U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 2U)
/* Implementation of LINFlexD2 handler named in startup code. */
void LINFlexD2_UART_IRQHandler(void)
{
    LINFlexD_UART_DRV_IRQHandler(2U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 3U)
/* Implementation of LINFlexD3 handler named in startup code. */
void LINFlexD3_UART_IRQHandler(void)
{
    LINFlexD_UART_DRV_IRQHandler(3U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 4U)
/* Implementation of LINFlexD4 handler named in startup code. */
void LINFlexD4_UART_IRQHandler(void)
{
    LINFlexD_UART_DRV_IRQHandler(4U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 5U)
/* Implementation of LINFlexD5 handler named in startup code. */
void LINFlexD5_UART_IRQHandler(void)
{
    LINFlexD_UART_DRV_IRQHandler(5U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 6U)
/* Implementation of LINFlexD6 handler named in startup code. */
void LINFlexD6_UART_IRQHandler(void)
{
    LINFlexD_UART_DRV_IRQHandler(6U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 7U)
/* Implementation of LINFlexD7 handler named in startup code. */
void LINFlexD7_UART_IRQHandler(void)
{
    LINFlexD_UART_DRV_IRQHandler(7U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 8U)
/* Implementation of LINFlexD8 handler named in startup code. */
void LINFlexD8_UART_IRQHandler(void)
{
    LINFlexD_UART_DRV_IRQHandler(8U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 9U)
/* Implementation of LINFlexD9 handler named in startup code. */
void LINFlexD9_UART_IRQHandler(void)
{
    LINFlexD_UART_DRV_IRQHandler(9U);
}
#endif

/* ISR array for LINFlexD UART driver */
const isr_t g_uartLINFlexDIsr[LINFlexD_INSTANCE_COUNT] = { /*PRQA S 1533*/
    #if (LINFlexD_INSTANCE_COUNT > 0U)
    &LINFlexD0_UART_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 1U)
    &LINFlexD1_UART_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 2U)
    &LINFlexD2_UART_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 3U)
    &LINFlexD3_UART_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 4U)
    &LINFlexD4_UART_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 5U)
    &LINFlexD5_UART_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 6U)
    &LINFlexD6_UART_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 7U)
    &LINFlexD7_UART_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 8U)
    &LINFlexD8_UART_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 9U)
    &LINFlexD9_UART_IRQHandler,
    #endif
};

#endif
/*******************************************************************************
 * EOF
 ******************************************************************************/

