/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file sent_hw_access.c
 * @version 1.4.0
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 * 
 * PRQA S 2985 Rule 2.2 : This operation is redundant. The value of the result is 
 *                        always that of the left-hand operand.
 */

#include "device_registers.h"
#include "sent_hw_access.h"
#include "sent_driver.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/**
 * Used for bringing the register to a default reset state
 */
#define SENT_CFG_RESET_VALUE    (0x00010010u)
#define SENT_CHCTRL_RESET_VALUE    (0x00000000u)
#define SENT_NIBCFG_RESET_VALUE  (0x11000000u)
#define SENT_DMA_RESET_VALUE   (0x00000000u)
#define SENT_FRDYINTE_RESET_VALUE (0x00000000u)
#define SENT_SRDYINTE_RESET_VALUE (0x00000000u)
#define SENT_CHn_CLK_RESET_VALUE    (0x00008000u)
#define SENT_CHn_CONFIG_RESET_VALUE  (0x00000104u)

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * Base pointers for the peripheral registers.
 */
static SENT_Type * const s_sentBase[SENT_INSTANCE_COUNT] = SENT_BASE_PTRS;

#if defined(ERRATA_E7425) /* Enable only when we have this active */
/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_ErrataE7425Workaroud
 * Description   : Workarounds for E7425.
 *
 *END**************************************************************************/
bool SENT_DRV_HW_ErrataE7425Workaroud(const uint32_t instance, const uint32_t channel)
{
    static uint8_t remCount[SENT_INSTANCE_COUNT][SENT_CH_COUNT]; /* Remaining clearing count */
    bool rval = true;

    /* Only the channel has PAUSE pulse detection enabled */
    if ((s_sentBase[instance]->CH[channel].CFG & (uint32_t)SENT_CH_CFG_PPEN_MASK) != 0u)
    {
        /* NUM_EDGES_ERR reporting depends on errata */
        if ((s_sentBase[instance]->CH[channel].STS
                & (uint32_t)SENT_CH_STS_ENUMERR_MASK) != 0u)
        {
            if (remCount[instance][channel] > 0u)
            {
                /* False event */
                rval = false;

                /* Clear */
                s_sentBase[instance]->CH[channel].STS |= SENT_CH_STS_ENUMERR_MASK;
                remCount[instance][channel]--;
            }
        }

        /* Reload counters */
        if ((s_sentBase[instance]->CH[channel].STS
                & (uint32_t)(SENT_CH_STS_NVERR_MASK | SENT_CH_STS_FCRCERR_MASK)) != 0u)
        {
            remCount[instance][channel] = 2u;
        }
        else if ((s_sentBase[instance]->CH[channel].STS
                & (uint32_t)(SENT_CH_STS_CLERR_MASK | SENT_CH_STS_FMSGOF_MASK)) != 0u)
        {
            remCount[instance][channel] = 1u;
        }
        else
        {
            /* MISRA */
        }
    }

    return rval;
}
#endif /* defined(ERRATA_E7425) */

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetTimestampPrescaler
 * Description   : This function sets the timestamp prescaler value
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetTimestampPrescaler(const uint32_t instance, const uint8_t value)
{
    uint32_t tmp = s_sentBase[instance]->CFG;
    /* Ensure clearing before modification */
    tmp &= ~SENT_CFG_TSPRSC_MASK;
    tmp |= SENT_CFG_TSPRSC(value); /*PRQA S 2985*/

#ifdef FEATURE_SENT_HAS_NIB_LEN_VAR_LIMIT
    /* Jitter compensation enabled */
    tmp |= SENT_CFG_NIB_LEN_VAR_LIMIT(1u);
#endif
    s_sentBase[instance]->CFG = tmp;
}

#ifdef FEATURE_SENT_DMA_HAS_FIFO
/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetFifoWm
 * Description   : This function sets the FIFO watermark level.
 * The level describes the number of valid messages that are to
 * be stored in the FIFO queue before a DMA transfer is triggered.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetFifoWm(const uint32_t instance, const uint8_t value)
{
    uint8_t locVal;

    /* Valid values are between 1 and FEATURE_SENT_MAX_FIFO_SIZE */
    locVal = (value < 1u) ? 1u : ((value > FEATURE_SENT_MAX_FIFO_SIZE) ? FEATURE_SENT_MAX_FIFO_SIZE : value);

    /* Ensure clearing before modification */
    s_sentBase[instance]->CFG &= ~SENT_CFG_FIFOWM_MASK;
    s_sentBase[instance]->CFG |= SENT_CFG_FIFOWM(locVal);
}
#endif /* FEATURE_SENT_DMA_HAS_FIFO */

#ifdef FEATURE_SENT_DMA_HAS_FIFO
/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetFifoState
 * Description   : Sets the enables state (active/inactive) for the DMA FIFO
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetFifoState(const uint32_t instance, const bool state)
{
    /* No pre-clearing required */
    if(state)
    {
        s_sentBase[instance]->CFG |= SENT_CFG_FIFOEN(1u);
    }
    else
    {
        s_sentBase[instance]->CFG &= ~SENT_CFG_FIFOEN(1u);
    }
}
#endif /* FEATURE_SENT_DMA_HAS_FIFO */

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetChannelPrescaler
 * Description   : Sets the channel prescaler.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetChannelPrescaler(const uint32_t instance, const uint8_t channel, const uint16_t prescaler)
{
    uint16_t lPre;
    uint16_t maxVal;
    uint32_t tmp = s_sentBase[instance]->CH[channel].CLK;

    /* Prescaler limited to maximum range of SENT_CH_CLK_RPRSC_WIDTH bits */
    maxVal = (uint16_t)((uint16_t)((uint16_t)1u << SENT_CH_CLK_RPRSC_WIDTH) - 1u);
    lPre = (prescaler > maxVal) ? maxVal : prescaler;

    tmp &= ~SENT_CH_CLK_RPRSC_MASK;
    tmp |= SENT_CH_CLK_RPRSC(lPre); /*PRQA S 2985*/
    s_sentBase[instance]->CH[channel].CLK = tmp;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetChannelCompensation
 * Description   : Sets the channel compensation state (enable / disable).
 * This enables automatic compesation for deviations in the receve clock.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetChannelCompensation(const uint32_t instance, const uint8_t channel, const bool state)
{
    /* No pre clearing required */
    if(state)
    {
        s_sentBase[instance]->CH[channel].CLK |= SENT_CH_CLK_CMEN(1u);
    }
    else
    {
        s_sentBase[instance]->CH[channel].CLK &= ~SENT_CH_CLK_CMEN(1u);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetBusIdleCnt
 * Description   : Sets the value for the Bus Idle Count paramter.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetBusIdleCnt(const uint32_t instance, const uint8_t channel, const sent_diag_idle_cnt_cfg_t count)
{
    /* Clear first */
    s_sentBase[instance]->CH[channel].CFG &= ~SENT_CH_CFG_IDLECNT_MASK;

    switch(count)
    {
    case SENT_BUS_IDLE_DISABLED:
        s_sentBase[instance]->CH[channel].CFG |= SENT_CH_CFG_IDLECNT(0u);
        break;

    case SENT_BUS_IDLE_245_CLK_TICKS:
        s_sentBase[instance]->CH[channel].CFG |= SENT_CH_CFG_IDLECNT(1u);
        break;

    case SENT_BUS_IDLE_508_CLK_TICKS:
        s_sentBase[instance]->CH[channel].CFG |= SENT_CH_CFG_IDLECNT(2u);
        break;

    case SENT_BUS_IDLE_1016_CLK_TICKS:
        s_sentBase[instance]->CH[channel].CFG |= SENT_CH_CFG_IDLECNT(4u);
        break;

    case SENT_BUS_IDLE_2032_CLK_TICKS:
        s_sentBase[instance]->CH[channel].CFG |= SENT_CH_CFG_IDLECNT(8u);
        break;

    default: /* Disabled since there was a clear first */
        break;
    }
}

#if FEATURE_SENT_HAS_INPUT_FILTER
/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetBusFilterCnt
 * Description   : Sets the value for the Bus Filter Count paramter.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetBusFilterCnt(const uint32_t instance, const uint8_t channel, const sent_channel_input_filter_t count)
{
    uint32_t tmp = s_sentBase[instance]->CH[channel].CFG;
    /* Clear first */
    tmp &= ~SENT_CH_CFG_FILCNT_MASK;

    /* Set filter */
    if (SENT_INPUT_FILTER_NONE != count)
    {
        uint8_t filter = 1u << (((uint8_t) count) - 1u);
        tmp |= SENT_CH_CFG_FILCNT(filter); /*PRQA S 2985*/
    }
    s_sentBase[instance]->CH[channel].CFG = tmp;
}
#endif /* FEATURE_SENT_HAS_INPUT_FILTER */  

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetCalRng
 * Description   : Sets the value for the Calibration Range paramter.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetCalRng(const uint32_t instance, const uint8_t channel, const sent_diag_calib_pulse_var_cfg_t range)
{
    /* No pre clearing required */
    if(range == SENT_CALIB_VAR_25_PERCENT)
    {
        s_sentBase[instance]->CH[channel].CFG |= SENT_CH_CFG_CALRNG(1u);
    }
    else
    {
        s_sentBase[instance]->CH[channel].CFG &= ~SENT_CH_CFG_CALRNG(1u);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetPpChkSel
 * Description   : Sets the value for the Pause Pulse Check paramter.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetPpChkSel(const uint32_t instance, const uint8_t channel, const sent_diag_pulse_cfg_t check)
{
    /* No pre clearing required */
    if(check == SENT_PULSE_CHECK_PAUSE)
    {
        s_sentBase[instance]->CH[channel].CFG |= SENT_CH_CFG_PPCHKSEL(1u);
    }
    else
    {
        s_sentBase[instance]->CH[channel].CFG &= ~SENT_CH_CFG_PPCHKSEL(1u);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetPausePulseEnable
 * Description   : Enables / disables the detection of pause pulses.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetPausePulseEnable(const uint32_t instance, const uint8_t channel, const sent_diag_pause_pulse_cfg_t stat)
{
    /* No pre clearing required */
    if(stat == SENT_PAUSE_PULSE_ENABLED)
    {
        s_sentBase[instance]->CH[channel].CFG |= SENT_CH_CFG_PPEN(1u);
    }
    else
    {
        s_sentBase[instance]->CH[channel].CFG &= ~SENT_CH_CFG_PPEN(1u);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetSuccCalChk
 * Description   : Sets the value of the Successive Calibration Check parameter
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetSuccCalChk(const uint32_t instance, const uint8_t channel, const sent_diag_succ_cal_check_cfg_t type)
{
    /* No pre clearing required */
    if(type == SENT_SUCC_CAL_CHK_PREFERRED)
    {
        s_sentBase[instance]->CH[channel].CFG |= SENT_CH_CFG_SCCHK(1u);
    }
    else
    {
        s_sentBase[instance]->CH[channel].CFG &= ~SENT_CH_CFG_SCCHK(1u);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetSlowCrcType
 * Description   : Sets the value of the Slow Message CRC Type parameter
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetSlowCrcType(const uint32_t instance, const uint8_t channel, const sent_msg_crc_t type)
{
    /* No pre clearing required */
    if(type == SENT_CRC_LEGACY)
    {
        s_sentBase[instance]->CH[channel].CFG |= SENT_CH_CFG_SCRCTYPE(1u);
    }
    else
    {
        s_sentBase[instance]->CH[channel].CFG &= ~SENT_CH_CFG_SCRCTYPE(1u);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetFastCrcType
 * Description   : Sets the value of the Fast Message CRC Type parameter
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetFastCrcType(const uint32_t instance, const uint8_t channel, const sent_msg_crc_t type)
{
    /* No pre clearing required */
    if(type == SENT_CRC_LEGACY)
    {
        s_sentBase[instance]->CH[channel].CFG |= SENT_CH_CFG_FCRCTYPE(1u);
    }
    else
    {
        s_sentBase[instance]->CH[channel].CFG &= ~SENT_CH_CFG_FCRCTYPE(1u);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetFastCrcIncStatus
 * Description   : Sets the Include Status Nibble in CRC calculation parameter.
 * This is valid only for Fast messages.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetFastCrcIncStatus(const uint32_t instance, const uint8_t channel, const bool status)
{
    /* No pre clearing required */
    if(status)
    {
        s_sentBase[instance]->CH[channel].CFG |= SENT_CH_CFG_FCRCSCEN(1u);
    }
    else
    {
        s_sentBase[instance]->CH[channel].CFG &= ~SENT_CH_CFG_FCRCSCEN(1u);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetFastDisableCrc
 * Description   : Sets the enabling / disabling of CRC verification for Fast
 * messages
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetFastDisableCrc(const uint32_t instance, const uint8_t channel, const bool status)
{
    /* No pre clearing required */
    if(status)
    {
        s_sentBase[instance]->CH[channel].CFG |= SENT_CH_CFG_FCRCCHK(1u);
    }
    else
    {
        s_sentBase[instance]->CH[channel].CFG &= ~SENT_CH_CFG_FCRCCHK(1u);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetFastNumNibbles
 * Description   : Configures the number of nibbles for the Fast channel.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetFastNumNibbles(const uint32_t instance, const uint8_t channel, const uint8_t num)
{
    uint32_t locVal = (num < 1u) ? 1u : ((num > 6u) ? 6u : num);
    uint32_t tmp = s_sentBase[instance]->NIBCFG;
    /* Clear the current value */
    tmp &= ~((uint32_t)SENT_NIBCFG_NIBCH0_MASK << (4u * channel));

    /* Write it back */
    tmp |= (locVal << (4u * channel)); /* Register layout */
    s_sentBase[instance]->NIBCFG = tmp;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetFastDma
 * Description   : Enables / disables DMA transfers for the given Fast channel.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetFastDma(const uint32_t instance, const uint8_t channel, const bool enable)
{
    /* Just mask the bit */
    /* Predefined masks not useable */
    if(enable)
    {
        s_sentBase[instance]->DMACTRL |= (uint32_t)((uint32_t)1u << (channel + 16u));
    }
    else
    {
        s_sentBase[instance]->DMACTRL &= (uint32_t)(~((uint32_t)1u << (channel + 16u)));
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetSlowDma
 * Description   : Enables / disables DMA transfers for the given Slow channel.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetSlowDma(const uint32_t instance, const uint8_t channel, const bool enable)
{
    /* Just mask the bit */
    /* Predefined masks not useable */
    if(enable)
    {
        s_sentBase[instance]->DMACTRL |= (uint32_t)((uint32_t)1u << channel);
    }
    else
    {
        s_sentBase[instance]->DMACTRL &= (uint32_t)(~((uint32_t)1u << channel));
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetChannelStatus
 * Description   : Enables / disables reception for the given channels.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetChannelStatus(const uint32_t instance, const uint8_t channel, const bool enable)
{
    if(enable)
    {
        s_sentBase[instance]->CHCTRL |= (uint32_t)((uint32_t)1u << channel);
    }
    else
    {
        s_sentBase[instance]->CHCTRL &= (uint32_t)(~((uint32_t)1u << channel));
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetPeripheralStatus
 * Description   : Enables / disables the peripheral
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetPeripheralStatus(const uint32_t instance, const bool enable)
{
    if(enable)
    {
        s_sentBase[instance]->CTRL |= SENT_CTRL_EN(1u);
    }
    else
    {
        s_sentBase[instance]->CTRL &= ~SENT_CTRL_EN(1u);
    }
}


/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetEventConfig
 * Description   : Enables events received in a mask. These events will signal
 * diagnostics notifications to the application.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetEventConfig(const uint32_t instance, const uint8_t channel, const sent_event_t events)
{
    /* Clear all masks first */
    s_sentBase[instance]->CH[channel].INTE = 0;

    /* Set the new ones */
    s_sentBase[instance]->CH[channel].INTE |= (uint32_t)events;

#if defined(ERRATA_E7425) /* Enable anyway so we can implement the workaround */

    s_sentBase[instance]->CH[channel].INTE |= SENT_CH_INTE_NVERRIE(1u);
    s_sentBase[instance]->CH[channel].INTE |= SENT_CH_INTE_FCRCERRIE(1u);
    s_sentBase[instance]->CH[channel].INTE |= SENT_CH_INTE_CLERRIE(1u);
    s_sentBase[instance]->CH[channel].INTE |= SENT_CH_INTE_FMSGOFIE(1u);

#endif /* defined(ERRATA_E7425) */
 }

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_GetActiveEvents
 * Description   : Returns current active diagnostics events
 *
 *END**************************************************************************/
sent_event_t SENT_DRV_HW_GetActiveEvents(const uint32_t instance, const uint8_t channel)
{
    uint32_t regState = s_sentBase[instance]->CH[channel].STS;
    uint32_t regGlobal = s_sentBase[instance]->GSTS;
    sent_event_t evTemp = 0u;

    /* Put events in temp var */
    if((regState & (uint32_t)SENT_CH_STS_IDLE_MASK) != 0u)
    {
        evTemp |= SENT_EV_BUS_IDLE;
    }

    if((regState & (uint32_t)SENT_CH_STS_CALRESYNC_MASK) != 0u)
    {
        evTemp |= SENT_EV_CAL_RESYNC;
    }

    if((regState & (uint32_t)SENT_CH_STS_CAL_20_25_MASK) != 0u)
    {
        evTemp |= SENT_EV_CAL_20_25;
    }

    if((regState & (uint32_t)SENT_CH_STS_SMOF_MASK) != 0u)
    {
        evTemp |= SENT_EV_SMSG_OFLW;
    }

    if((regState & (uint32_t)SENT_CH_STS_FMOF_MASK) != 0u)
    {
        evTemp |= SENT_EV_FMSG_OFLW;
    }

    if((regState & (uint32_t)SENT_CH_STS_PPDERR_MASK) != 0u)
    {
        evTemp |= SENT_EV_PP_DIAG_ERR;
    }

    if((regState & (uint32_t)SENT_CH_STS_CLERR_MASK) != 0u)
    {
        evTemp |= SENT_EV_CAL_LEN_ERR;
    }

    if((regState & (uint32_t)SENT_CH_STS_CDERR_MASK) != 0u)
    {
        evTemp |= SENT_EV_CAL_DIAG_ERR;
    }

    if((regState & (uint32_t)SENT_CH_STS_NVERR_MASK) != 0u)
    {
        evTemp |= SENT_EV_NIB_VAL_ERR;
    }

    if((regState & (uint32_t)SENT_CH_STS_SCRCERR_MASK) != 0u)
    {
        evTemp |= SENT_EV_SMSG_CRC_ERR;
    }

    if((regState & (uint32_t)SENT_CH_STS_FCRCERR_MASK) != 0u)
    {
        evTemp |= SENT_EV_FMSG_CRC_ERR;
    }

    if((regState & (uint32_t)SENT_CH_STS_ENUMERR_MASK) != 0u)
    {
        evTemp |= SENT_EV_NUM_EDGES_ERR;
    }

#ifdef FEATURE_SENT_DMA_HAS_FIFO
    if((regGlobal & (uint32_t)SENT_GSTS_FMFO_MASK) != 0u)
    {
        evTemp |= SENT_EV_FIFO_OVERFLOW;
    }
#endif

    if((regGlobal & (uint32_t)SENT_GSTS_FMDU_MASK) != 0u)
    {
        evTemp |= SENT_EV_FDMA_UNDERFLOW;
    }

    if((regGlobal & (uint32_t)SENT_GSTS_SMDU_MASK) != 0u)
    {
        evTemp |= SENT_EV_SDMA_UNDERFLOW;
    }

    return evTemp;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_ClearActiveEvents
 * Description   : Clears the flags for the events in the given mask.
 *
 *END**************************************************************************/
void SENT_DRV_HW_ClearActiveEvents(const uint32_t instance, const uint8_t channel, const sent_event_t events)
{
    uint32_t clrMask = 0u;
    uint32_t gClrMask = 0u; /* For global events */

    /* Put events in temp var */
    if((events & (sent_event_t)SENT_EV_BUS_IDLE) != 0u)
    {
        clrMask |= SENT_CH_STS_IDLE_MASK;
    }

    if((events & (sent_event_t)SENT_EV_CAL_RESYNC) != 0u)
    {
        clrMask |= SENT_CH_STS_CALRESYNC_MASK;
    }

    if((events & (sent_event_t)SENT_EV_CAL_20_25) != 0u)
    {
        clrMask |= SENT_CH_STS_CAL_20_25_MASK;
    }

    if((events & (sent_event_t)SENT_EV_SMSG_OFLW) != 0u)
    {
        clrMask |= SENT_CH_STS_SMOF_MASK;
    }

    if((events & (sent_event_t)SENT_EV_FMSG_OFLW) != 0u)
    {
        clrMask |= SENT_CH_STS_FMOF_MASK;
    }

    if((events & (sent_event_t)SENT_EV_PP_DIAG_ERR) != 0u)
    {
        clrMask |= SENT_CH_STS_PPDERR_MASK;
    }

    if((events & (sent_event_t)SENT_EV_CAL_LEN_ERR) != 0u)
    {
        clrMask |= SENT_CH_STS_CLERR_MASK;
    }

    if((events & (sent_event_t)SENT_EV_CAL_DIAG_ERR) != 0u)
    {
        clrMask |= SENT_CH_STS_CDERR_MASK;
    }

    if((events & (sent_event_t)SENT_EV_NIB_VAL_ERR) != 0u)
    {
        clrMask |= SENT_CH_STS_NVERR_MASK;
    }

    if((events & (sent_event_t)SENT_EV_SMSG_CRC_ERR) != 0u)
    {
        clrMask |= SENT_CH_STS_SCRCERR_MASK;
    }

    if((events & (sent_event_t)SENT_EV_FMSG_CRC_ERR) != 0u)
    {
        clrMask |= SENT_CH_STS_FCRCERR_MASK;
    }

    if((events & (sent_event_t)SENT_EV_NUM_EDGES_ERR) != 0u)
    {
        clrMask |= SENT_CH_STS_ENUMERR_MASK;
    }

    /* RMW the channel status register */
    s_sentBase[instance]->CH[channel].STS |= clrMask;

#ifdef FEATURE_SENT_DMA_HAS_FIFO
    if((events & (sent_event_t)SENT_EV_FIFO_OVERFLOW) != 0u)
    {
        gClrMask |= SENT_GSTS_FMFO_MASK;
    }
#endif /* FEATURE_SENT_DMA_HAS_FIFO */

    if((events & (sent_event_t)SENT_EV_FDMA_UNDERFLOW) != 0u)
    {
        gClrMask |= SENT_GSTS_FMDU_MASK;
    }

    if((events & (sent_event_t)SENT_EV_SDMA_UNDERFLOW) != 0u)
    {
        gClrMask |= SENT_GSTS_SMDU_MASK;
    }

    /* Global RMW */
    s_sentBase[instance]->GSTS |= gClrMask;

}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetFastRxInterrupt
 * Description   : Enables / disables Interrupts for the Rx
 * event on the Slow channel.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetFastRxInterrupt(const uint32_t instance, const uint8_t channel, const bool enable)
{
    /* Just mask the bit */
    /* Predefined masks not usable */
    if(enable)
    {
        s_sentBase[instance]->FINTE |= (uint32_t)((uint32_t)1u << channel);
    }
    else
    {
        s_sentBase[instance]->FINTE &= (uint32_t)(~((uint32_t)1u << channel));
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SetSlowRxInterruptStatus
 * Description   : Enables / disables Interrupts for the Rx
 * event on the Slow channel.
 *
 *END**************************************************************************/
void SENT_DRV_HW_SetSlowRxInterruptStatus(const uint32_t instance, const uint8_t channel, const bool enable)
{
    /* Just mask the bit */
    /* Predefined masks not usable */
    if(enable)
    {
        s_sentBase[instance]->SINTE |= (uint32_t)((uint32_t)1u << channel);
    }
    else
    {
        s_sentBase[instance]->SINTE &= (uint32_t)(~((uint32_t)1u << channel));
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_GetFastRxStatus
 * Description   : Returns current Rx status for the Fast channel.
 * This will return true if there
 * is a new message present in the buffer.
 *
 *END**************************************************************************/
bool SENT_DRV_HW_GetFastRxStatus(const uint32_t instance, const uint32_t channel)
{
    /* Just pass the status */
    return (((uint32_t)(s_sentBase[instance]->FRDY & ((uint32_t)1u << channel)) != 0u) ? true : false);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_GetSlowRxStatus
 * Description   : Returns current Rx status for the Slow channel.
 * This will return true if there
 * is a new message present in the buffer.
 *
 *END**************************************************************************/
bool SENT_DRV_HW_GetSlowRxStatus(const uint32_t instance, const uint32_t channel)
{
    /* Just pass the status */
    return (((uint32_t)(s_sentBase[instance]->SRDY & ((uint32_t)1u << channel)) != 0u) ? true : false);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_GetWakeupEvent
 * Description   : Returns wakeup status.
 * This will return true if there
 * is a valid wakeup event.
 *
 *END**************************************************************************/
bool SENT_DRV_HW_GetWakeupEvent(const uint32_t instance)
{
    bool retVal = false;
    /* Check INTE and pass the status if valid */
    if(((s_sentBase[instance]->GINTE & SENT_GINTE_WUIE_MASK) != 0u) && 
       ((s_sentBase[instance]->GSTS & SENT_GSTS_WUF_MASK) != 0u))
    {
        retVal = true;
    }
    return retVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_GetFastRawMsg
 * Description   : Returns current Fast message in the raw format (unformatted).
 *
 *END**************************************************************************/
void SENT_DRV_HW_GetFastRawMsg(const uint32_t instance, const uint8_t channel, sent_raw_msg_t * rawMsg)
{
    /* Populate according to registers */
    rawMsg->dataField0 = s_sentBase[instance]->CH[channel].FMDATA;
    rawMsg->dataField1 = s_sentBase[instance]->CH[channel].FMCHNUM;
    rawMsg->dataField2 = s_sentBase[instance]->CH[channel].FMTS;

    /* Clear the RDY channels */
    s_sentBase[instance]->FRDY = (uint32_t)((uint32_t)1u << channel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_GetSlowRawMsg
 * Description   : Returns current Slow message in the raw format (unformatted).
 *
 *END**************************************************************************/
void SENT_DRV_HW_GetSlowRawMsg(const uint32_t instance, const uint8_t channel, sent_raw_msg_t * rawMsg)
{
    /* Populate according to registers */
    rawMsg->dataField0 = s_sentBase[instance]->CH[channel].SMBIT3;
    rawMsg->dataField1 = s_sentBase[instance]->CH[channel].SMBIT2;
    rawMsg->dataField2 = s_sentBase[instance]->CH[channel].SMTS;

    /* Clear the RDY channels */
    s_sentBase[instance]->SRDY = (uint32_t)((uint32_t)1u << channel);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_ConvertFastRaw
 * Description   : Converts an unformatted (raw) Fast message
 * into a formatted one.
 *
 *END**************************************************************************/
void SENT_DRV_HW_ConvertFastRaw(sent_fast_msg_t * msg, const sent_raw_msg_t * rawMsg)
{
    /* FMDATA register */
    msg->channelNumber = (uint8_t)((uint32_t)(rawMsg->dataField1 & SENT_CH_FMCHNUM_CHNUM_MASK) >> SENT_CH_FMCHNUM_CHNUM_SHIFT); /*PRQA S 2985*/
    msg->statusField = (uint8_t)((uint32_t)(rawMsg->dataField0 & SENT_FMDATA_SCNIB_MASK) >> SENT_FMDATA_SCNIB_SHIFT);

    /* Get full data and then shift depending on configured number of nibbles */
    msg->data = rawMsg->dataField0 & (SENT_CH_FMDATA_DNIB0_MASK
                                        | SENT_CH_FMDATA_DNIB1_MASK
                                        | SENT_CH_FMDATA_DNIB2_MASK
                                        | SENT_CH_FMDATA_DNIB3_MASK
                                        | SENT_CH_FMDATA_DNIB4_MASK
                                        | SENT_CH_FMDATA_DNIB5_MASK);

    /* FM_CRC register */
    msg->crc = (uint8_t)((uint32_t)(rawMsg->dataField0 & SENT_CH_FMDATA_CRC4_MASK) >> SENT_CH_FMDATA_CRC4_SHIFT); /*PRQA S 2985*/

    /* FM TS register */
    msg->timeStamp = rawMsg->dataField2;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_ConvertSlowRaw
 * Description   : Converts an unformatted (raw) Slow message
 * into a formatted one.
 *
 *END**************************************************************************/
void SENT_DRV_HW_ConvertSlowRaw(sent_slow_msg_t * msg, const sent_raw_msg_t * rawMsg)
{
    sent_slow_msg_type_t locType;

    /* Channel number */
    msg->channelNumber = (uint8_t)((uint32_t)(rawMsg->dataField0 & SENT_CH_SMBIT3_CHNUM_MASK) >> SENT_CH_SMBIT3_CHNUM_SHIFT); /*PRQA S 2985*/

    /* CRC */
    msg->crc = (uint8_t)((uint32_t)(rawMsg->dataField1 & SENT_CH_SMBIT2_SMCRC_MASK) >> SENT_CH_SMBIT2_SMCRC_SHIFT);

    /* Timestamp */
    msg->timeStamp = rawMsg->dataField2;

    /* Lower 12 bits of data */
    msg->data = (uint16_t)(rawMsg->dataField1 & SENT_CH_SMBIT2_DATA_MASK);

    /* This is more complicated due to message type */
    if((rawMsg->dataField0 & (uint32_t)SENT_CH_SMBIT3_TYPE_MASK) != 0u)
    {
        /* Enhanced */
        if((rawMsg->dataField0 & (uint32_t)SENT_CH_SMBIT3_CFG_MASK) != 0u)
        {
            /* 4 bit ID, C = 1 */
            locType = SENT_SLOW_TYPE_ENHANCED_4_BIT;
            msg->data |= (uint16_t)((uint32_t)((uint32_t)(rawMsg->dataField0 & SENT_CH_SMBIT3_ID3_0_DATA15_12_MASK)
                            >> SENT_CH_SMBIT3_ID3_0_DATA15_12_SHIFT) << 12u);
            msg->id = (uint8_t)((uint32_t)(rawMsg->dataField0 & SENT_CH_SMBIT3_ID7_4_ID3_0_MASK)
                            >> SENT_CH_SMBIT3_ID7_4_ID3_0_SHIFT);
        }
        else
        {
            /* 8 bit ID, C = 0 */
            locType = SENT_SLOW_TYPE_ENHANCED_8_BIT;
            msg->id = (uint8_t)((uint32_t)(rawMsg->dataField0 & SENT_CH_SMBIT3_ID3_0_DATA15_12_MASK)
                            >> SENT_CH_SMBIT3_ID3_0_DATA15_12_SHIFT);
            msg->id |= (uint8_t)((uint32_t)((uint32_t)(rawMsg->dataField0 & SENT_CH_SMBIT3_ID7_4_ID3_0_MASK)
                            >> SENT_CH_SMBIT3_ID7_4_ID3_0_SHIFT) << 4u);
        }
    }
    else
    {
        /* Standard */
        locType = SENT_SLOW_TYPE_SHORT;
        msg->id = (uint8_t)(msg->data >> 8u);
        msg->data = (uint16_t)(msg->data & 0xFFu);
    }

    /* Message type */
    msg->type = locType;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_ResetPeripheral
 * Description   : Disables the peripheral and brings it's
 * register into a default state
 *
 *END**************************************************************************/
void SENT_DRV_HW_ResetPeripheral(const uint32_t instance)
{
    uint8_t chInd;

    /* Peripheral wide */
    s_sentBase[instance]->CFG = SENT_CFG_RESET_VALUE;
    s_sentBase[instance]->CHCTRL = SENT_CHCTRL_RESET_VALUE;
    s_sentBase[instance]->NIBCFG = SENT_NIBCFG_RESET_VALUE;
    s_sentBase[instance]->DMACTRL = SENT_DMA_RESET_VALUE;
    s_sentBase[instance]->FINTE = SENT_FRDYINTE_RESET_VALUE;
    s_sentBase[instance]->SINTE = SENT_SRDYINTE_RESET_VALUE;

    /* For each channel */
    for(chInd = 0u; chInd < SENT_CH_COUNT; chInd++)
    {
        s_sentBase[instance]->CH[chInd].CLK = SENT_CHn_CLK_RESET_VALUE;
        s_sentBase[instance]->CH[chInd].CFG = SENT_CHn_CONFIG_RESET_VALUE;
    }
}

#if FEATURE_SENT_HAS_SPC
/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_ConfigSPC
 * Description   : Config SPC for SENT transfer
 *
 *END**************************************************************************/
void SENT_DRV_HW_ConfigSPC(uint32_t instance, uint8_t channel, const sent_spc_config_t * config)
{
    s_sentBase[instance]->CH[channel].SPCCFG = SENT_CH_SPCCFG_PLEN(config->pulseLength) |
                                               SENT_CH_SPCCFG_SPCPOL(config->pulsePolarity) |
                                               SENT_CH_SPCCFG_SPCMD(config->workMode) |
                                               SENT_CH_SPCCFG_TRGMD(config->triggerMode) |
                                               SENT_CH_SPCCFG_SPCEN(config->enable);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_SPCSwTrigger
 * Description   : Generate a software trigger for SPC pulse
 *
 *END**************************************************************************/
void SENT_DRV_HW_SPCSwTrigger(uint32_t instance, uint8_t channel)
{
    s_sentBase[instance]->CH[channel].SPCCFG |= SENT_CH_SPCCFG_SWTRG_MASK;
}
#endif /* FEATURE_SENT_HAS_SPC */

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_GetSlowDmaRegStartAddr
 * Description   : Returns the start address for the Fast DMA buffer
 *
 *END**************************************************************************/
volatile const uint32_t * SENT_DRV_HW_GetSlowDmaRegStartAddr(const uint32_t instance)
{
    return &s_sentBase[instance]->SMBIT3;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SENT_DRV_HW_GetFastDmaRegStartAddr
 * Description   : Returns the start address for the Slow DMA buffer
 *
 *END**************************************************************************/
volatile const uint32_t * SENT_DRV_HW_GetFastDmaRegStartAddr(const uint32_t instance)
{
    return &s_sentBase[instance]->FMDATA;
}

/*******************************************************************************
* EOF
*******************************************************************************/
