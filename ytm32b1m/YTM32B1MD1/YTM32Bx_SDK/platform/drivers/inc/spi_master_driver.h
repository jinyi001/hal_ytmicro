/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file spi_master_driver.h
 * @brief SPI master mode driver API declarations.
 *
 * Provides initialization, bus configuration, and data transfer
 * functions for operating the SPI peripheral in master mode. Supports
 * polling, interrupt-driven (blocking and non-blocking), and DMA-based
 * transfer mechanisms.
 */

#ifndef SPI_MASTER_DRIVER_H
#define SPI_MASTER_DRIVER_H

#include "spi_shared_function.h"

/*!
 * @addtogroup spi
 * @brief Serial Peripheral Interface Peripheral Driver
 * @{
 */


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief SPI master mode configuration structure.
 *
 * Contains all user-configurable parameters for master mode operation.
 * Pass this structure to SPI_DRV_MasterInit() or
 * SPI_DRV_MasterConfigureBus() to configure the SPI bus.
 */
typedef struct
{
    uint32_t bitsPerSec;                 /*!< Baud rate in bits per second*/
    spi_which_pcs_t whichPcs;            /*!< Selects which PCS to use */
    spi_signal_polarity_t pcsPolarity;   /*!< PCS polarity */
    bool isPcsContinuous;                /*!< Keeps PCS asserted until transfer complete */
    uint16_t bitcount;                   /*!< Number of bits/frame, minimum is 8-bits */
    spi_clock_phase_t clkPhase;          /*!< Selects which phase of clock to capture data */
    spi_sck_polarity_t clkPolarity;      /*!< Selects clock polarity */
    bool lsbFirst;                       /*!< Option to transmit LSB first */
    spi_transfer_type transferType;      /*!< Type of SPI transfer */
#if FEATURE_SPI_HAS_DMA_ENABLE
    uint8_t rxDMAChannel;                /*!< Channel number for DMA rx channel. If DMA mode isn't used this field will be ignored. */
    uint8_t txDMAChannel;                /*!< Channel number for DMA tx channel. If DMA mode isn't used this field will be ignored. */
#endif
    spi_callback_t callback;             /*!< Select the callback to transfer complete */
    void *callbackParam;                 /*!< Select additional callback parameters if it's necessary */
    spi_transfer_width_t width;          /*!< Transfer width, single, 2-bit, or 4-bit transfer. */
} spi_master_config_t;


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
 * @brief Fill the master configuration structure with default values.
 *
 * Populates all fields of the spi_master_config_t structure with safe
 * defaults: 50 kbps baud rate, 8-bit frames, PCS0, interrupt mode.
 *
 * @param[out] spiConfig  Pointer to the configuration structure to fill.
 *
 * @note The default source clock is assumed to be 8 MHz. Update the
 *       bitsPerSec field if using a different clock frequency.
 */
void SPI_DRV_MasterGetDefaultConfig(spi_master_config_t *spiConfig);

/*!
 * @brief Initialize the SPI instance in master mode.
 *
 * Resets the SPI module, configures it for master operation based on
 * the provided configuration, enables interrupts, and activates the
 * module. The runtime state structure is initialized for transfer
 * tracking.
 *
 * @param[in] instance   SPI peripheral instance number.
 * @param[in] spiState   Pointer to the runtime state structure
 *                       (user-allocated, driver-populated).
 * @param[in] spiConfig  Pointer to the master configuration.
 * @return STATUS_SUCCESS on success, error code on failure.
 *
 * @pre The SPI peripheral clock must be enabled via clock_manager.
 * @post The SPI module is enabled and ready for transfers.
 */
status_t SPI_DRV_MasterInit(uint32_t instance, spi_state_t *spiState,
                            const spi_master_config_t *spiConfig);

/*!
 * @brief De-initialize the SPI master instance.
 *
 * Resets the SPI module, disables its interrupt, destroys the
 * semaphore, and clears the state pointer. Must not be called while
 * a transfer is in progress.
 *
 * @param[in] instance  SPI peripheral instance number.
 * @return STATUS_SUCCESS on success.
 *
 * @pre No transfer should be in progress.
 * @post The SPI module is disabled and its state is cleared.
 */
status_t SPI_DRV_MasterDeinit(uint32_t instance);

/*@}*/

/*!
 * @name Bus Configuration
 * @{
 */

/*!
 * @brief Reconfigure the SPI bus parameters at runtime.
 *
 * Applies a new bus configuration (baud rate, clock polarity/phase,
 * PCS, frame size, etc.) without requiring de-init and re-init. The
 * module is temporarily disabled during reconfiguration.
 *
 * @param[in]  instance          SPI peripheral instance number.
 * @param[in]  spiConfig         Pointer to the new configuration.
 * @param[out] calculatedBaudRate Pointer to store the actual calculated
 *                               baud rate (may be NULL if not needed).
 * @return STATUS_SUCCESS on success, STATUS_ERROR on failure.
 */
status_t SPI_DRV_MasterConfigureBus(uint32_t instance,
                                    const spi_master_config_t *spiConfig,
                                    uint32_t *calculatedBaudRate);

/*!
 * @brief Configure SPI master timing delays.
 *
 * Adjusts the SCK-to-PCS, PCS-to-SCK, and between-transfer delays
 * in units of baud rate clock cycles. The module is temporarily
 * disabled during configuration.
 *
 * @param[in] instance              SPI peripheral instance number.
 * @param[in] delayBetwenTransfers  Minimum delay between transfers (clock cycles).
 * @param[in] delaySCKtoPCS         SCK-to-PCS delay (clock cycles).
 * @param[in] delayPCStoSCK         PCS-to-SCK delay (clock cycles).
 * @return STATUS_SUCCESS on success, STATUS_ERROR if module is busy.
 *
 * @note Default delays are set automatically during init. Call this
 *       function only if non-default timing is required.
 */
status_t SPI_DRV_MasterSetDelay(uint32_t instance, uint32_t delayBetwenTransfers,
                                uint32_t delaySCKtoPCS, uint32_t delayPCStoSCK);

/*!
 * @brief Select the active chip select signal and its polarity.
 *
 * Temporarily disables the module, updates the PCS selection and
 * polarity, then re-enables the module.
 *
 * @param[in] instance  SPI peripheral instance number.
 * @param[in] whichPcs  Chip select signal to activate.
 * @param[in] polarity  Active-high or active-low polarity.
 * @return STATUS_SUCCESS on success, STATUS_ERROR on failure.
 */
status_t SPI_DRV_SetPcs(uint32_t instance, spi_which_pcs_t whichPcs, spi_signal_polarity_t polarity);

/*@}*/

/*!
 * @name Polling Transfer
 * @{
 */

/*!
 * @brief Perform a polling (busy-wait) full-duplex master transfer.
 *
 * Sends and receives data by polling status flags directly, without
 * using interrupts. The function blocks until all data is transferred.
 * This is the simplest transfer mode but consumes CPU during the
 * entire transfer.
 *
 * @param[in]  instance          SPI peripheral instance number.
 * @param[in]  sendBuffer        Pointer to TX data (NULL to send zeros).
 * @param[out] receiveBuffer     Pointer to RX buffer (NULL to discard).
 * @param[in]  transferByteCount Number of bytes to transfer.
 * @return STATUS_SUCCESS on success, STATUS_BUSY if module is busy.
 *
 * @note The transferByteCount must be a multiple of bytes-per-frame.
 */
status_t SPI_DRV_MasterTransferPolling(uint32_t instance,
                                       const uint8_t *sendBuffer,
                                       uint8_t *receiveBuffer,
                                       uint16_t transferByteCount);

/*@}*/

/*!
 * @name Blocking Transfer
 * @{
 */

/*!
 * @brief Perform an interrupt-driven blocking full-duplex master transfer.
 *
 * Initiates a transfer using interrupts and blocks until completion
 * or timeout. Uses a semaphore internally for synchronization.
 *
 * @param[in]  instance          SPI peripheral instance number.
 * @param[in]  sendBuffer        Pointer to TX data (NULL to send zeros).
 * @param[out] receiveBuffer     Pointer to RX buffer (NULL to discard).
 * @param[in]  transferByteCount Number of bytes to transfer.
 * @param[in]  timeout           Timeout in milliseconds.
 * @return STATUS_SUCCESS on success, STATUS_BUSY if already
 *         transferring, STATUS_TIMEOUT if timed out.
 *
 * @note The transferByteCount must be a multiple of bytes-per-frame.
 */
status_t SPI_DRV_MasterTransferBlocking(uint32_t instance,
                                        const uint8_t *sendBuffer,
                                        uint8_t *receiveBuffer,
                                        uint16_t transferByteCount,
                                        uint32_t timeout);

/*@}*/

/*!
 * @name Non-blocking Transfer
 * @{
 */

/*!
 * @brief Start a non-blocking full-duplex master transfer.
 *
 * Initiates a transfer using interrupts or DMA (depending on
 * transferType) and returns immediately. Use
 * SPI_DRV_MasterGetTransferStatus() to poll for completion, or
 * register a callback via the configuration structure.
 *
 * @param[in]  instance          SPI peripheral instance number.
 * @param[in]  sendBuffer        Pointer to TX data (NULL to send zeros).
 * @param[out] receiveBuffer     Pointer to RX buffer (NULL to discard).
 * @param[in]  transferByteCount Number of bytes to transfer.
 * @return STATUS_SUCCESS on success, STATUS_BUSY if already transferring.
 *
 * @note The transferByteCount must be a multiple of bytes-per-frame.
 */
status_t SPI_DRV_MasterTransfer(uint32_t instance,
                                const uint8_t *sendBuffer,
                                uint8_t *receiveBuffer,
                                uint16_t transferByteCount);

/*!
 * @brief Query the status of an ongoing non-blocking master transfer.
 *
 * Returns whether the transfer is still in progress or has completed,
 * and optionally reports the number of bytes remaining to receive.
 *
 * @param[in]  instance       SPI peripheral instance number.
 * @param[out] bytesRemained  Pointer to store remaining byte count
 *                            (may be NULL).
 * @return STATUS_SUCCESS if complete, STATUS_BUSY if in progress,
 *         STATUS_ERROR if an error occurred.
 */
status_t SPI_DRV_MasterGetTransferStatus(uint32_t instance, uint32_t *bytesRemained);

/*!
 * @brief Abort an in-progress non-blocking master transfer.
 *
 * Stops the current transfer, disables interrupts/DMA, flushes
 * FIFOs, and resets the transfer state.
 *
 * @param[in] instance  SPI peripheral instance number.
 * @return STATUS_SUCCESS.
 */
status_t SPI_DRV_MasterAbortTransfer(uint32_t instance);

/* @}*/

#if defined(__cplusplus)
}
#endif

/*! @}*/


#endif /* __SPI_MASTER_DRIVER_H__*/
/*******************************************************************************
 * EOF
 ******************************************************************************/
