/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file spi_hw_access.c
 * @brief SPI hardware register access layer implementation.
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 2985 Rule 2.2: This operation is redundant. The value of the result is 
 *                       always that of the left-hand operand.
 */

#include "spi_hw_access.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Reset the SPI module to its default state.
 */
void SPI_Init(SPI_Type *base)
{
    base->CTRL = 0x00000000; // software spi module
}

/*!
 * @brief Disable the SPI module.
 */
status_t SPI_Disable(SPI_Type *base)
{
    status_t status = STATUS_SUCCESS;
    uint32_t spi_tmp = base->STS;
    spi_tmp = (spi_tmp & SPI_STS_BUSY_MASK) >> SPI_STS_BUSY_SHIFT;

    if (spi_tmp == (uint32_t)1)
    {
        status = STATUS_BUSY;
    }
    else
    {
        base->CTRL = base->CTRL & (~(SPI_CTRL_EN_MASK));
    }
    return status;
}

/*!
 * @brief Configure the SPI for master or slave mode.
 */
status_t SPI_SetMasterSlaveMode(SPI_Type *base, spi_master_slave_mode_t mode)
{
    base->CTRL = (base->CTRL & (~SPI_CTRL_MODE_MASK)) | ((uint32_t)mode << SPI_CTRL_MODE_SHIFT);
    return STATUS_SUCCESS;
}
#if !defined(FEATURE_SPI_LITE_VERSION)
/*!
 * @brief Flush the SPI TX and/or RX FIFOs.
 */
void SPI_SetFlushFifoCmd(SPI_Type *base, bool flushTxFifo, bool flushRxFifo)
{
    base->TXFIFO |= (uint32_t)(flushTxFifo ? 1 : 0) << SPI_TXFIFO_RESET_SHIFT;
    base->RXFIFO |= (uint32_t)(flushRxFifo ? 1 : 0) << SPI_TXFIFO_RESET_SHIFT;
}

#endif /* FEATURE_SPI_LITE_VERSION */

/*!
 * @brief Clear a write-1-to-clear (W1C) status flag.
 */
status_t SPI_ClearStatusFlag(SPI_Type *base, spi_status_flag_t statusFlag)
{
    if (statusFlag == SPI_ALL_STATUS)
    {
        base->STS = (uint32_t)SPI_ALL_STATUS;
    }
    else
    {
        base->STS = ((uint32_t)1U << (uint32_t)statusFlag);
    }
    return STATUS_SUCCESS;
}

/*!
 * @brief Set the polarity of a specific PCS signal.
 */
status_t SPI_SetPcsPolarityMode(SPI_Type *base, spi_which_pcs_t whichPcs, spi_signal_polarity_t pcsPolarity)
{
    uint32_t cfgr1Value = 0;

    /* Clear the PCS polarity bit */
    cfgr1Value = (base->CTRL) & (~((uint32_t)1U << (SPI_CTRL_CSPOL_SHIFT + (uint32_t)whichPcs)));

    /* Configure the PCS polarity bit according to the pcsPolarity setting */
    cfgr1Value |= (uint32_t)pcsPolarity << (SPI_CTRL_CSPOL_SHIFT + (uint32_t)whichPcs);

    base->CTRL = cfgr1Value;

    return STATUS_SUCCESS;
}

/*!
 * @brief Configure the SDO/SDI pin routing and output behavior.
 */
status_t
SPI_SetPinConfigMode(SPI_Type *base, spi_pin_config_t pinCfg, spi_data_out_config_t dataOutConfig, bool pcs3and2Enable)
{
    uint32_t cfgr1Value = 0;
#if !defined(FEATURE_SPI_LITE_VERSION)
    cfgr1Value = base->CTRL & ~(SPI_CTRL_PINCFG_MASK | SPI_CTRL_DHZEN_MASK | SPI_CTRL_CSDEN_MASK);

    cfgr1Value |= ((uint32_t)(pinCfg) << SPI_CTRL_PINCFG_SHIFT) | ((uint32_t)(dataOutConfig) << SPI_CTRL_DHZEN_SHIFT) |
                  ((uint32_t)(pcs3and2Enable ? 0 : 1) << SPI_CTRL_CSDEN_SHIFT); /* enable = 0 */

    base->CTRL = cfgr1Value;
#else
    (void)pcs3and2Enable;
    DevAssert(pcs3and2Enable == false);
    cfgr1Value = base->CTRL & ~(SPI_CTRL_PINCFG_MASK | SPI_CTRL_DHZEN_MASK);

    cfgr1Value |= ((uint32_t)(pinCfg) << SPI_CTRL_PINCFG_SHIFT) | ((uint32_t)(dataOutConfig) << SPI_CTRL_DHZEN_SHIFT); /* enable = 0 */

    base->CTRL = cfgr1Value;
#endif /* FEATURE_SPI_LITE_VERSION */

    return STATUS_SUCCESS;
}

/*!
 * @brief Compute absolute difference between two unsigned values.
 *
 * Internal helper for baud rate calculation.
 */
static uint32_t abs_dif(uint32_t a, uint32_t b)
{
    return (a > b) ? (a - b) : (b - a);
}

/*!
 * @brief Calculate and set the SPI baud rate.
 */
uint32_t SPI_SetBaudRate(SPI_Type *base, uint32_t bitsPerSec, uint32_t sourceClockInHz, uint32_t *tcrPrescaleValue)
{
    uint32_t bestFreq = 0xFFFFFFFFU;
    uint32_t bestScaler = 0U;
    uint32_t bestPrescaler = 0U;
    uint32_t freq1 = 0U;
    uint32_t freq2 = 0U;
    uint8_t scaler = 0U;
    uint8_t prescaler = 0U;
    uint32_t low, high;
    uint32_t tempBestFreq = 0U;
    uint32_t tempBestScaler = 0U;

    for (prescaler = 0; prescaler < 8U; prescaler++)
    {
        low = 0U;
        high = 256U;

        /* Implement golden section search algorithm */
        do
        {
            scaler = (uint8_t)((low + high) / 2U);
            freq1 = sourceClockInHz / (s_baudratePrescaler[prescaler] * (scaler + (uint32_t)2U));

            if (abs_dif(bitsPerSec, bestFreq) > abs_dif(bitsPerSec, freq1))
            {
                bestFreq = freq1;
            }
            if (freq1 < bitsPerSec)
            {
                high = scaler;
            }
            else
            {
                low = scaler;
            }
        } while ((high - low) > 1U);

        /* Evaluate last 2 scaler values */
        freq1 = sourceClockInHz / (s_baudratePrescaler[prescaler] * (low + (uint32_t)2U));
        freq2 = sourceClockInHz / (s_baudratePrescaler[prescaler] * (high + (uint32_t)2U));

        if (abs_dif(bitsPerSec, freq1) > abs_dif(bitsPerSec, freq2))
        {
            tempBestFreq = freq2;
            tempBestScaler = high;
        }
        else
        {
            tempBestFreq = freq1;
            tempBestScaler = low;
        }

        if (abs_dif(bitsPerSec, bestFreq) >= abs_dif(bitsPerSec, tempBestFreq))
        {
            bestFreq = tempBestFreq;
            bestScaler = tempBestScaler;
            bestPrescaler = prescaler;
        }

        /* If current frequency is equal to target frequency  stop the search */
        if (bestFreq == bitsPerSec)
        {
            break;
        }
    }

    /* Add default values for delay between transfers, delay between sck to pcs and between pcs to sck. */
    (void)SPI_SetDelay(base, SPI_SCK_TO_PCS, bestScaler >> 2U);
    (void)SPI_SetDelay(base, SPI_PCS_TO_SCK, bestScaler >> 2U);
    (void)SPI_SetDelay(base, SPI_BETWEEN_TRANSFER, bestScaler >> 2U);

    /* Write the best baud rate scalar to the CCR.
     * Note, no need to check for error since we've already checked to make sure the module is
     * disabled and in master mode. Also, there is a limit on the maximum divider so we will not
     * exceed this.
     */
    (void)SPI_SetBaudRateDivisor(base, bestScaler);

    /* return the best prescaler value for user to use later */
    *tcrPrescaleValue = bestPrescaler;

    /* return the actual calculated baud rate */
    return bestFreq;
}

/*!
 * @brief Manually set the SCK clock divider value.
 */
status_t SPI_SetBaudRateDivisor(SPI_Type *base, uint32_t divisor)
{
    uint32_t spi_tmp;

    spi_tmp = base->CLK;
    spi_tmp &= ~(SPI_CLK_DIV_MASK);
    spi_tmp |= SPI_CLK_DIV(divisor); /*PRQA S 2985*/
    base->CLK = spi_tmp;

    return STATUS_SUCCESS;
}

/*!
 * @brief Write the full Transmit Command Register (TCR).
 */
void SPI_SetTxCommandReg(SPI_Type *base, const spi_tx_cmd_config_t *txCmdCfgSet)
{
    base->TXCFG = ((SPI_TXCFG_CPOL(txCmdCfgSet->clkPolarity)) | /*PRQA S 2985*/
                   (SPI_TXCFG_CPHA(txCmdCfgSet->clkPhase)) |
#if !defined(FEATURE_SPI_LITE_VERSION)
                   (SPI_TXCFG_PRESCALE(txCmdCfgSet->preDiv)) |
                   (SPI_TXCFG_BSW((txCmdCfgSet->byteSwap) ? 1U : 0U)) | /*PRQA S 2985*/
                   (SPI_TXCFG_CONTC((txCmdCfgSet->contCmd) ? 1U : 0U)) | /*PRQA S 2985*/
                   (SPI_TXCFG_WIDTH(txCmdCfgSet->width)) |
                   (SPI_TXCFG_FRAMESZ(txCmdCfgSet->frameSize - 1UL)) | /*PRQA S 2985*/
#endif /* FEATURE_SPI_LITE_VERSION */
                   (SPI_TXCFG_PCS(txCmdCfgSet->whichPcs)) |
                   (SPI_TXCFG_LSBF((txCmdCfgSet->lsbFirst) ? 1U : 0U)) | /*PRQA S 2985*/
                   (SPI_TXCFG_CONT((txCmdCfgSet->contTransfer) ? 1U : 0U)) | /*PRQA S 2985*/
                   (SPI_TXCFG_MSKRX((txCmdCfgSet->rxMask) ? 1U : 0U)) | /*PRQA S 2985*/
                   (SPI_TXCFG_MSKTX((txCmdCfgSet->txMask) ? 1U : 0U))); /*PRQA S 2985*/
}

/*!
 * @brief Select the active chip select (PCS) signal.
 */
void SPI_SetPcs(SPI_Type *base, spi_which_pcs_t whichPcs)
{
    uint32_t regVal;

    regVal = base->TXCFG;
    regVal &= (uint32_t)(~(SPI_TXCFG_PCS_MASK));
    regVal |= (uint32_t)((uint32_t)whichPcs << SPI_TXCFG_PCS_SHIFT);
    base->TXCFG = regVal;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
