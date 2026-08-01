/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file lptmr_driver.c
 * @version 1.4.1
 *
 * @brief lpTMR Driver — implementation of the public `lpTMR_DRV_*` API.
 */

#include "lptmr_driver.h"
#include "lptmr_hw_access.h"
#include "clock_manager.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* The compare register represents one tick less than the effective timeout. */
#define lpTMR_MAX_CMR_NTICKS (lpTMR_CMP_CMP_MASK + 1u)
#define lpTMR_MAX_PRESCALER  (1u << lpTMR_PRS_PRES_WIDTH)

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @cond DRIVER_INTERNAL_USE_ONLY */

/*! @brief Base-address table for lpTMR instances. */
static lpTMR_Type* const g_lptmrBase[lpTMR_INSTANCE_COUNT] = lpTMR_BASE_PTRS;
/*! @brief Clock-manager name table for instances that use IPC clock lookup. */
#if FEATURE_lpTMR_CLKSRC_SUPPORT_IPC
static const clock_names_t s_lptmrClkNames[lpTMR_INSTANCE_COUNT] = lpTMR_CLOCK_NAMES;
#endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_IPC */

/*! @brief Cached timer clock frequency used by the microsecond conversion helpers. */
static uint32_t s_lptmrClkFreq;

/*******************************************************************************
 * Timer Conversion Helpers
 ******************************************************************************/
/*!
 * @brief Convert prescaler configuration fields to a power-of-two shift value.
 */
static inline uint8_t lptmr_cfg2p(
    const lptmr_prescaler_t prescval,
    const bool bypass
    );

static inline uint64_t lptmr_us2nn(
    const uint32_t clkfreq,
    const uint32_t us
    );

static inline uint64_t lptmr_compute_nticks(
    uint64_t nn,
    uint8_t p
    );

static inline bool nticks2compare_ticks(
    uint64_t nticks,
    uint16_t* ticks
    );

static bool lptmr_Ticks2Us(
    const uint32_t clkfreq,
    const lptmr_prescaler_t pval,
    const bool bypass,
    const uint16_t ticks,
    uint32_t* const us
    );

static bool lptmr_ChooseClkConfig(
    const uint32_t clkfreq,
    const uint32_t us,
    lptmr_prescaler_t* const prescval,
    bool* const bypass,
    uint16_t* const ticks
    );

/*!
 * @brief Convert prescaler configuration fields to a power-of-two shift value.
 */
static inline uint8_t lptmr_cfg2p(
    const lptmr_prescaler_t prescval,
    const bool bypass
    )
{
    uint8_t p = 0u;

    if (!bypass)
    {
        p = (uint8_t)(((uint8_t)prescval) + 1u);
    }

    return p;
}

/*!
 * @brief Convert a microsecond period to unprescaled timer ticks.
 */
static inline uint64_t lptmr_us2nn(
    const uint32_t clkfreq,
    const uint32_t us
    )
{
    /* Round to the nearest input-clock tick when converting from microseconds. */
    /* The 64-bit intermediate avoids precision loss for supported clock ranges. */
    uint64_t nn = (uint64_t)( (uint64_t)us * (uint64_t)clkfreq );
    nn = (nn + 500000u) / 1000000u;
    return nn;
}

/*!
 * @brief Apply the selected prescaler shift to an unprescaled tick count.
 */
static inline uint64_t lptmr_compute_nticks(
    uint64_t nn,
    uint8_t p
    )
{
    uint64_t npresc = (uint64_t) 1u << p;
    DEV_ASSERT(npresc != 0u);

    /* Divide with rounding to the nearest prescaled tick. */
    uint64_t nticks = ((nn + (npresc >> 1u)) / npresc);

    return nticks;
}

/*!
 * @brief Convert an effective tick count to the value expected by the compare register.
 */
static inline bool nticks2compare_ticks(
    uint64_t nticks,
    uint16_t* ticks
    )
{
    bool success = true;

    /* Accept values that fit in the 16-bit compare register domain. */
    if (nticks <= lpTMR_MAX_CMR_NTICKS)
    {
        if (nticks == 0u)
        {
            /* The requested period is too short for the current prescaler choice. */
            *ticks = 0u;
            success = false;
        }
        else{
            /* Compensate for the extra increment described by the reference manual. */
            *ticks = (uint16_t)(nticks - 1u);
        }
    }
    else {
        /* Saturate when the requested period exceeds the compare register range. */
        *ticks = lpTMR_CMP_CMP_MASK;
        success = false;
    }

    return success;
}

/*!
 * @brief Convert the active compare setting from ticks back to microseconds.
 */
static bool lptmr_Ticks2Us(
    const uint32_t clkfreq,
    const lptmr_prescaler_t pval,
    const bool bypass,
    const uint16_t ticks,
    uint32_t* const us
    )
{
    bool success = true;
    uint8_t p = lptmr_cfg2p(pval, bypass);
    uint64_t nn = ( (uint64_t)ticks + 1u ) << p;
    uint64_t us_real = (nn * 1000000u) / (clkfreq);
    uint32_t us_local;

    if ( us_real <= (0xFFFFFFFFu) )
    {
        us_local = (uint32_t)us_real;
    }
    else
    {
        us_local = 0xFFFFFFFFu;
        success = false;
    }

    *us = us_local;
    return success;
}

/*!
 * @brief Choose the best prescaler and compare value for a microsecond period.
 */
static bool lptmr_ChooseClkConfig(
    const uint32_t clkfreq,
    const uint32_t us,
    lptmr_prescaler_t* const prescval,
    bool* const bypass,
    uint16_t* const ticks
    )
{
    uint8_t p;
    uint64_t nticks = 0ULL;
    bool success;

    uint64_t nn = lptmr_us2nn(clkfreq, us);

    /* Choose the smallest prescaler that keeps the compare value in range. */
    for (p = 0u; p <= lpTMR_MAX_PRESCALER; p++)
    {
        nticks = lptmr_compute_nticks(nn, p);

        if (nticks <= lpTMR_MAX_CMR_NTICKS)
        {
            /* Stop at the first prescaler that satisfies the 16-bit register limit. */
            break;
        }
    }

    success = nticks2compare_ticks(nticks, ticks);

    /* Translate the shift value back to the hardware configuration encoding. */
    if (p == 0u)
    {
        /* A shift of zero means the prescaler path is bypassed. */
        *bypass = true;
        *prescval = lpTMR_PRESCALE_2;
    }
    else{
        *bypass = false;
        p--; /* Adjust the shift count to the enum encoding. */
        *prescval = (lptmr_prescaler_t) p;
    }

    return success;
}


/*******************************************************************************
 * Initialization & De-initialization
 ******************************************************************************/

/*! @endcond */

/*!
 * @brief Populate a configuration structure with lpTMR driver defaults.
 */
void lpTMR_DRV_InitConfigStruct(lptmr_config_t * const config)
{
    DEV_ASSERT(config != NULL);

    /* Initialize the general configuration fields. */
#if defined(lpTMR_DIE_DMAEN_MASK)
    config->dmaRequest      = false;
#endif /* lpTMR_DIE_DMAEN_MASK */
    config->interruptEnable = false;
    config->freeRun         = false;
    config->workMode        = lpTMR_WORKMODE_TIMER;

    /* Initialize the timer and compare configuration fields. */
    config->prescaler       = lpTMR_PRESCALE_2;
    config->bypassPrescaler = false;
    config->compareValue    = 0u;
    config->counterUnits    = lpTMR_COUNTER_UNITS_TICKS;

    /* Initialize the pulse-counter specific fields. */
    config->pinSelect       = lpTMR_PINSELECT_TMU;
    config->pinPolarity     = lpTMR_PINPOLARITY_RISING;
#if (defined(FEATURE_lpTMR_HAS_CLOCK_SELECTION) && FEATURE_lpTMR_HAS_CLOCK_SELECTION)
    config->clockSource     = (lptmr_clock_source_t) 1U;
#endif /* FEATURE_lpTMR_HAS_CLOCK_SELECTION */
}

/*!
 * @brief Initialize an lpTMR instance from a user configuration.
 */
void lpTMR_DRV_Init(const uint32_t instance,
                    const lptmr_config_t * const config,
                    const bool startCounter)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    lpTMR_Type* const base = g_lptmrBase[instance];

    lpTMR_DRV_SetConfig(instance, config);

    /* Start the counter immediately when requested by the caller. */
    if (startCounter)
    {
        lpTMR_Enable(base);
    }
}

/*!
 * @brief Stop an lpTMR instance and restore reset-state register values.
 */
void lpTMR_DRV_Deinit(const uint32_t instance)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);

    lpTMR_Type* const base = g_lptmrBase[instance];
    lpTMR_Disable(base);

    lpTMR_Init(base);
}

/*******************************************************************************
 * Configuration
 ******************************************************************************/

/*!
 * @brief Apply a new configuration to an lpTMR instance.
 */
void lpTMR_DRV_SetConfig(const uint32_t instance,
                         const lptmr_config_t * const config)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    lpTMR_Type* const base          = g_lptmrBase[instance];
    uint32_t configCmpValue         = config->compareValue;
    lptmr_workmode_t configWorkMode = config->workMode;
    uint16_t cmpValueTicks          = 0U;
    lptmr_prescaler_t prescVal      = config->prescaler;
    bool prescBypass                = config->bypassPrescaler;
    lptmr_counter_units_t configCounterUnits = config->counterUnits;
    uint32_t clkFreq;

    if(configWorkMode == lpTMR_WORKMODE_TIMER)
    {
#if (defined(FEATURE_lpTMR_HAS_CLOCK_SELECTION) && (FEATURE_lpTMR_HAS_CLOCK_SELECTION == 1U))
        /* Resolve the timer clock selected for this lpTMR configuration. */
        switch (config->clockSource)
        {
        #if defined(FEATURE_lpTMR_CLKSRC_SUPPORT_FIRC) && (FEATURE_lpTMR_CLKSRC_SUPPORT_FIRC == 1U)
        case lpTMR_CLOCK_SOURCE_FIRC:
            (void)CLOCK_SYS_GetFreq(IPC_FIRC_CLK, &clkFreq);
            break;
        #endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_FIRC */
        #if defined(FEATURE_lpTMR_CLKSRC_SUPPORT_IPC) && (FEATURE_lpTMR_CLKSRC_SUPPORT_IPC == 1U)
        case lpTMR_CLOCK_SOURCE_IPC:
            (void)CLOCK_SYS_GetFreq(s_lptmrClkNames[instance], &clkFreq);
            break;
        #endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_IPC */
        #if defined(FEATURE_lpTMR_CLKSRC_SUPPORT_SIRC) && (FEATURE_lpTMR_CLKSRC_SUPPORT_SIRC == 1U)
        case lpTMR_CLOCK_SOURCE_SIRC:
            (void)CLOCK_SYS_GetFreq(IPC_SIRC_CLK, &clkFreq);
            break;
        #endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_SIRC */
        #if defined(FEATURE_lpTMR_CLKSRC_SUPPORT_SIRC_DIV4) && (FEATURE_lpTMR_CLKSRC_SUPPORT_SIRC_DIV4 == 1U)
                case lpTMR_CLOCK_SOURCE_SIRC_DIV4:
            (void)CLOCK_SYS_GetFreq(IPC_SIRC_DIV4_CLK, &clkFreq);
            break;
        #endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_SIRC */
        #if defined(FEATURE_lpTMR_CLKSRC_SUPPORT_SXOSC) && (FEATURE_lpTMR_CLKSRC_SUPPORT_SXOSC == 1U)
        case lpTMR_CLOCK_SOURCE_SXOSC:
            (void)CLOCK_SYS_GetFreq(IPC_SXOSC_CLK, &clkFreq);
            break;
        #endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_SXOSC */
        #if defined(FEATURE_lpTMR_CLKSRC_SUPPORT_FXOSC) && (FEATURE_lpTMR_CLKSRC_SUPPORT_FXOSC == 1U)
        case lpTMR_CLOCK_SOURCE_FXOSC:
            (void)CLOCK_SYS_GetFreq(IPC_FXOSC_CLK, &clkFreq);
            break;
        #endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_FXOSC */
        #if defined(FEATURE_lpTMR_CLKSRC_SUPPORT_OSC) && (FEATURE_lpTMR_CLKSRC_SUPPORT_OSC == 1U)
        case lpTMR_CLOCK_SOURCE_OSC:
            (void)CLOCK_SYS_GetFreq(IPC_OSC_CLK, &clkFreq);
            break;
        #endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_LPO */
        #if defined(FEATURE_lpTMR_CLKSRC_SUPPORT_LPO) && (FEATURE_lpTMR_CLKSRC_SUPPORT_LPO == 1U)
        case lpTMR_CLOCK_SOURCE_LPO:
            (void)CLOCK_SYS_GetFreq(IPC_LPO_CLK, &clkFreq);
            break;
        #endif /* FEATURE_lpTMR_CLKSRC_SUPPORT_LPO */
        default:
            clkFreq = 0U;
            break;
        }
#elif (FEATURE_lpTMR_CLKSRC_SUPPORT_IPC)
        /* Resolve the instance clock from the clock manager. */
        (void)CLOCK_SYS_GetFreq(s_lptmrClkNames[instance], &clkFreq);
#endif /* FEATURE_lpTMR_HAS_CLOCK_SELECTION */

        DEV_ASSERT(clkFreq != 0U); /* A zero frequency indicates an invalid clock selection. */
        s_lptmrClkFreq = clkFreq;

        if(configCounterUnits == lpTMR_COUNTER_UNITS_MICROSECONDS)
        {
            bool chooseClkConfigStatus;

            /* Derive the best-fit prescaler and compare value for the requested time period. */
            chooseClkConfigStatus = lptmr_ChooseClkConfig(clkFreq, configCmpValue, &prescVal, &prescBypass, &cmpValueTicks);
            DEV_ASSERT(chooseClkConfigStatus == true);
            (void) chooseClkConfigStatus;
        }
        else
        {
            DEV_ASSERT(configCounterUnits == lpTMR_COUNTER_UNITS_TICKS);
            DEV_ASSERT(configCmpValue <= lpTMR_CMP_CMP_MASK); /* Tick-based compare values must fit in the register. */

            cmpValueTicks = (uint16_t)(configCmpValue & lpTMR_CMP_CMP_MASK);
        }
    }
    else
    {
        /* Any non-timer configuration must be Pulse-Counter Mode. */
        DEV_ASSERT(configWorkMode == lpTMR_WORKMODE_PULSECOUNTER);

        /* Pulse counting always uses raw tick units for the compare register. */
        DEV_ASSERT(config->counterUnits == lpTMR_COUNTER_UNITS_TICKS);
        /* The glitch filter path does not support the divide-by-2 setting. */
        DEV_ASSERT(prescBypass || (prescVal != lpTMR_PRESCALE_2));

        DEV_ASSERT(configCmpValue <= lpTMR_CMP_CMP_MASK); /* Tick-based compare values must fit in the register. */

        cmpValueTicks = (uint16_t)(configCmpValue & lpTMR_CMP_CMP_MASK);
    }

    /* Reset the peripheral first, then program the requested configuration. */
    lpTMR_Init(base);

#if defined(lpTMR_DIE_DMAEN_MASK)
    lpTMR_SetDmaRequest   (base, config->dmaRequest);
#endif /* lpTMR_DIE_DMAEN_MASK */
    lpTMR_SetInterrupt    (base, config->interruptEnable);
    lpTMR_SetFreeRunning  (base, config->freeRun);
    lpTMR_SetWorkMode     (base, configWorkMode);
    lpTMR_SetPrescaler    (base, prescVal);
    lpTMR_SetBypass       (base, prescBypass);
    lpTMR_SetCompareValue (base, cmpValueTicks);
    lpTMR_SetPinSelect    (base, config->pinSelect);
    lpTMR_SetPinPolarity  (base, config->pinPolarity);
#if defined(FEATURE_lpTMR_HAS_CLOCK_SELECTION) && (FEATURE_lpTMR_HAS_CLOCK_SELECTION)
    lpTMR_SetClockSource(base, config->clockSource);
#endif /* FEATURE_lpTMR_HAS_CLOCK_SELECTION */
}

/*!
 * @brief Read back the active configuration of an lpTMR instance.
 */
void lpTMR_DRV_GetConfig(const uint32_t instance,
                         lptmr_config_t * const config)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);
    DEV_ASSERT(config != NULL);

    const lpTMR_Type* const base = g_lptmrBase[instance];

    /* Read the fields that are directly available in the hardware registers. */
#if defined(lpTMR_DIE_DMAEN_MASK)
    config->dmaRequest      = lpTMR_GetDmaRequest(base);
#endif /* lpTMR_DIE_DMAEN_MASK */
    config->interruptEnable = lpTMR_GetInterruptEnable(base);
    config->freeRun         = lpTMR_GetFreeRunning(base);
    config->workMode        = lpTMR_GetWorkMode(base);
    config->prescaler       = lpTMR_GetPrescaler(base);
    config->bypassPrescaler = lpTMR_GetBypass(base);
    config->compareValue    = lpTMR_GetCompareValue(base);
    config->counterUnits    = lpTMR_COUNTER_UNITS_TICKS;
    config->pinSelect       = lpTMR_GetPinSelect(base);
    config->pinPolarity     = lpTMR_GetPinPolarity(base);
}

/*******************************************************************************
 * Compare Value Access
 ******************************************************************************/

/*!
 * @brief Program the compare value directly in counter ticks.
 */
status_t lpTMR_DRV_SetCompareValueByCount(const uint32_t instance,
                                          const uint16_t compareValueByCount)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);

    lpTMR_Type* const base  = g_lptmrBase[instance];
    status_t statusCode     = STATUS_SUCCESS;

    bool timerEnabled = lpTMR_GetEnable(base);
    bool compareFlag  = lpTMR_GetCompareFlag(base);

    uint16_t counterVal;

    /* Preserve the debug-only reads used by the original implementation path. */
#if (defined (DEV_ERROR_DETECT) || defined (CUSTOM_DEVASSERT))
    bool bypass = lpTMR_GetBypass(base);
    lptmr_workmode_t workMode = lpTMR_GetWorkMode(base);
    (void) bypass;
    (void) workMode;
#endif /* (defined (DEV_ERROR_DETECT) || defined (CUSTOM_DEVASSERT)) */


    /* Writes are allowed only while stopped or after a compare event has occurred. */
    if (timerEnabled && !compareFlag)
    {
        statusCode = STATUS_ERROR;
    }
    else
    {
        /* Reject compare values that are already behind the running counter state. */
        lpTMR_SetCompareValue(base, compareValueByCount);
        counterVal = lpTMR_GetCounterValue(base);
        if (counterVal >= compareValueByCount)
        {
            statusCode = STATUS_TIMEOUT;
        }
    }

    return statusCode;
}


/*!
 * @brief Read the compare value in counter ticks.
 */
void lpTMR_DRV_GetCompareValueByCount(const uint32_t instance,
                                      uint16_t * const compareValueByCount)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);

    const lpTMR_Type* const base = g_lptmrBase[instance];

    *compareValueByCount = lpTMR_GetCompareValue(base);
}


/*!
 * @brief Program the compare value in microseconds for Timer Mode.
 */
status_t lpTMR_DRV_SetCompareValueByUs(const uint32_t instance,
                                       const uint32_t compareValueUs)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);
    DEV_ASSERT(s_lptmrClkFreq != 0U); /* A zero cached frequency indicates invalid timer-clock state. */

    status_t returnCode     = STATUS_SUCCESS;
    lpTMR_Type* const base  = g_lptmrBase[instance];
    bool timerEnabled, compareFlag;
    uint16_t cmpValTicks, currentCounterVal;
    lptmr_prescaler_t prescVal;
    bool prescBypass;

    /* This API is defined only for Timer Mode configurations. */
    DEV_ASSERT(lpTMR_GetWorkMode(base) == lpTMR_WORKMODE_TIMER);

    timerEnabled = lpTMR_GetEnable(base);
    compareFlag  = lpTMR_GetCompareFlag(base);
    /* Writes are allowed only while stopped or after a compare event has occurred. */
    if (timerEnabled && !compareFlag)
    {
        returnCode = STATUS_ERROR;
    }
    else
    {
        bool chooseClkConfigStatus;

        /* Derive the best-fit prescaler and compare value for the requested period. */
        chooseClkConfigStatus = lptmr_ChooseClkConfig(s_lptmrClkFreq, compareValueUs, &prescVal, &prescBypass, &cmpValTicks);
        DEV_ASSERT(chooseClkConfigStatus == true);
        (void) chooseClkConfigStatus;

        /* Apply the new compare setting and verify it is still ahead of the counter. */
        lpTMR_SetCompareValue(base, cmpValTicks);
        lpTMR_SetPrescaler(base, prescVal);
        lpTMR_SetBypass(base, prescBypass);
        currentCounterVal = lpTMR_GetCounterValue(base);

        if (currentCounterVal >= cmpValTicks)
        {
            returnCode = STATUS_TIMEOUT;
        }
    }

    return returnCode;
}

/*!
 * @brief Read the active compare value as microseconds.
 */
void lpTMR_DRV_GetCompareValueByUs(const uint32_t instance,
                                   uint32_t * const compareValueUs)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);
    DEV_ASSERT(compareValueUs != NULL);
    DEV_ASSERT(s_lptmrClkFreq != 0U);

    const lpTMR_Type* const base = g_lptmrBase[instance];
    uint16_t cmpValTicks;
    lptmr_prescaler_t prescVal;
    bool prescBypass, conversionStatus;

    /* This API is defined only for Timer Mode configurations. */
    DEV_ASSERT(lpTMR_GetWorkMode(base) == lpTMR_WORKMODE_TIMER);

    /* Read back the timer scaling fields required for unit conversion. */
    prescVal    = lpTMR_GetPrescaler(base);
    prescBypass = lpTMR_GetBypass(base);
    cmpValTicks = lpTMR_GetCompareValue(base);

    /* Convert the current compare register setting back to microseconds. */
    conversionStatus = lptmr_Ticks2Us(s_lptmrClkFreq, prescVal, prescBypass, cmpValTicks, compareValueUs);
    DEV_ASSERT(conversionStatus == true); /* The conversion result must fit in 32 bits. */
    (void) conversionStatus;
}


/*******************************************************************************
 * Runtime Status & Control
 ******************************************************************************/

/*!
 * @brief Read the compare-match flag state.
 */
bool lpTMR_DRV_GetCompareFlag(const uint32_t instance)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);

    const lpTMR_Type* const base = g_lptmrBase[instance];
    bool compareFlag = lpTMR_GetCompareFlag(base);

    return compareFlag;
}


/*!
 * @brief Clear the compare-match flag.
 */
void lpTMR_DRV_ClearCompareFlag(const uint32_t instance)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);

    lpTMR_Type* const base = g_lptmrBase[instance];

    lpTMR_ClearCompareFlag(base);
}


/*!
 * @brief Report whether the counter is currently enabled.
 */
bool lpTMR_DRV_IsRunning(const uint32_t instance)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);

    const lpTMR_Type* const base = g_lptmrBase[instance];

    bool runningState = lpTMR_GetEnable(base);

    return runningState;
}


/*!
 * @brief Enable or disable compare interrupts.
 */
void lpTMR_DRV_SetInterrupt(const uint32_t instance,
                            const bool enableInterrupt)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);

    lpTMR_Type* const base = g_lptmrBase[instance];

    lpTMR_SetInterrupt(base, enableInterrupt);
}


/*!
 * @brief Read the current counter value in raw ticks.
 */
uint16_t lpTMR_DRV_GetCounterValueByCount(const uint32_t instance)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);

    lpTMR_Type* const base = g_lptmrBase[instance];

    uint16_t counterVal = lpTMR_GetCounterValue(base);

    return counterVal;
}


/*!
 * @brief Enable the lpTMR counter.
 */
void lpTMR_DRV_StartCounter(const uint32_t instance)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);

    lpTMR_Type* const base = g_lptmrBase[instance];

    /* Preserve the debug-only reads used by the original implementation path. */
#if (defined (DEV_ERROR_DETECT) || defined (CUSTOM_DEVASSERT))
    bool bypass = lpTMR_GetBypass(base);
    lptmr_workmode_t workMode = lpTMR_GetWorkMode(base);
    (void) bypass;
    (void) workMode;
#endif /* (defined (DEV_ERROR_DETECT) || defined (CUSTOM_DEVASSERT)) */

    lpTMR_Enable(base);
}


/*!
 * @brief Disable the lpTMR counter.
 */
void lpTMR_DRV_StopCounter(const uint32_t instance)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);

    lpTMR_Type* const base = g_lptmrBase[instance];

    lpTMR_Disable(base);
}


/*******************************************************************************
 * Pulse Counter Pin Configuration
 ******************************************************************************/

/*!
 * @brief Update the pulse input source and active edge.
 */
void lpTMR_DRV_SetPinConfiguration(const uint32_t instance,
                                   const lptmr_pinselect_t pinSelect,
                                   const lptmr_pinpolarity_t pinPolarity)
{
    DEV_ASSERT(instance < lpTMR_INSTANCE_COUNT);

    lpTMR_Type* const base = g_lptmrBase[instance];

    lpTMR_SetPinSelect(base, pinSelect);
    lpTMR_SetPinPolarity(base, pinPolarity);
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
