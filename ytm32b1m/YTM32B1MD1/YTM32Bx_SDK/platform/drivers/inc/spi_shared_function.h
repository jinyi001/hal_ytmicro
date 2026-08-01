/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file spi_shared_function.h
 * @brief SPI shared types, state structure, and internal function declarations.
 *
 * This header defines common types, enumerations, and the runtime state
 * structure shared between the SPI master and slave drivers. It also
 * declares internal helper functions used by the IRQ handler and
 * transfer routines.
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 3449 Rule 8.5: Multiple declarations of external object or function.
 *
 */

#ifndef SPI_SHARED_FUNCTION_H
#define SPI_SHARED_FUNCTION_H

#include <stdbool.h>
#include "clock_manager.h"
#include "osif.h"
#include "status.h"
#include "callbacks.h"
#if defined(FEATURE_SPI_HAS_DMA_ENABLE) && (FEATURE_SPI_HAS_DMA_ENABLE > 0U)
#include "dma_driver.h"
#endif /* FEATURE_SPI_HAS_DMA_ENABLE */

/*!
 * @addtogroup spi
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @name IRQ Handler Declarations
 * @{
 */

/*!
 * @brief Master mode interrupt handler (implemented in spi_master_driver.c).
 *
 * @param[in] instance  SPI peripheral instance number.
 */
extern void SPI_DRV_MasterIRQHandler(uint32_t instance);

/*!
 * @brief Slave mode interrupt handler (implemented in spi_slave_driver.c).
 *
 * @param[in] instance  SPI peripheral instance number.
 */
extern void SPI_DRV_SlaveIRQHandler(uint32_t instance);

/*@}*/

/*!
 * @name Common Enumerations
 * @{
 */

/*!
 * @brief SPI Peripheral Chip Select (PCS) signal selection.
 *
 * | Value    | Signal |
 * |----------|--------|
 * | SPI_PCS0 | PCS[0] |
 * | SPI_PCS1 | PCS[1] |
 * | SPI_PCS2 | PCS[2] |
 * | SPI_PCS3 | PCS[3] |
 */
typedef enum
{
    SPI_PCS0 = 0U, /*!< PCS[0] */
    SPI_PCS1 = 1U, /*!< PCS[1] */
    SPI_PCS2 = 2U, /*!< PCS[2] */
    SPI_PCS3 = 3U, /*!< PCS[3] */
#if defined(FEATURE_SPI_HAS_EXTERNAL_DEVICES_SELECTION) && (FEATURE_SPI_HAS_EXTERNAL_DEVICES_SELECTION > 4)
    SPI_PCS4 = 4U, /*!< PCS[4] */
    SPI_PCS5 = 5U, /*!< PCS[5] */
    SPI_PCS6 = 6U, /*!< PCS[6] */
    SPI_PCS7 = 7U, /*!< PCS[7] */
#endif
} spi_which_pcs_t;

/*!
 * @brief SPI signal polarity configuration (PCS and Host Request).
 */
typedef enum
{
    SPI_ACTIVE_HIGH = 1U, /*!< Signal is Active High (idles low). */
    SPI_ACTIVE_LOW = 0U   /*!< Signal is Active Low (idles high). */
} spi_signal_polarity_t;

/*!
 * @brief SPI clock phase configuration.
 *
 * Determines the SCK edge on which data is captured and changed.
 */
typedef enum
{
    SPI_CLOCK_PHASE_1ST_EDGE = 0U, /*!< Data captured on SCK 1st edge, changed on 2nd. */
    SPI_CLOCK_PHASE_2ND_EDGE = 1U  /*!< Data changed on SCK 1st edge, captured on 2nd. */
} spi_clock_phase_t;

/*!
 * @brief SPI clock signal (SCK) polarity configuration.
 *
 * Determines the idle state of the SCK signal.
 */
typedef enum
{
    SPI_SCK_ACTIVE_HIGH = 0U, /*!< Signal is Active High (idles low). */
    SPI_SCK_ACTIVE_LOW = 1U   /*!< Signal is Active Low (idles high). */
} spi_sck_polarity_t;

/*!
 * @brief SPI transfer mechanism selection.
 *
 * Selects whether the driver uses interrupt-driven or DMA-driven
 * data movement for SPI transfers.
 */
typedef enum
{
    SPI_USING_DMA = 0,    /*!< The driver will use DMA to perform SPI transfer */
    SPI_USING_INTERRUPTS, /*!< The driver will use interrupts to perform SPI transfer */
} spi_transfer_type;

/*!
 * @brief SPI transfer error status codes.
 *
 * Reported via the spi_state_t::status field to indicate the
 * outcome of the most recent transfer.
 */
typedef enum
{
    SPI_TRANSFER_OK = 0U, /*!< Transfer OK */
    SPI_TRANSMIT_FAIL,    /*!< Error during transmission */
    SPI_RECEIVE_FAIL      /*!< Error during reception */
} transfer_status_t;

/*!
 * @brief SPI transfer width configuration.
 *
 * Controls how many data lines are used for each shift clock cycle.
 *
 * | Value              | Data Lines  | Description              |
 * |--------------------|:-----------:|--------------------------|
 * | SPI_SINGLE_BIT_XFER| 1           | Normal SDI/SDO mode      |
 * | SPI_TWO_BIT_XFER   | 2           | Dual I/O on SDI and SDO  |
 * | SPI_FOUR_BIT_XFER  | 4           | Quad I/O using PCS[3:2]  |
 */
typedef enum
{
    SPI_SINGLE_BIT_XFER = 0U,           /*!< 1-bit shift at a time, data out on SDO, in on SDI (normal mode) */
    SPI_TWO_BIT_XFER = 1U,              /*!< 2-bits shift out on SDO/SDI and in on SDO/SDI */
    SPI_FOUR_BIT_XFER = 2U              /*!< 4-bits shift out on SDO/SDI/PCS[3:2] and in on SDO/SDI/PCS[3:2] */
} spi_transfer_width_t;

/*@}*/

/*!
 * @name Runtime State
 * @{
 */

/*!
 * @brief SPI driver runtime state structure.
 *
 * Holds all per-instance runtime data needed by the SPI master and
 * slave drivers to manage transfers. The user must allocate this
 * structure and pass it to the init function; the driver populates
 * the members internally.
 *
 * @warning This structure must not be modified by the application
 * while a transfer is in progress.
 */
typedef struct
{
    uint16_t bitsPerFrame;              /*!< Number of bits per frame: 8- to 4096-bits; needed for
                                              TCR programming */
    uint16_t bytesPerFrame;             /*!< Number of bytes per frame: 1- to 512-bytes */
    bool isPcsContinuous;               /*!< Option to keep chip select asserted until transfer
                                              complete; needed for TCR programming */
    bool isBlocking;                    /*!< Save the transfer type */
    uint32_t spiSrcClk;                 /*!< Module source clock */
    volatile bool isTransferInProgress; /*!< True if there is an active transfer */
    const uint8_t *txBuff;              /*!< The buffer from which transmitted bytes are taken */
    uint8_t *rxBuff;                    /*!< The buffer into which received bytes are placed */
    volatile uint16_t txCount;          /*!< Number of bytes remaining to send  */
    volatile uint16_t rxCount;          /*!< Number of bytes remaining to receive */
    volatile uint16_t txFrameCnt;       /*!< Number of bytes from current frame which were already sent */
    volatile uint16_t rxFrameCnt;       /*!< Number of bytes from current frame which were already received */
    volatile bool lsb;                  /*!< True if first bit is LSB and false if first bit is MSB */
    uint8_t fifoSize;                   /*!< RX/TX fifo size */
#if FEATURE_SPI_HAS_DMA_ENABLE
    uint8_t rxDMAChannel;               /*!< Channel number for DMA rx channel */
    uint8_t txDMAChannel;               /*!< Channel number for DMA tx channel */
#endif
    spi_transfer_type transferType;     /*!< Type of SPI transfer */
    semaphore_t spiSemaphore;           /*!< The semaphore used for blocking transfers */
    transfer_status_t status;           /*!< The status of the current */
    spi_callback_t callback;            /*!< Select the callback to transfer complete */
    void *callbackParam;                /*!< Select additional callback parameters if it's necessary */
    uint32_t dummy;                     /*!< This field is used for the cases when TX is NULL and SPI is in DMA mode */
} spi_state_t;

/*@}*/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*!
 * @name Global Instance Tables
 * @{
 */

/*!
 * @brief Table of base pointers for SPI peripheral instances.
 */
extern SPI_Type *g_spiBase[SPI_INSTANCE_COUNT];

/*!
 * @brief Table of clock names for SPI instances (used with clock_manager).
 */
extern const clock_names_t s_spiClkNames[SPI_INSTANCE_COUNT];

/*!
 * @brief Table of IRQ numbers for SPI instances.
 */
extern IRQn_Type g_spiIrqId[SPI_INSTANCE_COUNT];

/*!
 * @brief Pointer to the runtime state structure for each SPI instance.
 */
extern spi_state_t *g_spiStatePtr[SPI_INSTANCE_COUNT];

/*@}*/

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/*!
 * @name Internal Transfer Helpers
 * These functions are used internally by the master/slave drivers
 * and IRQ handlers. They are not intended for direct application use.
 * @{
 */

/*!
 * @brief Dispatch SPI IRQ to the appropriate master or slave handler.
 *
 * Checks the master/slave mode of the given SPI instance and
 * dispatches the interrupt to SPI_DRV_MasterIRQHandler() or
 * SPI_DRV_SlaveIRQHandler() accordingly.
 *
 * @param[in] instance  SPI peripheral instance number.
 */
void SPI_DRV_IRQHandler(uint32_t instance);

/*!
 * @brief Fill the TX FIFO from the software transmit buffer.
 *
 * Writes data from the spi_state_t transmit buffer into the hardware
 * TX FIFO, handling frame-size packing and continuous mode logic.
 *
 * @param[in] instance  SPI peripheral instance number.
 */
void SPI_DRV_FillupTxBuffer(uint32_t instance);

/*!
 * @brief Read data from the RX FIFO into the software receive buffer.
 *
 * Transfers all available words from the hardware RX FIFO into the
 * spi_state_t receive buffer, handling frame-size unpacking.
 *
 * @param[in] instance  SPI peripheral instance number.
 */
void SPI_DRV_ReadRXBuffer(uint32_t instance);

/*!
 * @brief Disable transmit/receive error interrupts and clear error flags.
 *
 * Called at the end of a transfer to clean up error interrupt state.
 *
 * @param[in] instance  SPI peripheral instance number.
 */
void SPI_DRV_DisableTEIEInterrupts(uint32_t instance);

/*@}*/

/*! @} */

#endif /* __SPI_SHARED_FUNCTION_H__*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
