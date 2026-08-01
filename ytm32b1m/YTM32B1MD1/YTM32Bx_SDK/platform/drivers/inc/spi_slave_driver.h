/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file spi_slave_driver.h
 * @brief SPI slave mode driver API declarations.
 *
 * Provides initialization and data transfer functions for operating
 * the SPI peripheral in slave mode. Supports interrupt-driven
 * (blocking and non-blocking) and DMA-based transfer mechanisms.
 */

/*!
 * @page misra_violations MISRA-C:2012 violations list
 *
 * PRQA S 3449 Rule 8.5: Multiple declarations of external object or function.Declarations.
 *
 */

#ifndef SPI_SLAVE_DRIVER_H
#define SPI_SLAVE_DRIVER_H

#include "spi_shared_function.h"

/*!
 * @addtogroup spi
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief SPI slave mode configuration structure.
 *
 * Contains all user-configurable parameters for slave mode operation.
 * Pass this structure to SPI_DRV_SlaveInit() to configure the SPI
 * slave instance.
 */
typedef struct
{
    spi_signal_polarity_t pcsPolarity;      /*!< PCS polarity */
    uint16_t bitcount;                      /*!< Number of bits/frame, minimum is 8-bits */
    spi_clock_phase_t clkPhase;             /*!< Selects which phase of clock to capture data */
    spi_which_pcs_t whichPcs;               /*!< Selects which pcs */
    spi_sck_polarity_t clkPolarity;         /*!< Selects clock polarity */
    bool lsbFirst;                          /*!< Option to transmit LSB first */
    spi_transfer_type transferType;         /*!< Type of SPI transfer */
    uint8_t rxDMAChannel;                   /*!< Channel number for DMA rx channel. If DMA mode isn't used this field will be ignored. */
    uint8_t txDMAChannel;                   /*!< Channel number for DMA tx channel. If DMA mode isn't used this field will be ignored. */
    spi_callback_t callback;                /*!< Select the callback to transfer complete */
    void *callbackParam;                    /*!< Select additional callback parameters if it's necessary */
    spi_transfer_width_t width;             /*!< Transfer width, single, 2-bit, or 4-bit transfer. */
} spi_slave_config_t;


/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name Initialization and De-initialization
 * @{
 */

/*!
 * @brief Fill the slave configuration structure with default values.
 *
 * Populates all fields of the spi_slave_config_t structure with safe
 * defaults: 8-bit frames, PCS0, interrupt mode.
 *
 * @param[out] spiConfig  Pointer to the configuration structure to fill.
 */
void SPI_DRV_SlaveGetDefaultConfig(spi_slave_config_t *spiConfig);

/*!
 * @brief Initialize the SPI instance in slave mode.
 *
 * Resets the SPI module, configures it for slave operation, enables
 * interrupts, and activates the module.
 *
 * @param[in] instance     SPI peripheral instance number.
 * @param[in] spiState     Pointer to the runtime state structure
 *                         (user-allocated, driver-populated).
 * @param[in] slaveConfig  Pointer to the slave configuration.
 * @return STATUS_SUCCESS on success, error code on failure.
 *
 * @pre The SPI peripheral clock must be enabled via clock_manager.
 * @post The SPI module is enabled and ready for transfers.
 */
status_t SPI_DRV_SlaveInit(uint32_t instance,
                           spi_state_t *spiState,
                           const spi_slave_config_t *slaveConfig);

/*!
 * @brief De-initialize the SPI slave instance.
 *
 * Resets the SPI module, disables its interrupt, destroys the
 * semaphore, and clears the state pointer.
 *
 * @param[in] instance  SPI peripheral instance number.
 * @return STATUS_SUCCESS on success, STATUS_ERROR if busy.
 *
 * @pre No transfer should be in progress.
 * @post The SPI module is disabled and its state is cleared.
 */
status_t SPI_DRV_SlaveDeinit(uint32_t instance);

/*@}*/

/*!
 * @name Data Transfer
 * @{
 */

/*!
 * @brief Perform an interrupt-driven blocking slave transfer.
 *
 * Prepares the TX and RX buffers and waits for the master to drive
 * the clock. The function blocks until all data is transferred or
 * the timeout expires.
 *
 * @param[in]  instance          SPI peripheral instance number.
 * @param[in]  sendBuffer        Pointer to TX data (NULL to skip TX).
 * @param[out] receiveBuffer     Pointer to RX buffer (NULL to skip RX).
 * @param[in]  transferByteCount Number of bytes to transfer.
 * @param[in]  timeout           Timeout in milliseconds.
 * @return STATUS_SUCCESS on success, STATUS_BUSY if already
 *         transferring, STATUS_TIMEOUT if timed out.
 *
 * @note The transferByteCount must be a multiple of bytes-per-frame.
 */
status_t SPI_DRV_SlaveTransferBlocking(uint32_t instance,
                                       const uint8_t *sendBuffer,
                                       uint8_t *receiveBuffer,
                                       uint16_t transferByteCount,
                                       uint32_t timeout);

/*!
 * @brief Start a non-blocking slave transfer.
 *
 * Prepares the TX and RX buffers and returns immediately. The actual
 * data transfer occurs when the master drives the clock. Use
 * SPI_DRV_SlaveGetTransferStatus() to poll for completion, or
 * register a callback via the configuration structure.
 *
 * @param[in]  instance          SPI peripheral instance number.
 * @param[in]  sendBuffer        Pointer to TX data (NULL to skip TX).
 * @param[out] receiveBuffer     Pointer to RX buffer (NULL to skip RX).
 * @param[in]  transferByteCount Number of bytes to transfer.
 * @return STATUS_SUCCESS on success, STATUS_BUSY if already
 *         transferring, STATUS_ERROR on failure.
 *
 * @note The transferByteCount must be a multiple of bytes-per-frame.
 */
status_t SPI_DRV_SlaveTransfer(uint32_t instance,
                               const uint8_t *sendBuffer,
                               uint8_t *receiveBuffer,
                               uint16_t transferByteCount);

/*@}*/

/*!
 * @name Transfer Management
 * @{
 */

/*!
 * @brief Abort an in-progress slave transfer.
 *
 * Stops the current transfer, disables interrupts/DMA, and resets
 * the transfer state.
 *
 * @param[in] instance  SPI peripheral instance number.
 * @return STATUS_SUCCESS.
 */
status_t SPI_DRV_SlaveAbortTransfer(uint32_t instance);

/*!
 * @brief Query the status of an ongoing slave transfer.
 *
 * Returns whether the transfer is still in progress or has completed,
 * and optionally reports the number of bytes remaining.
 *
 * @param[in]  instance       SPI peripheral instance number.
 * @param[out] bytesRemained  Pointer to store remaining byte count
 *                            (may be NULL).
 * @return STATUS_SUCCESS if complete, STATUS_BUSY if in progress,
 *         STATUS_ERROR if an error occurred.
 */
status_t SPI_DRV_SlaveGetTransferStatus(uint32_t instance,
                                        uint32_t *bytesRemained);

/*@}*/

#if defined(__cplusplus)
}
#endif


/*! @} */


#endif /* SPI_SLAVE_DRIVER_H */

