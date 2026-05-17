/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file ptu_driver.c
 * @version 1.4.1
 */

#include "ptu_driver.h"
#include "ptu_hw_access.h"
#include "interrupt_manager.h"


/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Table of base addresses for PTU instances. */
static PTU_Type *const s_ptuBase[PTU_INSTANCE_COUNT] = PTU_BASE_PTRS;

/* Table to save PTU IRQ enum numbers. */
static const IRQn_Type s_ptuIrqId[PTU_INSTANCE_COUNT] = PTU_IRQS;

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_Init
 * Description   : This function initializes the PTU counter, input triggers
 * and general pre-trigger settings.
 * It resets PTU registers and enables the clock for PTU. So it should be
 * called before any operation to PTU module. After initialized, the PTU can
 * act as a triggered timer, which lays the foundation for other features in
 * PTU module.
 *
 * Implements : PTU_DRV_Init_Activity
 *END*************************************************************************/
void PTU_DRV_Init(const uint32_t instance,
                  const ptu_timer_config_t *const userConfigPtr)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    DEV_ASSERT(userConfigPtr != NULL);
    /* Table of PTU clock names as defined in clock manager. */

    PTU_Type *base = s_ptuBase[instance];

    /* Reset the registers for PTU module to reset state. */
    PTU_Init(base);
    PTU_ConfigTimer(base, userConfigPtr);

    /* Configure NVIC. */
    if (userConfigPtr->intEnable)
    {
        INT_SYS_EnableIRQ(s_ptuIrqId[instance]); /* Enable PTU interrupt in NVIC level.*/
    } else
    {
        INT_SYS_DisableIRQ(s_ptuIrqId[instance]); /* Disable PTU interrupt in NVIC level.*/
    }
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_Deinit
 * Description   : De-initialize the PTU module.
 * When the PTU module is not used. Calling this function would shut down the
 * PTU module and reduce the power consumption.
 *
 * Note: instance back to back configuration is common between PTU instances 0 and 1 (configures the same register even if configured for either PTU instance)
 * This function disables it, so affects all other instances.
 *
 * Implements : PTU_DRV_Deinit_Activity
 *END*************************************************************************/
void PTU_DRV_Deinit(const uint32_t instance)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    PTU_Type *base = s_ptuBase[instance];

    INT_SYS_DisableIRQ(s_ptuIrqId[instance]);

    PTU_Init(base);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_GetDefaultConfig
 * Description   : Gets the default configuration structure of PTU with default settings.
 * This function is used to initialize the hardware configuration structure to default values.
 *
 * Implements : PTU_DRV_GetDefaultConfig_Activity
 *END*************************************************************************/
void PTU_DRV_GetDefaultConfig(ptu_timer_config_t *const config)
{
    DEV_ASSERT(config != NULL);
    /* Load immediately after load operation. */
    config->loadValueMode = PTU_LOAD_VAL_IMMEDIATELY;
    /* Prescaler divider select by MULT */
    config->clkPreDiv = PTU_CLK_PREDIV_BY_1;
    /* Select multiplication source mode */
    config->clkPreMultFactor = PTU_CLK_PREMULT_FACT_AS_1;
    /* Trigger input source selection */
    config->triggerInput = PTU_SOFTWARE_TRIGGER;
    /* Run in One-shot mode */
    config->continuousModeEnable = false;
    /* Disable DMA */
    config->dmaEnable = false;
    /* Disable interrupt */
    config->intEnable = false;
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_Enable
 * Description   : Enables the PTU module, counter is on.
 *
 * Implements : PTU_DRV_Enable_Activity
 *END*************************************************************************/
void PTU_DRV_Enable(const uint32_t instance)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);

    PTU_Type *base = s_ptuBase[instance];
    PTU_Enable(base);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_Disable
 * Description   : This function disables the PTU module, counter is off also.
 *
 * Implements : PTU_DRV_Disable_Activity
 *END*************************************************************************/
void PTU_DRV_Disable(const uint32_t instance)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);

    PTU_Type *base = s_ptuBase[instance];
    PTU_Disable(base);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_SoftTriggerCmd
 * Description   : Trigger PTU by software trigger.
 * When the PTU is set to use software trigger as input, Calling this function
 * would trigger the PTU.
 *
 * Implements : PTU_DRV_SoftTriggerCmd_Activity
 *END*************************************************************************/
void PTU_DRV_SoftTriggerCmd(const uint32_t instance)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    PTU_Type *base = s_ptuBase[instance];

    PTU_SetSoftTriggerCmd(base);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_GetTimerValue
 * Description   : Get the current value of the PTU counter.
 *
 * Implements : PTU_DRV_GetTimerValue_Activity
 *END*************************************************************************/
uint32_t PTU_DRV_GetTimerValue(const uint32_t instance)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    PTU_Type const *base = s_ptuBase[instance];

    return PTU_GetTimerValue(base);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_GetTimerIntFlag
 * Description   : Get the interrupt flag for PTU module. It will be
 * asserted if the PTU interrupt occurs.
 *
 * Implements : PTU_DRV_GetTimerIntFlag_Activity
 *END*************************************************************************/
bool PTU_DRV_GetTimerIntFlag(const uint32_t instance)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    PTU_Type const *base = s_ptuBase[instance];

    return PTU_GetTimerIntFlag(base);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_ClearTimerIntFlag
 * Description   : Clear the interrupt flag for PTU module.
 *
 * Implements : PTU_DRV_ClearTimerIntFlag_Activity
 *END*************************************************************************/
void PTU_DRV_ClearTimerIntFlag(const uint32_t instance)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    PTU_Type *base = s_ptuBase[instance];

    PTU_ClearTimerIntFlag(base);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_LoadValuesCmd
 * Description   : Execute the command of loading values.
 *
 * Implements : PTU_DRV_LoadValuesCmd_Activity
 *END*************************************************************************/
void PTU_DRV_LoadValuesCmd(const uint32_t instance)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    PTU_Type *base = s_ptuBase[instance];

    PTU_SetLoadValuesCmd(base);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_SetTimerModulusValue
 * Description   : Set the value of timer modulus.
 *
 * Implements : PTU_DRV_SetTimerModulusValue_Activity
 *END*************************************************************************/
void PTU_DRV_SetTimerModulusValue(const uint32_t instance,
                                  const uint16_t value)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    PTU_Type *base = s_ptuBase[instance];

    PTU_SetTimerModulusValue(base, value);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_SetValueForTimerInterrupt
 * Description   : Set the value for the timer interrupt.
 *
 * Implements : PTU_DRV_SetValueForTimerInterrupt_Activity
 *END*************************************************************************/
void PTU_DRV_SetValueForTimerInterrupt(const uint32_t instance,
                                       const uint16_t value)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    PTU_Type *base = s_ptuBase[instance];

    PTU_SetValueForTimerInterrupt(base, value);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_ConfigAdcPreTrigger
 * Description   : Configure the ADC pre_trigger in the PTU module.
 *
 * Implements : PTU_DRV_ConfigAdcPreTrigger_Activity
 *END*************************************************************************/
void PTU_DRV_ConfigAdcPreTrigger(const uint32_t instance,
                                 const ptu_adc_pretrigger_config_t *configPtr)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    DEV_ASSERT(configPtr != NULL);
    PTU_Type *base = s_ptuBase[instance];

    PTU_SetAdcPreTriggerEnable(base, ((uint32_t) 1U << (configPtr->adcPreTriggerIdx)), configPtr->preTriggerEnable);
    PTU_SetAdcPreTriggerOutputEnable(base, ((uint32_t) 1U << (configPtr->adcPreTriggerIdx)),
                                     configPtr->preTriggerOutputEnable);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_GetAdcPreTriggerFlags
 * Description   : Get the ADC pre_trigger flag in the PTU module.
 *
 * Implements : PTU_DRV_GetAdcPreTriggerFlags_Activity
 *END*************************************************************************/
uint32_t PTU_DRV_GetAdcPreTriggerFlags(const uint32_t instance,
                                       const uint32_t preChnMask)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    PTU_Type const *base = s_ptuBase[instance];

    return PTU_GetAdcPreTriggerFlags(base, preChnMask);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_ClearAdcPreTriggerFlags
 * Description   : Clear the ADC pre_trigger flag in the PTU module.
 *
 * Implements : PTU_DRV_ClearAdcPreTriggerFlags_Activity
 *END*************************************************************************/
void PTU_DRV_ClearAdcPreTriggerFlags(const uint32_t instance,
                                     const uint32_t preChnMask)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    PTU_Type *base = s_ptuBase[instance];

    PTU_ClearAdcPreTriggerFlags(base, preChnMask);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_SetAdcPreTriggerDelayValue
 * Description   : Set the ADC pre_trigger delay value in the PTU module.
 *
 * Implements : PTU_DRV_SetAdcPreTriggerDelayValue_Activity
 *END*************************************************************************/
void PTU_DRV_SetAdcPreTriggerDelayValue(const uint32_t instance,
                                        const uint32_t preChn,
                                        const uint32_t value)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    DEV_ASSERT(value > 0u);
    PTU_Type *base = s_ptuBase[instance];

    PTU_SetAdcPreTriggerDelayValue(base, preChn, value);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_SetCmpPulseOutEnable
 * Description   : Switch on/off the CMP pulse out in the PTU module.
 *
 * Implements : PTU_DRV_SetCmpPulseOutEnable_Activity
 *END*************************************************************************/
void PTU_DRV_SetCmpPulseOutEnable(const uint32_t instance,
                                  const bool enable)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    PTU_Type *base = s_ptuBase[instance];

    PTU_SetCmpPulseOutEnable(base, enable);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_SetCmpPulseOutDelayForHigh
 * Description   : Set the CMP pulse out delay value for high in the PTU module.
 *
 * Implements : PTU_DRV_SetCmpPulseOutDelayForHigh_Activity
 *END*************************************************************************/
void PTU_DRV_SetCmpPulseOutDelayForHigh(const uint32_t instance,
                                        const uint16_t value)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    PTU_Type *base = s_ptuBase[instance];

    PTU_SetCmpPulseOutDelayForHigh(base, value);
}

/*FUNCTION*********************************************************************
 *
 * Function Name : PTU_DRV_SetCmpPulseOutDelayForLow
 * Description   : Set the CMP pulse out delay value for low in the PTU module.
 *
 * Implements : PTU_DRV_SetCmpPulseOutDelayForLow_Activity
 *END*************************************************************************/
void PTU_DRV_SetCmpPulseOutDelayForLow(const uint32_t instance,
                                       const uint16_t value)
{
    DEV_ASSERT(instance < PTU_INSTANCE_COUNT);
    PTU_Type *base = s_ptuBase[instance];

    PTU_SetCmpPulseOutDelayForLow(base, value);
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
