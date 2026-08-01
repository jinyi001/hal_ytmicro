/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file rtc_irq.c
 * @version 1.4.1
 *
 * @brief RTC interrupt vector wrappers.
 *
 * This file connects the device interrupt vectors to the instance-based RTC
 * driver interrupt handlers.
 */

#include "rtc_driver.h"

/*!
 * @brief Generic RTC interrupt vector for instance 0.
 */
void RTC_IRQHandler(void);

#if FEATURE_RTC_HAS_SEPARATE_SECOND_IRQ

/*!
 * @brief Dedicated RTC seconds interrupt vector for instance 0.
 */
void RTC_Seconds_IRQHandler(void);

#endif

/*!
 * @brief Dispatch the generic RTC interrupt to the driver layer.
 */
void RTC_IRQHandler(void)
{
    RTC_DRV_IRQHandler(0U);
#if (FEATURE_RTC_HAS_SEPARATE_SECOND_IRQ == 0)
    RTC_DRV_SecondsIRQHandler(0U);
#endif
}

#if FEATURE_RTC_HAS_SEPARATE_SECOND_IRQ

/*!
 * @brief Dispatch the dedicated RTC seconds interrupt to the driver layer.
 */
void RTC_Seconds_IRQHandler(void)
{
    RTC_DRV_SecondsIRQHandler(0U);
}

#endif
