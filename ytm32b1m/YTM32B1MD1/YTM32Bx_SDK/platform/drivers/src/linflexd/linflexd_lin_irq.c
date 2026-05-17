/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file linflexd_lin_irq.c
 * @version 1.4.1
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 1533 Rule 8.9: The object '%1s' is only referenced by function '%2s'.
 *
 */

#include "linflexd_lin_irq.h"

#if (FEATURE_LINFlexD_ORED_INT_LINES)

#if (LINFlexD_INSTANCE_COUNT > 0U)
/* Implementation of LINFlexD0 handler named in startup code. */
void LINFlexD0_LIN_IRQHandler(void)
{
    LINFlexD_LIN_DRV_IRQHandler(0U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 1U)
/* Implementation of LINFlexD1 handler named in startup code. */
void LINFlexD1_LIN_IRQHandler(void)
{
    LINFlexD_LIN_DRV_IRQHandler(1U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 2U)
/* Implementation of LINFlexD2 handler named in startup code. */
void LINFlexD2_LIN_IRQHandler(void)
{
    LINFlexD_LIN_DRV_IRQHandler(2U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 3U)
/* Implementation of LINFlexD3 handler named in startup code. */
void LINFlexD3_LIN_IRQHandler(void)
{
    LINFlexD_LIN_DRV_IRQHandler(3U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 4U)
/* Implementation of LINFlexD4 handler named in startup code. */
void LINFlexD4_LIN_IRQHandler(void)
{
    LINFlexD_LIN_DRV_IRQHandler(4U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 5U)
/* Implementation of LINFlexD5 handler named in startup code. */
void LINFlexD5_LIN_IRQHandler(void)
{
    LINFlexD_LIN_DRV_IRQHandler(5U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 6U)
/* Implementation of LINFlexD6 handler named in startup code. */
void LINFlexD6_LIN_IRQHandler(void)
{
    LINFlexD_LIN_DRV_IRQHandler(6U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 7U)
/* Implementation of LINFlexD7 handler named in startup code. */
void LINFlexD7_LIN_IRQHandler(void)
{
    LINFlexD_LIN_DRV_IRQHandler(7U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 8U)
/* Implementation of LINFlexD8 handler named in startup code. */
void LINFlexD8_LIN_IRQHandler(void)
{
    LINFlexD_LIN_DRV_IRQHandler(8U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 9U)
/* Implementation of LINFlexD9 handler named in startup code. */
void LINFlexD9_LIN_IRQHandler(void)
{
    LINFlexD_LIN_DRV_IRQHandler(9U);
}
#endif
/* ISR array for LINFlexD LIN driver */
const isr_t g_LinLINFlexDIsr[LINFlexD_INSTANCE_COUNT] = { /*PRQA S 1533*/
    #if (LINFlexD_INSTANCE_COUNT > 0U)
    &LINFlexD0_LIN_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 1U)
    &LINFlexD1_LIN_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 2U)
    &LINFlexD2_LIN_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 3U)
    &LINFlexD3_LIN_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 4U)
    &LINFlexD4_LIN_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 5U)
    &LINFlexD5_LIN_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 6U)
    &LINFlexD6_LIN_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 7U)
    &LINFlexD7_LIN_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 8U)
    &LINFlexD8_LIN_IRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 9U)
    &LINFlexD9_LIN_IRQHandler,
    #endif
};

#elif (FEATURE_LINFlexD_RX_TX_ERR_INT_LINES)

#if (LINFlexD_INSTANCE_COUNT > 0U)
/* Implementation of LINFlexD0 rx handler named in startup code. */
void LINFlexD0_LIN_RxIRQHandler(void)
{
    LINFlexD_LIN_DRV_RxIRQHandler(0U);
}

/* Implementation of LINFlexD0 tx handler named in startup code. */
void LINFlexD0_LIN_TxIRQHandler(void)
{
    LINFlexD_LIN_DRV_TxIRQHandler(0U);
}

/* Implementation of LINFlexD0 error handler named in startup code. */
void LINFlexD0_LIN_ErrIRQHandler(void)
{
    LINFlexD_LIN_DRV_ErrIRQHandler(0U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 1U)
/* Implementation of LINFlexD1 rx handler named in startup code. */
void LINFlexD1_LIN_RxIRQHandler(void)
{
    LINFlexD_LIN_DRV_RxIRQHandler(0U);
}

/* Implementation of LINFlexD1 tx handler named in startup code. */
void LINFlexD1_LIN_TxIRQHandler(void)
{
    LINFlexD_LIN_DRV_TxIRQHandler(0U);
}

/* Implementation of LINFlexD1 error handler named in startup code. */
void LINFlexD1_LIN_ErrIRQHandler(void)
{
    LINFlexD_LIN_DRV_ErrIRQHandler(0U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 2U)
/* Implementation of LINFlexD2 rx handler named in startup code. */
void LINFlexD2_LIN_RxIRQHandler(void)
{
    LINFlexD_LIN_DRV_RxIRQHandler(0U);
}

/* Implementation of LINFlexD2 tx handler named in startup code. */
void LINFlexD2_LIN_TxIRQHandler(void)
{
    LINFlexD_LIN_DRV_TxIRQHandler(0U);
}

/* Implementation of LINFlexD2 error handler named in startup code. */
void LINFlexD2_LIN_ErrIRQHandler(void)
{
    LINFlexD_LIN_DRV_ErrIRQHandler(0U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 3U)
/* Implementation of LINFlexD3 rx handler named in startup code. */
void LINFlexD3_LIN_RxIRQHandler(void)
{
    LINFlexD_LIN_DRV_RxIRQHandler(0U);
}

/* Implementation of LINFlexD3 tx handler named in startup code. */
void LINFlexD3_LIN_TxIRQHandler(void)
{
    LINFlexD_LIN_DRV_TxIRQHandler(0U);
}

/* Implementation of LINFlexD3 error handler named in startup code. */
void LINFlexD3_LIN_ErrIRQHandler(void)
{
    LINFlexD_LIN_DRV_ErrIRQHandler(0U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 4U)
/* Implementation of LINFlexD4 rx handler named in startup code. */
void LINFlexD4_LIN_RxIRQHandler(void)
{
    LINFlexD_LIN_DRV_RxIRQHandler(0U);
}

/* Implementation of LINFlexD4 tx handler named in startup code. */
void LINFlexD4_LIN_TxIRQHandler(void)
{
    LINFlexD_LIN_DRV_TxIRQHandler(0U);
}

/* Implementation of LINFlexD4 error handler named in startup code. */
void LINFlexD4_LIN_ErrIRQHandler(void)
{
    LINFlexD_LIN_DRV_ErrIRQHandler(0U);
}
#endif
#if (LINFlexD_INSTANCE_COUNT > 5U)
/* Implementation of LINFlexD5 rx handler named in startup code. */
void LINFlexD5_LIN_RxIRQHandler(void)
{
    LINFlexD_LIN_DRV_RxIRQHandler(0U);
}

/* Implementation of LINFlexD5 tx handler named in startup code. */
void LINFlexD5_LIN_TxIRQHandler(void)
{
    LINFlexD_LIN_DRV_TxIRQHandler(0U);
}

/* Implementation of LINFlexD5 error handler named in startup code. */
void LINFlexD5_LIN_ErrIRQHandler(void)
{
    LINFlexD_LIN_DRV_ErrIRQHandler(0U);
}
#endif

/* Rx ISR array for LINFlexD LIN driver */
const isr_t g_LinLINFlexDRxIsr[LINFlexD_INSTANCE_COUNT] = {
    #if (LINFlexD_INSTANCE_COUNT > 0U)
    &LINFlexD0_LIN_RxIRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 1U)
    &LINFlexD1_LIN_RxIRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 2U)
    &LINFlexD2_LIN_RxIRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 3U)
    &LINFlexD3_LIN_RxIRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 4U)
    &LINFlexD4_LIN_RxIRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 5U)
    &LINFlexD5_LIN_RxIRQHandler,
    #endif
};

/* Tx ISR array for LINFlexD LIN driver */
const isr_t g_LinLINFlexDTxIsr[LINFlexD_INSTANCE_COUNT] = {
    #if (LINFlexD_INSTANCE_COUNT > 0U)
    &LINFlexD0_LIN_TxIRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 1U)
    &LINFlexD1_LIN_TxIRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 2U)
    &LINFlexD2_LIN_TxIRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 3U)
    &LINFlexD3_LIN_TxIRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 4U)
    &LINFlexD4_LIN_TxIRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 5U)
    &LINFlexD5_LIN_TxIRQHandler,
    #endif
};

/* Err ISR array for LINFlexD LIN driver */
const isr_t g_LinLINFlexDErrIsr[LINFlexD_INSTANCE_COUNT] = {
    #if (LINFlexD_INSTANCE_COUNT > 0U)
    &LINFlexD0_LIN_ErrIRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 1U)
    &LINFlexD1_LIN_ErrIRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 2U)
    &LINFlexD2_LIN_ErrIRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 3U)
    &LINFlexD3_LIN_ErrIRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 4U)
    &LINFlexD4_LIN_ErrIRQHandler,
    #endif
    #if (LINFlexD_INSTANCE_COUNT > 5U)
    &LINFlexD5_LIN_ErrIRQHandler,
    #endif
};

#endif
/*******************************************************************************
 * EOF
 ******************************************************************************/

