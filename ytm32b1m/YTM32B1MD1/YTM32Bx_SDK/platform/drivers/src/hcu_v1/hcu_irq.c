/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file hcu_irq.c
 * @version 1.4.1
 *
 * @brief HCU V1 interrupt-vector wrapper.
 *
 * This file exposes the startup-visible HCU interrupt symbol and forwards the
 * interrupt flow into the shared driver handler implemented in
 * `HCU_DRV_IRQHandler()`.
 */

#include "device_registers.h"
#include "hcu_driver.h"

/*!
 * @addtogroup hcu_driver_v1
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
 * @brief Forward the startup-level HCU interrupt to the driver handler.
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
