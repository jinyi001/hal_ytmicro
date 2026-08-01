/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file lptmr_hw_access.c
 * @version 1.4.1
 *
 * @brief lpTMR Hardware Access Layer — reset-state initialization support.
 */

#include "lptmr_hw_access.h"

/*******************************************************************************
 * Initialization
 ******************************************************************************/

/*!
 * @brief Initialize the lpTMR registers to their reset-state values.
 */
void lpTMR_Init(lpTMR_Type* const base)
{
    DEV_ASSERT(base != NULL);

    /* Disable the module first so the remaining registers can be updated safely. */
    uint32_t tmp = base->CTRL;
    tmp &= ~(lpTMR_CTRL_EN_MASK);
    base->CTRL = tmp;

    base->CTRL = lpTMR_CTRL_EN(0u)      | \
                 lpTMR_CTRL_MODE(0u)    | \
                 lpTMR_CTRL_TMODE(0u)   | \
                 lpTMR_CTRL_PINPOL(0u)  | \
                 lpTMR_CTRL_PINSEL(0u);
    base->STS = lpTMR_STS_CCF_MASK;
#if defined(lpTMR_DIE_DMAEN_MASK)
    base->DIE = lpTMR_DIE_DMAEN(0u)     | \
                lpTMR_DIE_IE(0u);
#else
    base->DIE = lpTMR_DIE_IE(0u);
#endif /* lpTMR_DIE_DMAEN_MASK */

    base->PRS = lpTMR_PRS_BYPASS(0u)   | \
                lpTMR_PRS_PRES(0u);

    base->CMP = lpTMR_CMP_CMP(0u);
}
/*******************************************************************************
 * EOF
 ******************************************************************************/
