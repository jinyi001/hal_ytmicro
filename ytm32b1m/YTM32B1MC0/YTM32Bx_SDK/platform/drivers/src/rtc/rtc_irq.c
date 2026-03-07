/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file rtc_irq.c
 * @version 1.4.0
 */

#include "rtc_driver.h"

void RTC_IRQHandler(void);

#if FEATURE_RTC_HAS_SEPARATE_SECOND_IRQ

void RTC_Seconds_IRQHandler(void);

#endif

void RTC_IRQHandler(void)
{
    RTC_DRV_IRQHandler(0U);
#if (FEATURE_RTC_HAS_SEPARATE_SECOND_IRQ == 0)
    RTC_DRV_SecondsIRQHandler(0U);
#endif
}

#if FEATURE_RTC_HAS_SEPARATE_SECOND_IRQ

void RTC_Seconds_IRQHandler(void)
{
    RTC_DRV_SecondsIRQHandler(0U);
}

#endif
