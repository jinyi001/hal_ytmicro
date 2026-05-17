# Spi_Master_Slave_Dma_Demo

---

## Function Description

This example demonstrates data transmission and reception between an SPI Master and an SPI Slave using DMA mode, along with a self-test procedure:

- SPI1 acts as SPI Master; SPI2 acts as SPI Slave
- Wire the connections as instructed in the routine's printed guide
- Data received by both Master and Slave is output via the serial port for validation

---

## Hardware Requirements

+ **EVB**: YTM32B1MD1-EVB-Q100
+ **Debugger**: J-LINK v9.2 or later version

---

## Software Requirements

+ **SDK**      : YTM32B1MD1_1_4_0 or later version
+ **GCC**      : GNU GCC v10.3 or later version
+ **Keil/MDK** : ARM Keil/MDK v5.36 or later version
+ **IAR***     : IAR v8.4 or later version
+ **VS Code**  : Microsoft Visual Studio Code IDE v1.76 or later version(GNU GCC toolchain, CMake v3.25, ninja, Cortex-Debug extensions installation required)
+ **J-LINK** : SEGGER JLINK v6.96 or later version, YTM32B1MD1 patch(support package) installation required.
+ **Ozone**  : SEGGER Ozone v3.28 or later version.
  
---