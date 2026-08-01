/*
 * Copyright (c) 2021-2025 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file ewdg_driver.c
 * @version 1.4.1
 *
 * @brief EWDG Driver — implementation of the public API.
 *
 * This file implements the instance-based EWDG driver declared in
 * `ewdg_driver.h`. The implementation validates startup parameters,
 * resolves the hardware base address for each instance, protects the
 * refresh and restore sequences from interrupt latency, and exposes
 * lightweight helpers for input-mode query and interrupt masking.
 */

#include "stddef.h"
#include "ewdg_hw_access.h"
#include "interrupt_manager.h"
/*! @brief Local table used to store EWDG base pointers. */
static EWDG_Type * const s_ewdgBase[] = EWDG_BASE_PTRS;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Initialize one EWDG instance with the requested watchdog settings.
 *
 * @param[in] instance  EWDG instance index.
 * @param[in] config    Pointer to the startup configuration.
 * @return Execution status.
 */
status_t EWDG_DRV_Init(uint32_t instance, const ewdg_init_config_t *config)
{
    DEV_ASSERT(instance < EWDG_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    /* Return status variable */
    status_t statusCode = STATUS_SUCCESS;
    /* Flag to store if the module is enabled */
    bool isModuleEnabled;
    uint32_t tempValue = 0U;
    /* Base pointer */
    EWDG_Type * base = s_ewdgBase[instance];

    /* Get the enablement status of the module */
    isModuleEnabled = EWDG_IsEnabled(base);
    /* Check if the EWDG instance is already enabled or if the windows values are not correct */
    if ((isModuleEnabled == true) || (config->compareHigh <= config->compareLow) ||
        (config->compareHigh > FEATURE_EWDG_CMPH_MAX_VALUE))
    {
        /* If conditions are met change the status code to error */
        statusCode = STATUS_ERROR;
    }
    else
    {
        /* Set clock prescaler */
        EWDG_SetPrescaler(base, config->prescaler);
#if (defined(EWDG_CLK_CTRL_CLKSEL_MASK))
        /* Set clock source */
        EWDG_SetClockSource(base, (uint8_t)config->clockSource);
#endif
        /* Set compare high and low values */
        EWDG_SetCompareHigh(base, config->compareHigh);
        EWDG_SetCompareLow(base, config->compareLow);

        /* Configure the Control register and enable the instance.
         * Set the values that are not affected by the input pin
         */
        tempValue |= EWDG_CTRL_EN(1U);
        if (config->interruptEnable)
        {
            tempValue |= EWDG_CTRL_INTEN(1U);
        }

        /* Depending on how the input pin is configured set the values into the
         * temporary variable
         */
        switch (config->assertLogic)
        {
            case EWDG_IN_ASSERT_ON_LOGIC_ZERO:
                tempValue |= EWDG_CTRL_INEN_MASK;    /* Input pin enabled, Input asserted on logic 0 */
                break;
            case EWDG_IN_ASSERT_ON_LOGIC_ONE:
                tempValue |= (EWDG_CTRL_INEN_MASK | EWDG_CTRL_POLARITY_MASK);    /* Input pin enabled, Input asserted on logic 1 */
                break;
            case EWDG_IN_ASSERT_DISABLED:
            default:
                /* Input pin disabled */
                break;
        }

        /* Write the configuration into the Control register */
        EWDG_SetControl(base, tempValue);
    }

    /* Return the status code */
    return statusCode;
}

/*!
 * @brief Populate an EWDG configuration structure with default values.
 *
 * @param[out] config  Pointer to the configuration structure to initialize.
 */
void EWDG_DRV_GetDefaultConfig(ewdg_init_config_t * config)
{
    DEV_ASSERT(config != NULL);

    /* Disable interrupts */
    config->interruptEnable = false;
    /* Input pin enabled and configured to assert on logic 0 */
    config->assertLogic     = EWDG_IN_ASSERT_ON_LOGIC_ZERO;
    /* Maximum prescaler */
    config->prescaler       = 255U;
#if (defined(EWDG_CLK_CTRL_CLKSEL_MASK))
    /* Clock source is the internal clock */
    config->clockSource = EWDG_SIRC_CLOCK;
#endif
    /* Maximum service window */
    config->compareHigh     = FEATURE_EWDG_CMPH_MAX_VALUE;
    config->compareLow      = FEATURE_EWDG_CMPL_MIN_VALUE;
}

/*!
 * @brief Refresh the EWDG counter inside the programmed service window.
 *
 * @param[in] instance  EWDG instance index.
 */
void EWDG_DRV_Refresh(uint32_t instance)
{
    DEV_ASSERT(instance < EWDG_INSTANCE_COUNT);

    /* Base pointer */
    EWDG_Type * base = s_ewdgBase[instance];

    /* Disable global IRQ's to avoid disturbing the refresh process */
    INT_SYS_DisableIRQGlobal();
    /* Write the refresh values */
    EWDG_Refresh(base);
    /* Re-enable all IRQ's */
    INT_SYS_EnableIRQGlobal();
}

/*!
 * @brief Read the configured EWDG input-pin assert behavior.
 *
 * @param[in] instance  EWDG instance index.
 * @return Configured input-pin assert behavior.
 */
ewdg_in_assert_logic_t EWDG_DRV_GetInputPinAssertLogic(uint32_t instance)
{
    DEV_ASSERT(instance < EWDG_INSTANCE_COUNT);

    /* Base pointer */
    const EWDG_Type * const base = s_ewdgBase[instance];
    /* Variable where to save the retrieved configuration */
    ewdg_in_assert_logic_t returnValue;

    /* Check if input pin is enabled */
    if ((EWDG_GetControl(base) & EWDG_CTRL_INEN_MASK) != 0U)
    {
       /* If true get assert logic into the returnValue variable */
        returnValue = (((EWDG_GetControl(base) & EWDG_CTRL_POLARITY_MASK) != 0U)
                       ? EWDG_IN_ASSERT_ON_LOGIC_ONE : EWDG_IN_ASSERT_ON_LOGIC_ZERO);
    }
    else
    {
        /* Pin is disabled, return the corresponding value */
        returnValue = EWDG_IN_ASSERT_DISABLED;
    }

    return returnValue;
}

/*!
 * @brief Restore one EWDG instance to its reset-like register state.
 *
 * @param[in] instance  EWDG instance index.
 */
void EWDG_DRV_DeInit(uint32_t instance)
{
     /* Base pointer */
    EWDG_Type * base = s_ewdgBase[instance];

    /* Disable global interrupt*/
    INT_SYS_DisableIRQGlobal();
    EWDG_DeInit(base);
    /* Enable global interrupt */
    INT_SYS_EnableIRQGlobal();
}

/*!
 * @brief Disable EWDG interrupt generation for one instance.
 *
 * @param[in] instance  EWDG instance index.
 */
void EWDG_DRV_DisableInterrupt(uint32_t instance)
{
    uint32_t tmp = 0;
    EWDG_Type *base = s_ewdgBase[instance];

    tmp = EWDG_GetControl(base);
    EWDG_SetControl(base, tmp & ~EWDG_CTRL_INTEN_MASK);
}

/*! @}*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
