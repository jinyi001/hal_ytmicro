/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file hcu_irq.c
 * @version 1.4.0
 */

#include "device_registers.h"
#include "hcu_driver.h"

/*!
 * @addtogroup hcu_driver_v2
 * @{
 */

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

#ifdef HCU

/*!
 * @brief This function is the implementation of HCU handler named in startup code.
 *
 * It passes the instance to the shared HCU IRQ handler.
 */
void HCU_IRQHandler(void);

void HCU_IRQHandler(void)
{
    HCU_DRV_IRQHandler();
}

#endif
/*******************************************************************************
 * EOF
 ******************************************************************************/

