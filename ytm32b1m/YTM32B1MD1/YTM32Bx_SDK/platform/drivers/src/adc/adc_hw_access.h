/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file adc_hw_access.h
 * @version 1.4.1
 *
 * @brief ADC Hardware Access Layer.
 *
 * This header provides low-level inline helpers for the ADC registers. The
 * APIs are organized into functional groups for converter timing and control,
 * trigger / DMA programming, watchdog setup, sequence programming, interrupt
 * and flag handling, and FIFO runtime control.
 *
 * @note This is an internal helper layer used by `adc_driver.h` and
 *       `adc_driver.c`. Application code should prefer the `ADC_DRV_*` APIs.
 */

#ifndef ADC_HW_ACCESS_H
#define ADC_HW_ACCESS_H

#include <stdint.h>
#include <stdbool.h>
#include "device_registers.h"
#include "adc_driver.h"

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @defgroup adc_hw_access ADC Hardware Access
 * @ingroup adc
 * @brief Low-level register access helpers for the ADC peripheral.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ADC_WAIT_TIMEOUT (1000U)

#if defined (__cplusplus)
extern "C" {
#endif

/*!
 * @name Converter Control & Timing
 * @brief Functions for reading converter state and programming timing-related
 *        fields.
 */
/*! @{*/

/*!
 * @brief Get the conversion-active flag state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if a conversion is currently active; `false` otherwise.
 */
static inline bool ADC_GetConvActiveFlag(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t) baseAddr->CTRL;
    tmp = (tmp & ADC_CTRL_ADSTART_MASK) >> ADC_CTRL_ADSTART_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Get the ADC-enable flag state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if the ADC is enabled; `false` otherwise.
 */
static inline bool ADC_GetEnableFlag(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t) baseAddr->CTRL;
    tmp = (tmp & ADC_CTRL_ADEN_MASK) >> ADC_CTRL_ADEN_SHIFT;
    return (tmp != 0u) ? true : false;
}


/*!
 * @brief Get the sequence-done flag state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if the programmed sequence has completed; `false` otherwise.
 */
static inline bool ADC_GetSequenceDoneFlag(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t) baseAddr->STS;
    tmp = (tmp & ADC_STS_EOSEQ_MASK) >> ADC_STS_EOSEQ_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Read the ADC input clock divider setting.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return Encoded divider value stored in the hardware field.
 */
static inline adc_clk_divide_t ADC_GetClockDivide(const ADC_Type *const baseAddr)
{
    uint32_t tmp = baseAddr->CFG1;
    tmp = (tmp & ADC_CFG1_PRS_MASK) >> ADC_CFG1_PRS_SHIFT;

    /* The register field uses the same encoding as the public divider type. */
    return (adc_clk_divide_t) (tmp);
}

/*!
 * @brief Program the ADC input clock divider field.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] clockDivide Encoded divider value to write.
 */
static inline void ADC_SetClockDivide(ADC_Type *const baseAddr,
                                      const adc_clk_divide_t clockDivide)
{
    uint32_t tmp = baseAddr->CFG1;
    tmp &= ~(ADC_CFG1_PRS_MASK);
    tmp |= ADC_CFG1_PRS(clockDivide);
    baseAddr->CFG1 = tmp;
}

/*!
 * @brief Read the ADC startup delay field.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return Startup delay value encoded in ADC clock cycles.
 */
static inline uint8_t ADC_GetStartTime(const ADC_Type *const baseAddr)
{
    uint32_t tmp = baseAddr->CFG1;
    tmp = (tmp & ADC_CFG1_STCNT_MASK) >> ADC_CFG1_STCNT_SHIFT;
    return (uint8_t)(tmp);
}

/*!
 * @brief Program the ADC startup delay field.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] starttime Startup delay value in ADC clock cycles.
 */
static inline void ADC_SetStartTime(ADC_Type *const baseAddr,
                                    uint8_t starttime)
{
    /* Write the startup-delay field directly. */
    uint32_t tmp = baseAddr->CFG1;
    tmp &= ~(ADC_CFG1_STCNT_MASK);
    tmp |= ADC_CFG1_STCNT(starttime);
    baseAddr->CFG1 = tmp;
}

/*!
 * @brief Read the primary sample-time field.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return Encoded sample-time value in ADC clock cycles.
 */
static inline uint8_t ADC_GetSampleTime(const ADC_Type *const baseAddr)
{
    uint32_t tmp = baseAddr->SMP;
    tmp = (tmp & ADC_SMP_SMP_MASK) >> ADC_SMP_SMP_SHIFT;
    return (uint8_t)(tmp);
}

/*!
 * @brief Program the primary sample-time field.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] sampletime Sample-time value in ADC clock cycles.
 *
 * @note A value of `0` is clamped to `1` because the hardware does not
 *       support an effective sample time of one ADC clock.
 */
static inline void ADC_SetSampleTime(ADC_Type *const baseAddr,
                                     uint8_t sampletime)
{
    /* Clamp the programmed sample time to the minimum supported value. */
    uint8_t rsampletime = (uint8_t) ((sampletime > 0U) ? sampletime : 1U);
    uint32_t tmp = baseAddr->SMP;
    tmp &= ~(ADC_SMP_SMP_MASK);
    tmp |= ADC_SMP_SMP(rsampletime);
    baseAddr->SMP = tmp;
}

/*!
 * @brief Get the overrun-overwrite mode state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if new data overwrites existing FIFO contents on overrun.
 */
static inline bool ADC_GetOverrunModeFlag(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t) baseAddr->CFG0;
    tmp = (tmp & ADC_CFG0_OVRMD_MASK) >> ADC_CFG0_OVRMD_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Enable or disable overwrite-on-overrun behavior.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] state `true` to overwrite on overrun; `false` to preserve existing data.
 */
static inline void ADC_SetOverrunModeFlag(ADC_Type *const baseAddr,
                                          const bool state)
{
    uint32_t tmp = (uint32_t) baseAddr->CFG0;
    tmp &= (uint32_t) (~(ADC_CFG0_OVRMD_MASK));
    tmp |= ADC_CFG0_OVRMD(state ? (uint32_t) 1u : (uint32_t) 0u);
    baseAddr->CFG0 = (uint32_t) tmp;
}

/*!
 * @brief Get the auto-off enable state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if auto-off mode is enabled; `false` otherwise.
 */
static inline bool ADC_GetautoOffEnableFlag(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t) baseAddr->CFG0;
    tmp = (tmp & ADC_CFG0_AUTOOFF_MASK) >> ADC_CFG0_AUTOOFF_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Enable or disable auto-off mode.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] state `true` to enable auto-off mode; `false` otherwise.
 */
static inline void ADC_SetautoOffEnableFlag(ADC_Type *const baseAddr,
                                            const bool state)
{
    uint32_t tmp = (uint32_t) baseAddr->CFG0;
    tmp &= (uint32_t) (~(ADC_CFG0_AUTOOFF_MASK));
    tmp |= ADC_CFG0_AUTOOFF(state ? (uint32_t) 1u : (uint32_t) 0u);
    baseAddr->CFG0 = (uint32_t) tmp;
}


/*!
 * @brief Get the FIFO-wait enable state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if conversion waits when the FIFO is full; `false` otherwise.
 */
static inline bool ADC_GetWaitEnableFlag(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t) baseAddr->CFG0;
    tmp = (tmp & ADC_CFG0_WAIT_MASK) >> ADC_CFG0_WAIT_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Enable or disable FIFO wait behavior.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] state `true` to stall when the FIFO is full; `false` otherwise.
 */
static inline void ADC_SetWaitEnableFlag(ADC_Type *const baseAddr,
                                         const bool state)
{
    uint32_t tmp = (uint32_t) baseAddr->CFG0;
    tmp &= (uint32_t) (~(ADC_CFG0_WAIT_MASK));
    tmp |= ADC_CFG0_WAIT(state ? (uint32_t) 1u : (uint32_t) 0u);
    baseAddr->CFG0 = (uint32_t) tmp;
}

/*!
 * @brief Control the low-power-keep setting when supported.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] state `true` to keep the ADC active in low-power mode; `false`
 *                  to disable that behavior.
 */
static inline void ADC_SetLowPowerEnableFlag(ADC_Type *const baseAddr,
                                             const bool state)
{
#if FEATURE_ADC_SUPPORT_LOW_POWER_KEEP
    uint32_t tmp = (uint32_t) baseAddr->CFG0;
    tmp &= (uint32_t) (~(ADC_CFG0_LPEN_MASK));
    tmp |= ADC_CFG0_LPEN(state ? (uint32_t) 1u : (uint32_t) 0u);
    baseAddr->CFG0 = (uint32_t) tmp;
#else
    (void) baseAddr;
    (void) state;
#endif /* FEATURE_ADC_SUPPORT_LOW_POWER_KEEP */
}

/*!
 * @brief Read the hardware trigger source selector.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return Raw trigger-source selector encoded in the register field.
 */
static inline adc_trigger_t ADC_GetTriggerSource(const ADC_Type *const baseAddr)
{
#if defined(CPU_YTM32B1LD0)
    uint32_t tmp = baseAddr->CFG0;
    tmp = (tmp & ADC_CFG0_TRIGSRC_MASK) >> ADC_CFG0_TRIGSRC_SHIFT;

    /* The register field is returned as the raw trigger-source selector. */
    return (adc_trigger_t)(tmp);
#else
    uint32_t tmp = baseAddr->CFG0;
    tmp = 0;

    /* Unsupported devices report the selector as zero. */
    return (adc_trigger_t) (tmp);
#endif
}

/*!
 * @brief Program the hardware trigger source selector.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] trigsrc Raw trigger-source selector value to write.
 */
static inline void ADC_SetTriggerSource(ADC_Type *const baseAddr,
                                        uint8_t trigsrc)
{
#if defined(CPU_YTM32B1LD0)
    uint32_t tmp = baseAddr->CFG0;
    tmp &= ~(ADC_CFG0_TRIGSRC_MASK);
    tmp |= ADC_CFG0_TRIGSRC(trigsrc);
    baseAddr->CFG0 = tmp;
#else
    uint32_t tmp = baseAddr->CFG0;
    (void) trigsrc;
    (void) tmp;
#endif
}


/*!
 * @brief Read the trigger-mode field.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return Current normal/injected trigger mode setting.
 */

static inline adc_trigger_t ADC_GetTriggerMode(const ADC_Type *const baseAddr)
{
    uint32_t tmp = baseAddr->CFG0;
#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
    tmp = (tmp & (ADC_CFG0_NORTMD_MASK | ADC_CFG0_INJTMD_MASK)) >> ADC_CFG0_NORTMD_SHIFT;
#else
#if defined (ADC_CFG0_NORTMD_MASK)
    tmp = (tmp & ADC_CFG0_NORTMD_MASK) >> ADC_CFG0_NORTMD_SHIFT;
#else
    tmp = (tmp & ADC_CFG0_TRIGMD_MASK) >> ADC_CFG0_TRIGMD_SHIFT;
#endif /* defined (ADC_CFG0_NORTMD_MASK) */
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */

    /* The trigger-mode bitfield maps directly to the public trigger type. */
    return (adc_trigger_t) (tmp);
}


/*!
 * @brief Program the trigger-mode field.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] trigger Trigger mode value to write.
 */
static inline void ADC_SetTriggerMode(ADC_Type *const baseAddr,
                                      const adc_trigger_t trigger)
{
    uint32_t tmp = baseAddr->CFG0;
#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
    tmp &= ~(ADC_CFG0_NORTMD_MASK | ADC_CFG0_INJTMD_MASK);
    tmp |= ((uint32_t)(trigger) << ADC_CFG0_NORTMD_SHIFT) & (ADC_CFG0_NORTMD_MASK | ADC_CFG0_INJTMD_MASK);
#else
#if defined (ADC_CFG0_NORTMD_MASK)
    tmp &= ~ADC_CFG0_NORTMD_MASK;
    tmp |= ((uint32_t)(trigger) << ADC_CFG0_NORTMD_SHIFT) & ADC_CFG0_NORTMD_MASK;
#else
    tmp &= ~(ADC_CFG0_TRIGMD_MASK);
    tmp |= ADC_CFG0_TRIGMD((uint32_t)trigger);
#endif /* defined (ADC_CFG0_NORTMD_MASK) */
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */
    baseAddr->CFG0 = tmp;
}

/*!
 * @brief Read the conversion-resolution setting.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return Current ADC resolution setting.
 */
static inline adc_resolution_t ADC_GetResolution(const ADC_Type *const baseAddr)
{
    uint32_t tmp = baseAddr->CFG0;
    tmp = (tmp & ADC_CFG0_RES_MASK) >> ADC_CFG0_RES_SHIFT;
    return (adc_resolution_t) tmp;
}

/*!
 * @brief Program the conversion-resolution setting.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] resolution Resolution mode value to write.
 */
static inline void ADC_SetResolution(ADC_Type *const baseAddr,
                                     const adc_resolution_t resolution)
{
    uint32_t tmp = baseAddr->CFG0;
    tmp &= ~(ADC_CFG0_RES_MASK);
    tmp |= ADC_CFG0_RES(resolution);
    baseAddr->CFG0 = tmp;
}

/*!
 * @brief Read the DMA FIFO watermark setting.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return Encoded FIFO watermark value.
 */
static inline uint8_t ADC_GetDMAWatermark(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t) baseAddr->CFG0;
    tmp = (tmp & ADC_CFG0_WM_MASK) >> ADC_CFG0_WM_SHIFT;
    tmp = ADC_CFG0_WM(tmp);
    return (uint8_t)tmp;
}

/*!
 * @brief Program the DMA FIFO watermark setting.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] value Encoded FIFO watermark value to write.
 */
static inline void ADC_SetDMAWatermark(ADC_Type *const baseAddr,
                                       const uint8_t value)
{
    uint32_t tmp = (uint32_t) baseAddr->CFG0;
    tmp &= (uint32_t) (~(ADC_CFG0_WM_MASK));
    tmp |= ADC_CFG0_WM(value);
    baseAddr->CFG0 = (uint32_t) tmp;
}

/*!
 * @brief Get the DMA-request enable state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if DMA requests are enabled; `false` otherwise.
 */
static inline bool ADC_GetDMAEnableFlag(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t) baseAddr->CFG0;
    tmp = (tmp & ADC_CFG0_DMAEN_MASK) >> ADC_CFG0_DMAEN_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Enable or disable DMA requests.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] state `true` to enable DMA requests; `false` otherwise.
 */
static inline void ADC_SetDMAEnableFlag(ADC_Type *const baseAddr,
                                        const bool state)
{
    uint32_t tmp = (uint32_t) baseAddr->CFG0;
    tmp &= (uint32_t) (~(ADC_CFG0_DMAEN_MASK));
    tmp |= ADC_CFG0_DMAEN(state ? (uint32_t) 1u : (uint32_t) 0u);
    baseAddr->CFG0 = (uint32_t) tmp;
}

/*! @}*/

/*!
 * @name Hardware Compare Control
 * @brief Functions for programming the analog watchdog / hardware compare path.
 */
/*! @{*/

/*!
 * @brief Get the watchdog enable state for one watchdog channel.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] wdgChannel Watchdog channel index.
 * @return `true` if the watchdog channel is enabled; `false` otherwise.
 */
static inline bool ADC_GetHwCompareEnableFlag(const ADC_Type *const baseAddr, const uint8_t wdgChannel)
{
    uint32_t tmp = (uint32_t) baseAddr->WDCTRL;
    /* Enable bits are packed at bit positions 7, 15, 23, and 31. */
    uint8_t channelEnableBitShift = (uint8_t) (((wdgChannel + 1u) << 3U) - 1u);
    tmp = (tmp >> channelEnableBitShift) & 0x01U;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Enable or disable one watchdog channel.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] state `true` to enable the watchdog channel; `false` otherwise.
 * @param[in] wdgChannel Watchdog channel index.
 */
static inline void ADC_SetHwCompareEnableFlag(ADC_Type *const baseAddr,
                                              const bool state, const uint8_t wdgChannel)
{
    uint32_t tmp = (uint32_t) baseAddr->WDCTRL;
    /* Enable bits are packed at bit positions 7, 15, 23, and 31. */
    uint8_t channelEnableBitShift = (uint8_t) ((wdgChannel << 3U) + 7u);
    tmp &= (~(((uint32_t)1U << channelEnableBitShift)));
    tmp |= (state ? (uint32_t) 1u : (uint32_t) 0u) << channelEnableBitShift;
    baseAddr->WDCTRL = (uint32_t) tmp;
}

/*!
 * @brief Get the all-channel watchdog mode state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if the watchdog monitors all channels; `false` if it monitors
 *         only the selected channel.
 */
static inline bool ADC_GetHwCompareAllEnableFlag(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t) baseAddr->WDCTRL;
#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
    tmp = (tmp & ADC_WDCTRL_WD0SGL_MASK) >> ADC_WDCTRL_WD0SGL_SHIFT;
#else
    tmp = (tmp & ADC_WDCTRL_WDSGL_MASK) >> ADC_WDCTRL_WDSGL_SHIFT;
#endif /* (FEATURE_ADC_WDG_CHANNEL_COUNT > 1) */
    return (tmp != 0u) ? false : true;
}

/*!
 * @brief Enable or disable all-channel watchdog monitoring.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] state `true` to monitor all channels; `false` to monitor the
 *                  selected channel only.
 */
static inline void ADC_SetHwCompareAllEnableFlag(ADC_Type *const baseAddr,
                                                 const bool state)
{
    uint32_t tmp = (uint32_t) baseAddr->WDCTRL;
#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
    tmp &= (uint32_t) (~(ADC_WDCTRL_WD0SGL_MASK));
    tmp |= ADC_WDCTRL_WD0SGL(state ? (uint32_t) 0u : (uint32_t) 1u);
#else
    tmp &= (uint32_t) (~(ADC_WDCTRL_WDSGL_MASK));
    tmp |= ADC_WDCTRL_WDSGL(state ? (uint32_t) 0u : (uint32_t) 1u);
#endif /* (FEATURE_ADC_WDG_CHANNEL_COUNT > 1) */
    baseAddr->WDCTRL = (uint32_t) tmp;
}

/*!
 * @brief Read the channel assigned to a watchdog comparator.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] wdgChannel Watchdog channel index.
 * @return Input channel selected for the watchdog comparator.
 */
static inline adc_inputchannel_t ADC_GetHwCompareChannel(const ADC_Type *const baseAddr, const uint8_t wdgChannel)
{
    uint32_t tmp = (uint32_t) baseAddr->WDCTRL;
    /* Channel-select fields are packed at bit positions 0, 8, 16, and 24. */
    uint8_t channelCmpValBitShift = (uint8_t) (wdgChannel << 3u);
    tmp = (tmp >> channelCmpValBitShift) & 0x3FU;
    return (adc_inputchannel_t)tmp;
}

/*!
 * @brief Program the channel monitored by a watchdog comparator.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] channel ADC input channel assigned to the watchdog comparator.
 * @param[in] wdgChannel Watchdog channel index.
 */
static inline void ADC_SetHwCompareRangeEnableFlag(ADC_Type *const baseAddr,
                                                   const adc_inputchannel_t channel,
                                                   const uint8_t wdgChannel)
{
    uint32_t tmp = baseAddr->WDCTRL;
    /* Channel-select fields are packed at bit positions 0, 8, 16, and 24. */
    uint8_t channelCmpValBitShift = (uint8_t) (wdgChannel << 3);
    tmp &= ~((uint32_t)0x3FU << channelCmpValBitShift);
    tmp |= (uint32_t)((uint32_t)channel & 0x3FU) << channelCmpValBitShift;
    baseAddr->WDCTRL = tmp;
}

/*!
 * @brief Read the upper watchdog threshold.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] wdgChannel Watchdog channel index.
 * @return Upper threshold value in 12-bit scaling.
 */
static inline uint16_t ADC_GetHwCompareCompHighValue(const ADC_Type *const baseAddr, const uint8_t wdgChannel)
{
#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
    uint32_t tmp = (uint32_t) baseAddr->WDTH[wdgChannel];
    tmp = (tmp & ADC_WDTH_HIGH_MASK) >> ADC_WDTH_HIGH_SHIFT;
#else
    (void)wdgChannel;
    uint32_t tmp = (uint32_t) baseAddr->WDTH;
    tmp = (tmp & ADC_WDTH_HIGH_MASK) >> ADC_WDTH_HIGH_SHIFT;
#endif /* (FEATURE_ADC_WDG_CHANNEL_COUNT > 1) */
    return (uint16_t) tmp;
}

/*!
 * @brief Program the upper watchdog threshold.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] value Upper threshold value in 12-bit scaling.
 * @param[in] wdgChannel Watchdog channel index.
 */
static inline void ADC_SetHwCompareCompHighValue(ADC_Type *const baseAddr,
                                                 const uint16_t value,
                                                 const uint8_t wdgChannel)
{
#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
    uint32_t tmp = (uint32_t) baseAddr->WDTH[wdgChannel];
    tmp &= ~ADC_WDTH_HIGH_MASK;
    tmp |= ADC_WDTH_HIGH(value);
    baseAddr->WDTH[wdgChannel] = tmp;
#else
    uint32_t tmp = (uint32_t) baseAddr->WDTH;
    tmp &= ~ADC_WDTH_HIGH_MASK;
    tmp |= ADC_WDTH_HIGH(value);
    baseAddr->WDTH = tmp;
    (void) wdgChannel;
#endif /* (FEATURE_ADC_WDG_CHANNEL_COUNT > 1) */
}

/*!
 * @brief Read the lower watchdog threshold.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] wdgChannel Watchdog channel index.
 * @return Lower threshold value in 12-bit scaling.
 */
static inline uint16_t ADC_GetHwCompareCompLowValue(const ADC_Type *const baseAddr, const uint8_t wdgChannel)
{
#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
    uint32_t tmp = (uint32_t) baseAddr->WDTH[wdgChannel];
    tmp = (tmp & ADC_WDTH_LOW_MASK) >> ADC_WDTH_LOW_SHIFT;
#else
    uint32_t tmp = (uint32_t) baseAddr->WDTH;
    tmp = (tmp & ADC_WDTH_LOW_MASK) >> ADC_WDTH_LOW_SHIFT;
    (void) wdgChannel;
#endif /* (FEATURE_ADC_WDG_CHANNEL_COUNT > 1) */
    return (uint16_t) tmp;

}

/*!
 * @brief Program the lower watchdog threshold.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] value Lower threshold value in 12-bit scaling.
 * @param[in] wdgChannel Watchdog channel index.
 */
static inline void ADC_SetHwCompareCompLowValue(ADC_Type *const baseAddr,
                                                const uint16_t value,
                                                const uint8_t wdgChannel)
{
#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
    uint32_t tmp = (uint32_t) baseAddr->WDTH[wdgChannel];
    tmp &= ~ADC_WDTH_LOW_MASK;
    tmp |= ADC_WDTH_LOW(value);
    baseAddr->WDTH[wdgChannel] = tmp;
#else
    uint32_t tmp = (uint32_t) baseAddr->WDTH;
    tmp &= ~ADC_WDTH_LOW_MASK;
    tmp |= ADC_WDTH_LOW(value);
    baseAddr->WDTH = tmp;
    (void) wdgChannel;
#endif /* (FEATURE_ADC_WDG_CHANNEL_COUNT > 1) */
}

/*! @}*/

/*!
 * @name Sequence Programming
 * @brief Functions for programming normal and injected conversion sequences.
 */
/*! @{*/

/*!
 * @brief Read the injected-sequence channel at one slot index.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] idx Sequence slot index.
 * @return Injected-sequence channel assigned to the selected slot.
 */
static inline adc_inputchannel_t ADC_GetInjectChannel(const ADC_Type *const baseAddr,
                                                       uint8_t idx)
{
#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
    uint8_t tmp = (uint8_t) baseAddr->INJCH[idx];
    tmp = (tmp & ADC_INJCH_INJCH_MASK) >> ADC_INJCH_INJCH_SHIFT;
    return (adc_inputchannel_t) tmp;
#else
    (void) baseAddr;
    (void) idx;
    return (adc_inputchannel_t) 0u;
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */
}

/*!
 * @brief Read the normal-sequence channel at one slot index.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] idx Sequence slot index.
 * @return Normal-sequence channel assigned to the selected slot.
 */
static inline adc_inputchannel_t ADC_GetSequenceChannel(const ADC_Type *const baseAddr,
                                                       uint8_t idx)
{
    uint8_t tmp = (uint8_t) baseAddr->CHSEL[idx];
    tmp = (tmp & ADC_CHSEL_CHSEL_MASK) >> ADC_CHSEL_CHSEL_SHIFT;
    return (adc_inputchannel_t) tmp;
}

/*!
 * @brief Program one injected-sequence channel slot.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] idx Sequence slot index.
 * @param[in] channel Channel assigned to the selected injected slot.
 */
static inline void ADC_SetInjectChannel(ADC_Type *const baseAddr,
                                         uint8_t idx,
                                         adc_inputchannel_t channel)
{
#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
    uint8_t tmp = (uint8_t) baseAddr->INJCH[idx];
    tmp &= ~ADC_INJCH_INJCH_MASK;
    tmp |= ADC_INJCH_INJCH(channel);
    baseAddr->INJCH[idx] = tmp;
#else
    (void) baseAddr;
    (void) idx;
    (void) channel;
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */
}

/*!
 * @brief Program one normal-sequence channel slot.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] idx Sequence slot index.
 * @param[in] channel Channel assigned to the selected normal-sequence slot.
 */
static inline void ADC_SetSequenceChannel(ADC_Type *const baseAddr,
                                         uint8_t idx,
                                         adc_inputchannel_t channel)
{
    uint8_t tmp = (uint8_t) baseAddr->CHSEL[idx];
    tmp &= ~ADC_CHSEL_CHSEL_MASK;
    tmp |= (uint8_t)ADC_CHSEL_CHSEL(channel);
    baseAddr->CHSEL[idx] = tmp;
}

/*!
 * @brief Read the injected-sequence channel count.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return Number of active injected-sequence entries.
 */
static inline uint8_t ADC_GetInjectTotalChannel(const ADC_Type *const baseAddr)
{
#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
    uint32_t tmp = (uint32_t) baseAddr->CFG0;
    tmp = (tmp & ADC_CFG0_INJSEQ_MASK) >> ADC_CFG0_INJSEQ_SHIFT;
    return (uint8_t) tmp + 1;
#else
    (void) baseAddr;
    return 0;
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */
}

/*!
 * @brief Read the normal-sequence channel count.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return Number of active normal-sequence entries.
 */
static inline uint8_t ADC_GetSequenceTotalChannel(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t) baseAddr->CFG0;
    tmp = (tmp & ADC_CFG0_SEQLEN_MASK) >> ADC_CFG0_SEQLEN_SHIFT;
    return (uint8_t) (tmp + 1u);
}

/*!
 * @brief Program the injected-sequence channel count.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] value Number of active injected-sequence entries.
 */
static inline void ADC_SetInjectTotalChannel(ADC_Type *const baseAddr, uint8_t value)
{
#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
    uint32_t tmp = (uint32_t) baseAddr->CFG0;
    tmp &= ~ADC_CFG0_INJSEQ_MASK;
    tmp |= ADC_CFG0_INJSEQ(value - 1);
    baseAddr->CFG0 = tmp;
#else
    (void) baseAddr;
    (void) value;
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */
}

/*!
 * @brief Program the normal-sequence channel count.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] value Number of active normal-sequence entries.
 */
static inline void ADC_SetSequenceTotalChannel(ADC_Type *const baseAddr, uint8_t value)
{
    uint32_t tmp = (uint32_t) baseAddr->CFG0;
    tmp &= ~ADC_CFG0_SEQLEN_MASK;
    tmp |= ADC_CFG0_SEQLEN((uint32_t)value - 1u);
    baseAddr->CFG0 = tmp;
}

/*!
 * @brief Read the normal-sequence execution mode.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return Sequence execution mode derived from the hardware control bits.
 */
static inline adc_sequence_mode_t ADC_GetSequenceMode(const ADC_Type *const baseAddr)
{
    adc_sequence_mode_t sequenceMode;
#ifdef CPU_YTM32B1ME0
    if ((ADC_CFG0_CONT_MASK & (baseAddr->CFG0)) != 0u)
    {
        sequenceMode = ADC_CONV_CONTINUOUS;
    } else if ((ADC_CFG0_DISCEN_MASK & (baseAddr->CFG0)) != 0u)
    {
        sequenceMode = ADC_CONV_STEP;
    } else
    {
        sequenceMode = ADC_CONV_LOOP;
    }
#else
#ifdef ADC_CFG0_MODE_MASK
    if ((((ADC_CFG0_MODE_MASK & (baseAddr->CFG0)) >> ADC_CFG0_MODE_SHIFT) & 0x01u) != 0u)
    {
        sequenceMode = ADC_CONV_CONTINUOUS;
    } else if ((((ADC_CFG0_MODE_MASK & (baseAddr->CFG0)) >> ADC_CFG0_MODE_SHIFT) & 0x02u) != 0u)
    {
        sequenceMode = ADC_CONV_STEP;
    } else
    {
        sequenceMode = ADC_CONV_LOOP;
    }
#else
    /* Legacy conversion-mode bit layout used by YTM32B1LE0x devices. */
    if ((((ADC_CFG0_CONVMD_MASK & (baseAddr->CFG0)) >> ADC_CFG0_CONVMD_SHIFT) & 0x01u) != 0u)
    {
        sequenceMode = ADC_CONV_CONTINUOUS;
    } else if ((((ADC_CFG0_CONVMD_MASK & (baseAddr->CFG0)) >> ADC_CFG0_CONVMD_SHIFT) & 0x02u) != 0u)
    {
        sequenceMode = ADC_CONV_STEP;
    } else
    {
        sequenceMode = ADC_CONV_LOOP;
    }
#endif /* ADC_CFG0_MODE_MASK */
#endif /* CPU_YTM32B1ME0 */

    return sequenceMode;
}

/*!
 * @brief Program the normal-sequence execution mode.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] sequenceMode Sequence execution mode to write.
 */
static inline void ADC_SetSequenceMode(ADC_Type *const baseAddr,
                                       adc_sequence_mode_t sequenceMode)
{
#ifdef CPU_YTM32B1ME0
    switch (sequenceMode)
    {
        case ADC_CONV_CONTINUOUS:
            baseAddr->CFG0 |= (ADC_CFG0_CONT_MASK);
            baseAddr->CFG0 &= ~(ADC_CFG0_DISCEN_MASK);
            break;
        case ADC_CONV_STEP:
            baseAddr->CFG0 &= ~(ADC_CFG0_CONT_MASK);
            baseAddr->CFG0 |= (ADC_CFG0_DISCEN_MASK);
            break;
        case ADC_CONV_LOOP:
            baseAddr->CFG0 &= ~(ADC_CFG0_CONT_MASK);
            baseAddr->CFG0 &= ~(ADC_CFG0_DISCEN_MASK);
            break;
        default:
            (void) baseAddr;
            break;
    }
#else
#ifdef ADC_CFG0_MODE_MASK
    switch (sequenceMode)
    {
        case ADC_CONV_CONTINUOUS:
            baseAddr->CFG0 |= (ADC_CFG0_MODE_MASK);
            baseAddr->CFG0 &= ~(ADC_CFG0_MODE_MASK << 1);
            break;
        case ADC_CONV_STEP:
            baseAddr->CFG0 &= ~(ADC_CFG0_MODE_MASK);
            baseAddr->CFG0 |= (ADC_CFG0_MODE_MASK << 1);
            break;
        case ADC_CONV_LOOP:
            baseAddr->CFG0 &= ~(ADC_CFG0_MODE_MASK);
            baseAddr->CFG0 &= ~(ADC_CFG0_MODE_MASK << 1);
            break;
        default:
            (void) baseAddr;
            break;
    }
#else
    /* Legacy conversion-mode bit layout used by YTM32B1LE0x devices. */
    switch (sequenceMode)
    {
        case ADC_CONV_CONTINUOUS:
            baseAddr->CFG0 |= (ADC_CFG0_CONVMD_MASK);
            baseAddr->CFG0 &= ~(ADC_CFG0_CONVMD_MASK << 1);
            break;
        case ADC_CONV_STEP:
            baseAddr->CFG0 &= ~(ADC_CFG0_CONVMD_MASK);
            baseAddr->CFG0 |= (ADC_CFG0_CONVMD_MASK << 1);
            break;
        case ADC_CONV_LOOP:
            baseAddr->CFG0 &= ~(ADC_CFG0_CONVMD_MASK);
            baseAddr->CFG0 &= ~(ADC_CFG0_CONVMD_MASK << 1);
            break;
        default:
            (void) baseAddr;
            break;
    }
#endif /* ADC_CFG0_MODE_MASK */
#endif /* CPU_YTM32B1ME0 */
}

/*! @}*/

/*!
 * @name Interrupt & Flag Management
 * @brief Functions for interrupt enables, sticky-flag control, and status
 *        servicing.
 * @{
 */

/*!
 * @brief Get the end-of-conversion interrupt-enable state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if end-of-conversion interrupts are enabled; `false` otherwise.
 */
static inline bool ADC_GetEocIntEnableFlag(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t)baseAddr->INTE;
    tmp = (tmp & ADC_INTE_EOCIE_MASK) >> ADC_INTE_EOCIE_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Enable or disable analog-watchdog interrupts.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] state `true` to enable watchdog interrupts; `false` otherwise.
 */
static inline void ADC_SetAwdIntEnableFlag(ADC_Type *const baseAddr,
                                           const bool state)
{
    uint32_t tmp = (uint32_t) baseAddr->INTE;
    tmp &= (uint32_t) (~(ADC_INTE_WDIE_MASK));
    tmp |= ADC_INTE_WDIE(state ? (uint32_t) 1u : (uint32_t) 0u);
    baseAddr->INTE = (uint32_t) tmp;
}

/*!
 * @brief Program the effective-mode bit for one watchdog channel.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] effective_mode `true` for inside-window matching; `false` for
 *                           outside-window matching.
 * @param[in] wdgChannel Watchdog channel index.
 */
static inline void ADC_SetAwdEffectiveMode(ADC_Type *const baseAddr,
                                           const bool effective_mode,
                                           const uint8_t wdgChannel)
{
#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
    uint32_t tmp = (uint32_t) baseAddr->WDTH[wdgChannel];
    tmp &= (uint32_t) (~(ADC_WDTH_THMD_MASK));
    tmp |= ADC_WDTH_THMD(effective_mode ? (uint32_t) 1u : (uint32_t) 0u);
    baseAddr->WDTH[wdgChannel] = (uint32_t) tmp;
#else
    (void) wdgChannel;
    uint32_t tmp = (uint32_t) baseAddr->WDTH;
    tmp &= (uint32_t) (~(ADC_WDTH_THMD_MASK));
    tmp |= ADC_WDTH_THMD(effective_mode ? (uint32_t) 1u : (uint32_t) 0u);
    baseAddr->WDTH = (uint32_t) tmp;
#endif /* (FEATURE_ADC_WDG_CHANNEL_COUNT > 1) */
}

/*!
 * @brief Get the analog-watchdog interrupt-enable state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if watchdog interrupts are enabled; `false` otherwise.
 */
static inline bool ADC_GetAwdIntEnableFlag(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t) baseAddr->INTE;
    tmp = (tmp & ADC_INTE_WDIE_MASK) >> ADC_INTE_WDIE_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Enable or disable FIFO-overrun interrupts.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] state `true` to enable overrun interrupts; `false` otherwise.
 */
static inline void ADC_SetOvrIntEnableFlag(ADC_Type *const baseAddr,
                                           const bool state)
{
    uint32_t tmp = (uint32_t) baseAddr->INTE;
    tmp &= (uint32_t) (~(ADC_INTE_OVRIE_MASK));
    tmp |= ADC_INTE_OVRIE(state ? (uint32_t) 1u : (uint32_t) 0u);
    baseAddr->INTE = (uint32_t) tmp;
}

/*!
 * @brief Get the FIFO-overrun interrupt-enable state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if overrun interrupts are enabled; `false` otherwise.
 */
static inline bool ADC_GetOvrIntEnableFlag(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t) baseAddr->INTE;
    tmp = (tmp & ADC_INTE_OVRIE_MASK) >> ADC_INTE_OVRIE_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Enable or disable ADC-ready interrupts.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] state `true` to enable ready interrupts; `false` otherwise.
 */
static inline void ADC_SetReadyIntEnableFlag(ADC_Type *const baseAddr,
                                           const bool state)
{
    uint32_t tmp = (uint32_t) baseAddr->INTE;
    tmp &= (uint32_t) (~(ADC_INTE_ADRDYIE_MASK));
    tmp |= ADC_INTE_ADRDYIE(state ? (uint32_t) 1u : (uint32_t) 0u);
    baseAddr->INTE = (uint32_t) tmp;
}

/*!
 * @brief Get the ADC-ready interrupt-enable state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if ready interrupts are enabled; `false` otherwise.
 */
static inline bool ADC_GetReadyIntEnableFlag(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t) baseAddr->INTE;
    tmp = (tmp & ADC_INTE_ADRDYIE_MASK) >> ADC_INTE_ADRDYIE_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Enable or disable sample-end interrupts.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] state `true` to enable sample-end interrupts; `false` otherwise.
 */
static inline void ADC_SetSampEndIntEnableFlag(ADC_Type *const baseAddr,
                                           const bool state)
{
    uint32_t tmp = (uint32_t) baseAddr->INTE;
    tmp &= (uint32_t) (~(ADC_INTE_EOSMPIE_MASK));
    tmp |= ADC_INTE_EOSMPIE(state ? (uint32_t) 1u : (uint32_t) 0u);
    baseAddr->INTE = (uint32_t) tmp;
}

/*!
 * @brief Get the sample-end interrupt-enable state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if sample-end interrupts are enabled; `false` otherwise.
 */
static inline bool ADC_GetSampEndIntEnableFlag(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t) baseAddr->INTE;
    tmp = (tmp & ADC_INTE_EOSMPIE_MASK) >> ADC_INTE_EOSMPIE_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Enable or disable end-of-conversion interrupts.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] state `true` to enable end-of-conversion interrupts; `false`
 *                  otherwise.
 */
static inline void ADC_SetEocIntEnableFlag(ADC_Type *const baseAddr,
                                           const bool state)
{
    uint32_t tmp = (uint32_t) baseAddr->INTE;
    tmp &= (uint32_t) (~(ADC_INTE_EOCIE_MASK));
    tmp |= ADC_INTE_EOCIE(state ? (uint32_t) 1u : (uint32_t) 0u);
    baseAddr->INTE = (uint32_t) tmp;
}

/*!
 * @brief Get the end-of-sequence interrupt-enable state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if end-of-sequence interrupts are enabled; `false` otherwise.
 */
static inline bool ADC_GetEoSeqIntEnableFlag(const ADC_Type *const baseAddr)
{
    uint32_t tmp = (uint32_t) baseAddr->INTE;
    tmp = (tmp & ADC_INTE_EOSEQIE_MASK) >> ADC_INTE_EOSEQIE_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Enable or disable injected-conversion interrupts.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] state `true` to enable injected-conversion interrupts; `false`
 *                  otherwise.
 */
static inline void ADC_SetInjectIntEnableFlag(ADC_Type *const baseAddr,
                                           const bool state)
{
#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
    uint32_t tmp = (uint32_t) baseAddr->INTE;
    tmp &= (uint32_t) (~(ADC_INTE_INJIE_MASK));
    tmp |= ADC_INTE_INJIE(state ? (uint32_t) 1u : (uint32_t) 0u);
    baseAddr->INTE = (uint32_t) tmp;
#else
    (void) baseAddr;
    (void) state;
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */
}

/*!
 * @brief Get the injected-conversion interrupt-enable state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if injected-conversion interrupts are enabled; `false`
 *         otherwise.
 */
static inline bool ADC_GetInjectIntEnableFlag(const ADC_Type *const baseAddr)
{
#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
    uint32_t tmp = (uint32_t) baseAddr->INTE;
    tmp = (tmp & ADC_INTE_INJIE_MASK) >> ADC_INTE_INJIE_SHIFT;
    return (tmp != 0u) ? true : false;
#else
    (void) baseAddr;
    return true;
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */
}

/*!
 * @brief Enable or disable injected-conversion error interrupts.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] state `true` to enable injected error interrupts; `false`
 *                  otherwise.
 */
static inline void ADC_SetInjectErrIntEnableFlag(ADC_Type *const baseAddr,
                                           const bool state)
{
#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
    uint32_t tmp = (uint32_t) baseAddr->INTE;
    tmp &= (uint32_t) (~(ADC_INTE_INJERRIE_MASK));
    tmp |= ADC_INTE_INJERRIE(state ? (uint32_t) 1u : (uint32_t) 0u);
    baseAddr->INTE = (uint32_t) tmp;
#else
    (void) baseAddr;
    (void) state;
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */
}

/*!
 * @brief Get the injected-conversion error interrupt-enable state.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if injected error interrupts are enabled; `false` otherwise.
 */
static inline bool ADC_GetInjectErrIntEnableFlag(const ADC_Type *const baseAddr)
{
#if defined(FEATURE_ADC_SUPPORT_INJECTION_MODE) && (FEATURE_ADC_SUPPORT_INJECTION_MODE > 0)
    uint32_t tmp = (uint32_t) baseAddr->INTE;
    tmp = (tmp & ADC_INTE_INJERRIE_MASK) >> ADC_INTE_INJERRIE_SHIFT;
    return (tmp != 0u) ? true : false;
#else
    (void) baseAddr;
    return true;
#endif /* FEATURE_ADC_SUPPORT_INJECTION_MODE */
}

/*!
 * @brief Enable or disable end-of-sequence interrupts.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] state `true` to enable end-of-sequence interrupts; `false`
 *                  otherwise.
 */
static inline void ADC_SetEoSeqIntEnableFlag(ADC_Type *const baseAddr,
                                             const bool state)
{
    uint32_t tmp = (uint32_t) baseAddr->INTE;
    tmp &= (uint32_t) (~(ADC_INTE_EOSEQIE_MASK));
    tmp |= ADC_INTE_EOSEQIE(state ? (uint32_t) 1u : (uint32_t) 0u);
    baseAddr->INTE = (uint32_t) tmp;
}

/*!
 * @brief Clear the end-of-conversion status flag.
 *
 * @param[in] baseAddr ADC base pointer.
 */
static inline void ADC_ClearEocFlagCmd(ADC_Type *const baseAddr)
{
    uint16_t timeout_cnt = 0U;
    /* Write-one-to-clear the EOC bit, then wait for the hardware to drop it. */
    baseAddr->STS = ADC_STS_EOC_MASK;
    /* E600005: wait until the flag is observed as cleared. */
    while((baseAddr->STS & ADC_STS_EOC_MASK) != 0u)
    {
        if(timeout_cnt > ADC_WAIT_TIMEOUT)
        {
            break;
        }
        timeout_cnt++;
    }
}

/*!
 * @brief Clear the end-of-sequence status flag.
 *
 * @param[in] baseAddr ADC base pointer.
 */
static inline void ADC_ClearEoseqFlagCmd(ADC_Type *const baseAddr)
{
    uint16_t timeout_cnt = 0U;
    /* Write-one-to-clear the EOSEQ bit, then wait for the hardware to drop it. */
    baseAddr->STS = (uint32_t) ADC_STS_EOSEQ_MASK;
    /* E600005: wait until the flag is observed as cleared. */
    while((baseAddr->STS & ADC_STS_EOSEQ_MASK) != 0u)
    {
        if(timeout_cnt > ADC_WAIT_TIMEOUT)
        {
            break;
        }
        timeout_cnt++;
    }
}

/*!
 * @brief Clear the FIFO-overrun status flag.
 *
 * @param[in] baseAddr ADC base pointer.
 */
static inline void ADC_ClearOvrFlagCmd(ADC_Type *const baseAddr)
{
    uint16_t timeout_cnt = 0U;
    /* Write-one-to-clear the OVR bit, then wait for the hardware to drop it. */
    baseAddr->STS = (uint32_t) ADC_STS_OVR_MASK;
    /* E600005: wait until the flag is observed as cleared. */
    while((baseAddr->STS & ADC_STS_OVR_MASK) != 0u)
    {
        if(timeout_cnt > ADC_WAIT_TIMEOUT)
        {
            break;
        }
        timeout_cnt++;
    }
}

/*!
 * @brief Clear the watchdog status flag for one watchdog channel.
 *
 * @param[in] baseAddr ADC base pointer.
 * @param[in] wdgChannel Watchdog channel index.
 */
static inline void ADC_ClearWdFlagCmd(ADC_Type *const baseAddr, const uint8_t wdgChannel)
{
    uint16_t timeout_cnt = 0U;
    /* Write-one-to-clear the watchdog bit, then wait for the hardware to drop it. */
#if defined(FEATURE_ADC_WDG_CHANNEL_COUNT) && (FEATURE_ADC_WDG_CHANNEL_COUNT > 1)
    baseAddr->STS = (uint32_t) (ADC_STS_WD0_MASK << wdgChannel);
    /* E600005: wait until the flag is observed as cleared. */
    while((baseAddr->STS & (ADC_STS_WD0_MASK << wdgChannel)) != 0u)
    {
        if(timeout_cnt > ADC_WAIT_TIMEOUT)
        {
            break;
        }
        timeout_cnt++;
    }
#else
    baseAddr->STS = (uint32_t) ADC_STS_WD_MASK;
    /* E600005: wait until the flag is observed as cleared. */
    while((baseAddr->STS & ADC_STS_WD_MASK) != 0u)
    {
        if(timeout_cnt > ADC_WAIT_TIMEOUT)
        {
            break;
        }
        timeout_cnt++;
    }
    (void) wdgChannel;
#endif /* (FEATURE_ADC_WDG_CHANNEL_COUNT > 1) */
}

/*!
 * @brief Clear the ADC-ready status flag.
 *
 * @param[in] baseAddr ADC base pointer.
 */
static inline void ADC_ClearReadyFlagCmd(ADC_Type *const baseAddr)
{
    uint16_t timeout_cnt = 0U;
    /* Write-one-to-clear the ready bit, then wait for the hardware to drop it. */
    baseAddr->STS = (uint32_t) ADC_STS_ADRDY_MASK;
    /* E600005: wait until the flag is observed as cleared. */
    while((baseAddr->STS & ADC_STS_ADRDY_MASK) != 0u)
    {
        if(timeout_cnt > ADC_WAIT_TIMEOUT)
        {
            break;
        }
        timeout_cnt++;
    }
}

/*!
 * @brief Clear the sample-end status flag.
 *
 * @param[in] baseAddr ADC base pointer.
 */
static inline void ADC_ClearSampEndFlagCmd(ADC_Type *const baseAddr)
{
    uint16_t timeout_cnt = 0U;
    /* Write-one-to-clear the sample-end bit, then wait for the hardware to drop it. */
    baseAddr->STS = (uint32_t) ADC_STS_EOSMP_MASK;
    /* E600005: wait until the flag is observed as cleared. */
    while((baseAddr->STS & ADC_STS_EOSMP_MASK) != 0u)
    {
        if(timeout_cnt > ADC_WAIT_TIMEOUT)
        {
            break;
        }
        timeout_cnt++;
    }
}

/*! @}*/ /* End of Interrupt & Flag Management */

/*!
 * @name FIFO & Runtime Control
 * @brief Functions for FIFO reads and direct runtime control of the ADC state
 *        machine.
 * @{
 */

/*!
 * @brief Read one conversion result from the FIFO.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return Raw FIFO sample value.
 */
static inline uint16_t ADC_ReadFIFO(ADC_Type *const baseAddr)
{
    return (uint16_t)baseAddr->FIFO;
}

/*!
 * @brief Get the conversion-complete status flag.
 *
 * @param[in] baseAddr ADC base pointer.
 * @return `true` if the conversion-complete flag is set; `false` otherwise.
 */
static inline bool ADC_GetConvCompleteFlag(ADC_Type *const baseAddr)
{
    uint32_t tmp = baseAddr->STS;
    tmp = (tmp & ADC_STS_EOC_MASK) >> ADC_STS_EOC_SHIFT;
    return (tmp != 0u) ? true : false;
}

/*!
 * @brief Request the ADC to stop the active sequence.
 *
 * @param[in] baseAddr ADC base pointer.
 */
static inline void ADC_Stop(ADC_Type *const baseAddr)
{
    if (ADC_GetConvActiveFlag(baseAddr))
    {
        baseAddr->CTRL |= ADC_CTRL_ADSTOP_MASK;
        while ((ADC_CTRL_ADSTOP_MASK & baseAddr->CTRL) != 0u)
        {
            /* Wait until the hardware acknowledges the stop request. */
        }
    }
}

/*!
 * @brief Request the ADC to start conversion processing.
 *
 * @param[in] baseAddr ADC base pointer.
 */
static inline void ADC_Start(ADC_Type *const baseAddr)
{
    uint32_t tmp = baseAddr->CTRL;
    tmp &= (uint32_t) (~(ADC_CTRL_ADSTART_MASK));
    tmp |= ADC_CTRL_ADSTART(1);
    baseAddr->CTRL = (uint32_t) tmp;
}

/*!
 * @brief Disable the ADC hardware.
 *
 * @param[in] baseAddr ADC base pointer.
 */
static inline void ADC_Disable(ADC_Type *const baseAddr)
{
    baseAddr->CTRL |= ADC_CTRL_ADDIS_MASK;
    while ((ADC_CTRL_ADDIS_MASK & baseAddr->CTRL) != 0u)
    {
        /* Wait until the hardware acknowledges the disable request. */
    }
}

/*!
 * @brief Enable the ADC hardware.
 *
 * @param[in] baseAddr ADC base pointer.
 */
static inline void ADC_Enable(ADC_Type *const baseAddr)
{
#ifdef ADC_ERRATA_E0002
    /* Preserve the converter register image before issuing the erratum reset sequence. */
    uint32_t adcRegister[8];
    uint8_t chsel[8], i;
    for(i = 0; i < 8U; i ++)
    {
        adcRegister[i] = *(volatile uint32_t *)((uint32_t)baseAddr + (0x4UL * i));
        chsel[i] = baseAddr->CHSEL[i];
    }
    /* Perform the IPC-driven software reset required by the erratum workaround. */
#if (ADC_INSTANCE_COUNT > 1)
    if(ADC0 == baseAddr)
    {
        IPC->CTRL[IPC_ADC0_INDEX] |= IPC_CTRL_SWREN_MASK;
        IPC->CTRL[IPC_ADC0_INDEX] &= ~IPC_CTRL_SWREN_MASK;
    }else
    {
        IPC->CTRL[IPC_ADC1_INDEX] |= IPC_CTRL_SWREN_MASK;
        IPC->CTRL[IPC_ADC1_INDEX] &= ~IPC_CTRL_SWREN_MASK;
    }
#else
    IPC->CTRL[IPC_ADC0_INDEX] |= IPC_CTRL_SWREN_MASK;
    IPC->CTRL[IPC_ADC0_INDEX] &= ~IPC_CTRL_SWREN_MASK;
#endif
    /* Restore the saved register image after the software reset completes. */
    for(i = 0; i < 8U; i ++)
    {
        *(volatile uint32_t *)((uint32_t)baseAddr + (0x4UL * i)) = adcRegister[i];
        baseAddr->CHSEL[i] = chsel[i];
    }
#endif
    if (0u == (ADC_CTRL_ADEN_MASK & baseAddr->CTRL))
    {
        baseAddr->CTRL |= ADC_CTRL_ADEN_MASK;
    }
}

/*! @}*/ /* End of FIFO & Runtime Control */


#if defined (__cplusplus)
}
#endif

/*! @} */ /* End of adc_hw_access group */

#endif /* ADC_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
