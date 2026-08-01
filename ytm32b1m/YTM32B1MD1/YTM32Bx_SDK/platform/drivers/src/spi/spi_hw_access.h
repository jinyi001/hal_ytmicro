/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file spi_hw_access.h
 * @brief SPI hardware register access layer.
 *
 * This header provides low-level inline and non-inline functions for
 * direct SPI peripheral register manipulation. It is used internally
 * by the SPI master/slave driver layers and should not be called
 * directly by application code.
 */

#ifndef SPI_HW_ACCESS_H
#define SPI_HW_ACCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "status.h"
#include "device_registers.h"
#include "spi_shared_function.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Prescaler values for the SPI baud rate clock source.
 *
 * These values are programmed into the TCR PRESCALE field and applied
 * as a power-of-two divider to the module source clock before the
 * SCK divider stage.
 *
 * | Value      | Divide Ratio |
 * |------------|:------------:|
 * | SPI_DIV_1  | 1            |
 * | SPI_DIV_2  | 2            |
 * | SPI_DIV_4  | 4            |
 * | SPI_DIV_8  | 8            |
 * | SPI_DIV_16 | 16           |
 * | SPI_DIV_32 | 32           |
 * | SPI_DIV_64 | 64           |
 * | SPI_DIV_128| 128          |
 */
typedef enum
{
    SPI_DIV_1 = 0U,
    SPI_DIV_2 = 1U,
    SPI_DIV_4 = 2U,
    SPI_DIV_8 = 3U,
    SPI_DIV_16 = 4U,
    SPI_DIV_32 = 5U,
    SPI_DIV_64 = 6U,
    SPI_DIV_128 = 7U,
} spi_prescaler_t;

/*!
 * @brief SPI status and interrupt flag identifiers.
 *
 * Used with SPI_GetStatusFlag(), SPI_ClearStatusFlag(), and
 * SPI_SetIntMode() to query, clear, or enable/disable individual
 * status flags and their corresponding interrupt sources.
 *
 * | Flag                   | Bit   | W1C | Description                    |
 * |------------------------|:-----:|:---:|--------------------------------|
 * | SPI_RX_DATA_FLAG       | 0     | No  | RX data available              |
 * | SPI_TX_DATA_FLAG       | 1     | No  | TX FIFO ready for data         |
 * | SPI_WORD_COMPLETE      | 2     | Yes | One word transfer completed    |
 * | SPI_FRAME_COMPLETE     | 3     | Yes | One frame transfer completed   |
 * | SPI_TRANSFER_COMPLETE  | 4     | Yes | Entire transfer completed      |
 * | SPI_TRANSMIT_ERROR     | 5     | Yes | TX FIFO underrun               |
 * | SPI_RECEIVE_ERROR      | 6     | Yes | RX FIFO overrun                |
 * | SPI_DATA_MATCH         | 7     | Yes | Received data matches pattern  |
 * | SPI_MODULE_BUSY        | 0x12  | No  | Module busy with transfer      |
 * | SPI_ALL_STATUS         | 0xFC  | —   | Mask for clearing all W1C flags|
 */
typedef enum
{
    SPI_RX_DATA_FLAG = 0x0,             /*!< RX data flag */
    SPI_TX_DATA_FLAG = 0x1,             /*!< TX data flag */
    SPI_WORD_COMPLETE = 0x2,             /*!< Word Complete flag */
    SPI_FRAME_COMPLETE = 0x3,             /*!< Frame Complete flag */
    SPI_TRANSFER_COMPLETE = 0x4,             /*!< Transfer Complete flag */
    SPI_TRANSMIT_ERROR = 0x5,             /*!< Transmit Error flag (FIFO underrun) */
    SPI_RECEIVE_ERROR = 0x6,             /*!< Receive Error flag (FIFO overrun) */
    SPI_DATA_MATCH = 0x7,             /*!< Data Match flag */
    SPI_MODULE_BUSY = 0x12,            /*!< Module Busy flag */
    SPI_ALL_STATUS = 0x000000FC       /*!< Used for clearing all w1c status flags */
} spi_status_flag_t;

/*!
 * @brief SPI master or slave mode selection.
 */
typedef enum
{
    SPI_MASTER = 1U,     /*!< SPI peripheral operates in master mode. */
    SPI_SLAVE = 0U      /*!< SPI peripheral operates in slave mode. */
} spi_master_slave_mode_t;

/*!
 * @brief SPI SDO/SDI pin routing configuration.
 *
 * | Value               | SDI Pin      | SDO Pin      |
 * |---------------------|:------------:|:------------:|
 * | SPI_SDI_IN_SDO_OUT  | Input        | Output       |
 * | SPI_SDI_IN_OUT      | Input+Output | —            |
 * | SPI_SDO_IN_OUT      | —            | Input+Output |
 * | SPI_SDI_OUT_SDO_IN  | Output       | Input        |
 */
typedef enum
{
    SPI_SDI_IN_SDO_OUT = 0U,     /*!< SPI SDI input, SDO output. */
    SPI_SDI_IN_OUT = 1U,     /*!< SDI is used for both input and output data. */
    SPI_SDO_IN_OUT = 2U,     /*!< SDO is used for both input and output data. */
    SPI_SDI_OUT_SDO_IN = 3U      /*!< SPI SDO input, SDI output. */
} spi_pin_config_t;

/*!
 * @brief SPI data output behavior when chip select is de-asserted.
 */
typedef enum
{
    SPI_DATA_OUT_RETAINED = 0U, /*!< Data out retains last value when chip select de-asserted */
    SPI_DATA_OUT_TRISTATE = 1U  /*!< Data out is tri-stated when chip select de-asserted */
} spi_data_out_config_t;

/*!
 * @brief Transmit Command Register (TCR) configuration structure.
 *
 * Groups all parameters that affect the SPI transmission behavior.
 * Writing to the TCR pushes the entire register contents to the TX FIFO,
 * so all fields should be configured before writing.
 */
typedef struct
{
    uint32_t frameSize;              /*!< Number of bits/frame, minimum is 8-bits. */
    spi_transfer_width_t width;    /*!< Transfer width, single, 2-bit, or 4-bit transfer. */
    bool txMask;                     /*!< Option to mask the transmit data (won't load to FIFO). */
    bool rxMask;                     /*!< Option to mask the receive data (won't store in FIFO). */
    bool contCmd;                    /*!< Master option to change cmd word within cont transfer. */
    bool contTransfer;               /*!< Master option for continuous transfer. */
    bool byteSwap;                   /*!< Option to invoke the byte swap option in the FIFOs. */
    bool lsbFirst;                   /*!< Option to transmit LSB first. */
    spi_which_pcs_t whichPcs;      /*!< Selects which PCS to use. */
    uint32_t preDiv;                /*!< Selects the baud rate prescaler divider TCR bit setting. */
    spi_clock_phase_t clkPhase;    /*!< Selects which phase of clock to capture data. */
    spi_sck_polarity_t clkPolarity; /*!< Selects clock polarity. */
} spi_tx_cmd_config_t;

/*!
 * @brief SPI module initialization configuration structure.
 *
 * Contains basic parameters needed for initial SPI module setup,
 * including source clock frequency, desired baud rate, master/slave
 * mode, and PCS polarity.
 */
typedef struct
{
    uint32_t spiSrcClk;                /*!< SPI module clock */
    uint32_t baudRate;                   /*!< SPI baudrate */
    spi_master_slave_mode_t spiMode; /*!< SPI master/slave mode */
    spi_signal_polarity_t pcsPol;      /*!< SPI PCS polarity */
} spi_init_config_t;

/*!
 * @brief SPI timing delay type selection.
 *
 * Used with SPI_SetDelay() to configure one of the three
 * configurable timing parameters.
 */
typedef enum
{
    SPI_SCK_TO_PCS = SPI_CLK_SCKPCS_SHIFT,     /*!< SCK to PCS Delay */
    SPI_PCS_TO_SCK = SPI_CLK_PCSSCK_SHIFT,     /*!< PCS to SCK Delay */
    SPI_BETWEEN_TRANSFER = SPI_CLK_FMDLY_SHIFT  /*!< Delay between transfers */
} spi_delay_type_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Defines constant value arrays for the baud rate pre-scalar values.*/
static const uint32_t s_baudratePrescaler[] = {1, 2, 4, 8, 16, 32, 64, 128};

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name Module Control
 * Functions for SPI module initialization, enable/disable, and mode
 * selection.
 * @{
 */

/*!
 * @brief Reset the SPI module to its default state.
 *
 * Performs a software reset of the SPI internal logic and clears all
 * registers to their default values. The module is left disabled
 * after this call.
 *
 * @param[in] base  SPI peripheral base pointer.
 */
void SPI_Init(SPI_Type *base);

/*!
 * @brief Enable the SPI module.
 *
 * @param[in] base  SPI peripheral base pointer.
 */
static inline void SPI_Enable(SPI_Type *base)
{
    (base->CTRL) |= SPI_CTRL_EN_MASK;
}

/*!
 * @brief Disable the SPI module.
 *
 * Checks the Module Busy Flag (MBF) before disabling. If the module
 * is currently busy, returns STATUS_BUSY without disabling.
 *
 * @param[in] base  SPI peripheral base pointer.
 * @return STATUS_SUCCESS if disabled successfully, STATUS_BUSY if
 *         the module is currently processing a transfer.
 */
status_t SPI_Disable(SPI_Type *base);

/*!
 * @brief Configure the SPI for master or slave mode.
 *
 * @param[in] base  SPI peripheral base pointer.
 * @param[in] mode  Master or slave mode selection.
 * @return STATUS_SUCCESS on success.
 *
 * @pre The SPI module must be disabled before calling this function.
 */
status_t SPI_SetMasterSlaveMode(SPI_Type *base, spi_master_slave_mode_t mode);

/*!
 * @brief Check if the SPI module is configured as master.
 *
 * @param[in] base  SPI peripheral base pointer.
 * @return true if master mode, false if slave mode.
 */
static inline bool SPI_IsMaster(const SPI_Type *base)
{
    return (((base->CTRL >> SPI_CTRL_MODE_SHIFT) & 1U) != 0U);
}

/*@}*/

#if !defined(FEATURE_SPI_LITE_VERSION)
/*!
 * @name FIFO Management
 * Functions for querying FIFO sizes, flushing FIFOs, and configuring
 * watermark thresholds. Not available on FEATURE_SPI_LITE_VERSION.
 * @{
 */

/*!
 * @brief Get the hardware FIFO depth of the SPI module.
 *
 * Reads the FIFO size field from the TXFIFO register and returns
 * the depth as a power-of-two value.
 *
 * @param[in]  base      SPI peripheral base pointer.
 * @param[out] fifoSize  Pointer to store the FIFO depth (in words).
 */
static inline void SPI_GetFifoSizes(const SPI_Type *base, uint8_t *fifoSize)
{
    if (fifoSize != NULL)
    {
        *fifoSize = (uint8_t) (1U << ((base->TXFIFO & SPI_TXFIFO_SIZE_MASK) >> SPI_TXFIFO_SIZE_SHIFT));
    }
}

/*!
 * @brief Flush the SPI TX and/or RX FIFOs.
 *
 * @param[in] base        SPI peripheral base pointer.
 * @param[in] flushTxFifo true to flush the TX FIFO, false to leave unchanged.
 * @param[in] flushRxFifo true to flush the RX FIFO, false to leave unchanged.
 */
void SPI_SetFlushFifoCmd(SPI_Type *base, bool flushTxFifo, bool flushRxFifo);

/*!
 * @brief Set the RX FIFO watermark threshold.
 *
 * An RX data flag is asserted when the number of words in the RX FIFO
 * exceeds this watermark value.
 *
 * @param[in] base     SPI peripheral base pointer.
 * @param[in] rxWater  RX FIFO watermark value.
 */
static inline void SPI_SetRxWatermarks(SPI_Type *base, uint32_t rxWater)
{
    uint32_t spi_tmp = base->RXFIFO;
    spi_tmp &= ~(SPI_RXFIFO_WATER_MASK);
    spi_tmp |= (rxWater << SPI_RXFIFO_WATER_SHIFT);
    base->RXFIFO = spi_tmp;
}

/*!
 * @brief Set the TX FIFO watermark threshold.
 *
 * A TX data flag is asserted when the number of words in the TX FIFO
 * is less than or equal to this watermark value.
 *
 * @param[in] base     SPI peripheral base pointer.
 * @param[in] txWater  TX FIFO watermark value.
 */
static inline void SPI_SetTxWatermarks(SPI_Type *base, uint32_t txWater)
{
    uint32_t spi_tmp = base->TXFIFO;
    spi_tmp &= ~(SPI_TXFIFO_WATER_MASK);
    spi_tmp |= (txWater << SPI_TXFIFO_WATER_SHIFT);
    base->TXFIFO = spi_tmp;
}

/*@}*/
#endif /* !defined(FEATURE_SPI_LITE_VERSION) */

/*!
 * @name Status and Interrupt
 * Functions for reading/clearing status flags and enabling/disabling
 * interrupt sources.
 * @{
 */

/*!
 * @brief Get the state of a specific SPI status flag.
 *
 * @param[in] base        SPI peripheral base pointer.
 * @param[in] statusFlag  Status flag to query.
 * @return true if the flag is asserted, false otherwise.
 */
static inline bool SPI_GetStatusFlag(const SPI_Type *base,
                                     spi_status_flag_t statusFlag)
{
    return ((((base->STS) >> (uint8_t) statusFlag) & 1U) != 0U);
}

/*!
 * @brief Clear a write-1-to-clear (W1C) status flag.
 *
 * Only W1C-capable flags can be cleared: SPI_WORD_COMPLETE,
 * SPI_FRAME_COMPLETE, SPI_TRANSFER_COMPLETE, SPI_TRANSMIT_ERROR,
 * SPI_RECEIVE_ERROR, SPI_DATA_MATCH. Pass SPI_ALL_STATUS to clear
 * all W1C flags at once.
 *
 * @param[in] base        SPI peripheral base pointer.
 * @param[in] statusFlag  Status flag to clear, or SPI_ALL_STATUS.
 * @return STATUS_SUCCESS on success.
 */
status_t SPI_ClearStatusFlag(SPI_Type *base, spi_status_flag_t statusFlag);

/*!
 * @brief Enable or disable a specific SPI interrupt source.
 *
 * @param[in] base          SPI peripheral base pointer.
 * @param[in] interruptSrc  Interrupt source (uses spi_status_flag_t values).
 * @param[in] enable        true to enable, false to disable.
 */
static inline void SPI_SetIntMode(SPI_Type *base,
                                  spi_status_flag_t interruptSrc, bool enable)
{
    if (enable == true)
    {
        base->INTE |= (uint32_t) 1U << (uint8_t) interruptSrc;
    } else
    {
        base->INTE &= ~((uint32_t) 1U << (uint8_t) interruptSrc);
    }
}

/*!
 * @brief Check if a specific SPI interrupt source is enabled.
 *
 * @param[in] base          SPI peripheral base pointer.
 * @param[in] interruptSrc  Interrupt source to query.
 * @return true if enabled, false if disabled.
 */
static inline bool SPI_GetIntMode(const SPI_Type *base,
                                  spi_status_flag_t interruptSrc)
{
    return ((((base->INTE) >> (uint8_t) interruptSrc) & 1U) != 0U);
}

/*@}*/

#if !defined(FEATURE_SPI_LITE_VERSION)
/*!
 * @name DMA Configuration
 * Functions for enabling/disabling DMA requests for TX and RX paths.
 * Not available on FEATURE_SPI_LITE_VERSION.
 * @{
 */

/*!
 * @brief Enable or disable the TX DMA request.
 *
 * When enabled, the SPI module generates a DMA request whenever
 * the TX FIFO watermark condition is met.
 *
 * @param[in] base    SPI peripheral base pointer.
 * @param[in] enable  true to enable TX DMA, false to disable.
 */
static inline void SPI_SetTxDmaCmd(SPI_Type *base, bool enable)
{
    base->CTRL = (base->CTRL & (~SPI_CTRL_TXDMAEN_MASK)) | (enable ? SPI_CTRL_TXDMAEN_MASK : 0U);
}

/*!
 * @brief Enable or disable the RX DMA request.
 *
 * When enabled, the SPI module generates a DMA request whenever
 * the RX FIFO watermark condition is met.
 *
 * @param[in] base    SPI peripheral base pointer.
 * @param[in] enable  true to enable RX DMA, false to disable.
 */
static inline void SPI_SetRxDmaCmd(SPI_Type *base, bool enable)
{
    (base->CTRL) = (base->CTRL & (~SPI_CTRL_RXDMAEN_MASK)) | (enable ? SPI_CTRL_RXDMAEN_MASK : 0U);
}

/*@}*/
#endif /* !defined(FEATURE_SPI_LITE_VERSION) */

/*!
 * @name Clock and Baud Rate
 * Functions for configuring SCK frequency, prescaler, and timing
 * delays between transfers and chip-select edges.
 * @{
 */

/*!
 * @brief Configure a specific SPI timing delay.
 *
 * Sets one of the three configurable delay parameters:
 * - SCK to PCS delay: delay + 1 SCK cycles
 * - PCS to SCK delay: delay + 1 SCK cycles
 * - Between-transfer delay: delay + 2 SCK cycles
 *
 * @param[in] base        SPI peripheral base pointer.
 * @param[in] whichDelay  Delay type to configure.
 * @param[in] delay       8-bit delay value (0x00–0xFF).
 * @return STATUS_SUCCESS.
 *
 * @pre The SPI module must be disabled and in master mode.
 */
static inline status_t SPI_SetDelay(SPI_Type *base, spi_delay_type_t whichDelay, uint32_t delay)
{
    uint32_t ccrValue = 0;

    ccrValue = base->CLK & ~(0xFFUL << (uint32_t) whichDelay);
    ccrValue |= delay << (uint32_t) whichDelay;
    base->CLK = ccrValue;
    return STATUS_SUCCESS;
}

/*!
 * @brief Calculate and set the SPI baud rate.
 *
 * Finds the best prescaler and divider combination to achieve a baud
 * rate as close as possible to the requested value. Also configures
 * default timing delay values based on the calculated divider.
 *
 * @param[in]  base             SPI peripheral base pointer.
 * @param[in]  bitsPerSec       Desired baud rate in bits per second.
 * @param[in]  sourceClockInHz  SPI module source clock frequency in Hz.
 * @param[out] tcrPrescaleValue Pointer to store the calculated TCR
 *                              PRESCALE value for later TCR programming.
 * @return The actual calculated baud rate in bits per second, or 0
 *         on error.
 *
 * @pre The SPI module must be disabled and in master mode.
 */
uint32_t SPI_SetBaudRate(SPI_Type *base, uint32_t bitsPerSec,
                         uint32_t sourceClockInHz, uint32_t *tcrPrescaleValue);

/*!
 * @brief Manually set the SCK clock divider value.
 *
 * Directly programs the CLK[SCKDIV] field without automatic
 * prescaler calculation. Use when the divider value is already known.
 *
 * @param[in] base     SPI peripheral base pointer.
 * @param[in] divisor  Baud rate divisor (0x00–0xFF).
 * @return STATUS_SUCCESS on success.
 *
 * @pre The SPI module must be disabled and in master mode.
 */
status_t SPI_SetBaudRateDivisor(SPI_Type *base, uint32_t divisor);

#if !defined(FEATURE_SPI_LITE_VERSION)
/*!
 * @brief Set the clock prescaler for master mode.
 *
 * Configures the PRESCALE field in the Transmit Command Register (TCR).
 *
 * @param[in] base       SPI peripheral base pointer.
 * @param[in] prescaler  Prescaler divider value.
 */
static inline void SPI_SetClockPrescaler(SPI_Type *base, spi_prescaler_t prescaler)
{
    uint32_t spi_tmp = base->TXCFG;
    spi_tmp &= ~(SPI_TXCFG_PRESCALE_MASK);
    spi_tmp |= ((uint32_t) prescaler << SPI_TXCFG_PRESCALE_SHIFT);
    base->TXCFG = spi_tmp;
}

/*!
 * @brief Get the current clock prescaler value.
 *
 * @param[in] base  SPI peripheral base pointer.
 * @return Current prescaler divider value.
 */
static inline spi_prescaler_t SPI_GetClockPrescaler(const SPI_Type *base)
{
    uint32_t prescalerValue = (((base->TXCFG) & SPI_TXCFG_PRESCALE_MASK) >> SPI_TXCFG_PRESCALE_SHIFT);
    spi_prescaler_t prescaler = (spi_prescaler_t)prescalerValue;
    return prescaler;
}
#endif /* !FEATURE_SPI_LITE_VERSION */

/*@}*/

/*!
 * @name Bus Configuration
 * Functions for configuring PCS polarity, pin routing, chip select
 * selection, and sampling point.
 * @{
 */

/*!
 * @brief Set the polarity of a specific PCS signal.
 *
 * @param[in] base         SPI peripheral base pointer.
 * @param[in] whichPcs     PCS signal to configure.
 * @param[in] pcsPolarity  Active-high or active-low polarity.
 * @return STATUS_SUCCESS on success.
 *
 * @pre The SPI module must be disabled.
 */
status_t SPI_SetPcsPolarityMode(SPI_Type *base, spi_which_pcs_t whichPcs,
                                spi_signal_polarity_t pcsPolarity);

/*!
 * @brief Configure the SDO/SDI pin routing and output behavior.
 *
 * Configures three aspects of the SPI pin behavior:
 * - SDI/SDO pin direction (input/output assignment).
 * - Data output state when PCS is de-asserted (retained or tri-state).
 * - PCS[3:2] pin enable for 4-bit transfer support.
 *
 * @param[in] base            SPI peripheral base pointer.
 * @param[in] pinCfg          SDO/SDI pin configuration.
 * @param[in] dataOutConfig   Data output behavior after PCS de-assertion.
 * @param[in] pcs3and2Enable  true to enable PCS[3:2] as chip selects,
 *                            false to use them as data I/O for 4-bit mode.
 * @return STATUS_SUCCESS on success.
 *
 * @pre The SPI module must be disabled.
 */
status_t SPI_SetPinConfigMode(SPI_Type *base,
                              spi_pin_config_t pinCfg,
                              spi_data_out_config_t dataOutConfig,
                              bool pcs3and2Enable);

/*!
 * @brief Select the active chip select (PCS) signal.
 *
 * Updates the PCS field in the Transmit Command Register.
 *
 * @param[in] base      SPI peripheral base pointer.
 * @param[in] whichPcs  Chip select signal to activate.
 */
void SPI_SetPcs(SPI_Type *base, spi_which_pcs_t whichPcs);

/*!
 * @brief Configure the sampling point for master mode.
 *
 * When enabled, the data sampling point is delayed by half a SCK
 * cycle, which can help with signal integrity at higher frequencies.
 *
 * @param[in] base                   SPI peripheral base pointer.
 * @param[in] isSamplingPointDelayed true to delay the sampling point.
 */
static inline void SPI_SetSamplingPoint(SPI_Type *base, bool isSamplingPointDelayed)
{
    uint32_t spi_tmp = base->CTRL;
    spi_tmp &= ~(SPI_CTRL_SPDEN_MASK);
    spi_tmp |= (isSamplingPointDelayed ? SPI_CTRL_SPDEN(1U) : 0U);
    base->CTRL = spi_tmp;
}

/*@}*/

/*!
 * @name Transmit Command and Mask Control
 * Functions for configuring the Transmit Command Register (TCR) and
 * controlling TX/RX masking and continuous transfer mode bits.
 * @{
 */

/*!
 * @brief Write the full Transmit Command Register (TCR).
 *
 * Configures all TCR parameters including clock phase/polarity, PCS
 * selection, frame size, prescaler, and mask/continuous bits. Writing
 * to TCR pushes the entire register to the TX FIFO, so all fields
 * must be set before calling this function.
 *
 * @param[in] base         SPI peripheral base pointer.
 * @param[in] txCmdCfgSet  Pointer to the TCR configuration structure.
 */
void SPI_SetTxCommandReg(SPI_Type *base, const spi_tx_cmd_config_t *txCmdCfgSet);

/*!
 * @brief Clear the RX mask bit in the TCR (enable RX data storage).
 *
 * @param[in] base  SPI peripheral base pointer.
 */
static inline void SPI_ClearRxmaskBit(SPI_Type *base)
{
    (base->TXCFG) = ((base->TXCFG) & (~SPI_TXCFG_MSKRX_MASK));
}

/*!
 * @brief Set the RX mask bit in the TCR (discard received data).
 *
 * @param[in] base  SPI peripheral base pointer.
 */
static inline void SPI_SetRxmskBit(SPI_Type *base)
{
    (base->TXCFG) = ((base->TXCFG) | (SPI_TXCFG_MSKRX_MASK));
}

/*!
 * @brief Clear the TX mask bit in the TCR (enable normal TX).
 *
 * @param[in] base  SPI peripheral base pointer.
 */
static inline void SPI_ClearTxmaskBit(SPI_Type *base)
{
    (base->TXCFG) = ((base->TXCFG) & (~SPI_TXCFG_MSKTX_MASK));
}

/*!
 * @brief Set the TX mask bit in the TCR (mask TX data output).
 *
 * @param[in] base  SPI peripheral base pointer.
 */
static inline void SPI_SetTxmskBit(SPI_Type *base)
{
    (base->TXCFG) = ((base->TXCFG) | (SPI_TXCFG_MSKTX_MASK));
}

/*!
 * @brief Clear the CONT bit to end continuous PCS assertion.
 *
 * @param[in] base  SPI peripheral base pointer.
 */
static inline void SPI_ClearContBit(SPI_Type *base)
{
    (base->TXCFG) = ((base->TXCFG) & (~SPI_TXCFG_CONT_MASK));
}

/*!
 * @brief Set the CONT bit for continuous PCS assertion.
 *
 * @param[in] base  SPI peripheral base pointer.
 */
static inline void SPI_SetContBit(SPI_Type *base)
{
    (base->TXCFG) = ((base->TXCFG) | (SPI_TXCFG_CONT_MASK));
}

#if !defined(FEATURE_SPI_LITE_VERSION)
/*!
 * @brief Clear the CONTC bit (disable command change in continuous mode).
 *
 * @param[in] base  SPI peripheral base pointer.
 */
static inline void SPI_ClearContCBit(SPI_Type *base)
{
    (base->TXCFG) = ((base->TXCFG) & (~SPI_TXCFG_CONTC_MASK));
}

/*!
 * @brief Set the CONTC bit (enable command change in continuous mode).
 *
 * @param[in] base  SPI peripheral base pointer.
 */
static inline void SPI_SetContCBit(SPI_Type *base)
{
    (base->TXCFG) = ((base->TXCFG) | (SPI_TXCFG_CONTC_MASK));
}
#endif /* !FEATURE_SPI_LITE_VERSION */

/*@}*/

/*!
 * @name Data Transfer
 * Functions for writing data to the TX FIFO, reading data from the
 * RX FIFO, and querying FIFO fill levels.
 * @{
 */

/*!
 * @brief Write a data word to the TX FIFO.
 *
 * The written word is immediately pushed into the TX FIFO. For frames
 * exceeding 32 bits, data must be written one 32-bit word at a time.
 *
 * @param[in] base  SPI peripheral base pointer.
 * @param[in] data  32-bit data word to transmit.
 */
static inline void SPI_WriteData(SPI_Type *base, uint32_t data)
{
    base->DATA = data;
}

/*!
 * @brief Read a data word from the RX FIFO.
 *
 * @param[in] base  SPI peripheral base pointer.
 * @return 32-bit data word received from the SPI bus.
 */
static inline uint32_t SPI_ReadData(const SPI_Type *base)
{
    return (uint32_t) base->DATA;
}

#if !defined(FEATURE_SPI_LITE_VERSION)
/*!
 * @brief Get the number of words currently in the TX FIFO.
 *
 * @param[in] base  SPI peripheral base pointer.
 * @return TX FIFO word count.
 */
static inline uint32_t SPI_ReadTxCount(const SPI_Type *base)
{
    return (uint32_t) (((uint32_t) (base->TXFIFO & SPI_TXFIFO_COUNT_MASK)) >> SPI_TXFIFO_COUNT_SHIFT);
}

/*!
 * @brief Get the number of words currently in the RX FIFO.
 *
 * @param[in] base  SPI peripheral base pointer.
 * @return RX FIFO word count.
 */
static inline uint32_t SPI_ReadRxCount(const SPI_Type *base)
{
    return (uint32_t) ((((uint32_t) base->RXFIFO & (uint32_t) SPI_RXFIFO_COUNT_MASK))
        >> (uint32_t) SPI_RXFIFO_COUNT_SHIFT);
}
#endif /* !defined(FEATURE_SPI_LITE_VERSION) */

/*@}*/

#ifdef SPI_CTRL_I2SMOD_MASK
/*!
 * @name I2S Mode
 * Functions for querying and configuring I2S mode on supported devices.
 * @{
 */

/*!
 * @brief Check if I2S mode is enabled.
 *
 * @param[in] base  SPI peripheral base pointer.
 * @return true if I2S mode is enabled, false otherwise.
 */
static inline bool SPI_IsI2SMOD(const SPI_Type *base)
{
    return (base->CTRL & SPI_CTRL_I2SMOD_MASK) != 0U;
}

/*!
 * @brief Enable or disable I2S mode.
 *
 * @param[in] base      SPI peripheral base pointer.
 * @param[in] isI2SMOD  true to enable I2S mode, false to disable.
 */
static inline void SPI_SetI2SMOD(SPI_Type *base, bool isI2SMOD)
{
    uint32_t spi_tmp = base->CTRL;
    spi_tmp &= ~(SPI_CTRL_I2SMOD_MASK);
    spi_tmp |= (isI2SMOD ? SPI_CTRL_I2SMOD(1U) : 0U);
    base->CTRL = spi_tmp;
}

/*@}*/
#endif

#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* SPI_HW_ACCESS_H*/

/*******************************************************************************
 * EOF
 ******************************************************************************/
